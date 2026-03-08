import QtQuick
import HuskarUI.Basic

HusIconButton {
    id: root

    property string tooltipText: ''
    property int tooltipPosition: HusToolTip.Position_Left
    property bool tooltipArrow: true

    width: 32
    height: 32
    iconSize: 16
    type: HusButton.Type_Text

    HusToolTip {
        visible: root.hovered
        showArrow: root.tooltipArrow
        position: root.tooltipPosition
        text: root.tooltipText
    }
}