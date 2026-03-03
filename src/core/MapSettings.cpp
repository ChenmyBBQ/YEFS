/**
 * @file MapSettings.cpp
 * @brief 地图配置管理器实现
 */

#include "MapSettings.h"
#include "SettingsManager.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace YEFS {

MapSettings* MapSettings::s_instance = nullptr;

MapSettings* MapSettings::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

MapSettings* MapSettings::instance()
{
    if (!s_instance) {
        s_instance = new MapSettings();
    }
    return s_instance;
}

MapSettings::MapSettings(QObject *parent)
    : QObject(parent)
    , m_currentProviderIndex(0)
{
    // 初始化可用的地图服务提供商
    m_providers = {
        QVariantMap{
            {"name", tr("MapTiler 街道")},
            {"category", "MapTiler"},
            {"type", "vector"},
            {"styleUrl", "https://api.maptiler.com/maps/streets-v2/style.json?key={apiKey}"},
            {"thumbnailUrl", "https://api.maptiler.com/maps/streets-v2/256/0/0/0.png?key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("MapTiler 卫星")},
            {"category", "MapTiler"},
            {"type", "vector"},
            {"styleUrl", "https://api.maptiler.com/maps/hybrid/style.json?key={apiKey}"},
            {"thumbnailUrl", "https://api.maptiler.com/maps/hybrid/256/0/0/0.png?key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("MapTiler 拓扑")},
            {"category", "MapTiler"},
            {"type", "vector"},
            {"styleUrl", "https://api.maptiler.com/maps/topo-v2/style.json?key={apiKey}"},
            {"thumbnailUrl", "https://api.maptiler.com/maps/topo-v2/256/0/0/0.png?key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("MapTiler 极简")},
            {"category", "MapTiler"},
            {"type", "vector"},
            {"styleUrl", "https://api.maptiler.com/maps/basic-v2/style.json?key={apiKey}"},
            {"thumbnailUrl", "https://api.maptiler.com/maps/basic-v2/256/0/0/0.png?key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("高德 矢量")},
            {"category", "高德"},
            {"type", "raster"},
            {"urlTemplate", "https://webrd01.is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=8&x={x}&y={y}&z={z}&key={apiKey}&jscode={securityCode}"},
            {"thumbnailUrl", "https://webrd01.is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=8&x=843&y=388&z=10&key={apiKey}&jscode={securityCode}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("高德 卫星")},
            {"category", "高德"},
            {"type", "raster"},
            {"urlTemplate", "https://webst01.is.autonavi.com/appmaptile?style=6&x={x}&y={y}&z={z}&key={apiKey}&jscode={securityCode}"},
            {"thumbnailUrl", "https://webst01.is.autonavi.com/appmaptile?style=6&x=843&y=388&z=10&key={apiKey}&jscode={securityCode}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("Google 矢量")},
            {"category", "Google"},
            {"type", "raster"},
            {"urlTemplate", "https://mt0.google.com/vt/lyrs=m&hl=zh-CN&x={x}&y={y}&z={z}"},
            {"thumbnailUrl", "https://mt0.google.com/vt/lyrs=m&hl=zh-CN&x=843&y=388&z=10"},
            {"needsApiKey", false}
        },
        QVariantMap{
            {"name", tr("Google 卫星")},
            {"category", "Google"},
            {"type", "raster"},
            {"urlTemplate", "https://mt0.google.com/vt/lyrs=s&hl=zh-CN&x={x}&y={y}&z={z}"},
            {"thumbnailUrl", "https://mt0.google.com/vt/lyrs=s&hl=zh-CN&x=843&y=388&z=10"},
            {"needsApiKey", false}
        },
        QVariantMap{
            {"name", tr("Bing 路网")},
            {"category", "Bing"},
            {"type", "raster"},
            {"urlTemplate", "https://ecn.t3.tiles.virtualearth.net/tiles/r{quadkey}.jpeg?g=1&key={apiKey}"},
            {"thumbnailUrl", "https://ecn.t3.tiles.virtualearth.net/tiles/r1321222230.jpeg?g=1&key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("Bing 卫星")},
            {"category", "Bing"},
            {"type", "raster"},
            {"urlTemplate", "https://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=1&key={apiKey}"},
            {"thumbnailUrl", "https://ecn.t3.tiles.virtualearth.net/tiles/a1321222230.jpeg?g=1&key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("自定义")},
            {"category", "自定义"},
            {"type", "vector"},
            {"styleUrl", ""},
            {"thumbnailUrl", ""},
            {"needsApiKey", false}
        }
    };
    
    load();
}

MapSettings::~MapSettings() { save(); }

void MapSettings::load()
{
    SettingsManager* settings = SettingsManager::instance();
    m_currentProviderIndex = settings->getValue("map", "providerIndex", 0).toInt();
    m_customStyleUrl = settings->getValue("map", "customStyleUrl", "").toString();

    QVariantMap defaultSourceKeys;
    defaultSourceKeys.insert("MapTiler", "pIYKyqRw5KwCNhksntqa");
    defaultSourceKeys.insert("Bing", "");
    defaultSourceKeys.insert("Google", "");
    defaultSourceKeys.insert("高德", "");

    QVariantMap defaultSourceSecrets;
    defaultSourceSecrets.insert("高德", "");

    QVariantMap defaultSourceVisibility;
    defaultSourceVisibility.insert("MapTiler", true);
    defaultSourceVisibility.insert("Bing", true);
    defaultSourceVisibility.insert("Google", true);
    defaultSourceVisibility.insert("高德", true);

    m_sourceKeys = settings->getValue("map", "sourceKeys", defaultSourceKeys).toMap();
    m_sourceSecrets = settings->getValue("map", "sourceSecrets", defaultSourceSecrets).toMap();
    m_sourceVisibility = settings->getValue("map", "sourceVisibility", defaultSourceVisibility).toMap();

    // 兼容旧版 apiKey 字段
    const QString legacyApiKey = settings->getValue("map", "apiKey", "").toString();
    if (m_sourceKeys.value("MapTiler").toString().isEmpty()) {
        m_sourceKeys.insert("MapTiler", legacyApiKey.isEmpty() ? "pIYKyqRw5KwCNhksntqa" : legacyApiKey);
    }
    if (!m_sourceSecrets.contains("高德")) {
        m_sourceSecrets.insert("高德", "");
    }

    m_apiKey = m_sourceKeys.value("MapTiler").toString();
    
    if (m_currentProviderIndex < 0 || m_currentProviderIndex >= m_providers.size()) {
        m_currentProviderIndex = 0;
    }

    // 若当前选择 Bing 且仍使用默认 MapTiler Key，则回退到可用默认底图
    if (m_currentProviderIndex >= 0 && m_currentProviderIndex < m_providers.size()) {
        const QVariantMap provider = m_providers[m_currentProviderIndex].toMap();
        const QString category = provider.value("category").toString();
        if (category == "Bing") {
            const bool keyInvalid = m_apiKey.isEmpty() || m_apiKey == "pIYKyqRw5KwCNhksntqa" || m_apiKey.length() < 8;
            if (keyInvalid) {
                m_currentProviderIndex = 0;
            }
        }
    }

    updateStyleUrl();
}

void MapSettings::save()
{
    SettingsManager* settings = SettingsManager::instance();
    settings->setValue("map", "providerIndex", m_currentProviderIndex);
    settings->setValue("map", "customStyleUrl", m_customStyleUrl);
    settings->setValue("map", "apiKey", m_sourceKeys.value("MapTiler").toString());
    settings->setValue("map", "sourceKeys", m_sourceKeys);
    settings->setValue("map", "sourceSecrets", m_sourceSecrets);
    settings->setValue("map", "sourceVisibility", m_sourceVisibility);
    settings->setValue("map", "styleUrl", m_styleUrl);
    
    qDebug() << "[MapSettings] Configuration saved";
}

void MapSettings::reset()
{
    m_currentProviderIndex = 0;
    m_customStyleUrl.clear();
    m_sourceKeys.insert("MapTiler", "pIYKyqRw5KwCNhksntqa");
    m_sourceKeys.insert("Bing", "");
    m_sourceKeys.insert("Google", "");
    m_sourceKeys.insert("高德", "");
    m_sourceSecrets.insert("高德", "");

    m_sourceVisibility.insert("MapTiler", true);
    m_sourceVisibility.insert("Bing", true);
    m_sourceVisibility.insert("Google", true);
    m_sourceVisibility.insert("高德", true);

    m_apiKey = m_sourceKeys.value("MapTiler").toString();
    updateStyleUrl();
    save();
    
    emit currentProviderIndexChanged();
    emit customStyleUrlChanged();
    emit apiKeyChanged();
    emit sourceConfigChanged();
    emit settingsChanged();
}

int MapSettings::currentProviderIndex() const { return m_currentProviderIndex; }

void MapSettings::setCurrentProviderIndex(int index)
{
    if (index < 0 || index >= m_providers.size()) return;
    if (m_currentProviderIndex != index) {
        m_currentProviderIndex = index;
        updateStyleUrl();
        save();
        emit currentProviderIndexChanged();
        emit settingsChanged();
    }
}

QString MapSettings::styleUrl() const { return m_styleUrl; }
QVariantList MapSettings::providers() const { return m_providers; }
QString MapSettings::customStyleUrl() const { return m_customStyleUrl; }

void MapSettings::setCustomStyleUrl(const QString &url)
{
    if (m_customStyleUrl != url) {
        m_customStyleUrl = url;
        if (m_currentProviderIndex == m_providers.size() - 1) updateStyleUrl();
        save();
        emit customStyleUrlChanged();
    }
}

QString MapSettings::apiKey() const { return m_apiKey; }

void MapSettings::setApiKey(const QString &key)
{
    setSourceKey("MapTiler", key);
}

QVariantMap MapSettings::sourceKeys() const { return m_sourceKeys; }

QVariantMap MapSettings::sourceVisibility() const { return m_sourceVisibility; }

QVariantMap MapSettings::sourceSecrets() const { return m_sourceSecrets; }

QString MapSettings::getProviderStyleUrl(int index) const
{
    if (index < 0 || index >= m_providers.size()) return QString();
    return m_providers[index].toMap().value("styleUrl").toString();
}

QString MapSettings::getProviderName(int index) const
{
    if (index < 0 || index >= m_providers.size()) return QString();
    return m_providers[index].toMap().value("name").toString();
}

QString MapSettings::getThumbnailUrl(int index) const
{
    if (index < 0 || index >= m_providers.size()) return QString();
    
    QVariantMap provider = m_providers[index].toMap();
    QString url = provider.value("thumbnailUrl").toString();
    if (provider.value("needsApiKey").toBool()) {
        const QString category = provider.value("category").toString();
        QString apiKey = keyForCategory(category);
        QString secret = secretForCategory(category);
        url.replace("{apiKey}", apiKey);
        url.replace("{securityCode}", secret);
    }
    return url;
}

QString MapSettings::getSourceKey(const QString &category) const
{
    return m_sourceKeys.value(category).toString();
}

void MapSettings::setSourceKey(const QString &category, const QString &key)
{
    const QString oldValue = m_sourceKeys.value(category).toString();
    if (oldValue == key) {
        return;
    }

    m_sourceKeys.insert(category, key);
    m_apiKey = m_sourceKeys.value("MapTiler").toString();

    updateStyleUrl();
    save();

    emit apiKeyChanged();
    emit sourceConfigChanged();
    emit settingsChanged();
}

QString MapSettings::getSourceSecret(const QString &category) const
{
    return m_sourceSecrets.value(category).toString();
}

void MapSettings::setSourceSecret(const QString &category, const QString &secret)
{
    const QString oldValue = m_sourceSecrets.value(category).toString();
    if (oldValue == secret) {
        return;
    }

    m_sourceSecrets.insert(category, secret);

    updateStyleUrl();
    save();

    emit sourceConfigChanged();
    emit settingsChanged();
}

bool MapSettings::isCategoryVisible(const QString &category) const
{
    return m_sourceVisibility.value(category, true).toBool();
}

void MapSettings::setCategoryVisible(const QString &category, bool visible)
{
    const bool oldValue = m_sourceVisibility.value(category, true).toBool();
    if (oldValue == visible) {
        return;
    }

    m_sourceVisibility.insert(category, visible);

    // 若当前类别被隐藏，则回到默认底图
    if (!visible && m_currentProviderIndex >= 0 && m_currentProviderIndex < m_providers.size()) {
        const QVariantMap provider = m_providers[m_currentProviderIndex].toMap();
        if (provider.value("category").toString() == category) {
            m_currentProviderIndex = 0;
            emit currentProviderIndexChanged();
        }
    }

    updateStyleUrl();
    save();

    emit sourceConfigChanged();
    emit settingsChanged();
}

void MapSettings::updateStyleUrl()
{
    if (m_currentProviderIndex < 0 || m_currentProviderIndex >= m_providers.size()) return;
    
    QString newUrl;
    
    // 如果是自定义服务
    if (m_currentProviderIndex == m_providers.size() - 1) {
        newUrl = m_customStyleUrl;
    } else {
        QVariantMap provider = m_providers[m_currentProviderIndex].toMap();
        QString type = provider.value("type").toString();
        QString category = provider.value("category").toString();
        bool needsApiKey = provider.value("needsApiKey").toBool();
        QString apiKey = keyForCategory(category);
        QString secret = secretForCategory(category);

        if (type == "vector") {
            QString templateUrl = provider.value("styleUrl").toString();
            newUrl = needsApiKey ? templateUrl.replace("{apiKey}", apiKey) : templateUrl;
        } else if (type == "raster") {
            // == 核心逻辑 ==
            // 针对高德/Google/Bing，将 XYZ 瓦片服务包裹为 MapLibre 标准 style.json ！
            QString templateUrl = provider.value("urlTemplate").toString();
            if (needsApiKey) {
                templateUrl.replace("{apiKey}", apiKey);
                templateUrl.replace("{securityCode}", secret);
            }

            QJsonObject root;
            root["version"] = 8;
            root["name"] = "Raster Base Map";

            QJsonObject sources;
            QJsonObject source;
            source["type"] = "raster";
            QJsonArray tiles;
            tiles.append(templateUrl);
            source["tiles"] = tiles;
            source["tileSize"] = 256;
            sources["raster-tiles"] = source;
            root["sources"] = sources;

            QJsonArray layers;
            
            // 1. 底色层（防止无数据区域漆黑）
            QJsonObject bgLayer;
            bgLayer["id"] = "background";
            bgLayer["type"] = "background";
            QJsonObject bgPaint;
            bgPaint["background-color"] = "#E0E0E0";
            bgLayer["paint"] = bgPaint;
            layers.append(bgLayer);

            // 2. 图像层
            QJsonObject layer;
            layer["id"] = "simple-tiles";
            layer["type"] = "raster";
            layer["source"] = "raster-tiles";
            layer["minzoom"] = 0;
            layer["maxzoom"] = 22;
            layers.append(layer);
            
            root["layers"] = layers;

            QJsonDocument doc(root);
            newUrl = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        }
    }
    
    if (m_styleUrl != newUrl) {
        m_styleUrl = newUrl;
        qDebug() << "[MapSettings] BaseMap Style URL updated:" << m_styleUrl;
        emit styleUrlChanged();
    }
}

QString MapSettings::keyForCategory(const QString &category) const
{
    QString key = m_sourceKeys.value(category).toString();
    if (category == "MapTiler") {
        if (key.isEmpty()) {
            key = "pIYKyqRw5KwCNhksntqa";
        }
    }
    return key;
}

QString MapSettings::secretForCategory(const QString &category) const
{
    return m_sourceSecrets.value(category).toString();
}

} // namespace YEFS
