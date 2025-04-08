#ifndef FILENAME_REALTIME_STRATEGY_H
#define FILENAME_REALTIME_STRATEGY_H

#include "basestrategy.h"

DFM_SEARCH_BEGIN_NS

// TODO (search): Impl me

/**
 * @brief 文件名实时搜索策略
 */
class FileNameRealTimeStrategy : public FileNameBaseStrategy
{
    Q_OBJECT

public:
    explicit FileNameRealTimeStrategy(const SearchOptions &options, QObject *parent = nullptr);
    ~FileNameRealTimeStrategy() override;

    void search(const SearchQuery &query) override;
    void cancel() override;

private:
    // 拼音匹配
    bool matchPinyin(const QString &fileName, const QString &keyword);

    // 布尔查询匹配
    bool matchBoolean(const QString &fileName, const SearchQuery &query,
                      bool caseSensitive, bool pinyinEnabled);
};

DFM_SEARCH_END_NS

#endif   // FILENAME_REALTIME_STRATEGY_H
