#include "MigrationManager.h"
#include "MigrationBackup.h"
#include "../query/QueryBuilder.h"
#include "../utils/DBLogger.h"

namespace dbcompt {

bool MigrationManager::migrate(IDriver* driver, const SchemaDef& schema,
                                const QString& dbPath, QString& outError)
{
    // 判断是否有破坏性迁移（改名 / 删列），有则先备份
    bool needsBackup = false;
    for (const auto& table : schema.tables) {
        if (!table.migration.rename.isEmpty() || !table.migration.drop.isEmpty()) {
            needsBackup = true;
            break;
        }
    }

    if (needsBackup && !dbPath.isEmpty()) {
        QString backupPath, backupErr;
        if (!MigrationBackup::backup(dbPath, backupPath, backupErr))
            DBLogger::warning(QStringLiteral("数据库备份失败（继续迁移）: ") + backupErr);
        else
            DBLogger::info(QStringLiteral("已备份数据库到: ") + backupPath);
    }

    for (const auto& table : schema.tables) {
        if (!migrateTable(driver, table, schema.database.type, outError))
            return false;
    }
    return true;
}

bool MigrationManager::migrateTable(IDriver* driver, const TableDef& table,
                                     const QString& backendType, QString& outError)
{
    const QStringList existingTables = driver->tables();

    if (!existingTables.contains(table.name))
        return createTable(driver, table, backendType, outError);

    // ── 表已存在，执行增量迁移 ────────────────────────────────────────────────
    QStringList existingCols = driver->tableColumns(table.name);

    // 1. 应用列改名
    for (auto it = table.migration.rename.constBegin();
         it != table.migration.rename.constEnd(); ++it)
    {
        const QString& oldName = it.key();
        const QString& newName = it.value();
        if (existingCols.contains(oldName) && !existingCols.contains(newName)) {
            DBLogger::debug(QStringLiteral("改名列 %1.%2 -> %3")
                            .arg(table.name, oldName, newName));
            const QString sql = QueryBuilder::buildRenameColumn(table.name, oldName, newName);
            if (!driver->execRaw(sql)) {
                outError = QStringLiteral("改名列失败: ") + driver->lastError();
                return false;
            }
            existingCols.removeOne(oldName);
            existingCols.append(newName);
        }
    }

    // 2. 删除标记删除的列
    for (const QString& colToDrop : table.migration.drop) {
        if (existingCols.contains(colToDrop)) {
            DBLogger::debug(QStringLiteral("删除列 %1.%2").arg(table.name, colToDrop));
            const QString sql = QueryBuilder::buildDropColumn(table.name, colToDrop);
            if (!driver->execRaw(sql)) {
                outError = QStringLiteral("删除列失败: ") + driver->lastError();
                return false;
            }
            existingCols.removeOne(colToDrop);
        }
    }

    // 3. 添加 schema 中新增的列
    for (const auto& field : table.fields) {
        if (field.primaryKey) continue;  // PK 列不可追加
        if (!existingCols.contains(field.name)) {
            DBLogger::debug(QStringLiteral("添加列 %1.%2").arg(table.name, field.name));
            const QString sql = QueryBuilder::buildAddColumn(table.name, field, backendType);
            if (!driver->execRaw(sql)) {
                outError = QStringLiteral("添加列失败: ") + driver->lastError();
                return false;
            }
        }
    }

    // 4. 确保索引存在
    return ensureIndexes(driver, table, outError);
}

bool MigrationManager::createTable(IDriver* driver, const TableDef& table,
                                    const QString& backendType, QString& outError)
{
    DBLogger::debug(QStringLiteral("创建表: ") + table.name);
    const QString ddl = QueryBuilder::buildCreateTable(table, backendType);
    if (!driver->execRaw(ddl)) {
        outError = QStringLiteral("创建表 %1 失败: ").arg(table.name) + driver->lastError();
        return false;
    }
    return ensureIndexes(driver, table, outError);
}

bool MigrationManager::ensureIndexes(IDriver* driver, const TableDef& table, QString& outError)
{
    for (const auto& idx : table.indexes) {
        const QString sql = QueryBuilder::buildCreateIndex(idx, table.name);
        if (!driver->execRaw(sql)) {
            outError = QStringLiteral("创建索引失败: ") + driver->lastError();
            return false;
        }
    }
    return true;
}

} // namespace dbcompt
