import QtQuick
import YEFSApp

Item {
    id: root

    property var mapView
    property var blockedItems: []

    // 是否处于绘图选择模式（已选中图形类型但尚未开始绘制，或绘制进行中）
    readonly property bool inDrawingMode: MapPageStateController.selectedShapeType >= 0
                                       || MapPageStateController.drawingActive

    function isBlockedPosition(position) {
        if (!root.blockedItems || root.blockedItems.length === 0)
            return false

        for (let index = 0; index < root.blockedItems.length; ++index) {
            const item = root.blockedItems[index]
            if (!item || !item.visible || item.width <= 0 || item.height <= 0)
                continue

            const mappedTopLeft = item.mapToItem(root, 0, 0)
            const left = mappedTopLeft.x
            const top = mappedTopLeft.y
            const right = left + item.width
            const bottom = top + item.height
            if (position.x >= left && position.x <= right
                    && position.y >= top && position.y <= bottom) {
                return true
            }
        }

        return false
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onSingleTapped: function(eventPoint) {
            if (root.isBlockedPosition(eventPoint.position))
                return
            MapPageStateController.handleMapTap(eventPoint.position)
        }
        onDoubleTapped: function(eventPoint) {
            if (root.isBlockedPosition(eventPoint.position))
                return
            MapPageStateController.handleMapDoubleTap(eventPoint.position)
        }
    }

    HoverHandler {
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        cursorShape: root.inDrawingMode ? Qt.CrossCursor : Qt.ArrowCursor
        onPointChanged: {
            if (root.isBlockedPosition(point.position))
                return
            MapPageStateController.handleMapHover(point.position)
        }
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
            if (root.isBlockedPosition(wheel.point.position)) {
                event.accepted = true
                return
            }

            if (event.modifiers & Qt.ControlModifier) {
                var newPitch = root.mapView.pitch - (event.angleDelta.y * 0.1)
                root.mapView.pitch = Math.max(0, Math.min(85, newPitch))
            } else {
                root.mapView.scale(Math.pow(2.0, event.angleDelta.y / 120), wheel.point.position)
            }
        }
    }
}