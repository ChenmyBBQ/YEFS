#include "logsetup.h"

#include <QDateTime>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QLoggingCategory>
#include <cstdlib>

namespace {

QFile *s_logFile = nullptr;
QtMessageHandler s_defaultHandler = nullptr;

QString logLevelText(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg: return QStringLiteral("DEBUG");
    case QtInfoMsg: return QStringLiteral("INFO");
    case QtWarningMsg: return QStringLiteral("WARN");
    case QtCriticalMsg: return QStringLiteral("ERROR");
    case QtFatalMsg: return QStringLiteral("FATAL");
    }
    return QStringLiteral("UNKNOWN");
}

QString simplifiedFunctionName(const char *function)
{
    if (!function || function[0] == '\0')
        return QStringLiteral("unknown");

    QString fn = QString::fromUtf8(function);
    const int leftParen = fn.indexOf('(');
    if (leftParen > 0)
        fn = fn.left(leftParen);

    const int lastScope = fn.lastIndexOf(QStringLiteral("::"));
    if (lastScope >= 0)
        return fn.mid(lastScope + 2);

    return fn;
}

QString normalizedModuleName(const QMessageLogContext &ctx)
{
    if (ctx.category && ctx.category[0] != '\0') {
        const QString category = QString::fromUtf8(ctx.category);
        if (category != QStringLiteral("default"))
            return category;
    }
    return QStringLiteral("MainApp");
}

void closeLogFile()
{
    if (s_logFile) {
        s_logFile->close();
        delete s_logFile;
        s_logFile = nullptr;
    }
}

void fileMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    // 过滤 Linux 平台不支持窗口透明度的警告
    if (type == QtWarningMsg && msg.contains(QStringLiteral("does not support setting window opacity")))
        return;

    if (s_logFile && s_logFile->isOpen()) {
        QTextStream out(s_logFile);
        const QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        const QString level = logLevelText(type);
        const QString module = normalizedModuleName(ctx);
        const QString functionName = simplifiedFunctionName(ctx.function);
#ifdef YEFS_VERSION
        const QString version = QStringLiteral(YEFS_VERSION);
#else
        const QString version = QStringLiteral("unknown");
#endif
        out << timestamp
            << " [" << level << "]"
            << " [M:" << module << "]"
            << " [FN:" << functionName << "]"
            << " [V:" << version << "] "
            << msg << "\n";
        out.flush();
    }

    if (s_defaultHandler)
        s_defaultHandler(type, ctx, msg);
}

} // namespace

namespace YEFS::LogSetup {

void initialize()
{
    if (s_logFile)
        return;

    s_logFile = new QFile(QStringLiteral("yefs_debug.log"));
    s_logFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

    s_defaultHandler = qInstallMessageHandler(fileMessageHandler);
    std::atexit(closeLogFile);
}

} // namespace YEFS::LogSetup
