---
name: YEFS QML 规则
description: Qt6/QML 与 HuskarUI 页面开发规则
applyTo: "{src/qml/**,plugins/**/qml/**,reference/qml/**}/**/*.qml"
---

- 当前 UI 层是 Qt6/QML，涉及页面布局、交互、组件选型、导航装配时优先遵守 QML 规则。
- 优先使用 HuskarUI，默认引入 HuskarUI.Basic，优先选用 Hus* 组件。
- 保持现有桌面 GIS 产品的交互节奏与导航方式，不随意引入偏 Web 的页面结构。
- 不在 QML 页面中堆积复杂业务逻辑；QML 负责表现和轻量绑定，复杂行为放到 C++ 控制器、模型或现有单例。
- 修改导航与页面装配时，优先保持 Main.qml 与 YefsGlobal.qml 的菜单驱动结构稳定。
- 若修改只涉及布局或交互，至少说明手工验证路径。
- 详细规则参见 [.github/copilot/qml-rules.md](../copilot/qml-rules.md)。
