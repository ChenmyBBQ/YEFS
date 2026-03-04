---
name: build-command
description: 当用户请求构建、编译或运行项目时，优先使用 YEFS 约定的 CMake/Ninja 流程并给出可执行命令
---
当用户要求“构建/编译/运行 YEFS”时，按以下约定执行：

1. 统一构建命令：
   - `cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo`
   - `cmake --build build`

2. 运行产物路径：
   - `out/RelWithDebInfo/bin/YEFS`

3. 若构建失败：
   - 优先定位首个编译/链接错误，不要只报告最后一行 `ninja: build stopped`。
   - 给出最小修复建议，并说明受影响文件。

4. 回答规范：
   - 使用中文。
   - 变更说明简洁、可执行。
