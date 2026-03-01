#ifndef AIRSPACEDATABASE_H
#define AIRSPACEDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QDateTime>

/**
 * @brief 空域形状类型枚举
 */
enum class ShapeType {
    Rectangle  = 0,
    Square     = 1,
    Circle     = 2,
    Polygon    = 3,
    Boundary   = 4,   // 不闭合连线
    Ring       = 5,    // 圆环
    Arc        = 6,    // 圆弧
    Sector     = 7,    // 扇形
    SectorRing = 8     // 扇环
};

/**
 * @brief 空域数据记录
 */
struct AirspaceRecord {
    QString   id;             // UUID
    QString   name;           // 空域名称
    int       shapeType;      // ShapeType 枚举值
    QString   geoJson;        // GeoJSON 字符串
    QString   styleJson;      // 图形样式 JSON
    QString   propertiesJson; // 数据属性 JSON
    bool      visible;
    QDateTime createdAt;
    QDateTime updatedAt;
};

/**
 * @brief 空域 SQLite 数据库管理
 */
class AirspaceDatabase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)

public:
    explicit AirspaceDatabase(QObject* parent = nullptr);
    ~AirspaceDatabase() override;

    /// 打开/创建数据库
    Q_INVOKABLE bool open();

    /// 关闭数据库
    Q_INVOKABLE void close();

    /// 是否已打开
    Q_INVOKABLE bool isOpen() const;

    /// 添加空域（自动生成 UUID）
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

    /// 获取单条空域
    Q_INVOKABLE AirspaceRecord getAirspace(const QString& id) const;

    /// 获取全部空域
    QList<AirspaceRecord> getAllAirspaces() const;

    /// 获取空域计数
    Q_INVOKABLE int count() const;

    /// 设置可见性
    Q_INVOKABLE bool setVisible(const QString& id, bool visible);

    /// 清空全部
    Q_INVOKABLE bool clearAll();

    /// 数据库文件路径
    Q_INVOKABLE QString databasePath() const { return m_dbPath; }

signals:
    void airspaceAdded(const QString& id);
    void airspaceUpdated(const QString& id);
    void airspaceRemoved(const QString& id);
    void databaseCleared();

private:
    bool createTables();
    QString generateUuid() const;

    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_open = false;
};

#endif // AIRSPACEDATABASE_H
