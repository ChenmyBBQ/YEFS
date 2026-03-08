import QtQuick
import YEFSApp

Item {
    id: root

    property var mapView

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: function(eventPoint) {
            MapPageStateController.handleMapTap(eventPoint.position)
        }
    }

    HoverHandler {
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        cursorShape: (MapPageStateController.selectedShapeType >= 0 || MapPageStateController.drawingActive)
            ? Qt.CrossCursor
            : Qt.ArrowCursor
        onPointChanged: MapPageStateController.handleMapHover(point.position)
        onHoveredChanged: MapPageStateController.handleMapHoverChanged(hovered)
    }

    PinchHandler {
        id: pinch
        target: null
        onScaleChanged: (delta) => {
            root.mapView.scale(delta, pinch.centroid.position)
        }
        onRotationChanged: (delta) => {
            root.mapView.bearing -= delta
        }
        grabPermissions: PointerHandler.TakeOverForbidden
    }

    DragHandler {
        target: null
        acceptedButtons: Qt.LeftButton
        onTranslationChanged: (delta) => root.mapView.pan(delta)
    }

    DragHandler {
        target: null
        acceptedButtons: Qt.RightButton
        onTranslationChanged: (delta) => {
            root.mapView.bearing -= delta.x * 0.2
            var newPitch = root.mapView.pitch - delta.y * 0.2
            root.mapView.pitch = Math.max(0, Math.min(85, newPitch))
        }
    }

    WheelHandler {
        id: wheel
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        onWheel: (event) => {
            if (event.modifiers & Qt.ControlModifier) {
                var newPitch = root.mapView.pitch - (event.angleDelta.y * 0.1)
                root.mapView.pitch = Math.max(0, Math.min(85, newPitch))
            } else {
                root.mapView.scale(Math.pow(2.0, event.angleDelta.y / 120), wheel.point.position)
            }
        }
    }
}