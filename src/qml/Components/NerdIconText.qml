import QtQuick
import QtQuick.Controls

Text {
    id: root

    // 绑定我们在 C++ 注册时的名称，NerdFontsSymbolsOnly 通常注册名叫 "Symbols Nerd Font"
    font.family: "Symbols Nerd Font"
    
    // 默认大小
    font.pixelSize: 16
    
    // 默认颜色可以跟随系统的主题，或者由外部传入
    // color: HusTheme.colorText // 如果引入了 HuskarUI 可以绑定
    
    // Antialiasing 保证矢量清晰
    renderType: Text.QtRendering
    antialiasing: true
    
    // 垂直和水平居中，保证作为图标排版时不会偏位
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter
}
