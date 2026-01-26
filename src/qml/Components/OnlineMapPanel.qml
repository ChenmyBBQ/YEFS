import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic

import YEFSApp

/**
 * 在线地图管理面板
 */
Rectangle {
    id: root
    width: 300
    height: parent ? parent.height : 600
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
    border.color: HusTheme.Primary.colorBorder

    Column {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        // 标题栏
        Row {
            width: parent.width
            height: 36
            spacing: 10

            HusText {
                text: qsTr('在线地图')
                font.weight: Font.DemiBold
                font.pixelSize: 16
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { Layout.fillWidth: true; width: parent.width - 100 }
        }

        HusDivider {
            width: parent.width
        }

        // 在线地图提供商列表
        ScrollView {
            width: parent.width
            height: parent.height - 100
            clip: true

            Column {
                width: parent.width
                spacing: 12

                // OpenStreetMap
                Rectangle {
                    width: parent.width
                    height: onlineMapItem1.height + 24
                    radius: HusTheme.Primary.radiusPrimary
                    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.5)
                    border.color: HusTheme.Primary.colorFillPrimary

                    Column {
                        id: onlineMapItem1
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.topMargin: 12
                        spacing: 6

                        HusText {
                            text: qsTr('OpenStreetMap')
                            font.weight: Font.DemiBold
                            font.pixelSize: 14
                        }

                        HusText {
                            text: qsTr('开源社区驱动的世界地图')
                            font.pixelSize: 12
                            color: HusTheme.Primary.colorTextSecondary
                            width: root.width - 100
                            wrapMode: Text.WordWrap
                        }

                        HusButton {
                            text: qsTr('添加到图层')
                            type: HusButton.Type_Primary
                            size: HusButton.Size_Small
                            onClicked: {
                                addOnlineMap(0) // OpenStreetMap
                            }
                        }
                    }
                }

                // OpenFreeMap
                Rectangle {
                    width: parent.width
                    height: onlineMapItem2.height + 24
                    radius: HusTheme.Primary.radiusPrimary
                    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.5)
                    border.color: HusTheme.Primary.colorFillPrimary

                    Column {
                        id: onlineMapItem2
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.topMargin: 12
                        spacing: 6

                        HusText {
                            text: qsTr('OpenFreeMap')
                            font.weight: Font.DemiBold
                            font.pixelSize: 14
                        }

                        HusText {
                            text: qsTr('免费的 OpenStreetMap 托管服务')
                            font.pixelSize: 12
                            color: HusTheme.Primary.colorTextSecondary
                            width: root.width - 100
                            wrapMode: Text.WordWrap
                        }

                        HusButton {
                            text: qsTr('添加到图层')
                            type: HusButton.Type_Primary
                            size: HusButton.Size_Small
                            onClicked: {
                                addOnlineMap(1) // OpenFreeMap
                            }
                        }
                    }
                }

                // ESRI World Imagery
                Rectangle {
                    width: parent.width
                    height: onlineMapItem3.height + 24
                    radius: HusTheme.Primary.radiusPrimary
                    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.5)
                    border.color: HusTheme.Primary.colorFillPrimary

                    Column {
                        id: onlineMapItem3
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.topMargin: 12
                        spacing: 6

                        HusText {
                            text: qsTr('ESRI 卫星影像')
                            font.weight: Font.DemiBold
                            font.pixelSize: 14
                        }

                        HusText {
                            text: qsTr('Esri 提供的全球卫星影像')
                            font.pixelSize: 12
                            color: HusTheme.Primary.colorTextSecondary
                            width: root.width - 100
                            wrapMode: Text.WordWrap
                        }

                        HusButton {
                            text: qsTr('添加到图层')
                            type: HusButton.Type_Primary
                            size: HusButton.Size_Small
                            onClicked: {
                                addOnlineMap(4) // EsriWorldImagery
                            }
                        }
                    }
                }

                // CartoDB
                Rectangle {
                    width: parent.width
                    height: onlineMapItem4.height + 24
                    radius: HusTheme.Primary.radiusPrimary
                    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.5)
                    border.color: HusTheme.Primary.colorFillPrimary

                    Column {
                        id: onlineMapItem4
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.topMargin: 12
                        spacing: 6

                        HusText {
                            text: qsTr('CartoDB Positron')
                            font.weight: Font.DemiBold
                            font.pixelSize: 14
                        }

                        HusText {
                            text: qsTr('简洁清爽的地图样式')
                            font.pixelSize: 12
                            color: HusTheme.Primary.colorTextSecondary
                            width: root.width - 100
                            wrapMode: Text.WordWrap
                        }

                        HusButton {
                            text: qsTr('添加到图层')
                            type: HusButton.Type_Primary
                            size: HusButton.Size_Small
                            onClicked: {
                                addOnlineMap(5) // CartoDB
                            }
                        }
                    }
                }

                // MapTiler（需要 API Key）
                Rectangle {
                    width: parent.width
                    height: onlineMapItem5.height + 24
                    radius: HusTheme.Primary.radiusPrimary
                    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.5)
                    border.color: HusTheme.Primary.colorFillPrimary

                    Column {
                        id: onlineMapItem5
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.topMargin: 12
                        spacing: 6

                        HusText {
                            text: qsTr('MapTiler (需要 API Key)')
                            font.weight: Font.DemiBold
                            font.pixelSize: 14
                        }

                        HusText {
                            text: qsTr('高质量全球地图服务')
                            font.pixelSize: 12
                            color: HusTheme.Primary.colorTextSecondary
                            width: root.width - 100
                            wrapMode: Text.WordWrap
                        }

                        HusInput {
                            id: mapTilerKeyInput
                            width: parent.width
                            placeholderText: qsTr('请输入 MapTiler API Key')
                            text: SettingsManager.getValue("map", "maptilerKey", "")
                            onTextChanged: {
                                SettingsManager.setValue("map", "maptilerKey", text)
                            }
                        }

                        HusButton {
                            text: qsTr('添加到图层')
                            type: HusButton.Type_Primary
                            size: HusButton.Size_Small
                            enabled: mapTilerKeyInput.text.length > 0
                            onClicked: {
                                addOnlineMap(2, mapTilerKeyInput.text) // MapTiler
                            }
                        }
                    }
                }
            }
        }

        HusDivider {
            width: parent.width
        }

        // 底部提示
        HusText {
            text: qsTr('在线地图将作为底图图层添加')
            font.pixelSize: 12
            color: HusTheme.Primary.colorTextSecondary
        }
    }

    // 添加在线地图
    function addOnlineMap(providerType, apiKey) {
        apiKey = apiKey || ""
        let provider = OnlineMapProviderManager.createProvider(providerType, apiKey)
        if (provider) {
            MapSourceManager.addSource(provider)
            HusApp.showSuccess(qsTr('已添加在线地图: ') + provider.name)
        } else {
            HusApp.showError(qsTr('添加失败'))
        }
    }
}
