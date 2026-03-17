#include "EditRuntime.h"

#include "MapLibreEngine.h"
#include "MessageBus.h"

#include <QVariantMap>
#include <QUuid>

namespace YEFS {

EditRuntime* EditRuntime::s_instance = nullptr;

EditRuntime::EditRuntime(QObject* parent)
    : QObject(parent)
{
    connect(MessageBus::instance(), &MessageBus::message,
            this, &EditRuntime::onBusMessage);
}

EditRuntime* EditRuntime::instance()
{
    if (!s_instance) {
        s_instance = new EditRuntime();
    }
    return s_instance;
}

EditRuntime* EditRuntime::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

void EditRuntime::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

bool EditRuntime::beginAirspaceEditSession(int shapeType)
{
    if (shapeType < 0) {
        return false;
    }

    if (!m_sessionActive) {
        m_sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    m_sessionActive = true;
    m_shapeType = shapeType;
    emit sessionStateChanged();

    MessageBus::instance()->send(QStringLiteral("airspace-manager/draw"), QVariantMap{
        { QStringLiteral("shapeType"), shapeType }
    });
    return true;
}

bool EditRuntime::addControlPoint(double latitude, double longitude)
{
    if (!m_sessionActive) {
        return false;
    }

    MapLibreEngine::instance()->onMapClicked(latitude, longitude);
    return true;
}

bool EditRuntime::undoLastControlPoint()
{
    if (!m_sessionActive) {
        return false;
    }

    MessageBus::instance()->send(QStringLiteral("airspace-manager/undo"), QVariantMap{});
    return true;
}

bool EditRuntime::updateHoverPreview(double latitude, double longitude)
{
    if (!m_sessionActive) {
        return false;
    }

    MessageBus::instance()->send(QStringLiteral("map/hovered"), QVariantMap{
        { QStringLiteral("latitude"), latitude },
        { QStringLiteral("longitude"), longitude }
    });
    return true;
}

void EditRuntime::clearHoverPreview()
{
    if (!m_sessionActive) {
        return;
    }

    MessageBus::instance()->send(QStringLiteral("map/hovered/clear"), QVariantMap{});
}

bool EditRuntime::commitEdit()
{
    if (!m_sessionActive) {
        return false;
    }

    MessageBus::instance()->send(QStringLiteral("airspace-manager/finish"), QVariantMap{});
    return true;
}

void EditRuntime::cancelEdit()
{
    if (!m_sessionActive) {
        return;
    }

    MessageBus::instance()->send(QStringLiteral("airspace-manager/cancel"), QVariantMap{});
    MessageBus::instance()->publish(Topics::MAP_PREVIEW_ANNOTATION_CLEAR, QVariantList{});
    resetSessionState();
}

void EditRuntime::onBusMessage(const QString& topic, const QVariant& data)
{
    if (topic == QStringLiteral("airspace-manager/drawing-state")) {
        const QVariantMap payload = data.toMap();
        applyDrawingState(payload.value(QStringLiteral("active")).toBool(),
                          payload.value(QStringLiteral("requiresFinish")).toBool(),
                          payload.value(QStringLiteral("statusText")).toString());
        return;
    }

    if (topic == QStringLiteral("airspace-manager/cancel")) {
        resetSessionState();
    }
}

void EditRuntime::resetSessionState()
{
    const bool changed = m_sessionActive || !m_sessionId.isEmpty() || m_shapeType != -1
        || m_requiresFinish || !m_statusText.isEmpty();

    m_sessionActive = false;
    m_sessionId.clear();
    m_shapeType = -1;
    m_requiresFinish = false;
    m_statusText.clear();

    if (changed) {
        emit sessionStateChanged();
    }
}

void EditRuntime::applyDrawingState(bool active, bool requiresFinish, const QString& statusText)
{
    const bool wasActive = m_sessionActive;
    const bool changed = (m_sessionActive != active)
        || (m_requiresFinish != requiresFinish)
        || (m_statusText != statusText);

    m_sessionActive = active;
    m_requiresFinish = requiresFinish;
    m_statusText = statusText;

    if (active && m_sessionId.isEmpty()) {
        m_sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    if (!active && wasActive) {
        m_sessionId.clear();
        m_shapeType = -1;
        m_requiresFinish = false;
    }

    if (changed || (!active && wasActive)) {
        emit sessionStateChanged();
    }
}

} // namespace YEFS