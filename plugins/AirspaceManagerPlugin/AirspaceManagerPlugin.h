#ifndef AIRSPACEMANAGERPLUGIN_H
#define AIRSPACEMANAGERPLUGIN_H

#include "IPlugin.h"
#include <QQmlEngine>

#include "AirspaceDatabase.h"
#include "AirspaceModel.h"
#include "AirspaceSelectionController.h"
#include "ShapeGenerator.h"
#include "DrawingController.h"

/**
 * @brief 空域管理插件
 *
 * 提供空域创建、绘制（矩形/正方/圆/多边形/边界/圆环/圆弧/扇形/扇环）、
 * 编辑和持久化管理功能。
 */
class AirspaceManagerPlugin : public YEFS::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID YEFS_PLUGIN_IID FILE "plugin.json")
    Q_INTERFACES(YEFS::IPlugin)

public:
    explicit AirspaceManagerPlugin(QObject* parent = nullptr);
    ~AirspaceManagerPlugin() override;

    // IPlugin 接口
    bool initialize(YEFS::PluginContext* context) override;
    void shutdown() override;

    QString id()      const override { return QStringLiteral("airspace-manager"); }
    QString name()    const override { return QStringLiteral("空域管理"); }
    QString version() const override { return QStringLiteral("1.0.0"); }
    QString author()  const override { return QStringLiteral("YEFS Team"); }
    QString description() const override {
        return QStringLiteral("创建和管理空域区域（矩形、圆、多边形等）");
    }
    PluginType pluginType() const override { return ToolPlugin; }

    QUrl qmlEntry()     const override;
    QUrl settingsPage() const override;
    QUrl toolbarItem()  const override;
    QUrl sidePanel()    const override;

    // QML 可访问的子对象 getter
    Q_INVOKABLE QObject* drawingController() const { return m_drawCtrl; }
    Q_INVOKABLE QObject* airspaceModel()     const { return m_model; }
    Q_INVOKABLE QObject* airspaceSelection() const { return m_selectionCtrl; }
    Q_INVOKABLE QObject* shapeGenerator()    const { return m_shapeGen; }
    Q_INVOKABLE QObject* database()          const { return m_database; }

private:
    void registerQmlTypes();
    void loadAirspacesToMap();

    AirspaceDatabase*  m_database  = nullptr;
    AirspaceModel*     m_model     = nullptr;
    AirspaceSelectionController* m_selectionCtrl = nullptr;
    ShapeGenerator*    m_shapeGen  = nullptr;
    DrawingController* m_drawCtrl  = nullptr;
    bool               m_previewLayerExists = false;
};

#endif // AIRSPACEMANAGERPLUGIN_H
