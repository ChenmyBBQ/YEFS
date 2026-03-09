import QtQuick
import HuskarUI.Basic

Rectangle {
    id: root

    property bool toolbarVisible: false
    property int selectedShapeType: -1
    property real hostWidth: 0
    property real hostHeight: 0
    property string dragHandleText: ""
    property string pointerIconText: ""
    property var shapeOptions: []

    signal pointerRequested()
    signal shapeToggled(int shapeType)
    signal closeRequested()

    property real targetWidth: toolbarContent.width + 16

    width: root.toolbarVisible ? targetWidth : 0
    height: toolbarContent.height + 12
    radius: HusTheme.Primary.radiusPrimary
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.85)
    border.width: 1
    border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorBorder, 0.35)
    opacity: root.toolbarVisible ? 1 : 0
    visible: root.toolbarVisible || width > 0
    clip: true

    Behavior on width {
        NumberAnimation {
            duration: HusTheme.Primary.durationMid
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: HusTheme.Primary.durationMid
        }
    }

    Item {
        anchors.fill: parent
        clip: true

        Row {
            id: toolbarContent
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            Rectangle {
                width: 24
                height: 36
                color: "transparent"
                anchors.verticalCenter: parent.verticalCenter

                NerdIconText {
                    anchors.centerIn: parent
                    text: root.dragHandleText
                    font.pixelSize: 18
                    color: dragHandler.active ? HusTheme.Primary.colorPrimary : HusTheme.Primary.colorTextSecondary
                }

                DragHandler {
                    id: dragHandler
                    target: root
                    xAxis.minimum: 0
                    xAxis.maximum: root.hostWidth - root.width
                    yAxis.minimum: 0
                    yAxis.maximum: root.hostHeight - root.height
                }

                HoverHandler {
                    cursorShape: dragHandler.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                }
            }

            HusDivider {
                orientation: Qt.Vertical
                height: 20
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle {
                id: pointerToolBtn
                width: 36
                height: 36
                radius: 4
                color: root.selectedShapeType < 0
                    ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.15)
                    : (pointerHover.hovered ? HusThemeFunctions.alpha(HusTheme.Primary.colorTextBase, 0.08) : "transparent")
                border.color: root.selectedShapeType < 0 ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.6) : "transparent"
                border.width: root.selectedShapeType < 0 ? 1 : 0

                NerdIconText {
                    anchors.centerIn: parent
                    text: root.pointerIconText
                    font.pixelSize: 18
                    color: root.selectedShapeType < 0 ? HusTheme.Primary.colorPrimary : HusTheme.Primary.colorTextBase
                }

                HoverHandler { id: pointerHover }

                TapHandler {
                    onTapped: root.pointerRequested()
                }

                HusToolTip {
                    visible: pointerHover.hovered
                    text: qsTr('指针')
                    position: HusToolTip.Position_Top
                }
            }

            HusDivider {
                orientation: Qt.Vertical
                height: 20
                anchors.verticalCenter: parent.verticalCenter
            }

            Row {
                id: shapeQuickButtons
                spacing: 6

                Repeater {
                    model: root.shapeOptions

                    delegate: AirspaceShapeButton {
                        label: modelData.name
                        shapeType: modelData.type
                        iconSource: modelData.iconSource
                        selectedShapeType: root.selectedShapeType
                        onToggled: shapeType => root.shapeToggled(shapeType)
                    }
                }
            }

            MapPanelActionButton {
                width: 28
                height: 28
                iconSource: HusIcon.CloseOutlined
                iconSize: 14
                anchors.verticalCenter: parent.verticalCenter
                tooltipPosition: HusToolTip.Position_Top
                tooltipText: qsTr('关闭')
                onClicked: root.closeRequested()
            }
        }
    }
}