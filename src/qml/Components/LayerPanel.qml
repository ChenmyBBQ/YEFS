import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic
import QtQuick.Dialogs

import YEFSApp

/**
 * 图层管理面板
 */
Rectangle {
    id: root
    width: 300
    height: parent ? parent.height : 600
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
    border.color: HusTheme.Primary.colorBorder

    Column {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        // 标题栏
        Row {
            width: parent.width
            height: 36
            spacing: 10

            HusText {
                text: qsTr('图层管理')
                font.weight: Font.DemiBold
                font.pixelSize: 16
                anchors.verticalCenter: parent.verticalCenter
            }

            Item { Layout.fillWidth: true; width: parent.width - 150 }

            HusIconButton {
                width: 32
                height: 32
                iconSource: HusIcon.PlusOutlined
                type: HusButton.Type_Primary
                anchors.verticalCenter: parent.verticalCenter
                onClicked: fileDialog.open()

                HusToolTip {
                    visible: parent.hovered
                    text: qsTr('导入地图文件')
                }
            }

            HusIconButton {
                width: 32
                height: 32
                iconSource: HusIcon.DeleteOutlined
                type: HusButton.Type_Text
                danger: true
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    MapSourceManager.removeAllSources()
                }

                HusToolTip {
                    visible: parent.hovered
                    text: qsTr('清空所有图层')
                }
            }
        }

        HusDivider {
            width: parent.width
        }

        // 图层列表
        ScrollView {
            width: parent.width
            height: parent.height - 100
            clip: true

            Column {
                width: parent.width
                spacing: 8

                Repeater {
                    model: MapSourceManager.sources

                    delegate: Rectangle {
                        width: parent.width
                        height: 60
                        radius: HusTheme.Primary.radiusPrimary
                        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.5)
                        border.color: HusTheme.Primary.colorFillPrimary

                        Column {
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4

                            HusText {
                                text: modelData.name
                                font.weight: Font.DemiBold
                                font.pixelSize: 14
                            }

                            HusText {
                                text: getSourceTypeText(modelData.type)
                                font.pixelSize: 12
                                color: HusTheme.Primary.colorTextSecondary
                            }
                        }

                        Row {
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4

                            HusIconButton {
                                width: 28
                                height: 28
                                iconSource: HusIcon.EyeOutlined
                                iconSize: 14
                                type: HusButton.Type_Text
                                onClicked: {
                                    // TODO: 切换图层可见性
                                }
                            }

                            HusIconButton {
                                width: 28
                                height: 28
                                iconSource: HusIcon.AimOutlined
                                iconSize: 14
                                type: HusButton.Type_Text
                                onClicked: {
                                    // TODO: 定位到图层
                                    let bounds = modelData.bounds
                                    if (bounds.isValid) {
                                        MessageBus.send("map/flyTo", {
                                            latitude: bounds.center.latitude,
                                            longitude: bounds.center.longitude,
                                            zoom: 10
                                        })
                                    }
                                }
                            }

                            HusIconButton {
                                width: 28
                                height: 28
                                iconSource: HusIcon.DeleteOutlined
                                iconSize: 14
                                type: HusButton.Type_Text
                                danger: true
                                onClicked: {
                                    MapSourceManager.removeSource(modelData.id)
                                }
                            }
                        }
                    }
                }

                // 空状态
                Item {
                    width: parent.width
                    height: 200
                    visible: MapSourceManager.sourceCount === 0

                    Column {
                        anchors.centerIn: parent
                        spacing: 12

                        HusIcon {
                            id: emptyIcon
                            type: HusIcon.InboxOutlined
                            size: 48
                            color: HusTheme.Primary.colorTextTertiary
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        HusText {
                            text: qsTr('暂无图层')
                            color: HusTheme.Primary.colorTextTertiary
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        HusButton {
                            text: qsTr('导入地图文件')
                            type: HusButton.Type_Primary
                            anchors.horizontalCenter: parent.horizontalCenter
                            onClicked: fileDialog.open()
                        }
                    }
                }
            }
        }

        HusDivider {
            width: parent.width
        }

        // 底部统计
        HusText {
            text: qsTr('共 %1 个图层').arg(MapSourceManager.sourceCount)
            font.pixelSize: 12
            color: HusTheme.Primary.colorTextSecondary
        }
    }

    // 文件选择对话框
    FileDialog {
        id: fileDialog
        title: qsTr('选择地图文件')
        nameFilters: [
            qsTr('所有支持的格式 (*.geojson *.json *.gpx *.kml)'),
            qsTr('GeoJSON 文件 (*.geojson *.json)'),
            qsTr('GPX 轨迹 (*.gpx)'),
            qsTr('KML 文件 (*.kml)')
        ]
        fileMode: FileDialog.OpenFiles
        onAccepted: {
            let files = []
            for (let i = 0; i < selectedFiles.length; i++) {
                let urlStr = selectedFiles[i].toString()
                // 移除 file:// 前缀
                if (urlStr.startsWith('file://')) {
                    urlStr = urlStr.substring(7)
                }
                files.push(urlStr)
            }
            
            if (files.length > 0) {
                let success = MapSourceManager.loadFiles(files)
                if (success) {
                    HusApp.showSuccess(qsTr('导入成功'))
                } else {
                    HusApp.showError(qsTr('部分文件导入失败'))
                }
            }
        }
    }

    // 辅助函数
    function getSourceTypeText(type) {
        switch(type) {
            case 0: return qsTr('矢量数据')
            case 1: return qsTr('栅格瓦片')
            case 2: return qsTr('在线地图')
            case 3: return qsTr('地形数据')
            default: return qsTr('未知类型')
        }
    }
}
