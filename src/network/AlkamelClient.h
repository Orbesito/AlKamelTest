#pragma once

#include <QObject>
#include <QSslError>
#include <QSslSocket>

#include "network/AppConfig.h"

namespace network
{

class AlkamelClient : public QObject
{
    Q_OBJECT

public:
    explicit AlkamelClient(AppConfig config, QObject *parent = nullptr);

    void connectToServer();
    void disconnectFromServer();
    void sendRawLine(const QString &line);

    [[nodiscard]] bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void rawLineReceived(const QString &line);
    void logMessage(const QString &message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSslErrors(const QList<QSslError> &errors);
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    void emitLog(const QString &message);

    // Connection settings are copied by value so the transport stays self-contained.
    AppConfig m_config;
    // Value member: QSslSocket lives inside AlkamelClient, no manual delete required.
    QSslSocket m_socket;
    // Persistent receive buffer used to reassemble line-delimited frames across reads.
    QByteArray m_readBuffer;
};

} // namespace network
