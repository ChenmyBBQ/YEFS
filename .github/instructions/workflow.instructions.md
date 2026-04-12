---
name: YEFS 修复流程
description: "Use when: diagnosing bugs, crashes, abnormal behavior, blockers, log analysis, root cause analysis, fix workflow, deciding edit scope and confirmation boundary"
applyTo: "**"
---

- 处理缺陷、异常行为或功能阻塞时，优先查看日志与现象，再结合源码定位问题，避免猜测式修改。
- 先定位根因，再决定修改路径，避免做表面补丁。
- 回答中优先说明问题原因，再说明修复方案、影响范围与验证方式。
- 普通实现、小范围修复、非破坏性改动可以直接落地；涉及结构重组、规则体系重写或明显歧义时，应先确认。
- 详细规则参见 [.github/copilot/workflow-rules.md](../copilot/workflow-rules.md)。