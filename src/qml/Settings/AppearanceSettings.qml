import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

import YEFSApp

/**
 * 外观设置分类组件
 */
Column {
    id: root
    width: parent ? parent.width : 400
    spacing: 10

    HusText {
        text: qsTr('外观设置')
        font.weight: Font.DemiBold
    }

    Rectangle {
        width: parent.width
        height: appearanceContent.height + 40
        radius: 6
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.6)
        border.color: HusTheme.Primary.colorFillPrimary

        Column {
            id: appearanceContent
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: 15

            Row {
                height: 36
                spacing: 20

                HusText {
                    width: 100
                    height: parent.height
                    text: qsTr('主题模式')
                    verticalAlignment: Text.AlignVCenter
                }

                HusRadioBlock {
                    id: themeRadio
                    anchors.verticalCenter: parent.verticalCenter
                    initCheckedIndex: HusTheme.darkMode === HusTheme.System ? 0 : 
                                      HusTheme.darkMode === HusTheme.Light ? 1 : 2
                    model: [
                        { label: qsTr('跟随系统'), value: HusTheme.System },
                        { label: qsTr('浅色'), value: HusTheme.Light },
                        { label: qsTr('深色'), value: HusTheme.Dark }
                    ]
                    Component.onCompleted: completed = true
                    onCurrentCheckedValueChanged: {
                        if (completed) {
                            HusTheme.darkMode = currentCheckedValue;
                            SettingsManager.setValue("appearance", "darkMode", currentCheckedValue);
                        }
                    }
                    property bool completed: false
                }
            }

            Row {
                height: 36
                spacing: 20

                HusText {
                    width: 100
                    height: parent.height
                    text: qsTr('动画效果')
                    verticalAlignment: Text.AlignVCenter
                }

                HusSwitch {
                    id: animSwitch
                    anchors.verticalCenter: parent.verticalCenter
                    checked: HusTheme.animationEnabled
                    Component.onCompleted: completed = true
                    onCheckedChanged: {
                        if (completed) {
                            HusTheme.animationEnabled = checked;
                            SettingsManager.setValue("appearance", "animationEnabled", checked);
                        }
                    }
                    property bool completed: false
                }
            }
        }
    }
}
