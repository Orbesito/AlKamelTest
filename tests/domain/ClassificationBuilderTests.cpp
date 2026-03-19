#include "domain/ClassificationBuilder.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <iostream>
#include <string>

namespace
{

QJsonObject parseObject(const char *jsonText)
{
    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray(jsonText));
    return doc.object();
}

bool expectTrue(bool condition, const std::string &message)
{
    if (condition) {
        return true;
    }

    std::cerr << "[FAIL] " << message << '\n';
    return false;
}

bool testParticipantMappingByCarNumberFallback()
{
    const QJsonObject mergedState = parseObject(R"({
        "timing": {
            "session": {
                "entry": {
                    "0": {
                        "number": "30",
                        "class": "LMP1",
                        "currentDriver": 1,
                        "team": "Team A",
                        "vehicle": "Car A",
                        "drivers": {
                            "1": { "firstName": "Alice", "lastName": "Walker" }
                        }
                    }
                },
                "standings": {
                    "overall": {
                        "active": {
                            "1": {
                                "position": 1,
                                "participant": "30",
                                "lapNumber": 12,
                                "status": "CLASSIFIED",
                                "gapFirstTime": 0,
                                "gapPreviousTime": 0
                            }
                        }
                    }
                }
            }
        }
    })");

    const domain::ClassificationBuilder::BuildResult result = domain::ClassificationBuilder::build(mergedState);
    if (!expectTrue(result.rows.size() == 1, "expected one classification row")) {
        return false;
    }

    const domain::ClassificationRow &row = result.rows.first();
    return expectTrue(row.position == 1, "position should be extracted") &&
           expectTrue(row.carNumber == QStringLiteral("30"), "car number should map from entry.number") &&
           expectTrue(row.driverName == QStringLiteral("Alice Walker"), "driver name should resolve using currentDriver") &&
           expectTrue(row.team == QStringLiteral("Team A"), "team should map from entry data") &&
           expectTrue(row.className == QStringLiteral("LMP1"), "class should map from entry data");
}

} // namespace

int main()
{
    const bool ok = testParticipantMappingByCarNumberFallback();
    if (!ok) {
        return 1;
    }

    std::cout << "[PASS] ClassificationBuilder tests succeeded.\n";
    return 0;
}
