import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import AirspaceManager 1.0
import "components"

/**
 * 空域编辑弹窗 (右侧抽屉)
 * 绘制完成后弹出，编辑图形属性和数据属性
 */
HusDrawer {
    id: root
    edge: Qt.RightEdge
    drawerSize: 420
    title: editMode ? qsTr('编辑空域') : qsTr('新建空域')

    property bool editMode: false
    property string editId: ""
    property int shapeType: -1
    property string originalStyleJson: "{}"
    property bool saveSucceeded: false

    signal saved(string airspaceId, bool created)
    signal cancelled()

    function currentLayerId() {
        return 'airspace-' + root.editId
    }

    function currentLayerStyle() {
        try {
            return JSON.parse(shapeEditor.getStyleJson())
        } catch (e) {
            return {}
        }
    }

    function originalLayerStyle() {
        try {
            return JSON.parse(root.originalStyleJson || '{}')
        } catch (e) {
            return {}
        }
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
        root.saveSucceeded = false
        shapeEditor.setStyleData('{}')
        dataEditor.airspaceName = ""
        dataEditor.setPropertiesData('{}')
        root.open()
    }

    function openForEdit(airspaceId) {
        root.editMode = true
        root.editId = airspaceId
        let data = AirspaceModel.getAirspace(airspaceId)
        if (!data.id) return

        root.shapeType = data.shapeType
        root.originalStyleJson = data.styleJson || '{}'
        root.saveSucceeded = false
        dataEditor.airspaceName = data.name
        shapeEditor.setStyleData(data.styleJson)
        dataEditor.setPropertiesData(data.propertiesJson)
        root.open()
    }

    onClosed: {
        if (!root.saveSucceeded) {
            root.restoreOriginalLayerStyle()
        }
    }

    contentDelegate: Component {
        Flickable {
            anchors.fill: parent
            anchors.margins: 16
            contentHeight: contentCol.height
            clip: true

            ColumnLayout {
                id: contentCol
                width: parent.width
                spacing: 16

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

                // Tab 切换
                HusText {
                    text: qsTr('图形属性')
                    font.pixelSize: 14
                    font.bold: true
                }

                ShapePropertyEditor {
                    id: shapeEditor
                    Layout.fillWidth: true

                    onStyleChanged: function() {
                        root.applyCurrentStyleToLayer()
                    }
                }

                HusDivider {}

                HusText {
                    text: qsTr('数据属性')
                    font.pixelSize: 14
                    font.bold: true
                }

                DataPropertyEditor {
                    id: dataEditor
                    Layout.fillWidth: true
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
                            let styleJson = shapeEditor.getStyleJson()
                            let propsJson = dataEditor.getPropertiesJson()

                            if (root.editMode) {
                                let data = AirspaceModel.getAirspace(root.editId)
                                let updated = AirspaceModel.updateAirspace(
                                    root.editId,
                                    dataEditor.airspaceName,
                                    root.shapeType,
                                    data.geoJson,
                                    styleJson,
                                    propsJson
                                )
                                if (updated) {
                                    MapLibreEngine.updateLayerStyle(root.currentLayerId(), root.currentLayerStyle())
                                    root.saveSucceeded = true
                                    root.saved(root.editId, false)
                                }
                            } else {
                                let uuid = DrawCtrl.saveAirspace(
                                    dataEditor.airspaceName,
                                    styleJson,
                                    propsJson
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
}
