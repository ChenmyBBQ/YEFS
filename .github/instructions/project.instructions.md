---
name: YEFS 项目总则
description: "Use when: working anywhere in YEFS, applying global project constraints, Qt6/QML/C++ desktop GIS architecture, module boundaries, MessageBus, SettingsManager, PluginManager, startup chain"
applyTo: "**"
---

- 所有会话输出、解释、计划、错误分析与代码注释默认使用中文。
- 当前主工程是 Qt6/QML/C++ 桌面 GIS 应用，新增规则与修改不得反向破坏这一主架构。
- 启动链路与导航骨架保持稳定：src/cpp/main.cpp 启动应用并加载 Main.qml，页面优先接入既有菜单驱动结构。
- 主体分层保持清晰：src/core 承载核心逻辑，src/qml 承载表现层，plugins 承载扩展，3rdparty 视为外部依赖。
- 优先分析日志、现象与根因，再实施修改，避免表面补丁式修复。
- 复杂业务逻辑放在 C++ 控制器、模型或核心服务，不堆到 QML 页面中。
- 跨模块事件优先通过 MessageBus 协调，设置持久化统一经 SettingsManager，插件统一经 PluginManager 管理。
- 构建、编译、测试、运行与重建优先使用 VS Code 的 CMake Tools 能力。
- 对当前项目，优先遵守 [.github/copilot/architecture.md](../copilot/architecture.md) 与 [.github/copilot/workflow-rules.md](../copilot/workflow-rules.md) 中的约束。
