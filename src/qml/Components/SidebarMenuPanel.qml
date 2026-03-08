import QtQuick
import HuskarUI.Basic

import YEFSApp

HusMenu {
    id: root

    property string resetMenuKey: 'MapPage'

    signal leafTriggered(string key, var data)

    onClickMenu: function(deep, key, keyPath, data) {
        if (data && !data.hasOwnProperty('menuChildren'))
            root.leafTriggered(key, data)
    }

    Connections {
        target: MessageBus
        function onMessage(topic, data) {
            if (topic === 'airspace-manager/hide')
                root.gotoMenu(root.resetMenuKey)
        }
    }
}