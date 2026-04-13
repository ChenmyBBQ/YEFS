#pragma once

namespace YEFS::LogSetup {

// 安装统一日志处理器并注册退出时清理。
void initialize();

// 主动关闭日志文件（quick_exit 前调用）。
void shutdown();

} // namespace YEFS::LogSetup
