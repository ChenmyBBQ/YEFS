#include "MapPageStyleController.h"

#include "MapSettings.h"
#include "MessageBus.h"
#include "SettingsManager.h"

namespace YEFS {

MapPageStyleController* MapPageStyleController::s_instance = nullptr;

MapPageStyleController* MapPageStyleController::instance()
{
    if (!s_instance) {
        s_instance = new MapPageStyleController();
    }
    return s_instance;
}

MapPageStyleController* MapPageStyleController::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

void MapPageStyleController::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

MapPageStyleController::MapPageStyleController(QObject* parent)
    : QObject(parent)
{
    m_currentStyleUrl = resolveConfiguredStyleUrl();

    connect(SettingsManager::instance(), &SettingsManager::settingsChanged,
            this, &MapPageStyleController::onSettingsChanged);
}

QString MapPageStyleController::currentStyleUrl() const
{
    return m_currentStyleUrl;
}

bool MapPageStyleController::styleSwitching() const
{
    return m_styleSwitching;
}

bool MapPageStyleController::fallbackTimeout() const
{
    return m_fallbackTimeout;
}

void MapPageStyleController::handleLoadTimeout()
{
    setStyleSwitching(false);
    setFallbackTimeout(true);
    notifyStartupVisualReady(QStringLiteral("map.load-timeout"));
}

bool MapPageStyleController::handleFirstFrameReady()
{
    if (!m_styleSwitching && !m_fallbackTimeout) {
        return false;
    }

    setStyleSwitching(false);
    setFallbackTimeout(false);
    notifyStartupVisualReady(QStringLiteral("map.first-frame-ready"));
    return true;
}

void MapPageStyleController::notifyOverlayHidden()
{
    notifyStartupVisualReady(QStringLiteral("overlay.hidden"));
}

void MapPageStyleController::onSettingsChanged(const QString& category, const QString& key)
{
    if (category == QStringLiteral("map") && key == QStringLiteral("styleUrl")) {
        refreshConfiguredStyle();
    }
}

QString MapPageStyleController::resolveConfiguredStyleUrl() const
{
    QString styleUrl = MapSettings::instance()->styleUrl();
    if (styleUrl.isEmpty()) {
        styleUrl = SettingsManager::instance()->getValue(QStringLiteral("map"), QStringLiteral("styleUrl"), QString()).toString();
    }
    if (styleUrl.isEmpty()) {
        styleUrl = QStringLiteral("https://demotiles.maplibre.org/style.json");
    }
    return styleUrl;
}

void MapPageStyleController::refreshConfiguredStyle(bool forceNotify)
{
    const QString styleUrl = resolveConfiguredStyleUrl();
    if (!forceNotify && m_currentStyleUrl == styleUrl) {
        return;
    }

    m_currentStyleUrl = styleUrl;
    m_startupVisualReadyEmitted = false;
    setStyleSwitching(true);
    setFallbackTimeout(false);
    emit currentStyleUrlChanged();
}

void MapPageStyleController::notifyStartupVisualReady(const QString& reason)
{
    if (m_startupVisualReadyEmitted) {
        return;
    }

    m_startupVisualReadyEmitted = true;
    MessageBus::instance()->send(QStringLiteral("map/startup-visual-ready"), QVariantMap{
        {QStringLiteral("reason"), reason}
    });
}

void MapPageStyleController::setStyleSwitching(bool styleSwitching)
{
    if (m_styleSwitching == styleSwitching) {
        return;
    }

    m_styleSwitching = styleSwitching;
    emit styleSwitchingChanged();
}

void MapPageStyleController::setFallbackTimeout(bool fallbackTimeout)
{
    if (m_fallbackTimeout == fallbackTimeout) {
        return;
    }

    m_fallbackTimeout = fallbackTimeout;
    emit fallbackTimeoutChanged();
}

} // namespace YEFS