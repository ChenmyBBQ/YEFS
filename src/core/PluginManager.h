#ifndef YEFS_PLUGINMANAGER_H
#define YEFS_PLUGINMANAGER_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QDir>
#include <QQmlEngine>
#include "IPlugin.h"

namespace YEFS {

class MessageBus;

/**
 * @brief 插件管理器
 * 
 * 负责插件的发现、加载、卸载和生命周期管理
 */
class PluginManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QList<QObject*> plugins READ pluginObjects NOTIFY pluginsChanged)
    Q_PROPERTY(int pluginCount READ pluginCount NOTIFY pluginsChanged)

public:
    static PluginManager* instance();
    static PluginManager* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    // 插件目录
    void addPluginPath(const QString& path);
    QStringList pluginPaths() const { return m_pluginPaths; }

    // 插件管理
    void scanPlugins();
    bool loadPlugin(const QString& pluginId);
    bool unloadPlugin(const QString& pluginId);
    void loadAllPlugins();

    // 查询
    IPlugin* plugin(const QString& pluginId) const;
    QList<IPlugin*> plugins() const { return m_plugins.values(); }
    QList<IPlugin*> pluginsByType(IPlugin::PluginType type) const;
    int pluginCount() const { return m_plugins.count(); }

    // QML 访问
    QList<QObject*> pluginObjects() const;
    Q_INVOKABLE QObject* getPlugin(const QString& pluginId) const;
    Q_INVOKABLE QUrl getPluginQmlEntry(const QString& pluginId) const;
    Q_INVOKABLE QUrl getPluginSettingsPage(const QString& pluginId) const;
    Q_INVOKABLE bool isPluginLoaded(const QString& pluginId) const;

signals:
    void pluginsChanged();
    void pluginLoaded(const QString& pluginId);
    void pluginUnloaded(const QString& pluginId);
    void pluginError(const QString& pluginId, const QString& error);

private:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager() override;

    void createPluginContext();
    bool loadPluginFromPath(const QString& filePath);

    static PluginManager* s_instance;
    QStringList m_pluginPaths;
    QHash<QString, IPlugin*> m_plugins;
    QHash<QString, QString> m_pluginFiles;  // pluginId -> filePath
    PluginContext* m_context = nullptr;
};

} // namespace YEFS

#endif // YEFS_PLUGINMANAGER_H
