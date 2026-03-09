import QtQuick
import YEFSApp

Item {
    id: root

    property var mapView

    // 是否处于绘图选择模式（已选中图形类型但尚未开始绘制，或绘制进行中）
    readonly property bool inDrawingMode: MapPageStateController.selectedShapeType >= 0
                                       || MapPageStateController.drawingActive

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: function(eventPoint) {
            MapPageStateController.handleMapTap(eventPoint.position)
        }
    }

    HoverHandler {
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        cursorShape: root.inDrawingMode ? Qt.CrossCursor : Qt.ArrowCursor
        onPointChanged: MapPageStateController.handleMapHover(point.position)
        onHoveredChanged: MapPageStateController.handleMapHoverChanged(hovered)
    }

    PinchHandler {
        id: pinch
        target: null
        enabled: !root.inDrawingMode
        onScaleChanged: (delta) => {
            root.mapView.scale(delta, pinch.centroid.position)
        }
        onRotationChanged: (delta) => {
            root.mapView.bearing -= delta
        }
        grabPermissions: PointerHandler.TakeOverForbidden
    }

    // 绘图模式下禁用左键拖拽（防止与坐标点击冲突）
    DragHandler {
        target: null
        acceptedButtons: Qt.LeftButton
        enabled: !root.inDrawingMode
        onTranslationChanged: (delta) => root.mapView.pan(delta)
    }

    // 绘图模式下禁用右键旋转/俯仰
    DragHandler {
        target: null
        acceptedButtons: Qt.RightButton
        enabled: !root.inDrawingMode
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