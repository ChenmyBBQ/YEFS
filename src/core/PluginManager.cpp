#include "PluginManager.h"
#include "MessageBus.h"
#include "SettingsManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace YEFS {

PluginManager* PluginManager::s_instance = nullptr;

/**
 * @brief 默认插件上下文实现
 */
class DefaultPluginContext : public PluginContext
{
public:
    explicit DefaultPluginContext(QObject* parent = nullptr) : PluginContext(parent) {}

    QObject* getService(const QString& serviceName) override {
        if (serviceName == "MessageBus") {
            return MessageBus::instance();
        } else if (serviceName == "SettingsManager") {
            return SettingsManager::instance();
        }
        // 可以添加更多服务
        return nullptr;
    }

    void sendMessage(const QString& topic, const QVariant& data) override {
        MessageBus::instance()->publish(topic, data);
    }

    QVariant getConfig(const QString& key, const QVariant& defaultValue) override {
        // Parse key as "category.key" format
        QStringList parts = key.split('.');
        if (parts.size() == 2) {
            return SettingsManager::instance()->getValue(parts[0], parts[1], defaultValue);
        }
        // If no category specified, use "plugins" as default category
        return SettingsManager::instance()->getValue("plugins", key, defaultValue);
    }

    void setConfig(const QString& key, const QVariant& value) override {
        // Parse key as "category.key" format
        QStringList parts = key.split('.');
        if (parts.size() == 2) {
            SettingsManager::instance()->setValue(parts[0], parts[1], value);
        } else {
            // If no category specified, use "plugins" as default category
            SettingsManager::instance()->setValue("plugins", key, value);
        }
    }
};

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
{
    createPluginContext();

    // 添加默认插件目录
    QString appDir = QCoreApplication::applicationDirPath();
    addPluginPath(appDir + "/plugins");
    addPluginPath(appDir + "/../plugins");

#ifdef Q_OS_LINUX
    addPluginPath(QDir::homePath() + "/.local/share/YEFS/plugins");
#elif defined(Q_OS_WIN)
    addPluginPath(QDir::homePath() + "/AppData/Local/YEFS/plugins");
#elif defined(Q_OS_MACOS)
    addPluginPath(QDir::homePath() + "/Library/Application Support/YEFS/plugins");
#endif
}

PluginManager::~PluginManager()
{
    // 卸载所有插件
    for (auto plugin : m_plugins) {
        plugin->shutdown();
    }
    m_plugins.clear();
}

PluginManager* PluginManager::instance()
{
    if (!s_instance) {
        s_instance = new PluginManager();
    }
    return s_instance;
}

PluginManager* PluginManager::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

void PluginManager::createPluginContext()
{
    m_context = new DefaultPluginContext(this);
}

void PluginManager::addPluginPath(const QString& path)
{
    if (!m_pluginPaths.contains(path)) {
        m_pluginPaths.append(path);
    }
}

void PluginManager::scanPlugins()
{
    qDebug() << "[PluginManager] Scanning plugins in paths:" << m_pluginPaths;

    for (const QString& path : m_pluginPaths) {
        QDir dir(path);
        if (!dir.exists()) continue;

        // 扫描插件文件
#ifdef Q_OS_WIN
        QStringList filters = {"*.dll"};
#elif defined(Q_OS_MACOS)
        QStringList filters = {"*.dylib", "*.bundle"};
#else
        QStringList filters = {"*.so"};
#endif

        for (const QString& fileName : dir.entryList(filters, QDir::Files)) {
            QString filePath = dir.absoluteFilePath(fileName);
            
            QPluginLoader loader(filePath);
            QJsonObject metaData = loader.metaData().value("MetaData").toObject();
            
            if (!metaData.isEmpty()) {
                QString pluginId = metaData.value("id").toString();
                if (!pluginId.isEmpty()) {
                    m_pluginFiles[pluginId] = filePath;
                    qDebug() << "[PluginManager] Found plugin:" << pluginId << "at" << filePath;
                }
            }
        }
    }
}

