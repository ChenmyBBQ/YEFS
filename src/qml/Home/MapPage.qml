import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import MapLibre 4.0

import YEFSApp

Rectangle {
    id: root
    color: HusTheme.isDark ? '#1a1a1a' : '#f0f0f0'

    // 获取配置的样式URL
    function getMapStyleUrl() {
        let url = SettingsManager.getValue("map", "styleUrl", "");
        return url || "https://demotiles.maplibre.org/style.json";
    }

    // 监听设置变化
    Connections {
        target: SettingsManager
        function onSettingsChanged(category, key) {
            if (category === "map" && key === "styleUrl") {
                console.log("[MapPage] Style URL changed:", root.getMapStyleUrl());
                mapView.style = root.getMapStyleUrl();
            }
        }
    }

    // 地图视图
    MapLibre {
        id: mapView
        anchors.fill: parent
        focus: true
        
        // 使用配置的样式URL
        style: root.getMapStyleUrl()
        zoomLevel: SettingsManager.getValue("map", "defaultZoom", 2)
        coordinate: [39.9042, 116.4074]

        // 地图手势处理
        PinchHandler {
            id: pinch
            target: null
            onScaleChanged: (delta) => {
                mapView.scale(delta, pinch.centroid.position)
            }
            grabPermissions: PointerHandler.TakeOverForbidden
        }

        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => mapView.pan(delta)
        }

        WheelHandler {
            id: wheel
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: (event) => {
                mapView.scale(Math.pow(2.0, event.angleDelta.y / 120), wheel.point.position)
            }
        }
    }

    // 地图控制面板
    Rectangle {
        id: controlPanel
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 15
        width: 44
        height: controlColumn.height + 16
        radius: HusTheme.Primary.radiusPrimary
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.9)
        border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorBorder, 0.5)

        Column {
            id: controlColumn
            anchors.centerIn: parent
            spacing: 4

            HusIconButton {
                width: 32
                height: 32
                iconSource: HusIcon.PlusOutlined
                iconSize: 16
                type: HusButton.Type_Text
                onClicked: mapView.zoomLevel = mapView.zoomLevel + 1

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    position: HusToolTip.Position_Left
                    text: qsTr('放大')
                }
            }

            HusDivider {
                width: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            HusIconButton {
                width: 32
                height: 32
                iconSource: HusIcon.MinusOutlined
                iconSize: 16
                type: HusButton.Type_Text
                onClicked: mapView.zoomLevel = mapView.zoomLevel - 1

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    position: HusToolTip.Position_Left
                    text: qsTr('缩小')
                }
            }

            HusDivider {
                width: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            HusIconButton {
                width: 32
                height: 32
                iconSource: HusIcon.CompassOutlined
                iconSize: 16
                type: HusButton.Type_Text
                onClicked: {
                    mapView.bearing = 0
                    mapView.pitch = 0
                }

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    position: HusToolTip.Position_Left
                    text: qsTr('重置视图')
                }
            }

            HusDivider {
                width: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            HusIconButton {
                width: 32
                height: 32
                iconSource: HusIcon.AimOutlined
                iconSize: 16
                type: HusButton.Type_Text
                onClicked: mapView.coordinate = [39.9042, 116.4074]

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    position: HusToolTip.Position_Left
                    text: qsTr('定位北京')
                }
            }
        }
    }

    // 地图样式选择
    Rectangle {
        id: stylePanel
        anchors.top: controlPanel.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.margins: 15
        width: 44
        height: styleColumn.height + 16
        radius: HusTheme.Primary.radiusPrimary
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.9)
        border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorBorder, 0.5)

        Column {
            id: styleColumn
            anchors.centerIn: parent
            spacing: 4

            HusIconButton {
                width: 32
                height: 32
                iconSource: HusIcon.GlobalOutlined
                iconSize: 16
                type: HusButton.Type_Text
                onClicked: mapView.style = "https://demotiles.maplibre.org/style.json"

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    position: HusToolTip.Position_Left
                    text: qsTr('标准样式')
                }
            }

            HusDivider {
                width: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            HusIconButton {
                width: 32
                height: 32
                iconSource: HusIcon.AppstoreOutlined
                iconSize: 16
                type: HusButton.Type_Text
                checkable: true
                onClicked: {
                    MessageBus.send("panel/layers", { visible: checked })
                }

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    position: HusToolTip.Position_Left
                    text: qsTr('图层面板')
                }
            }
        }
    }

    // 状态栏
    Rectangle {
        id: statusBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 28
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.9)

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 15
            anchors.rightMargin: 15
            spacing: 20

            HusText {
                text: qsTr('就绪')
                font.pixelSize: 12
                color: HusTheme.Primary.colorTextSecondary
            }

            Item { Layout.fillWidth: true }

            HusText {
                text: qsTr('缩放: ') + mapView.zoomLevel.toFixed(1)
                font.pixelSize: 12
                color: HusTheme.Primary.colorTextSecondary
            }

            HusDivider {
                Layout.preferredHeight: 14
                orientation: Qt.Vertical
            }

            HusText {
                text: mapView.coordinate[0].toFixed(4) + '°, ' + mapView.coordinate[1].toFixed(4) + '°'
                font.pixelSize: 12
                color: HusTheme.Primary.colorTextSecondary
            }
        }
    }
}
