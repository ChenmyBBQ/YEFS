#pragma once
#include "../dbcompt_global.h"
#include <QString>

namespace dbcompt {

class DBCOMPT_EXPORT MigrationBackup {
public:
    // 在数据库文件旁边创建带时间戳的备份，outBackupPath 返回备份路径
    static bool backup(const QString& dbPath, QString& outBackupPath, QString& outError);
};

} // namespace dbcompt
