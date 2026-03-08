pragma ComponentBehavior: Bound

import QtQuick
// removed unused imports: QtQuick.Layouts, QtQuick.Controls.Basic
import HuskarUI.Basic

import YEFSApp

import './Home'
import './Components'

HusWindow {
    id: mainWindow
    property bool startupCoverDismissed: false
    property color startupCoverColor: "#b8d9f0"

    function cycleMenuCompactMode() {
        if (yefsMenu.compactMode === HusMenu.Mode_Relaxed) {
            yefsMenu.compactMode = HusMenu.Mode_Standard
        } else if (yefsMenu.compactMode === HusMenu.Mode_Standard) {
            yefsMenu.compactMode = HusMenu.Mode_Compact
        } else {
            yefsMenu.compactMode = HusMenu.Mode_Relaxed
        }
    }

    function toggleAboutWindow() {
        if (!mainWindow.aboutWindow) {
            mainWindow.aboutWindow = aboutWindowComponent.createObject(null)
        }

        mainWindow.aboutWindow.visible = !mainWindow.aboutWindow.visible
        if (mainWindow.aboutWindow.visible) {
            mainWindow.aboutWindow.raise()
            mainWindow.aboutWindow.requestActivate()
        }
    }

    function toggleSettingsPanel() {
        console.log("[Settings] Button clicked, active:", settingsLoader.active, "visible:", settingsLoader.visible)
        if (!settingsLoader.active)
            settingsLoader.active = true
        settingsLoader.visible = !settingsLoader.visible
        console.log("[Settings] After toggle, visible:", settingsLoader.visible)
    }

    function handleSearchSelection(option) {
        yefsMenu.gotoMenu(option.key)
    }

    function handleMenuLeafClick(key, data) {
        if (!data || data.hasOwnProperty('menuChildren'))
            return

        containerLoader.source = data.source
        containerLoader.visible = true

        console.debug('onClickMenu', key, JSON.stringify(data))

        if (key === 'NewAirspace') {
            MessageBus.send('airspace-manager/new', {})
        } else if (key === 'AirspaceManager') {
            MessageBus.send('airspace-manager/show', {})
        } else {
            MessageBus.send('airspace-manager/hide', {})
        }
    }

    width: 1400
    height: 900
    minimumWidth: 800
    minimumHeight: 600
    title: qsTr('YEFS - GIS Platform')
    followThemeSwitch: true
    color: startupCoverDismissed ? HusTheme.Primary.colorBgContainer : startupCoverColor
    captionBar.visible: Qt.platform.os === 'windows' || Qt.platform.os === 'linux' || Qt.platform.os === 'osx'
    captionBar.height: captionBar.visible ? 30 : 0
    captionBar.color: startupCoverDismissed ? HusTheme.Primary.colorBgContainer : startupCoverColor
    captionBar.showWinIcon: Qt.platform.os !== 'osx'
    captionBar.winIconWidth: 22
    captionBar.winIconHeight: 22
    captionBar.winIconDelegate: Item {
        Image {
            width: 16
            height: 16
            anchors.centerIn: parent
            source: 'qrc:/YEFSApp/resources/images/YEFS.svg'
        }
    }
    captionBar.themeCallback: () => {
        HusTheme.darkMode = HusTheme.isDark ? HusTheme.Light : HusTheme.Dark;
    }
    captionBar.topCallback: (checked) => {
        HusApi.setWindowStaysOnTopHint(mainWindow, checked);
    }

    captionBar.winPresetButtonsDelegate: Row {
        Connections {
            target: mainWindow.captionBar
            function onWindowAgentChanged() {
                mainWindow.captionBar.addInteractionItem(themeButton);
                mainWindow.captionBar.addInteractionItem(topButton);
                mainWindow.captionBar.addInteractionItem(themeColorCaptionButton);
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
            onClicked: mainWindow.captionBar.themeCallback(); // qmllint disable missing-property
        }

        HusCaptionButton {
            id: topButton
            height: parent.height
            noDisabledState: true
            iconSource: HusIcon.PushpinOutlined
            iconSize: 14
            checkable: true
            checked: mainWindow.captionBar.topButtonChecked // qmllint disable missing-property
            contentDescription: qsTr('置顶')
            onClicked: mainWindow.captionBar.topCallback(checked); // qmllint disable missing-property
        }
    }

    // 确保窗口关闭时完全退出应用程序
    onClosing: (close) => {
        console.log("[Main] Window closing, calling Qt.quit()");
        Qt.quit()
    }

    property var yefsGlobal: YefsGlobal { }
    property var aboutWindow: null

    Component {
        id: aboutWindowComponent
        AboutPage {
            visible: false
            onCloseRequested: visible = false
        }
    }

    Item {
        id: content
        anchors.top: mainWindow.captionBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        opacity: 0

        Behavior on opacity { NumberAnimation { duration: 300 } }

        Timer {
            running: true
            interval: 200
            onTriggered: content.opacity = 1
        }

        // 内容区背景（参考设置页：左侧导航/右侧内容）
        Rectangle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: yefsMenu.width
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgElevated, 0.8)
        }

        Rectangle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: yefsMenu.right
            anchors.right: parent.right
            color: HusTheme.Primary.colorBgContainer
        }

        Rectangle {
            width: 1
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: yefsMenu.right
            color: HusTheme.Primary.colorBorder
        }

        // 作者卡片区域
        SidebarBrandCard {
            id: authorCard
            anchors.top: parent.top
            anchors.topMargin: 5
            expandedWidth: yefsMenu.defaultMenuWidth
            compactMode: yefsMenu.compactMode
        }

        SidebarSearchPanel {
            id: searchPanel
            compactMode: yefsMenu.compactMode
            defaultMenuWidth: yefsMenu.defaultMenuWidth
            compactWidth: yefsMenu.compactWidth
            options: yefsGlobal.options
            topTarget: authorCard
            onOptionSelected: mainWindow.handleSearchSelection(option)
        }

        // 左侧菜单
        SidebarMenuPanel {
            id: yefsMenu
            anchors.left: parent.left
            anchors.top: searchPanel.bottom
            anchors.bottom: buttonsColumn.top
            anchors.bottomMargin: 8  // 增加底部间距
            showEdge: false          // 禁用自带边框，使用统一的分割线
            showToolTip: true
            defaultMenuWidth: 260
            defaultSelectedKey: ['MapPage']
            initModel: yefsGlobal.menus
            onLeafTriggered: mainWindow.handleMenuLeafClick(key, data)
        }

        // 底部按钮列
        SidebarUtilityButtons {
            id: buttonsColumn
            anchors.bottom: parent.bottom
            menuWidth: yefsMenu.width
            compactMode: yefsMenu.compactMode
            defaultMenuIconSize: yefsMenu.defaultMenuIconSize
            onCycleCompactModeRequested: mainWindow.cycleMenuCompactMode()
            onAboutRequested: mainWindow.toggleAboutWindow()
            onSettingsRequested: mainWindow.toggleSettingsPanel()
        }

        // 主内容区域
        Item {
            id: container
            anchors.left: yefsMenu.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 0
            clip: true

            Loader {
                id: containerLoader
                anchors.fill: parent
                visible: !settingsLoader.visible
                source: './Home/MapPage.qml'
            }

            // 设置页面 Loader
            Loader {
                id: settingsLoader
                anchors.fill: parent
                active: false
                visible: false
                source: "./Home/SettingsPage.qml"

                onStatusChanged: {
                    if (status === Loader.Error) {
                        console.error("[Settings] Loader error:", errorString());
                    } else {
                        console.log("[Settings] Loader status:", status, "active:", active, "visible:", visible);
                    }
                }
                
                Connections {
                    target: settingsLoader.item
                    function onCloseRequested() { settingsLoader.visible = false }
                    function onApplyRequested() { console.log("[Settings] Apply requested") }
                }
            }

            Rectangle {
                anchors.centerIn: parent
                width: Math.min(parent.width - 40, 700)
                height: settingsErrorText.implicitHeight + 24
                radius: HusTheme.Primary.radiusPrimary
                color: HusThemeFunctions.alpha(HusTheme.Primary.colorError, 0.08)
                border.color: HusTheme.Primary.colorError
                visible: settingsLoader.visible && settingsLoader.status === Loader.Error
                z: 999

                HusText {
                    id: settingsErrorText
                    anchors.centerIn: parent
                    width: parent.width - 24
                    wrapMode: HusText.WrapAnywhere
                    color: "red" // Force red color to ensure visibility
                    text: {
                        if (settingsLoader.status === Loader.Error)
                            return qsTr("设置页面加载失败：请查看控制台日志。");
                        return "";
                    }
                }
            }

            Rectangle {
                id: startupCover
                anchors.fill: parent
                z: 10000
                color: mainWindow.startupCoverColor
                visible: opacity > 0.01
                opacity: mainWindow.startupCoverDismissed ? 0.0 : 1.0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 180
                        easing.type: Easing.OutCubic
                    }
                }
            }

        }
    }

    // 消息总线连接
    Connections {
        target: MessageBus

        function onMessage(topic, data) {
            if (topic === "map/startup-visual-ready" && !mainWindow.startupCoverDismissed) {
                mainWindow.startupCoverDismissed = true
            }

            console.log("[Main] Message:", topic, JSON.stringify(data))
        }
    }
}
