#include "state/JsonStateStore.h"

#include <QJsonDocument>
#include <QStringList>

namespace state
{

void JsonStateStore::clear()
{
    m_root = QJsonObject();
}

void JsonStateStore::mergeUpdate(const QJsonObject &partialUpdate)
{
    if (partialUpdate.isEmpty()) {
        return;
    }

    // Al Kamel V2 sends partial updates, so we recursively merge object nodes.
    m_root = mergeObjects(m_root, partialUpdate);
}

const QJsonObject &JsonStateStore::root() const
{
    return m_root;
}

QString JsonStateStore::toCompactJson() const
{
    return QString::fromUtf8(QJsonDocument(m_root).toJson(QJsonDocument::Compact));
}

QString JsonStateStore::toIndentedJson() const
{
    return QString::fromUtf8(QJsonDocument(m_root).toJson(QJsonDocument::Indented));
}

QString JsonStateStore::rootKeysSummary() const
{
    const QStringList keys = m_root.keys();
    if (keys.isEmpty()) {
        return QStringLiteral("<empty>");
    }

    return keys.join(QStringLiteral(","));
}

QJsonObject JsonStateStore::mergeObjects(const QJsonObject &base, const QJsonObject &patch)
{
    QJsonObject merged = base;

    for (auto it = patch.begin(); it != patch.end(); ++it) {
        const QString &key = it.key();
        const QJsonValue &patchValue = it.value();

        if (patchValue.isObject()) {
            const QJsonObject baseObject = merged.value(key).toObject();
            merged.insert(key, mergeObjects(baseObject, patchValue.toObject()));
            continue;
        }

        // Scalars, arrays and null values replace the previous value at this key.
        merged.insert(key, patchValue);
    }

    return merged;
}

} // namespace state
