import QtQuick
import HuskarUI.Basic

Row {
    id: root

    property var captionBarHost: null

    Connections {
        target: root.captionBarHost

        function onWindowAgentChanged() {
            if (!root.captionBarHost)
                return

            root.captionBarHost.addInteractionItem(themeButton)
            root.captionBarHost.addInteractionItem(topButton)
            root.captionBarHost.addInteractionItem(themeColorCaptionButton)
        }
    }

    ThemeColorPickerBtn {
        id: themeColorCaptionButton
    }

    HusCaptionButton {
        id: themeButton
        height: parent.height
        noDisabledState: true
        iconSource: HusTheme.isDark ? HusIcon.MoonOutlined : HusIcon.SunOutlined
        iconSize: 14
        contentDescription: qsTr('明暗主题切换')
        onClicked: root.captionBarHost.themeCallback() // qmllint disable missing-property
    }

    HusCaptionButton {
        id: topButton
        height: parent.height
        noDisabledState: true
        iconSource: HusIcon.PushpinOutlined
        iconSize: 14
        checkable: true
        checked: root.captionBarHost.topButtonChecked // qmllint disable missing-property
        contentDescription: qsTr('置顶')
        onClicked: root.captionBarHost.topCallback(checked) // qmllint disable missing-property
    }
}