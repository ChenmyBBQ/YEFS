#include "AirspaceSelectionController.h"

#include "AirspaceMapLayerMapper.h"
#include "AirspaceModel.h"
#include "IPlugin.h"

#include <QMetaObject>

namespace {

QString resolveAirspaceIdFromResult(const QVariantMap& result)
{
    const QString businessObjectId = result.value(QStringLiteral("businessObjectId")).toString();
    if (!businessObjectId.isEmpty()) {
        return businessObjectId;
    }

    const QString layerId = result.value(QStringLiteral("layerId")).toString();
    if (layerId.startsWith(QStringLiteral("airspace-"))
        && layerId != QStringLiteral("airspace-preview")) {
        return layerId.mid(QStringLiteral("airspace-").size());
    }

    return {};
}

} // namespace

AirspaceSelectionController::AirspaceSelectionController(AirspaceModel* model, QObject* parent)
    : QObject(parent)
    , m_model(model)
{
    if (m_model) {
        connect(m_model, &AirspaceModel::airspaceRemoved, this, [this](const QString& airspaceId) {
            if (airspaceId == m_selectedAirspaceId) {
                clearSelection();
            }
        });

        connect(m_model, &AirspaceModel::airspaceUpdated, this, [this](const QString& airspaceId) {
            if (airspaceId != m_selectedAirspaceId) {
                return;
            }

            const QVariantMap previousData = m_selectedAirspaceData;
            m_selectedAirspaceData = buildAirspaceData(airspaceId);
            applySelectionStyle(airspaceId, true);
            emitSelectionChangedIfNeeded(airspaceId, previousData);
        });
    }
}

void AirspaceSelectionController::attachContext(YEFS::PluginContext* context)
{
    m_context = context;
}

bool AirspaceSelectionController::selectAirspace(const QString& airspaceId)
{
    if (!m_model || airspaceId.isEmpty()) {
        return false;
    }

    const QVariantMap nextData = buildAirspaceData(airspaceId);
    if (nextData.isEmpty()) {
        return false;
    }

    const QString previousId = m_selectedAirspaceId;
    const QVariantMap previousData = m_selectedAirspaceData;

    if (!previousId.isEmpty() && previousId != airspaceId) {
        applySelectionStyle(previousId, false);
    }

    m_selectedAirspaceId = airspaceId;
    m_selectedAirspaceData = nextData;
    applySelectionStyle(airspaceId, true);

    if (m_context) {
        m_context->sendMessage(QStringLiteral("airspace-manager/show"), QVariantMap{});
    }

    emitSelectionChangedIfNeeded(previousId, previousData);
    return true;
}

void AirspaceSelectionController::clearSelection()
{
    const QString previousId = m_selectedAirspaceId;
    const QVariantMap previousData = m_selectedAirspaceData;
    if (previousId.isEmpty()) {
        return;
    }

    applySelectionStyle(previousId, false);
    m_selectedAirspaceId.clear();
    m_selectedAirspaceData.clear();
    emitSelectionChangedIfNeeded(previousId, previousData);
}

void AirspaceSelectionController::onMessage(const QString& topic, const QVariant& data)
{
    if (topic == QStringLiteral("map/query/empty")) {
        clearSelection();
        return;
    }

    if (topic != QStringLiteral("map/query/result")) {
        return;
    }

    const QVariantMap payload = data.toMap();
    const QVariantList results = payload.value(QStringLiteral("results")).toList();
    for (const QVariant& resultValue : results) {
        const QString airspaceId = resolveAirspaceIdFromResult(resultValue.toMap());
        if (!airspaceId.isEmpty()) {
            selectAirspace(airspaceId);
            return;
        }
    }
}

QVariantMap AirspaceSelectionController::buildAirspaceData(const QString& airspaceId) const
{
    if (!m_model) {
        return {};
    }

    QVariantMap data = m_model->getAirspace(airspaceId);
    if (data.isEmpty()) {
        return {};
    }

    data[QStringLiteral("layerId")] = AirspaceMapLayerMapper::layerId(airspaceId);
    data[QStringLiteral("businessObjectId")] = airspaceId;
    return data;
}

QVariantMap AirspaceSelectionController::baseStyleForAirspace(const QString& airspaceId) const
{
    if (!m_model) {
        return {};
    }

    const AirspaceEntity entity = m_model->getAirspaceEntity(airspaceId);
    if (entity.id.isEmpty()) {
        return {};
    }

    return AirspaceMapLayerMapper::baseStyle(entity);
}

QVariantMap AirspaceSelectionController::highlightedStyle(const QVariantMap& baseStyle) const
{
    return AirspaceMapLayerMapper::highlightedStyle(baseStyle);
}

void AirspaceSelectionController::applySelectionStyle(const QString& airspaceId, bool selected)
{
    if (!m_context || airspaceId.isEmpty() || !m_model) {
        return;
    }

    QObject* engine = m_context->getService(QStringLiteral("MapLibreEngine"));
    if (!engine) {
        return;
    }

    const AirspaceEntity entity = m_model->getAirspaceEntity(airspaceId);
    if (entity.id.isEmpty()) {
        return;
    }

    const AirspaceLayerUpdate layerUpdate = AirspaceMapLayerMapper::toLayerUpdate(entity, selected);

    QMetaObject::invokeMethod(engine, "updateGeoJSONLayer",
        Q_ARG(QString, layerUpdate.layerId),
        Q_ARG(QJsonObject, layerUpdate.geoJson),
        Q_ARG(QVariantMap, layerUpdate.style));
}

void AirspaceSelectionController::emitSelectionChangedIfNeeded(const QString& previousId,
                                                               const QVariantMap& previousData)
{
    if (previousId != m_selectedAirspaceId || previousData != m_selectedAirspaceData) {
        emit selectionChanged();
    }
}