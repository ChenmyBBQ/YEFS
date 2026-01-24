/**
 * @file SettingsManager.h
 * @brief 全局设置管理器 - 使用 JSON 文件存储
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

namespace YEFS {

/**
 * @class SettingsManager
 * @brief 管理应用程序设置，使用 settings.json 存储
 */
class SettingsManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static SettingsManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static SettingsManager* instance();

    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager() override;

    // 获取设置值
    Q_INVOKABLE QVariant getValue(const QString &category, const QString &key, const QVariant &defaultValue = QVariant()) const;
    
    // 设置值
    Q_INVOKABLE void setValue(const QString &category, const QString &key, const QVariant &value);
    
    // 获取整个分类
    Q_INVOKABLE QVariantMap getCategory(const QString &category) const;
    
    // 设置整个分类
    Q_INVOKABLE void setCategory(const QString &category, const QVariantMap &values);
    
    // 保存到文件
    Q_INVOKABLE void save();
    
    // 重新加载
    Q_INVOKABLE void reload();
    
    // 重置所有设置
    Q_INVOKABLE void resetAll();
    
    // 重置某个分类
    Q_INVOKABLE void resetCategory(const QString &category);
    
    // 获取设置文件路径
    Q_INVOKABLE QString settingsFilePath() const;

signals:
    void settingsChanged(const QString &category, const QString &key);
    void categoryChanged(const QString &category);
    void settingsLoaded();
    void settingsSaved();

private:
    void load();
    void ensureSettingsDir();
    
    static SettingsManager* s_instance;
    
    QString m_settingsPath;
    QJsonObject m_settings;
    bool m_dirty;
};

} // namespace YEFS

#endif // SETTINGSMANAGER_H
