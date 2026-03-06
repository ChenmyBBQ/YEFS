#pragma once
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace dbcompt {

struct QueryResult {
    bool         success      = false;
    QString      error;
    QVariantList rows;           // 每个元素为 QVariantMap（列名 -> 值）
    QString      lastInsertId;
    int          affectedRows = 0;
};

} // namespace dbcompt
