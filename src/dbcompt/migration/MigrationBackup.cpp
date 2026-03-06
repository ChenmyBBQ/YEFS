#include "MigrationBackup.h"
#include <QFile>
#include <QDateTime>

namespace dbcompt {

bool MigrationBackup::backup(const QString& dbPath, QString& outBackupPath, QString& outError)
{
    const QString ts = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    outBackupPath = dbPath + QStringLiteral(".bak.") + ts;

    if (!QFile::copy(dbPath, outBackupPath)) {
        outError = QStringLiteral("备份失败: ") + dbPath + QStringLiteral(" -> ") + outBackupPath;
        return false;
    }
    return true;
}

} // namespace dbcompt
