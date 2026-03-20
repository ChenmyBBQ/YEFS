#ifndef DRAWINGCONTROLLER_H
#define DRAWINGCONTROLLER_H

#include <QObject>
#include <QVariantList>
#include <QJsonObject>

#include "AirspaceGeometryConverter.h"
#include "AirspaceTypes.h"

class ShapeGenerator;
class AirspaceModel;

/**
 * @brief 空域绘制状态机
 *
 * 管理绘制流程：选择形状类型 → 在地图上采集坐标点 → 实时预览 → 完成/取消。
 * 通过 MessageBus 监听 "map/clicked" 事件获取坐标。
 */
class DrawingController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int drawingState READ drawingState NOTIFY drawingStateChanged)
    Q_PROPERTY(int currentShapeType READ currentShapeType NOTIFY currentShapeTypeChanged)
    Q_PROPERTY(int pointCount READ pointCount NOTIFY pointsChanged)
    Q_PROPERTY(int requiredPoints READ requiredPoints NOTIFY currentShapeTypeChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QVariantMap currentShapeInfo READ currentShapeInfo NOTIFY currentShapeInfoChanged)

public:
    enum DrawingState {
        Idle = 0,
        Drawing,
        Editing
    };
    Q_ENUM(DrawingState)

    explicit DrawingController(ShapeGenerator* shapeGen,
                                AirspaceModel* model,
                                QObject* parent = nullptr);

    int drawingState() const { return m_state; }
    int currentShapeType() const { return airspaceShapeTypeToInt(m_draft.geometry.shapeType); }
    int pointCount() const { return m_draft.geometry.controlPoints.size(); }
    int requiredPoints() const;
    QString statusText() const { return m_statusText; }
    QVariantMap currentShapeInfo() const { return m_draft.shapeInfo; }

    /// 开始绘制指定形状
    Q_INVOKABLE void startDrawing(int shapeType);

    /// 手动添加一个坐标点
    Q_INVOKABLE void addPoint(double latitude, double longitude);

    /// 撤销最后一点
    Q_INVOKABLE void undoLastPoint();

    /// 完成绘制 (多边形/边界的手动结束)
    Q_INVOKABLE void finishDrawing();

    /// 取消绘制
    Q_INVOKABLE void cancel();

    /// 获取当前预览 GeoJSON
    Q_INVOKABLE QJsonObject previewGeoJson() const { return m_geometryConverter.buildPreview(m_draft).geoJson; }

    /// 获取收集到的点 (供 QML 使用)
    Q_INVOKABLE QVariantList collectedPoints() const { return m_draft.collectedPoints(); }

    /// 保存到模型并返回 UUID
    Q_INVOKABLE QString saveAirspace(const QString& name,
                                      const QString& styleJson,
                                      const QString& propertiesJson);
    Q_INVOKABLE QString saveAirspaceData(const QString& name,
                                          const QVariantMap& styleData,
                                          const QVariantMap& propertiesData);

signals:
    void drawingStateChanged(int state);
    void currentShapeTypeChanged(int shapeType);
    void pointsChanged();
    void statusTextChanged();
    void currentShapeInfoChanged(const QVariantMap& info);

    /// 高频预览标注（绕过GeoJSON层）
    void previewAnnotationSet(const QVariantList& points);
    void previewAnnotationCleared();

    /// 预览数据更新
    void previewUpdated(const QJsonObject& geoJson);

    /// 绘制完成，弹出编辑框
    void drawingCompleted(const QJsonObject& geoJson, int shapeType);

    /// 绘制取消
    void drawingCancelled();

public slots:
    /// 接收 MessageBus 消息
    void onMessage(const QString& topic, const QVariant& data);

private:
    void updateHoverPoint(double latitude, double longitude);
    void clearHoverPoint();
    void updatePreview();
    void setStatusText(const QString& text);

    /// 不同形状需要的最少点数 (-1 表示可变长)
    ShapeGenerator* m_shapeGen;
    AirspaceModel*  m_model;
    AirspaceGeometryConverter m_geometryConverter;

    DrawingState m_state      = Idle;
    AirspaceDraft m_draft;
    QString      m_statusText;
};

#endif // DRAWINGCONTROLLER_H
