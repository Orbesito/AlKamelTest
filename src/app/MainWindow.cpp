#include "app/MainWindow.h"

#include <QDateTime>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

#include "domain/ClassificationBuilder.h"
#include "domain/RaceState.h"
#include "network/AppConfig.h"
#include "network/AlkamelSession.h"
#include "ui/ClassificationTableModel.h"

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
    m_stateStore.clear();
    m_tableModel->setRows({});
    m_jsonUpdateCount = 0;
    appendLog(QStringLiteral("Phase 5 bootstrap: starting protocol session with full UI model/view."));
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
    resize(1280, 760);

    // Qt widget/layout objects are heap-allocated and attached to the QObject parent tree.
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    m_statusLabel = new QLabel(QStringLiteral("Status: idle"), central);
    m_connectButton = new QPushButton(QStringLiteral("Connect"), central);
    m_disconnectButton = new QPushButton(QStringLiteral("Disconnect"), central);
    m_disconnectButton->setEnabled(false);

    auto *topBar = new QHBoxLayout();
    topBar->addWidget(m_statusLabel);
    topBar->addStretch(1);
    topBar->addWidget(m_connectButton);
    topBar->addWidget(m_disconnectButton);

    m_tableModel = new ui::ClassificationTableModel(this);
    m_tableView = new QTableView(central);
    m_tableView->setModel(m_tableModel);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSortingEnabled(false);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setStretchLastSection(true);

    m_logView = new QPlainTextEdit(central);
    m_logView->setReadOnly(true);
    m_logView->setMaximumBlockCount(3000);
    m_logView->setPlaceholderText(QStringLiteral("Runtime logs..."));

    auto *splitter = new QSplitter(Qt::Vertical, central);
    splitter->addWidget(m_tableView);
    splitter->addWidget(m_logView);
    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({500, 220});

    layout->addLayout(topBar);
    layout->addWidget(splitter, 1);

    setCentralWidget(central);

    connect(m_connectButton, &QPushButton::clicked, this, [this]() {
        if (m_session->isConnected()) {
            appendLog(QStringLiteral("Connect requested, but session is already connected."));
            return;
        }

        appendLog(QStringLiteral("Connect button clicked."));
        m_stateStore.clear();
        m_tableModel->setRows({});
        m_jsonUpdateCount = 0;
        m_session->start();
    });

    connect(m_disconnectButton, &QPushButton::clicked, this, [this]() {
        appendLog(QStringLiteral("Disconnect button clicked."));
        m_session->stop();
    });
}

void MainWindow::connectSignals()
{
    connect(m_session, &network::AlkamelSession::connected, this, [this]() {
        m_statusLabel->setText(QStringLiteral("Status: connected"));
        m_connectButton->setEnabled(false);
        m_disconnectButton->setEnabled(true);
    });

    connect(m_session, &network::AlkamelSession::loggedIn, this, [this]() {
        m_statusLabel->setText(QStringLiteral("Status: logged in"));
        m_connectButton->setEnabled(false);
        m_disconnectButton->setEnabled(true);
    });

    connect(m_session, &network::AlkamelSession::disconnected, this, [this]() {
        m_statusLabel->setText(QStringLiteral("Status: disconnected"));
        m_connectButton->setEnabled(true);
        m_disconnectButton->setEnabled(false);
    });

    connect(m_session, &network::AlkamelSession::logMessage, this, [this](const QString &message) {
        appendLog(message);
    });

    connect(m_session, &network::AlkamelSession::jsonPayloadReceived, this, [this](const QJsonObject &partialUpdate) {
        m_stateStore.mergeUpdate(partialUpdate);
        const domain::ClassificationBuilder::BuildResult buildResult = domain::ClassificationBuilder::build(m_stateStore.root());
        m_tableModel->setRows(buildResult.rows);
        ++m_jsonUpdateCount;

        // Keep the log readable: show state-merge snapshots periodically.
        if (m_jsonUpdateCount == 1 || (m_jsonUpdateCount % 15) == 0) {
            appendLog(QStringLiteral("State merged (%1 updates). Root keys: %2")
                          .arg(m_jsonUpdateCount)
                          .arg(m_stateStore.rootKeysSummary()));

            if (!m_tableModel->rows().isEmpty()) {
                const domain::ClassificationRow &leader = m_tableModel->rows().first();
                appendLog(QStringLiteral("Classification rows=%1 | Leader P%2 #%3 %4 | status=%5")
                              .arg(m_tableModel->rows().size())
                              .arg(leader.position)
                              .arg(leader.carNumber)
                              .arg(leader.driverName)
                              .arg(domain::raceStatusToString(leader.status)));
            } else {
                appendLog(QStringLiteral("Classification rows=0 (waiting for complete mapping data)."));
            }

            if (!buildResult.warnings.isEmpty()) {
                appendLog(QStringLiteral("Classification info: %1")
                              .arg(buildResult.warnings.join(QStringLiteral(" | "))));
            }
        }
    });
}

} // namespace app
