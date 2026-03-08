import QtQuick 2.15
import HuskarUI.Basic 1.0

Item {
    id: root

    property var categoryDefs: []
    property bool settingsVisible: false
    property int currentProviderIndex: -1
    property var providerIndicesResolver: null

    signal openSettingsRequested()
    signal providerSwitchRequested(int providerIndex)

    Column {
        id: layoutRoot
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10
        transform: Translate {
            x: root.settingsVisible ? -root.width : 0

            Behavior on x {
                NumberAnimation {
                    duration: 10
                    easing.type: Easing.InOutCubic
                }
            }
        }

        Row {
            id: headerRow
            width: parent.width
            height: 34

            HusText {
                text: qsTr('在线底图切换')
                font.pixelSize: 16
                font.weight: Font.DemiBold
                anchors.verticalCenter: parent.verticalCenter
            }

            Item {
                width: parent.width - 120
                height: 1
            }

            HusIconButton {
                width: 30
                height: 30
                iconSource: HusIcon.SettingOutlined
                type: HusButton.Type_Default
                anchors.verticalCenter: parent.verticalCenter
                onClicked: root.openSettingsRequested()
            }
        }

        HusDivider {
            id: topDivider
            width: parent.width
        }

        Item {
            width: parent.width
            height: Math.max(120, parent.height - headerRow.height - topDivider.height - bottomDivider.height - layoutRoot.spacing * 3)

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
                        var deltaY = event.angleDelta.y
                        if (deltaY === 0 && event.pixelDelta.y !== 0)
                            deltaY = event.pixelDelta.y

                        mapFlickable.contentY = Math.max(
                            0,
                            Math.min(
                                mapFlickable.contentHeight - mapFlickable.height,
                                mapFlickable.contentY - deltaY
                            )
                        )
                        event.accepted = true
                    }
                }

                Column {
                    id: contentColumn
                    width: mapFlickable.width
                    spacing: 12

                    Repeater {
                        model: root.categoryDefs

                        delegate: Rectangle {
                            id: categoryDelegate
                            required property var modelData

                            property var providerIndices: root.providerIndicesResolver
                                ? root.providerIndicesResolver(modelData.key)
                                : []

                            width: parent.width
                            radius: HusTheme.Primary.radiusPrimary
                            color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.52)
                            border.width: 1
                            border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.42)
                            height: groupContent.implicitHeight + 18
                            visible: providerIndices.length > 0

                            Column {
                                id: groupContent
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.margins: 9
                                spacing: 8

                                Rectangle {
                                    width: parent.width
                                    height: 26
                                    radius: HusTheme.Primary.radiusPrimary
                                    color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.16)
                                    border.width: 1
                                    border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.48)

                                    HusText {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 8
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: categoryDelegate.modelData.label
                                        font.pixelSize: 14
                                        font.weight: Font.DemiBold
                                        color: HusTheme.Primary.colorTextBase
                                    }
                                }

                                Flow {
                                    id: cardFlow
                                    width: parent.width
                                    spacing: 10

                                    Repeater {
                                        model: categoryDelegate.providerIndices

                                        delegate: OnlineMapProviderCard {
                                            required property var modelData

                                            providerIndex: modelData
                                            width: (cardFlow.width - 10) / 2
                                            selected: providerIndex === root.currentProviderIndex
                                            providerName: MapSettings.getProviderName(providerIndex)
                                            thumbnailSource: MapSettings.getThumbnailUrl(providerIndex)
                                            onClicked: currentProviderIndex => root.providerSwitchRequested(currentProviderIndex)
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
                visible: (mapFlickable.contentHeight - mapFlickable.height) > 12
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

        HusDivider {
            id: bottomDivider
            width: parent.width
        }
    }
}