import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

Rectangle {
    id: root

    property string latLonText: "--"
    property string utmText: "--"
    property string mgrsText: "--"

    height: 28
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.78)

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 15
        anchors.rightMargin: 15
        spacing: 20

        HusText {
            text: qsTr('经纬度: ') + root.latLonText
            font.pixelSize: 12
            color: HusTheme.Primary.colorTextSecondary
        }

        HusDivider {
            Layout.preferredHeight: 14
            orientation: Qt.Vertical
        }

        HusText {
            text: qsTr('UTM: ') + root.utmText
            font.pixelSize: 12
            color: HusTheme.Primary.colorTextSecondary
        }

        HusDivider {
            Layout.preferredHeight: 14
            orientation: Qt.Vertical
        }

        HusText {
            text: qsTr('MGRS: ') + root.mgrsText
            font.pixelSize: 12
            color: HusTheme.Primary.colorTextSecondary
        }
    }
}