import QtQuick
import MapLibre 4.0

import YEFSApp
import "../Components"

Rectangle {
    id: root

    // 与加载遮罩同色，保证 QML 场景图第一帧前窗口背景即为蓝色，消除启动时粉色闪烁
    color: "#b8d9f0"

    readonly property var pageStateController: MapPageStateController
    readonly property var pageStyleController: MapPageStyleController
    readonly property var settingsManager: SettingsManager
    readonly property var mapLibreEngine: MapLibreEngine
    readonly property var appIconManager: AppIconManager

    Timer {
        id: mapLoadTimeoutTimer
        interval: 30000
        repeat: false
        onTriggered: {
            root.pageStyleController.handleLoadTimeout()
            // 网络慢或样式端异常时，不再一直锁死蓝色遮罩
            styleTransitionOverlay.hideAnimation.stop()
            styleTransitionOverlay.hideAnimation.start()
        }
    }

    // 首帧稳定延迟：firstFrameReady 触发后等待 100ms 让 GPU 渲染管线稳定，
    // 再开始遮罩淡出。底层 C++ 已确保每次样式变化只发出一次 firstFrameReady，
    // 此处作为额外的 GPU flush 等待窗口。
    Timer {
        id: firstFrameSettleTimer
        interval: 100
        repeat: false
        onTriggered: {
            styleTransitionOverlay.hideAnimation.stop()
            styleTransitionOverlay.hideAnimation.start()
        }
    }

    // 地图底色垫层：与加载遮罩同色，保证遮罩淡出过程中透出来的始终是蓝色
    // 而非 FBO 重清时可能出现的红色/其他颜色
    Rectangle {
        anchors.fill: root
        color: "#b8d9f0"
        z: 0
    }

    // 地图视图
    MapLibre {
        id: mapView
        anchors.fill: root
        focus: true
        
        // 启动时直接使用当前配置样式，避免先加载 demo 样式再立刻切换到底图配置，
        // 这会在 OpenGL 后端上额外触发一次纹理/样式切换，容易出现瞬时粉色帧。
        style: root.pageStyleController.currentStyleUrl
        zoomLevel: root.settingsManager.getValue("map", "defaultZoom", 2)
        coordinate: [39.9042, 116.4074]

        Component.onCompleted: {
            root.mapLibreEngine.setMapItem(mapView)
            mapLoadTimeoutTimer.restart()
        }

        // ----------- 精准撤遮罩：在 Qt SceneGraph 已持有有效纹理后触发 -----------
        // onFirstFrameReady 由 C++ 渲染线程确认纹理写入后发到主线程，时序上晚于
        // onMapFullyLoaded，从根本上消除"mapFullyLoaded → 250ms 延迟 → 纹理写入"
        // 之间遮罩透出未初始化像素导致的红屏闪烁问题
        onFirstFrameReady: {
            if (root.pageStyleController.handleFirstFrameReady()) {
                mapLoadTimeoutTimer.stop()
                // 等待 100ms 让 GPU 渲染管线稳定后再开始淡出遮罩
                firstFrameSettleTimer.restart()
            }
        }

        onMapLoadFailed: {
            console.warn('[MapPage] map load failed, fallback to demotiles style')
            mapLoadTimeoutTimer.stop()
            firstFrameSettleTimer.stop()

            root.pageStyleController.handleLoadTimeout()

            if (root.pageStyleController.currentStyleUrl !== 'https://demotiles.maplibre.org/style.json') {
                mapView.style = 'https://demotiles.maplibre.org/style.json'
            }

            styleTransitionOverlay.hideAnimation.stop()
            styleTransitionOverlay.hideAnimation.start()
        }
    }

    Connections {
        target: root.pageStyleController
        function onCurrentStyleUrlChanged() {
            console.log('[MapPage] onCurrentStyleUrlChanged → mapView.style =',
                root.pageStyleController.currentStyleUrl.substring(0, 80))
            styleTransitionOverlay.hideAnimation.stop()
            firstFrameSettleTimer.stop()
            styleTransitionOverlay.opacity = 1.0
            mapLoadTimeoutTimer.restart()
            mapView.style = root.pageStyleController.currentStyleUrl
        }
    }

    MapInteractionLayer {
        anchors.fill: root
        z: 1
        mapView: mapView
        blockedItems: [newAirspaceToolbar, airspaceListPanel, airspaceInfoPanelLoader, drawingHint, statusBar]
    }

    MapStyleLoadingOverlay {
        id: styleTransitionOverlay
        anchors.fill: root
        z: 1
        loadingActive: root.pageStyleController.styleSwitching
        timeoutActive: root.pageStyleController.fallbackTimeout
        onHidden: root.pageStyleController.notifyOverlayHidden()
    }

    MapControlDock {
        anchors.fill: root
        z: 40
        mapView: mapView
    }

    NewAirspaceToolbar {
        id: newAirspaceToolbar
        x: 16
        y: 15
        z: 55
        toolbarVisible: root.pageStateController.newAirspaceToolbarVisible
        selectedShapeType: root.pageStateController.selectedShapeType
        hostWidth: root.width
        hostHeight: root.height
        dragHandleText: root.appIconManager.dragHandle
        pointerIconText: root.appIconManager.cursorPointer
        shapeOptions: root.pageStateController.shapeOptions
        onPointerRequested: root.pageStateController.clearShapeSelection()
        onShapeToggled: shapeType => root.pageStateController.toggleShapeSelection(shapeType)
        onCloseRequested: root.pageStateController.requestHideAirspacePanel()
    }

    MapCoordinateStatusBar {
        id: statusBar
        anchors.bottom: root.bottom
        anchors.left: root.left
        anchors.right: root.right
        z: 45
        latLonText: root.pageStateController.mouseLatLonText
        utmText: root.pageStateController.mouseUtmText
        mgrsText: root.pageStateController.mouseMgrsText
    }

    MapDrawingHint {
        id: drawingHint
        anchors.bottom: root.bottom
        anchors.bottomMargin: statusBar.height + 10
        anchors.horizontalCenter: root.horizontalCenter
        z: 60
        active: root.pageStateController.drawingActive
        requiresFinish: root.pageStateController.drawingRequiresFinish
        statusText: root.pageStateController.drawingStatusText
        onFinishRequested: root.pageStateController.finishDrawing()
        onCancelRequested: root.pageStateController.cancelDrawing()
    }

    AirspaceListPanel {
        id: airspaceListPanel
        anchors.top: root.top
        anchors.bottom: root.bottom
        anchors.right: root.right
        anchors.margins: 15
        visible: root.pageStateController.airspaceListVisible
        z: 50
        onCloseRequested: root.pageStateController.hideAirspaceList()
    }

    Loader {
        id: airspaceInfoPanelLoader
        anchors.top: root.top
        anchors.right: root.right
        anchors.topMargin: root.pageStateController.airspaceListVisible ? 15 : 60
        anchors.rightMargin: root.pageStateController.airspaceListVisible ? airspaceListPanel.width + 30 : 15
        z: 99
        active: root.pageStateController.airspacePanelVisible
        visible: root.pageStateController.airspacePanelVisible
        source: "qrc:/AirspaceManagerPlugin/qml/components/AirspaceInfoPanel.qml"

        onStatusChanged: {
            if (status === Loader.Error) {
                console.error('[MapPage] AirspaceInfoPanel load failed, source =', source)
            }
        }
    }
}








