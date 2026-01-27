import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

import YEFSApp
import "../Settings" as Settings

/**
 * 设置页面 - 左侧分类导航，右侧内容区域
 */
Rectangle {
    id: root
    color: HusTheme.Primary.colorBgContainer

    // 关闭信号
    signal closeRequested()
    signal applyRequested()

    onVisibleChanged: {
        if (!visible) {
            // 页面隐藏时关闭子页面弹窗
            dataSection.closeAllPopups()
        }
    }

    // 分类配置列表
    property var categories: [
        { id: "appearance", name: qsTr("外观"), icon: HusIcon.SkinOutlined },
        { id: "units", name: qsTr("单位"), icon: HusIcon.DashboardOutlined },
        { id: "map", name: qsTr("地图"), icon: HusIcon.EnvironmentOutlined },
        { id: "data", name: qsTr("数据"), icon: HusIcon.DatabaseOutlined }
    ]

    // 当前选中的分类索引
    property int currentCategoryIndex: 0

    // 是否正在程序滚动（避免循环触发）
    property bool isProgrammaticScroll: false

    Row {
        anchors.fill: parent
        spacing: 0

        // 左侧导航区域
        Rectangle {
            id: leftNav
            width: 140
            height: parent.height
            color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgElevated, 0.8)

            Column {
                anchors.fill: parent
                anchors.topMargin: 20
                spacing: 4

                Repeater {
                    model: root.categories

                    Rectangle {
                        id: navItem
                        width: leftNav.width - 16
                        height: 44
                        anchors.horizontalCenter: parent.horizontalCenter
                        radius: 6
                        color: root.currentCategoryIndex === index ? 
                               HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.15) : 
                               navItemMouse.containsMouse ? 
                               HusThemeFunctions.alpha(HusTheme.Primary.colorPrimary, 0.08) : 
                               "transparent"

                        Row {
                            anchors.centerIn: parent
                            spacing: 8

                            HusIconText {
                                anchors.verticalCenter: parent.verticalCenter
                                iconSize: 16
                                colorIcon: root.currentCategoryIndex === index ? 
                                           HusTheme.Primary.colorPrimary : 
                                           HusTheme.Primary.colorTextBase
                                iconSource: modelData.icon
                            }

                            HusText {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.name || ""
                                color: root.currentCategoryIndex === index ? 
                                       HusTheme.Primary.colorPrimary : 
                                       HusTheme.Primary.colorTextBase
                                font.weight: root.currentCategoryIndex === index ? 
                                             Font.DemiBold : Font.Normal
                            }
                        }

                        MouseArea {
                            id: navItemMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                scrollToCategory(index);
                            }
                        }
                    }
                }
            }
        }

        // 分隔线
        Rectangle {
            width: 1
            height: parent.height
            color: HusTheme.Primary.colorBorder
        }

        // 右侧内容区域
        Column {
            width: parent.width - leftNav.width - 1
            height: parent.height

            Flickable {
                id: contentFlickable
                width: parent.width
                height: parent.height - bottomButtons.height
                contentWidth: width
                contentHeight: contentColumn.height + 40
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                // 监听滚动位置变化
                onContentYChanged: {
                    if (!root.isProgrammaticScroll) {
                        updateCurrentCategory();
                    }
                }

                Column {
                    id: contentColumn
                    width: parent.width - 40
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 20
                    spacing: 30

                    // 外观设置
                    Settings.AppearanceSettings {
                        id: appearanceSection
                        width: parent.width
                    }

                    // 单位设置
                    Settings.UnitSettings {
                        id: unitsSection
                        width: parent.width
                    }

                    // 地图设置
                    Settings.MapSettings {
                        id: mapSection
                        width: parent.width
                    }

                    // 数据设置
                    Settings.DataSettings {
                        id: dataSection
                        width: parent.width
                    }
                }

                HusScrollBar.vertical: HusScrollBar {}
            }

            // 底部按钮区域
            Rectangle {
                id: bottomButtons
                width: parent.width
                height: 60
                color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgElevated, 0.9)

                Rectangle {
                    width: parent.width
                    height: 1
                    color: HusTheme.Primary.colorBorder
                    anchors.top: parent.top
                }

                Row {
                    anchors.right: parent.right
                    anchors.rightMargin: 20
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 15
                    layoutDirection: Qt.LeftToRight

                    HusButton {
                        text: qsTr('确定')
                        type: HusButton.Type_Primary
                        onClicked: {
                            root.applyRequested();
                            root.closeRequested();
                        }
                    }

                    HusButton {
                        text: qsTr('应用')
                        type: HusButton.Type_Default
                        onClicked: {
                            root.applyRequested();
                        }
                    }

                    HusButton {
                        text: qsTr('取消')
                        type: HusButton.Type_Text
                        onClicked: {
                            root.closeRequested();
                        }
                    }
                }
            }
        }
    }

    // 根据滚动位置更新当前分类
    function updateCurrentCategory() {
        var scrollY = contentFlickable.contentY;
        var sections = [appearanceSection, unitsSection, mapSection, dataSection];
        
        for (var i = sections.length - 1; i >= 0; i--) {
            var section = sections[i];
            // 计算 section 在 Column 中的 y 位置
            var sectionY = section.y;
            
            if (scrollY >= sectionY - 50) {
                if (root.currentCategoryIndex !== i) {
                    root.currentCategoryIndex = i;
                }
                return;
            }
        }
        
        // 默认选中第一个
        if (root.currentCategoryIndex !== 0) {
            root.currentCategoryIndex = 0;
        }
    }

    // 滚动到指定分类
    function scrollToCategory(categoryIndex) {
        var sections = [appearanceSection, unitsSection, mapSection, dataSection];
        
        if (categoryIndex >= 0 && categoryIndex < sections.length) {
            root.isProgrammaticScroll = true;
            root.currentCategoryIndex = categoryIndex;
            
            var targetY = sections[categoryIndex].y;
            
            // 使用动画滚动
            scrollAnimation.to = targetY;
            scrollAnimation.start();
        }
    }

    NumberAnimation {
        id: scrollAnimation
        target: contentFlickable
        property: "contentY"
        duration: 300
        easing.type: Easing.OutCubic
        onFinished: {
            root.isProgrammaticScroll = false;
        }
    }
}

