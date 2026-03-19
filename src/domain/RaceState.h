#pragma once

#include <QString>
#include <QVector>

namespace domain
{

enum class RaceStatus
{
    Classified,
    Substitute,
    NotClassified,
    Retired,
    Excluded,
    NotStarted,
    Disqualified,
    Unknown
};

RaceStatus raceStatusFromString(const QString &value);
QString raceStatusToString(RaceStatus status);

struct ClassificationRow
{
    int position = -1;
    QString carNumber;
    QString driverName;
    QString team;
    QString vehicle;
    QString className;
    QString gap;
    QString interval;
    int lapNumber = -1;
    RaceStatus status = RaceStatus::Unknown;
    QString participantToken;
};

using ClassificationRows = QVector<ClassificationRow>;

} // namespace domain