bool PluginManager::loadPlugin(const QString& pluginId)
{
    if (m_plugins.contains(pluginId)) {
        qDebug() << "[PluginManager] Plugin already loaded:" << pluginId;
        return true;
    }

    if (!m_pluginFiles.contains(pluginId)) {
        qWarning() << "[PluginManager] Plugin not found:" << pluginId;
        emit pluginError(pluginId, "Plugin not found");
        return false;
    }

    return loadPluginFromPath(m_pluginFiles[pluginId]);
}

bool PluginManager::loadPluginFromPath(const QString& filePath)
{
    QPluginLoader loader(filePath);
    QObject* instance = loader.instance();

    if (!instance) {
        qWarning() << "[PluginManager] Failed to load plugin:" << filePath << loader.errorString();
        return false;
    }

    IPlugin* plugin = qobject_cast<IPlugin*>(instance);
    if (!plugin) {
        qWarning() << "[PluginManager] Invalid plugin interface:" << filePath;
        loader.unload();
        return false;
    }

    // 检查依赖
    for (const QString& dep : plugin->dependencies()) {
        if (!m_plugins.contains(dep)) {
            if (!loadPlugin(dep)) {
                qWarning() << "[PluginManager] Failed to load dependency:" << dep;
                loader.unload();
                return false;
            }
        }
    }

    // 初始化插件
    if (!plugin->initialize(m_context)) {
        qWarning() << "[PluginManager] Plugin initialization failed:" << plugin->id();
        loader.unload();
        return false;
    }

    // 连接消息
    connect(plugin, &IPlugin::messageToHost, this, [](const QString& topic, const QVariant& data) {
        MessageBus::instance()->publish(topic, data);
    });

    m_plugins[plugin->id()] = plugin;
    m_pluginFiles[plugin->id()] = filePath;

    qDebug() << "[PluginManager] Loaded plugin:" << plugin->id() << plugin->name();

    emit pluginLoaded(plugin->id());
    emit pluginsChanged();

    MessageBus::instance()->publish(Topics::PLUGIN_LOADED, plugin->id());

    return true;
}

bool PluginManager::unloadPlugin(const QString& pluginId)
{
    if (!m_plugins.contains(pluginId)) {
        return false;
    }

    IPlugin* plugin = m_plugins.take(pluginId);
    plugin->shutdown();

    emit pluginUnloaded(pluginId);
    emit pluginsChanged();

    MessageBus::instance()->publish(Topics::PLUGIN_UNLOADED, pluginId);

    return true;
}

void PluginManager::loadAllPlugins()
{
    scanPlugins();

    for (const QString& pluginId : m_pluginFiles.keys()) {
        if (!m_plugins.contains(pluginId)) {
            loadPlugin(pluginId);
        }
    }
}

IPlugin* PluginManager::plugin(const QString& pluginId) const
{
    return m_plugins.value(pluginId, nullptr);
}

QList<IPlugin*> PluginManager::pluginsByType(IPlugin::PluginType type) const
{
    QList<IPlugin*> result;
    for (IPlugin* plugin : m_plugins) {
        if (plugin->pluginType() == type) {
            result.append(plugin);
        }
    }
    return result;
}

QList<QObject*> PluginManager::pluginObjects() const
{
    QList<QObject*> result;
    for (IPlugin* plugin : m_plugins) {
        result.append(plugin);
    }
    return result;
}

QObject* PluginManager::getPlugin(const QString& pluginId) const
{
    return m_plugins.value(pluginId, nullptr);
}

QUrl PluginManager::getPluginQmlEntry(const QString& pluginId) const
{
    IPlugin* p = plugin(pluginId);
    return p ? p->qmlEntry() : QUrl();
}

QUrl PluginManager::getPluginSettingsPage(const QString& pluginId) const
{
    IPlugin* p = plugin(pluginId);
    return p ? p->settingsPage() : QUrl();
}

bool PluginManager::isPluginLoaded(const QString& pluginId) const
{
    return m_plugins.contains(pluginId);
}

} // namespace YEFS
