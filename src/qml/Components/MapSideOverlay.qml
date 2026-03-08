import QtQuick
import HuskarUI.Basic

Loader {
    id: root

    property string panelSource: ''
    property real panelWidth: 300

    active: visible
    sourceComponent: Component {
        Rectangle {
            width: root.panelWidth
            height: root.height
            radius: HusTheme.Primary.radiusPrimary
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
            border.color: HusTheme.Primary.colorBorder

            Loader {
                anchors.fill: parent
                source: root.panelSource
            }
        }
    }
}