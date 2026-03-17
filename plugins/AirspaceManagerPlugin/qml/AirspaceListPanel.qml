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
    property var selectedData: AirspaceSelection.selectedAirspaceData
    property var selectedProps: selectedData.properties ? selectedData.properties : ({})

    function isSelected(airspaceId) {
        return AirspaceSelection.selectedAirspaceId === airspaceId
    }

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

        Rectangle {
            id: selectionCard
            Layout.fillWidth: true
            visible: AirspaceSelection.hasSelection
            implicitHeight: selectionContent.implicitHeight + 20
            radius: HusTheme.Primary.radiusPrimary
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.08)
            border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.25)

            ColumnLayout {
                id: selectionContent
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true

                    HusText {
                        text: qsTr('当前选中')
                        font.pixelSize: 12
                        font.bold: true
                        color: HusTheme.Primary.colorPrimary
                    }

                    Item { Layout.fillWidth: true }

                    HusIconButton {
                        width: 24
                        height: 24
                        iconSource: HusIcon.CloseOutlined
                        iconSize: 12
                        type: HusButton.Type_Text
                        onClicked: AirspaceSelection.clearSelection()
                    }
                }

                HusText {
                    Layout.fillWidth: true
                    text: root.selectedData.name || qsTr('未命名空域')
                    font.pixelSize: 14
                    font.bold: true
                    elide: Text.ElideRight
                }

                HusText {
                    Layout.fillWidth: true
                    text: (root.selectedData.shapeTypeName || qsTr('未知'))
                          + qsTr('  |  ID: %1').arg(AirspaceSelection.selectedAirspaceId)
                    font.pixelSize: 11
                    color: HusTheme.Primary.colorTextSecondary
                    wrapMode: Text.WrapAnywhere
                }

                HusText {
                    Layout.fillWidth: true
                    visible: root.selectedProps.minAltitude !== undefined
                    text: qsTr('高度: %1m - %2m').arg(
                              root.selectedProps.minAltitude !== undefined ? root.selectedProps.minAltitude : 0,
                              root.selectedProps.maxAltitude !== undefined ? root.selectedProps.maxAltitude : 0)
                    font.pixelSize: 11
                    color: HusTheme.Primary.colorTextSecondary
                }

                HusText {
                    Layout.fillWidth: true
                    visible: !!root.selectedProps.remarks
                    text: qsTr('备注: %1').arg(root.selectedProps.remarks)
                    font.pixelSize: 11
                    color: HusTheme.Primary.colorTextSecondary
                    wrapMode: Text.WordWrap
                }
            }
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
                                color: root.isSelected(model.airspaceId)
                                             ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.12)
                                             : itemMouse.containsMouse
                                                 ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.06)
                                                 : "transparent"
                                border.width: root.isSelected(model.airspaceId) ? 1 : 0
                                border.color: root.isSelected(model.airspaceId)
                                                            ? HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.35)
                                                            : "transparent"

                visible: searchField.text.length === 0 ||
                         model.name.toLowerCase().indexOf(searchField.text.toLowerCase()) >= 0
                height: visible ? 40 : 0

                MouseArea {
                    id: itemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: AirspaceSelection.selectAirspace(model.airspaceId)
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
                            font.bold: root.isSelected(model.airspaceId)
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
