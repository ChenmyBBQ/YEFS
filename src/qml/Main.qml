pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import HuskarUI.Basic

import YEFSApp

import './Home'

HusWindow {
    id: mainWindow
    width: 1400
    height: 900
    opacity: 0
    minimumWidth: 800
    minimumHeight: 600
    title: qsTr('YEFS - GIS Platform')
    followThemeSwitch: true
    captionBar.visible: Qt.platform.os === 'windows' || Qt.platform.os === 'linux' || Qt.platform.os === 'osx'
    captionBar.height: captionBar.visible ? 30 : 0
    captionBar.color: HusTheme.Primary.colorFillTertiary
    captionBar.showThemeButton: true
    captionBar.showTopButton: true
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

    Component.onCompleted: {
        if (Qt.platform.os === 'windows') {
            if (setSpecialEffect(HusWindow.Win_MicaAlt)) return;
            if (setSpecialEffect(HusWindow.Win_Mica)) return;
            if (setSpecialEffect(HusWindow.Win_AcrylicMaterial)) return;
            if (setSpecialEffect(HusWindow.Win_DwmBlur)) return;
        } else if (Qt.platform.os === 'osx') {
            if (setSpecialEffect(HusWindow.Mac_BlurEffect)) return;
        }
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

    Behavior on opacity { NumberAnimation { } }

    Timer {
        running: true
        interval: 200
        onTriggered: {
            mainWindow.opacity = 1;
        }
    }

    Rectangle {
        id: mainBackground
        anchors.fill: content
        color: HusTheme.isDark ? '#181818' : '#f5f5f5'
        opacity: 0.2
    }

    Item {
        id: content
        anchors.top: mainWindow.captionBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        // 作者卡片区域
        Rectangle {
            id: authorCard
            width: visible ? yefsMenu.defaultMenuWidth : 0
            height: visible ? 80 : 0
            anchors.top: parent.top
            anchors.topMargin: 5
            radius: HusTheme.Primary.radiusPrimary
            color: hovered ? HusTheme.isDark ? '#10ffffff' : '#10000000' : 'transparent'
            visible: yefsMenu.compactMode === HusMenu.Mode_Relaxed
            clip: true

            property bool hovered: authorCardHover.hovered

            Behavior on height { NumberAnimation { duration: HusTheme.Primary.durationFast } }
            Behavior on color { ColorAnimation { duration: HusTheme.Primary.durationFast } }

            Item {
                height: parent.height
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 10

                HusImage {
                    id: logoIcon
                    width: 50
                    height: 50
                    anchors.verticalCenter: parent.verticalCenter
                    source: 'qrc:/YEFSApp/resources/images/YEFS.svg'
                }

                Column {
                    anchors.left: logoIcon.right
                    anchors.leftMargin: 10
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    HusText {
                        text: 'YEFS'
                        font.weight: Font.DemiBold
                        font.pixelSize: HusTheme.Primary.fontPrimarySize + 4
                    }

                    HusText {
                        text: qsTr('GIS Platform')
                        font.pixelSize: HusTheme.Primary.fontPrimarySize - 1
                        color: HusTheme.Primary.colorTextSecondary
                    }

                    HusText {
                        width: parent.width
                        text: qsTr('无人机地面站系统')
                        font.pixelSize: HusTheme.Primary.fontPrimarySize - 1
                        color: HusTheme.Primary.colorTextSecondary
                        wrapMode: HusText.WrapAnywhere
                    }
                }
            }

            HoverHandler {
                id: authorCardHover
            }
        }

        // 搜索组件
        HusAutoComplete {
            id: searchComponent
            property bool expanded: false
            z: 10
            clip: true
            width: (yefsMenu.compactMode === HusMenu.Mode_Relaxed || expanded) ? (yefsMenu.defaultMenuWidth - 20) : 0
            anchors.top: authorCard.bottom
            anchors.left: yefsMenu.compactMode === HusMenu.Mode_Relaxed ? yefsMenu.left : yefsMenu.right
            anchors.margins: 10
            topPadding: 6
            bottomPadding: 6
            rightPadding: 50
            showToolTip: true
            placeholderText: qsTr('搜索功能')
            iconSource: HusIcon.SearchOutlined
            colorBg: !(yefsMenu.compactMode === HusMenu.Mode_Relaxed) ? HusTheme.HusInput.colorBg : 'transparent'
            options: yefsGlobal.options
            filterOption: (input, option) => option.label.toUpperCase().indexOf(input.toUpperCase()) !== -1
            onSelect: option => yefsMenu.gotoMenu(option.key)

            Keys.onEscapePressed: {
                if (expanded) {
                    expanded = false;
                } else {
                    closePopup();
                }
            }

            Behavior on width {
                enabled: !(yefsMenu.compactMode === HusMenu.Mode_Relaxed) &&
                         yefsMenu.width === yefsMenu.compactWidth
                NumberAnimation { duration: HusTheme.Primary.durationFast }
            }
        }

        // 紧凑模式下的搜索按钮
        HusIconButton {
            id: searchCollapse
            visible: !(yefsMenu.compactMode === HusMenu.Mode_Relaxed)
            anchors.top: parent.top
            anchors.left: yefsMenu.left
            anchors.right: yefsMenu.right
            anchors.margins: 10
            type: HusButton.Type_Text
            colorText: HusTheme.Primary.colorTextBase
            iconSource: HusIcon.SearchOutlined
            iconSize: searchComponent.iconSize
            onClicked: {
                searchComponent.expanded = !searchComponent.expanded;
                if (searchComponent.expanded) {
                    searchComponent.forceActiveFocus();
                }
            }
            onVisibleChanged: {
                if (visible) {
                    searchComponent.closePopup();
                    searchComponent.expanded = false;
                }
            }
        }

        // 左侧菜单
        HusMenu {
            id: yefsMenu
            anchors.left: parent.left
            anchors.top: searchComponent.bottom
            anchors.bottom: buttonsColumn.top
            showEdge: true
            showToolTip: true
            defaultMenuWidth: 260
            defaultSelectedKey: ['MapPage']
            initModel: yefsGlobal.menus
            onClickMenu: function(deep, key, keyPath, data) {
                if (data) {
                    if (!data.hasOwnProperty('menuChildren')) {
                        containerLoader.source = data.source;
                        containerLoader.visible = true;
                        console.debug('onClickMenu', deep, key, keyPath, JSON.stringify(data));
                    }
                }
            }
        }

        HusDivider {
            width: yefsMenu.width
            height: 5
            anchors.bottom: buttonsColumn.top
        }

        // 底部按钮列
        Column {
            id: buttonsColumn
            width: yefsMenu.width
            anchors.bottom: parent.bottom

            HusIconButton {
                id: aboutButton
                width: parent.width
                height: 40
                type: HusButton.Type_Text
                radiusBg.all: 0
                text: yefsMenu.compactMode !== HusMenu.Mode_Relaxed ? '' : qsTr('关于')
                colorText: HusTheme.Primary.colorTextBase
                iconSize: yefsMenu.defaultMenuIconSize
                iconSource: HusIcon.InfoCircleOutlined
                onClicked: {
                    if (!aboutWindow) {
                        aboutWindow = aboutWindowComponent.createObject(null);
                    }
                    aboutWindow.visible = !aboutWindow.visible;
                    if (aboutWindow.visible) {
                        aboutWindow.raise();
                        aboutWindow.requestActivate();
                    }
                }

                HusToolTip {
                    visible: parent.hovered
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
                text: yefsMenu.compactMode !== HusMenu.Mode_Relaxed ? '' : qsTr('设置')
                colorText: HusTheme.Primary.colorTextBase
                iconSize: yefsMenu.defaultMenuIconSize
                iconSource: HusIcon.SettingOutlined
                onClicked: {
                    console.log("[Settings] Button clicked, active:", settingsLoader.active, "visible:", settingsLoader.visible);
                    if (!settingsLoader.active)
                        settingsLoader.active = true;
                    settingsLoader.visible = !settingsLoader.visible;
                    console.log("[Settings] After toggle, visible:", settingsLoader.visible);
                }

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    text: qsTr('设置')
                }
            }
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
                
                Connections {
                    target: settingsLoader.item
                    function onCloseRequested() { settingsLoader.visible = false }
                    function onApplyRequested() { console.log("[Settings] Apply requested") }
                }
            }

        }
    }

    // 消息总线连接
    Connections {
        target: MessageBus

        function onMessage(topic, data) {
            console.log("[Main] Message:", topic, JSON.stringify(data))
        }
    }
}
