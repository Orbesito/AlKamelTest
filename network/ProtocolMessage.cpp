#include "network/ProtocolMessage.h"

namespace network
{

QString protocolCommandToString(ProtocolCommand command)
{
    switch (command) {
    case ProtocolCommand::Login:
        return QStringLiteral("LOGIN");
    case ProtocolCommand::Join:
        return QStringLiteral("JOIN");
    case ProtocolCommand::Leave:
        return QStringLiteral("LEAVE");
    case ProtocolCommand::Ping:
        return QStringLiteral("PING");
    case ProtocolCommand::Ack:
        return QStringLiteral("ACK");
    case ProtocolCommand::Error:
        return QStringLiteral("ERROR");
    case ProtocolCommand::Reply:
        return QStringLiteral("REPLY");
    case ProtocolCommand::Cmd:
        return QStringLiteral("CMD");
    case ProtocolCommand::Json:
        return QStringLiteral("JSON");
    case ProtocolCommand::Unknown:
        break;
    }

    return QStringLiteral("UNKNOWN");
}

ProtocolCommand protocolCommandFromString(const QString &value)
{
    const QString normalized = value.trimmed().toUpper();

    if (normalized == QStringLiteral("LOGIN")) {
        return ProtocolCommand::Login;
    }
    if (normalized == QStringLiteral("JOIN")) {
        return ProtocolCommand::Join;
    }
    if (normalized == QStringLiteral("LEAVE")) {
        return ProtocolCommand::Leave;
    }
    if (normalized == QStringLiteral("PING")) {
        return ProtocolCommand::Ping;
    }
    if (normalized == QStringLiteral("ACK")) {
        return ProtocolCommand::Ack;
    }
    if (normalized == QStringLiteral("ERROR")) {
        return ProtocolCommand::Error;
    }
    if (normalized == QStringLiteral("REPLY")) {
        return ProtocolCommand::Reply;
    }
    if (normalized == QStringLiteral("CMD")) {
        return ProtocolCommand::Cmd;
    }
    if (normalized == QStringLiteral("JSON")) {
        return ProtocolCommand::Json;
    }

    return ProtocolCommand::Unknown;
}

bool ProtocolMessage::hasData() const
{
    return !rawData.isEmpty();
}

bool ProtocolMessage::hasJsonData() const
{
    return jsonData.has_value();
}

} // namespace network
