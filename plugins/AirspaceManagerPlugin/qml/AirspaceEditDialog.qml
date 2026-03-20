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
    property var originalStyleData: ({})
    property bool saveSucceeded: false
    property bool styleExpanded: true
    property bool dataExpanded: true

    signal saved(string airspaceId, bool created)
    signal cancelled()

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

    onClosed: {
        if (!root.saveSucceeded) {
            root.restoreOriginalLayerStyle()
        }
    }

    contentDelegate: Component {
        Flickable {
            anchors.fill: parent
            anchors.margins: 10
            contentHeight: contentCol.height
            clip: true

            ColumnLayout {
                id: contentCol
                width: parent.width
                spacing: 8

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

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: styleSection.implicitHeight + 20
                    radius: HusTheme.Primary.radiusPrimary
                    color: '#1A2534'
                    border.color: 'transparent'

                    ColumnLayout {
                        id: styleSection
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        Rectangle {
                            id: styleHeader
                            Layout.fillWidth: true
                            implicitHeight: 34
                            radius: HusTheme.Primary.radiusPrimary
                            color: '#87CEEB'

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 4
                                spacing: 8

                                HusText {
                                    text: qsTr('图形属性')
                                    font.pixelSize: 13
                                    font.bold: true
                                    color: '#ffffff'
                                }

                                Item { Layout.fillWidth: true }

                                HusIconButton {
                                    width: 28
                                    height: 28
                                    type: HusButton.Type_Text
                                    iconSource: root.styleExpanded ? HusIcon.UpOutlined : HusIcon.DownOutlined
                                    iconSize: 14
                                    colorText: '#ffffff'
                                    colorIcon: '#ffffff'
                                    onClicked: root.styleExpanded = !root.styleExpanded
                                }
                            }
                        }

                        ShapePropertyEditor {
                            id: shapeEditor
                            visible: root.styleExpanded
                            Layout.fillWidth: true

                            onStyleChanged: function() {
                                root.applyCurrentStyleToLayer()
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: dataSection.implicitHeight + 20
                    radius: HusTheme.Primary.radiusPrimary
                    color: '#1A2534'
                    border.color: 'transparent'

                    ColumnLayout {
                        id: dataSection
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        Rectangle {
                            id: dataHeader
                            Layout.fillWidth: true
                            implicitHeight: 34
                            radius: HusTheme.Primary.radiusPrimary
                            color: '#87CEEB'

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 4
                                spacing: 8

                                HusText {
                                    text: qsTr('数据属性')
                                    font.pixelSize: 13
                                    font.bold: true
                                    color: '#ffffff'
                                }

                                Item { Layout.fillWidth: true }

                                HusIconButton {
                                    width: 28
                                    height: 28
                                    type: HusButton.Type_Text
                                    iconSource: root.dataExpanded ? HusIcon.UpOutlined : HusIcon.DownOutlined
                                    iconSize: 14
                                    colorText: '#ffffff'
                                    colorIcon: '#ffffff'
                                    onClicked: root.dataExpanded = !root.dataExpanded
                                }
                            }
                        }

                        DataPropertyEditor {
                            id: dataEditor
                            visible: root.dataExpanded
                            Layout.fillWidth: true
                        }
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
}
