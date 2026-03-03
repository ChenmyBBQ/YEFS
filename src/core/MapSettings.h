/**
 * @file MapSettings.h
 * @brief 地图配置管理器
 */

#ifndef MAPSETTINGS_H
#define MAPSETTINGS_H

#include <QObject>
#include <QQmlEngine>
#include <QVariantList>
#include <QVariantMap>

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

    // 图源 Key 配置（按类别）
    Q_PROPERTY(QVariantMap sourceKeys READ sourceKeys NOTIFY sourceConfigChanged)

    // 图源显示配置（按类别）
    Q_PROPERTY(QVariantMap sourceVisibility READ sourceVisibility NOTIFY sourceConfigChanged)

    // 图源安全密钥配置（按类别）
    Q_PROPERTY(QVariantMap sourceSecrets READ sourceSecrets NOTIFY sourceConfigChanged)

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

    QVariantMap sourceKeys() const;
    QVariantMap sourceVisibility() const;
    QVariantMap sourceSecrets() const;

    // 获取指定提供商的样式URL模板
    Q_INVOKABLE QString getProviderStyleUrl(int index) const;
    
    // 获取指定提供商的名称
    Q_INVOKABLE QString getProviderName(int index) const;
    
    // 获取缩略图URL
    Q_INVOKABLE QString getThumbnailUrl(int index) const;

    // 获取/设置图源 Key（按类别，例如 MapTiler/Bing）
    Q_INVOKABLE QString getSourceKey(const QString &category) const;
    Q_INVOKABLE void setSourceKey(const QString &category, const QString &key);

    // 获取/设置图源是否显示
    Q_INVOKABLE bool isCategoryVisible(const QString &category) const;
    Q_INVOKABLE void setCategoryVisible(const QString &category, bool visible);

    // 获取/设置图源安全密钥（例如高德 securityJsCode）
    Q_INVOKABLE QString getSourceSecret(const QString &category) const;
    Q_INVOKABLE void setSourceSecret(const QString &category, const QString &secret);
    
    // 保存配置
    Q_INVOKABLE void save();
    
    // 重置为默认配置
    Q_INVOKABLE void reset();

signals:
    void currentProviderIndexChanged();
    void styleUrlChanged();
    void customStyleUrlChanged();
    void apiKeyChanged();
    void sourceConfigChanged();
    void settingsChanged();

private:
    void load();
    void updateStyleUrl();
    QString keyForCategory(const QString &category) const;
    QString secretForCategory(const QString &category) const;

    static MapSettings* s_instance;

    int m_currentProviderIndex;
    QString m_styleUrl;
    QString m_customStyleUrl;
    QString m_apiKey;
    QVariantMap m_sourceKeys;
    QVariantMap m_sourceSecrets;
    QVariantMap m_sourceVisibility;
    QVariantList m_providers;
};

} // namespace YEFS

#endif // MAPSETTINGS_H
