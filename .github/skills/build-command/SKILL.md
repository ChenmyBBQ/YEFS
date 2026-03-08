---
name: build-command
description: 当用户请求构建、编译或运行项目时，优先使用 VS Code 的 CMake Tools 扩展，不要退回终端 cmake 命令
---
当用户要求“构建/编译/运行 YEFS”时，按以下约定执行：

1. 构建与测试优先级：
   - 第一选择：使用 VS Code 的 CMake Tools 扩展。
   - 构建时优先调用 CMake Tools 的 build 能力，而不是终端里的 `cmake --build`、`ninja`、`make`。
   - 测试时优先调用 CMake Tools 的 CTest 能力，而不是终端里的 `ctest`。

2. 仅在解释项目产物位置时，说明运行产物路径：
   - `out/RelWithDebInfo/bin/YEFS`

3. 若构建失败：
   - 优先定位首个编译/链接错误，不要只报告最后一行 `ninja: build stopped`。
   - 给出最小修复建议，并说明受影响文件。

4. 若必须给出命令说明：
   - 只把终端 `cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo` 视为初始化或独立环境说明。
   - 不把终端构建命令作为默认执行路径。

5. 回答规范：
   - 使用中文。
   - 变更说明简洁、可执行。
