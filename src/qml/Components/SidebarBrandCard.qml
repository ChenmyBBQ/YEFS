import QtQuick
import HuskarUI.Basic

Rectangle {
    id: root

    property int expandedWidth: 0
    property int compactMode: HusMenu.Mode_Relaxed

    width: visible ? expandedWidth : 0
    height: visible ? 80 : 0
    radius: HusTheme.Primary.radiusPrimary
    color: hovered ? HusThemeFunctions.alpha(HusTheme.Primary.colorTextBase, 0.06) : 'transparent'
    visible: compactMode === HusMenu.Mode_Relaxed
    clip: true

    readonly property bool hovered: cardHover.hovered

    Behavior on height { NumberAnimation { duration: HusTheme.Primary.durationFast } }
    Behavior on color { ColorAnimation { duration: HusTheme.Primary.durationFast } }

    Item {
        height: parent.height
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10

        HusImage {
            id: logoIcon
            width: 50
            height: 50
            anchors.verticalCenter: parent.verticalCenter
            source: 'qrc:/YEFSApp/resources/images/YEFS.svg'
        }

        Column {
            anchors.left: logoIcon.right
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            HusText {
                text: 'YEFS'
                font.weight: Font.DemiBold
                font.pixelSize: HusTheme.Primary.fontPrimarySize + 4
            }

            HusText {
                text: qsTr('GIS Platform')
                font.pixelSize: HusTheme.Primary.fontPrimarySize - 1
                color: HusTheme.Primary.colorTextSecondary
            }

            HusText {
                width: parent.width
                text: qsTr('无人机地面站系统')
                font.pixelSize: HusTheme.Primary.fontPrimarySize - 1
                color: HusTheme.Primary.colorTextSecondary
                wrapMode: HusText.WrapAnywhere
            }
        }
    }

    HoverHandler {
        id: cardHover
    }
}