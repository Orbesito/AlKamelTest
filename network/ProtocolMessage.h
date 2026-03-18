#pragma once

#include <QJsonDocument>
#include <QString>

#include <optional>

namespace network
{

enum class ProtocolCommand
{
    Login,
    Join,
    Leave,
    Ping,
    Ack,
    Error,
    Reply,
    Cmd,
    Json,
    Unknown
};

QString protocolCommandToString(ProtocolCommand command);
ProtocolCommand protocolCommandFromString(const QString &value);

struct ProtocolMessage
{
    ProtocolCommand command = ProtocolCommand::Unknown;
    QString messageId;
    bool isReply = false;
    QString channel;
    QString rawData;
    std::optional<QJsonDocument> jsonData;

    [[nodiscard]] bool hasData() const;
    [[nodiscard]] bool hasJsonData() const;
};

} // namespace network
