import QtQuick 2.15
import HuskarUI.Basic 1.0

import YEFSApp

Rectangle {
    id: root

    property var categoryDefs: []
    property var draftKeys: ({})
    property var draftSecrets: ({})
    property bool settingsVisible: false

    signal applyRequested()
    signal resetRequested()
    signal draftKeyChanged(string categoryKey, string value)
    signal draftSecretChanged(string categoryKey, string value)

    z: 100
    radius: 8
    clip: true
    visible: root.settingsVisible || settingsShift.x < root.width
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.68)

    MouseArea {
        anchors.fill: parent
        enabled: root.settingsVisible
    }

    Item {
        anchors.fill: parent
        transform: Translate {
            id: settingsShift
            x: root.settingsVisible ? 0 : root.width

            Behavior on x {
                NumberAnimation {
                    duration: 10
                    easing.type: Easing.InOutCubic
                }
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8

            Item {
                width: parent.width
                height: 34

                HusIconButton {
                    width: 24
                    height: 24
                    iconSource: HusIcon.LeftOutlined
                    type: HusButton.Type_Default
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    onClicked: root.applyRequested()
                }

                HusText {
                    text: qsTr('底图源设置')
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    anchors.centerIn: parent
                }
            }

            HusDivider {
                width: parent.width
            }

            Flickable {
                id: settingsFlick
                width: parent.width
                height: parent.height - 58
                clip: true
                contentWidth: width
                contentHeight: settingsColumn.height + 12
                boundsBehavior: Flickable.StopAtBounds

                Column {
                    id: settingsColumn
                    width: settingsFlick.width - 6
                    spacing: 10

                    Repeater {
                        model: root.categoryDefs

                        delegate: Rectangle {
                            required property var modelData

                            width: parent.width
                            height: modelData.supportsSecret ? 132 : (modelData.supportsKey ? 98 : 64)
                            radius: HusTheme.Primary.radiusPrimary
                            color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.55)
                            border.color: HusTheme.Primary.colorBorder

                            Column {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 6

                                Item {
                                    width: parent.width
                                    height: 24

                                    HusText {
                                        text: modelData.label
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                        anchors.left: parent.left
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Row {
                                        spacing: 6
                                        anchors.right: parent.right
                                        anchors.verticalCenter: parent.verticalCenter

                                        HusText {
                                            text: qsTr('显示')
                                            anchors.verticalCenter: parent.verticalCenter
                                        }

                                        HusSwitch {
                                            checked: MapSettings.isCategoryVisible(modelData.key)
                                            anchors.verticalCenter: parent.verticalCenter
                                            onToggled: MapSettings.setCategoryVisible(modelData.key, checked)
                                        }
                                    }
                                }

                                HusInput {
                                    visible: modelData.supportsKey
                                    width: parent.width
                                    text: root.draftKeys[modelData.key] || ''
                                    placeholderText: modelData.key === 'MapTiler'
                                        ? qsTr('输入 MapTiler Key')
                                        : (modelData.key === 'Bing'
                                            ? qsTr('输入 Bing Key')
                                            : qsTr('输入 高德 Key'))
                                    onTextChanged: root.draftKeyChanged(modelData.key, text)
                                }

                                HusInput {
                                    visible: modelData.supportsSecret
                                    width: parent.width
                                    text: root.draftSecrets[modelData.key] || ''
                                    echoMode: HusInput.Password
                                    placeholderText: qsTr('输入 高德安全密钥')
                                    onTextChanged: root.draftSecretChanged(modelData.key, text)
                                }
                            }
                        }
                    }

                    HusButton {
                        width: parent.width
                        text: qsTr('恢复默认')
                        type: HusButton.Type_Default
                        onClicked: root.resetRequested()
                    }
                }
            }
        }
    }
}