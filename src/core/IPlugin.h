#ifndef YEFS_IPLUGIN_H
#define YEFS_IPLUGIN_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QJsonObject>
#include <QtPlugin>

namespace YEFS {

class PluginContext;

/**
 * @brief 插件基础接口
 * 
 * 所有 YEFS 插件都必须实现此接口
 */
class IPlugin : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(PluginType pluginType READ pluginType CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

public:
    enum PluginType {
        ToolPlugin,         // 工具插件 (航线规划、空域管理等)
        DataProvider,       // 数据提供者 (ICAO数据、气象数据等)
        UIExtension,        // UI扩展 (面板、工具栏等)
        MapOverlay          // 地图叠加层 (图层、标注等)
    };
    Q_ENUM(PluginType)

    explicit IPlugin(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IPlugin() = default;

    // 生命周期
    virtual bool initialize(PluginContext* context) = 0;
    virtual void shutdown() = 0;

    // 插件信息
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QString author() const { return QString(); }
    virtual QString description() const { return QString(); }
    virtual PluginType pluginType() const = 0;

    // 依赖管理
    virtual QStringList dependencies() const { return {}; }

    // 状态
    virtual bool isEnabled() const { return m_enabled; }
    virtual void setEnabled(bool enabled) { 
        if (m_enabled != enabled) {
            m_enabled = enabled;
            emit enabledChanged(enabled);
        }
    }

    // QML 资源
    virtual QUrl qmlEntry() const { return {}; }           // 插件主 QML 页面
    virtual QUrl settingsPage() const { return {}; }       // 设置页面
    virtual QUrl toolbarItem() const { return {}; }        // 工具栏项
    virtual QUrl sidePanel() const { return {}; }          // 侧边面板

signals:
    void enabledChanged(bool enabled);
    void messageToHost(const QString& topic, const QVariant& data);

public slots:
    virtual void onMessageFromHost(const QString& topic, const QVariant& data) {
        Q_UNUSED(topic)
        Q_UNUSED(data)
    }

protected:
    bool m_enabled = true;
    PluginContext* m_context = nullptr;
};

/**
 * @brief 插件上下文 - 提供插件访问主框架服务的接口
 */
class PluginContext : public QObject
{
    Q_OBJECT

public:
    explicit PluginContext(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~PluginContext() = default;

    // 获取服务
    virtual QObject* getService(const QString& serviceName) = 0;
    
    // 消息总线
    virtual void sendMessage(const QString& topic, const QVariant& data) = 0;
    
    // 配置
    virtual QVariant getConfig(const QString& key, const QVariant& defaultValue = {}) = 0;
    virtual void setConfig(const QString& key, const QVariant& value) = 0;
};

} // namespace YEFS

#define YEFS_PLUGIN_IID "org.yefs.plugin/1.0"
Q_DECLARE_INTERFACE(YEFS::IPlugin, YEFS_PLUGIN_IID)

#endif // YEFS_IPLUGIN_H
