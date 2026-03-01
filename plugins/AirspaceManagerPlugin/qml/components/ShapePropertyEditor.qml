import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import AirspaceManager 1.0

/**
 * 图形属性编辑器
 * 编辑填充颜色/透明度、边框颜色/宽度/样式等
 */
ColumnLayout {
    id: root
    spacing: 12

    property var styleData: ({
        "fill-color": "#3388ff",
        "fill-opacity": 0.3,
        "line-color": "#3388ff",
        "line-width": 2,
        "line-dasharray": []
    })

    signal styleChanged(var newStyle)

    function getStyleJson() {
        return JSON.stringify(styleData)
    }

    function setStyleData(json) {
        try {
            let obj = JSON.parse(json)
            styleData = obj
            fillColorPicker.currentColor = obj["fill-color"] || "#3388ff"
            fillOpacitySlider.value = (obj["fill-opacity"] || 0.3) * 100
            lineColorPicker.currentColor = obj["line-color"] || "#3388ff"
            lineWidthSlider.value = obj["line-width"] || 2
            let dash = obj["line-dasharray"] || []
            lineStyleSelect.currentIndex = dash.length > 0 ? (dash[0] === 4 ? 1 : 2) : 0
        } catch(e) {}
    }

    // 填充颜色
    RowLayout {
        spacing: 8
        HusText { text: qsTr('填充颜色'); Layout.preferredWidth: 80 }
        HusColorPicker {
            id: fillColorPicker
            currentColor: "#3388ff"
            onCurrentColorChanged: {
                styleData["fill-color"] = currentColor
                root.styleChanged(styleData)
            }
        }
    }

    // 填充透明度
    RowLayout {
        spacing: 8
        HusText { text: qsTr('填充透明度'); Layout.preferredWidth: 80 }
        HusSlider {
            id: fillOpacitySlider
            Layout.fillWidth: true
            from: 0; to: 100; value: 30
            stepSize: 5
            onValueChanged: {
                styleData["fill-opacity"] = value / 100.0
                root.styleChanged(styleData)
            }
        }
        HusText { text: fillOpacitySlider.value + '%'; Layout.preferredWidth: 40 }
    }

    HusDivider {}

    // 边框颜色
    RowLayout {
        spacing: 8
        HusText { text: qsTr('边框颜色'); Layout.preferredWidth: 80 }
        HusColorPicker {
            id: lineColorPicker
            currentColor: "#3388ff"
            onCurrentColorChanged: {
                styleData["line-color"] = currentColor
                root.styleChanged(styleData)
            }
        }
    }

    // 边框宽度
    RowLayout {
        spacing: 8
        HusText { text: qsTr('边框宽度'); Layout.preferredWidth: 80 }
        HusSlider {
            id: lineWidthSlider
            Layout.fillWidth: true
            from: 1; to: 10; value: 2
            stepSize: 1
            onValueChanged: {
                styleData["line-width"] = value
                root.styleChanged(styleData)
            }
        }
        HusText { text: lineWidthSlider.value + 'px'; Layout.preferredWidth: 40 }
    }

    // 边框样式
    RowLayout {
        spacing: 8
        HusText { text: qsTr('边框样式'); Layout.preferredWidth: 80 }
        HusSelect {
            id: lineStyleSelect
            Layout.fillWidth: true
            model: [qsTr('实线'), qsTr('虚线'), qsTr('点线')]
            currentIndex: 0
            onCurrentIndexChanged: {
                switch (currentIndex) {
                case 0: styleData["line-dasharray"] = []; break
                case 1: styleData["line-dasharray"] = [4, 4]; break
                case 2: styleData["line-dasharray"] = [1, 4]; break
                }
                root.styleChanged(styleData)
            }
        }
    }
}
