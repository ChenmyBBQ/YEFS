#include "SchemaLoader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

namespace dbcompt {

bool SchemaLoader::load(const QString& filePath, SchemaDef& outSchema, QString& outError)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        outError = QStringLiteral("无法打开 schema 文件: ") + filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);
    if (parseErr.error != QJsonParseError::NoError) {
        outError = QStringLiteral("JSON 解析错误: ") + parseErr.errorString();
        return false;
    }

    if (!doc.isObject()) {
        outError = QStringLiteral("Schema 根节点必须是 JSON 对象");
        return false;
    }

    QJsonObject root = doc.object();
    outSchema.version  = root[QStringLiteral("version")].toString();
    outSchema.database = parseDatabase(root[QStringLiteral("database")].toObject());

    const QJsonArray tables = root[QStringLiteral("tables")].toArray();
    for (const auto& tval : tables)
        outSchema.tables.append(parseTable(tval.toObject()));

    return true;
}

DatabaseConfig SchemaLoader::parseDatabase(const QJsonObject& obj)
{
    DatabaseConfig cfg;
    cfg.type     = obj[QStringLiteral("type")].toString(QStringLiteral("sqlite"));
    cfg.path     = obj[QStringLiteral("path")].toString();
    cfg.host     = obj[QStringLiteral("host")].toString(QStringLiteral("localhost"));
    cfg.port     = obj[QStringLiteral("port")].toInt(5432);
    cfg.dbname   = obj[QStringLiteral("dbname")].toString();
    cfg.user     = obj[QStringLiteral("user")].toString();
    cfg.password = obj[QStringLiteral("password")].toString();
    return cfg;
}

FieldDef SchemaLoader::parseField(const QJsonObject& obj)
{
    FieldDef f;
    f.name          = obj[QStringLiteral("name")].toString();
    f.type          = obj[QStringLiteral("type")].toString(QStringLiteral("text")).toLower();
    f.primaryKey    = obj[QStringLiteral("primaryKey")].toBool(false);
    f.notNull       = obj[QStringLiteral("notNull")].toBool(false);
    f.unique        = obj[QStringLiteral("unique")].toBool(false);
    f.autoIncrement = obj[QStringLiteral("autoIncrement")].toBool(false);
    f.defaultValue  = obj[QStringLiteral("default")].toString();
    return f;
}

IndexDef SchemaLoader::parseIndex(const QJsonObject& obj)
{
    IndexDef idx;
    idx.name   = obj[QStringLiteral("name")].toString();
    idx.unique = obj[QStringLiteral("unique")].toBool(false);
    for (const auto& v : obj[QStringLiteral("fields")].toArray())
        idx.fields.append(v.toString());
    return idx;
}

TableDef SchemaLoader::parseTable(const QJsonObject& obj)
{
    TableDef t;
    t.name = obj[QStringLiteral("name")].toString();

    for (const auto& fv : obj[QStringLiteral("fields")].toArray())
        t.fields.append(parseField(fv.toObject()));

    for (const auto& iv : obj[QStringLiteral("indexes")].toArray())
        t.indexes.append(parseIndex(iv.toObject()));

    const QJsonObject migObj = obj[QStringLiteral("migration")].toObject();
    const QJsonObject renameObj = migObj[QStringLiteral("rename")].toObject();
    for (const QString& key : renameObj.keys())
        t.migration.rename[key] = renameObj[key].toString();

    for (const auto& dv : migObj[QStringLiteral("drop")].toArray())
        t.migration.drop.append(dv.toString());

    return t;
}

} // namespace dbcompt
