#ifndef YEFS_MAPPAGESTATECONTROLLER_H
#define YEFS_MAPPAGESTATECONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <QPointF>
#include <QVariantList>
#include <QTimer>

namespace YEFS {

class MapPageStateController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString mouseLatLonText READ mouseLatLonText NOTIFY mouseCoordinateChanged)
    Q_PROPERTY(QString mouseUtmText READ mouseUtmText NOTIFY mouseCoordinateChanged)
    Q_PROPERTY(QString mouseMgrsText READ mouseMgrsText NOTIFY mouseCoordinateChanged)
    Q_PROPERTY(bool airspacePanelVisible READ airspacePanelVisible NOTIFY airspacePanelVisibleChanged)
    Q_PROPERTY(bool airspaceListVisible READ airspaceListVisible NOTIFY airspaceListVisibleChanged)
    Q_PROPERTY(bool drawingActive READ drawingActive NOTIFY drawingActiveChanged)
    Q_PROPERTY(bool drawingRequiresFinish READ drawingRequiresFinish NOTIFY drawingRequiresFinishChanged)
    Q_PROPERTY(bool newAirspaceToolbarVisible READ newAirspaceToolbarVisible NOTIFY newAirspaceToolbarVisibleChanged)
    Q_PROPERTY(int selectedShapeType READ selectedShapeType NOTIFY selectedShapeTypeChanged)
    Q_PROPERTY(QString drawingStatusText READ drawingStatusText NOTIFY drawingStatusTextChanged)
    Q_PROPERTY(QVariantList shapeOptions READ shapeOptions CONSTANT)

public:
    static MapPageStateController* instance();
    static MapPageStateController* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);
    static void destroy();

    ~MapPageStateController() override = default;

    QString mouseLatLonText() const;
    QString mouseUtmText() const;
    QString mouseMgrsText() const;
    bool airspacePanelVisible() const;
    bool airspaceListVisible() const;
    bool drawingActive() const;
    bool drawingRequiresFinish() const;
    bool newAirspaceToolbarVisible() const;
    int selectedShapeType() const;
    QString drawingStatusText() const;
    QVariantList shapeOptions() const;

    Q_INVOKABLE void handleMapTap(const QPointF& position);
    Q_INVOKABLE void handleMapDoubleTap(const QPointF& position);
    Q_INVOKABLE void handleMapHover(const QPointF& position);
    Q_INVOKABLE void handleMapHoverChanged(bool hovered);
    Q_INVOKABLE void toggleShapeSelection(int shapeType);
    Q_INVOKABLE void clearShapeSelection();
    Q_INVOKABLE void hideNewAirspaceToolbar();
    Q_INVOKABLE void hideAirspaceList();
    Q_INVOKABLE void requestHideAirspacePanel();
    Q_INVOKABLE void resetMouseCoordinate();
    Q_INVOKABLE void finishDrawing();
    Q_INVOKABLE void cancelDrawing();

signals:
    void mouseCoordinateChanged();
    void airspacePanelVisibleChanged();
    void airspaceListVisibleChanged();
    void drawingActiveChanged();
    void drawingRequiresFinishChanged();
    void newAirspaceToolbarVisibleChanged();
    void selectedShapeTypeChanged();
    void drawingStatusTextChanged();

private:
    explicit MapPageStateController(QObject* parent = nullptr);

    static MapPageStateController* s_instance;

    void handleAirspaceMessage(const QString& topic, const QVariant& data);
    void updateMouseCoordinateTexts(double latitude, double longitude);
    void setAirspacePanelVisible(bool visible);
    void setAirspaceListVisible(bool visible);
    void setDrawingActive(bool active);
    void setDrawingRequiresFinish(bool requiresFinish);
    void setNewAirspaceToolbarVisible(bool visible);
    void setSelectedShapeType(int shapeType);
    void setDrawingStatusText(const QString& statusText);
    void initializeShapeOptions();

private slots:
    void onBusMessage(const QString& topic, const QVariant& data);
    void processHoverUpdate();

private:
    QTimer*  m_hoverTimer{nullptr};
    QPointF  m_pendingHoverPos;
    QPointF  m_lastProcessedHoverPos;

    QString m_mouseLatLonText;
    QString m_mouseUtmText;
    QString m_mouseMgrsText;
    QVariantList m_shapeOptions;
    bool m_airspacePanelVisible = false;
    bool m_airspaceListVisible = false;
    bool m_drawingActive = false;
    bool m_drawingRequiresFinish = false;
    bool m_newAirspaceToolbarVisible = false;
    int m_selectedShapeType = -1;
    QString m_drawingStatusText;
};

} // namespace YEFS

#endif // YEFS_MAPPAGESTATECONTROLLER_H