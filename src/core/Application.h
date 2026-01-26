#ifndef YEFS_APPLICATION_H
#define YEFS_APPLICATION_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QGuiApplication>

namespace YEFS {

class MessageBus;
class PluginManager;
class MapLibreEngine;

/**
 * @brief YEFS 应用程序类
 * 
 * 管理应用程序生命周期和核心服务初始化
 */
class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QGuiApplication* app, QObject* parent = nullptr);
    ~Application() override;

    // 初始化
    bool initialize();

    // 启动应用
    int run();

    // 获取引擎
    QQmlApplicationEngine* engine() const { return m_engine; }

    // 服务访问
    static MessageBus* messageBus();
    static PluginManager* pluginManager();
    static MapLibreEngine* mapEngine();

private:
    void registerQmlTypes();
    void registerQmlSingletons();
    void initializeMapParsers();
    void setupConnections();

    QGuiApplication* m_app = nullptr;
    QQmlApplicationEngine* m_engine = nullptr;
};

} // namespace YEFS

#endif // YEFS_APPLICATION_H
