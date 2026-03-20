import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic
import AirspaceManager 1.0
import YEFSApp

HusCard {
    id: root
    width: 320
    height: 780

    visible: (DrawCtrl.drawingState === DrawCtrl.Drawing || DrawCtrl.drawingState === DrawCtrl.Editing)
             && DrawCtrl.pointCount > 0

    property var shapeInfo: DrawCtrl.currentShapeInfo || {}
    property int shapeType: shapeInfo.type !== undefined ? shapeInfo.type : -1
    property int pointCount: shapeInfo.pointCount !== undefined ? shapeInfo.pointCount : 0
    readonly property string shapeName: root.getShapeName(root.shapeType)
    readonly property int infoLabelWidth: 92
    readonly property int sectionInnerMargin: 10
    readonly property int sectionSpacing: 8
    readonly property int sectionHeaderHeight: 34
    readonly property int sectionToggleButtonSize: 28
    readonly property color sectionCardColor: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.98)
    readonly property color sectionHeaderColor: '#87CEEB'
    readonly property color sectionHeaderTextColor: '#ffffff'
    readonly property color contentTextColor: '#111111'
    property bool styleExpanded: true
    property bool dataExpanded: true
    property bool detailExpanded: false

    function triggerSave(complete) {
        let styleData = shapeEditor.getStyleObject()
        let propertiesData = dataEditor.getPropertiesObject()
        
        let uuid = DrawCtrl.saveAirspaceData(
            dataEditor.airspaceName || '未命名空域_' + Date.now(),
            styleData,
            propertiesData
        )
        if (uuid) {
            let data = AirspaceModel.getAirspace(uuid)
            if (data.id) {
                MapLibreEngine.addGeoJSONLayer('airspace-' + data.id, data.geoJsonObject || {}, data.styleData || {})
            }
        }
        if (complete) {
            EditRuntime.cancelEdit()
        }
    }

    function currentPreviewStyle(styleOverride) {
        let style = styleOverride || shapeEditor.styleData || {}
        return {
            "fill-color": style["fill-color"] || "#3388ff",
            "fill-opacity": style["fill-opacity"] !== undefined ? style["fill-opacity"] : 0.3,
            "line-color": style["line-color"] || "#3388ff",
            "line-width": style["line-width"] !== undefined ? style["line-width"] : 2,
            "line-dasharray": style["line-dasharray"] || []
        }
    }

    function applyPreviewStyle(styleOverride) {
        let geoJson = DrawCtrl.previewGeoJson()
        if (!geoJson || Object.keys(geoJson).length === 0)
            return

        MapLibreEngine.updateLayerStyle('airspace-preview', root.currentPreviewStyle(styleOverride))
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            HusText {
                text: '空域名称'
                Layout.preferredWidth: 60
                font.pixelSize: 12
                color: root.contentTextColor
            }

            HusInput {
                id: airspaceNameInput
                Layout.fillWidth: true
                Layout.preferredHeight: 34

                placeholderText: '请输入空域名称'
                text: dataEditor.airspaceName
                onTextChanged: {
                    if (dataEditor.airspaceName !== text) {
                        dataEditor.airspaceName = text
                        dataEditor.dataChanged()
                    }
                }
            }

            HusText {
                id: nameTag
                text: root.shapeName
                font.pixelSize: 12
                color: root.contentTextColor
                Layout.alignment: Qt.AlignVCenter
            }
        }

        ScrollView {
            id: scrollArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            LayoutMirroring.enabled: false
            LayoutMirroring.childrenInherit: false
            contentWidth: availableWidth
            contentHeight: contentColumn.implicitHeight + 16
            ScrollBar.vertical: HusScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: contentColumn
                width: Math.max(0, scrollArea.availableWidth - 10)
                spacing: root.sectionSpacing

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: styleSection.implicitHeight + 20
                    radius: HusTheme.Primary.radiusPrimary
                    color: root.sectionCardColor
                    border.color: 'transparent'

                    ColumnLayout {
                        id: styleSection
                        anchors.fill: parent
                        anchors.margins: root.sectionInnerMargin
                        spacing: root.sectionSpacing

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: root.sectionHeaderHeight
                            radius: HusTheme.Primary.radiusPrimary
                            color: root.sectionHeaderColor

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 4
                                spacing: 8

                                HusText {
                                    text: '图形属性'
                                    font.pixelSize: 13
                                    font.bold: true
                                    color: root.sectionHeaderTextColor
                                }

                                Item { Layout.fillWidth: true }

                                HusIconButton {
                                    width: root.sectionToggleButtonSize
                                    height: root.sectionToggleButtonSize
                                    type: HusButton.Type_Text
                                    iconSource: root.styleExpanded ? HusIcon.UpOutlined : HusIcon.DownOutlined
                                    iconSize: 14
                                    colorText: root.sectionHeaderTextColor
                                    colorIcon: root.sectionHeaderTextColor
                                    onClicked: root.styleExpanded = !root.styleExpanded
                                }
                            }
                        }

                        ShapePropertyEditor {
                            id: shapeEditor
                            visible: root.styleExpanded
                            Layout.fillWidth: true

                            onStyleChanged: function(newStyle) {
                                root.applyPreviewStyle(newStyle)
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: dataSection.implicitHeight + 20
                    radius: HusTheme.Primary.radiusPrimary
                    color: root.sectionCardColor
                    border.color: 'transparent'

                    ColumnLayout {
                        id: dataSection
                        anchors.fill: parent
                        anchors.margins: root.sectionInnerMargin
                        spacing: root.sectionSpacing

                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: root.sectionHeaderHeight
                            radius: HusTheme.Primary.radiusPrimary
                            color: root.sectionHeaderColor

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 4
                                spacing: 8

                                HusText {
                                    text: '数据属性'
                                    font.pixelSize: 13
                                    font.bold: true
                                    color: root.sectionHeaderTextColor
                                }

                                Item { Layout.fillWidth: true }

                                HusIconButton {
                                    width: root.sectionToggleButtonSize
                                    height: root.sectionToggleButtonSize
                                    type: HusButton.Type_Text
                                    iconSource: root.dataExpanded ? HusIcon.UpOutlined : HusIcon.DownOutlined
                                    iconSize: 14
                                    colorText: root.sectionHeaderTextColor
                                    colorIcon: root.sectionHeaderTextColor
                                    onClicked: root.dataExpanded = !root.dataExpanded
                                }
                            }
                        }

                        DataPropertyEditor {
                            id: dataEditor
                            visible: root.dataExpanded
                            Layout.fillWidth: true
                            showNameField: false
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: detailHeader.implicitHeight + (root.detailExpanded ? detailSection.implicitHeight + 30 : 20)
                    radius: HusTheme.Primary.radiusPrimary
                    color: root.sectionCardColor
                    border.color: 'transparent'

                    ColumnLayout {
                        id: detailSection
                        anchors.fill: parent
                        anchors.margins: root.sectionInnerMargin
                        spacing: root.sectionSpacing

                        Rectangle {
                            id: detailHeader
                            Layout.fillWidth: true
                            implicitHeight: root.sectionHeaderHeight
                            radius: HusTheme.Primary.radiusPrimary
                            color: root.sectionHeaderColor

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 4
                                spacing: 8

                                HusText {
                                    text: '实际经纬度数据'
                                    font.pixelSize: 13
                                    font.bold: true
                                    color: root.sectionHeaderTextColor
                                }

                                Item { Layout.fillWidth: true }

                                HusIconButton {
                                    width: root.sectionToggleButtonSize
                                    height: root.sectionToggleButtonSize
                                    type: HusButton.Type_Text
                                    iconSource: root.detailExpanded ? HusIcon.UpOutlined : HusIcon.DownOutlined
                                    iconSize: 14
                                    colorText: root.sectionHeaderTextColor
                                    colorIcon: root.sectionHeaderTextColor
                                    onClicked: root.detailExpanded = !root.detailExpanded
                                }
                            }
                        }

                        ColumnLayout {
                            visible: root.detailExpanded
                            Layout.fillWidth: true
                            spacing: 10

                                GridLayout {
                                    id: geometryGrid
                                    visible: (root.shapeType === 0 || root.shapeType === 1)
                                             && root.shapeInfo.diagonalMeters !== undefined
                                    columns: 2
                                    rowSpacing: 8
                                    columnSpacing: 10
                                    HusText { text: '中心纬度'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: Number(root.shapeInfo.centerLat || 0).toFixed(6) + '°'; color: root.contentTextColor }
                                    HusText { text: '中心经度'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: Number(root.shapeInfo.centerLng || 0).toFixed(6) + '°'; color: root.contentTextColor }
                                    HusText { text: '对角距离'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: UnitManager.formatDistance(root.shapeInfo.diagonalMeters || 0); color: root.contentTextColor }
                                }

                                GridLayout {
                                    id: circleGrid
                                    visible: root.shapeType === 2 && root.shapeInfo.radiusMeters !== undefined
                                    columns: 2
                                    rowSpacing: 8
                                    columnSpacing: 10
                                    HusText { text: '圆心纬度'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: Number(root.shapeInfo.centerLat || 0).toFixed(6) + '°'; color: root.contentTextColor }
                                    HusText { text: '圆心经度'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: Number(root.shapeInfo.centerLng || 0).toFixed(6) + '°'; color: root.contentTextColor }
                                    HusText { text: '半径'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: UnitManager.formatDistance(root.shapeInfo.radiusMeters || 0); color: root.contentTextColor }
                                }

                                GridLayout {
                                    id: pointGrid
                                    visible: root.shapeType === 3 || root.shapeType === 4
                                    columns: 2
                                    rowSpacing: 8
                                    columnSpacing: 10
                                    HusText { text: '当前节点'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: root.pointCount + ' 个点'; color: root.contentTextColor }
                                }

                                GridLayout {
                                    id: ringGrid
                                    visible: (root.shapeType === 5 || root.shapeType === 8)
                                             && root.shapeInfo.outerRadius !== undefined
                                    columns: 2
                                    rowSpacing: 8
                                    columnSpacing: 10
                                    HusText { text: '外半径'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: UnitManager.formatDistance(root.shapeInfo.outerRadius || 0); color: root.contentTextColor }
                                    HusText { text: '内半径'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: UnitManager.formatDistance(root.shapeInfo.innerRadius || 0); color: root.contentTextColor }
                                }

                                GridLayout {
                                    id: angleGrid
                                    visible: (root.shapeType === 6 || root.shapeType === 7 || root.shapeType === 8)
                                             && root.shapeInfo.startAngle !== undefined
                                    columns: 2
                                    rowSpacing: 8
                                    columnSpacing: 10
                                    HusText { text: '起始角'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: Number(root.shapeInfo.startAngle || 0).toFixed(1) + '°'; color: root.contentTextColor }
                                    HusText { text: '结束角'; color: root.contentTextColor; Layout.preferredWidth: root.infoLabelWidth }
                                    HusText { text: Number(root.shapeInfo.endAngle || 0).toFixed(1) + '°'; color: root.contentTextColor }
                                }
                            }
                        }
                    }
                }
            }

        HusDivider { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            HusButton {
                text: '确认'
                type: HusButton.Type_Primary

                enabled: airspaceNameInput.text.length > 0
                onClicked: triggerSave(true)
            }

            HusButton {
                text: '应用'

                enabled: airspaceNameInput.text.length > 0
                onClicked: triggerSave(false)
            }

            Item { Layout.fillWidth: true }

            HusButton {
                text: '取消'

                onClicked: EditRuntime.cancelEdit()
            }
        }
    }

    function getShapeName(type) {
        switch (type) {
            case 0: return '矩形'
            case 1: return '正方形'
            case 2: return '圆形'
            case 3: return '多边形'
            case 4: return '边界线'
            case 5: return '圆环'
            case 6: return '圆弧'
            case 7: return '扇形'
            case 8: return '扇环'
            default: return '未知图形'
        }
    }
}
