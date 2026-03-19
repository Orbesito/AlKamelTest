#include "domain/ClassificationBuilder.h"

#include <QHash>
#include <QJsonValue>
#include <QSet>

#include <algorithm>

namespace
{

struct EntrySnapshot
{
    // "key" is the entry object key in timing.session.entry (often not car number).
    QString key;
    QString carNumber;
    int currentDriverNumber = -1;
    QString team;
    QString vehicle;
    QString className;
    QJsonObject drivers;
};

QJsonObject objectAtPath(const QJsonObject &root, std::initializer_list<const char *> path)
{
    // Defensive path resolver: returns empty object if any segment is missing/non-object.
    QJsonObject current = root;

    for (const char *segment : path) {
        const QJsonValue value = current.value(QString::fromUtf8(segment));
        if (!value.isObject()) {
            return QJsonObject();
        }
        current = value.toObject();
    }

    return current;
}

QString jsonValueToString(const QJsonValue &value)
{
    if (value.isString()) {
        return value.toString();
    }
    if (value.isDouble()) {
        const qint64 number = static_cast<qint64>(value.toDouble());
        return QString::number(number);
    }
    if (value.isBool()) {
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }

    return QString();
}

int jsonValueToInt(const QJsonValue &value, int fallback = -1)
{
    if (value.isDouble()) {
        return value.toInt(fallback);
    }

    if (value.isString()) {
        bool ok = false;
        const int converted = value.toString().toInt(&ok);
        if (ok) {
            return converted;
        }
    }

    return fallback;
}

QString combineName(const QString &firstName, const QString &lastName)
{
    const QString combined = QStringLiteral("%1 %2").arg(firstName.trimmed(), lastName.trimmed()).trimmed();
    return combined;
}

QString resolveDriverName(const EntrySnapshot &entry)
{
    auto nameFromDriverObject = [](const QJsonObject &driver) -> QString {
        const QString fullName = combineName(driver.value(QStringLiteral("firstName")).toString(),
                                             driver.value(QStringLiteral("lastName")).toString());
        if (!fullName.isEmpty()) {
            return fullName;
        }

        const QString shortName = driver.value(QStringLiteral("shortName")).toString().trimmed();
        if (!shortName.isEmpty()) {
            return shortName;
        }

        return driver.value(QStringLiteral("name")).toString().trimmed();
    };

    // Prefer the currently driving driver first; fallback to first available driver object.
    if (entry.currentDriverNumber > 0) {
        const QString currentDriverKey = QString::number(entry.currentDriverNumber);
        const QJsonValue currentDriverValue = entry.drivers.value(currentDriverKey);
        if (currentDriverValue.isObject()) {
            const QString resolved = nameFromDriverObject(currentDriverValue.toObject());
            if (!resolved.isEmpty()) {
                return resolved;
            }
        }
    }

    for (auto it = entry.drivers.begin(); it != entry.drivers.end(); ++it) {
        if (!it.value().isObject()) {
            continue;
        }

        const QString resolved = nameFromDriverObject(it.value().toObject());
        if (!resolved.isEmpty()) {
            return resolved;
        }
    }

    return QString();
}

QString formatGap(const QJsonObject &standingsRow, const QString &timeField, const QString &lapsField)
{
    // Server may provide time-based gaps or lap-based gaps depending on context.
    const int timeValue = jsonValueToInt(standingsRow.value(timeField), -1);
    if (timeValue >= 0) {
        return QString::number(timeValue);
    }

    const int lapsValue = jsonValueToInt(standingsRow.value(lapsField), -99999);
    if (lapsValue != -99999) {
        return QStringLiteral("%1L").arg(lapsValue);
    }

    return QString();
}

QJsonObject selectStandingsBucket(const QJsonObject &overall, QString *bucketName)
{
    // Preferred buckets for this assignment. If neither exists, pick first non-empty object.
    const QStringList preferredBuckets = {
        QStringLiteral("active"),
        QStringLiteral("finishLine")
    };

    for (const QString &name : preferredBuckets) {
        const QJsonValue candidate = overall.value(name);
        if (candidate.isObject() && !candidate.toObject().isEmpty()) {
            if (bucketName != nullptr) {
                *bucketName = name;
            }
            return candidate.toObject();
        }
    }

    for (auto it = overall.begin(); it != overall.end(); ++it) {
        if (!it.value().isObject()) {
            continue;
        }
        if (it.value().toObject().isEmpty()) {
            continue;
        }

        if (bucketName != nullptr) {
            *bucketName = it.key();
        }
        return it.value().toObject();
    }

    if (bucketName != nullptr) {
        *bucketName = QStringLiteral("<none>");
    }
    return QJsonObject();
}

} // namespace

