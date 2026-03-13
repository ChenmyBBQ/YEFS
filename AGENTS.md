# YEFS Agent Rules

本仓库是 Qt6/QML/C++ 桌面 GIS 应用。

始终遵守以下规则：

- 所有回答、计划、说明、提交说明与代码注释默认使用中文。
- 新增功能与修改不得破坏 Qt6/QML/C++ 桌面 GIS 主架构。
- 先定位根因，再实施修改，避免表面补丁。
- 涉及 QML 页面、交互、导航、HuskarUI 组件时，优先遵守 [.github/instructions/qml.instructions.md](.github/instructions/qml.instructions.md)。
- 涉及 C++ 核心逻辑、对象生命周期、资源管理与接口修改时，优先遵守 [.github/instructions/cpp.instructions.md](.github/instructions/cpp.instructions.md)。
- 涉及验证、构建、测试与回归说明时，优先遵守 [.github/instructions/testing.instructions.md](.github/instructions/testing.instructions.md)。
- 涉及缺陷分析、日志排查、根因定位与修改边界时，优先遵守 [.github/instructions/workflow.instructions.md](.github/instructions/workflow.instructions.md)。
- 详细项目背景与架构约束见 [.github/copilot/architecture.md](.github/copilot/architecture.md)。
