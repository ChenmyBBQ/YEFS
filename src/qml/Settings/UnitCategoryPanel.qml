import QtQuick
import HuskarUI.Basic

import YEFSApp

Rectangle {
    id: root

    property var unitCategories: []

    function currentUnit(categoryId) {
        switch (categoryId) {
        case 'altitude':
            return UnitManager.altitudeUnit
        case 'distance':
            return UnitManager.distanceUnit
        case 'speed':
            return UnitManager.speedUnit
        case 'verticalSpeed':
            return UnitManager.verticalSpeedUnit
        case 'pressure':
            return UnitManager.pressureUnit
        case 'temperature':
            return UnitManager.temperatureUnit
        case 'weight':
            return UnitManager.weightUnit
        case 'volume':
            return UnitManager.volumeUnit
        case 'visibility':
            return UnitManager.visibilityUnit
        }
        return ''
    }

    function updateUnit(categoryId, unitId) {
        switch (categoryId) {
        case 'altitude':
            UnitManager.altitudeUnit = unitId
            break
        case 'distance':
            UnitManager.distanceUnit = unitId
            break
        case 'speed':
            UnitManager.speedUnit = unitId
            break
        case 'verticalSpeed':
            UnitManager.verticalSpeedUnit = unitId
            break
        case 'pressure':
            UnitManager.pressureUnit = unitId
            break
        case 'temperature':
            UnitManager.temperatureUnit = unitId
            break
        case 'weight':
            UnitManager.weightUnit = unitId
            break
        case 'volume':
            UnitManager.volumeUnit = unitId
            break
        case 'visibility':
            UnitManager.visibilityUnit = unitId
            break
        }
    }

    function previewText(categoryId) {
        switch (categoryId) {
        case 'altitude':
            return qsTr('如: ') + UnitManager.formatAltitude(1000)
        case 'distance':
            return qsTr('如: ') + UnitManager.formatDistance(10000)
        case 'speed':
            return qsTr('如: ') + UnitManager.formatSpeed(50)
        case 'verticalSpeed':
            return qsTr('如: ') + UnitManager.formatVerticalSpeed(5)
        case 'pressure':
            return qsTr('如: ') + UnitManager.formatPressure(1013.25)
        case 'temperature':
            return qsTr('如: ') + UnitManager.formatTemperature(15)
        case 'weight':
            return qsTr('如: ') + UnitManager.formatWeight(25)
        case 'volume':
            return qsTr('如: ') + UnitManager.formatVolume(100)
        case 'visibility':
            return qsTr('如: ') + UnitManager.formatVisibility(10000)
        }
        return ''
    }

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

        Repeater {
            model: root.unitCategories

            Row {
                width: detailContent.width
                height: 40
                spacing: 16

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

                HusSelect {
                    id: unitSelect
                    width: 140
                    anchors.verticalCenter: parent.verticalCenter

                    property string categoryId: modelData.id
                    property var availableUnits: UnitManager.getAvailableUnits(categoryId)

                    model: availableUnits.map(function(unitItem) {
                        return {
                            label: unitItem.name + ' (' + unitItem.symbol + ')',
                            value: unitItem.id
                        }
                    })

                    currentIndex: {
                        var selectedUnit = root.currentUnit(categoryId)
                        for (var index = 0; index < availableUnits.length; ++index) {
                            if (availableUnits[index].id === selectedUnit)
                                return index
                        }
                        return 0
                    }

                    onCurrentValueChanged: {
                        if (currentValue && currentValue !== root.currentUnit(categoryId))
                            root.updateUnit(categoryId, currentValue)
                    }
                }

                HusText {
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.previewText(modelData.id)
                    color: HusTheme.Primary.colorTextTertiary
                    font.pixelSize: 12
                }
            }
        }
    }
}