namespace domain
{

ClassificationBuilder::BuildResult ClassificationBuilder::build(const QJsonObject &mergedState)
{
    BuildResult result;

    const QJsonObject entriesObject = objectAtPath(mergedState, {"timing", "session", "entry"});
    const QJsonObject standingsOverall = objectAtPath(mergedState, {"timing", "session", "standings", "overall"});

    if (entriesObject.isEmpty()) {
        result.warnings.push_back(QStringLiteral("Missing timing.session.entry in merged state."));
    }
    if (standingsOverall.isEmpty()) {
        result.warnings.push_back(QStringLiteral("Missing timing.session.standings.overall in merged state."));
        return result;
    }

    QString standingsBucketName;
    const QJsonObject standingsRows = selectStandingsBucket(standingsOverall, &standingsBucketName);
    if (standingsRows.isEmpty()) {
        result.warnings.push_back(QStringLiteral("No standings bucket with rows found under overall."));
        return result;
    }

    QHash<QString, EntrySnapshot> entryByKey;
    QHash<QString, EntrySnapshot> entryByCarNumber;

    for (auto it = entriesObject.begin(); it != entriesObject.end(); ++it) {
        if (!it.value().isObject()) {
            continue;
        }

        const QJsonObject entryObject = it.value().toObject();
        EntrySnapshot entry;
        entry.key = it.key();
        entry.carNumber = jsonValueToString(entryObject.value(QStringLiteral("number"))).trimmed();
        entry.currentDriverNumber = jsonValueToInt(entryObject.value(QStringLiteral("currentDriver")), -1);
        entry.team = entryObject.value(QStringLiteral("team")).toString();
        entry.vehicle = entryObject.value(QStringLiteral("vehicle")).toString();
        entry.className = entryObject.value(QStringLiteral("class")).toString();
        entry.drivers = entryObject.value(QStringLiteral("drivers")).toObject();

        entryByKey.insert(entry.key, entry);
        if (!entry.carNumber.isEmpty()) {
            entryByCarNumber.insert(entry.carNumber, entry);
        }
    }

    QSet<QString> unresolvedParticipants;

    for (auto it = standingsRows.begin(); it != standingsRows.end(); ++it) {
        if (!it.value().isObject()) {
            continue;
        }

        const QJsonObject standing = it.value().toObject();
        ClassificationRow row;
        row.position = jsonValueToInt(standing.value(QStringLiteral("position")), jsonValueToInt(QJsonValue(it.key()), -1));
        row.participantToken = jsonValueToString(standing.value(QStringLiteral("participant"))).trimmed();
        row.lapNumber = jsonValueToInt(standing.value(QStringLiteral("lapNumber")), -1);
        row.gap = formatGap(standing, QStringLiteral("gapFirstTime"), QStringLiteral("gapFirstLaps"));
        row.interval = formatGap(standing, QStringLiteral("gapPreviousTime"), QStringLiteral("gapPreviousLaps"));
        row.status = raceStatusFromString(standing.value(QStringLiteral("status")).toString());

        // Participant mapping is ambiguous across feeds:
        // try entry key first, then fallback to car number.
        const EntrySnapshot *resolvedEntry = nullptr;
        auto entryIt = entryByKey.constFind(row.participantToken);
        if (entryIt != entryByKey.constEnd()) {
            resolvedEntry = &entryIt.value();
        } else {
            entryIt = entryByCarNumber.constFind(row.participantToken);
            if (entryIt != entryByCarNumber.constEnd()) {
                resolvedEntry = &entryIt.value();
            }
        }

        if (resolvedEntry != nullptr) {
            row.carNumber = !resolvedEntry->carNumber.isEmpty() ? resolvedEntry->carNumber : row.participantToken;
            row.driverName = resolveDriverName(*resolvedEntry);
            row.team = resolvedEntry->team;
            row.vehicle = resolvedEntry->vehicle;
            row.className = resolvedEntry->className;
        } else {
            row.carNumber = row.participantToken;
            if (!row.participantToken.isEmpty()) {
                unresolvedParticipants.insert(row.participantToken);
            }
        }

        result.rows.push_back(row);
    }

    std::sort(result.rows.begin(), result.rows.end(), [](const ClassificationRow &lhs, const ClassificationRow &rhs) {
        const bool lhsValid = lhs.position >= 0;
        const bool rhsValid = rhs.position >= 0;

        if (lhsValid != rhsValid) {
            return lhsValid;
        }
        if (!lhsValid && !rhsValid) {
            return lhs.carNumber < rhs.carNumber;
        }
        return lhs.position < rhs.position;
    });

    if (!unresolvedParticipants.isEmpty()) {
        QStringList tokens;
        for (const QString &token : unresolvedParticipants) {
            tokens.push_back(token);
        }
        tokens.sort();
        result.warnings.push_back(QStringLiteral("Unresolved participant mapping for token(s): %1")
                                      .arg(tokens.join(QStringLiteral(","))));
    }
    result.warnings.push_back(QStringLiteral("Standings bucket used: %1").arg(standingsBucketName));

    return result;
}

} // namespace domain
