import QtQuick
// removed unused import: QtQuick.Layouts
import HuskarUI.Basic

import YEFSApp

/**
 * 单位设置分类组件
 * 支持航空任务规划相关的公制/英制单位切换
 */
Column {
    id: root
    width: parent ? parent.width : 400
    spacing: 16

    // 单位分类配置
    readonly property var unitCategories: [
        { id: "altitude", name: qsTr("高度"), desc: qsTr("飞行高度、海拔高度") },
        { id: "distance", name: qsTr("距离"), desc: qsTr("航段距离、航程") },
        { id: "speed", name: qsTr("速度"), desc: qsTr("空速、地速、风速") },
        { id: "verticalSpeed", name: qsTr("垂直速度"), desc: qsTr("爬升率、下降率") },
        { id: "pressure", name: qsTr("气压"), desc: qsTr("QNH、高度表设定") },
        { id: "temperature", name: qsTr("温度"), desc: qsTr("气温、露点") },
        { id: "weight", name: qsTr("重量"), desc: qsTr("载重、燃油重量") },
        { id: "volume", name: qsTr("体积"), desc: qsTr("燃油体积") },
        { id: "visibility", name: qsTr("能见度"), desc: qsTr("水平能见度、RVR") }
    ]

    HusText {
        text: qsTr('单位设置')
        font.weight: Font.DemiBold
    }

    UnitPresetPanel {
    }

    UnitCategoryPanel {
        unitCategories: root.unitCategories
    }

    UnitInfoPanel {
    }
}
