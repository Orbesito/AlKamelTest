#include "state/JsonStateStore.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

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

bool testRecursiveMergePreservesSiblings()
{
    state::JsonStateStore store;
    store.mergeUpdate(parseObject(R"({
        "timing": {
            "session": {
                "entry": {
                    "0": {
                        "name": "Driver A",
                        "team": "Team A",
                        "drivers": {
                            "1": {
                                "firstName": "John"
                            }
                        }
                    }
                }
            }
        }
    })"));

    store.mergeUpdate(parseObject(R"({
        "timing": {
            "session": {
                "entry": {
                    "0": {
                        "team": "Team B",
                        "drivers": {
                            "1": {
                                "lastName": "Smith"
                            }
                        }
                    }
                },
                "standings": {
                    "overall": {
                        "active": {
                            "1": {
                                "position": 1
                            }
                        }
                    }
                }
            }
        }
    })"));

    const QJsonObject root = store.root();
    const QJsonObject entry0 = root
                                   .value(QStringLiteral("timing")).toObject()
                                   .value(QStringLiteral("session")).toObject()
                                   .value(QStringLiteral("entry")).toObject()
                                   .value(QStringLiteral("0")).toObject();
    const QJsonObject driver1 = entry0.value(QStringLiteral("drivers")).toObject().value(QStringLiteral("1")).toObject();
    const int position = root
                             .value(QStringLiteral("timing")).toObject()
                             .value(QStringLiteral("session")).toObject()
                             .value(QStringLiteral("standings")).toObject()
                             .value(QStringLiteral("overall")).toObject()
                             .value(QStringLiteral("active")).toObject()
                             .value(QStringLiteral("1")).toObject()
                             .value(QStringLiteral("position")).toInt(-1);

    return expectTrue(entry0.value(QStringLiteral("name")).toString() == QStringLiteral("Driver A"), "entry.name should be preserved") &&
           expectTrue(entry0.value(QStringLiteral("team")).toString() == QStringLiteral("Team B"), "entry.team should be overwritten") &&
           expectTrue(driver1.value(QStringLiteral("firstName")).toString() == QStringLiteral("John"), "nested firstName should be preserved") &&
           expectTrue(driver1.value(QStringLiteral("lastName")).toString() == QStringLiteral("Smith"), "nested lastName should be merged") &&
           expectTrue(position == 1, "standings position should be merged");
}

bool testArrayReplacement()
{
    state::JsonStateStore store;
    store.mergeUpdate(parseObject(R"({"timing":{"liveEvents":[1,2,3]}})"));
    store.mergeUpdate(parseObject(R"({"timing":{"liveEvents":[4]}})"));

    const QJsonArray mergedArray = store.root()
                                       .value(QStringLiteral("timing")).toObject()
                                       .value(QStringLiteral("liveEvents")).toArray();

    return expectTrue(mergedArray.size() == 1, "arrays should be replaced, not recursively merged") &&
           expectTrue(mergedArray.first().toInt(-1) == 4, "array replacement should keep latest payload");
}

} // namespace

int main()
{
    bool success = true;
    success = testRecursiveMergePreservesSiblings() && success;
    success = testArrayReplacement() && success;

    if (!success) {
        return 1;
    }

    std::cout << "[PASS] JsonStateStore tests succeeded.\n";
    return 0;
}
