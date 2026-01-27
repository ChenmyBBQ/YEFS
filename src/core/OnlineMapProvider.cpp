#include "OnlineMapProvider.h"
#include <QDebug>
#include <QUuid>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace YEFS {

// ============================================================================
// OnlineTileSource 实现
// ============================================================================

OnlineTileSource::OnlineTileSource(const QString& id, const QString& name, 
                                   const QString& urlTemplate,
                                   int minZoom, int maxZoom,
                                   const QString& attribution,
                                   QObject* parent)
    : IOnlineMapSource(parent)
    , m_id(id)
    , m_name(name)
    , m_urlTemplate(urlTemplate)
    , m_minZoom(minZoom)
    , m_maxZoom(maxZoom)
    , m_attribution(attribution)
{
}

QGeoRectangle OnlineTileSource::bounds() const
{
    // 在线地图通常覆盖全球
    return QGeoRectangle(QGeoCoordinate(85.0, -180.0), 
                         QGeoCoordinate(-85.0, 180.0));
}

QImage OnlineTileSource::tile(int z, int x, int y) const
{
    // TODO: 实现异步瓦片下载
    Q_UNUSED(z)
    Q_UNUSED(x)
    Q_UNUSED(y)
    return QImage();
}

QString OnlineTileSource::tileUrl(int z, int x, int y) const
{
    QString url = m_urlTemplate;
    
    // 替换占位符
    url.replace("{z}", QString::number(z));
    url.replace("{x}", QString::number(x));
    url.replace("{y}", QString::number(y));
    
    // 替换 API key（如果需要）
    if (m_requiresApiKey && !m_apiKey.isEmpty()) {
        url.replace("{key}", m_apiKey);
        url.replace("{apikey}", m_apiKey);
    }
    
    // 替换子域名（简单实现，轮询 a, b, c）
    if (url.contains("{s}")) {
        int subdomain = (x + y + z) % 3;
        QString sub = QString(QChar('a' + subdomain));
        url.replace("{s}", sub);
    }
    
    return url;
}

QVariantMap OnlineTileSource::toMapLibreLayer() const
{
    QVariantMap layer;
    layer["id"] = m_id;
    layer["type"] = "raster";
    
    QVariantMap source;
    source["type"] = "raster";
    source["tiles"] = QStringList{m_urlTemplate};
    source["tileSize"] = m_tileSize;
    source["minzoom"] = m_minZoom;
    source["maxzoom"] = m_maxZoom;
    source["attribution"] = m_attribution;
    
    layer["source"] = source;
    
    return layer;
}

// ============================================================================
// OnlineMapProviderManager 实现
// ============================================================================

OnlineMapProviderManager* OnlineMapProviderManager::s_instance = nullptr;

OnlineMapProviderManager* OnlineMapProviderManager::instance()
{
    if (!s_instance) {
        s_instance = new OnlineMapProviderManager();
    }
    return s_instance;
}

OnlineMapProviderManager* OnlineMapProviderManager::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

OnlineMapProviderManager::OnlineMapProviderManager(QObject* parent)
    : QObject(parent)
{
    initializeProviders();
    qDebug() << "[OnlineMapProviderManager] Initialized with" << m_providers.size() << "providers";
}

