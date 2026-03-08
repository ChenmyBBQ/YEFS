import QtQuick
import HuskarUI.Basic

Rectangle {
    id: root

    property bool loadingActive: false
    property bool timeoutActive: false
    property alias hideAnimation: overlayHideAnim

    signal hidden(string reason)

    visible: opacity > 0.01
    color: "#b8d9f0"
    opacity: 1.0

    onVisibleChanged: {
        if (!visible)
            root.hidden("overlay.hidden")
    }

    PropertyAnimation {
        id: overlayHideAnim
        target: root
        property: "opacity"
        to: 0.0
        duration: 500
        easing.type: Easing.OutCubic
    }

    Column {
        anchors.centerIn: parent
        spacing: 12
        visible: root.loadingActive || root.timeoutActive

        HusSpin {
            anchors.horizontalCenter: parent.horizontalCenter
            spinning: true
            sizeHint: "large"
            spinSize: 36
            indicatorItemCount: 8
        }

        HusText {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.timeoutActive ? qsTr("加载超时，仍在尝试…") : qsTr("底图加载中…")
            color: HusTheme.Primary.colorTextSecondary
            font.pixelSize: 13
        }
    }
}