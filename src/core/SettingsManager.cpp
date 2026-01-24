/**
 * @file SettingsManager.cpp
 * @brief 全局设置管理器实现
 */

#include "SettingsManager.h"
#include <QDebug>
#include <QJsonArray>

namespace YEFS {

SettingsManager* SettingsManager::s_instance = nullptr;

SettingsManager* SettingsManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

SettingsManager* SettingsManager::instance()
{
    if (!s_instance) {
        s_instance = new SettingsManager();
    }
    return s_instance;
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_dirty(false)
{
    ensureSettingsDir();
    load();
}

SettingsManager::~SettingsManager()
{
    if (m_dirty) {
        save();
    }
}

void SettingsManager::ensureSettingsDir()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_settingsPath = dataPath + "/settings.json";
    qDebug() << "[SettingsManager] Settings file path:" << m_settingsPath;
}

void SettingsManager::load()
{
    QFile file(m_settingsPath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            m_settings = doc.object();
            qDebug() << "[SettingsManager] Settings loaded successfully";
        } else {
            qWarning() << "[SettingsManager] Failed to parse settings:" << error.errorString();
            m_settings = QJsonObject();
        }
    } else {
        qDebug() << "[SettingsManager] No settings file found, using defaults";
        m_settings = QJsonObject();
    }
    
    emit settingsLoaded();
}

void SettingsManager::save()
{
    QFile file(m_settingsPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(m_settings);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        m_dirty = false;
        qDebug() << "[SettingsManager] Settings saved";
        emit settingsSaved();
    } else {
        qWarning() << "[SettingsManager] Failed to save settings:" << file.errorString();
    }
}

void SettingsManager::reload()
{
    load();
}

void SettingsManager::resetAll()
{
    m_settings = QJsonObject();
    m_dirty = true;
    save();
    emit settingsLoaded();
}

void SettingsManager::resetCategory(const QString &category)
{
    if (m_settings.contains(category)) {
        m_settings.remove(category);
        m_dirty = true;
        save();
        emit categoryChanged(category);
    }
}

QVariant SettingsManager::getValue(const QString &category, const QString &key, const QVariant &defaultValue) const
{
    if (!m_settings.contains(category)) {
        return defaultValue;
    }
    
    QJsonObject cat = m_settings[category].toObject();
    if (!cat.contains(key)) {
        return defaultValue;
    }
    
    return cat[key].toVariant();
}

void SettingsManager::setValue(const QString &category, const QString &key, const QVariant &value)
{
    QJsonObject cat;
    if (m_settings.contains(category)) {
        cat = m_settings[category].toObject();
    }
    
    cat[key] = QJsonValue::fromVariant(value);
    m_settings[category] = cat;
    m_dirty = true;
    
    // 自动保存
    save();
    
    emit settingsChanged(category, key);
}

QVariantMap SettingsManager::getCategory(const QString &category) const
{
    if (!m_settings.contains(category)) {
        return QVariantMap();
    }
    
    return m_settings[category].toObject().toVariantMap();
}

void SettingsManager::setCategory(const QString &category, const QVariantMap &values)
{
    m_settings[category] = QJsonObject::fromVariantMap(values);
    m_dirty = true;
    save();
    
    emit categoryChanged(category);
}

QString SettingsManager::settingsFilePath() const
{
    return m_settingsPath;
}

} // namespace YEFS
