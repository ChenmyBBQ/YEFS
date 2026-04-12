# YEFS Copilot 主引导

## 项目总纲
- 所有会话输出默认使用中文，包括解释、计划、错误分析、提交说明与代码注释。
- 当前主工程是 Qt6/QML/C++ 桌面 GIS 应用；新增规则、实现与重构不得反向破坏这一主架构。
- 处理缺陷、异常或行为偏差时，先看日志与现象，再结合源码定位根因，避免猜测式修改。
- 优先做根因修复，不做头痛医头脚痛医脚式补丁。
- 构建、编译、测试、重建与运行优先使用 VS Code 的 CMake Tools 能力。

## 模块与架构基线
- 启动入口在 src/cpp/main.cpp，由 YEFS::Application 完成 initialize() 与 run()，随后加载 qrc:/YEFSApp/qml/Main.qml。
- 主分层保持稳定：src/core 为 C++ 核心层，src/qml 为 QML 表现层，plugins 为插件扩展层，3rdparty 为外部依赖层。
- 复杂业务逻辑放在 C++ 控制器、模型或核心服务，不堆到 QML 页面中；QML 负责界面表现、装配与轻量绑定。
- 关键机制默认沿用既有实现：MessageBus 负责跨模块通信，SettingsManager 负责设置持久化，PluginManager 负责插件管理，UnitManager 负责单位与格式化。
- QML 导航保持 Main.qml 与 YefsGlobal.qml 的菜单驱动结构，新增页面优先接入现有导航体系。
- 不直接修改 3rdparty 承载业务需求，优先在宿主层做适配或封装。

## 规则分层
- 第一层总入口：本文件负责项目定位、模块边界、硬性约束与规则路由。
- 第二层正式规则：.github/instructions 下的 *.instructions.md 负责按目录或文件类型命中。
- 第三层扩展文档：.github/copilot 下的规则文档用于提供更完整的背景说明与细节，不应单独承担硬约束。

## 规则路由
- 处理 C++ 核心逻辑、对象生命周期、资源管理、接口修改时，优先遵守 .github/instructions/cpp.instructions.md，并参考 .github/copilot/cpp-rules.md。
- 处理 QML 页面、导航、交互、HuskarUI 组件时，优先遵守 .github/instructions/qml.instructions.md，并参考 .github/copilot/qml-rules.md。
- 处理验证、构建、测试、回归说明时，优先遵守 .github/instructions/testing.instructions.md，并参考 .github/copilot/testing-policy.md。
- 处理缺陷排查、日志分析、修改边界与修复流程时，优先遵守 .github/instructions/workflow.instructions.md，并参考 .github/copilot/workflow-rules.md。
- 处理项目通用约束时，遵守 .github/instructions/project.instructions.md。
- 仅当仓库新增独立 Web 前端子项目时，才启用 .github/copilot/frontend-rules.md。
- 仅当仓库新增独立 Go/Node.js 服务、数据同步进程或工具链时，才启用 .github/copilot/backend-rules.md。
- 涉及 Git 提交、推送、历史改写与保护策略时，参考 .github/copilot/git-rules.md。
- 涉及模块说明文档与主框架说明维护时，参考 .github/copilot/readme-maintenance.md。

## 执行要求
- 修改前先判断任务属于 C++、QML、测试、工作流或文档哪一类，再加载对应规则，避免误用 Web 规范到 Qt/QML 主工程。
- 若改动涉及控制器、数据模型、设置、插件管理、消息分发或接口变更，优先补充自动化验证；若只涉及 QML 布局或交互，至少提供手工验证路径。
- 回答缺陷修复类任务时，优先说明问题原因，再说明修复方案、影响范围与验证方式。
