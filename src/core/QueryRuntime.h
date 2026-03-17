#ifndef YEFS_QUERYRUNTIME_H
#define YEFS_QUERYRUNTIME_H

#include <QObject>
#include <QQmlEngine>
#include <QPointF>
#include <QVariantList>

namespace YEFS {

class QueryRuntime : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantList lastResults READ lastResults NOTIFY lastResultsChanged)

public:
    static QueryRuntime* instance();
    static QueryRuntime* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);
    static void destroy();

    QVariantList lastResults() const { return m_lastResults; }

    Q_INVOKABLE QVariantList queryFeaturesAtScreenPoint(const QPointF& screenPosition,
                                                        const QVariantList& layerIds = {},
                                                        const QString& groupId = QString(),
                                                        double tolerancePx = 8.0);
    Q_INVOKABLE QVariantList handleTap(const QPointF& screenPosition,
                                       const QVariantList& layerIds = {},
                                       const QString& groupId = QString());

signals:
    void lastResultsChanged();

private:
    explicit QueryRuntime(QObject* parent = nullptr);
    ~QueryRuntime() override = default;

    void setLastResults(const QVariantList& results);

    static QueryRuntime* s_instance;

    QVariantList m_lastResults;
};

} // namespace YEFS

#endif // YEFS_QUERYRUNTIME_H