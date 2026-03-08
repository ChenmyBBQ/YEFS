import QtQuick

Rectangle {
    id: root

    property bool dismissed: false
    property color coverColor: '#b8d9f0'

    z: 10000
    color: root.coverColor
    visible: opacity > 0.01
    opacity: root.dismissed ? 0.0 : 1.0

    Behavior on opacity {
        NumberAnimation {
            duration: 180
            easing.type: Easing.OutCubic
        }
    }
}