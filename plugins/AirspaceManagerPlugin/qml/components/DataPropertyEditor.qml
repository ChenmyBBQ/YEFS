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

    readonly property int labelWidth: 82
    readonly property int controlHeight: 34
    readonly property color textColor: '#ffffff'
    property bool showNameField: true

    property string airspaceName: ""
    property int airspaceType: 0
    property double minAltitude: 0
    property double maxAltitude: 1000
    property string effectiveTime: ""
    property string expirationTime: ""
    property string remarks: ""

    signal dataChanged()

    function getPropertiesObject() {
        return {
            "airspaceType": airspaceType,
            "minAltitude": minAltitude,
            "maxAltitude": maxAltitude,
            "effectiveTime": effectiveTime,
            "expirationTime": expirationTime,
            "remarks": remarks
        }
    }

    function getPropertiesJson() {
        return JSON.stringify(getPropertiesObject())
    }

    function setPropertiesData(json) {
        try {
            let obj = JSON.parse(json)
            setPropertiesObject(obj)
        } catch(e) {}
    }

    function setPropertiesObject(obj) {
        let next = obj || {}
        airspaceType = next.airspaceType || 0
        minAltitude = next.minAltitude || 0
        maxAltitude = next.maxAltitude || 1000
        effectiveTime = next.effectiveTime || ""
        expirationTime = next.expirationTime || ""
        remarks = next.remarks || ""
    }

    // 空域名称
    RowLayout {
        visible: root.showNameField
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('空域名称'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusInput {
            id: nameInput
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight

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
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('空域类型'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusSelect {
            id: typeSelect
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight

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
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('最低高度(m)'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusInput {
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight

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
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('最高高度(m)'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusInput {
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight

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
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('生效时间'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusInput {
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight

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
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('失效时间'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor }
        HusInput {
            Layout.fillWidth: true
            Layout.preferredHeight: root.controlHeight

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
        Layout.fillWidth: true
        spacing: 10
        HusText { text: qsTr('备注'); Layout.preferredWidth: root.labelWidth; font.pixelSize: 12; color: root.textColor; Layout.alignment: Qt.AlignTop | Qt.AlignLeft }
        HusTextArea {
            Layout.fillWidth: true
            Layout.preferredHeight: 92
            placeholderText: qsTr('可选')
            text: root.remarks
            onTextChanged: {
                root.remarks = text
                root.dataChanged()
            }
        }
    }
}
