import QtQuick 2.15
import HuskarUI.Basic 1.0

Rectangle {
    id: root

    property int providerIndex: -1
    property bool selected: false
    property string providerName: ''
    property string thumbnailSource: ''

    signal clicked(int providerIndex)

    height: 124
    radius: HusTheme.Primary.radiusPrimary
    clip: true
    color: selected
        ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.22)
        : HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.88)
    border.width: selected ? 2 : 1
    border.color: selected
        ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.52)
        : HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.38)

    Image {
        id: thumbnail
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 94
        fillMode: Image.PreserveAspectCrop
        source: root.thumbnailSource

        Rectangle {
            anchors.fill: parent
            color: '#22000000'
            visible: thumbnail.status !== Image.Ready

            HusText {
                anchors.centerIn: parent
                text: thumbnail.status === Image.Error ? qsTr('加载失败') : qsTr('加载中...')
                color: HusTheme.Primary.colorTextSecondary
                font.pixelSize: 10
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 30
        color: root.selected
            ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.52)
            : HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.92)

        HusText {
            anchors.centerIn: parent
            text: root.providerName
            color: HusTheme.Primary.colorTextBase
            font.pixelSize: 12
            font.weight: root.selected ? Font.DemiBold : Font.Normal
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked(root.providerIndex)
    }
}