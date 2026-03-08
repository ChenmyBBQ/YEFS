import QtQuick
import HuskarUI.Basic

HusCaptionButton {
    id: root

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
        y: root.height + 2

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
