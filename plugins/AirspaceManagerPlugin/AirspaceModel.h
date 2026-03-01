#ifndef AIRSPACEMODEL_H
#define AIRSPACEMODEL_H

#include <QAbstractListModel>
#include <QJsonObject>
#include "AirspaceDatabase.h"

/**
 * @brief 空域列表数据模型
 *
 * 为 QML ListView 提供数据，后端由 AirspaceDatabase 驱动。
 */
class AirspaceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ShapeTypeRole,
        ShapeTypeNameRole,
        GeoJsonRole,
        StyleJsonRole,
        PropertiesJsonRole,
        VisibleRole,
        CreatedAtRole,
        UpdatedAtRole
    };

    explicit AirspaceModel(AirspaceDatabase* db, QObject* parent = nullptr);

    // QAbstractListModel 接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    /// 从数据库重新加载全部记录
    Q_INVOKABLE void loadFromDatabase();

    /// 添加空域并返回 UUID
    Q_INVOKABLE QString addAirspace(const QString& name, int shapeType,
                                     const QString& geoJson,
                                     const QString& styleJson,
                                     const QString& propertiesJson);

    /// 更新空域
    Q_INVOKABLE bool updateAirspace(const QString& id,
                                     const QString& name, int shapeType,
                                     const QString& geoJson,
                                     const QString& styleJson,
                                     const QString& propertiesJson);

    /// 删除空域
    Q_INVOKABLE bool removeAirspace(const QString& id);

    /// 切换可见性
    Q_INVOKABLE bool toggleVisibility(const QString& id);

    /// 获取指定空域数据 (返回 QVariantMap 供 QML 使用)
    Q_INVOKABLE QVariantMap getAirspace(const QString& id) const;

    /// 获取形状类型名称
    Q_INVOKABLE static QString shapeTypeName(int type);

signals:
    void countChanged();
    void airspaceAdded(const QString& id);
    void airspaceUpdated(const QString& id);
    void airspaceRemoved(const QString& id);

private:
    int findIndexById(const QString& id) const;

    AirspaceDatabase* m_db;
    QList<AirspaceRecord> m_records;
};

#endif // AIRSPACEMODEL_H
