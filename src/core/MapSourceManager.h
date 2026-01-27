#ifndef YEFS_MAPSOURCEMANAGER_H
#define YEFS_MAPSOURCEMANAGER_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QQmlEngine>
#include "IMapSource.h"

namespace YEFS {

/**
 * @brief 地图数据源管理器
 * 
 * 管理所有已加载的地图数据源
 */
class MapSourceManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QList<QObject*> sources READ sourceObjects NOTIFY sourcesChanged)
    Q_PROPERTY(int sourceCount READ sourceCount NOTIFY sourcesChanged)

public:
    static MapSourceManager* instance();
    static MapSourceManager* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    // 数据源管理
    void addSource(IMapSource* source);
    void removeSource(const QString& sourceId);
    void removeAllSources();

    // 查询
    IMapSource* source(const QString& sourceId) const;
    QList<IMapSource*> sources() const { return m_sources.values(); }
    QList<IMapSource*> sourcesByType(MapSourceType type) const;
    int sourceCount() const { return m_sources.count(); }

    // QML 访问
    QList<QObject*> sourceObjects() const;
    Q_INVOKABLE QObject* getSource(const QString& sourceId) const;
    Q_INVOKABLE QStringList sourceIds() const;
    Q_INVOKABLE bool hasSource(const QString& sourceId) const;

    // 文件加载
    Q_INVOKABLE bool loadFile(const QString& filePath);
    Q_INVOKABLE bool loadFiles(const QStringList& filePaths);

    // 在线地图
    Q_INVOKABLE bool addOnlineMap(const QString& name, const QString& urlTemplate, 
                                  int minZoom = 0, int maxZoom = 18);

signals:
    void sourcesChanged();
    void sourceAdded(const QString& sourceId);
    void sourceRemoved(const QString& sourceId);
    void sourceError(const QString& sourceId, const QString& error);

private:
    explicit MapSourceManager(QObject* parent = nullptr);
    ~MapSourceManager() override;

    static MapSourceManager* s_instance;
    QHash<QString, IMapSource*> m_sources;
};

} // namespace YEFS

#endif // YEFS_MAPSOURCEMANAGER_H
