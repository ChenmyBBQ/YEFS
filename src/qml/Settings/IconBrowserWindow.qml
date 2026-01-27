import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import HuskarUI.Basic
import YEFSApp

HusWindow {
    id: window
    width: 900
    height: 700
    title: qsTr("图标资源浏览器")
    captionBar.color: HusTheme.Primary.colorFillTertiary
    
    // 缓存图标数据
    property var iconNames: []
    property var iconValues: ({})
    
    Component.onCompleted: {
        var map = HusIcon.allIconNames()
        iconValues = map
        var keys = Object.keys(map)
        // 排序
        keys.sort()
        iconNames = keys
    }

    // 搜索过滤
    property string searchText: ""

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: window.captionBar.height
        color: HusTheme.Primary.colorBgContainer
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10
            
            // 顶部搜索栏
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                HusInput {
                    Layout.fillWidth: true
                    placeholderText: qsTr("搜索图标名称...")
                    iconSource: HusIcon.SearchOutlined
                    iconPosition: HusInput.Position_Left
                    onTextChanged: {
                        window.searchText = text.toLowerCase()
                    }
                }
                
                HusText {
                    text: qsTr("共 %1 个图标").arg(view.count)
                    color: HusTheme.Primary.colorTextSecondary
                }
            }
            
            // 图标网格
            GridView {
                id: view
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                cellWidth: 120
                cellHeight: 140
                
                model: iconNames.filter(name => searchText === "" || name.toLowerCase().indexOf(searchText) !== -1)
                
                delegate: Rectangle {
                    id: delegateRoot
                    width: view.cellWidth - 10
                    height: view.cellHeight - 10
                    radius: 4
                    color: mouseArea.containsMouse ? HusTheme.Primary.colorFillSecondary : HusTheme.Primary.colorBgContainer
                    border.color: mouseArea.containsMouse ? HusTheme.Primary.colorPrimary : "transparent"
                    
                    property string iconName: modelData
                    property int iconValue: iconValues[modelData]
                    
                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            detailPopup.openWithIcon(delegateRoot.iconName, delegateRoot.iconValue)
                        }
                    }
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        width: parent.width
                        spacing: 8
                        
                        HusText {
                            text: String.fromCharCode(delegateRoot.iconValue)
                            font.family: "HuskarUI-Icons"
                            font.pixelSize: 32
                            Layout.alignment: Qt.AlignHCenter
                            color: HusTheme.Primary.colorTextBase
                        }
                        
                        HusText {
                            text: delegateRoot.iconName
                            Layout.fillWidth: true
                            Layout.leftMargin: 4
                            Layout.rightMargin: 4
                            elide: Text.ElideMiddle
                            horizontalAlignment: Text.AlignHCenter
                            Layout.alignment: Qt.AlignHCenter
                            font.pixelSize: 12
                            color: HusTheme.Primary.colorTextSecondary
                            
                            HusToolTip {
                                visible: mouseArea.containsMouse
                                text: parent.text
                            }
                        }
                    }
                }
                
                ScrollBar.vertical: HusScrollBar {}
            }
        }
    }
    
    // 详情/编辑弹窗
    HusPopup {
        id: detailPopup
        width: 400
        height: 500
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        
        property string curName: ""
        property int curValue: 0
        property color curColor: HusTheme.Primary.colorPrimary
        property int curSize: 64
        
        function openWithIcon(name, value) {
            curName = name
            curValue = value
            curColor = HusTheme.Primary.colorPrimary
            curSize = 64
            open()
        }
        
        contentItem: ColumnLayout {
            spacing: 20
            
            RowLayout {
                Layout.fillWidth: true
                HusText {
                    text: detailPopup.curName
                    font.weight: Font.Bold
                    font.pixelSize: 18
                    Layout.fillWidth: true
                }
            }
            
            // 预览区域
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 150
                color: HusTheme.Primary.colorFillQuaternary
                radius: 8
                border.width: 1
                border.color: HusTheme.Primary.colorBorder
                
                HusText {
                    anchors.centerIn: parent
                    text: String.fromCharCode(detailPopup.curValue)
                    font.family: "HuskarUI-Icons"
                    font.pixelSize: detailPopup.curSize
                    color: detailPopup.curColor
                }
                
                // 背景网格效果（可选，为了看清透明度）
                HusCheckerBoard {
                    anchors.fill: parent
                    z: -1
                    visible: detailPopup.curColor.a < 1
                }
            }
            
            // 代码复制
            RowLayout {
                Layout.fillWidth: true
                HusInput {
                    Layout.fillWidth: true
                    readOnly: true
                    text: 'iconSource: HusIcon.' + detailPopup.curName
                }
                HusButton {
                    text: qsTr("复制")
                    onClicked: {
                        selectAll()
                        copy()
                        HusMessage.success(qsTr("已复制到剪贴板"))
                    }
                }
            }
            
            HusDivider {}
            
            // 样式调整
            GridLayout {
                columns: 2
                rowSpacing: 10
                columnSpacing: 10
                
                HusText { text: qsTr("颜色:") }
                HusColorPicker {
                    Layout.fillWidth: true
                    defaultValue: detailPopup.curColor
                    onChange: (color) => { detailPopup.curColor = color }
                    alphaEnabled: true
                }
                
                HusText { text: qsTr("尺寸:") }
                RowLayout {
                    HusSlider {
                        Layout.fillWidth: true
                        min: 16
                        max: 200
                        value: detailPopup.curSize
                        onCurrentValueChanged: detailPopup.curSize = currentValue
                    }
                    HusText {
                        text: detailPopup.curSize + "px"
                        width: 40
                    }
                }
            }
            
            Item { Layout.fillHeight: true }

            HusDivider { Layout.fillWidth: true }

            RowLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: 10
                Item { Layout.fillWidth: true }
                HusButton {
                    text: qsTr("关闭")
                    onClicked: detailPopup.close()
                }
            }
        }
    }
}
