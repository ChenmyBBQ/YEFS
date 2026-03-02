import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import AirspaceManager 1.0

/**
 * 空域管理设置页面 (插件 settingsPage)
 * 配置默认样式、数据库管理等
 */
Rectangle {
    id: root
    color: HusTheme.Primary.colorBgContainer

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        HusText {
            text: qsTr('空域管理设置')
            font.pixelSize: 18
            font.bold: true
        }

        HusDivider {}

        // 默认样式设置
        HusText {
            text: qsTr('默认绘制样式')
            font.pixelSize: 14
            font.bold: true
            color: HusTheme.Primary.colorTextSecondary
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 10

            HusText { text: qsTr('填充颜色') }
            HusInput {
                id: defaultFillColor
                Layout.fillWidth: true
                text: "#1890ff"
                placeholderText: "#RRGGBB"
            }

            HusText { text: qsTr('填充透明度') }
            HusSlider {
                id: defaultFillOpacity
                Layout.fillWidth: true
                from: 0
                to: 100
                value: 30
                stepSize: 1
            }

            HusText { text: qsTr('边框颜色') }
            HusInput {
                id: defaultLineColor
                Layout.fillWidth: true
                text: "#1890ff"
                placeholderText: "#RRGGBB"
            }

            HusText { text: qsTr('边框宽度') }
            HusSlider {
                id: defaultLineWidth
                Layout.fillWidth: true
                from: 1
                to: 10
                value: 2
                stepSize: 1
            }
        }

        HusDivider {}

        // 数据管理
        HusText {
            text: qsTr('数据管理')
            font.pixelSize: 14
            font.bold: true
            color: HusTheme.Primary.colorTextSecondary
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            HusText {
                text: qsTr('数据库路径:')
                font.pixelSize: 13
            }

            HusText {
                text: AirspaceDB.databasePath
                font.pixelSize: 12
                color: HusTheme.Primary.colorTextSecondary
                Layout.fillWidth: true
                elide: Text.ElideMiddle
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            HusText {
                text: qsTr('空域数量: %1').arg(AirspaceModel.count)
                font.pixelSize: 13
            }

            Item { Layout.fillWidth: true }

            HusButton {
                text: qsTr('清空所有数据')
                type: HusButton.Type_Default
                danger: true
                onClicked: {
                    clearConfirmDialog.open()
                }
            }
        }

        HusDivider {}

        // 绘制设置
        HusText {
            text: qsTr('绘制参数')
            font.pixelSize: 14
            font.bold: true
            color: HusTheme.Primary.colorTextSecondary
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 10

            HusText { text: qsTr('圆形分段数') }
            HusSlider {
                id: circleSegments
                Layout.fillWidth: true
                from: 16
                to: 128
                value: 64
                stepSize: 8
            }

            HusText { text: qsTr('弧线分段数') }
            HusSlider {
                id: arcSegments
                Layout.fillWidth: true
                from: 8
                to: 64
                value: 32
                stepSize: 4
            }
        }

        Item { Layout.fillHeight: true }

        // 底部操作
        RowLayout {
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }

            HusButton {
                text: qsTr('恢复默认')
                type: HusButton.Type_Default
                onClicked: {
                    defaultFillColor.text = "#1890ff"
                    defaultFillOpacity.value = 30
                    defaultLineColor.text = "#1890ff"
                    defaultLineWidth.value = 2
                    circleSegments.value = 64
                    arcSegments.value = 32
                }
            }

            HusButton {
                text: qsTr('保存设置')
                type: HusButton.Type_Primary
                onClicked: {
                    // TODO: 通过 SettingsManager 持久化设置
                }
            }
        }
    }

    // 清空确认对话框
    HusModal {
        id: clearConfirmDialog
        parent: Overlay.overlay
        title: qsTr('确认清空')

        contentItem: HusText {
            text: qsTr('确定要清空所有空域数据吗？此操作不可恢复。')
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            AirspaceDB.clearAll()
            AirspaceModel.loadFromDatabase()
        }
    }
}
