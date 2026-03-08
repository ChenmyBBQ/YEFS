import QtQuick
import HuskarUI.Basic

Rectangle {
    id: root

    signal closeRequested()

    width: 320
    radius: HusTheme.Primary.radiusPrimary
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
    border.color: HusTheme.Primary.colorBorder
    clip: true

    Loader {
        id: airspacePanelLoader
        anchors.fill: parent
        source: "qrc:/AirspaceManagerPlugin/qml/AirspaceListPanel.qml"
    }

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 8
        anchors.rightMargin: 8
        width: 28
        height: 28
        radius: 14
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgElevated, 0.88)
        border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorBorder, 0.6)
        z: 2

        HusIconButton {
            anchors.fill: parent
            iconSource: HusIcon.CloseOutlined
            iconSize: 14
            type: HusButton.Type_Text
            onClicked: root.closeRequested()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: HusTheme.Primary.colorBorder
        radius: root.radius
        visible: airspacePanelLoader.status !== Loader.Ready

        Column {
            anchors.centerIn: parent
            spacing: 8

            HusText {
                anchors.horizontalCenter: parent.horizontalCenter
                text: airspacePanelLoader.status === Loader.Error
                    ? qsTr('空域面板加载失败')
                    : qsTr('空域面板加载中...')
                font.pixelSize: 13
                color: HusTheme.Primary.colorTextSecondary
            }
        }
    }
}