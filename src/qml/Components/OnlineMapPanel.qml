import QtQuick 2.15
import HuskarUI.Basic 1.0

import YEFSApp

Rectangle {
    id: root
    width: 340
    height: parent ? parent.height : 600
    radius: 8
    clip: true
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.68)

    readonly property string defaultMapTilerKey: "pIYKyqRw5KwCNhksntqa"
    property var categoryDefs: [
        { "key": "MapTiler", "label": "MapTiler", "supportsKey": true, "supportsSecret": false },
        { "key": "Google", "label": "Google 地图", "supportsKey": false, "supportsSecret": false },
        { "key": "Bing", "label": "Bing 地图", "supportsKey": true, "supportsSecret": false },
        { "key": "高德", "label": "高德地图", "supportsKey": true, "supportsSecret": true }
    ]

    property var draftKeys: ({})
    property var draftSecrets: ({})
    property bool settingsVisible: false

    function providerIndicesByCategory(categoryKey) {
        var indices = []
        if (!MapSettings.isCategoryVisible(categoryKey)) {
            return indices
        }
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
            var bingKey = MapSettings.getSourceKey("Bing")
            if (!bingKey || bingKey.length < 8)
                return false
        } else if (p.category === "高德") {
            var gaodeKey = MapSettings.getSourceKey("高德")
            var gaodeSecret = MapSettings.getSourceSecret("高德")
            if (!gaodeKey || gaodeKey.length < 8 || !gaodeSecret || gaodeSecret.length < 8)
                return false
        }
        return true
    }

    function openSettings() {
        var data = {}
        var secretData = {}
        for (var i = 0; i < root.categoryDefs.length; ++i) {
            var c = root.categoryDefs[i]
            data[c.key] = MapSettings.getSourceKey(c.key)
            secretData[c.key] = MapSettings.getSourceSecret(c.key)
        }
        root.draftKeys = data
        root.draftSecrets = secretData
        root.settingsVisible = true
    }

    function closeSettingsAndApply() {
        for (var i = 0; i < root.categoryDefs.length; ++i) {
            var c = root.categoryDefs[i]
            if (c.supportsKey) {
                MapSettings.setSourceKey(c.key, root.draftKeys[c.key] || "")
            }
            if (c.supportsSecret) {
                MapSettings.setSourceSecret(c.key, root.draftSecrets[c.key] || "")
            }
        }
        root.settingsVisible = false
        toast.success(qsTr("设置已更新"))
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
        transform: Translate {
            id: mainShift
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
                text: qsTr("在线底图切换")
                font.pixelSize: 16
                font.weight: Font.DemiBold
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { width: parent.width - 120; height: 1 }

            HusIconButton {
                width: 30
                height: 30
                iconSource: HusIcon.SettingOutlined
                type: HusButton.Type_Default
                anchors.verticalCenter: parent.verticalCenter
                onClicked: root.openSettings()
            }
        }

        HusDivider {
            id: topDivider
            width: parent.width
        }

        Item {
            id: listArea
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
                    spacing: 12

                    Repeater {
                        model: root.categoryDefs

                        delegate: Rectangle {
                            required property var modelData
                            property var providerIndices: root.providerIndicesByCategory(modelData.key)

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
                                        text: modelData.label
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
                                        model: parent.parent.parent.providerIndices

                                        delegate: Rectangle {
                                            required property var modelData
                                            property int providerIndex: modelData

                                            width: (cardFlow.width - 10) / 2
                                            height: 124
                                            radius: HusTheme.Primary.radiusPrimary
                                            clip: true

                                            color: providerIndex === MapSettings.currentProviderIndex
                                                ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.22)
                                                : HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.88)
                                            border.width: providerIndex === MapSettings.currentProviderIndex ? 2 : 1
                                            border.color: providerIndex === MapSettings.currentProviderIndex
                                                ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.52)
                                                : HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.38)

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
                                                    ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.52)
                                                    : HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.92)

                                            HusText {
                                                anchors.centerIn: parent
                                                text: MapSettings.getProviderName(providerIndex)
                                                color: HusTheme.Primary.colorTextBase
                                                font.pixelSize: 12
                                                font.weight: providerIndex === MapSettings.currentProviderIndex ? Font.DemiBold : Font.Normal
                                            }
                                        }

                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    if (!root.canSwitchProvider(providerIndex)) {
                                                        var provider = MapSettings.providers[providerIndex]
                                                        var category = provider ? provider.category : ""
                                                        if (category === "Bing")
                                                            toast.error(qsTr("Bing 地图需要有效 Key，请先在设置页输入。"))
                                                        else if (category === "高德")
                                                            toast.error(qsTr("高德地图需要有效 Key 和安全密钥，请先在设置页输入。"))
                                                        else
                                                            toast.error(qsTr("当前底图不可用"))
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

    Rectangle {
        id: settingsOverlay
        anchors.fill: parent
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
            id: settingsPanel
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
                        onClicked: root.closeSettingsAndApply()
                    }

                    HusText {
                        text: qsTr("底图源设置")
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                        anchors.centerIn: parent
                    }
                }

                HusDivider { width: parent.width }

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
                                            text: qsTr("显示")
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
                                    text: root.draftKeys[modelData.key] || ""
                                    placeholderText: modelData.key === "MapTiler"
                                        ? qsTr("输入 MapTiler Key")
                                        : (modelData.key === "Bing"
                                            ? qsTr("输入 Bing Key")
                                            : qsTr("输入 高德 Key"))
                                    onTextChanged: {
                                        var data = root.draftKeys
                                        data[modelData.key] = text
                                        root.draftKeys = data
                                    }
                                }

                                HusInput {
                                    visible: modelData.supportsSecret
                                    width: parent.width
                                    text: root.draftSecrets[modelData.key] || ""
                                    echoMode: HusInput.Password
                                    placeholderText: qsTr("输入 高德安全密钥")
                                    onTextChanged: {
                                        var data = root.draftSecrets
                                        data[modelData.key] = text
                                        root.draftSecrets = data
                                    }
                                }
                            }
                        }
                    }

                        HusButton {
                            width: parent.width
                            text: qsTr("恢复默认")
                            type: HusButton.Type_Default
                            onClicked: {
                                MapSettings.reset()
                                root.openSettings()
                                toast.success(qsTr("已恢复默认设置"))
                            }
                        }
                    }
                }
            }
        }
    }
}
