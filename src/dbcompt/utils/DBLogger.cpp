#include "DBLogger.h"
#include <QDebug>

namespace dbcompt {

void DBLogger::info(const QString& message)    { qInfo()     << "[DBCompt]" << message; }
void DBLogger::warning(const QString& message) { qWarning()  << "[DBCompt]" << message; }
void DBLogger::error(const QString& message)   { qCritical() << "[DBCompt]" << message; }
void DBLogger::debug(const QString& message)   { qDebug()    << "[DBCompt]" << message; }

} // namespace dbcompt
