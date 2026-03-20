import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import HuskarUI.Basic
import AirspaceManager 1.0
import "components"

/**
 * 空域编辑弹窗 (浮动窗口)
 * 绘制完成后弹出，编辑图形属性和数据属性
 */
Window {
    id: root
    flags: Qt.Window
    width: 420
    height: 700
    title: editMode ? qsTr('编辑空域') : qsTr('新建空域')
    color: HusTheme.Primary.colorBgContainer

    property bool editMode: false
    property string editId: ""
    property int shapeType: -1
    property string originalStyleJson: "{}"
    property var originalStyleData: ({})
    property bool saveSucceeded: false

    signal saved(string airspaceId, bool created)
    signal cancelled()

    function open() { root.show() }
    function close() { root.hide() }

    function currentLayerId() {
        return 'airspace-' + root.editId
    }

    function currentLayerStyle() {
        return shapeEditor.getStyleObject()
    }

    function originalLayerStyle() {
        return root.originalStyleData || {}
    }

    function applyCurrentStyleToLayer() {
        if (!root.editMode || !root.editId)
            return

        MapLibreEngine.updateLayerStyle(root.currentLayerId(), root.currentLayerStyle())
    }

    function restoreOriginalLayerStyle() {
        if (!root.editMode || !root.editId)
            return

        MapLibreEngine.updateLayerStyle(root.currentLayerId(), root.originalLayerStyle())
    }

    function openForNew(geoJson, shapeType) {
        root.editMode = false
        root.editId = ""
        root.shapeType = shapeType
        root.originalStyleJson = '{}'
        root.originalStyleData = {}
        root.saveSucceeded = false
        shapeEditor.setStyleObject({})
        dataEditor.airspaceName = ""
        dataEditor.setPropertiesObject({})
        root.open()
    }

    function openForEdit(airspaceId) {
        root.editMode = true
        root.editId = airspaceId
        let data = AirspaceModel.getAirspace(airspaceId)
        if (!data.id) return

        root.shapeType = data.shapeType
        root.originalStyleJson = data.styleJson || '{}'
        root.originalStyleData = data.styleData || {}
        root.saveSucceeded = false
        dataEditor.airspaceName = data.name
        shapeEditor.setStyleObject(data.styleData || {})
        dataEditor.setPropertiesObject(data.propertiesData || {})
        root.open()
    }

    onClosing: {
        if (!root.saveSucceeded) {
            root.restoreOriginalLayerStyle()
        }
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: 8
        contentHeight: contentCol.height
        clip: true

        ColumnLayout {
            id: contentCol
            width: parent.width
            spacing: 6

            // 形状类型显示
            RowLayout {
                spacing: 8
                HusText {
                    text: qsTr('形状类型')
                    font.bold: true
                }
                HusText {
                    text: AirspaceModel.shapeTypeName
                            ? AirspaceModel.shapeTypeName(root.shapeType)
                            : '—'
                    color: HusTheme.Primary.colorPrimary
                }
            }

            HusDivider {}

            // 图形属性
            HusText {
                text: qsTr('图形属性')
                font.pixelSize: 14
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: shapeEditor.implicitHeight + 16
                color: "#1E2B3C" // Dark blue
                radius: 4

                ShapePropertyEditor {
                    id: shapeEditor
                    anchors.fill: parent
                    anchors.margins: 8
                    
                    onStyleChanged: function() {
                        root.applyCurrentStyleToLayer()
                    }
                }
            }

            HusDivider {}

            // 数据属性
            HusText {
                text: qsTr('数据属性')
                font.pixelSize: 14
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: dataEditor.implicitHeight + 16
                color: "#1E2B3C" // Dark blue
                radius: 4

                DataPropertyEditor {
                    id: dataEditor
                    anchors.fill: parent
                    anchors.margins: 8
                }
            }

            // 底部按钮
            Item { Layout.preferredHeight: 16 }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Item { Layout.fillWidth: true }

                HusButton {
                    text: qsTr('取消')
                    type: HusButton.Type_Default
                    onClicked: {
                        root.saveSucceeded = false
                        root.restoreOriginalLayerStyle()
                        root.close()
                        root.cancelled()
                    }
                }

                HusButton {
                    text: root.editMode ? qsTr('更新') : qsTr('保存')
                    type: HusButton.Type_Primary
                    enabled: dataEditor.airspaceName.length > 0
                    onClicked: {
                        let styleData = shapeEditor.getStyleObject()
                        let propertiesData = dataEditor.getPropertiesObject()

                        if (root.editMode) {
                            let data = AirspaceModel.getAirspace(root.editId)
                            let updated = AirspaceModel.updateAirspaceData(
                                root.editId,
                                dataEditor.airspaceName,
                                root.shapeType,
                                data.geoJsonObject || {},
                                styleData,
                                propertiesData
                            )
                            if (updated) {
                                MapLibreEngine.updateLayerStyle(root.currentLayerId(), root.currentLayerStyle())
                                root.saveSucceeded = true
                                root.saved(root.editId, false)
                            }
                        } else {
                            let uuid = DrawCtrl.saveAirspaceData(
                                dataEditor.airspaceName,
                                styleData,
                                propertiesData
                            )
                            if (uuid) {
                                root.saveSucceeded = true
                                root.saved(uuid, true)
                            }
                        }
                        root.close()
                    }
                }
            }
        }
    }
}
