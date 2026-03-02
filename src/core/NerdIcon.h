#ifndef YEFS_NERDICON_H
#define YEFS_NERDICON_H

#include <QObject>
#include <QQmlEngine>

namespace YEFS {

/**
 * @brief Nerd Fonts 图标字符映射 - QML 单例
 *
 * 通过 C++ QML_SINGLETON 注册，保证在 QML 中可以可靠地以
 * NerdIcon.shapeRectangle 等方式访问。
 *
 * 字体文件：SymbolsNerdFont-Regular.ttf（family: "Symbols Nerd Font"）
 */
class NerdIcon : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // ── 基本操作 ──────────────────────────────────────────
    Q_PROPERTY(QString close          READ close          CONSTANT)
    Q_PROPERTY(QString check          READ check          CONSTANT)
    Q_PROPERTY(QString plus           READ plus           CONSTANT)
    Q_PROPERTY(QString edit           READ edit           CONSTANT)
    Q_PROPERTY(QString trash          READ trash          CONSTANT)

    // ── GIS / 地图 ────────────────────────────────────────
    Q_PROPERTY(QString map            READ map            CONSTANT)
    Q_PROPERTY(QString mapMarker      READ mapMarker      CONSTANT)
    Q_PROPERTY(QString crosshairs     READ crosshairs     CONSTANT)
    Q_PROPERTY(QString airplane       READ airplane       CONSTANT)
    Q_PROPERTY(QString helicopter     READ helicopter     CONSTANT)
    Q_PROPERTY(QString polygon        READ polygon        CONSTANT)
    Q_PROPERTY(QString radar          READ radar          CONSTANT)
    Q_PROPERTY(QString satellite      READ satellite      CONSTANT)

    // ── 动作 / 状态 ───────────────────────────────────────
    Q_PROPERTY(QString dragHandle     READ dragHandle     CONSTANT)
    Q_PROPERTY(QString newAirspace    READ newAirspace    CONSTANT)

    // ── 空域形状 (Geometric Shapes) ───────────────────────
    Q_PROPERTY(QString shapeRectangle READ shapeRectangle CONSTANT)
    Q_PROPERTY(QString shapeSquare    READ shapeSquare    CONSTANT)
    Q_PROPERTY(QString shapeCircle    READ shapeCircle    CONSTANT)
    Q_PROPERTY(QString shapePolygon   READ shapePolygon   CONSTANT)
    Q_PROPERTY(QString shapeLine      READ shapeLine      CONSTANT)
    Q_PROPERTY(QString shapeRing      READ shapeRing      CONSTANT)
    Q_PROPERTY(QString shapeArc       READ shapeArc       CONSTANT)
    Q_PROPERTY(QString shapeSector    READ shapeSector    CONSTANT)
    Q_PROPERTY(QString shapePieSegment READ shapePieSegment CONSTANT)

public:
    static NerdIcon *instance();
    static NerdIcon *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static void destroy();

    // ── 基本操作 ──────────────────────────────────────────
    QString close()      const { return QStringLiteral("\uf00d"); }  // fa-times
    QString check()      const { return QStringLiteral("\uf00c"); }  // fa-check
    QString plus()       const { return QStringLiteral("\uf067"); }  // fa-plus
    QString edit()       const { return QStringLiteral("\uf044"); }  // fa-edit
    QString trash()      const { return QStringLiteral("\uf1f8"); }  // fa-trash

    // ── GIS / 地图 ────────────────────────────────────────
    QString map()        const { return QStringLiteral("\uf279"); }  // fa-map
    QString mapMarker()  const { return QStringLiteral("\uf041"); }  // fa-map-marker
    QString crosshairs() const { return QStringLiteral("\uf05b"); }  // fa-crosshairs
    QString airplane()   const { return QStringLiteral("\uf072"); }  // fa-plane
    QString helicopter() const { return QStringLiteral("\ue4be"); }  // mdi-helicopter
    QString polygon()    const { return QStringLiteral("\uf5df"); }  // fa-draw-polygon
    QString radar()      const { return QStringLiteral("\ue023"); }  // mdi-radar
    QString satellite()  const { return QStringLiteral("\uf7bf"); }  // fa-satellite

    // ── 动作 / 状态 ───────────────────────────────────────
    QString dragHandle()  const { return QStringLiteral("\uf0b2"); }  // fa-arrows-alt
    QString newAirspace() const { return QStringLiteral("\uf247"); }

    // ── 空域形状 ──────────────────────────────────────────
    QString shapeRectangle()  const { return QStringLiteral("\uf84f"); }  // md-rectangle
    QString shapeSquare()     const { return QStringLiteral("\uf0c8"); }  // fa-square
    QString shapeCircle()     const { return QStringLiteral("\uf111"); }  // fa-circle
    QString shapePolygon()    const { return QStringLiteral("\uf42b"); }  // fa-draw-polygon
    QString shapeLine()       const { return QStringLiteral("\uf746"); }  // mdi-vector-line
    QString shapeRing()       const { return QStringLiteral("\uf10c"); }  // fa-circle-o
    QString shapeArc()        const { return QStringLiteral("\uf745"); }  // arc
    QString shapeSector()     const { return QStringLiteral("\uf2aa"); }  // fa-pie-chart
    QString shapePieSegment() const { return QStringLiteral("\uf129"); }

private:
    explicit NerdIcon(QObject *parent = nullptr) : QObject(parent) {}

    static NerdIcon *s_instance;
};

} // namespace YEFS

#endif // YEFS_NERDICON_H
