/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     xushitong <xushitong@uniontech.com>
 *
 * Maintainer: xushitong <xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/dfmdevice.h"
#include "dfmdevicemanager.h"
#include "private/dfmdevicemanager_p.h"
#include "dfmblockmonitor.h"

#include <QDebug>

DFM_MOUNT_USE_NS

DFMDeviceManagerPrivate::DFMDeviceManagerPrivate(DFMDeviceManager *qq)
    : q(qq)
{

}

bool DFMDeviceManagerPrivate::registerMonitor(DeviceType type, DFMMonitor *monitor)
{
    Q_ASSERT_X(monitor, __FUNCTION__, "monitor must be valid to register!");

    if (monitors.contains(type)) {
        lastError = MonitorError::MonitorAlreadyRegistered;
        return false;
    }
    monitors.insert(type, monitor);

    QObject::connect(monitor, &DFMMonitor::driveAdded,       q, &DFMDeviceManager::driveAdded);
    QObject::connect(monitor, &DFMMonitor::driveRemoved,     q, &DFMDeviceManager::driveRemoved);
    QObject::connect(monitor, &DFMMonitor::deviceAdded,      q, &DFMDeviceManager::blockDeviceAdded);
    QObject::connect(monitor, &DFMMonitor::deviceRemoved,    q, &DFMDeviceManager::blockDeviceRemoved);
    QObject::connect(monitor, &DFMMonitor::mountAdded,       q, &DFMDeviceManager::mounted);
    QObject::connect(monitor, &DFMMonitor::mountRemoved,     q, &DFMDeviceManager::unmounted);
    QObject::connect(monitor, &DFMMonitor::propertyChanged,  q, &DFMDeviceManager::propertyChanged);

    return true;
}

DFMMonitor *DFMDeviceManagerPrivate::getRegisteredMonitor(DeviceType type) const
{
    return monitors.value(type, nullptr);
}

bool DFMDeviceManagerPrivate::startMonitor()
{
    bool res = true;
    for (const auto &monitor: monitors) {
        int type = static_cast<int>(monitor->monitorObjectType());
        res &= monitor->startMonitor();
        if (res)
            qDebug() << type << "started...";
        else
            qWarning() << type << "failed to start...";
    }
    return res;
}

bool DFMDeviceManagerPrivate::stopMonitor()
{
    bool res = true;
    for (const auto &monitor: monitors) {
        int type = static_cast<int>(monitor->monitorObjectType());
        res &= monitor->stopMonitor();
        if (res)
            qDebug() << type << "stopped...";
        else
            qWarning() << type << "failed to stop...";
    }
    return res;
}

QList<DFMDevice *> DFMDeviceManagerPrivate::devices(DeviceType type)
{
    auto getAllDev = [this]{
        QList<DFMDevice *> ret;
        for (const auto &monitor: monitors) {
            if (monitor)
                ret << monitor->getDevices();
            else
                lastError = MonitorError::MonitorNotRegister;
        }
        return ret;
    };

    auto getDevsOfType = [this](DeviceType type) {
        auto monitor = monitors.value(type);
        if (monitor)
            return monitor->getDevices();
        lastError = MonitorError::MonitorNotRegister;
        return QList<DFMDevice *>();
    };

    switch (type) {
    case DeviceType::AllDevice:
        return getAllDev();
    case DeviceType::NetDevice:     // by intentionally
    case DeviceType::ProtocolDevice:
    case DeviceType::BlockDevice:
        return getDevsOfType(type);
    }
    return QList<DFMDevice *>();
}

DFMDeviceManager::DFMDeviceManager(QObject *parent)
    : QObject (parent), d(new DFMDeviceManagerPrivate(this))
{
    registerMonitor(DeviceType::BlockDevice, new DFMBlockMonitor(this));
}

DFMDeviceManager::~DFMDeviceManager()
{

}

DFMDeviceManager *DFMDeviceManager::instance()
{
    static DFMDeviceManager mng;
    return &mng;
}

/*!
 * \brief DFMDeviceManager::registerMonitor for register new device monitors
 * \param type      the type of Device, must be declared in DeviceType
 * \param monitor   the instance of Monitor
 * \return          return true if success, return false when the specified type is already registered.
 */
bool DFMDeviceManager::registerMonitor(DeviceType type, DFMMonitor *monitor)
{
    return d->registerMonitor(type, monitor);
}

DFMMonitor *DFMDeviceManager::getRegisteredMonitor(DeviceType type) const
{
    return d->getRegisteredMonitor(type);
}

bool DFMDeviceManager::startMonitorWatch()
{
    return d->startMonitor();
}

bool DFMDeviceManager::stopMonitorWatch()
{
    return d->stopMonitor();
}

MonitorError DFMDeviceManager::lastError() const
{
    return d->lastError;
}

QList<DFMDevice *> DFMDeviceManager::devices(DeviceType type)
{
    return d->devices(type);
}

