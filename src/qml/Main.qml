pragma ComponentBehavior: Bound

import QtQuick
// removed unused imports: QtQuick.Layouts, QtQuick.Controls.Basic
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
    captionBar.color: HusTheme.Primary.colorBgContainer
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
            target: captionBar
            function onWindowAgentChanged() {
                captionBar.addInteractionItem(themeButton);
                captionBar.addInteractionItem(topButton);
                captionBar.addInteractionItem(themeColorCaptionButton);
            }
        }

        HusCaptionButton {
            id: themeColorCaptionButton
            height: parent.height
            noDisabledState: true
            iconSource: HusIcon.BgColorsOutlined
            iconSize: 14
            contentDescription: qsTr('主题色')
            onClicked: themeColorPopup.open()

            // 当前主题色指示器
            Rectangle {
                width: 6
                height: 6
                radius: 3
                anchors.right: parent.right
                anchors.rightMargin: 6
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 6
                color: HusTheme.Primary.colorPrimary
                border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorTextBase, 0.3)
                border.width: 1
            }

            HusPopup {
                id: themeColorPopup
                width: 280
                height: themeColorContent.height + 30
                y: themeColorCaptionButton.height + 2

                Column {
                    id: themeColorContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 15
                    spacing: 12

                    HusText {
                        text: qsTr('主题色')
                        font.weight: Font.DemiBold
                        font.pixelSize: HusTheme.Primary.fontPrimarySizeHeading5
                    }

                    // 预设颜色网格
                    Grid {
                        columns: 7
                        spacing: 8

                        HusColorGenerator { id: husColorGenerator }

                        Repeater {
                            id: presetRepeater
                            model: [
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Red), name: qsTr('红') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Volcano), name: qsTr('火山') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Orange), name: qsTr('橙') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Gold), name: qsTr('金') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Yellow), name: qsTr('黄') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Lime), name: qsTr('绿黄') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Green), name: qsTr('绿') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Cyan), name: qsTr('青') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Blue), name: qsTr('蓝') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Geekblue), name: qsTr('极客蓝') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Purple), name: qsTr('紫') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Magenta), name: qsTr('洋红') },
                                { color: husColorGenerator.presetToColor(HusColorGenerator.Preset_Grey), name: qsTr('灰') }
                            ]
                            delegate: Rectangle {
                                id: presetItem
                                required property var modelData
                                required property int index
                                width: 28
                                height: 28
                                radius: HusTheme.Primary.radiusPrimary
                                scale: presetHover.hovered ? 1.15 : 1
                                color: presetHover.hovered ? HusThemeFunctions.lighter(presetItem.modelData.color, 110) : presetItem.modelData.color
                                border.color: presetRepeater.currentIndex === presetItem.index || presetHover.hovered
                                              ? HusTheme.Primary.colorPrimaryBorderHover : 'transparent'
                                border.width: 1

                                Behavior on color { ColorAnimation { duration: HusTheme.Primary.durationFast } }
                                Behavior on scale {
                                    NumberAnimation {
                                        easing.type: Easing.OutBack
                                        duration: HusTheme.Primary.durationSlow
                                    }
                                }

                                HusIconText {
                                    anchors.centerIn: parent
                                    iconSource: HusIcon.CheckOutlined
                                    iconSize: 14
                                    colorIcon: 'white'
                                    visible: presetRepeater.currentIndex === presetItem.index
                                }

                                HoverHandler { id: presetHover }

                                TapHandler {
                                    onTapped: {
                                        presetRepeater.currentIndex = presetItem.index;
                                        HusTheme.installThemePrimaryColorBase(presetItem.modelData.color);
                                    }
                                }

                                HusToolTip {
                                    visible: presetHover.hovered
                                    text: presetItem.modelData.name
                                    delay: 300
                                }
                            }
                            property int currentIndex: -1
                        }
                    }

                    HusDivider { width: parent.width }

                    // 自定义颜色选择
                    Row {
                        spacing: 10
                        HusText {
                            text: qsTr('自定义:')
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        HusColorPicker {
                            id: customThemeColorPicker
                            defaultValue: HusTheme.Primary.colorPrimary
                            showText: true
                            autoChange: false
                            onChange: color => selectColor = color;
                            popup.closePolicy: HusPopup.NoAutoClose
                            property color selectColor: value
                            footerDelegate: Item {
                                height: 45
                                HusDivider {
                                    width: parent.width - 24
                                    height: 1
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                                Row {
                                    spacing: 20
                                    anchors.centerIn: parent
                                    HusButton {
                                        text: qsTr('确定')
                                        onClicked: {
                                            customThemeColorPicker.changeValue = customThemeColorPicker.selectColor;
                                            customThemeColorPicker.open = false;
                                            presetRepeater.currentIndex = -1;
                                            HusTheme.installThemePrimaryColorBase(customThemeColorPicker.selectColor);
                                        }
                                    }
                                    HusButton {
                                        text: qsTr('取消')
                                        type: HusButton.Type_Default
                                        onClicked: {
                                            customThemeColorPicker.changeValue = customThemeColorPicker.value;
                                            customThemeColorPicker.defaultValue = customThemeColorPicker.value;
                                            customThemeColorPicker.open = false;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        HusCaptionButton {
            id: themeButton
            height: parent.height
            noDisabledState: true
            iconSource: HusTheme.isDark ? HusIcon.MoonOutlined : HusIcon.SunOutlined
            iconSize: 14
            contentDescription: qsTr('明暗主题切换')
            onClicked: captionBar.themeCallback();
        }

        HusCaptionButton {
            id: topButton
            height: parent.height
            noDisabledState: true
            iconSource: HusIcon.PushpinOutlined
            iconSize: 14
            checkable: true
            checked: captionBar.topButtonChecked
            contentDescription: qsTr('置顶')
            onClicked: captionBar.topCallback(checked);
        }
    }

    // 确保窗口关闭时完全退出应用程序
    onClosing: (close) => {
        Qt.quit()
    }

    Component.onCompleted: {
        // 直接设置窗口底色，不启用系统特效，确保所有区域颜色一致
        mainWindow.color = HusTheme.Primary.colorBgContainer;
    }

    // 主题切换时同步更新窗口底色
    Connections {
        target: HusTheme
        function onIsDarkChanged() {
            mainWindow.color = HusTheme.Primary.colorBgContainer;
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

    Item {
        id: content
        anchors.top: mainWindow.captionBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

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
        Rectangle {
            id: authorCard
            width: visible ? yefsMenu.defaultMenuWidth : 0
            height: visible ? 80 : 0
            anchors.top: parent.top
            anchors.topMargin: 5
            radius: HusTheme.Primary.radiusPrimary
            color: hovered ? HusThemeFunctions.alpha(HusTheme.Primary.colorTextBase, 0.06) : 'transparent'
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
            anchors.bottomMargin: 8  // 增加底部间距
            showEdge: false          // 禁用自带边框，使用统一的分割线
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

        // 底部按钮列
        Column {
            id: buttonsColumn
            width: yefsMenu.width
            anchors.bottom: parent.bottom

            HusIconButton {
                id: navModeButton
                width: parent.width
                height: 40
                type: HusButton.Type_Text
                radiusBg.all: 0
                text: yefsMenu.compactMode !== HusMenu.Mode_Relaxed ? '' : qsTr('导航模式')
                colorText: HusTheme.Primary.colorTextBase
                iconSize: yefsMenu.defaultMenuIconSize
                iconSource: HusIcon.MenuOutlined
                onClicked: {
                    // 循环切换：宽松 → 标准 → 紧凑 → 宽松
                    if (yefsMenu.compactMode === HusMenu.Mode_Relaxed) {
                        yefsMenu.compactMode = HusMenu.Mode_Standard;
                    } else if (yefsMenu.compactMode === HusMenu.Mode_Standard) {
                        yefsMenu.compactMode = HusMenu.Mode_Compact;
                    } else {
                        yefsMenu.compactMode = HusMenu.Mode_Relaxed;
                    }
                }

                HusToolTip {
                    visible: parent.hovered
                    showArrow: true
                    text: {
                        switch (yefsMenu.compactMode) {
                        case HusMenu.Mode_Relaxed: return qsTr('当前: 宽松模式 (点击切换)');
                        case HusMenu.Mode_Standard: return qsTr('当前: 标准模式 (点击切换)');
                        case HusMenu.Mode_Compact: return qsTr('当前: 紧凑模式 (点击切换)');
                        default: return qsTr('导航模式');
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
