#pragma once

#include <QJsonObject>
#include <QStringList>

#include "domain/RaceState.h"

namespace domain
{

class ClassificationBuilder
{
public:
    struct BuildResult
    {
        ClassificationRows rows;
        QStringList warnings;
    };

    static BuildResult build(const QJsonObject &mergedState);
};

} // namespace domain
