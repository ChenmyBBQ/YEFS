import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import AirspaceManager 1.0
import YEFSApp

/**
 * 空域管理主页面 (插件 qmlEntry)
 * 包含空域列表、搜索筛选、创建/编辑/删除操作
 */
Rectangle {
    id: root
    color: HusTheme.Primary.colorBgContainer

    // 工具栏浮层 (绘制模式时显示在顶部)
    Loader {
        id: toolbarLoader
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        z: 100
        active: EditRuntime.sessionActive
        sourceComponent: AirspaceToolbar {}
    }
    // 绘制信息面板(绘制模式时显示在右侧)
    Loader {
        id: infoPanelLoader
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 80
        anchors.rightMargin: 16
        z: 99
        active: EditRuntime.sessionActive
        source: "components/AirspaceInfoPanel.qml"
    }
    // 编辑弹窗
    AirspaceEditDialog {
        id: editDialog
        parent: Overlay.overlay

        onSaved: function(airspaceId, created) {
            // 将空域渲染到地图
            let data = AirspaceModel.getAirspace(airspaceId)
            if (data.id) {
                let geoJson = JSON.parse(data.geoJson)
                let style = JSON.parse(data.styleJson || '{}')
                if (created) {
                    MapLibreEngine.addGeoJSONLayer("airspace-" + data.id, geoJson, style)
                } else {
                    MapLibreEngine.updateLayerStyle("airspace-" + data.id, style)
                }
            }
        }
    }

    // 绘制完成 → 弹出编辑框
    Connections {
        target: DrawCtrl
        function onDrawingCompleted(geoJson, shapeType) {
            // 绘制完成后不再弹出编辑框，直接由右侧属性编辑框完成保存或应用操作
            // 如果需要支持保存未命名的空域，可以在这里加默认逻辑，或在组件内部处理。
            // 这里我们只需要移除预览图层，因为最终图层在保存时由 AirspaceInfoPanel 调用。
            MapLibreEngine.removeLayer("airspace-preview")
        }
        function onDrawingCancelled() {
            // 移除预览图层
            MapLibreEngine.removeLayer("airspace-preview")
        }
        function onPreviewUpdated(geoJson) {
            // 更新预览图层
            if (Object.keys(geoJson).length > 0) {
                let style = {
                    "fill-color": "#1890ff",
                    "fill-opacity": 0.2,
                    "line-color": "#1890ff",
                    "line-width": 2,
                    "line-dasharray": [4, 4]
                }
                try {
                    MapLibreEngine.removeLayer("airspace-preview")
                } catch(e) {}
                MapLibreEngine.addGeoJSONLayer("airspace-preview", geoJson, style)
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // 顶部标题栏
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            HusText {
                text: qsTr('空域管理')
                font.pixelSize: 20
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            HusButton {
                text: qsTr('创建空域')
                type: HusButton.Type_Primary
                iconSource: HusIcon.PlusOutlined
                enabled: !EditRuntime.sessionActive
                onClicked: {
                    // 切换到地图页面并激活绘制工具栏
                    // 首先显示工具栏让用户选择形状
                    EditRuntime.beginAirspaceEditSession(0) // 默认矩形
                }
            }
        }

        // 搜索栏
        HusInput {
            id: searchInput
            Layout.fillWidth: true
            placeholderText: qsTr('搜索空域名称...')
            iconSource: HusIcon.SearchOutlined
        }

        HusDivider {}

        // 空域列表
        ListView {
            id: airspaceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8

            model: AirspaceModel

            delegate: Rectangle {
                width: airspaceList.width
                radius: HusTheme.Primary.radiusPrimary
                color: mouseArea.containsMouse
                       ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.05)
                       : HusTheme.Primary.colorBgContainer
                border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorBorder, 0.5)

                // 过滤搜索
                visible: searchInput.text.length === 0 ||
                         model.name.toLowerCase().indexOf(searchInput.text.toLowerCase()) >= 0
                height: visible ? delegateContent.height + 16 : 0

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                }

                ColumnLayout {
                    id: delegateContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 8
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        HusText {
                            text: model.name
                            font.pixelSize: 14
                            font.bold: true
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        Rectangle {
                            width: shapeLabel.width + 12
                            height: 20
                            radius: 10
                            color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.1)

                            HusText {
                                id: shapeLabel
                                anchors.centerIn: parent
                                text: model.shapeTypeName
                                font.pixelSize: 11
                                color: HusTheme.Primary.colorPrimary
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        HusText {
                            text: model.createdAt
                            font.pixelSize: 11
                            color: HusTheme.Primary.colorTextSecondary
                        }

                        Item { Layout.fillWidth: true }

                        // 操作按钮
                        HusIconButton {
                            width: 28; height: 28
                            iconSource: model.visible ? HusIcon.EyeOutlined : HusIcon.EyeInvisibleOutlined
                            iconSize: 14
                            type: HusButton.Type_Text
                            onClicked: {
                                AirspaceModel.toggleVisibility(model.airspaceId)
                                MapLibreEngine.setLayerVisibility("airspace-" + model.airspaceId, !model.visible)
                            }
                        }

                        HusIconButton {
                            width: 28; height: 28
                            iconSource: HusIcon.AimOutlined
                            iconSize: 14
                            type: HusButton.Type_Text
                            onClicked: {
                                // 定位到空域中心 (TODO: 计算 bbox 中心)
                            }
                            HusToolTip {
                                visible: parent.hovered
                                text: qsTr('定位')
                            }
                        }

                        HusIconButton {
                            width: 28; height: 28
                            iconSource: HusIcon.EditOutlined
                            iconSize: 14
                            type: HusButton.Type_Text
                            onClicked: editDialog.openForEdit(model.airspaceId)
                            HusToolTip {
                                visible: parent.hovered
                                text: qsTr('编辑')
                            }
                        }

                        HusIconButton {
                            width: 28; height: 28
                            iconSource: HusIcon.DeleteOutlined
                            iconSize: 14
                            type: HusButton.Type_Text
                            onClicked: {
                                AirspaceModel.removeAirspace(model.airspaceId)
                                MapLibreEngine.removeLayer("airspace-" + model.airspaceId)
                            }
                            HusToolTip {
                                visible: parent.hovered
                                text: qsTr('删除')
                            }
                        }
                    }
                }
            }
        }

        // 空状态
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16
            visible: AirspaceModel.count === 0

            Item { Layout.fillHeight: true }

            HusText {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr('暂无空域数据')
                font.pixelSize: 16
                color: HusTheme.Primary.colorTextSecondary
            }

            HusText {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr('点击「创建空域」开始绘制')
                font.pixelSize: 13
                color: HusTheme.Primary.colorTextTertiary
            }

            Item { Layout.fillHeight: true }
        }

        // 底部统计
        HusText {
            Layout.alignment: Qt.AlignRight
            text: qsTr('共 %1 个空域').arg(AirspaceModel.count)
            font.pixelSize: 12
            color: HusTheme.Primary.colorTextSecondary
            visible: AirspaceModel.count > 0
        }
    }
}
