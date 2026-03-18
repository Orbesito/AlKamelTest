#include "app/MainWindow.h"

#include <QDateTime>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "network/AlkamelClient.h"
#include "network/AppConfig.h"

namespace app
{

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();

    m_client = new network::AlkamelClient(network::AppConfig::fromAssignmentDefaults(), this);
    connectSignals();
}

void MainWindow::start()
{
    appendLog(QStringLiteral("Phase 1 bootstrap: attempting SSL connection."));
    m_client->connectToServer();
}

void MainWindow::appendLog(const QString &message)
{
    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    m_logView->appendPlainText(QStringLiteral("[%1] %2").arg(timestamp, message));
}

void MainWindow::buildUi()
{
    setWindowTitle(QStringLiteral("Alkamel Classification Client"));
    resize(900, 560);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    m_statusLabel = new QLabel(QStringLiteral("Status: idle"), central);
    m_logView = new QPlainTextEdit(central);
    m_logView->setReadOnly(true);

    auto *reconnectButton = new QPushButton(QStringLiteral("Reconnect"), central);

    layout->addWidget(m_statusLabel);
    layout->addWidget(reconnectButton);
    layout->addWidget(m_logView, 1);

    setCentralWidget(central);

    connect(reconnectButton, &QPushButton::clicked, this, [this]() {
        appendLog(QStringLiteral("Reconnect button clicked."));
        m_client->disconnectFromServer();
        m_client->connectToServer();
    });
}

void MainWindow::connectSignals()
{
    connect(m_client, &network::AlkamelClient::connected, this, [this]() {
        m_statusLabel->setText(QStringLiteral("Status: connected"));
    });

    connect(m_client, &network::AlkamelClient::disconnected, this, [this]() {
        m_statusLabel->setText(QStringLiteral("Status: disconnected"));
    });

    connect(m_client, &network::AlkamelClient::rawLineReceived, this, [this](const QString &) {
        // Raw lines are already logged by AlkamelClient in phase 1.
    });

    connect(m_client, &network::AlkamelClient::logMessage, this, [this](const QString &message) {
        appendLog(message);
    });
}

} // namespace app
