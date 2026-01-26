/**
 * @file UnitManager.h
 * @brief 全局单位管理器 - 航空任务规划单位设置与换算
 * 
 * 支持的单位分类：
 * - 高度 (Altitude): ft, m
 * - 距离 (Distance): NM, km, m, mi
 * - 速度 (Speed): kt, km/h, m/s, mph
 * - 垂直速度 (VerticalSpeed): ft/min, m/s
 * - 气压 (Pressure): hPa, inHg, mbar
 * - 温度 (Temperature): °C, °F, K
 * - 重量 (Weight): kg, lb
 * - 体积 (Volume): L, gal
 * - 能见度 (Visibility): m, SM, ft
 */

#ifndef YEFS_UNITMANAGER_H
#define YEFS_UNITMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include <QVariantMap>

namespace YEFS {

/**
 * @class UnitManager
 * @brief 管理全局单位设置，提供换算功能
 * 
 * 使用方式：
 * - QML: UnitManager.formatAltitude(1000) -> "3280.84 ft" 或 "1000 m"
 * - QML: UnitManager.convert(100, "distance", "km", "NM") -> 53.996
 */
class UnitManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // 各分类当前单位
    Q_PROPERTY(QString altitudeUnit READ altitudeUnit WRITE setAltitudeUnit NOTIFY altitudeUnitChanged)
    Q_PROPERTY(QString distanceUnit READ distanceUnit WRITE setDistanceUnit NOTIFY distanceUnitChanged)
    Q_PROPERTY(QString speedUnit READ speedUnit WRITE setSpeedUnit NOTIFY speedUnitChanged)
    Q_PROPERTY(QString verticalSpeedUnit READ verticalSpeedUnit WRITE setVerticalSpeedUnit NOTIFY verticalSpeedUnitChanged)
    Q_PROPERTY(QString pressureUnit READ pressureUnit WRITE setPressureUnit NOTIFY pressureUnitChanged)
    Q_PROPERTY(QString temperatureUnit READ temperatureUnit WRITE setTemperatureUnit NOTIFY temperatureUnitChanged)
    Q_PROPERTY(QString weightUnit READ weightUnit WRITE setWeightUnit NOTIFY weightUnitChanged)
    Q_PROPERTY(QString volumeUnit READ volumeUnit WRITE setVolumeUnit NOTIFY volumeUnitChanged)
    Q_PROPERTY(QString visibilityUnit READ visibilityUnit WRITE setVisibilityUnit NOTIFY visibilityUnitChanged)

