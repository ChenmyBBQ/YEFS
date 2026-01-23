import QtQuick
import QtLocation
import QtPositioning
import MapLibre.Location
import HuskarUI.Basic

Rectangle {
    id: root
    color: HusTheme.Primary.colorBgBase
    anchors.margins: 1
    Plugin {
        id: mapPlugin
        name: "maplibre"

        PluginParameter {
            name: "maplibre.map.styles"
            value: "https://demotiles.maplibre.org/style.json"
        }
    }

    MapView {
        id: mapView
        anchors.fill: parent
        map.plugin: mapPlugin
        map.center: QtPositioning.coordinate(39.9042, 116.4074) // Beijing
        map.zoomLevel: 2

        Component.onCompleted: {
            console.log("Map loaded with plugin:", map.plugin.name)
        }
    }

    // Map Controls
    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 15
        spacing: 10

        HusIconButton {
            iconSource: HusIcon.PlusOutlined
            onClicked: mapView.map.zoomLevel = Math.min(mapView.map.zoomLevel + 1, 20)
            
            HusToolTip {
                visible: parent.hovered
                text: qsTr('放大')
            }
        }

        HusIconButton {
            iconSource: HusIcon.MinusOutlined
            onClicked: mapView.map.zoomLevel = Math.max(mapView.map.zoomLevel - 1, 0)
            
            HusToolTip {
                visible: parent.hovered
                text: qsTr('缩小')
            }
        }

        HusIconButton {
            iconSource: HusIcon.RedoOutlined
            onClicked: mapView.map.bearing = (mapView.map.bearing + 15) % 360
            
            HusToolTip {
                visible: parent.hovered
                text: qsTr('顺时针旋转')
            }
        }

        HusIconButton {
            iconSource: HusIcon.UndoOutlined
            onClicked: mapView.map.bearing = (mapView.map.bearing - 15 + 360) % 360
            
            HusToolTip {
                visible: parent.hovered
                text: qsTr('逆时针旋转')
            }
        }

        HusIconButton {
            iconSource: HusIcon.CompassOutlined
            onClicked: mapView.map.bearing = 0
            
            HusToolTip {
                visible: parent.hovered
                text: qsTr('重置方向')
            }
        }
    }

    // Info panel
    Column {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 10
        spacing: 4

        HusText {
            text: qsTr('缩放: ') + mapView.map.zoomLevel.toFixed(1)
            color: HusTheme.Primary.colorTextBase
            style: Text.Outline
            styleColor: HusTheme.Primary.colorBgBase
        }

        HusText {
            text: qsTr('方向: ') + mapView.map.bearing.toFixed(1) + '°'
            color: HusTheme.Primary.colorTextBase
            style: Text.Outline
            styleColor: HusTheme.Primary.colorBgBase
        }

        HusText {
            text: qsTr('提示: Shift+滚轮旋转, Ctrl+滚轮倾斜')
            color: HusTheme.Primary.colorTextBase
            style: Text.Outline
            styleColor: HusTheme.Primary.colorBgBase
            font.pixelSize: 10
        }
    }
}
