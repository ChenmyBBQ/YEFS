import QtQuick
import HuskarUI.Basic

Rectangle {
    width: parent.width
    height: infoContent.height + 30
    radius: 6
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorInfoBg, 0.5)
    border.color: HusTheme.Primary.colorInfoBorder

    Column {
        id: infoContent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Row {
            spacing: 8

            HusIconText {
                iconSource: HusIcon.InfoCircleOutlined
                colorIcon: HusTheme.Primary.colorInfo
                iconSize: 16
            }

            HusText {
                text: qsTr('单位设置说明')
                font.weight: Font.Medium
                color: HusTheme.Primary.colorInfo
            }
        }

        HusText {
            width: parent.width
            text: qsTr('• 航空标准：国际民航组织(ICAO)推荐，高度用ft、距离用NM、速度用kt\n')
                + qsTr('• 公制(SI)：国际单位制，适合无人机作业、测绘任务\n')
                + qsTr('• 英制：美国习惯单位，包含华氏度和英寸汞柱气压\n')
                + qsTr('• 内部计算统一使用SI单位，仅显示时进行换算')
            wrapMode: Text.WordWrap
            font.pixelSize: 12
            color: HusTheme.Primary.colorTextSecondary
            lineHeight: 1.4
        }
    }
}