import QtQuick
import QtQuick.Effects
import HuskarUI.Basic

/**
 * AppIcon - 统一图标渲染组件
 *
 * 自动识别图标类型：
 *   - source 以 "qrc:/" 开头 → SVG 模式：Image + MultiEffect 颜色化，随主题变色
 *   - 其他字符串            → 字体模式：NerdIconText（Symbols Nerd Font）
 *
 * 用法示例：
 *   // SVG 图标（空域形状，颜色随主题变化）
 *   AppIcon { source: AppIconManager.shapeRectangle; color: HusTheme.Primary.colorTextBase }
 *
 *   // 字体图标（Nerd Fonts）
 *   AppIcon { source: AppIconManager.dragHandle; color: HusTheme.Primary.colorTextSecondary }
 */
Item {
    id: root

    /// 图标来源：qrc:// 路径（SVG）或 Unicode 字符（字体图标）
    property string source: ""

    /// 图标颜色，默认绑定主题基础文字色——主题切换时自动变色
    property color color: HusTheme.Primary.colorTextBase

    /// 图标大小（宽高相等）
    property real size: 18

    width: size
    height: size

    // ── SVG 模式 ──────────────────────────────────────────────
    // source 以 "qrc:/" 开头时启用
    readonly property bool _isSvg: source ? String(source).startsWith("qrc:/") : false

    // SVG 源图（白色，不可见，作为 MultiEffect 的 source）
    Image {
        id: svgSource
        visible: false
        source: root._isSvg ? root.source : ""
        width: root.size
        height: root.size
        sourceSize.width: root.size * 2   // 高分辨率渲染
        sourceSize.height: root.size * 2
        smooth: true
        mipmap: true
    }

    // MultiEffect 将白色 SVG 着色为目标颜色，透明背景保持不变
    // Qt 6.5+ 内置 QtQuick.Effects，无需额外依赖
    MultiEffect {
        visible: root._isSvg
        source: svgSource
        width: root.size
        height: root.size
        // colorization = 1.0：完全着色，SVG 白色路径 → colorizationColor
        colorization: 1.0
        colorizationColor: root.color

        Behavior on colorizationColor {
            enabled: HusTheme.animationEnabled
            ColorAnimation { duration: HusTheme.Primary.durationMid }
        }
    }

    // ── 字体图标模式 ──────────────────────────────────────────
    // source 为 Unicode 字符时启用
    NerdIconText {
        visible: !root._isSvg
        anchors.centerIn: parent
        width: root.size
        height: root.size
        text: root._isSvg ? "" : root.source
        font.pixelSize: root.size
        color: root.color
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
