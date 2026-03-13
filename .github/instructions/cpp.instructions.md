---
name: YEFS C++ 规则
description: C++ 核心逻辑、内存管理与接口修改规则
applyTo: "{src/**,plugins/**}/**/*.{h,hpp,c,cc,cpp,cxx}"
---

- 当前项目是 Qt6/QML/C++ 桌面 GIS 应用，核心 C++ 代码遵循现代 C++ 与 Qt6 适配规则。
- 优先使用 RAII、const/constexpr、类型安全与清晰简单的结构。
- 禁止手写零散的 new/delete 进行裸管理；QObject 且传入 parent 的场景除外。
- 修改函数签名、重命名符号或分析引用时，优先使用语义级 C++ 导航工具而不是纯文本搜索。
- 涉及 C++ 核心逻辑、对象生命周期与资源管理时，以 C++ 规则为最高优先级之一。
- 详细规则参见 [.github/copilot/cpp-rules.md](../copilot/cpp-rules.md)。
