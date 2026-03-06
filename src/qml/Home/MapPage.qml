import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import MapLibre 4.0

import YEFSApp
import "../Components"

Rectangle {
    id: root

    // 与加载遮罩同色，保证 QML 场景图第一帧前窗口背景即为蓝色，消除启动时粉色闪烁
    color: "#b8d9f0"

    property real mouseLatitude: NaN
    property real mouseLongitude: NaN
    property string mouseLatLonText: "--"
    property string mouseUtmText: "--"
    property string mouseMgrsText: "--"
    property string lastAppliedStyle: ""
    property bool styleSwitching: true
    property bool fallbackTimeout: false
    property bool startupVisualReadyEmitted: false

    function notifyStartupVisualReady(reason) {
        if (startupVisualReadyEmitted)
            return

        startupVisualReadyEmitted = true
        MessageBus.send("map/startup-visual-ready", { "reason": reason })
    }

    // 获取配置的样式URL
    function getMapStyleUrl() {
        let url = MapSettings.styleUrl;
        if (!url || url.length === 0)
            url = SettingsManager.getValue("map", "styleUrl", "");
        return url || "https://demotiles.maplibre.org/style.json";
    }

    function isInlineStyleJson(styleText) {
        if (!styleText)
            return false
        let trimmed = styleText.trim()
        return trimmed.length > 0 && (trimmed[0] === '{' || trimmed[0] === '[')
    }

    function applyConfiguredStyle() {
        let styleValue = root.getMapStyleUrl()
        if (!styleValue || styleValue.length === 0)
            return

        // 必须先 stop 再赋值，否则 PropertyAnimation 下一帧会覆盖赋值，导致 opacity 出现异常
        overlayHideAnim.stop()
        firstFrameSettleTimer.stop()  // 取消上一次尚未触发的淡出延迟
        styleTransitionOverlay.opacity = 1.0  // 立即不透明，不走任何动画
        root.lastAppliedStyle = styleValue
        root.styleSwitching = true
        root.fallbackTimeout = false
        mapLoadTimeoutTimer.restart()
        mapView.style = styleValue
    }

    function clearMouseCoordinate() {
        mouseLatitude = NaN
        mouseLongitude = NaN
        mouseLatLonText = "--"
        mouseUtmText = "--"
        mouseMgrsText = "--"
    }

    function updateMouseCoordinate(pos) {
        let coord = mapView.coordinateForPixel(pos)
        if (!coord || coord.length !== 2)
            return

        let latitude = coord[0]
        let longitude = coord[1]
        if (!isFinite(latitude) || !isFinite(longitude))
            return

        mouseLatitude = latitude
        mouseLongitude = longitude
        mouseLatLonText = CoordinateConverter.formatLatLon(latitude, longitude, 6)
        mouseUtmText = CoordinateConverter.latLonToUtmText(latitude, longitude)
        mouseMgrsText = CoordinateConverter.latLonToMgrsText(latitude, longitude)
    }

    // 监听设置变化
    Connections {
        target: SettingsManager
        function onSettingsChanged(category, key) {
            if (category === "map" && key === "styleUrl") {
                // 立即遮住地图，防止暴露窗口
                overlayHideAnim.stop()
                styleTransitionOverlay.opacity = 1.0
                root.applyConfiguredStyle();
            }
        }
    }

    Timer {
        id: styleWatchTimer
        interval: 250
        repeat: true
        running: true
        onTriggered: {
            let currentStyle = root.getMapStyleUrl()
            if (currentStyle && currentStyle.length > 0 && currentStyle !== root.lastAppliedStyle) {
                // 立即遮住地图（停止淡出动画），防止轮询间隔内出现暴露窗口
                overlayHideAnim.stop()
                styleTransitionOverlay.opacity = 1.0
                root.applyConfiguredStyle()
            }
        }
    }

    Timer {
        id: styleApplyTimer
        interval: 200
        repeat: true
        running: false
        onTriggered: {
            if (mapView.map) {
                root.applyConfiguredStyle()
                stop()
            }
        }
    }

    Timer {
        id: mapLoadTimeoutTimer
        interval: 30000
        repeat: false
        onTriggered: {
            root.styleSwitching = false
            root.fallbackTimeout = true
            // 超时后也不强制隐藏遮罩，继续保持可见并更新文字提示
            // 等 firstFrameReady 到来，再由 overlayHideAnim 淡出
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
            overlayHideAnim.stop()
            overlayHideAnim.start()
        }
    }

    // 地图底色垫层：与加载遮罩同色，保证遮罩淡出过程中透出来的始终是蓝色
    // 而非 FBO 重清时可能出现的红色/其他颜色
    Rectangle {
        anchors.fill: parent
        color: "#b8d9f0"
        z: 0
    }

    // 地图视图
    MapLibre {
        id: mapView
        anchors.fill: parent
        focus: true
        
        // 启动时直接使用当前配置样式，避免先加载 demo 样式再立刻切换到底图配置，
        // 这会在 OpenGL 后端上额外触发一次纹理/样式切换，容易出现瞬时粉色帧。
        style: root.getMapStyleUrl()
        zoomLevel: SettingsManager.getValue("map", "defaultZoom", 2)
        coordinate: [39.9042, 116.4074]

        Component.onCompleted: {
            MapLibreEngine.setMapItem(mapView)
            root.applyConfiguredStyle()
        }

        // ----------- 精准撤遮罩：在 Qt SceneGraph 已持有有效纹理后触发 -----------
        // onFirstFrameReady 由 C++ 渲染线程确认纹理写入后发到主线程，时序上晚于
        // onMapFullyLoaded，从根本上消除"mapFullyLoaded → 250ms 延迟 → 纹理写入"
        // 之间遮罩透出未初始化像素导致的红屏闪烁问题
        onFirstFrameReady: {
            if (root.styleSwitching || root.fallbackTimeout) {
                root.styleSwitching = false
                root.fallbackTimeout = false
                mapLoadTimeoutTimer.stop()
                // 等待 100ms 让 GPU 渲染管线稳定后再开始淡出遮罩
                firstFrameSettleTimer.restart()
            }
        }
        // ---------- GeoJSON 图层桥接函数 ----------
        function addGeoJSONLayer(layerId, geoJson, style) {
            var map = mapView.map
            if (!map) return

            var geoJsonStr = JSON.stringify(geoJson)
            map.addSource(layerId + "-source", {
                "type": "geojson",
                "data": geoJsonStr
            })

            var geomType = _detectGeometryType(geoJson)
            if (geomType === "Polygon" || geomType === "MultiPolygon") {
                map.addLayer(layerId + "-fill", {
                    "type": "fill",
                    "source": layerId + "-source"
                })
                map.setPaintProperty(layerId + "-fill", "fill-color", style["fill-color"] || "#3388ff")
                map.setPaintProperty(layerId + "-fill", "fill-opacity", style["fill-opacity"] || 0.3)

                map.addLayer(layerId + "-line", {
                    "type": "line",
                    "source": layerId + "-source"
                })
                map.setPaintProperty(layerId + "-line", "line-color", style["line-color"] || "#3388ff")
                map.setPaintProperty(layerId + "-line", "line-width", style["line-width"] || 2)
                if (style["line-dasharray"])
                    map.setPaintProperty(layerId + "-line", "line-dasharray", style["line-dasharray"])
            } else {
                map.addLayer(layerId + "-line", {
                    "type": "line",
                    "source": layerId + "-source"
                })
                map.setPaintProperty(layerId + "-line", "line-color", style["line-color"] || "#3388ff")
                map.setPaintProperty(layerId + "-line", "line-width", style["line-width"] || 2)
                if (style["line-dasharray"])
                    map.setPaintProperty(layerId + "-line", "line-dasharray", style["line-dasharray"])
            }
        }

        function updateLayerData(layerId, geoJson) {
            var map = mapView.map
            if (!map) return
            map.updateSource(layerId + "-source", {
                "type": "geojson",
                "data": JSON.stringify(geoJson)
            })
        }

        function removeLayer(layerId) {
            var map = mapView.map
            if (!map) return
            try { map.removeLayer(layerId + "-fill") } catch(e) {}
            try { map.removeLayer(layerId + "-line") } catch(e) {}
            try { map.removeSource(layerId + "-source") } catch(e) {}
        }

        function setLayerVisibility(layerId, visible) {
            var map = mapView.map
            if (!map) return
            var vis = visible ? "visible" : "none"
            try { map.setLayoutProperty(layerId + "-fill", "visibility", vis) } catch(e) {}
            try { map.setLayoutProperty(layerId + "-line", "visibility", vis) } catch(e) {}
        }

        function _detectGeometryType(geoJson) {
            if (geoJson.type === "Feature") return geoJson.geometry ? geoJson.geometry.type : "Point"
            if (geoJson.type === "FeatureCollection" && geoJson.features && geoJson.features.length > 0)
                return geoJson.features[0].geometry ? geoJson.features[0].geometry.type : "Point"
            return geoJson.type || "Point"
        }

        // ---------- 地图点击坐标传�?----------
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: function(eventPoint) {
                // 从像素坐标精确转换为地理坐标
                var pixelPos = eventPoint.position
                var coord = mapView.coordinateForPixel(pixelPos)
                if (coord && coord.length === 2) {
                    MapLibreEngine.onMapClicked(coord[0], coord[1])
                }
            }
        }

        HoverHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onPointChanged: {
                root.updateMouseCoordinate(point.position)
            }
            onHoveredChanged: {
                if (!hovered)
                    root.clearMouseCoordinate()
            }
        }

        // 地图手势处理
        PinchHandler {
            id: pinch
            target: null
            onScaleChanged: (delta) => {
                mapView.scale(delta, pinch.centroid.position)
            }
            onRotationChanged: (delta) => {
                mapView.bearing -= delta
            }
            grabPermissions: PointerHandler.TakeOverForbidden
        }

        DragHandler {
            id: drag
            target: null
            acceptedButtons: Qt.LeftButton
            onTranslationChanged: (delta) => mapView.pan(delta)
        }

        // 右键拖拽控制旋转和倾斜
        DragHandler {
            id: rotateTiltHandler
            target: null
            acceptedButtons: Qt.RightButton
            onTranslationChanged: (delta) => {
                mapView.bearing -= delta.x * 0.2
                // 限制俯仰角在 0-85 度之�?
                var newPitch = mapView.pitch - delta.y * 0.2
                mapView.pitch = Math.max(0, Math.min(85, newPitch))
            }
        }

        WheelHandler {
            id: wheel
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: (event) => {
                if (event.modifiers & Qt.ControlModifier) {
                    // Ctrl + 滚轮调整俯仰�?
                    var newPitch = mapView.pitch - (event.angleDelta.y * 0.1)
                    mapView.pitch = Math.max(0, Math.min(85, newPitch))
                } else {
                    mapView.scale(Math.pow(2.0, event.angleDelta.y / 120), wheel.point.position)
                }
            }
        }
    }

    Rectangle {
        id: styleTransitionOverlay
        anchors.fill: mapView
        z: 1
        visible: opacity > 0.01
        // 浅蓝色背景，寓意大海——陆地（底图）正在加载中
        color: "#b8d9f0"
        opacity: 1.0  // 由 applyConfiguredStyle() 直接赋值 1.0，由下方动画归零

        onVisibleChanged: {
            if (!visible)
                root.notifyStartupVisualReady("overlay.hidden")
        }

        // 只在「隐藏」时执行淡出动画；出现时由 JS 直接设 opacity=1.0 保证瞬间不透明
        // 先快后慢：地图先从蓝色背景下"透出"，再慢慢完全清晰，过渡更自然
        PropertyAnimation {
            id: overlayHideAnim
            target: styleTransitionOverlay
            property: "opacity"
            to: 0.0
            duration: 500
            easing.type: Easing.OutCubic
        }

        // 居中加载指示器区域
        Column {
            anchors.centerIn: parent
            spacing: 12
            visible: root.styleSwitching || root.fallbackTimeout

            HusSpin {
                anchors.horizontalCenter: parent.horizontalCenter
                spinning: true
                sizeHint: "large"
                spinSize: 36
                indicatorItemCount: 8
            }

            HusText {
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.fallbackTimeout ? qsTr("加载超时，仍在尝试…") : qsTr("底图加载中…")
                color: HusTheme.Primary.colorTextSecondary
                font.pixelSize: 13
            }
        }
    }

    // 地图控制面板
    Rectangle {
        id: controlPanel
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 15
        z: 2
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
        z: 2
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
                id: layerPanelButton
                width: 32
                height: 32
                iconSource: HusIcon.AppstoreOutlined
                iconSize: 16
                type: HusButton.Type_Text
                checkable: true
                checked: layerPanel.visible
                onClicked: {
                    layerPanel.visible = checked
                    if (checked) {
                        onlineMapPanel.visible = false
                    }
                }

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    position: HusToolTip.Position_Left
                    text: qsTr('图层面板')
                }
            }

            HusDivider {
                width: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            HusIconButton {
                id: onlineMapPanelButton
                width: 32
                height: 32
                iconSource: HusIcon.CloudOutlined
                iconSize: 16
                type: HusButton.Type_Text
                checkable: true
                checked: onlineMapPanel.visible
                onClicked: {
                    onlineMapPanel.visible = checked
                    if (checked) {
                        layerPanel.visible = false
                    }
                }

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    position: HusToolTip.Position_Left
                    text: qsTr('在线地图')
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
        z: 45
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.78)

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 15
            anchors.rightMargin: 15
            spacing: 20

            HusText {
                text: qsTr('经纬度: ') + root.mouseLatLonText
                font.pixelSize: 12
                color: HusTheme.Primary.colorTextSecondary
            }

            HusDivider {
                Layout.preferredHeight: 14
                orientation: Qt.Vertical
            }

            HusText {
                text: qsTr('UTM: ') + root.mouseUtmText
                font.pixelSize: 12
                color: HusTheme.Primary.colorTextSecondary
            }

            HusDivider {
                Layout.preferredHeight: 14
                orientation: Qt.Vertical
            }

            HusText {
                text: qsTr('MGRS: ') + root.mouseMgrsText
                font.pixelSize: 12
                color: HusTheme.Primary.colorTextSecondary
            }
        }
    }

    // 图层管理面板
    Loader {
        id: layerPanel
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 15
        z: 2
        visible: false
        active: visible
        sourceComponent: Component {
            Rectangle {
                width: 300
                height: parent.height
                radius: HusTheme.Primary.radiusPrimary
                color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
                border.color: HusTheme.Primary.colorBorder
                
                Loader {
                    anchors.fill: parent
                    source: "../Components/LayerPanel.qml"
                }
            }
        }
    }

    // 在线地图面板
    Loader {
        id: onlineMapPanel
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 15
        z: 2
        visible: false
        active: visible
        sourceComponent: Component {
            Rectangle {
                width: 300
                height: parent.height
                radius: HusTheme.Primary.radiusPrimary
                color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
                border.color: HusTheme.Primary.colorBorder
                
                Loader {
                    anchors.fill: parent
                    source: "../Components/OnlineMapPanel.qml"
                }
            }
        }
    }

    // ===== 空域管理面板 =====
    property bool airspacePanelVisible: false
    property bool _drawingActive: false
    property bool _drawingRequiresFinish: false
    property bool _newAirspaceToolbarVisible: false

    // 监听 MessageBus 消息来控制空域面板和绘制状�?
    Connections {
        target: MessageBus
        function onMessage(topic, data) {
            console.log("[MapPage] MessageBus received:", topic)
            if (topic === "airspace-manager/new") {
                root.airspacePanelVisible = true
                root._newAirspaceToolbarVisible = true
                airspaceListPanel.visible = false
            } else if (topic === "airspace-manager/show") {
                console.log("[MapPage] Setting airspacePanelVisible = true")
                root.airspacePanelVisible = true
                root._newAirspaceToolbarVisible = false
                airspaceListPanel.visible = true
            } else if (topic === "airspace-manager/hide") {
                root.airspacePanelVisible = false
                root._newAirspaceToolbarVisible = false
                airspaceListPanel.visible = false
            } else if (topic === "airspace-manager/drawing-state") {
                root._drawingActive = data.active
                if (data.statusText)
                    drawingStatusText.text = data.statusText
                root._drawingRequiresFinish = data.requiresFinish || false
            }
        }
    }

    // 新建空域快捷工具栏（悬浮可拖拽）
    Rectangle {
        id: newAirspaceToolbar
        x: 16
        y: 15
        property real targetWidth: toolbarContent.width + 16
        width: root._newAirspaceToolbarVisible ? targetWidth : 0
        height: toolbarContent.height + 12
        radius: HusTheme.Primary.radiusPrimary
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.8)
        border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorBorder, 0.5)
        opacity: root._newAirspaceToolbarVisible ? 1 : 0
        visible: root._newAirspaceToolbarVisible || width > 0
        z: 55
        clip: true

        Behavior on width {
            NumberAnimation {
                duration: HusTheme.Primary.durationMid
                easing.type: Easing.InOutQuad
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: HusTheme.Primary.durationMid
            }
        }

        Item {
            anchors.fill: parent
            clip: true

            Row {
                id: toolbarContent
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            // 拖拽把手
            Rectangle {
                width: 24
                height: 36
                color: "transparent"
                anchors.verticalCenter: parent.verticalCenter
                
                NerdIconText {
                    anchors.centerIn: parent
                    text: AppIconManager.dragHandle
                    font.pixelSize: 18
                    color: dragHandler.active ? HusTheme.Primary.colorPrimary : HusTheme.Primary.colorTextSecondary
                }
                
                DragHandler {
                    id: dragHandler
                    target: newAirspaceToolbar
                    xAxis.minimum: 0
                    xAxis.maximum: root.width - newAirspaceToolbar.width
                    yAxis.minimum: 0
                    yAxis.maximum: root.height - newAirspaceToolbar.height
                }

                HoverHandler {
                    id: hoverHandler
                    cursorShape: dragHandler.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                }
            }

            HusDivider {
                orientation: Qt.Vertical
                height: 20
                anchors.verticalCenter: parent.verticalCenter
            }

            Row {
                id: shapeQuickButtons
                spacing: 6

                Repeater {
                    model: [
                        { name: qsTr('矩形'),   type: 0, iconSource: AppIconManager.shapeRectangle },
                        { name: qsTr('正方形'), type: 1, iconSource: AppIconManager.shapeSquare },
                        { name: qsTr('圆形'),     type: 2, iconSource: AppIconManager.shapeCircle },
                        { name: qsTr('多边形'), type: 3, iconSource: AppIconManager.shapePolygon },
                        { name: qsTr('边界线'), type: 4, iconSource: AppIconManager.shapeLine },
                        { name: qsTr('圆环'),   type: 5, iconSource: AppIconManager.shapeRing },
                        { name: qsTr('圆弧'),   type: 6, iconSource: AppIconManager.shapeArc },
                        { name: qsTr('扇形'),   type: 7, iconSource: AppIconManager.shapeSector },
                        { name: qsTr('扇环'),   type: 8, iconSource: AppIconManager.shapePieSegment }
                    ]

                    delegate: Rectangle {
                        required property var modelData
                        width: 36
                        height: 36
                        radius: 4
                        color: hoverHandlerBtn.hovered ? HusThemeFunctions.alpha(HusTheme.Primary.colorTextBase, 0.08) : "transparent"

                        AppIcon {
                            anchors.centerIn: parent
                            source: parent.modelData.iconSource
                            size: 18
                            color: HusTheme.Primary.colorTextBase
                        }

                        HoverHandler {
                            id: hoverHandlerBtn
                        }

                        TapHandler {
                            onTapped: {
                                // 通过 MessageBus 发送开始绘制命�?
                                MessageBus.send("airspace-manager/draw", { "shapeType": parent.modelData.type })
                            }
                        }

                        HusToolTip {
                            visible: hoverHandlerBtn.hovered
                            text: parent.modelData.name
                            position: HusToolTip.Position_Top
                        }
                    }
                }
            }

            HusIconButton {
                width: 28
                height: 28
                type: HusButton.Type_Text
                iconSource: HusIcon.CloseOutlined
                iconSize: 14
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    root._newAirspaceToolbarVisible = false
                    // 通知主程序重新启用该快捷栏或清除左侧高亮状态
                    MessageBus.send("airspace-manager/hide", {})
                }
            }
        }
        }
    }

    // 绘制状态提�?(底部中央)
    Rectangle {
        id: drawingHint
        anchors.bottom: parent.bottom
        anchors.bottomMargin: statusBar.height + 10
        anchors.horizontalCenter: parent.horizontalCenter
        height: 40
        width: drawingHintRow.width + 24
        radius: HusTheme.Primary.radiusPrimary
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
        border.color: HusTheme.Primary.colorPrimary
        visible: root._drawingActive
        z: 60

        Row {
            id: drawingHintRow
            anchors.centerIn: parent
            spacing: 12

            HusText {
                id: drawingStatusText
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr('请在地图上点击选取坐标点')
                font.pixelSize: 13
                color: HusTheme.Primary.colorPrimary
            }

            HusButton {
                text: qsTr('完成')
                type: HusButton.Type_Primary
                sizeHint: 'small'
                visible: root._drawingRequiresFinish
                onClicked: MessageBus.send("airspace-manager/finish", {})
            }

            HusButton {
                text: qsTr('取消')
                type: HusButton.Type_Outlined
                sizeHint: 'small'
                colorText: HusTheme.Primary.colorError
                colorBorder: HusTheme.Primary.colorError
                onClicked: {
                    MessageBus.send("airspace-manager/cancel", {})
                    root._drawingActive = false
                }
            }
        }
    }

    // 空域列表面板 (右侧浮层)
    Rectangle {
        id: airspaceListPanel
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 15
        width: 320
        radius: HusTheme.Primary.radiusPrimary
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
        border.color: HusTheme.Primary.colorBorder
        visible: false
        z: 50
        clip: true

        // 面板标题�?
        Rectangle {
            id: listPanelHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 8

                HusText {
                    text: qsTr('空域列表')
                    font.pixelSize: 14
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                HusIconButton {
                    width: 28; height: 28
                    iconSource: HusIcon.CloseOutlined
                    iconSize: 14
                    type: HusButton.Type_Text
                    onClicked: airspaceListPanel.visible = false
                }
            }

            HusDivider {
                anchors.bottom: parent.bottom
                width: parent.width
            }
        }

        // 空状态提�?
        HusText {
            anchors.centerIn: parent
            text: qsTr('暂无空域数据\n请先创建空域')
            font.pixelSize: 13
            color: HusTheme.Primary.colorTextSecondary
            horizontalAlignment: Text.AlignHCenter
        }
    }
}








