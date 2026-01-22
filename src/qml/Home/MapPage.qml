import QtQuick
import QtLocation
import QtPositioning
import MapLibre.Location
import HuskarUI.Basic

Rectangle {
    id: root
    color: HusTheme.Primary.colorBgBase

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
        map.zoomLevel: 10

        Component.onCompleted: {
            console.log("Map loaded with plugin:", map.plugin.name)
        }
    }

    // Zoom Controls
    Column {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 10

        HusIconButton {
            iconSource: HusIcon.PlusOutlined
            onClicked: mapView.map.zoomLevel = Math.min(mapView.map.zoomLevel + 1, 20)
            
            HusToolTip {
                visible: parent.hovered
                text: qsTr('Zoom In')
            }
        }

        HusIconButton {
            iconSource: HusIcon.MinusOutlined
            onClicked: mapView.map.zoomLevel = Math.max(mapView.map.zoomLevel - 1, 0)
            
            HusToolTip {
                visible: parent.hovered
                text: qsTr('Zoom Out')
            }
        }
    }

    HusText {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 10
        text: qsTr('Zoom: ') + mapView.map.zoomLevel.toFixed(1)
        color: HusTheme.Primary.colorTextBase
        style: Text.Outline
        styleColor: HusTheme.Primary.colorBgBase
    }
}
