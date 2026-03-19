#pragma once

#include <QMainWindow>
#include <QString>

#include "state/JsonStateStore.h"

class QLabel;
class QPlainTextEdit;
class QTableView;

namespace network
{
class AlkamelSession;
}

namespace ui
{
class ClassificationTableModel;
}

namespace app
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void start();

private:
    void appendLog(const QString &message);
    void buildUi();
    void connectSignals();

    QLabel *m_statusLabel = nullptr;
    QTableView *m_tableView = nullptr;
    ui::ClassificationTableModel *m_tableModel = nullptr;
    QPlainTextEdit *m_logView = nullptr;
    network::AlkamelSession *m_session = nullptr;
    state::JsonStateStore m_stateStore;
    int m_jsonUpdateCount = 0;
};

} // namespace app
