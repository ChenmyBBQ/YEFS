# 整体架构设计

## 文档定位
- 本文档是架构背景与模块边界说明，供 .github/copilot-instructions.md 与 .github/instructions/*.instructions.md 引用。
- 本文档用于补充架构细节，不单独承担仓库级硬性约束。

## 项目定位
- YEFS 是基于 Qt6/QML 的 GIS 地面站应用。
- 当前主架构是本地桌面应用，不是前后端分离 Web 系统。
- 规则设计必须以现有 C++ 核心、QML 界面、插件扩展机制为中心。

## 启动链路
- 启动入口位于 src/cpp/main.cpp。
- main.cpp 创建 YEFS::Application，执行 initialize()，随后 run() 加载 qrc:/YEFSApp/qml/Main.qml。
- 应用启动后会写入调试日志文件 yefs_debug.log，可用于排查启动、关闭与运行期问题。
- 应用最终通过 Main.qml 组织顶层窗口、菜单、主内容区与设置面板。

## 分层结构
- C++ 核心层：位于 src/core，承载应用生命周期、地图引擎适配、消息总线、设置管理、插件管理、业务控制器等。
- QML 表现层：位于 src/qml，负责页面结构、视觉交互、菜单装配和轻量状态绑定。
- 插件层：位于 plugins，插件通过 IPlugin 接口接入宿主，由 PluginManager 扫描、加载和管理。
- 第三方依赖层：位于 3rdparty，包括 HuskarUI 与 maplibre-native-qt，默认视为外部依赖，不随业务任务轻易改动。

## 运行与产物
- 构建输出目录统一为 out/<BuildType>/bin。
- 运行所需的 HuskarUI、MapLibre QML 模块与相关插件由 CMake 在构建后复制到输出目录。

## 关键机制
- 消息通信：跨模块事件优先通过 MessageBus 协调，减少 QML 与业务对象的直接耦合。
- 设置持久化：统一经 SettingsManager 读写，配置文件位于应用数据目录。
- 插件扩展：统一经 PluginManager 管理，QML 页面可通过单例获取插件入口与设置页。
- 单位与格式化：统一经 UnitManager 处理，避免界面层重复实现换算逻辑。

## QML 导航结构
- Main.qml 是导航骨架，负责顶层窗口、侧边菜单、内容 Loader 与设置覆盖层。
- YefsGlobal.qml 定义菜单与搜索选项，菜单项通过 source 指向 Home 下页面。
- 新页面优先挂接到既有菜单驱动结构，不绕过导航体系直接拼装临时入口。

## 架构约束
- 不把复杂业务逻辑堆到 QML；QML 负责表现和简单绑定，复杂行为放到 C++ 控制器或模型。
- 不直接修改 3rdparty 代码来承载业务需求，优先在宿主层做适配或封装。
- 新增子系统前，先明确它属于核心层、插件层还是外部服务，避免职责漂移。
- 所有新增规则、目录和模块都要与当前桌面 GIS 架构一致，不能默认按 Web 项目思路重组主工程。