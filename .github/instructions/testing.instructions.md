---
name: YEFS 验证规则
description: 测试、构建与回归说明规则
applyTo: "**"
---

- 每次改动至少说明如何验证行为没有被破坏。
- 若修改只涉及 QML 布局或交互，至少提供手工验证路径。
- 若修改涉及 C++ 控制器、数据模型、设置、插件管理或消息分发，应优先补充可自动化验证的测试。
- 用户要求构建、编译、重建、测试或运行 YEFS 时，优先使用 VS Code 的 CMake Tools 能力。
- 若构建失败，优先定位首个真实编译或链接错误，不只报告末尾的构建中止提示。
- 详细规则参见 [.github/copilot/testing-policy.md](../copilot/testing-policy.md)。
