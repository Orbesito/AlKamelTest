#pragma once

#include <QJsonObject>
#include <QString>

namespace state
{

class JsonStateStore
{
public:
    void clear();
    void mergeUpdate(const QJsonObject &partialUpdate);

    [[nodiscard]] const QJsonObject &root() const;
    [[nodiscard]] QString toCompactJson() const;
    [[nodiscard]] QString toIndentedJson() const;
    [[nodiscard]] QString rootKeysSummary() const;

private:
    static QJsonObject mergeObjects(const QJsonObject &base, const QJsonObject &patch);

    QJsonObject m_root;
};

} // namespace state
