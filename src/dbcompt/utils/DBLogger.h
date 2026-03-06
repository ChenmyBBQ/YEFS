#pragma once
#include "../dbcompt_global.h"
#include <QString>

namespace dbcompt {

class DBCOMPT_EXPORT DBLogger {
public:
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);
    static void debug(const QString& message);
};

} // namespace dbcompt
