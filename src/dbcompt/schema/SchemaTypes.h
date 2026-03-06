#pragma once
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>

namespace dbcompt {

struct FieldDef {
    QString name;
    QString type;           // "text" | "integer" | "real" | "blob" | "boolean"
    bool    primaryKey    = false;
    bool    notNull       = false;
    bool    unique        = false;
    bool    autoIncrement = false;
    QString defaultValue;   // 空字符串表示无默认值
};

struct IndexDef {
    QString     name;
    QStringList fields;
    bool        unique = false;
};

// 字段迁移映射：声明式说明哪些列需要改名/删除
struct MigrationMapping {
    QMap<QString, QString> rename;  // oldName -> newName
    QStringList            drop;    // 要删除的列名
};

struct TableDef {
    QString              name;
    QList<FieldDef>      fields;
    QList<IndexDef>      indexes;
    MigrationMapping     migration;
};

struct DatabaseConfig {
    QString type;       // "sqlite" | "postgres"
    // SQLite
    QString path;
    // PostgreSQL / PostGIS
    QString host;
    int     port     = 5432;
    QString dbname;
    QString user;
    QString password;
};

struct SchemaDef {
    QString          version;
    DatabaseConfig   database;
    QList<TableDef>  tables;
};

} // namespace dbcompt
