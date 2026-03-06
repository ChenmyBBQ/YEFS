#pragma once
#include "dbcompt_global.h"

#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <QString>
#include <memory>

// 内部实现前向声明（使用小写命名空间，避免与此类名 DBCompt 冲突）
namespace dbcompt { class DBCore; }

/**
 * @brief 数据库组件公共 API 单例
 *
 * 在 QML 中 import YEFSApp，然后直接调用 DBCompt.open / insert / select 等。
 * 在 C++ 中通过 DBCompt::instance() 取得单例。
 */
class DBCOMPT_EXPORT DBCompt : public QObject
{
    Q_OBJECT

public:
    static DBCompt* instance();
    static void     destroy();

    // 使用 schema 文件打开数据库（相对路径以 applicationDirPath 为基准）
    Q_INVOKABLE bool open(const QString& schemaFilePath);
    Q_INVOKABLE void close();
    Q_INVOKABLE bool isOpen() const;

    // ── CRUD ─────────────────────────────────────────────────────────────────
    // 插入一行，返回 lastInsertId（失败返回空字符串）
    Q_INVOKABLE QString      insert(const QString& table, const QVariantMap& values);

    // 查询，返回 QVariantList，每个元素为 QVariantMap（列名 -> 值）
    Q_INVOKABLE QVariantList select(const QString& table,
                                     const QVariantMap& conditions = {},
                                     const QString& orderBy        = {},
                                     int limit = -1);

    Q_INVOKABLE bool update(const QString& table,
                             const QVariantMap& values,
                             const QVariantMap& conditions);

    Q_INVOKABLE bool remove(const QString& table, const QVariantMap& conditions);

    // ── 原生 SQL ─────────────────────────────────────────────────────────────
    Q_INVOKABLE QVariantList rawQuery  (const QString& sql, const QVariantList& bindings = {});
    Q_INVOKABLE bool         rawExecute(const QString& sql, const QVariantList& bindings = {});

    Q_INVOKABLE QString lastError() const;

signals:
    void opened();
    void closed();
    void errorOccurred(const QString& error);

private:
    explicit DBCompt(QObject* parent = nullptr);
    ~DBCompt() override;

    std::unique_ptr<dbcompt::DBCore> m_core;
    QString                          m_lastError;

    static DBCompt* s_instance;
};
