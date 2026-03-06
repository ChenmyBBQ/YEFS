#pragma once
#include "../schema/SchemaTypes.h"
#include "../query/QueryResult.h"
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace dbcompt {

// 数据库驱动抽象接口
class IDriver {
public:
    virtual ~IDriver() = default;

    virtual bool open(const DatabaseConfig& config, const QString& connectionName) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    // 执行 DDL 或无返回值操作
    virtual bool execRaw(const QString& sql) = 0;

    // 执行查询，返回结果行集合
    virtual QueryResult query(const QString& sql, const QVariantList& bindings = {}) = 0;

    // 执行 INSERT/UPDATE/DELETE，返回影响行数、lastInsertId 等
    virtual QueryResult execute(const QString& sql, const QVariantList& bindings = {}) = 0;

    virtual bool beginTransaction() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;

    virtual QString backendType()     const = 0;
    virtual QString connectionName()  const = 0;
    virtual QString lastError()       const = 0;

    // 获取指定表的现有列名
    virtual QStringList tableColumns(const QString& tableName) = 0;

    // 获取数据库中所有表名
    virtual QStringList tables() = 0;
};

} // namespace dbcompt
