#include "AirspaceDraftValidator.h"

int AirspaceDraftValidator::minimumPoints(AirspaceShapeType shapeType)
{
    switch (shapeType) {
    case AirspaceShapeType::Rectangle: return 2;
    case AirspaceShapeType::Square: return 2;
    case AirspaceShapeType::Circle: return 2;
    case AirspaceShapeType::Polygon: return -1;
    case AirspaceShapeType::Boundary: return -1;
    case AirspaceShapeType::Ring: return 3;
    case AirspaceShapeType::Arc: return 4;
    case AirspaceShapeType::Sector: return 4;
    case AirspaceShapeType::SectorRing: return 5;
    }

    return -1;
}

bool AirspaceDraftValidator::canPreview(const AirspaceDraft& draft)
{
    const int count = draft.geometry.controlPoints.size();
    if (count <= 0) {
        return false;
    }

    switch (draft.geometry.shapeType) {
    case AirspaceShapeType::Rectangle:
    case AirspaceShapeType::Square:
    case AirspaceShapeType::Circle:
        return count >= 1;
    case AirspaceShapeType::Polygon:
    case AirspaceShapeType::Boundary:
        return count >= 2;
    case AirspaceShapeType::Ring:
        return count >= 2;
    case AirspaceShapeType::Arc:
    case AirspaceShapeType::Sector:
        return count >= 2;
    case AirspaceShapeType::SectorRing:
        return count >= 2;
    }

    return false;
}

bool AirspaceDraftValidator::canFinish(const AirspaceDraft& draft)
{
    return finishError(draft).isEmpty();
}

QString AirspaceDraftValidator::finishError(const AirspaceDraft& draft)
{
    const int count = draft.geometry.controlPoints.size();

    switch (draft.geometry.shapeType) {
    case AirspaceShapeType::Polygon:
        if (count < 3) {
            return QStringLiteral("多边形至少需要 3 个点");
        }
        break;
    case AirspaceShapeType::Boundary:
        if (count < 2) {
            return QStringLiteral("边界线至少需要 2 个点");
        }
        break;
    default: {
        const int minPoints = minimumPoints(draft.geometry.shapeType);
        if (minPoints > 0 && count < minPoints) {
            return QStringLiteral("当前图形至少需要 %1 个点").arg(minPoints);
        }
        break;
    }
    }

    return {};
}