import QtQuick
import HuskarUI.Basic

import YEFSApp

Rectangle {
    width: parent.width
    height: presetContent.height + 40
    radius: 6
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.6)
    border.color: HusTheme.Primary.colorFillPrimary

    Column {
        id: presetContent
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        Row {
            spacing: 10

            HusText {
                width: 80
                height: 32
                text: qsTr('快捷预设')
                font.weight: Font.Medium
                verticalAlignment: Text.AlignVCenter
            }

            HusTag {
                text: UnitManager.presetMode === 'aviation' ? qsTr('航空标准')
                    : UnitManager.presetMode === 'metric' ? qsTr('公制')
                    : UnitManager.presetMode === 'imperial' ? qsTr('英制')
                    : qsTr('自定义')
                presetColor: UnitManager.presetMode === 'custom'
                    ? HusTheme.Primary.colorWarning
                    : HusTheme.Primary.colorPrimary
            }
        }

        Row {
            spacing: 12

            HusButton {
                text: qsTr('航空标准')
                type: UnitManager.presetMode === 'aviation' ? HusButton.Type_Primary : HusButton.Type_Default
                onClicked: UnitManager.applyPreset('aviation')

                HusToolTip {
                    text: qsTr('ft / NM / kt / hPa / °C\n国际民航组织(ICAO)通用标准')
                }
            }

            HusButton {
                text: qsTr('公制 (SI)')
                type: UnitManager.presetMode === 'metric' ? HusButton.Type_Primary : HusButton.Type_Default
                onClicked: UnitManager.applyPreset('metric')

                HusToolTip {
                    text: qsTr('m / km / km·h⁻¹ / hPa / °C\n国际单位制，无人机/测绘常用')
                }
            }

            HusButton {
                text: qsTr('英制')
                type: UnitManager.presetMode === 'imperial' ? HusButton.Type_Primary : HusButton.Type_Default
                onClicked: UnitManager.applyPreset('imperial')

                HusToolTip {
                    text: qsTr('ft / mi / mph / inHg / °F\n美国习惯单位制')
                }
            }
        }
    }
}