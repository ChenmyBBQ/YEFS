#include "DBCore.h"
#include "../schema/SchemaLoader.h"
#include "../migration/MigrationManager.h"
#include "../query/QueryBuilder.h"
#include "../driver/SQLiteDriver.h"
#include "../driver/PostgresDriver.h"
#include "../utils/DBLogger.h"

#include <QCoreApplication>
#include <QDir>

namespace dbcompt {

DBCore::DBCore()  = default;
DBCore::~DBCore() { close(); }

bool DBCore::open(const QString& schemaFilePath, QString& outError)
{
    if (m_open) return true;

    // 1. 加载 schema
    if (!SchemaLoader::load(schemaFilePath, m_schema, outError)) {
        DBLogger::error(QStringLiteral("Schema 加载失败: ") + outError);
        return false;
    }

    // 2. 解析 SQLite 路径（相对路径以 applicationDirPath 为基准）
    DatabaseConfig cfg = m_schema.database;
    if (cfg.type == QLatin1String("sqlite")
        && !cfg.path.isEmpty()
        && QDir::isRelativePath(cfg.path))
    {
        cfg.path = QCoreApplication::applicationDirPath() + QStringLiteral("/") + cfg.path;
    }
    m_schema.database = cfg;

    // 3. 创建驱动
    m_driver = createDriver(cfg.type);
    if (!m_driver) {
        outError = QStringLiteral("未知数据库类型: ") + cfg.type;
        return false;
    }

    // 4. 打开连接
    const QString connName = QStringLiteral("dbcompt_") + cfg.type
                           + QStringLiteral("_")
                           + QString::number(reinterpret_cast<quintptr>(this));
    if (!m_driver->open(cfg, connName)) {
        outError = m_driver->lastError();
        DBLogger::error(QStringLiteral("驱动打开失败: ") + outError);
        return false;
    }

    // 5. 执行迁移
    MigrationManager migMgr;
    const QString dbPath = (cfg.type == QLatin1String("sqlite")) ? cfg.path : QString{};
    if (!migMgr.migrate(m_driver.get(), m_schema, dbPath, outError)) {
        DBLogger::error(QStringLiteral("迁移失败: ") + outError);
        m_driver->close();
        return false;
    }

    m_open = true;
    DBLogger::info(QStringLiteral("数据库已打开 (type=%1, path=%2)").arg(cfg.type, cfg.path));
    return true;
}

void DBCore::close()
{
    if (m_open && m_driver) {
        m_driver->close();
        m_open = false;
    }
}

bool DBCore::isOpen() const { return m_open; }

QueryResult DBCore::insert(const QString& table, const QVariantMap& values)
{
    auto ps = QueryBuilder::buildInsert(table, values);
    return m_driver->execute(ps.sql, ps.bindings);
}

QueryResult DBCore::select(const QString& table, const QVariantMap& conditions,
                            const QString& orderBy, int limit)
{
    auto ps = QueryBuilder::buildSelect(table, conditions, orderBy, limit);
    return m_driver->query(ps.sql, ps.bindings);
}

QueryResult DBCore::update(const QString& table, const QVariantMap& values,
                            const QVariantMap& conditions)
{
    auto ps = QueryBuilder::buildUpdate(table, values, conditions);
    return m_driver->execute(ps.sql, ps.bindings);
}

QueryResult DBCore::remove(const QString& table, const QVariantMap& conditions)
{
    auto ps = QueryBuilder::buildDelete(table, conditions);
    return m_driver->execute(ps.sql, ps.bindings);
}

QueryResult DBCore::rawQuery(const QString& sql, const QVariantList& bindings)
{
    return m_driver->query(sql, bindings);
}

QueryResult DBCore::rawExecute(const QString& sql, const QVariantList& bindings)
{
    return m_driver->execute(sql, bindings);
}

std::unique_ptr<IDriver> DBCore::createDriver(const QString& type)
{
    if (type == QLatin1String("sqlite"))
        return std::make_unique<SQLiteDriver>();
    if (type == QLatin1String("postgres")
        || type == QLatin1String("postgresql")
        || type == QLatin1String("postgis"))
        return std::make_unique<PostgresDriver>();
    return nullptr;
}

} // namespace dbcompt
