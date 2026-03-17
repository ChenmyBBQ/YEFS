#ifndef AIRSPACESELECTIONCONTROLLER_H
#define AIRSPACESELECTIONCONTROLLER_H

#include <QObject>
#include <QPointer>
#include <QVariantMap>

namespace YEFS {
class PluginContext;
}

class AirspaceModel;

class AirspaceSelectionController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString selectedAirspaceId READ selectedAirspaceId NOTIFY selectionChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(QVariantMap selectedAirspaceData READ selectedAirspaceData NOTIFY selectionChanged)

public:
    explicit AirspaceSelectionController(AirspaceModel* model, QObject* parent = nullptr);

    QString selectedAirspaceId() const { return m_selectedAirspaceId; }
    bool hasSelection() const { return !m_selectedAirspaceId.isEmpty(); }
    QVariantMap selectedAirspaceData() const { return m_selectedAirspaceData; }

    void attachContext(YEFS::PluginContext* context);

    Q_INVOKABLE bool selectAirspace(const QString& airspaceId);
    Q_INVOKABLE void clearSelection();

signals:
    void selectionChanged();

public slots:
    void onMessage(const QString& topic, const QVariant& data);

private:
    QVariantMap buildAirspaceData(const QString& airspaceId) const;
    QVariantMap baseStyleForAirspace(const QString& airspaceId) const;
    QVariantMap highlightedStyle(const QVariantMap& baseStyle) const;
    void applySelectionStyle(const QString& airspaceId, bool selected);
    void emitSelectionChangedIfNeeded(const QString& previousId, const QVariantMap& previousData);

    QPointer<AirspaceModel> m_model;
    YEFS::PluginContext* m_context = nullptr;
    QString m_selectedAirspaceId;
    QVariantMap m_selectedAirspaceData;
};

#endif // AIRSPACESELECTIONCONTROLLER_H