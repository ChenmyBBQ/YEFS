#ifndef AIRSPACERECORDMAPPER_H
#define AIRSPACERECORDMAPPER_H

#include "AirspaceDatabase.h"
#include "AirspaceTypes.h"

class AirspaceRecordMapper
{
public:
    static AirspaceEntity toEntity(const AirspaceRecord& record);
    static AirspaceRecord toRecord(const AirspaceEntity& entity);
};

#endif // AIRSPACERECORDMAPPER_H