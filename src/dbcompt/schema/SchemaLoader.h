#pragma once
#include "../dbcompt_global.h"
#include "SchemaTypes.h"
#include <QString>
#include <QJsonObject>

namespace dbcompt {

class DBCOMPT_EXPORT SchemaLoader {
public:
    static bool load(const QString& filePath, SchemaDef& outSchema, QString& outError);

private:
    static DatabaseConfig parseDatabase(const QJsonObject& obj);
    static TableDef       parseTable(const QJsonObject& obj);
    static FieldDef       parseField(const QJsonObject& obj);
    static IndexDef       parseIndex(const QJsonObject& obj);
};

} // namespace dbcompt