    // 预设模式
    Q_PROPERTY(QString presetMode READ presetMode NOTIFY presetModeChanged)

public:
    static UnitManager* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);
    static UnitManager* instance();

    explicit UnitManager(QObject* parent = nullptr);
    ~UnitManager() override = default;

    // ========== 单位访问器 ==========
    QString altitudeUnit() const { return m_altitudeUnit; }
    QString distanceUnit() const { return m_distanceUnit; }
    QString speedUnit() const { return m_speedUnit; }
    QString verticalSpeedUnit() const { return m_verticalSpeedUnit; }
    QString pressureUnit() const { return m_pressureUnit; }
    QString temperatureUnit() const { return m_temperatureUnit; }
    QString weightUnit() const { return m_weightUnit; }
    QString volumeUnit() const { return m_volumeUnit; }
    QString visibilityUnit() const { return m_visibilityUnit; }
    QString presetMode() const { return m_presetMode; }

    // ========== 单位设置器 ==========
    void setAltitudeUnit(const QString& unit);
    void setDistanceUnit(const QString& unit);
    void setSpeedUnit(const QString& unit);
    void setVerticalSpeedUnit(const QString& unit);
    void setPressureUnit(const QString& unit);
    void setTemperatureUnit(const QString& unit);
    void setWeightUnit(const QString& unit);
    void setVolumeUnit(const QString& unit);
    void setVisibilityUnit(const QString& unit);

    // ========== 预设模式 ==========
    /**
     * @brief 应用预设模式
     * @param mode "metric" (公制), "imperial" (英制), "aviation" (航空标准), "custom" (自定义)
     */
    Q_INVOKABLE void applyPreset(const QString& mode);

    /**
     * @brief 获取可用的预设模式列表
     */
    Q_INVOKABLE QVariantList getPresetModes() const;

    /**
     * @brief 获取某分类可用的单位列表
     * @param category 分类名：altitude, distance, speed, verticalSpeed, pressure, temperature, weight, volume, visibility
     */
    Q_INVOKABLE QVariantList getAvailableUnits(const QString& category) const;

    // ========== 单位换算 ==========
    /**
     * @brief 通用换算函数
     * @param value 原始值
     * @param category 分类名
     * @param fromUnit 源单位
     * @param toUnit 目标单位
     * @return 换算后的值
     */
    Q_INVOKABLE double convert(double value, const QString& category, 
                               const QString& fromUnit, const QString& toUnit) const;

    /**
     * @brief 从内部单位（SI）换算到当前显示单位
     * @param value 内部值（SI单位）
     * @param category 分类名
     * @return 换算后的值
     */
    Q_INVOKABLE double fromInternal(double value, const QString& category) const;

    /**
     * @brief 从当前显示单位换算到内部单位（SI）
     * @param value 显示值
     * @param category 分类名
     * @return 内部值（SI单位）
     */
    Q_INVOKABLE double toInternal(double value, const QString& category) const;

    // ========== 格式化输出 ==========
    /**
     * @brief 格式化高度显示
     * @param valueInMeters 高度值（内部单位：米）
     * @param decimals 小数位数
     * @return 格式化字符串，如 "3280 ft" 或 "1000 m"
     */
    Q_INVOKABLE QString formatAltitude(double valueInMeters, int decimals = 0) const;

    /**
     * @brief 格式化距离显示
     * @param valueInMeters 距离值（内部单位：米）
     * @param decimals 小数位数
     */
    Q_INVOKABLE QString formatDistance(double valueInMeters, int decimals = 2) const;

    /**
     * @brief 格式化速度显示
     * @param valueInMps 速度值（内部单位：m/s）
     * @param decimals 小数位数
     */
    Q_INVOKABLE QString formatSpeed(double valueInMps, int decimals = 1) const;

    /**
     * @brief 格式化垂直速度显示
     * @param valueInMps 垂直速度值（内部单位：m/s）
     * @param decimals 小数位数
     */
    Q_INVOKABLE QString formatVerticalSpeed(double valueInMps, int decimals = 0) const;

    /**
     * @brief 格式化气压显示
     * @param valueInHPa 气压值（内部单位：hPa）
     * @param decimals 小数位数
     */
    Q_INVOKABLE QString formatPressure(double valueInHPa, int decimals = 2) const;

    /**
     * @brief 格式化温度显示
     * @param valueInCelsius 温度值（内部单位：摄氏度）
     * @param decimals 小数位数
     */
    Q_INVOKABLE QString formatTemperature(double valueInCelsius, int decimals = 1) const;

    /**
     * @brief 格式化重量显示
     * @param valueInKg 重量值（内部单位：kg）
     * @param decimals 小数位数
     */
    Q_INVOKABLE QString formatWeight(double valueInKg, int decimals = 2) const;

    /**
     * @brief 格式化体积显示
     * @param valueInLiters 体积值（内部单位：升）
     * @param decimals 小数位数
     */
    Q_INVOKABLE QString formatVolume(double valueInLiters, int decimals = 2) const;

    /**
     * @brief 格式化能见度显示
     * @param valueInMeters 能见度值（内部单位：米）
     * @param decimals 小数位数
     */
    Q_INVOKABLE QString formatVisibility(double valueInMeters, int decimals = 0) const;

    /**
     * @brief 获取分类的当前单位符号
     */
    Q_INVOKABLE QString getUnitSymbol(const QString& category) const;

    /**
     * @brief 获取分类的内部单位（SI）
     */
    Q_INVOKABLE QString getInternalUnit(const QString& category) const;

signals:
    void altitudeUnitChanged();
    void distanceUnitChanged();
    void speedUnitChanged();
    void verticalSpeedUnitChanged();
    void pressureUnitChanged();
    void temperatureUnitChanged();
    void weightUnitChanged();
    void volumeUnitChanged();
    void visibilityUnitChanged();
    void presetModeChanged();

    /**
     * @brief 任意单位设置变更时发出
     * @param category 变更的分类
     * @param unit 新单位
     */
    void unitChanged(const QString& category, const QString& unit);

    /**
     * @brief 应用预设后发出
     * @param mode 预设模式
     */
    void presetApplied(const QString& mode);

private:
    void loadSettings();
    void saveSettings();
    void updatePresetMode();
    void notifyGlobalChange(const QString& category, const QString& unit);

    // 换算因子表（转换到SI基准单位）
    double getConversionFactor(const QString& category, const QString& unit) const;

    static UnitManager* s_instance;

    // 当前单位设置
    QString m_altitudeUnit;      // 默认: ft (航空标准)
    QString m_distanceUnit;      // 默认: NM
    QString m_speedUnit;         // 默认: kt
    QString m_verticalSpeedUnit; // 默认: ft/min
    QString m_pressureUnit;      // 默认: hPa
    QString m_temperatureUnit;   // 默认: °C
    QString m_weightUnit;        // 默认: kg
    QString m_volumeUnit;        // 默认: L
    QString m_visibilityUnit;    // 默认: m

    QString m_presetMode;        // metric, imperial, aviation, custom
};

} // namespace YEFS

#endif // YEFS_UNITMANAGER_H
