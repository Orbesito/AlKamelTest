#pragma once

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QTimer>

#include "network/AlkamelClient.h"
#include "network/AppConfig.h"
#include "network/ProtocolMessage.h"

namespace network
{

class AlkamelSession : public QObject
{
    Q_OBJECT

public:
    explicit AlkamelSession(AppConfig config, QObject *parent = nullptr);

    void start();
    void stop();

    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] bool isLoggedIn() const;

signals:
    void connected();
    void disconnected();
    void loggedIn();
    void jsonPayloadReceived(const QJsonObject &partialUpdate);
    void logMessage(const QString &message);

private slots:
    void onTransportConnected();
    void onTransportDisconnected();
    void onRawLineReceived(const QString &line);
    void onPingTimerTimeout();

private:
    QString nextMessageId();
    void clearSessionState();
    void sendLogin();
    void sendJoin(const QString &channel);
    void sendPing();
    void sendMessage(const ProtocolMessage &message);
    void dispatchIncoming(const ProtocolMessage &message);
    void handleLoginReply(const ProtocolMessage &message);
    void handleJoinReply(const ProtocolMessage &message);
    void handleAck(const ProtocolMessage &message);
    void handleError(const ProtocolMessage &message);
    int parsePositiveIntegerField(const QJsonObject &object, const QString &field, int fallbackValue);
    QString takePendingCommandLabel(const QString &messageId);
    void emitLog(const QString &message);

    AppConfig m_config;
    AlkamelClient *m_transport = nullptr;
    QTimer m_pingTimer;
    int m_nextMessageId = 1;
    int m_pingRateSeconds = 20;
    int m_timeoutSeconds = 40;
    bool m_loggedIn = false;
    QHash<QString, ProtocolCommand> m_pendingCommands;
};

} // namespace network
