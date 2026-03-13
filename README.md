# YEFS

## VS Code 与 GitHub Copilot 规则说明

本仓库已提交面向 VS Code + GitHub Copilot 的仓库级协作规则，目标是让协作者在拉取仓库后，进入工作区即可自动加载统一的开发约束。

已纳入仓库的规则入口包括：

- `.vscode/settings.json`：开启 Copilot 对仓库 instructions、AGENTS、CLAUDE 的读取。
- `.github/instructions/*.instructions.md`：按项目总则、测试、修复流程、C++、QML 等维度提供规则。
- `.github/copilot-instructions.md`：Copilot 主引导。
- `AGENTS.md`：面向支持 AGENTS 约定的工具链。
- `CLAUDE.md`：面向支持 Claude 规则文件的工具链。

当前仓库设置采用相对工作区路径，不再绑定某一台机器的盘符路径，因此适合提交到版本库并供团队共享。

使用前提：

1. 使用 VS Code，并安装 GitHub Copilot / GitHub Copilot Chat。
2. 以仓库根目录作为工作区打开。
3. 本地 Qt SDK、QML Language Server 与构建环境仍需开发者自行安装和配置；仓库只统一 Copilot 规则与工作区内相对路径配置，不托管个人机器的 Qt 安装路径。