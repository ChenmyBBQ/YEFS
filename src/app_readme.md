# YEFS 主框架说明

## 1. 主框架职责

主框架位于 `src/`，负责把 Qt/QML 界面、C++ 核心服务、地图引擎、数据库组件和插件系统组织成一个可运行的桌面 GIS 应用。当前主链路不是传统前后端架构，而是本地单进程桌面应用，核心协作关系如下：

- `cpp/main.cpp`：创建 `QGuiApplication`，安装日志输出，构造 `YEFS::Application`，完成初始化后进入事件循环。
- `core/Application.*`：统一负责服务初始化、QML 类型注册、DBCompt 打开、插件加载、地图解析器注册、退出清理。
- `core/MessageBus.*`：作为跨模块消息总线，承接 QML、主框架和插件之间的发布/订阅通信。
- `core/PluginManager.*`：扫描 `plugins` 目录并按依赖顺序加载插件，向插件暴露主框架服务。
- `qml/Main.qml`：顶层窗口与导航骨架，管理左侧菜单、页面 Loader、设置覆盖层与全局消息监听。
- `qml/Home/MapPage.qml`：地图主工作区，承载 MapLibre 地图、状态栏、绘制提示、新建空域工具栏、空域列表面板。

## 2. 启动流程

### 2.1 进程启动

1. `main.cpp` 启动 Qt 应用，并把 Qt 日志重定向到 `yefs_debug.log`。
2. 构造 `YEFS::Application`，调用 `initialize()`。
3. `Application::initialize()` 完成以下步骤：
   - 设置渲染循环与 MapLibre 对应图形 API。
   - 配置应用名、组织信息、版本号。
   - 创建 `QQmlApplicationEngine` 并加入应用目录到 import path。
   - 初始化 HuskarUI。
   - 注册 `YEFSApp` QML 类型和单例。
   - 打开 `AppConfig/db_schema.json` 指定的数据库 schema。
   - 加载所有插件。
   - 注册 GeoJSON、GPX、KML 解析器并初始化地图源管理器。
   - 绑定 `aboutToQuit`，在退出前执行统一清理。
4. `Application::run()` 加载 `qrc:/YEFSApp/qml/Main.qml`。
5. 根对象创建成功后发布 `app/ready` 消息，随后进入 Qt 事件循环。

### 2.2 退出流程

退出由 `Application::cleanup()` 统一按依赖逆序处理：

1. 先销毁 `PluginManager`，触发插件 `shutdown()`，避免插件仍持有后台资源。
2. 再销毁 `QQmlApplicationEngine`，让地图与页面对象完整析构。
3. 最后销毁单位管理、地图源、地图解析器、地图引擎、图标、数据库、设置与消息总线等单例。

这个顺序的目的是避免 QML 或插件对象在底层服务已经释放后仍尝试访问它们。

## 3. 主界面交互流程

### 3.1 导航与页面切换

- `qml/YefsGlobal.qml` 定义左侧菜单树和搜索项。
- `qml/Main.qml` 通过 `SidebarMenuPanel` 渲染菜单，通过 `containerLoader` 加载页面。
- 当前多数菜单都指向 `Home/MapPage.qml`，不同业务状态通过消息总线而不是多页面切换来驱动。

### 3.2 菜单与插件的联动

`Main.qml` 在菜单叶子项点击时会执行 `handleMenuLeafClick()`：

- 点击“新建空域”时发送 `airspace-manager/new`。
- 点击“空域管理”时发送 `airspace-manager/show`。
- 点击其他菜单时发送 `airspace-manager/hide`。

主框架并不直接调用插件内部对象，而是通过 `MessageBus` 通知地图页状态控制器和插件自身分别做响应，从而保持 QML 与插件的耦合度可控。

### 3.3 地图页状态协作

`MapPage.qml` 是当前业务主工作区，其状态由 `MapPageStateController` 协调：

- 鼠标移动时更新经纬度、UTM、MGRS 文本。
- 进入绘制模式后控制右下提示条、右侧空域列表、左上新建空域工具栏的显示状态。
- 接收 `airspace-manager/drawing-state`，同步“是否正在绘制”“是否需要手动完成”“当前提示文案”。
- 在用户点击地图时，把屏幕坐标转换为地理坐标，再转发给插件绘制控制器。

### 3.4 地图加载与遮罩流程

地图页为了避免首帧闪烁，采用了比 `mapFullyLoaded` 更保守的显示时序：

1. 样式切换时先把加载遮罩重新置为可见。
2. `MapLibre` 首帧纹理真正准备好后触发 `onFirstFrameReady`。
3. 再额外等待 100ms，给 GPU 管线留出稳定窗口。
4. 最后再执行遮罩淡出。

这条链路的核心目标是避免样式刚切换时出现未初始化纹理或中间帧闪烁。

## 4. 主框架与插件交互机制

