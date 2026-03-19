#include "domain/RaceState.h"

namespace domain
{

RaceStatus raceStatusFromString(const QString &value)
{
    const QString normalized = value.trimmed().toUpper();

    if (normalized == QStringLiteral("CLASSIFIED")) {
        return RaceStatus::Classified;
    }
    if (normalized == QStringLiteral("SUBSTITUTE")) {
        return RaceStatus::Substitute;
    }
    if (normalized == QStringLiteral("NOT_CLASSIFIED")) {
        return RaceStatus::NotClassified;
    }
    if (normalized == QStringLiteral("RETIRED")) {
        return RaceStatus::Retired;
    }
    if (normalized == QStringLiteral("EXCLUDED")) {
        return RaceStatus::Excluded;
    }
    if (normalized == QStringLiteral("NOT_STARTED")) {
        return RaceStatus::NotStarted;
    }
    if (normalized == QStringLiteral("DISQUALIFIED")) {
        return RaceStatus::Disqualified;
    }

    return RaceStatus::Unknown;
}

QString raceStatusToString(RaceStatus status)
{
    switch (status) {
    case RaceStatus::Classified:
        return QStringLiteral("CLASSIFIED");
    case RaceStatus::Substitute:
        return QStringLiteral("SUBSTITUTE");
    case RaceStatus::NotClassified:
        return QStringLiteral("NOT_CLASSIFIED");
    case RaceStatus::Retired:
        return QStringLiteral("RETIRED");
    case RaceStatus::Excluded:
        return QStringLiteral("EXCLUDED");
    case RaceStatus::NotStarted:
        return QStringLiteral("NOT_STARTED");
    case RaceStatus::Disqualified:
        return QStringLiteral("DISQUALIFIED");
    case RaceStatus::Unknown:
        break;
    }

    return QStringLiteral("UNKNOWN");
}

} // namespace domain
