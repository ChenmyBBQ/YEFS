# YEFS（Qt6/QML GIS 地面站）Copilot 指南

## 硬性规则（必须遵守）
1. 所有会话输出必须使用中文（含解释、计划、报错分析、提交说明等）。
2. 编写/重构 QML 页面时，优先使用 HuskarUI（`import HuskarUI.Basic`，优先选用 `Hus*` 组件）。
3. 编写/重构 QML 页面时，交互/布局/组件用法优先参考 HuskarUI 的 gallery 示例。

## 入口与运行链路（可从源码验证）
- 启动入口：`src/cpp/main.cpp` → 创建 `YEFS::Application` → `initialize()` → `run()` 加载 `qrc:/YEFSApp/qml/Main.qml`。
- 构建输出/运行：可执行文件位于 `out/<BuildType>/bin/YEFS`，直接运行该二进制。

## 构建与部署（可从 CMake 验证）
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
./out/RelWithDebInfo/bin/YEFS
```
- 顶层 `CMakeLists.txt` 统一设置输出到 `out/<BuildType>/bin`。
- `src/CMakeLists.txt` 在 POST_BUILD 阶段复制运行所需的 QML 模块/插件到 `out/<BuildType>/bin`：
    - HuskarUI QML 模块到 `bin/HuskarUI/*`，并将 Basic/Impl 插件放入对应模块目录。
    - MapLibre QML 模块到 `bin/MapLibre/*`，GeoServices 插件到 `bin/geoservices/`。

## QML 结构与导航（可从 QML 验证）
- 导航骨架：`src/qml/Main.qml`（HusWindow + HusMenu + Loader）。
- 菜单/路由：`src/qml/YefsGlobal.qml` 定义 `menus/options`，菜单项 `source` 指向 `src/qml/Home/*.qml`。
- 页面组织：主页面在 `src/qml/Home/`（如 MapPage/SettingsPage/AboutPage），设置子面板在 `src/qml/Settings/`。

## 核心 QML 单例与用法（可从头文件验证）
在 QML 中先 `import YEFSApp`，然后直接调用（这些类均为 `QML_SINGLETON`）：
- `MessageBus.send(topic, data)`；主题常量在 `src/core/MessageBus.h` 的 `YEFS::Topics::*`。
- `SettingsManager.getValue(category, key, default)` / `setValue(...)`；配置文件位置为 `QStandardPaths::AppDataLocation/settings.json`，可用 `SettingsManager.settingsFilePath()` 获取。
- `PluginManager.loadAllPlugins()` / `getPluginQmlEntry(id)` / `getPluginSettingsPage(id)`。
- `UnitManager.*`：单位格式化/换算（如 `formatAltitude`、`convert`）。

## 插件机制（可从源码验证）
- 接口：实现 `src/core/IPlugin.h`（必须实现 `initialize/shutdown/id/name/version/pluginType`）。
- 扫描目录（Linux）：`<bin>/plugins`、`<bin>/../plugins`、`~/.local/share/YEFS/plugins`（见 `PluginManager` 构造与 `scanPlugins()`）。
