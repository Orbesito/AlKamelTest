#include "network/AlkamelSession.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <utility>

#include "network/ProtocolParser.h"
#include "network/ProtocolSerializer.h"

namespace
{

QString commandLabel(network::ProtocolCommand command)
{
    return network::protocolCommandToString(command);
}

QString inferJsonRootChannel(const network::ProtocolMessage &message)
{
    if (!message.channel.trimmed().isEmpty()) {
        return message.channel.trimmed();
    }

    if (!message.hasJsonData() || !message.jsonData->isObject()) {
        return QStringLiteral("<empty>");
    }

    // For JSON:n:: payloads, infer a readable channel hint from root keys.
    const QStringList keys = message.jsonData->object().keys();
    if (keys.isEmpty()) {
        return QStringLiteral("<empty>");
    }
    if (keys.size() == 1) {
        return keys.first();
    }

    return keys.join(QStringLiteral(","));
}

} // namespace

namespace network
{

AlkamelSession::AlkamelSession(AppConfig config, QObject *parent)
    : QObject(parent),
      m_config(std::move(config))
{
    m_transport = new AlkamelClient(m_config, this);

    m_pingTimer.setSingleShot(false);
    m_pingTimer.setTimerType(Qt::CoarseTimer);

    connect(m_transport, &AlkamelClient::connected, this, &AlkamelSession::onTransportConnected);
    connect(m_transport, &AlkamelClient::disconnected, this, &AlkamelSession::onTransportDisconnected);
    connect(m_transport, &AlkamelClient::rawLineReceived, this, &AlkamelSession::onRawLineReceived);
    connect(m_transport, &AlkamelClient::logMessage, this, &AlkamelSession::logMessage);
    connect(&m_pingTimer, &QTimer::timeout, this, &AlkamelSession::onPingTimerTimeout);
}

void AlkamelSession::start()
{
    emitLog(QStringLiteral("Session start requested."));
    m_transport->connectToServer();
}

void AlkamelSession::stop()
{
    emitLog(QStringLiteral("Session stop requested."));
    m_pingTimer.stop();
    m_transport->disconnectFromServer();
}

bool AlkamelSession::isConnected() const
{
    return m_transport->isConnected();
}

bool AlkamelSession::isLoggedIn() const
{
    return m_loggedIn;
}

void AlkamelSession::onTransportConnected()
{
    clearSessionState();
    emit connected();

    // Protocol session bootstrap: authenticate first, then subscribe.
    emitLog(QStringLiteral("Transport connected. Sending LOGIN."));
    sendLogin();
}

void AlkamelSession::onTransportDisconnected()
{
    m_pingTimer.stop();
    m_loggedIn = false;
    emit disconnected();
}

void AlkamelSession::onRawLineReceived(const QString &line)
{
    if (line.trimmed().isEmpty()) {
        return;
    }

    const ProtocolParseResult parsed = ProtocolParser::parseLine(line);
    if (!parsed.ok) {
        const QString sanitizedLine = line.isEmpty() ? QStringLiteral("<empty>") : line;
        emitLog(QStringLiteral("Protocol parse error: %1 | line=%2").arg(parsed.error, sanitizedLine));
        return;
    }

    if (!parsed.error.isEmpty()) {
        emitLog(parsed.error);
    }

    dispatchIncoming(parsed.message);
}

void AlkamelSession::onPingTimerTimeout()
{
    if (!m_loggedIn) {
        return;
    }

    // Keepalive cadence is server-driven (pingRate from LOGIN reply).
    sendPing();
}

QString AlkamelSession::nextMessageId()
{
    const QString value = QString::number(m_nextMessageId);
    ++m_nextMessageId;
    return value;
}

void AlkamelSession::clearSessionState()
{
    m_nextMessageId = 1;
    m_pingRateSeconds = 20;
    m_timeoutSeconds = 40;
    m_loggedIn = false;
    m_pendingCommands.clear();
}

void AlkamelSession::sendLogin()
{
    ProtocolMessage message;
    message.command = ProtocolCommand::Login;
    message.messageId = nextMessageId();

    QJsonObject payload;
    payload.insert(QStringLiteral("user"), m_config.user);
    payload.insert(QStringLiteral("password"), m_config.password);
    payload.insert(QStringLiteral("app"), m_config.appName);
    payload.insert(QStringLiteral("app_ver"), m_config.appVersion);
    payload.insert(QStringLiteral("protocol"), m_config.protocolName);
    payload.insert(QStringLiteral("protocol_ver"), m_config.protocolVersion);
    message.jsonData = QJsonDocument(payload);

    sendMessage(message);
}

void AlkamelSession::sendJoin(const QString &channel)
{
    if (channel.trimmed().isEmpty()) {
        emitLog(QStringLiteral("Skipping JOIN for an empty channel."));
        return;
    }

    ProtocolMessage message;
    message.command = ProtocolCommand::Join;
    message.messageId = nextMessageId();
    message.channel = channel.trimmed();

    sendMessage(message);
}

void AlkamelSession::sendPing()
{
    ProtocolMessage message;
    message.command = ProtocolCommand::Ping;
    message.messageId = nextMessageId();
    sendMessage(message);
}

void AlkamelSession::sendMessage(const ProtocolMessage &message)
{
    const QString serialized = ProtocolSerializer::serialize(message);
    if (serialized.isEmpty()) {
        emitLog(QStringLiteral("Refusing to send unsupported protocol command."));
        return;
    }

    if (!message.messageId.isEmpty()) {
        // Track client requests so ACK/ERROR can be correlated in logs.
        m_pendingCommands.insert(message.messageId, message.command);
    }

    m_transport->sendRawLine(serialized);
}

void AlkamelSession::dispatchIncoming(const ProtocolMessage &message)
{
    switch (message.command) {
    case ProtocolCommand::Login:
        handleLoginReply(message);
        return;
    case ProtocolCommand::Join:
        handleJoinReply(message);
        return;
    case ProtocolCommand::Ack:
        handleAck(message);
        return;
    case ProtocolCommand::Error:
        handleError(message);
        return;
    case ProtocolCommand::Json:
        emitLog(QStringLiteral("JSON message received on channel '%1'.")
                    .arg(inferJsonRootChannel(message)));
        if (message.hasJsonData() && message.jsonData->isObject()) {
            emit jsonPayloadReceived(message.jsonData->object());
        } else {
            emitLog(QStringLiteral("Ignoring JSON command without object payload."));
        }
        return;
    case ProtocolCommand::Reply:
    case ProtocolCommand::Cmd:
    case ProtocolCommand::Leave:
    case ProtocolCommand::Ping:
    case ProtocolCommand::Unknown:
        break;
    }

    emitLog(QStringLiteral("Unhandled command: %1").arg(protocolCommandToString(message.command)));
}

void AlkamelSession::handleLoginReply(const ProtocolMessage &message)
{
    if (!message.isReply) {
        emitLog(QStringLiteral("Ignoring LOGIN command without reply marker."));
        return;
    }

    takePendingCommandLabel(message.messageId);

    if (!message.hasJsonData() || !message.jsonData->isObject()) {
        emitLog(QStringLiteral("LOGIN reply missing JSON payload."));
        return;
    }

    const QJsonObject payload = message.jsonData->object();
    m_pingRateSeconds = parsePositiveIntegerField(payload, QStringLiteral("pingRate"), 20);
    m_timeoutSeconds = parsePositiveIntegerField(payload, QStringLiteral("timeout"), 40);

    const QString serverName = payload.value(QStringLiteral("name")).toString();
    const QString serverVersion = payload.value(QStringLiteral("ver")).toString();
    const QString minVersion = payload.value(QStringLiteral("min_ver")).toString();

    m_loggedIn = true;
    emit loggedIn();

    emitLog(QStringLiteral("LOGIN accepted. server='%1' ver='%2' minVer='%3' pingRate=%4s timeout=%5s")
                .arg(serverName, serverVersion, minVersion)
                .arg(m_pingRateSeconds)
                .arg(m_timeoutSeconds));

    m_pingTimer.start(m_pingRateSeconds * 1000);
    emitLog(QStringLiteral("PING timer started (%1s interval).").arg(m_pingRateSeconds));

    // Channel list is configurable in AppConfig, so mapping assumptions stay isolated.
    for (const QString &channel : m_config.joinChannels) {
        sendJoin(channel);
    }
}

void AlkamelSession::handleJoinReply(const ProtocolMessage &message)
{
    if (!message.isReply) {
        emitLog(QStringLiteral("Ignoring JOIN command without reply marker."));
        return;
    }

    const QString related = takePendingCommandLabel(message.messageId);
    if (!related.isEmpty()) {
        emitLog(QStringLiteral("JOIN acknowledged for pending command %1 on channel '%2'.")
                    .arg(related, message.channel));
    } else {
        emitLog(QStringLiteral("JOIN acknowledged on channel '%1'.").arg(message.channel));
    }
}

void AlkamelSession::handleAck(const ProtocolMessage &message)
{
    const QString related = takePendingCommandLabel(message.messageId);
    if (!related.isEmpty()) {
        emitLog(QStringLiteral("ACK received for %1 (message-id=%2).").arg(related, message.messageId));
    } else {
        emitLog(QStringLiteral("ACK received (message-id=%1).").arg(message.messageId));
    }
}

void AlkamelSession::handleError(const ProtocolMessage &message)
{
    const QString related = takePendingCommandLabel(message.messageId);
    const QString relatedLabel = related.isEmpty() ? QStringLiteral("unknown") : related;

    QString reason;
    QString advice;
    if (message.hasJsonData() && message.jsonData->isObject()) {
        const QJsonObject payload = message.jsonData->object();
        reason = payload.value(QStringLiteral("reason")).toString();
        advice = payload.value(QStringLiteral("advice")).toString();
    }

    emitLog(QStringLiteral("ERROR received (message-id=%1, related=%2, reason=%3, advice=%4).")
                .arg(message.messageId, relatedLabel, reason, advice));
}

int AlkamelSession::parsePositiveIntegerField(const QJsonObject &object, const QString &field, int fallbackValue)
{
    const QJsonValue value = object.value(field);
    int parsedValue = -1;

    if (value.isString()) {
        bool ok = false;
        parsedValue = value.toString().toInt(&ok);
        if (!ok) {
            parsedValue = -1;
        }
    } else if (value.isDouble()) {
        parsedValue = value.toInt(-1);
    }

    if (parsedValue > 0) {
        return parsedValue;
    }

    emitLog(QStringLiteral("Invalid '%1' value in LOGIN reply. Using fallback=%2.")
                .arg(field)
                .arg(fallbackValue));
    return fallbackValue;
}

QString AlkamelSession::takePendingCommandLabel(const QString &messageId)
{
    if (messageId.isEmpty()) {
        return QString();
    }

    const auto it = m_pendingCommands.find(messageId);
    if (it == m_pendingCommands.end()) {
        return QString();
    }

    const ProtocolCommand command = it.value();
    m_pendingCommands.erase(it);
    return commandLabel(command);
}

void AlkamelSession::emitLog(const QString &message)
{
    emit logMessage(message);
}

} // namespace network
