import QtQuick
import HuskarUI.Basic

Rectangle {
    id: root

    property real panelWidth: 44
    property real contentSpacing: 4
    default property alias contentData: contentColumn.data

    width: panelWidth
    height: contentColumn.height + 16
    radius: HusTheme.Primary.radiusPrimary
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.9)
    border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorBorder, 0.5)

    Column {
        id: contentColumn
        anchors.centerIn: parent
        spacing: root.contentSpacing
    }
}