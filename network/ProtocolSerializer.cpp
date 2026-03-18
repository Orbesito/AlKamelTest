#include "network/ProtocolSerializer.h"

#include <QJsonDocument>

namespace
{

QString jsonDocumentToCompactText(const QJsonDocument &json)
{
    return QString::fromUtf8(json.toJson(QJsonDocument::Compact));
}

} // namespace

namespace network
{

QString ProtocolSerializer::serialize(const ProtocolMessage &message)
{
    const QString commandToken = protocolCommandToString(message.command);
    if (commandToken == QStringLiteral("UNKNOWN")) {
        return QString();
    }

    QString messageIdToken = message.messageId;
    if (message.isReply) {
        // Reply frames carry the '+' marker on message-id.
        messageIdToken.append('+');
    }

    QString dataToken = message.rawData;
    if (message.jsonData.has_value()) {
        dataToken = jsonDocumentToCompactText(*message.jsonData);
    }

    return QStringLiteral("%1:%2:%3:%4").arg(commandToken, messageIdToken, message.channel, dataToken);
}

} // namespace network
