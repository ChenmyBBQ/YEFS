#ifndef YEFS_COORDINATECONVERTER_H
#define YEFS_COORDINATECONVERTER_H

#include <QObject>
#include <QQmlEngine>
#include <QVariantMap>

namespace YEFS {

class CoordinateConverter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static CoordinateConverter* instance();
    static CoordinateConverter* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);
    static void destroy();

    Q_INVOKABLE QString formatLatLon(double latitude, double longitude, int decimals = 6) const;
    Q_INVOKABLE QString latLonToUtmText(double latitude, double longitude) const;
    Q_INVOKABLE QString latLonToMgrsText(double latitude, double longitude) const;
    Q_INVOKABLE QVariantMap latLonToUtm(double latitude, double longitude) const;

private:
    explicit CoordinateConverter(QObject* parent = nullptr);
    ~CoordinateConverter() override = default;

    struct UTMResult {
        bool valid = false;
        int zone = 0;
        QChar hemisphere;
        QChar band;
        double easting = 0.0;
        double northing = 0.0;
    };

    static CoordinateConverter* s_instance;

    static UTMResult convertToUtm(double latitude, double longitude);
    static QChar latitudeBand(double latitude);
    static QString utmToMgrs(const UTMResult& utm);
};

} // namespace YEFS

#endif // YEFS_COORDINATECONVERTER_H
