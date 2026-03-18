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

    AppConfig m_config;
    QSslSocket m_socket;
    QByteArray m_readBuffer;
};

} // namespace network
