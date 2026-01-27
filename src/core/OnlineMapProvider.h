#ifndef YEFS_ONLINEMAPPROVIDER_H
#define YEFS_ONLINEMAPPROVIDER_H

#include "IMapSource.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QQmlEngine>

namespace YEFS {

/**
 * @brief 在线地图瓦片源
 */
class OnlineTileSource : public IOnlineMapSource
{
    Q_OBJECT

public:
    explicit OnlineTileSource(const QString& id, const QString& name, 
                             const QString& urlTemplate,
                             int minZoom = 0, int maxZoom = 18,
                             const QString& attribution = QString(),
                             QObject* parent = nullptr);
    ~OnlineTileSource() override = default;

    // IMapSource 接口实现
    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    MapSourceType type() const override { return MapSourceType::Online; }
    bool isLoaded() const override { return true; }
    bool isValid() const override { return !m_urlTemplate.isEmpty(); }
    QGeoRectangle bounds() const override;
    int minZoom() const override { return m_minZoom; }
    int maxZoom() const override { return m_maxZoom; }

    // IOnlineMapSource 接口实现
    QString urlTemplate() const override { return m_urlTemplate; }
    bool requiresApiKey() const override { return m_requiresApiKey; }
    void setApiKey(const QString& key) override { m_apiKey = key; }
    QString attribution() const override { return m_attribution; }
    QUrl termsOfServiceUrl() const override { return m_tosUrl; }

    // IRasterMapSource 接口实现
    QImage tile(int z, int x, int y) const override;
    QString tileUrl(int z, int x, int y) const override;
    QString format() const override { return m_format; }
    int tileSize() const override { return m_tileSize; }

    // 数据访问
    QJsonObject toGeoJSON() const override { return QJsonObject(); }
    QVariantMap toMapLibreLayer() const override;

    // 配置
    void setRequiresApiKey(bool requires) { m_requiresApiKey = requires; }
    void setFormat(const QString& format) { m_format = format; }
    void setTileSize(int size) { m_tileSize = size; }
    void setAttribution(const QString& attribution) { m_attribution = attribution; }
    void setTermsOfServiceUrl(const QUrl& url) { m_tosUrl = url; }

private:
    QString m_id;
    QString m_name;
    QString m_urlTemplate;
    int m_minZoom;
    int m_maxZoom;
    QString m_attribution;
    QUrl m_tosUrl;
    QString m_format = QStringLiteral("png");
    int m_tileSize = 256;
    bool m_requiresApiKey = false;
    QString m_apiKey;
};

/**
 * @brief 在线地图提供商管理器
 */
class OnlineMapProviderManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static OnlineMapProviderManager* instance();
    static OnlineMapProviderManager* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    // 预定义提供商
    enum ProviderType {
        OpenStreetMap,
        OpenFreeMap,
        MapTiler,
        BingMaps,
        EsriWorldImagery,
        CartoDB,
        Custom
    };
    Q_ENUM(ProviderType)

    // 创建预定义提供商
    Q_INVOKABLE OnlineTileSource* createProvider(ProviderType type, const QString& apiKey = QString());
    
    // 创建自定义提供商
    Q_INVOKABLE OnlineTileSource* createCustomProvider(
        const QString& name,
        const QString& urlTemplate,
        int minZoom = 0,
        int maxZoom = 18,
        const QString& attribution = QString());

    // 获取提供商信息
    Q_INVOKABLE QString getProviderName(ProviderType type) const;
    Q_INVOKABLE QString getProviderDescription(ProviderType type) const;
    Q_INVOKABLE bool providerRequiresApiKey(ProviderType type) const;
    Q_INVOKABLE QStringList availableProviders() const;

signals:
    void providerCreated(OnlineTileSource* provider);

private:
    explicit OnlineMapProviderManager(QObject* parent = nullptr);
    ~OnlineMapProviderManager() override = default;

    static OnlineMapProviderManager* s_instance;
    
    struct ProviderInfo {
        QString name;
        QString description;
        QString urlTemplate;
        QString attribution;
        QUrl tosUrl;
        int minZoom;
        int maxZoom;
        bool requiresApiKey;
    };
    
    void initializeProviders();
    QHash<ProviderType, ProviderInfo> m_providers;
};

} // namespace YEFS

#endif // YEFS_ONLINEMAPPROVIDER_H
