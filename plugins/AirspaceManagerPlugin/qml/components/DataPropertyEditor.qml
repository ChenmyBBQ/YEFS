import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

/**
 * 数据属性编辑器
 * 编辑空域名称、类型、高度、时间、备注等
 */
ColumnLayout {
    id: root
    spacing: 12

    property string airspaceName: ""
    property int airspaceType: 0
    property double minAltitude: 0
    property double maxAltitude: 1000
    property string effectiveTime: ""
    property string expirationTime: ""
    property string remarks: ""

    signal dataChanged()

    function getPropertiesJson() {
        return JSON.stringify({
            "airspaceType": airspaceType,
            "minAltitude": minAltitude,
            "maxAltitude": maxAltitude,
            "effectiveTime": effectiveTime,
            "expirationTime": expirationTime,
            "remarks": remarks
        })
    }

    function setPropertiesData(json) {
        try {
            let obj = JSON.parse(json)
            airspaceType = obj.airspaceType || 0
            minAltitude = obj.minAltitude || 0
            maxAltitude = obj.maxAltitude || 1000
            effectiveTime = obj.effectiveTime || ""
            expirationTime = obj.expirationTime || ""
            remarks = obj.remarks || ""
        } catch(e) {}
    }

    // 空域名称
    RowLayout {
        spacing: 8
        HusText { text: qsTr('空域名称'); Layout.preferredWidth: 80; color: HusTheme.Primary.colorError }
        HusInput {
            id: nameInput
            Layout.fillWidth: true
            placeholderText: qsTr('请输入空域名称')
            text: root.airspaceName
            onTextChanged: {
                root.airspaceName = text
                root.dataChanged()
            }
        }
    }

    // 空域类型
    RowLayout {
        spacing: 8
        HusText { text: qsTr('空域类型'); Layout.preferredWidth: 80 }
        HusSelect {
            id: typeSelect
            Layout.fillWidth: true
            model: [qsTr('限制区'), qsTr('禁飞区'), qsTr('危险区'), qsTr('训练区'), qsTr('自定义')]
            currentIndex: root.airspaceType
            onCurrentIndexChanged: {
                root.airspaceType = currentIndex
                root.dataChanged()
            }
        }
    }

    HusDivider {}

    // 最低高度
    RowLayout {
        spacing: 8
        HusText { text: qsTr('最低高度(m)'); Layout.preferredWidth: 80 }
        HusInput {
            Layout.fillWidth: true
            placeholderText: "0"
            text: root.minAltitude.toString()
            onTextChanged: {
                root.minAltitude = parseFloat(text) || 0
                root.dataChanged()
            }
        }
    }

    // 最高高度
    RowLayout {
        spacing: 8
        HusText { text: qsTr('最高高度(m)'); Layout.preferredWidth: 80 }
        HusInput {
            Layout.fillWidth: true
            placeholderText: "1000"
            text: root.maxAltitude.toString()
            onTextChanged: {
                root.maxAltitude = parseFloat(text) || 1000
                root.dataChanged()
            }
        }
    }

    HusDivider {}

    // 生效时间
    RowLayout {
        spacing: 8
        HusText { text: qsTr('生效时间'); Layout.preferredWidth: 80 }
        HusInput {
            Layout.fillWidth: true
            placeholderText: qsTr('可选，如 2026-01-01 08:00')
            text: root.effectiveTime
            onTextChanged: {
                root.effectiveTime = text
                root.dataChanged()
            }
        }
    }

    // 失效时间
    RowLayout {
        spacing: 8
        HusText { text: qsTr('失效时间'); Layout.preferredWidth: 80 }
        HusInput {
            Layout.fillWidth: true
            placeholderText: qsTr('可选')
            text: root.expirationTime
            onTextChanged: {
                root.expirationTime = text
                root.dataChanged()
            }
        }
    }

    HusDivider {}

    // 备注
    RowLayout {
        spacing: 8
        HusText { text: qsTr('备注'); Layout.preferredWidth: 80 }
        HusInput {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            placeholderText: qsTr('可选')
            text: root.remarks
            onTextChanged: {
                root.remarks = text
                root.dataChanged()
            }
        }
    }
}
