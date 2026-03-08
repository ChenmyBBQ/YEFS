import QtQuick
import HuskarUI.Basic

Column {
    id: root

    property int menuWidth: 0
    property int compactMode: HusMenu.Mode_Relaxed
    property int defaultMenuIconSize: 0

    signal cycleCompactModeRequested()
    signal aboutRequested()
    signal settingsRequested()

    width: menuWidth

    HusIconButton {
        id: navModeButton
        width: parent.width
        height: 40
        type: HusButton.Type_Text
        radiusBg.all: 0
        text: root.compactMode !== HusMenu.Mode_Relaxed ? '' : qsTr('导航模式')
        colorText: HusTheme.Primary.colorTextBase
        iconSize: root.defaultMenuIconSize
        iconSource: HusIcon.MenuOutlined
        onClicked: root.cycleCompactModeRequested()

        HusToolTip {
            visible: navModeButton.hovered
            showArrow: true
            text: {
                switch (root.compactMode) {
                case HusMenu.Mode_Relaxed: return qsTr('当前: 宽松模式 (点击切换)')
                case HusMenu.Mode_Standard: return qsTr('当前: 标准模式 (点击切换)')
                case HusMenu.Mode_Compact: return qsTr('当前: 紧凑模式 (点击切换)')
                default: return qsTr('导航模式')
                }
            }
        }
    }

    HusIconButton {
        id: aboutButton
        width: parent.width
        height: 40
        type: HusButton.Type_Text
        radiusBg.all: 0
        text: root.compactMode !== HusMenu.Mode_Relaxed ? '' : qsTr('关于')
        colorText: HusTheme.Primary.colorTextBase
        iconSize: root.defaultMenuIconSize
        iconSource: HusIcon.InfoCircleOutlined
        onClicked: root.aboutRequested()

        HusToolTip {
            visible: aboutButton.hovered
            showArrow: true
            text: qsTr('关于 YEFS')
        }
    }

    HusIconButton {
        id: settingsButton
        width: parent.width
        height: 40
        type: HusButton.Type_Text
        radiusBg.all: 0
        text: root.compactMode !== HusMenu.Mode_Relaxed ? '' : qsTr('设置')
        colorText: HusTheme.Primary.colorTextBase
        iconSize: root.defaultMenuIconSize
        iconSource: HusIcon.SettingOutlined
        onClicked: root.settingsRequested()

        HusToolTip {
            visible: settingsButton.hovered
            showArrow: true
            text: qsTr('设置')
        }
    }
}