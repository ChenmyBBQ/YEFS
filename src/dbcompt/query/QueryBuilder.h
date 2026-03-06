#pragma once
#include "../dbcompt_global.h"
#include "../schema/SchemaTypes.h"
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace dbcompt {

// SQL 语句及其绑定参数
struct PreparedStatement {
    QString      sql;
    QVariantList bindings;
};

class DBCOMPT_EXPORT QueryBuilder {
public:
    // ── DDL ─────────────────────────────────────────────────────────────────
    static QString buildCreateTable(const TableDef& table,    const QString& backendType);
    static QString buildCreateIndex(const IndexDef& index,    const QString& tableName);
    static QString buildDropTable  (const QString& tableName);
    static QString buildAddColumn  (const QString& tableName, const FieldDef& field,  const QString& backendType);
    static QString buildRenameColumn(const QString& tableName, const QString& oldName, const QString& newName);
    static QString buildDropColumn (const QString& tableName, const QString& columnName);

    // ── DML ─────────────────────────────────────────────────────────────────
    static PreparedStatement buildInsert(const QString& tableName, const QVariantMap& values);
    static PreparedStatement buildSelect(const QString& tableName,
                                         const QVariantMap& conditions = {},
                                         const QString& orderBy        = {},
                                         int limit = -1);
    static PreparedStatement buildUpdate(const QString& tableName,
                                         const QVariantMap& values,
                                         const QVariantMap& conditions);
    static PreparedStatement buildDelete(const QString& tableName,
                                         const QVariantMap& conditions);

private:
    static QString fieldTypeToDDL(const QString& type, const QString& backendType);
    static QString fieldDefToDDL (const FieldDef& field, const QString& backendType);
    static QString esc(const QString& name);   // 引号转义列/表名
};

} // namespace dbcompt
