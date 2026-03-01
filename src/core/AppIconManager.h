#ifndef YEFS_APPICONMANAGER_H
#define YEFS_APPICONMANAGER_H

#include <QObject>
#include <QQmlEngine>

namespace YEFS {

/**
 * @brief 应用图标资源管理器 - QML 单例
 *
 * 统一管理两类图标资源：
 *   1. 字体图标（Nerd Fonts Unicode 字符）：直接在 NerdIconText / Text 中作为 text 使用
 *   2. SVG 图标（qrc:// 路径）：配合 AppIcon.qml 使用，通过 MultiEffect 实现颜色随主题切换
 *
 * QML 使用示例：
 *   // 字体图标
 *   NerdIconText { text: AppIconManager.dragHandle; color: HusTheme.Primary.colorTextBase }
 *
 *   // SVG 图标（自动颜色化）
 *   AppIcon { source: AppIconManager.shapeRectangle; color: HusTheme.Primary.colorTextBase }
 *
 * 字体文件：SymbolsNerdFont-Regular.ttf（family: "Symbols Nerd Font"）
 * SVG 资源：qrc:/YEFSApp/resources/icons/shapes/（白色路径，透明背景）
 */
class AppIconManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // ── 基本操作（字体图标）────────────────────────────────────
    Q_PROPERTY(QString close          READ close          CONSTANT)
    Q_PROPERTY(QString check          READ check          CONSTANT)
    Q_PROPERTY(QString plus           READ plus           CONSTANT)
    Q_PROPERTY(QString edit           READ edit           CONSTANT)
    Q_PROPERTY(QString trash          READ trash          CONSTANT)

    // ── GIS / 地图（字体图标）──────────────────────────────────
    Q_PROPERTY(QString map            READ map            CONSTANT)
    Q_PROPERTY(QString mapMarker      READ mapMarker      CONSTANT)
    Q_PROPERTY(QString crosshairs     READ crosshairs     CONSTANT)
    Q_PROPERTY(QString airplane       READ airplane       CONSTANT)
    Q_PROPERTY(QString helicopter     READ helicopter     CONSTANT)
    Q_PROPERTY(QString polygon        READ polygon        CONSTANT)
    Q_PROPERTY(QString radar          READ radar          CONSTANT)
    Q_PROPERTY(QString satellite      READ satellite      CONSTANT)

    // ── 动作 / 状态（字体图标）────────────────────────────────
    Q_PROPERTY(QString dragHandle     READ dragHandle     CONSTANT)
    Q_PROPERTY(QString newAirspace    READ newAirspace    CONSTANT)

    // ── 空域形状（SVG 图标，配合 AppIcon.qml 使用）────────────
    Q_PROPERTY(QString shapeRectangle  READ shapeRectangle  CONSTANT)
    Q_PROPERTY(QString shapeSquare     READ shapeSquare     CONSTANT)
    Q_PROPERTY(QString shapeCircle     READ shapeCircle     CONSTANT)
    Q_PROPERTY(QString shapePolygon    READ shapePolygon    CONSTANT)
    Q_PROPERTY(QString shapeLine       READ shapeLine       CONSTANT)
    Q_PROPERTY(QString shapeRing       READ shapeRing       CONSTANT)
    Q_PROPERTY(QString shapeArc        READ shapeArc        CONSTANT)
    Q_PROPERTY(QString shapeSector     READ shapeSector     CONSTANT)
    Q_PROPERTY(QString shapePieSegment READ shapePieSegment CONSTANT)

public:
    static AppIconManager *instance();
    static AppIconManager *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    // ── 基本操作 ────────────────────────────────────────────
    QString close()      const { return QStringLiteral("\uf00d"); }  // fa-times
    QString check()      const { return QStringLiteral("\uf00c"); }  // fa-check
    QString plus()       const { return QStringLiteral("\uf067"); }  // fa-plus
    QString edit()       const { return QStringLiteral("\uf044"); }  // fa-edit
    QString trash()      const { return QStringLiteral("\uf1f8"); }  // fa-trash

    // ── GIS / 地图 ──────────────────────────────────────────
    QString map()        const { return QStringLiteral("\uf279"); }  // fa-map
    QString mapMarker()  const { return QStringLiteral("\uf041"); }  // fa-map-marker
    QString crosshairs() const { return QStringLiteral("\uf05b"); }  // fa-crosshairs
    QString airplane()   const { return QStringLiteral("\uf072"); }  // fa-plane
    QString helicopter() const { return QStringLiteral("\ue4be"); }  // mdi-helicopter
    QString polygon()    const { return QStringLiteral("\uf5df"); }  // fa-draw-polygon
    QString radar()      const { return QStringLiteral("\ue023"); }  // mdi-radar
    QString satellite()  const { return QStringLiteral("\uf7bf"); }  // fa-satellite

    // ── 动作 / 状态 ─────────────────────────────────────────
    QString dragHandle()  const { return QStringLiteral("\uf0b2"); }  // fa-arrows-alt
    QString newAirspace() const { return QStringLiteral("\uf247"); }

    // ── 空域形状（SVG qrc:// 路径）──────────────────────────
    QString shapeRectangle()  const { return QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_rectangle.svg"); }
    QString shapeSquare()     const { return QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_square.svg"); }
    QString shapeCircle()     const { return QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_circle.svg"); }
    QString shapePolygon()    const { return QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_polygon.svg"); }
    QString shapeLine()       const { return QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_line.svg"); }
    QString shapeRing()       const { return QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_ring.svg"); }
    QString shapeArc()        const { return QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_arc.svg"); }
    QString shapeSector()     const { return QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_sector.svg"); }
    QString shapePieSegment() const { return QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_pie_segment.svg"); }

private:
    explicit AppIconManager(QObject *parent = nullptr) : QObject(parent) {}

    static AppIconManager *s_instance;
};

} // namespace YEFS

#endif // YEFS_APPICONMANAGER_H
