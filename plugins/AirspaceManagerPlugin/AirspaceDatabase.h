#ifndef AIRSPACEDATABASE_H
#define AIRSPACEDATABASE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>

#include "AirspaceTypes.h"

/**
 * @brief 空域数据记录
 */
struct AirspaceRecord {
    QString   id;
    QString   name;
    int       shapeType;
    QString   geoJson;
    QString   styleJson;
    QString   propertiesJson;
    bool      visible;
    QDateTime createdAt;
    QDateTime updatedAt;
};

/**
 * @brief 空域数据库适配器，内部委托给 DBCompt 全局单例
 */
class AirspaceDatabase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)

public:
    explicit AirspaceDatabase(QObject* parent = nullptr);
    ~AirspaceDatabase() override = default;

    Q_INVOKABLE bool open();
    Q_INVOKABLE void close();
    Q_INVOKABLE bool isOpen() const;

    Q_INVOKABLE QString addAirspace(const QString& name, int shapeType,
                                     const QString& geoJson,
                                     const QString& styleJson,
                                     const QString& propertiesJson);

    QString addAirspace(const AirspaceEntity& airspace);

    Q_INVOKABLE bool updateAirspace(const QString& id,
                                     const QString& name, int shapeType,
                                     const QString& geoJson,
                                     const QString& styleJson,
                                     const QString& propertiesJson);

    bool updateAirspace(const AirspaceEntity& airspace);

    Q_INVOKABLE bool removeAirspace(const QString& id);
    Q_INVOKABLE AirspaceRecord getAirspace(const QString& id) const;
    AirspaceEntity getAirspaceEntity(const QString& id) const;
    QList<AirspaceRecord> getAllAirspaces() const;
    QList<AirspaceEntity> getAllAirspaceEntities() const;
    Q_INVOKABLE int  count() const;
    Q_INVOKABLE bool setVisible(const QString& id, bool visible);
    Q_INVOKABLE bool clearAll();
    Q_INVOKABLE QString databasePath() const;

signals:
    void airspaceAdded(const QString& id);
    void airspaceUpdated(const QString& id);
    void airspaceRemoved(const QString& id);
    void databaseCleared();

private:
    static AirspaceRecord rowToRecord(const QVariantMap& row);
    static QString generateUuid();

    static constexpr char TABLE[] = "airspaces";
};

#endif // AIRSPACEDATABASE_H
