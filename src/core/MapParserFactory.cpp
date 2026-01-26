#include "IMapParser.h"
#include "IMapSource.h"
#include <QFileInfo>
#include <QDebug>

namespace YEFS {

MapParserFactory* MapParserFactory::s_instance = nullptr;

MapParserFactory* MapParserFactory::instance()
{
    if (!s_instance) {
        s_instance = new MapParserFactory();
    }
    return s_instance;
}

MapParserFactory* MapParserFactory::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

MapParserFactory::MapParserFactory(QObject* parent)
    : QObject(parent)
{
    qDebug() << "[MapParserFactory] Initialized";
}

void MapParserFactory::registerParser(IMapParser* parser)
{
    if (!parser) {
        qWarning() << "[MapParserFactory] Attempting to register null parser";
        return;
    }

    QString name = parser->name();
    if (m_parsers.contains(name)) {
        qWarning() << "[MapParserFactory] Parser already registered:" << name;
        return;
    }

    parser->setParent(this);
    m_parsers.insert(name, parser);
    qDebug() << "[MapParserFactory] Registered parser:" << name 
             << "for extensions:" << parser->supportedExtensions();
    
    emit parserRegistered(name);
}

void MapParserFactory::unregisterParser(const QString& name)
{
    if (!m_parsers.contains(name)) {
        qWarning() << "[MapParserFactory] Parser not found:" << name;
        return;
    }

    auto parser = m_parsers.take(name);
    if (parser) {
        parser->deleteLater();
    }

    qDebug() << "[MapParserFactory] Unregistered parser:" << name;
    emit parserUnregistered(name);
}

IMapParser* MapParserFactory::parser(const QString& name) const
{
    return m_parsers.value(name, nullptr);
}

IMapParser* MapParserFactory::parserForFile(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    for (IMapParser* parser : m_parsers) {
        if (parser->supportedExtensions().contains(extension, Qt::CaseInsensitive)) {
            if (parser->canParse(filePath)) {
                return parser;
            }
        }
    }

    return nullptr;
}

IMapParser* MapParserFactory::parserForExtension(const QString& extension) const
{
    QString ext = extension.toLower();
    if (ext.startsWith('.')) {
        ext = ext.mid(1);
    }

    for (IMapParser* parser : m_parsers) {
        if (parser->supportedExtensions().contains(ext, Qt::CaseInsensitive)) {
            return parser;
        }
    }

    return nullptr;
}

QStringList MapParserFactory::supportedExtensions() const
{
    QStringList extensions;
    for (IMapParser* parser : m_parsers) {
        extensions << parser->supportedExtensions();
    }
    extensions.removeDuplicates();
    extensions.sort();
    return extensions;
}

QStringList MapParserFactory::supportedMimeTypes() const
{
    QStringList mimeTypes;
    for (IMapParser* parser : m_parsers) {
        mimeTypes << parser->mimeTypes();
    }
    mimeTypes.removeDuplicates();
    mimeTypes.sort();
    return mimeTypes;
}

IMapSource* MapParserFactory::parseFile(const QString& filePath)
{
    IMapParser* parser = parserForFile(filePath);
    if (!parser) {
        qWarning() << "[MapParserFactory] No parser found for file:" << filePath;
        return nullptr;
    }

    qDebug() << "[MapParserFactory] Parsing file:" << filePath << "with parser:" << parser->name();
    return parser->parse(filePath);
}

} // namespace YEFS
