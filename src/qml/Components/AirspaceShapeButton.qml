import QtQuick
import HuskarUI.Basic

Rectangle {
    id: root

    property string label: ''
    property int shapeType: -1
    property int selectedShapeType: -1
    property string iconSource: ''

    signal toggled(int shapeType)

    readonly property bool isSelected: selectedShapeType === shapeType

    width: 36
    height: 36
    radius: 4
    color: isSelected
        ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.15)
        : (hoverHandler.hovered ? HusThemeFunctions.alpha(HusTheme.Primary.colorTextBase, 0.08) : 'transparent')
    border.color: isSelected ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.6) : 'transparent'
    border.width: isSelected ? 1 : 0

    AppIcon {
        anchors.centerIn: parent
        source: root.iconSource
        size: 18
        color: root.isSelected ? HusTheme.Primary.colorPrimary : HusTheme.Primary.colorTextBase
    }

    HoverHandler {
        id: hoverHandler
    }

    TapHandler {
        onTapped: root.toggled(root.shapeType)
    }

    HusToolTip {
        visible: hoverHandler.hovered
        text: root.label
        position: HusToolTip.Position_Top
    }
}