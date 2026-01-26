#ifndef YEFS_MESSAGEBUS_H
#define YEFS_MESSAGEBUS_H

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QList>
#include <QPair>
#include <QJSValue>
#include <QQmlEngine>

namespace YEFS {

/**
 * @brief 消息总线 - 用于模块间通信
 * 
 * 支持发布/订阅模式，C++ 和 QML 都可以使用
 */
class MessageBus : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static MessageBus* instance();
    static MessageBus* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    // C++ API
    void publish(const QString& topic, const QVariant& data = QVariant());
    void subscribe(const QString& topic, QObject* receiver, const char* slot);
    void unsubscribe(const QString& topic, QObject* receiver);
    void unsubscribeAll(QObject* receiver);

    // QML API
    Q_INVOKABLE void send(const QString& topic, const QVariant& data = QVariant());

signals:
    /**
     * @brief 全局消息信号，QML 可以通过 Connections 监听
     */
    void message(const QString& topic, const QVariant& data);

private:
    explicit MessageBus(QObject* parent = nullptr);
    ~MessageBus() override = default;

    static MessageBus* s_instance;
    QMultiHash<QString, QPair<QObject*, const char*>> m_subscribers;
};

// 预定义消息主题
namespace Topics {
    // 地图相关
    constexpr const char* MAP_READY = "map/ready";
    constexpr const char* MAP_CENTER_CHANGED = "map/center/changed";
    constexpr const char* MAP_ZOOM_CHANGED = "map/zoom/changed";
    constexpr const char* MAP_CLICKED = "map/clicked";
    constexpr const char* MAP_LAYER_ADDED = "map/layer/added";
    constexpr const char* MAP_LAYER_REMOVED = "map/layer/removed";
    constexpr const char* MAP_STYLE_CHANGED = "map/style/changed";

    // 插件相关
    constexpr const char* PLUGIN_LOADED = "plugin/loaded";
    constexpr const char* PLUGIN_UNLOADED = "plugin/unloaded";
    constexpr const char* PLUGIN_ERROR = "plugin/error";

    // 系统相关
    constexpr const char* APP_READY = "app/ready";
    constexpr const char* CONFIG_CHANGED = "config/changed";

    // 单位设置相关
    constexpr const char* UNITS_CHANGED = "units/changed";
    constexpr const char* UNITS_PRESET_APPLIED = "units/preset/applied";
}

} // namespace YEFS

#endif // YEFS_MESSAGEBUS_H
