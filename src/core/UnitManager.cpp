/**
 * @file UnitManager.cpp
 * @brief 全局单位管理器实现
 */

#include "UnitManager.h"
#include "SettingsManager.h"
#include "MessageBus.h"
#include <QDebug>
#include <QtMath>

namespace YEFS {

UnitManager* UnitManager::s_instance = nullptr;

UnitManager* UnitManager::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

UnitManager* UnitManager::instance()
{
    if (!s_instance) {
        s_instance = new UnitManager();
    }
    return s_instance;
}

UnitManager::UnitManager(QObject* parent)
    : QObject(parent)
    , m_altitudeUnit("ft")
    , m_distanceUnit("NM")
    , m_speedUnit("kt")
    , m_verticalSpeedUnit("ft/min")
    , m_pressureUnit("hPa")
    , m_temperatureUnit("°C")
    , m_weightUnit("kg")
    , m_volumeUnit("L")
    , m_visibilityUnit("m")
    , m_presetMode("aviation")
{
    if (!s_instance) {
        s_instance = this;
    }
    loadSettings();
}

void UnitManager::loadSettings()
{
    auto* settings = SettingsManager::instance();
    if (!settings) return;

    m_altitudeUnit = settings->getValue("units", "altitude", "ft").toString();
    m_distanceUnit = settings->getValue("units", "distance", "NM").toString();
    m_speedUnit = settings->getValue("units", "speed", "kt").toString();
    m_verticalSpeedUnit = settings->getValue("units", "verticalSpeed", "ft/min").toString();
    m_pressureUnit = settings->getValue("units", "pressure", "hPa").toString();
    m_temperatureUnit = settings->getValue("units", "temperature", "°C").toString();
    m_weightUnit = settings->getValue("units", "weight", "kg").toString();
    m_volumeUnit = settings->getValue("units", "volume", "L").toString();
    m_visibilityUnit = settings->getValue("units", "visibility", "m").toString();

    updatePresetMode();
}

void UnitManager::saveSettings()
{
    auto* settings = SettingsManager::instance();
    if (!settings) return;

    settings->setValue("units", "altitude", m_altitudeUnit);
    settings->setValue("units", "distance", m_distanceUnit);
    settings->setValue("units", "speed", m_speedUnit);
    settings->setValue("units", "verticalSpeed", m_verticalSpeedUnit);
    settings->setValue("units", "pressure", m_pressureUnit);
    settings->setValue("units", "temperature", m_temperatureUnit);
    settings->setValue("units", "weight", m_weightUnit);
    settings->setValue("units", "volume", m_volumeUnit);
    settings->setValue("units", "visibility", m_visibilityUnit);
    settings->save();
}

void UnitManager::updatePresetMode()
{
    // 检查是否匹配预设
    if (m_altitudeUnit == "ft" && m_distanceUnit == "NM" && m_speedUnit == "kt" &&
        m_verticalSpeedUnit == "ft/min" && m_pressureUnit == "hPa" &&
        m_temperatureUnit == "°C" && m_weightUnit == "kg") {
        m_presetMode = "aviation";
    } else if (m_altitudeUnit == "m" && m_distanceUnit == "km" && m_speedUnit == "km/h" &&
               m_verticalSpeedUnit == "m/s" && m_pressureUnit == "hPa" &&
               m_temperatureUnit == "°C" && m_weightUnit == "kg") {
        m_presetMode = "metric";
    } else if (m_altitudeUnit == "ft" && m_distanceUnit == "mi" && m_speedUnit == "mph" &&
               m_verticalSpeedUnit == "ft/min" && m_pressureUnit == "inHg" &&
               m_temperatureUnit == "°F" && m_weightUnit == "lb") {
        m_presetMode = "imperial";
    } else {
        m_presetMode = "custom";
    }
}

void UnitManager::notifyGlobalChange(const QString& category, const QString& unit)
{
    emit unitChanged(category, unit);
    
    // 通过 MessageBus 发送全局通知
    auto* bus = MessageBus::instance();
    if (bus) {
        QVariantMap data;
        data["category"] = category;
        data["unit"] = unit;
        bus->publish("units/changed", data);
    }
}

// ========== 单位设置器 ==========
void UnitManager::setAltitudeUnit(const QString& unit)
{
    if (m_altitudeUnit != unit) {
        m_altitudeUnit = unit;
        saveSettings();
        updatePresetMode();
        emit altitudeUnitChanged();
        emit presetModeChanged();
        notifyGlobalChange("altitude", unit);
    }
}

void UnitManager::setDistanceUnit(const QString& unit)
{
    if (m_distanceUnit != unit) {
        m_distanceUnit = unit;
        saveSettings();
        updatePresetMode();
        emit distanceUnitChanged();
        emit presetModeChanged();
        notifyGlobalChange("distance", unit);
    }
}

void UnitManager::setSpeedUnit(const QString& unit)
{
    if (m_speedUnit != unit) {
        m_speedUnit = unit;
        saveSettings();
        updatePresetMode();
        emit speedUnitChanged();
        emit presetModeChanged();
        notifyGlobalChange("speed", unit);
    }
}

void UnitManager::setVerticalSpeedUnit(const QString& unit)
{
    if (m_verticalSpeedUnit != unit) {
        m_verticalSpeedUnit = unit;
        saveSettings();
        updatePresetMode();
        emit verticalSpeedUnitChanged();
        emit presetModeChanged();
        notifyGlobalChange("verticalSpeed", unit);
    }
}

void UnitManager::setPressureUnit(const QString& unit)
{
    if (m_pressureUnit != unit) {
        m_pressureUnit = unit;
        saveSettings();
        updatePresetMode();
        emit pressureUnitChanged();
        emit presetModeChanged();
        notifyGlobalChange("pressure", unit);
    }
}

void UnitManager::setTemperatureUnit(const QString& unit)
{
    if (m_temperatureUnit != unit) {
        m_temperatureUnit = unit;
        saveSettings();
        updatePresetMode();
        emit temperatureUnitChanged();
        emit presetModeChanged();
        notifyGlobalChange("temperature", unit);
    }
}

void UnitManager::setWeightUnit(const QString& unit)
{
    if (m_weightUnit != unit) {
        m_weightUnit = unit;
        saveSettings();
        updatePresetMode();
        emit weightUnitChanged();
        emit presetModeChanged();
        notifyGlobalChange("weight", unit);
    }
}

void UnitManager::setVolumeUnit(const QString& unit)
{
    if (m_volumeUnit != unit) {
        m_volumeUnit = unit;
        saveSettings();
        updatePresetMode();
        emit volumeUnitChanged();
        emit presetModeChanged();
        notifyGlobalChange("volume", unit);
    }
}

void UnitManager::setVisibilityUnit(const QString& unit)
{
    if (m_visibilityUnit != unit) {
        m_visibilityUnit = unit;
        saveSettings();
        updatePresetMode();
        emit visibilityUnitChanged();
        emit presetModeChanged();
        notifyGlobalChange("visibility", unit);
    }
}

// ========== 预设模式 ==========
void UnitManager::applyPreset(const QString& mode)
{
    if (mode == "aviation") {
        // 航空标准: ft + NM + kt + hPa + °C
        m_altitudeUnit = "ft";
        m_distanceUnit = "NM";
        m_speedUnit = "kt";
        m_verticalSpeedUnit = "ft/min";
        m_pressureUnit = "hPa";
        m_temperatureUnit = "°C";
        m_weightUnit = "kg";
        m_volumeUnit = "L";
        m_visibilityUnit = "m";
    } else if (mode == "metric") {
        // 公制: m + km + km/h
        m_altitudeUnit = "m";
        m_distanceUnit = "km";
        m_speedUnit = "km/h";
        m_verticalSpeedUnit = "m/s";
        m_pressureUnit = "hPa";
        m_temperatureUnit = "°C";
        m_weightUnit = "kg";
        m_volumeUnit = "L";
        m_visibilityUnit = "m";
    } else if (mode == "imperial") {
        // 英制: ft + mi + mph + inHg + °F
        m_altitudeUnit = "ft";
        m_distanceUnit = "mi";
        m_speedUnit = "mph";
        m_verticalSpeedUnit = "ft/min";
        m_pressureUnit = "inHg";
        m_temperatureUnit = "°F";
        m_weightUnit = "lb";
        m_volumeUnit = "gal";
        m_visibilityUnit = "ft";
    } else {
        return; // 未知模式
    }

    m_presetMode = mode;
    saveSettings();

    // 发出所有变更信号
    emit altitudeUnitChanged();
    emit distanceUnitChanged();
    emit speedUnitChanged();
    emit verticalSpeedUnitChanged();
    emit pressureUnitChanged();
    emit temperatureUnitChanged();
    emit weightUnitChanged();
    emit volumeUnitChanged();
    emit visibilityUnitChanged();
    emit presetModeChanged();
    emit presetApplied(mode);

    // 通过 MessageBus 发送全局预设变更通知
    auto* bus = MessageBus::instance();
    if (bus) {
        QVariantMap data;
        data["mode"] = mode;
        bus->publish("units/preset/applied", data);
    }
}

QVariantList UnitManager::getPresetModes() const
{
    return QVariantList{
        QVariantMap{{"id", "aviation"}, {"name", tr("航空标准")}, {"desc", tr("ft/NM/kt/hPa/°C - 国际民航通用")}},
        QVariantMap{{"id", "metric"}, {"name", tr("公制")}, {"desc", tr("m/km/km·h⁻¹/hPa/°C - SI国际单位制")}},
        QVariantMap{{"id", "imperial"}, {"name", tr("英制")}, {"desc", tr("ft/mi/mph/inHg/°F - 美国习惯单位")}}
    };
}

QVariantList UnitManager::getAvailableUnits(const QString& category) const
{
    if (category == "altitude") {
        return QVariantList{
            QVariantMap{{"id", "ft"}, {"name", tr("英尺")}, {"symbol", "ft"}},
            QVariantMap{{"id", "m"}, {"name", tr("米")}, {"symbol", "m"}}
        };
    } else if (category == "distance") {
        return QVariantList{
            QVariantMap{{"id", "NM"}, {"name", tr("海里")}, {"symbol", "NM"}},
            QVariantMap{{"id", "km"}, {"name", tr("千米")}, {"symbol", "km"}},
            QVariantMap{{"id", "m"}, {"name", tr("米")}, {"symbol", "m"}},
            QVariantMap{{"id", "mi"}, {"name", tr("英里")}, {"symbol", "mi"}}
        };
    } else if (category == "speed") {
        return QVariantList{
            QVariantMap{{"id", "kt"}, {"name", tr("节")}, {"symbol", "kt"}},
            QVariantMap{{"id", "km/h"}, {"name", tr("千米/时")}, {"symbol", "km/h"}},
            QVariantMap{{"id", "m/s"}, {"name", tr("米/秒")}, {"symbol", "m/s"}},
            QVariantMap{{"id", "mph"}, {"name", tr("英里/时")}, {"symbol", "mph"}}
        };
    } else if (category == "verticalSpeed") {
        return QVariantList{
            QVariantMap{{"id", "ft/min"}, {"name", tr("英尺/分")}, {"symbol", "ft/min"}},
            QVariantMap{{"id", "m/s"}, {"name", tr("米/秒")}, {"symbol", "m/s"}}
        };
    } else if (category == "pressure") {
        return QVariantList{
            QVariantMap{{"id", "hPa"}, {"name", tr("百帕")}, {"symbol", "hPa"}},
            QVariantMap{{"id", "inHg"}, {"name", tr("英寸汞柱")}, {"symbol", "inHg"}},
            QVariantMap{{"id", "mbar"}, {"name", tr("毫巴")}, {"symbol", "mbar"}}
        };
    } else if (category == "temperature") {
        return QVariantList{
            QVariantMap{{"id", "°C"}, {"name", tr("摄氏度")}, {"symbol", "°C"}},
            QVariantMap{{"id", "°F"}, {"name", tr("华氏度")}, {"symbol", "°F"}},
            QVariantMap{{"id", "K"}, {"name", tr("开尔文")}, {"symbol", "K"}}
        };
    } else if (category == "weight") {
        return QVariantList{
            QVariantMap{{"id", "kg"}, {"name", tr("千克")}, {"symbol", "kg"}},
            QVariantMap{{"id", "lb"}, {"name", tr("磅")}, {"symbol", "lb"}}
        };
    } else if (category == "volume") {
        return QVariantList{
            QVariantMap{{"id", "L"}, {"name", tr("升")}, {"symbol", "L"}},
            QVariantMap{{"id", "gal"}, {"name", tr("加仑")}, {"symbol", "gal"}}
        };
    } else if (category == "visibility") {
        return QVariantList{
            QVariantMap{{"id", "m"}, {"name", tr("米")}, {"symbol", "m"}},
            QVariantMap{{"id", "km"}, {"name", tr("千米")}, {"symbol", "km"}},
            QVariantMap{{"id", "SM"}, {"name", tr("法定英里")}, {"symbol", "SM"}},
            QVariantMap{{"id", "ft"}, {"name", tr("英尺")}, {"symbol", "ft"}}
        };
    }
    return QVariantList();
}

// ========== 换算因子（转换到SI基准单位）==========
double UnitManager::getConversionFactor(const QString& category, const QString& unit) const
{
    // 高度：基准单位 m
    if (category == "altitude" || category == "visibility") {
        if (unit == "m") return 1.0;
        if (unit == "ft") return 0.3048;
        if (unit == "km") return 1000.0;
        if (unit == "SM") return 1609.344;  // statute mile
    }
    // 距离：基准单位 m
    if (category == "distance") {
        if (unit == "m") return 1.0;
        if (unit == "km") return 1000.0;
        if (unit == "NM") return 1852.0;
        if (unit == "mi") return 1609.344;
    }
    // 速度：基准单位 m/s
    if (category == "speed") {
        if (unit == "m/s") return 1.0;
        if (unit == "km/h") return 1.0 / 3.6;
        if (unit == "kt") return 1852.0 / 3600.0;  // 0.51444...
        if (unit == "mph") return 1609.344 / 3600.0;
    }
    // 垂直速度：基准单位 m/s
    if (category == "verticalSpeed") {
        if (unit == "m/s") return 1.0;
        if (unit == "ft/min") return 0.3048 / 60.0;  // 0.00508
    }
    // 气压：基准单位 hPa
    if (category == "pressure") {
        if (unit == "hPa" || unit == "mbar") return 1.0;
        if (unit == "inHg") return 33.8639;
    }
    // 重量：基准单位 kg
    if (category == "weight") {
        if (unit == "kg") return 1.0;
        if (unit == "lb") return 0.45359237;
    }
    // 体积：基准单位 L
    if (category == "volume") {
        if (unit == "L") return 1.0;
        if (unit == "gal") return 3.785411784;  // US gallon
    }
    return 1.0;
}

double UnitManager::convert(double value, const QString& category,
                            const QString& fromUnit, const QString& toUnit) const
{
    if (fromUnit == toUnit) return value;

    // 温度特殊处理（非线性换算）
    if (category == "temperature") {
        double celsius = value;
        // 先转到摄氏度
        if (fromUnit == "°F") {
            celsius = (value - 32.0) * 5.0 / 9.0;
        } else if (fromUnit == "K") {
            celsius = value - 273.15;
        }
        // 再转到目标单位
        if (toUnit == "°C") return celsius;
        if (toUnit == "°F") return celsius * 9.0 / 5.0 + 32.0;
        if (toUnit == "K") return celsius + 273.15;
        return celsius;
    }

    // 线性换算：先转到基准单位，再转到目标单位
    double factorFrom = getConversionFactor(category, fromUnit);
    double factorTo = getConversionFactor(category, toUnit);
    return value * factorFrom / factorTo;
}

double UnitManager::fromInternal(double value, const QString& category) const
{
    QString currentUnit = getUnitSymbol(category);
    QString internalUnit = getInternalUnit(category);
    return convert(value, category, internalUnit, currentUnit);
}

double UnitManager::toInternal(double value, const QString& category) const
{
    QString currentUnit = getUnitSymbol(category);
    QString internalUnit = getInternalUnit(category);
    return convert(value, category, currentUnit, internalUnit);
}

QString UnitManager::getUnitSymbol(const QString& category) const
{
    if (category == "altitude") return m_altitudeUnit;
    if (category == "distance") return m_distanceUnit;
    if (category == "speed") return m_speedUnit;
    if (category == "verticalSpeed") return m_verticalSpeedUnit;
    if (category == "pressure") return m_pressureUnit;
    if (category == "temperature") return m_temperatureUnit;
    if (category == "weight") return m_weightUnit;
    if (category == "volume") return m_volumeUnit;
    if (category == "visibility") return m_visibilityUnit;
    return "";
}

QString UnitManager::getInternalUnit(const QString& category) const
{
    // 内部统一使用 SI 单位
    if (category == "altitude") return "m";
    if (category == "distance") return "m";
    if (category == "speed") return "m/s";
    if (category == "verticalSpeed") return "m/s";
    if (category == "pressure") return "hPa";
    if (category == "temperature") return "°C";
    if (category == "weight") return "kg";
    if (category == "volume") return "L";
    if (category == "visibility") return "m";
    return "";
}

// ========== 格式化输出 ==========
QString UnitManager::formatAltitude(double valueInMeters, int decimals) const
{
    double converted = fromInternal(valueInMeters, "altitude");
    return QString::number(converted, 'f', decimals) + " " + m_altitudeUnit;
}

QString UnitManager::formatDistance(double valueInMeters, int decimals) const
{
    double converted = fromInternal(valueInMeters, "distance");
    return QString::number(converted, 'f', decimals) + " " + m_distanceUnit;
}

QString UnitManager::formatSpeed(double valueInMps, int decimals) const
{
    double converted = fromInternal(valueInMps, "speed");
    return QString::number(converted, 'f', decimals) + " " + m_speedUnit;
}

QString UnitManager::formatVerticalSpeed(double valueInMps, int decimals) const
{
    double converted = fromInternal(valueInMps, "verticalSpeed");
    return QString::number(converted, 'f', decimals) + " " + m_verticalSpeedUnit;
}

QString UnitManager::formatPressure(double valueInHPa, int decimals) const
{
    double converted = fromInternal(valueInHPa, "pressure");
    return QString::number(converted, 'f', decimals) + " " + m_pressureUnit;
}

QString UnitManager::formatTemperature(double valueInCelsius, int decimals) const
{
    double converted = fromInternal(valueInCelsius, "temperature");
    return QString::number(converted, 'f', decimals) + " " + m_temperatureUnit;
}

QString UnitManager::formatWeight(double valueInKg, int decimals) const
{
    double converted = fromInternal(valueInKg, "weight");
    return QString::number(converted, 'f', decimals) + " " + m_weightUnit;
}

QString UnitManager::formatVolume(double valueInLiters, int decimals) const
{
    double converted = fromInternal(valueInLiters, "volume");
    return QString::number(converted, 'f', decimals) + " " + m_volumeUnit;
}

QString UnitManager::formatVisibility(double valueInMeters, int decimals) const
{
    double converted = fromInternal(valueInMeters, "visibility");
    return QString::number(converted, 'f', decimals) + " " + m_visibilityUnit;
}

} // namespace YEFS