#include "network/AlkamelClient.h"

#include <QAbstractSocket>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QtGlobal>
#include <utility>

namespace
{

QString socketErrorToString(QAbstractSocket::SocketError error)
{
    return QString::number(static_cast<int>(error));
}

} // namespace

namespace network
{

AlkamelClient::AlkamelClient(AppConfig config, QObject *parent)
    : QObject(parent),
      m_config(std::move(config))
{
    m_socket.setPeerVerifyMode(QSslSocket::VerifyNone);

    connect(&m_socket, &QSslSocket::connected, this, &AlkamelClient::onConnected);
    connect(&m_socket, &QSslSocket::disconnected, this, &AlkamelClient::onDisconnected);
    connect(&m_socket, &QSslSocket::readyRead, this, &AlkamelClient::onReadyRead);
    connect(&m_socket, &QSslSocket::sslErrors, this, &AlkamelClient::onSslErrors);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(&m_socket, &QSslSocket::errorOccurred, this, &AlkamelClient::onSocketError);
#else
    connect(&m_socket, qOverload<QAbstractSocket::SocketError>(&QSslSocket::error), this, &AlkamelClient::onSocketError);
#endif
}

void AlkamelClient::connectToServer()
{
    if (m_socket.state() == QAbstractSocket::ConnectedState || m_socket.state() == QAbstractSocket::ConnectingState) {
        emitLog(QStringLiteral("Connect request ignored: socket is already connected/connecting."));
        return;
    }

    m_readBuffer.clear();
    emitLog(QStringLiteral("Connecting with SSL to %1:%2 ...").arg(m_config.host).arg(m_config.port));
    m_socket.connectToHostEncrypted(m_config.host, m_config.port);
}

void AlkamelClient::disconnectFromServer()
{
    emitLog(QStringLiteral("Disconnect requested."));
    m_socket.disconnectFromHost();
}

void AlkamelClient::sendRawLine(const QString &line)
{
    if (!isConnected()) {
        emitLog(QStringLiteral("Cannot send line while disconnected: %1").arg(line));
        return;
    }

    QByteArray payload = line.toUtf8();
    payload.append("\r\n");
    m_socket.write(payload);

    emitLog(QStringLiteral("TX: %1").arg(line));
}

bool AlkamelClient::isConnected() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

void AlkamelClient::onConnected()
{
    emitLog(QStringLiteral("SSL connection established."));
    emit connected();
}

void AlkamelClient::onDisconnected()
{
    emitLog(QStringLiteral("Disconnected from server."));
    emit disconnected();
}

void AlkamelClient::onReadyRead()
{
    m_readBuffer.append(m_socket.readAll());

    while (true) {
        const int terminatorIndex = m_readBuffer.indexOf("\r\n");
        if (terminatorIndex < 0) {
            break;
        }

        const QByteArray rawLine = m_readBuffer.left(terminatorIndex);
        m_readBuffer.remove(0, terminatorIndex + 2);

        const QString decoded = QString::fromUtf8(rawLine);
        emitLog(QStringLiteral("RX: %1").arg(decoded));
        emit rawLineReceived(decoded);
    }
}

void AlkamelClient::onSslErrors(const QList<QSslError> &errors)
{
    if (!errors.isEmpty()) {
        QStringList messages;
        messages.reserve(errors.size());
        for (const QSslError &error : errors) {
            messages.push_back(error.errorString());
        }
        emitLog(QStringLiteral("SSL errors received: %1").arg(messages.join(QStringLiteral(" | "))));
    }

    emitLog(QStringLiteral("Ignoring SSL certificate errors (assignment requirement)."));
    m_socket.ignoreSslErrors();
}

void AlkamelClient::onSocketError(QAbstractSocket::SocketError socketError)
{
    emitLog(QStringLiteral("Socket error (%1): %2")
                .arg(socketErrorToString(socketError), m_socket.errorString()));
}

void AlkamelClient::emitLog(const QString &message)
{
    qInfo().noquote() << message;
    emit logMessage(message);
}

} // namespace network
