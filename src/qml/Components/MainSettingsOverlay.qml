import QtQuick
import HuskarUI.Basic

Item {
    id: root

    property bool settingsVisible: false
    property bool loadedOnce: false
    property string source: ''

    signal closeRequested()
    signal applyRequested()

    onSettingsVisibleChanged: {
        if (settingsVisible)
            loadedOnce = true
    }

    Loader {
        id: settingsLoader
        anchors.fill: parent
        active: root.loadedOnce || root.settingsVisible
        visible: root.settingsVisible
        source: root.source

        onStatusChanged: {
            if (status === Loader.Error) {
                console.error('[Settings] Loader error:', errorString())
            } else {
                console.log('[Settings] Loader status:', status, 'active:', active, 'visible:', visible)
            }
        }

        Connections {
            target: settingsLoader.item

            function onCloseRequested() {
                root.settingsVisible = false
                root.closeRequested()
            }

            function onApplyRequested() {
                console.log('[Settings] Apply requested')
                root.applyRequested()
            }
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 40, 700)
        height: settingsErrorText.implicitHeight + 24
        radius: HusTheme.Primary.radiusPrimary
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorError, 0.08)
        border.color: HusTheme.Primary.colorError
        visible: root.settingsVisible && settingsLoader.status === Loader.Error
        z: 999

        HusText {
            id: settingsErrorText
            anchors.centerIn: parent
            width: parent.width - 24
            wrapMode: HusText.WrapAnywhere
            color: 'red'
            text: settingsLoader.status === Loader.Error
                ? qsTr('设置页面加载失败：请查看控制台日志。')
                : ''
        }
    }
}