import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import AirspaceManager 1.0
import YEFSApp

/**
 * 空域列表侧面板 (插件 sidePanel)
 * 紧凑型列表，嵌入 MapPage 左侧
 */
Rectangle {
    id: root
    color: HusTheme.Primary.colorBgContainer
    width: 260

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // 标题
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            HusText {
                text: qsTr('空域列表')
                font.pixelSize: 14
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            HusText {
                text: qsTr('%1 项').arg(AirspaceModel.count)
                font.pixelSize: 11
                color: HusTheme.Primary.colorTextSecondary
                visible: AirspaceModel.count > 0
            }
        }

        HusDivider {}

        // 搜索
        HusInput {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr('搜索...')
            iconSource: HusIcon.SearchOutlined
        }

        // 列表
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4

            model: AirspaceModel

            delegate: Rectangle {
                width: listView.width
                radius: HusTheme.Primary.radiusPrimary
                color: itemMouse.containsMouse
                       ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.06)
                       : "transparent"

                visible: searchField.text.length === 0 ||
                         model.name.toLowerCase().indexOf(searchField.text.toLowerCase()) >= 0
                height: visible ? 40 : 0

                MouseArea {
                    id: itemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 4
                    spacing: 6

                    // 可见性指示
                    Rectangle {
                        width: 6; height: 6
                        radius: 3
                        color: model.visible ? HusTheme.Primary.colorSuccess : HusTheme.Primary.colorTextDisabled
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1

                        HusText {
                            text: model.name
                            font.pixelSize: 13
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        HusText {
                            text: model.shapeTypeName
                            font.pixelSize: 10
                            color: HusTheme.Primary.colorTextTertiary
                        }
                    }

                    // 可见性切换
                    HusIconButton {
                        width: 24; height: 24
                        iconSource: model.visible ? HusIcon.EyeOutlined : HusIcon.EyeInvisibleOutlined
                        iconSize: 12
                        type: HusButton.Type_Text
                        onClicked: {
                            AirspaceModel.toggleVisibility(model.airspaceId)
                            MapLibreEngine.setLayerVisibility("airspace-" + model.airspaceId, !model.visible)
                        }
                    }

                    // 定位
                    HusIconButton {
                        width: 24; height: 24
                        iconSource: HusIcon.AimOutlined
                        iconSize: 12
                        type: HusButton.Type_Text
                        visible: itemMouse.containsMouse
                    }
                }
            }
        }

        // 空状态
        HusText {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr('无空域数据')
            font.pixelSize: 12
            color: HusTheme.Primary.colorTextSecondary
            visible: AirspaceModel.count === 0
        }
    }
}
