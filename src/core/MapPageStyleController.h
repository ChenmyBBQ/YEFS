#ifndef YEFS_MAPPAGESTYLECONTROLLER_H
#define YEFS_MAPPAGESTYLECONTROLLER_H

#include <QObject>
#include <QQmlEngine>

namespace YEFS {

class MapPageStyleController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString currentStyleUrl READ currentStyleUrl NOTIFY currentStyleUrlChanged)
    Q_PROPERTY(bool styleSwitching READ styleSwitching NOTIFY styleSwitchingChanged)
    Q_PROPERTY(bool fallbackTimeout READ fallbackTimeout NOTIFY fallbackTimeoutChanged)

public:
    static MapPageStyleController* instance();
    static MapPageStyleController* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);
    static void destroy();

    ~MapPageStyleController() override = default;

    QString currentStyleUrl() const;
    bool styleSwitching() const;
    bool fallbackTimeout() const;

    Q_INVOKABLE void handleLoadTimeout();
    Q_INVOKABLE bool handleFirstFrameReady();
    Q_INVOKABLE void notifyOverlayHidden();

signals:
    void currentStyleUrlChanged();
    void styleSwitchingChanged();
    void fallbackTimeoutChanged();

private slots:
    void onSettingsChanged(const QString& category, const QString& key);

private:
    explicit MapPageStyleController(QObject* parent = nullptr);

    static MapPageStyleController* s_instance;

    QString resolveConfiguredStyleUrl() const;
    void refreshConfiguredStyle(bool forceNotify = false);
    void notifyStartupVisualReady(const QString& reason);
    void setStyleSwitching(bool styleSwitching);
    void setFallbackTimeout(bool fallbackTimeout);

    QString m_currentStyleUrl;
    bool m_styleSwitching = true;
    bool m_fallbackTimeout = false;
    bool m_startupVisualReadyEmitted = false;
};

} // namespace YEFS

#endif // YEFS_MAPPAGESTYLECONTROLLER_H