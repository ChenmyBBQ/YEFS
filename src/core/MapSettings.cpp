/**
 * @file MapSettings.cpp
 * @brief 地图配置管理器实现
 */

#include "MapSettings.h"
#include <QDebug>

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
    , m_settings("YEFS", "YEFS")
    , m_currentProviderIndex(0)
{
    // 初始化可用的地图服务提供商
    m_providers = {
        QVariantMap{
            {"name", tr("MapLibre Demo")},
            {"description", tr("MapLibre 演示瓦片服务（免费）")},
            {"styleUrl", "https://demotiles.maplibre.org/style.json"},
            {"needsApiKey", false}
        },
        QVariantMap{
            {"name", tr("OpenFreeMap")},
            {"description", tr("OpenStreetMap 免费瓦片服务")},
            {"styleUrl", "https://tiles.openfreemap.org/styles/liberty"},
            {"needsApiKey", false}
        },
        QVariantMap{
            {"name", tr("MapTiler Streets")},
            {"description", tr("MapTiler 街道地图（需要 API Key）")},
            {"styleUrl", "https://api.maptiler.com/maps/streets-v2/style.json?key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("MapTiler Satellite")},
            {"description", tr("MapTiler 卫星地图（需要 API Key）")},
            {"styleUrl", "https://api.maptiler.com/maps/satellite/style.json?key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("MapTiler Basic")},
            {"description", tr("MapTiler 基础地图（需要 API Key）")},
            {"styleUrl", "https://api.maptiler.com/maps/basic-v2/style.json?key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("Stadia Maps Alidade")},
            {"description", tr("Stadia Maps 样式（需要 API Key）")},
            {"styleUrl", "https://tiles.stadiamaps.com/styles/alidade_smooth.json?api_key={apiKey}"},
            {"needsApiKey", true}
        },
        QVariantMap{
            {"name", tr("自定义")},
            {"description", tr("自定义地图样式 URL")},
            {"styleUrl", ""},
            {"needsApiKey", false}
        }
    };
    
    load();
}

MapSettings::~MapSettings()
{
    save();
}

void MapSettings::load()
{
    m_settings.beginGroup("Map");
    m_currentProviderIndex = m_settings.value("providerIndex", 0).toInt();
    m_customStyleUrl = m_settings.value("customStyleUrl", "").toString();
    m_apiKey = m_settings.value("apiKey", "").toString();
    m_settings.endGroup();
    
    // 确保索引有效
    if (m_currentProviderIndex < 0 || m_currentProviderIndex >= m_providers.size()) {
        m_currentProviderIndex = 0;
    }
    
    updateStyleUrl();
}

void MapSettings::save()
{
    m_settings.beginGroup("Map");
    m_settings.setValue("providerIndex", m_currentProviderIndex);
    m_settings.setValue("customStyleUrl", m_customStyleUrl);
    m_settings.setValue("apiKey", m_apiKey);
    m_settings.endGroup();
    m_settings.sync();
    
    qDebug() << "[MapSettings] Configuration saved";
}

void MapSettings::reset()
{
    m_currentProviderIndex = 0;
    m_customStyleUrl.clear();
    m_apiKey.clear();
    
    updateStyleUrl();
    save();
    
    emit currentProviderIndexChanged();
    emit customStyleUrlChanged();
    emit apiKeyChanged();
    emit settingsChanged();
}

int MapSettings::currentProviderIndex() const
{
    return m_currentProviderIndex;
}

void MapSettings::setCurrentProviderIndex(int index)
{
    if (index < 0 || index >= m_providers.size()) {
        return;
    }
    
    if (m_currentProviderIndex != index) {
        m_currentProviderIndex = index;
        updateStyleUrl();
        save();
        
        emit currentProviderIndexChanged();
        emit settingsChanged();
    }
}

QString MapSettings::styleUrl() const
{
    return m_styleUrl;
}

QVariantList MapSettings::providers() const
{
    return m_providers;
}

QString MapSettings::customStyleUrl() const
{
    return m_customStyleUrl;
}

void MapSettings::setCustomStyleUrl(const QString &url)
{
    if (m_customStyleUrl != url) {
        m_customStyleUrl = url;
        
        // 如果当前选中的是自定义提供商，更新样式URL
        if (m_currentProviderIndex == m_providers.size() - 1) {
            updateStyleUrl();
        }
        
        save();
        emit customStyleUrlChanged();
    }
}

QString MapSettings::apiKey() const
{
    return m_apiKey;
}

void MapSettings::setApiKey(const QString &key)
{
    if (m_apiKey != key) {
        m_apiKey = key;
        updateStyleUrl();
        save();
        
        emit apiKeyChanged();
        emit settingsChanged();
    }
}

QString MapSettings::getProviderStyleUrl(int index) const
{
    if (index < 0 || index >= m_providers.size()) {
        return QString();
    }
    
    return m_providers[index].toMap().value("styleUrl").toString();
}

QString MapSettings::getProviderName(int index) const
{
    if (index < 0 || index >= m_providers.size()) {
        return QString();
    }
    
    return m_providers[index].toMap().value("name").toString();
}

void MapSettings::updateStyleUrl()
{
    QString newUrl;
    
    // 最后一个是"自定义"选项
    if (m_currentProviderIndex == m_providers.size() - 1) {
        newUrl = m_customStyleUrl;
    } else {
        QVariantMap provider = m_providers[m_currentProviderIndex].toMap();
        QString templateUrl = provider.value("styleUrl").toString();
        bool needsApiKey = provider.value("needsApiKey").toBool();
        
        if (needsApiKey && !m_apiKey.isEmpty()) {
            newUrl = templateUrl.replace("{apiKey}", m_apiKey);
        } else if (!needsApiKey) {
            newUrl = templateUrl;
        } else {
            // 需要 API Key 但未提供，使用默认
            newUrl = m_providers[0].toMap().value("styleUrl").toString();
            qWarning() << "[MapSettings] API Key required but not provided, using default";
        }
    }
    
    if (m_styleUrl != newUrl) {
        m_styleUrl = newUrl;
        qDebug() << "[MapSettings] Style URL updated:" << m_styleUrl;
        emit styleUrlChanged();
    }
}

} // namespace YEFS
