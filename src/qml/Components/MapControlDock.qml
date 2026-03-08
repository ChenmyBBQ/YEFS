import QtQuick
import HuskarUI.Basic

Item {
    id: root

    property var mapView
    property url layerPanelSource: "../Components/LayerPanel.qml"
    property url onlineMapPanelSource: "../Components/OnlineMapPanel.qml"

    FloatingToolPanel {
        id: controlPanel
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 15
        z: 2

        MapPanelActionButton {
            iconSource: HusIcon.PlusOutlined
            onClicked: root.mapView.zoomLevel = root.mapView.zoomLevel + 1
            tooltipText: qsTr('放大')
        }

        HusDivider {
            width: 24
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MapPanelActionButton {
            iconSource: HusIcon.MinusOutlined
            onClicked: root.mapView.zoomLevel = root.mapView.zoomLevel - 1
            tooltipText: qsTr('缩小')
        }

        HusDivider {
            width: 24
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MapPanelActionButton {
            iconSource: HusIcon.CompassOutlined
            onClicked: {
                root.mapView.bearing = 0
                root.mapView.pitch = 0
            }
            tooltipText: qsTr('重置视图')
        }

        HusDivider {
            width: 24
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MapPanelActionButton {
            iconSource: HusIcon.AimOutlined
            onClicked: root.mapView.coordinate = [39.9042, 116.4074]
            tooltipText: qsTr('定位北京')
        }
    }

    FloatingToolPanel {
        id: stylePanel
        anchors.top: controlPanel.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        anchors.margins: 15
        z: 2

        MapPanelActionButton {
            iconSource: HusIcon.GlobalOutlined
            onClicked: root.mapView.style = "https://demotiles.maplibre.org/style.json"
            tooltipText: qsTr('标准样式')
        }

        HusDivider {
            width: 24
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MapPanelActionButton {
            id: layerPanelButton
            iconSource: HusIcon.AppstoreOutlined
            checkable: true
            checked: layerPanel.visible
            onClicked: root.toggleExclusivePanel(layerPanel)
            tooltipText: qsTr('图层面板')
        }

        HusDivider {
            width: 24
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MapPanelActionButton {
            id: onlineMapPanelButton
            iconSource: HusIcon.CloudOutlined
            checkable: true
            checked: onlineMapPanel.visible
            onClicked: root.toggleExclusivePanel(onlineMapPanel)
            tooltipText: qsTr('在线地图')
        }
    }

    MapSideOverlay {
        id: layerPanel
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 15
        z: 2
        visible: false
        panelSource: root.layerPanelSource
    }

    MapSideOverlay {
        id: onlineMapPanel
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 15
        z: 2
        visible: false
        panelSource: root.onlineMapPanelSource
    }

    function toggleExclusivePanel(panelLoader) {
        const shouldShow = !panelLoader.visible
        layerPanel.visible = false
        onlineMapPanel.visible = false
        panelLoader.visible = shouldShow
    }
}