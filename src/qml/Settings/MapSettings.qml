import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

import YEFSApp

/**
 * 地图设置分类组件
 */
Column {
    id: root
    width: parent ? parent.width : 400
    spacing: 10

    // 地图服务提供商列表
    property var providerList: [
        { name: "OpenFreeMap", value: "openfreemap" },
        { name: "MapTiler", value: "maptiler" },
        { name: "自定义", value: "custom" }
    ]

    // 当前选中的提供商索引
    property int currentProviderIndex: {
        var saved = SettingsManager.getValue("map", "provider", "openfreemap");
        for (var i = 0; i < providerList.length; i++) {
            if (providerList[i].value === saved) return i;
        }
        return 0;
    }

    HusText {
        text: qsTr('地图设置')
        font.weight: Font.DemiBold
    }

    Rectangle {
        width: parent.width
        height: mapContent.height + 40
        radius: 6
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.6)
        border.color: HusTheme.Primary.colorFillPrimary

        Column {
            id: mapContent
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: 15

            Row {
                height: 36
                spacing: 20

                HusText {
                    width: 100
                    height: parent.height
                    text: qsTr('地图服务')
                    verticalAlignment: Text.AlignVCenter
                }

                HusSelect {
                    id: providerSelect
                    width: 200
                    anchors.verticalCenter: parent.verticalCenter

                    model: ListModel {
                        id: providerModel
                        ListElement { label: "OpenFreeMap"; value: "openfreemap" }
                        ListElement { label: "MapTiler"; value: "maptiler" }
                        ListElement { label: "自定义"; value: "custom" }
                    }

                    currentIndex: root.currentProviderIndex
                    
                    Component.onCompleted: completed = true
                    property bool completed: false

                    onCurrentIndexChanged: {
                        if (completed && currentIndex >= 0 && currentIndex < providerModel.count) {
                            var item = providerModel.get(currentIndex);
                            SettingsManager.setValue("map", "provider", item.value);
                            updateMapStyleUrl();
                        }
                    }
                }
            }

            // API Key 输入行 - 仅当选择 MapTiler 时显示
            Row {
                height: 36
                spacing: 20
                visible: providerSelect.currentIndex === 1

                HusText {
                    width: 100
                    height: parent.height
                    text: qsTr('API Key')
                    verticalAlignment: Text.AlignVCenter
                }

                HusInput {
                    id: apiKeyInput
                    width: 300
                    anchors.verticalCenter: parent.verticalCenter
                    placeholderText: qsTr('请输入 MapTiler API Key')
                    Component.onCompleted: {
                        text = SettingsManager.getValue("map", "apiKey", "");
                        completed = true;
                    }
                    property bool completed: false
                    onTextChanged: {
                        if (completed) {
                            SettingsManager.setValue("map", "apiKey", text);
                            updateMapStyleUrl();
                        }
                    }
                }
            }

            // 自定义 URL 输入行 - 仅当选择自定义时显示
            Row {
                height: 36
                spacing: 20
                visible: providerSelect.currentIndex === 2

                HusText {
                    width: 100
                    height: parent.height
                    text: qsTr('样式 URL')
                    verticalAlignment: Text.AlignVCenter
                }

                HusInput {
                    id: customUrlInput
                    width: 400
                    anchors.verticalCenter: parent.verticalCenter
                    placeholderText: qsTr('请输入自定义地图样式 URL')
                    Component.onCompleted: {
                        text = SettingsManager.getValue("map", "customStyleUrl", "");
                        completed = true;
                    }
                    property bool completed: false
                    onTextChanged: {
                        if (completed) {
                            SettingsManager.setValue("map", "customStyleUrl", text);
                            updateMapStyleUrl();
                        }
                    }
                }
            }
        }
    }

    function updateMapStyleUrl() {
        var provider = "";
        if (providerSelect.currentIndex >= 0 && providerSelect.currentIndex < providerModel.count) {
            provider = providerModel.get(providerSelect.currentIndex).value;
        } else {
            provider = "openfreemap";
        }
        
        var styleUrl = "";
        
        if (provider === "openfreemap") {
            styleUrl = "https://tiles.openfreemap.org/styles/liberty";
        } else if (provider === "maptiler") {
            var apiKey = SettingsManager.getValue("map", "apiKey", "");
            if (apiKey && apiKey.length > 0) {
                styleUrl = "https://api.maptiler.com/maps/streets-v2/style.json?key=" + apiKey;
            } else {
                styleUrl = "";
            }
        } else if (provider === "custom") {
            styleUrl = SettingsManager.getValue("map", "customStyleUrl", "");
        }
        
        SettingsManager.setValue("map", "styleUrl", styleUrl);
    }
}
