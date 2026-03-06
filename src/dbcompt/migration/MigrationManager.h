#pragma once
#include "../dbcompt_global.h"
#include "../schema/SchemaTypes.h"
#include "../driver/IDriver.h"
#include <QString>

namespace dbcompt {

class DBCOMPT_EXPORT MigrationManager {
public:
    // 执行全量迁移：建表、增列、改名、删列、建索引
    // dbPath 仅用于 SQLite 备份，传空字符串则跳过备份
    bool migrate(IDriver* driver, const SchemaDef& schema,
                 const QString& dbPath, QString& outError);

private:
    bool migrateTable(IDriver* driver, const TableDef& table,
                      const QString& backendType, QString& outError);

    bool createTable(IDriver* driver, const TableDef& table,
                     const QString& backendType, QString& outError);

    bool ensureIndexes(IDriver* driver, const TableDef& table, QString& outError);
};

} // namespace dbcompt
