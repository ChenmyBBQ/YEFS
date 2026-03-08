import QtQuick
import HuskarUI.Basic

Item {
    id: root

    property int compactMode: HusMenu.Mode_Relaxed
    property real defaultMenuWidth: 260
    property real compactWidth: 48
    property var options: []
    property Item topTarget: null

    signal optionSelected(var option)

    property bool expanded: false
    readonly property bool relaxedMode: compactMode === HusMenu.Mode_Relaxed

    x: 0
    y: topTarget ? topTarget.y + topTarget.height : 0
    width: relaxedMode ? defaultMenuWidth : compactWidth + defaultMenuWidth
    height: 48

    function collapseSearch() {
        searchComponent.closePopup()
        expanded = false
    }

    HusAutoComplete {
        id: searchComponent
        z: 10
        clip: true
        x: root.relaxedMode ? 10 : root.compactWidth + 10
        width: (root.relaxedMode || root.expanded) ? (root.defaultMenuWidth - 20) : 0
        anchors.verticalCenter: parent.verticalCenter
        topPadding: 6
        bottomPadding: 6
        rightPadding: 50
        showToolTip: true
        placeholderText: qsTr('搜索功能')
        iconSource: HusIcon.SearchOutlined
        colorBg: !root.relaxedMode ? HusTheme.HusInput.colorBg : 'transparent'
        options: root.options
        filterOption: (input, option) => option.label.toUpperCase().indexOf(input.toUpperCase()) !== -1
        onSelect: option => root.optionSelected(option)

        Keys.onEscapePressed: {
            if (root.expanded) {
                root.expanded = false
            } else {
                closePopup()
            }
        }

        Behavior on width {
            enabled: !root.relaxedMode
            NumberAnimation { duration: HusTheme.Primary.durationFast }
        }
    }

    HusIconButton {
        id: searchCollapse
        visible: !root.relaxedMode
        x: 0
        width: root.compactWidth
        height: 40
        anchors.verticalCenter: parent.verticalCenter
        type: HusButton.Type_Text
        colorText: HusTheme.Primary.colorTextBase
        iconSource: HusIcon.SearchOutlined
        iconSize: searchComponent.iconSize
        onClicked: {
            root.expanded = !root.expanded
            if (root.expanded)
                searchComponent.forceActiveFocus()
        }
        onVisibleChanged: {
            if (visible)
                root.collapseSearch()
        }
    }
}