### 4.1 插件发现与加载

`PluginManager` 会扫描以下目录：

- 应用目录下的 `plugins`
- 应用上一级目录下的 `plugins`
- 用户目录下的 YEFS 插件目录

扫描规则依赖 `QPluginLoader` 的元数据 `MetaData.id`。加载时会：

1. 先检查是否已加载。
2. 若有依赖插件，先递归加载依赖。
3. 调用插件 `initialize(context)`。
4. 监听插件向宿主发送的 `messageToHost` 信号。
5. 发布 `plugin/loaded` 消息。

### 4.2 插件上下文

主框架通过 `DefaultPluginContext` 向插件暴露有限服务：

- `MessageBus`
- `MapLibreEngine`
- `SettingsManager`
- `MapSourceManager`

插件只能通过上下文拿到这些服务，不能直接依赖整个宿主内部实现，这是当前插件边界的主要约束。

## 5. 数据与消息主链路

### 5.1 消息总线

`MessageBus` 同时支持两种消费方式：

- QML 通过 `Connections { target: MessageBus }` 监听统一 `message(topic, data)` 信号。
- C++ 通过 `subscribe/unsubscribe` 维护订阅表，内部使用 `QMetaObject::invokeMethod` 回调接收方槽函数。

当前高频主题主要包括：

- 地图：`map/clicked`、`map/hovered`、`map/preview/annotation/set`
- 插件：`plugin/loaded`、`plugin/unloaded`
- 应用：`app/ready`
- 业务：`airspace-manager/new`、`airspace-manager/show`、`airspace-manager/drawing-state`

### 5.2 数据持久化

主框架自身不直接拼装空域表 SQL，而是先初始化 `DBCompt`，再由插件中的 `AirspaceDatabase` 通过 DBCompt 公共 API 读写。这样数据库连接、schema 迁移、驱动差异都被限制在 `src/dbcompt` 模块内部。

## 6. 关键算法与实现策略

### 6.1 高频 Hover 合并

`MapPageStateController` 不会对每次鼠标移动都立即做完整转换和预览更新，而是采用：

- `0ms` 单次定时器合并一个事件循环中的 hover 事件。
- 小于 2px 的抖动直接忽略。
- 绘制模式下优先保证预览更新，只刷新经纬度文本，跳过更重的 UTM/MGRS 转换。

这样做的目的，是避免快速移动鼠标时频繁 JSON 序列化、坐标转换和 GPU 上传导致卡顿。

### 6.2 解析器注册与格式扩展

`Application::initializeMapParsers()` 在启动时向 `MapParserFactory` 注册 GeoJSON、GPX、KML 解析器。当前扩展策略是“按格式注册解析器 + 由工厂按后缀路由”，后续增加格式时应优先沿用这一机制，而不是在调用侧写大量 `if/else`。

### 6.3 构建产物组织

顶层 CMake 把运行产物固定到 `out/<BuildType>/bin`，并在构建后复制：

- HuskarUI 动态库与 QML 插件
- DBCompt 动态库
- `AppConfig/db_schema.json`
- 业务插件到 `bin/plugins`

因此运行时查找插件、QML 模块和 schema 的路径假设都建立在这套输出目录布局上。

## 7. 设计约束与维护建议

- 复杂业务逻辑优先放在 C++ 控制器、模型或插件，不要回灌到 QML。
- 跨模块通知优先用 `MessageBus`，避免页面直接强依赖插件私有对象。
- 新插件若依赖数据库，应继续通过 `DBCompt` 访问，而不是自行维护第二套连接。
- 任何会影响主导航、消息主题、状态流或核心算法的改动，都需要同步更新本文件。

## 8. 日志规范（模块/子模块/功能/版本）

为提高排障效率，主程序与各模块统一采用以下日志标签格式：

- `[M:<模块>]`：一级模块，例如 `MainApp`、`Application`、`PluginManager`。
- `[SM:<子模块>]`：二级模块，例如 `SettingsOverlay`、`MessageBus`。
- `[FN:<功能>]`：功能或函数名，例如 `toggleSettingsPanel`、`onMessage`。
- `[V:<版本>]`：应用版本，来自 `YEFS_VERSION` 或 `Qt.application.version`。

当前落地策略：

1. `cpp/main.cpp` 的消息处理器统一追加 `[M][FN][V]` 基础字段。
2. QML 关键链路（主窗口、设置入口）通过统一 `logTag(...)` 输出 `[M][SM][FN][V]`。
3. 其余 C++/QML 模块按功能改造时逐步迁移，避免一次性大面积重写日志。

建议新增或改造日志时，优先记录“动作 + 关键状态 + 输入参数”，例如：

- 打开/关闭页面：记录可见状态变化。
- 加载器状态变化：记录 source、status、active、visible。
- 消息总线转发：记录 topic 与关键 payload。