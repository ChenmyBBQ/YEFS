import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

import YEFSApp

/**
 * 数据设置分类组件
 */
Column {
    id: root
    width: parent ? parent.width : 400
    spacing: 10

    HusText {
        text: qsTr('数据设置')
        font.weight: Font.DemiBold
    }

    Rectangle {
        width: parent.width
        height: dataContent.height + 40
        radius: 6
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.6)
        border.color: HusTheme.Primary.colorFillPrimary

        Column {
            id: dataContent
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
                    text: qsTr('缓存大小')
                    verticalAlignment: Text.AlignVCenter
                }

                HusSlider {
                    id: cacheSlider
                    width: 200
                    anchors.verticalCenter: parent.verticalCenter
                    min: 50
                    max: 500
                    stepSize: 50
                    value: 100
                    Component.onCompleted: {
                        value = SettingsManager.getValue("data", "cacheSize", 100);
                    }
                    onCurrentValueChanged: {
                        if (completed) {
                            SettingsManager.setValue("data", "cacheSize", currentValue);
                        }
                    }
                    property bool completed: false
                    Timer {
                        interval: 100
                        running: true
                        onTriggered: parent.completed = true
                    }
                }

                HusText {
                    width: 60
                    height: parent.height
                    text: cacheSlider.currentValue + " MB"
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Row {
                height: 36
                spacing: 20

                HusText {
                    width: 100
                    height: parent.height
                    text: qsTr('自动保存')
                    verticalAlignment: Text.AlignVCenter
                }

                HusSwitch {
                    id: autoSaveSwitch
                    anchors.verticalCenter: parent.verticalCenter
                    checked: true
                    Component.onCompleted: {
                        checked = SettingsManager.getValue("data", "autoSave", true);
                        completed = true;
                    }
                    onCheckedChanged: {
                        if (completed) {
                            SettingsManager.setValue("data", "autoSave", checked);
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
                    text: qsTr('配置路径')
                    verticalAlignment: Text.AlignVCenter
                }

                HusText {
                    height: parent.height
                    text: SettingsManager.settingsFilePath()
                    color: HusTheme.Primary.colorTextSecondary
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                }
            }
        }
    }
}
