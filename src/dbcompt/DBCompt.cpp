#include "DBCompt.h"
#include "core/DBCore.h"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>

DBCompt* DBCompt::s_instance = nullptr;

DBCompt* DBCompt::instance()
{
    if (!s_instance)
        s_instance = new DBCompt();
    return s_instance;
}

void DBCompt::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

DBCompt::DBCompt(QObject* parent)
    : QObject(parent)
    , m_core(std::make_unique<dbcompt::DBCore>())
{}

DBCompt::~DBCompt()
{
    if (m_core && m_core->isOpen())
        m_core->close();
}

bool DBCompt::open(const QString& schemaFilePath)
{
    QString path = schemaFilePath;
    if (QDir::isRelativePath(path))
        path = QCoreApplication::applicationDirPath() + QStringLiteral("/") + path;

    QString err;
    if (!m_core->open(path, err)) {
        m_lastError = err;
        emit errorOccurred(err);
        return false;
    }
    emit opened();
    return true;
}

void DBCompt::close()
{
    m_core->close();
    emit closed();
}

bool DBCompt::isOpen() const
{
    return m_core->isOpen();
}

QString DBCompt::insert(const QString& table, const QVariantMap& values)
{
    auto result = m_core->insert(table, values);
    if (!result.success) {
        m_lastError = result.error;
        emit errorOccurred(result.error);
        return {};
    }
    return result.lastInsertId;
}

QVariantList DBCompt::select(const QString& table, const QVariantMap& conditions,
                              const QString& orderBy, int limit)
{
    auto result = m_core->select(table, conditions, orderBy, limit);
    if (!result.success)
        m_lastError = result.error;
    return result.rows;
}

bool DBCompt::update(const QString& table, const QVariantMap& values,
                      const QVariantMap& conditions)
{
    auto result = m_core->update(table, values, conditions);
    if (!result.success) {
        m_lastError = result.error;
        emit errorOccurred(result.error);
        return false;
    }
    return true;
}

bool DBCompt::remove(const QString& table, const QVariantMap& conditions)
{
    auto result = m_core->remove(table, conditions);
    if (!result.success) {
        m_lastError = result.error;
        emit errorOccurred(result.error);
        return false;
    }
    return true;
}

QVariantList DBCompt::rawQuery(const QString& sql, const QVariantList& bindings)
{
    auto result = m_core->rawQuery(sql, bindings);
    if (!result.success)
        m_lastError = result.error;
    return result.rows;
}

bool DBCompt::rawExecute(const QString& sql, const QVariantList& bindings)
{
    auto result = m_core->rawExecute(sql, bindings);
    if (!result.success) {
        m_lastError = result.error;
        emit errorOccurred(result.error);
        return false;
    }
    return true;
}

QString DBCompt::lastError() const
{
    return m_lastError;
}
