#include "MapSourceManager.h"
#include "IMapParser.h"
#include <QDebug>
#include <QFileInfo>

namespace YEFS {

MapSourceManager* MapSourceManager::s_instance = nullptr;

MapSourceManager* MapSourceManager::instance()
{
    if (!s_instance) {
        s_instance = new MapSourceManager();
    }
    return s_instance;
}

MapSourceManager* MapSourceManager::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

MapSourceManager::MapSourceManager(QObject* parent)
    : QObject(parent)
{
    qDebug() << "[MapSourceManager] Initialized";
}

MapSourceManager::~MapSourceManager()
{
    removeAllSources();
}

void MapSourceManager::addSource(IMapSource* source)
{
    if (!source) {
        qWarning() << "[MapSourceManager] Attempting to add null source";
        return;
    }

    QString id = source->id();
    if (m_sources.contains(id)) {
        qWarning() << "[MapSourceManager] Source already exists:" << id;
        return;
    }

    source->setParent(this);
    m_sources.insert(id, source);

    // 连接信号
    connect(source, &IMapSource::error, this, [this, id](const QString& message) {
        emit sourceError(id, message);
    });

    qDebug() << "[MapSourceManager] Added source:" << id << "type:" << (int)source->type();
    emit sourceAdded(id);
    emit sourcesChanged();
}

void MapSourceManager::removeSource(const QString& sourceId)
{
    if (!m_sources.contains(sourceId)) {
        qWarning() << "[MapSourceManager] Source not found:" << sourceId;
        return;
    }

    auto source = m_sources.take(sourceId);
    if (source) {
        source->deleteLater();
    }

    qDebug() << "[MapSourceManager] Removed source:" << sourceId;
    emit sourceRemoved(sourceId);
    emit sourcesChanged();
}

void MapSourceManager::removeAllSources()
{
    QStringList ids = m_sources.keys();
    for (const QString& id : ids) {
        removeSource(id);
    }
}

IMapSource* MapSourceManager::source(const QString& sourceId) const
{
    return m_sources.value(sourceId, nullptr);
}

QList<IMapSource*> MapSourceManager::sourcesByType(MapSourceType type) const
{
    QList<IMapSource*> result;
    for (IMapSource* source : m_sources) {
        if (source->type() == type) {
            result.append(source);
        }
    }
    return result;
}

QList<QObject*> MapSourceManager::sourceObjects() const
{
    QList<QObject*> objects;
    for (IMapSource* source : m_sources) {
        objects.append(source);
    }
    return objects;
}

QObject* MapSourceManager::getSource(const QString& sourceId) const
{
    return source(sourceId);
}

QStringList MapSourceManager::sourceIds() const
{
    return m_sources.keys();
}

bool MapSourceManager::hasSource(const QString& sourceId) const
{
    return m_sources.contains(sourceId);
}

bool MapSourceManager::loadFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "[MapSourceManager] File does not exist:" << filePath;
        return false;
    }

    // 使用解析器工厂解析文件
    MapParserFactory* factory = MapParserFactory::instance();
    IMapSource* source = factory->parseFile(filePath);

    if (!source) {
        qWarning() << "[MapSourceManager] Failed to parse file:" << filePath;
        return false;
    }

    addSource(source);
    qDebug() << "[MapSourceManager] Loaded file:" << filePath << "as source:" << source->id();
    return true;
}

bool MapSourceManager::loadFiles(const QStringList& filePaths)
{
    bool allSuccess = true;
    for (const QString& path : filePaths) {
        if (!loadFile(path)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

bool MapSourceManager::addOnlineMap(const QString& name, const QString& urlTemplate, 
                                    int minZoom, int maxZoom)
{
    // TODO: 实现在线地图数据源创建
    Q_UNUSED(name)
    Q_UNUSED(urlTemplate)
    Q_UNUSED(minZoom)
    Q_UNUSED(maxZoom)
    qWarning() << "[MapSourceManager] Online map creation not yet implemented";
    return false;
}

} // namespace YEFS
