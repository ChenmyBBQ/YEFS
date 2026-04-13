#include "CoordinateConverter.h"

#include <QtMath>

namespace YEFS {

CoordinateConverter* CoordinateConverter::s_instance = nullptr;

CoordinateConverter::CoordinateConverter(QObject* parent)
    : QObject(parent)
{
}

CoordinateConverter* CoordinateConverter::instance()
{
    if (!s_instance) {
        s_instance = new CoordinateConverter();
    }
    return s_instance;
}

CoordinateConverter* CoordinateConverter::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    auto *inst = instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

void CoordinateConverter::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

QString CoordinateConverter::formatLatLon(double latitude, double longitude, int decimals) const
{
    if (!qIsFinite(latitude) || !qIsFinite(longitude)) {
        return QStringLiteral("--");
    }

    const QString latHemisphere = latitude >= 0.0 ? tr("N ") : tr("S ");
    const QString lonHemisphere = longitude >= 0.0 ? tr("E ") : tr("W ");

        return QString(latHemisphere)
            + QString::number(qAbs(latitude), 'f', decimals)
            + QStringLiteral("°")
            + QStringLiteral(", ")
            + QString(lonHemisphere)
            + QString::number(qAbs(longitude), 'f', decimals)
            + QStringLiteral("°");
}

QVariantMap CoordinateConverter::latLonToUtm(double latitude, double longitude) const
{
    const UTMResult utm = convertToUtm(latitude, longitude);
    if (!utm.valid) {
        return QVariantMap();
    }

    QVariantMap result;
    result.insert(QStringLiteral("zone"), utm.zone);
    result.insert(QStringLiteral("hemisphere"), QString(utm.hemisphere));
    result.insert(QStringLiteral("band"), QString(utm.band));
    result.insert(QStringLiteral("easting"), utm.easting);
    result.insert(QStringLiteral("northing"), utm.northing);
    return result;
}

QString CoordinateConverter::latLonToUtmText(double latitude, double longitude) const
{
    const UTMResult utm = convertToUtm(latitude, longitude);
    if (!utm.valid) {
        return QStringLiteral("--");
    }

    return QString::number(utm.zone)
            + utm.hemisphere
            + QStringLiteral(" E ") + QString::number(qRound64(utm.easting))
            + QStringLiteral(" N ") + QString::number(qRound64(utm.northing));
}

QString CoordinateConverter::latLonToMgrsText(double latitude, double longitude) const
{
    const UTMResult utm = convertToUtm(latitude, longitude);
    if (!utm.valid) {
        return QStringLiteral("--");
    }
    return utmToMgrs(utm);
}

QChar CoordinateConverter::latitudeBand(double latitude)
{
    static const QString bands = QStringLiteral("CDEFGHJKLMNPQRSTUVWX");
    int index = static_cast<int>(qFloor((latitude + 80.0) / 8.0));
    index = qBound(0, index, 19);
    return bands.at(index);
}

CoordinateConverter::UTMResult CoordinateConverter::convertToUtm(double latitude, double longitude)
{
    UTMResult result;

    if (!qIsFinite(latitude) || !qIsFinite(longitude)) {
        return result;
    }
    if (latitude < -80.0 || latitude > 84.0 || longitude < -180.0 || longitude > 180.0) {
        return result;
    }

    const double a = 6378137.0;
    const double f = 1.0 / 298.257223563;
    const double k0 = 0.9996;
    const double e2 = f * (2.0 - f);
    const double ep2 = e2 / (1.0 - e2);

    const int zone = static_cast<int>(qFloor((longitude + 180.0) / 6.0)) + 1;
    const double lonOrigin = (zone - 1) * 6.0 - 180.0 + 3.0;

    const double latRad = qDegreesToRadians(latitude);
    const double lonRad = qDegreesToRadians(longitude);
    const double lonOriginRad = qDegreesToRadians(lonOrigin);

    const double sinLat = qSin(latRad);
    const double cosLat = qCos(latRad);
    const double tanLat = qTan(latRad);

    const double N = a / qSqrt(1.0 - e2 * sinLat * sinLat);
    const double T = tanLat * tanLat;
    const double C = ep2 * cosLat * cosLat;
    const double A = cosLat * (lonRad - lonOriginRad);

    const double M = a * ((1.0 - e2 / 4.0 - 3.0 * e2 * e2 / 64.0 - 5.0 * qPow(e2, 3) / 256.0) * latRad
                          - (3.0 * e2 / 8.0 + 3.0 * e2 * e2 / 32.0 + 45.0 * qPow(e2, 3) / 1024.0) * qSin(2.0 * latRad)
                          + (15.0 * e2 * e2 / 256.0 + 45.0 * qPow(e2, 3) / 1024.0) * qSin(4.0 * latRad)
                          - (35.0 * qPow(e2, 3) / 3072.0) * qSin(6.0 * latRad));

    double easting = k0 * N * (A
                               + (1.0 - T + C) * qPow(A, 3) / 6.0
                               + (5.0 - 18.0 * T + T * T + 72.0 * C - 58.0 * ep2) * qPow(A, 5) / 120.0)
            + 500000.0;

    double northing = k0 * (M + N * tanLat * (A * A / 2.0
                                              + (5.0 - T + 9.0 * C + 4.0 * C * C) * qPow(A, 4) / 24.0
                                              + (61.0 - 58.0 * T + T * T + 600.0 * C - 330.0 * ep2) * qPow(A, 6) / 720.0));

    QChar hemisphere = QLatin1Char('N');
    if (latitude < 0.0) {
        northing += 10000000.0;
        hemisphere = QLatin1Char('S');
    }

    result.valid = true;
    result.zone = zone;
    result.hemisphere = hemisphere;
    result.band = latitudeBand(latitude);
    result.easting = easting;
    result.northing = northing;
    return result;
}

QString CoordinateConverter::utmToMgrs(const UTMResult& utm)
{
    static const QStringList e100kSets = {
        QStringLiteral("ABCDEFGH"),
        QStringLiteral("JKLMNPQR"),
        QStringLiteral("STUVWXYZ"),
        QStringLiteral("ABCDEFGH"),
        QStringLiteral("JKLMNPQR"),
        QStringLiteral("STUVWXYZ")
    };
    static const QStringList n100kSets = {
        QStringLiteral("ABCDEFGHJKLMNPQRSTUV"),
        QStringLiteral("FGHJKLMNPQRSTUVABCDE")
    };

    const int easting100k = (static_cast<int>(qFloor(utm.easting / 100000.0))) % 8;
    const int northing100k = (static_cast<int>(qFloor(utm.northing / 100000.0))) % 20;

    const QChar col = e100kSets.at((utm.zone - 1) % 6).at(easting100k);
    const QChar row = n100kSets.at((utm.zone - 1) % 2).at(northing100k);

    const QString eastingRemainder = QStringLiteral("%1")
            .arg(static_cast<int>(qFloor(std::fmod(utm.easting, 100000.0))), 5, 10, QLatin1Char('0'));
    const QString northingRemainder = QStringLiteral("%1")
            .arg(static_cast<int>(qFloor(std::fmod(utm.northing, 100000.0))), 5, 10, QLatin1Char('0'));

    return QString::number(utm.zone)
            + utm.band
            + QStringLiteral(" ")
            + col
            + row
            + QStringLiteral(" ")
            + eastingRemainder
            + QStringLiteral(" ")
            + northingRemainder;
}

} // namespace YEFS
