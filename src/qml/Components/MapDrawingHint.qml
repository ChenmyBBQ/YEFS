import QtQuick
import HuskarUI.Basic

Rectangle {
    id: root

    property bool active: false
    property bool requiresFinish: false
    property string statusText: qsTr('请在地图上点击选取坐标点')

    signal finishRequested()
    signal cancelRequested()

    height: 40
    width: drawingHintRow.width + 24
    radius: HusTheme.Primary.radiusPrimary
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
    border.color: HusTheme.Primary.colorPrimary
    visible: active

    Row {
        id: drawingHintRow
        anchors.centerIn: parent
        spacing: 12

        HusText {
            anchors.verticalCenter: parent.verticalCenter
            text: root.statusText
            font.pixelSize: 13
            color: HusTheme.Primary.colorPrimary
        }

        HusButton {
            text: qsTr('完成')
            type: HusButton.Type_Primary
            sizeHint: 'small'
            visible: root.requiresFinish
            onClicked: root.finishRequested()
        }

        HusButton {
            text: qsTr('取消')
            type: HusButton.Type_Outlined
            sizeHint: 'small'
            colorText: HusTheme.Primary.colorError
            colorBorder: HusTheme.Primary.colorError
            onClicked: root.cancelRequested()
        }
    }
}