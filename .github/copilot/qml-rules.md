# QML 与 UI 规范

## 适用范围
- 当前 YEFS 主界面为 Qt6/QML。
- 涉及页面布局、交互、组件选型、导航装配、QML 与单例交互时，优先遵守本文规则。

## 组件与视觉约束
- 编写或重构 QML 页面时，优先使用 HuskarUI，默认引入 HuskarUI.Basic，优先选用 Hus* 组件。
- QML 交互、布局、组件用法优先参考 HuskarUI gallery 与 reference/qml 示例。
- 保持现有桌面 GIS 产品的交互节奏与导航方式，不随意引入偏 Web 的页面结构。

## 页面与导航约束
- 主导航骨架在 src/qml/Main.qml，菜单与页面路由定义在 src/qml/YefsGlobal.qml。
- 修改导航与页面装配时，优先保持 Main.qml、YefsGlobal.qml 的菜单驱动结构稳定。
- 新页面优先挂接到既有菜单和 Loader 机制，不绕过现有导航体系临时拼装入口。

## 逻辑边界
- 不在 QML 页面中堆积复杂业务逻辑；状态协调、地图交互、消息分发优先放入 C++ 控制器、模型或现有单例。
- QML 负责表现和轻量绑定，复杂行为放到 C++ 核心层。

## 单例与系统能力
- QML 中先导入 YEFSApp，再通过单例访问系统能力。
- MessageBus 用于消息分发与跨模块解耦。
- SettingsManager 用于配置持久化与读取。
- PluginManager 用于插件扫描、加载和插件页面入口解析。
- UnitManager 用于单位换算和格式化。

## 验证要求
- 若修改只涉及布局或交互，至少说明手工验证路径。
- 若修改影响设置、插件、消息流或地图行为，需同步说明影响范围与回归点。