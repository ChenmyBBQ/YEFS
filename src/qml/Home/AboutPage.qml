import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import HuskarUI.Basic

import YEFSApp

HusWindow {
    id: root
    width: 400
    height: 450
    minimumWidth: 400
    minimumHeight: 450
    captionBar.showMinimizeButton: false
    captionBar.showMaximizeButton: false
    captionBar.winTitle: qsTr('关于')
    
    signal closeRequested()
    
    captionBar.winIconDelegate: Item {
        Image {
            width: 16
            height: 16
            anchors.centerIn: parent
            source: 'qrc:/YEFSApp/resources/images/YEFS.svg'
        }
    }
    captionBar.closeCallback: () => {
        root.closeRequested();
        root.visible = false;
    }

    onClosing: (close) => {
        close.accepted = false;
        root.closeRequested();
        root.visible = false;
    }

    Item {
        anchors.fill: parent

        Rectangle {
            id: backRect
            anchors.fill: parent
            radius: 6
            color: HusTheme.Primary.colorBgBase
            border.color: HusThemeFunctions.alpha(HusTheme.Primary.colorTextBase, 0.2)
        }

        Column {
            width: parent.width
            anchors.top: parent.top
            anchors.topMargin: captionBar.height + 30
            spacing: 15

            Item {
                width: 80
                height: width
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    width: parent.width
                    height: width
                    anchors.centerIn: parent
                    source: 'qrc:/YEFSApp/resources/images/YEFS.svg'
                }
            }

            HusText {
                anchors.horizontalCenter: parent.horizontalCenter
                font {
                    family: HusTheme.Primary.fontPrimaryFamily
                    pixelSize: HusTheme.Primary.fontPrimarySizeHeading3
                    bold: true
                }
                text: 'YEFS'
            }

            HusText {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr('无人机地面站系统')
                color: HusTheme.Primary.colorTextSecondary
            }

            HusCopyableText {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr('版本: 1.0.0')
            }

            HusCopyableText {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr('基于 Qt / QML / MapLibre')
            }

            Item { width: 1; height: 20 }

            HusCopyableText {
                width: parent.width - 30
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: HusCopyableText.WordWrap
                horizontalAlignment: HusCopyableText.AlignHCenter
                text: qsTr('UI组件库: <a href=\'https://github.com/mengps/HuskarUI\' style=\'color:#722ED1\'>HuskarUI</a>')
                textFormat: HusCopyableText.RichText
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }

            HusCopyableText {
                width: parent.width - 30
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: HusCopyableText.WordWrap
                horizontalAlignment: HusCopyableText.AlignHCenter
                text: qsTr('地图渲染: <a href=\'https://maplibre.org/\' style=\'color:#722ED1\'>MapLibre</a>')
                textFormat: HusCopyableText.RichText
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }
        }
    }
}