void OnlineMapProviderManager::initializeProviders()
{
    // OpenStreetMap
    m_providers[OpenStreetMap] = {
        QStringLiteral("OpenStreetMap"),
        QStringLiteral("开源社区驱动的世界地图"),
        QStringLiteral("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"),
        QStringLiteral("© OpenStreetMap contributors"),
        QUrl(QStringLiteral("https://www.openstreetmap.org/copyright")),
        0, 19, false
    };

    // OpenFreeMap
    m_providers[OpenFreeMap] = {
        QStringLiteral("OpenFreeMap"),
        QStringLiteral("免费的 OpenStreetMap 托管服务"),
        QStringLiteral("https://tiles.openfreemap.org/styles/liberty/{z}/{x}/{y}.png"),
        QStringLiteral("© OpenFreeMap, © OpenStreetMap contributors"),
        QUrl(QStringLiteral("https://openfreemap.org/")),
        0, 19, false
    };

    // MapTiler
    m_providers[MapTiler] = {
        QStringLiteral("MapTiler"),
        QStringLiteral("高质量全球地图服务（需要 API Key）"),
        QStringLiteral("https://api.maptiler.com/maps/streets-v2/{z}/{x}/{y}.png?key={key}"),
        QStringLiteral("© MapTiler © OpenStreetMap contributors"),
        QUrl(QStringLiteral("https://www.maptiler.com/cloud/terms/")),
        0, 20, true
    };

    // Bing Maps
    m_providers[BingMaps] = {
        QStringLiteral("Bing Maps"),
        QStringLiteral("微软 Bing 地图服务（需要 API Key）"),
        QStringLiteral("https://dev.virtualearth.net/REST/v1/Imagery/Metadata/Aerial/{z}?key={key}"),
        QStringLiteral("© Microsoft"),
        QUrl(QStringLiteral("https://www.microsoft.com/maps/product/terms.html")),
        0, 20, true
    };

    // ESRI World Imagery
    m_providers[EsriWorldImagery] = {
        QStringLiteral("ESRI World Imagery"),
        QStringLiteral("Esri 全球卫星影像"),
        QStringLiteral("https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}"),
        QStringLiteral("© Esri, DigitalGlobe, GeoEye, Earthstar Geographics"),
        QUrl(QStringLiteral("https://www.esri.com/en-us/legal/terms/full-master-agreement")),
        0, 19, false
    };

    // CartoDB
    m_providers[CartoDB] = {
        QStringLiteral("CartoDB Positron"),
        QStringLiteral("简洁清爽的地图样式"),
        QStringLiteral("https://{s}.basemaps.cartocdn.com/light_all/{z}/{x}/{y}.png"),
        QStringLiteral("© CartoDB © OpenStreetMap contributors"),
        QUrl(QStringLiteral("https://carto.com/legal/")),
        0, 19, false
    };
}

OnlineTileSource* OnlineMapProviderManager::createProvider(ProviderType type, const QString& apiKey)
{
    if (!m_providers.contains(type)) {
        qWarning() << "[OnlineMapProviderManager] Unknown provider type:" << type;
        return nullptr;
    }

    const ProviderInfo& info = m_providers[type];
    
    if (info.requiresApiKey && apiKey.isEmpty()) {
        qWarning() << "[OnlineMapProviderManager] API key required for provider:" << info.name;
        return nullptr;
    }

    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    auto provider = new OnlineTileSource(id, info.name, info.urlTemplate, 
                                        info.minZoom, info.maxZoom, 
                                        info.attribution, this);
    
    provider->setRequiresApiKey(info.requiresApiKey);
    if (!apiKey.isEmpty()) {
        provider->setApiKey(apiKey);
    }
    provider->setTermsOfServiceUrl(info.tosUrl);

    qDebug() << "[OnlineMapProviderManager] Created provider:" << info.name;
    emit providerCreated(provider);
    
    return provider;
}

OnlineTileSource* OnlineMapProviderManager::createCustomProvider(
    const QString& name,
    const QString& urlTemplate,
    int minZoom,
    int maxZoom,
    const QString& attribution)
{
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    auto provider = new OnlineTileSource(id, name, urlTemplate, 
                                        minZoom, maxZoom, attribution, this);
    
    qDebug() << "[OnlineMapProviderManager] Created custom provider:" << name;
    emit providerCreated(provider);
    
    return provider;
}

QString OnlineMapProviderManager::getProviderName(ProviderType type) const
{
    return m_providers.value(type).name;
}

QString OnlineMapProviderManager::getProviderDescription(ProviderType type) const
{
    return m_providers.value(type).description;
}

bool OnlineMapProviderManager::providerRequiresApiKey(ProviderType type) const
{
    return m_providers.value(type).requiresApiKey;
}

QStringList OnlineMapProviderManager::availableProviders() const
{
    QStringList providers;
    for (const auto& info : m_providers) {
        providers.append(info.name);
    }
    return providers;
}

} // namespace YEFS
