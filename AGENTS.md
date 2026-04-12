# YEFS Agent Rules

本仓库是 Qt6/QML/C++ 桌面 GIS 应用。

始终遵守以下规则：

- 所有回答、计划、说明、提交说明与代码注释默认使用中文。
- 新增功能与修改不得破坏 Qt6/QML/C++ 桌面 GIS 主架构。
- 先定位根因，再实施修改，避免表面补丁。
- 启动入口位于 src/cpp/main.cpp，应用最终加载 Main.qml；新增页面优先接入既有菜单驱动导航。
- 主体分层保持稳定：src/core 为核心层，src/qml 为界面层，plugins 为插件层，3rdparty 视为外部依赖。
- 复杂业务逻辑不堆到 QML；跨模块事件优先通过 MessageBus，设置持久化统一经 SettingsManager，插件统一经 PluginManager 管理。
- 涉及 QML 页面、交互、导航、HuskarUI 组件时，优先遵守 [.github/instructions/qml.instructions.md](.github/instructions/qml.instructions.md)。
- 涉及 C++ 核心逻辑、对象生命周期、资源管理与接口修改时，优先遵守 [.github/instructions/cpp.instructions.md](.github/instructions/cpp.instructions.md)。
- 涉及验证、构建、测试与回归说明时，优先遵守 [.github/instructions/testing.instructions.md](.github/instructions/testing.instructions.md)。
- 涉及缺陷分析、日志排查、根因定位与修改边界时，优先遵守 [.github/instructions/workflow.instructions.md](.github/instructions/workflow.instructions.md)。
- 仅当仓库新增独立 Web 前端子项目时，再参考 [.github/copilot/frontend-rules.md](.github/copilot/frontend-rules.md)。
- 仅当仓库新增独立 Go/Node.js 服务或工具链时，再参考 [.github/copilot/backend-rules.md](.github/copilot/backend-rules.md)。
- 详细项目背景与架构约束见 [.github/copilot/architecture.md](.github/copilot/architecture.md)。
