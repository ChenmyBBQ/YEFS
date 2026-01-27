#ifndef YEFS_IMAPPARSER_H
#define YEFS_IMAPPARSER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QIODevice>
#include <QQmlEngine>
#include <QHash>

#include "IMapSource.h"

namespace YEFS {

/**
 * @brief 地图格式解析器接口
 * 
 * 定义了地图格式解析器的标准接口
 * 各种格式（GPX、KML、GeoJSON、MBTiles等）的解析器都需要实现此接口
 */
class IMapParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QStringList supportedExtensions READ supportedExtensions CONSTANT)

public:
    explicit IMapParser(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IMapParser() = default;

    // 解析器信息
    virtual QString name() const = 0;
    virtual QString description() const { return QString(); }
    virtual QStringList supportedExtensions() const = 0;
    virtual QStringList mimeTypes() const { return QStringList(); }

    // 格式检测
    virtual bool canParse(const QString& filePath) const = 0;
    virtual bool canParse(QIODevice* device) const = 0;

    // 解析
    virtual IMapSource* parse(const QString& filePath) = 0;
    virtual IMapSource* parse(QIODevice* device, const QString& sourceName) = 0;

    // 批量解析
    virtual QList<IMapSource*> parseMultiple(const QString& filePath) { 
        auto source = parse(filePath);
        return source ? QList<IMapSource*>{source} : QList<IMapSource*>{};
    }

signals:
    void parseProgress(int current, int total);
    void parseError(const QString& message);
};

/**
 * @brief 地图解析器工厂
 * 
 * 管理所有已注册的地图格式解析器
 */
class MapParserFactory : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static MapParserFactory* instance();
    static MapParserFactory* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    // 注册解析器
    void registerParser(IMapParser* parser);
    void unregisterParser(const QString& name);

    // 查询解析器
    IMapParser* parser(const QString& name) const;
    QList<IMapParser*> allParsers() const { return m_parsers.values(); }
    IMapParser* parserForFile(const QString& filePath) const;
    IMapParser* parserForExtension(const QString& extension) const;

    // 支持的格式
    Q_INVOKABLE QStringList supportedExtensions() const;
    Q_INVOKABLE QStringList supportedMimeTypes() const;

    // 解析文件
    Q_INVOKABLE IMapSource* parseFile(const QString& filePath);

signals:
    void parserRegistered(const QString& name);
    void parserUnregistered(const QString& name);

private:
    explicit MapParserFactory(QObject* parent = nullptr);
    ~MapParserFactory() override = default;

    static MapParserFactory* s_instance;
    QHash<QString, IMapParser*> m_parsers;
};

} // namespace YEFS

#endif // YEFS_IMAPPARSER_H
