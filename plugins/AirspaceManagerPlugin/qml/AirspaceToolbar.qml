import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import AirspaceManager 1.0

/**
 * 空域绘制工具栏
 * 显示 9 种形状按钮 + 撤销/完成/取消操作
 */
Rectangle {
    id: root
    width: toolbarRow.width + 24
    height: 48
    radius: HusTheme.Primary.radiusPrimary
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.95)
    border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorBorder, 0.5)

    property bool isDrawing: EditRuntime.sessionActive

    RowLayout {
        id: toolbarRow
        anchors.centerIn: parent
        spacing: 4

        // ---- 形状按钮 ----
        Repeater {
            model: [
                { type: 0, icon: HusIcon.BorderOutlined,    tip: '矩形' },
                { type: 1, icon: HusIcon.StopOutlined,      tip: '正方形' },
                { type: 2, icon: HusIcon.LoadingOutlined,   tip: '圆形' },
                { type: 3, icon: HusIcon.StarOutlined,      tip: '多边形' },
                { type: 4, icon: HusIcon.LineOutlined,       tip: '边界线' },
                { type: 5, icon: HusIcon.SyncOutlined,      tip: '圆环' },
                { type: 6, icon: HusIcon.DashboardOutlined, tip: '圆弧' },
                { type: 7, icon: HusIcon.PieChartOutlined,  tip: '扇形' },
                { type: 8, icon: HusIcon.RadarChartOutlined, tip: '扇环形' }
            ]

            delegate: HusIconButton {
                width: 36
                height: 36
                iconSource: modelData.icon
                iconSize: 18
                type: DrawCtrl.currentShapeType === modelData.type
                      ? HusButton.Type_Primary : HusButton.Type_Text
                enabled: !root.isDrawing || DrawCtrl.currentShapeType === modelData.type

                onClicked: {
                    if (!root.isDrawing) {
                        EditRuntime.beginAirspaceEditSession(modelData.type)
                    }
                }

                HusToolTip {
                    visible: parent.hovered
                    text: modelData.tip
                    showArrow: true
                    position: HusToolTip.Position_Bottom
                }
            }
        }

        HusDivider {
            Layout.preferredHeight: 28
            orientation: Qt.Vertical
        }

        // ---- 操作按钮 ----
        HusIconButton {
            width: 36; height: 36
            iconSource: HusIcon.UndoOutlined
            iconSize: 16
            type: HusButton.Type_Text
            enabled: root.isDrawing && DrawCtrl.pointCount > 0
            onClicked: EditRuntime.undoLastControlPoint()

            HusToolTip {
                visible: parent.hovered
                text: qsTr('撤销')
                showArrow: true
                position: HusToolTip.Position_Bottom
            }
        }

        HusButton {
            text: qsTr('完成')
            type: HusButton.Type_Primary
            visible: root.isDrawing && DrawCtrl.requiredPoints < 0
            onClicked: EditRuntime.commitEdit()
        }

        HusButton {
            text: qsTr('取消')
            type: HusButton.Type_Text
            visible: root.isDrawing
            onClicked: EditRuntime.cancelEdit()
        }
    }

    // 状态提示
    HusText {
        anchors.top: parent.bottom
        anchors.topMargin: 6
        anchors.horizontalCenter: parent.horizontalCenter
        text: EditRuntime.statusText
        font.pixelSize: 12
        color: HusTheme.Primary.colorTextSecondary
        visible: text.length > 0
    }
}
