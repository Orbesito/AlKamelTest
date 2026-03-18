#include "app/MainWindow.h"

#include <QDateTime>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "network/AppConfig.h"
#include "network/AlkamelSession.h"

namespace app
{

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();

    m_session = new network::AlkamelSession(network::AppConfig::fromAssignmentDefaults(), this);
    connectSignals();
}

void MainWindow::start()
{
    appendLog(QStringLiteral("Phase 2 bootstrap: starting protocol session."));
    m_session->start();
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
        m_session->stop();
        m_session->start();
    });
}

void MainWindow::connectSignals()
{
    connect(m_session, &network::AlkamelSession::connected, this, [this]() {
        m_statusLabel->setText(QStringLiteral("Status: connected"));
    });

    connect(m_session, &network::AlkamelSession::loggedIn, this, [this]() {
        m_statusLabel->setText(QStringLiteral("Status: logged in"));
    });

    connect(m_session, &network::AlkamelSession::disconnected, this, [this]() {
        m_statusLabel->setText(QStringLiteral("Status: disconnected"));
    });

    connect(m_session, &network::AlkamelSession::logMessage, this, [this](const QString &message) {
        appendLog(message);
    });
}

} // namespace app
