#include "QueryBuilder.h"

namespace dbcompt {

// ── 私有工具函数 ──────────────────────────────────────────────────────────────

QString QueryBuilder::esc(const QString& name)
{
    return QStringLiteral("\"") + name + QStringLiteral("\"");
}

QString QueryBuilder::fieldTypeToDDL(const QString& type, const QString& /*backendType*/)
{
    if (type == QLatin1String("text")    || type == QLatin1String("string"))         return QStringLiteral("TEXT");
    if (type == QLatin1String("integer") || type == QLatin1String("int"))            return QStringLiteral("INTEGER");
    if (type == QLatin1String("real")    || type == QLatin1String("float")
                                         || type == QLatin1String("double"))         return QStringLiteral("REAL");
    if (type == QLatin1String("blob")    || type == QLatin1String("binary"))         return QStringLiteral("BLOB");
    if (type == QLatin1String("boolean") || type == QLatin1String("bool"))           return QStringLiteral("INTEGER");
    if (type == QLatin1String("timestamp")|| type == QLatin1String("datetime"))      return QStringLiteral("TEXT");
    return type.toUpper();
}

QString QueryBuilder::fieldDefToDDL(const FieldDef& field, const QString& backendType)
{
    QString sql = esc(field.name) + QStringLiteral(" ") + fieldTypeToDDL(field.type, backendType);
    if (field.primaryKey)                          sql += QStringLiteral(" PRIMARY KEY");
    if (field.autoIncrement && backendType == QLatin1String("sqlite"))
                                                   sql += QStringLiteral(" AUTOINCREMENT");
    if (field.notNull    && !field.primaryKey)     sql += QStringLiteral(" NOT NULL");
    if (field.unique     && !field.primaryKey)     sql += QStringLiteral(" UNIQUE");
    if (!field.defaultValue.isEmpty())             sql += QStringLiteral(" DEFAULT '") + field.defaultValue + QStringLiteral("'");
    return sql;
}

// ── DDL ──────────────────────────────────────────────────────────────────────

QString QueryBuilder::buildCreateTable(const TableDef& table, const QString& backendType)
{
    QStringList cols;
    for (const auto& f : table.fields)
        cols.append(fieldDefToDDL(f, backendType));

    return QStringLiteral("CREATE TABLE IF NOT EXISTS ") + esc(table.name)
         + QStringLiteral(" (\n  ") + cols.join(QStringLiteral(",\n  ")) + QStringLiteral("\n)");
}

QString QueryBuilder::buildCreateIndex(const IndexDef& index, const QString& tableName)
{
    QStringList escapedFields;
    for (const auto& f : index.fields)
        escapedFields.append(esc(f));

    return (index.unique ? QStringLiteral("CREATE UNIQUE INDEX IF NOT EXISTS ")
                         : QStringLiteral("CREATE INDEX IF NOT EXISTS "))
         + esc(index.name) + QStringLiteral(" ON ") + esc(tableName)
         + QStringLiteral(" (") + escapedFields.join(QStringLiteral(", ")) + QStringLiteral(")");
}

QString QueryBuilder::buildDropTable(const QString& tableName)
{
    return QStringLiteral("DROP TABLE IF EXISTS ") + esc(tableName);
}

QString QueryBuilder::buildAddColumn(const QString& tableName, const FieldDef& field,
                                      const QString& backendType)
{
    return QStringLiteral("ALTER TABLE ") + esc(tableName)
         + QStringLiteral(" ADD COLUMN ") + fieldDefToDDL(field, backendType);
}

QString QueryBuilder::buildRenameColumn(const QString& tableName,
                                         const QString& oldName, const QString& newName)
{
    // SQLite 3.25+ 和 PostgreSQL 均支持 RENAME COLUMN
    return QStringLiteral("ALTER TABLE ") + esc(tableName)
         + QStringLiteral(" RENAME COLUMN ") + esc(oldName)
         + QStringLiteral(" TO ") + esc(newName);
}

QString QueryBuilder::buildDropColumn(const QString& tableName, const QString& columnName)
{
    // SQLite 3.35+ 和 PostgreSQL 均支持 DROP COLUMN
    return QStringLiteral("ALTER TABLE ") + esc(tableName)
         + QStringLiteral(" DROP COLUMN ") + esc(columnName);
}

// ── DML ──────────────────────────────────────────────────────────────────────

PreparedStatement QueryBuilder::buildInsert(const QString& tableName, const QVariantMap& values)
{
    PreparedStatement ps;
    QStringList cols, placeholders;
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        cols.append(esc(it.key()));
        placeholders.append(QStringLiteral("?"));
        ps.bindings.append(it.value());
    }
    ps.sql = QStringLiteral("INSERT INTO ") + esc(tableName)
           + QStringLiteral(" (") + cols.join(QStringLiteral(", "))
           + QStringLiteral(") VALUES (") + placeholders.join(QStringLiteral(", "))
           + QStringLiteral(")");
    return ps;
}

PreparedStatement QueryBuilder::buildSelect(const QString& tableName,
                                             const QVariantMap& conditions,
                                             const QString& orderBy,
                                             int limit)
{
    PreparedStatement ps;
    ps.sql = QStringLiteral("SELECT * FROM ") + esc(tableName);

    if (!conditions.isEmpty()) {
        QStringList clauses;
        for (auto it = conditions.constBegin(); it != conditions.constEnd(); ++it) {
            clauses.append(esc(it.key()) + QStringLiteral(" = ?"));
            ps.bindings.append(it.value());
        }
        ps.sql += QStringLiteral(" WHERE ") + clauses.join(QStringLiteral(" AND "));
    }

    if (!orderBy.isEmpty())
        ps.sql += QStringLiteral(" ORDER BY ") + orderBy;

    if (limit > 0)
        ps.sql += QStringLiteral(" LIMIT ") + QString::number(limit);

    return ps;
}

PreparedStatement QueryBuilder::buildUpdate(const QString& tableName,
                                             const QVariantMap& values,
                                             const QVariantMap& conditions)
{
    PreparedStatement ps;
    QStringList setClauses;
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        setClauses.append(esc(it.key()) + QStringLiteral(" = ?"));
        ps.bindings.append(it.value());
    }
    ps.sql = QStringLiteral("UPDATE ") + esc(tableName)
           + QStringLiteral(" SET ") + setClauses.join(QStringLiteral(", "));

    if (!conditions.isEmpty()) {
        QStringList whereClauses;
        for (auto it = conditions.constBegin(); it != conditions.constEnd(); ++it) {
            whereClauses.append(esc(it.key()) + QStringLiteral(" = ?"));
            ps.bindings.append(it.value());
        }
        ps.sql += QStringLiteral(" WHERE ") + whereClauses.join(QStringLiteral(" AND "));
    }
    return ps;
}

PreparedStatement QueryBuilder::buildDelete(const QString& tableName,
                                             const QVariantMap& conditions)
{
    PreparedStatement ps;
    ps.sql = QStringLiteral("DELETE FROM ") + esc(tableName);

    if (!conditions.isEmpty()) {
        QStringList whereClauses;
        for (auto it = conditions.constBegin(); it != conditions.constEnd(); ++it) {
            whereClauses.append(esc(it.key()) + QStringLiteral(" = ?"));
            ps.bindings.append(it.value());
        }
        ps.sql += QStringLiteral(" WHERE ") + whereClauses.join(QStringLiteral(" AND "));
    }
    return ps;
}

} // namespace dbcompt
