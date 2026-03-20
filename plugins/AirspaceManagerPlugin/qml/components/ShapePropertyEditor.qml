import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

/**
 * 图形属性编辑器
 * 编辑填充颜色、透明度、边框颜色、宽度与样式。
 */
ColumnLayout {
    id: root
    spacing: 12

    readonly property int labelWidth: 82
    readonly property int controlHeight: 34
    readonly property int sliderHeight: 30
    readonly property int valueWidth: 42
    readonly property color textColor: '#111111'

    property var styleData: ({
        "fill-color": "#3388ff",
        "fill-opacity": 0.3,
        "line-color": "#3388ff",
        "line-width": 2,
        "line-dasharray": []
    })

    signal styleChanged(var newStyle)

    function getStyleObject() {
        return styleData || {}
    }

    function getStyleJson() {
        return JSON.stringify(styleData)
    }

    function setStyleData(json) {
        try {
            let obj = JSON.parse(json)
            setStyleObject(obj)
        } catch(e) {}
    }

    function setStyleObject(obj) {
        let next = obj || {}
        styleData = next
        fillColorPicker.changeValue = next["fill-color"] || "#3388ff"
        fillOpacitySlider.value = (next["fill-opacity"] || 0.3) * 100
        lineColorPicker.changeValue = next["line-color"] || "#3388ff"
        lineWidthSlider.value = next["line-width"] || 2
        let dash = next["line-dasharray"] || []
        lineStyleSelect.currentIndex = dash.length > 0 ? (dash[0] === 4 ? 1 : 2) : 0
    }

    // 填充颜色
    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('填充颜色'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusColorPicker {
            id: fillColorPicker
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight

            contentItem: Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                implicitHeight: root.controlHeight - 4
                color: fillColorPicker.value || "#3388ff"
                radius: HusTheme.Primary.radiusPrimary
                border.width: 1
                border.color: HusTheme.Primary.colorBorder
            }
            defaultValue: "#3388ff"
            onChange: function(color) {
                root.styleData["fill-color"] = color
                root.styleChanged(root.styleData)
            }
        }
    }

    // 填充透明度
    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('填充透明度'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusSlider {
            id: fillOpacitySlider
            Layout.fillWidth: true
            Layout.preferredHeight: root.sliderHeight
            Layout.alignment: Qt.AlignVCenter
            min: 0; max: 100; value: 30
            stepSize: 5
            onCurrentValueChanged: {
                if (currentValue !== undefined) {
                    root.styleData["fill-opacity"] = currentValue / 100.0
                    root.styleChanged(root.styleData)
                }
            }
        }
        HusText { text: Math.round(fillOpacitySlider.currentValue || 0) + '%'; Layout.preferredWidth: root.valueWidth; font.pixelSize: 12; horizontalAlignment: Text.AlignRight; color: root.textColor; Layout.alignment: Qt.AlignVCenter }
    }

    HusDivider {}

    // 边框颜色
    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('边框颜色'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusColorPicker {
            id: lineColorPicker
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight

            contentItem: Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                implicitHeight: root.controlHeight - 4
                color: lineColorPicker.value || "#3388ff"
                radius: HusTheme.Primary.radiusPrimary
                border.width: 1
                border.color: HusTheme.Primary.colorBorder
            }
            defaultValue: "#3388ff"
            onChange: function(color) {
                root.styleData["line-color"] = color
                root.styleChanged(root.styleData)
            }
        }
    }

    // 边框宽度
    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('边框宽度'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusSlider {
            id: lineWidthSlider
            Layout.fillWidth: true
            Layout.preferredHeight: root.sliderHeight
            Layout.alignment: Qt.AlignVCenter
            min: 1; max: 10; value: 2
            stepSize: 1
            onCurrentValueChanged: {
                if (currentValue !== undefined) {
                    root.styleData["line-width"] = currentValue
                    root.styleChanged(root.styleData)
                }
            }
        }
        HusText { text: Math.round(lineWidthSlider.currentValue || 0) + 'px'; Layout.preferredWidth: root.valueWidth; font.pixelSize: 12; horizontalAlignment: Text.AlignRight; color: root.textColor; Layout.alignment: Qt.AlignVCenter }
    }

    // 边框样式
    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('边框样式'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusSelect {
            id: lineStyleSelect
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight

            model: [qsTr('实线'), qsTr('虚线'), qsTr('点线')]
            currentIndex: 0
            onCurrentIndexChanged: {
                switch (currentIndex) {
                case 0: root.styleData["line-dasharray"] = []; break
                case 1: root.styleData["line-dasharray"] = [4, 4]; break
                case 2: root.styleData["line-dasharray"] = [1, 4]; break
                }
                root.styleChanged(root.styleData)
            }
        }
    }
}
