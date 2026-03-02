import QtQuick 2.15
import HuskarUI.Basic 1.0

import YEFSApp

Rectangle {
    id: root
    width: 340
    height: parent ? parent.height : 600
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)

    readonly property string defaultMapTilerKey: "pIYKyqRw5KwCNhksntqa"
    property var categoryDefs: [
        { "key": "MapTiler", "label": "MapTiler" },
        { "key": "Google", "label": "Google 地图" },
        { "key": "Bing", "label": "Bing 地图" },
        { "key": "高德", "label": "高德地图" }
    ]

    function providerIndicesByCategory(categoryKey) {
        var indices = []
        var list = MapSettings.providers
        for (var i = 0; i < list.length; ++i) {
            var provider = list[i]
            if (provider && provider.category === categoryKey) {
                indices.push(i)
            }
        }
        return indices
    }

    function canSwitchProvider(providerIndex) {
        var p = MapSettings.providers[providerIndex]
        if (!p) return false
        if (p.category === "Bing") {
            var k = MapSettings.apiKey
            if (!k || k.length < 8 || k === defaultMapTilerKey)
                return false
        }
        return true
    }

    HusMessage {
        id: toast
        anchors.fill: parent
        z: 999
    }

    Column {
        id: layoutRoot
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        Row {
            width: parent.width
            height: 34
            HusText {
                text: qsTr("在线底图切换")
                font.pixelSize: 16
                font.weight: Font.DemiBold
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        HusDivider { width: parent.width }

        Item {
            id: listArea
            width: parent.width
            height: Math.max(120, parent.height - 220)

            Flickable {
                id: mapFlickable
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: scrollTrack.left
                anchors.rightMargin: 8
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                contentWidth: width
                contentHeight: contentColumn.height

                WheelHandler {
                    acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                    onWheel: function(event) {
                        var dy = event.angleDelta.y
                        if (dy === 0 && event.pixelDelta.y !== 0)
                            dy = event.pixelDelta.y

                        mapFlickable.contentY = Math.max(0,
                            Math.min(mapFlickable.contentHeight - mapFlickable.height,
                                     mapFlickable.contentY - dy))
                        event.accepted = true
                    }
                }

                Column {
                    id: contentColumn
                    width: mapFlickable.width
                    spacing: 14

                    Repeater {
                        model: root.categoryDefs

                        delegate: Column {
                            required property var modelData
                            property var providerIndices: root.providerIndicesByCategory(modelData.key)

                            width: parent.width
                            spacing: 8
                            visible: providerIndices.length > 0

                            HusText {
                                text: modelData.label
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                            }

                            Flow {
                                id: cardFlow
                                width: parent.width
                                spacing: 10

                                Repeater {
                                    model: parent.parent.providerIndices

                                    delegate: Rectangle {
                                        required property var modelData
                                        property int providerIndex: modelData

                                        width: (cardFlow.width - 10) / 2
                                        height: 124
                                        radius: HusTheme.Primary.radiusPrimary
                                        clip: true

                                        color: providerIndex === MapSettings.currentProviderIndex
                                            ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.18)
                                            : HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.5)
                                        border.width: providerIndex === MapSettings.currentProviderIndex ? 2 : 1
                                        border.color: providerIndex === MapSettings.currentProviderIndex
                                            ? HusTheme.Primary.colorPrimary
                                            : HusTheme.Primary.colorBorder

                                        Image {
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.top: parent.top
                                            height: 94
                                            fillMode: Image.PreserveAspectCrop
                                            source: MapSettings.getThumbnailUrl(providerIndex)

                                            Rectangle {
                                                anchors.fill: parent
                                                color: "#22000000"
                                                visible: parent.status !== Image.Ready
                                                HusText {
                                                    anchors.centerIn: parent
                                                    text: parent.status === Image.Error ? qsTr("加载失败") : qsTr("加载中...")
                                                    color: HusTheme.Primary.colorTextSecondary
                                                    font.pixelSize: 10
                                                }
                                            }
                                        }

                                        Rectangle {
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.bottom: parent.bottom
                                            height: 30
                                            color: providerIndex === MapSettings.currentProviderIndex
                                                ? HusTheme.Primary.colorPrimary
                                                : HusTheme.Primary.colorBgContainer

                                            HusText {
                                                anchors.centerIn: parent
                                                text: MapSettings.getProviderName(providerIndex)
                                                color: HusTheme.Primary.colorText
                                                font.pixelSize: 12
                                                font.weight: providerIndex === MapSettings.currentProviderIndex ? Font.DemiBold : Font.Normal
                                            }
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                if (!root.canSwitchProvider(providerIndex)) {
                                                    toast.error(qsTr("Bing 地图需要有效 Key，请先在下方输入。"))
                                                    return
                                                }
                                                MapSettings.currentProviderIndex = providerIndex
                                                toast.success(qsTr("已切换到底图：") + MapSettings.getProviderName(providerIndex))
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: scrollTrack
                width: 6
                radius: 3
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                visible: mapFlickable.contentHeight > mapFlickable.height
                color: HusThemeFunctions.alpha(HusTheme.Primary.colorBorder, 0.35)

                Rectangle {
                    width: parent.width
                    radius: 3
                    height: Math.max(24, parent.height * mapFlickable.visibleArea.heightRatio)
                    y: (parent.height - height) * mapFlickable.visibleArea.yPosition
                    color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.82)
                }
            }
        }

        HusDivider { width: parent.width }

        Row {
            width: parent.width
            height: 34
            spacing: 8

            HusText {
                text: qsTr("地图 Key:")
                font.pixelSize: 12
                anchors.verticalCenter: parent.verticalCenter
            }

            HusInput {
                width: parent.width - 80
                text: MapSettings.apiKey
                placeholderText: qsTr("输入 MapTiler / Bing Key")
                onEditingFinished: {
                    MapSettings.apiKey = text
                    toast.success(qsTr("已保存地图 Key"))
                }
            }
        }

        HusButton {
            width: parent.width
            text: qsTr("还原默认设置")
            type: HusButton.Type_Default
            onClicked: {
                MapSettings.reset()
                toast.success(qsTr("已恢复默认底图"))
            }
        }
    }
}
