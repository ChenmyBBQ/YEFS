# YEFS Copilot 主引导

## 规则入口
- 整体架构先看 .github/copilot/architecture.md。
- 遇到异常、BUG与代码问题排查看 .github/copilot/workflow-rules.md。
- C++开发与核心逻辑编程规范看 .github/copilot/cpp-rules.md。
- Qt/QML 界面与交互相关任务看 .github/copilot/qml-rules.md。
- Git 提交、推送与变更保护看 .github/copilot/git-rules.md。
- 模块与主框架说明文档维护看 .github/copilot/readme-maintenance.md。
- React 与 Tailwind 相关任务看 .github/copilot/frontend-rules.md。
- Go 与 Node.js 相关任务看 .github/copilot/backend-rules.md。
- 测试与验证要求看 .github/copilot/testing-policy.md。

## 硬性规则
1. 所有会话输出必须使用中文，包括解释、计划、提交说明、错误分析与代码注释。
2. 当前主工程是 Qt6/QML/C++ 桌面 GIS 应用；新增规则不得反向破坏这一主架构。

## 执行原则
- 修改前先确认任务适用哪一类规则文件，避免把 Web 规范误用于 QML 主工程。
- 分析问题、定位根因与修复流程，以 workflow-rules.md 为优先依据。
- 涉及 C++ 核心逻辑、内存管理、对象生命周期时，以 cpp-rules.md 为优先依据。
- 涉及架构、模块边界、插件扩展、设置持久化时，以 architecture.md 为优先依据。
- 涉及 QML 页面、导航、单例调用与 HuskarUI 组件时，以 qml-rules.md 为优先依据。
- 涉及提交、推送、历史改写与工作区保护时，以 git-rules.md 为优先依据。
- 涉及模块说明文档或主框架说明文档维护时，以 readme-maintenance.md 为优先依据。
- 涉及验证与回归说明时，遵守 testing-policy.md。
