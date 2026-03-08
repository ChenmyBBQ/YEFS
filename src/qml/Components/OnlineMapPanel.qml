import QtQuick 2.15
import HuskarUI.Basic 1.0

import YEFSApp

Rectangle {
    id: root
    width: 340
    height: parent ? parent.height : 600
    radius: 8
    clip: true
    color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgContainer, 0.68)

    readonly property string defaultMapTilerKey: "pIYKyqRw5KwCNhksntqa"
    property var categoryDefs: [
        { "key": "MapTiler", "label": "MapTiler", "supportsKey": true, "supportsSecret": false },
        { "key": "Google", "label": "Google 地图", "supportsKey": false, "supportsSecret": false },
        { "key": "Bing", "label": "Bing 地图", "supportsKey": true, "supportsSecret": false },
        { "key": "高德", "label": "高德地图", "supportsKey": true, "supportsSecret": true }
    ]

    property var draftKeys: ({})
    property var draftSecrets: ({})
    property bool settingsVisible: false

    function setDraftKey(categoryKey, value) {
        var data = root.draftKeys
        data[categoryKey] = value
        root.draftKeys = data
    }

    function setDraftSecret(categoryKey, value) {
        var data = root.draftSecrets
        data[categoryKey] = value
        root.draftSecrets = data
    }

    function resetSettings() {
        MapSettings.reset()
        root.openSettings()
        toast.success(qsTr('已恢复默认设置'))
    }

    function switchProvider(providerIndex) {
        if (!root.canSwitchProvider(providerIndex)) {
            var provider = MapSettings.providers[providerIndex]
            var category = provider ? provider.category : ''
            if (category === 'Bing')
                toast.error(qsTr('Bing 地图需要有效 Key，请先在设置页输入。'))
            else if (category === '高德')
                toast.error(qsTr('高德地图需要有效 Key 和安全密钥，请先在设置页输入。'))
            else
                toast.error(qsTr('当前底图不可用'))
            return
        }

        MapSettings.currentProviderIndex = providerIndex
        toast.success(qsTr('已切换到底图：') + MapSettings.getProviderName(providerIndex))
    }

    function providerIndicesByCategory(categoryKey) {
        var indices = []
        if (!MapSettings.isCategoryVisible(categoryKey)) {
            return indices
        }
        var list = MapSettings.providers
        for (var i = 0; i < list.length; ++i) {
            var provider = list[i]
            if (provider && provider.category === categoryKey) {
                indices.push(i)
            }
        }
        return indices
    }

    function canSwitchProvider(providerIndex) {
        var p = MapSettings.providers[providerIndex]
        if (!p) return false
        if (p.category === "Bing") {
            var bingKey = MapSettings.getSourceKey("Bing")
            if (!bingKey || bingKey.length < 8)
                return false
        } else if (p.category === "高德") {
            var gaodeKey = MapSettings.getSourceKey("高德")
            var gaodeSecret = MapSettings.getSourceSecret("高德")
            if (!gaodeKey || gaodeKey.length < 8 || !gaodeSecret || gaodeSecret.length < 8)
                return false
        }
        return true
    }

    function openSettings() {
        var data = {}
        var secretData = {}
        for (var i = 0; i < root.categoryDefs.length; ++i) {
            var c = root.categoryDefs[i]
            data[c.key] = MapSettings.getSourceKey(c.key)
            secretData[c.key] = MapSettings.getSourceSecret(c.key)
        }
        root.draftKeys = data
        root.draftSecrets = secretData
        root.settingsVisible = true
    }

    function closeSettingsAndApply() {
        for (var i = 0; i < root.categoryDefs.length; ++i) {
            var c = root.categoryDefs[i]
            if (c.supportsKey) {
                MapSettings.setSourceKey(c.key, root.draftKeys[c.key] || "")
            }
            if (c.supportsSecret) {
                MapSettings.setSourceSecret(c.key, root.draftSecrets[c.key] || "")
            }
        }
        root.settingsVisible = false
        toast.success(qsTr("设置已更新"))
    }

    HusMessage {
        id: toast
        anchors.fill: parent
        z: 999
    }

    OnlineMapProviderList {
        anchors.fill: parent
        categoryDefs: root.categoryDefs
        settingsVisible: root.settingsVisible
        currentProviderIndex: MapSettings.currentProviderIndex
        providerIndicesResolver: root.providerIndicesByCategory
        onOpenSettingsRequested: root.openSettings()
        onProviderSwitchRequested: providerIndex => root.switchProvider(providerIndex)
    }

    OnlineMapSettingsPanel {
        anchors.fill: parent
        categoryDefs: root.categoryDefs
        draftKeys: root.draftKeys
        draftSecrets: root.draftSecrets
        settingsVisible: root.settingsVisible
        onApplyRequested: root.closeSettingsAndApply()
        onResetRequested: root.resetSettings()
        onDraftKeyChanged: (categoryKey, value) => root.setDraftKey(categoryKey, value)
        onDraftSecretChanged: (categoryKey, value) => root.setDraftSecret(categoryKey, value)
    }
}
