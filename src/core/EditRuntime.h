#ifndef YEFS_EDITRUNTIME_H
#define YEFS_EDITRUNTIME_H

#include <QObject>
#include <QQmlEngine>

namespace YEFS {

class EditRuntime : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool sessionActive READ sessionActive NOTIFY sessionStateChanged)
    Q_PROPERTY(QString sessionId READ sessionId NOTIFY sessionStateChanged)
    Q_PROPERTY(int shapeType READ shapeType NOTIFY sessionStateChanged)
    Q_PROPERTY(bool requiresFinish READ requiresFinish NOTIFY sessionStateChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY sessionStateChanged)

public:
    static EditRuntime* instance();
    static EditRuntime* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);
    static void destroy();

    bool sessionActive() const { return m_sessionActive; }
    QString sessionId() const { return m_sessionId; }
    int shapeType() const { return m_shapeType; }
    bool requiresFinish() const { return m_requiresFinish; }
    QString statusText() const { return m_statusText; }

    Q_INVOKABLE bool beginAirspaceEditSession(int shapeType);
    Q_INVOKABLE bool addControlPoint(double latitude, double longitude);
    Q_INVOKABLE bool undoLastControlPoint();
    Q_INVOKABLE bool updateHoverPreview(double latitude, double longitude);
    Q_INVOKABLE void clearHoverPreview();
    Q_INVOKABLE bool commitEdit();
    Q_INVOKABLE void cancelEdit();

signals:
    void sessionStateChanged();

private slots:
    void onBusMessage(const QString& topic, const QVariant& data);

private:
    explicit EditRuntime(QObject* parent = nullptr);
    ~EditRuntime() override = default;

    void resetSessionState();
    void applyDrawingState(bool active, bool requiresFinish, const QString& statusText);

    static EditRuntime* s_instance;

    bool m_sessionActive = false;
    QString m_sessionId;
    int m_shapeType = -1;
    bool m_requiresFinish = false;
    QString m_statusText;
};

} // namespace YEFS

#endif // YEFS_EDITRUNTIME_H