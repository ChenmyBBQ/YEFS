import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic

import YEFSApp

/**
 * 单位设置分类组件
 * 支持航空任务规划相关的公制/英制单位切换
 */
Column {
    id: root
    width: parent ? parent.width : 400
    spacing: 16

    // 单位分类配置
    readonly property var unitCategories: [
        { id: "altitude", name: qsTr("高度"), desc: qsTr("飞行高度、海拔高度") },
        { id: "distance", name: qsTr("距离"), desc: qsTr("航段距离、航程") },
        { id: "speed", name: qsTr("速度"), desc: qsTr("空速、地速、风速") },
        { id: "verticalSpeed", name: qsTr("垂直速度"), desc: qsTr("爬升率、下降率") },
        { id: "pressure", name: qsTr("气压"), desc: qsTr("QNH、高度表设定") },
        { id: "temperature", name: qsTr("温度"), desc: qsTr("气温、露点") },
        { id: "weight", name: qsTr("重量"), desc: qsTr("载重、燃油重量") },
        { id: "volume", name: qsTr("体积"), desc: qsTr("燃油体积") },
        { id: "visibility", name: qsTr("能见度"), desc: qsTr("水平能见度、RVR") }
    ]

    HusText {
        text: qsTr('单位设置')
        font.weight: Font.DemiBold
    }

    // 快捷预设区域
    Rectangle {
        width: parent.width
        height: presetContent.height + 40
        radius: 6
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.6)
        border.color: HusTheme.Primary.colorFillPrimary

        Column {
            id: presetContent
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12

            Row {
                spacing: 10

                HusText {
                    width: 80
                    height: 32
                    text: qsTr('快捷预设')
                    font.weight: Font.Medium
                    verticalAlignment: Text.AlignVCenter
                }

                HusTag {
                    text: UnitManager.presetMode === "aviation" ? qsTr("航空标准") :
                          UnitManager.presetMode === "metric" ? qsTr("公制") :
                          UnitManager.presetMode === "imperial" ? qsTr("英制") : qsTr("自定义")
                    presetColor: UnitManager.presetMode === "custom" ? 
                           HusTheme.Primary.colorWarning : HusTheme.Primary.colorPrimary
                }
            }

            Row {
                spacing: 12

                HusButton {
                    text: qsTr('航空标准')
                    type: UnitManager.presetMode === "aviation" ? HusButton.Type_Primary : HusButton.Type_Default
                    onClicked: UnitManager.applyPreset("aviation")

                    HusToolTip {
                        text: qsTr("ft / NM / kt / hPa / °C\n国际民航组织(ICAO)通用标准")
                    }
                }

                HusButton {
                    text: qsTr('公制 (SI)')
                    type: UnitManager.presetMode === "metric" ? HusButton.Type_Primary : HusButton.Type_Default
                    onClicked: UnitManager.applyPreset("metric")

                    HusToolTip {
                        text: qsTr("m / km / km·h⁻¹ / hPa / °C\n国际单位制，无人机/测绘常用")
                    }
                }

                HusButton {
                    text: qsTr('英制')
                    type: UnitManager.presetMode === "imperial" ? HusButton.Type_Primary : HusButton.Type_Default
                    onClicked: UnitManager.applyPreset("imperial")

                    HusToolTip {
                        text: qsTr("ft / mi / mph / inHg / °F\n美国习惯单位制")
                    }
                }
            }
        }
    }

    // 分类详细设置区域
    Rectangle {
        width: parent.width
        height: detailContent.height + 40
        radius: 6
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorBgBase, 0.6)
        border.color: HusTheme.Primary.colorBorderSecondary

        Column {
            id: detailContent
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12

            HusText {
                text: qsTr('分类单位设置')
                font.weight: Font.Medium
                color: HusTheme.Primary.colorTextSecondary
            }

            // 使用 Repeater 生成每个分类的设置行
            Repeater {
                model: root.unitCategories

                Row {
                    width: detailContent.width
                    height: 40
                    spacing: 16

                    // 分类名称
                    Column {
                        width: 100
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        HusText {
                            text: modelData.name
                            font.weight: Font.Medium
                        }
                        HusText {
                            text: modelData.desc
                            font.pixelSize: 11
                            color: HusTheme.Primary.colorTextTertiary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    // 单位选择器
                    HusSelect {
                        id: unitSelect
                        width: 140
                        anchors.verticalCenter: parent.verticalCenter
                        
                        property string categoryId: modelData.id
                        property var availableUnits: UnitManager.getAvailableUnits(categoryId)
                        
                        model: availableUnits.map(u => ({
                            label: u.name + " (" + u.symbol + ")",
                            value: u.id
                        }))
                        
                        currentIndex: {
                            let currentUnit = getCurrentUnit(categoryId);
                            for (let i = 0; i < availableUnits.length; i++) {
                                if (availableUnits[i].id === currentUnit) return i;
                            }
                            return 0;
                        }
                        
                        onCurrentValueChanged: {
                            if (currentValue && currentValue !== getCurrentUnit(categoryId)) {
                                setCurrentUnit(categoryId, currentValue);
                            }
                        }
                        
                        function getCurrentUnit(cat) {
                            switch(cat) {
                                case "altitude": return UnitManager.altitudeUnit;
                                case "distance": return UnitManager.distanceUnit;
                                case "speed": return UnitManager.speedUnit;
                                case "verticalSpeed": return UnitManager.verticalSpeedUnit;
                                case "pressure": return UnitManager.pressureUnit;
                                case "temperature": return UnitManager.temperatureUnit;
                                case "weight": return UnitManager.weightUnit;
                                case "volume": return UnitManager.volumeUnit;
                                case "visibility": return UnitManager.visibilityUnit;
                            }
                            return "";
                        }
                        
                        function setCurrentUnit(cat, unit) {
                            switch(cat) {
                                case "altitude": UnitManager.altitudeUnit = unit; break;
                                case "distance": UnitManager.distanceUnit = unit; break;
                                case "speed": UnitManager.speedUnit = unit; break;
                                case "verticalSpeed": UnitManager.verticalSpeedUnit = unit; break;
                                case "pressure": UnitManager.pressureUnit = unit; break;
                                case "temperature": UnitManager.temperatureUnit = unit; break;
                                case "weight": UnitManager.weightUnit = unit; break;
                                case "volume": UnitManager.volumeUnit = unit; break;
                                case "visibility": UnitManager.visibilityUnit = unit; break;
                            }
                        }
                    }

                    // 当前值预览
                    HusText {
                        anchors.verticalCenter: parent.verticalCenter
                        text: getPreviewText(modelData.id)
                        color: HusTheme.Primary.colorTextTertiary
                        font.pixelSize: 12
                        
                        function getPreviewText(cat) {
                            // 显示一个示例值的换算结果
                            switch(cat) {
                                case "altitude": return qsTr("如: ") + UnitManager.formatAltitude(1000);
                                case "distance": return qsTr("如: ") + UnitManager.formatDistance(10000);
                                case "speed": return qsTr("如: ") + UnitManager.formatSpeed(50);
                                case "verticalSpeed": return qsTr("如: ") + UnitManager.formatVerticalSpeed(5);
                                case "pressure": return qsTr("如: ") + UnitManager.formatPressure(1013.25);
                                case "temperature": return qsTr("如: ") + UnitManager.formatTemperature(15);
                                case "weight": return qsTr("如: ") + UnitManager.formatWeight(25);
                                case "volume": return qsTr("如: ") + UnitManager.formatVolume(100);
                                case "visibility": return qsTr("如: ") + UnitManager.formatVisibility(10000);
                            }
                            return "";
                        }
                    }
                }
            }
        }
    }

    // 说明区域
    Rectangle {
        width: parent.width
        height: infoContent.height + 30
        radius: 6
        color: HusThemeFunctions.alpha(HusTheme.Primary.colorInfoBg, 0.5)
        border.color: HusTheme.Primary.colorInfoBorder

        Column {
            id: infoContent
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 16
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Row {
                spacing: 8
                HusIconText {
                    iconSource: HusIcon.InfoCircleOutlined
                    colorIcon: HusTheme.Primary.colorInfo
                    iconSize: 16
                }
                HusText {
                    text: qsTr("单位设置说明")
                    font.weight: Font.Medium
                    color: HusTheme.Primary.colorInfo
                }
            }

            HusText {
                width: parent.width
                text: qsTr("• 航空标准：国际民航组织(ICAO)推荐，高度用ft、距离用NM、速度用kt\n") +
                      qsTr("• 公制(SI)：国际单位制，适合无人机作业、测绘任务\n") +
                      qsTr("• 英制：美国习惯单位，包含华氏度和英寸汞柱气压\n") +
                      qsTr("• 内部计算统一使用SI单位，仅显示时进行换算")
                wrapMode: Text.WordWrap
                font.pixelSize: 12
                color: HusTheme.Primary.colorTextSecondary
                lineHeight: 1.4
            }
        }
    }
}
