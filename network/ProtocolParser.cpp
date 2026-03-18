#include "network/ProtocolParser.h"

#include <QJsonDocument>
#include <QJsonParseError>

namespace network
{

ProtocolParseResult ProtocolParser::parseLine(const QString &line)
{
    ProtocolParseResult result;

    // Split only the first 3 separators: command:message-id:channel:data
    // The data field may contain additional ':' characters (JSON payload).
    const int firstColon = line.indexOf(':');
    const int secondColon = firstColon >= 0 ? line.indexOf(':', firstColon + 1) : -1;
    const int thirdColon = secondColon >= 0 ? line.indexOf(':', secondColon + 1) : -1;

    if (firstColon < 0 || secondColon < 0 || thirdColon < 0) {
        result.error = QStringLiteral("Invalid protocol frame (expected 4 colon-separated fields).");
        return result;
    }

    const QString commandToken = line.left(firstColon).trimmed();
    QString messageIdToken = line.mid(firstColon + 1, secondColon - firstColon - 1).trimmed();
    const QString channelToken = line.mid(secondColon + 1, thirdColon - secondColon - 1).trimmed();
    const QString dataToken = line.mid(thirdColon + 1).trimmed();

    result.message.command = protocolCommandFromString(commandToken);
    if (result.message.command == ProtocolCommand::Unknown) {
        result.error = QStringLiteral("Unknown protocol command: %1").arg(commandToken);
        return result;
    }

    if (messageIdToken.endsWith('+')) {
        // Server replies append '+' to the original message id.
        result.message.isReply = true;
        messageIdToken.chop(1);
        messageIdToken = messageIdToken.trimmed();
    }

    result.message.messageId = messageIdToken;
    result.message.channel = channelToken;
    result.message.rawData = dataToken;

    if (!dataToken.isEmpty()) {
        QJsonParseError parseError;
        const QJsonDocument json = QJsonDocument::fromJson(dataToken.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            result.message.jsonData = json;
        } else {
            result.error = QStringLiteral("JSON parse warning: %1").arg(parseError.errorString());
        }
    }

    result.ok = true;
    return result;
}

} // namespace network
