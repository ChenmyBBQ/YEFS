/**
 * @file MapSettings.h
 * @brief 地图配置管理器
 */

#ifndef MAPSETTINGS_H
#define MAPSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QQmlEngine>
#include <QVariantList>

namespace YEFS {

/**
 * @class MapSettings
 * @brief 管理在线地图服务配置
 * 
 * 提供地图服务提供商选择、API Key 配置等功能
 */
class MapSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // 当前选中的地图服务索引
    Q_PROPERTY(int currentProviderIndex READ currentProviderIndex WRITE setCurrentProviderIndex NOTIFY currentProviderIndexChanged)
    
    // 当前地图样式URL（根据配置自动生成）
    Q_PROPERTY(QString styleUrl READ styleUrl NOTIFY styleUrlChanged)
    
    // 可用的地图服务提供商列表
    Q_PROPERTY(QVariantList providers READ providers CONSTANT)
    
    // 自定义样式URL
    Q_PROPERTY(QString customStyleUrl READ customStyleUrl WRITE setCustomStyleUrl NOTIFY customStyleUrlChanged)
    
    // 自定义API Key
    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY apiKeyChanged)

public:
    static MapSettings* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static MapSettings* instance();

    explicit MapSettings(QObject *parent = nullptr);
    ~MapSettings() override;

    int currentProviderIndex() const;
    void setCurrentProviderIndex(int index);

    QString styleUrl() const;
    
    QVariantList providers() const;
    
    QString customStyleUrl() const;
    void setCustomStyleUrl(const QString &url);
    
    QString apiKey() const;
    void setApiKey(const QString &key);

    // 获取指定提供商的样式URL模板
    Q_INVOKABLE QString getProviderStyleUrl(int index) const;
    
    // 获取指定提供商的名称
    Q_INVOKABLE QString getProviderName(int index) const;
    
    // 保存配置
    Q_INVOKABLE void save();
    
    // 重置为默认配置
    Q_INVOKABLE void reset();

signals:
    void currentProviderIndexChanged();
    void styleUrlChanged();
    void customStyleUrlChanged();
    void apiKeyChanged();
    void settingsChanged();

private:
    void load();
    void updateStyleUrl();

    static MapSettings* s_instance;
    
    QSettings m_settings;
    int m_currentProviderIndex;
    QString m_styleUrl;
    QString m_customStyleUrl;
    QString m_apiKey;
    QVariantList m_providers;
};

} // namespace YEFS

#endif // MAPSETTINGS_H
