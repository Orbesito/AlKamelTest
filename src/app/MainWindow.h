#pragma once

#include <QMainWindow>
#include <QString>

#include "state/JsonStateStore.h"

class QLabel;
class QPlainTextEdit;

namespace network
{
class AlkamelSession;
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
    QPlainTextEdit *m_logView = nullptr;
    network::AlkamelSession *m_session = nullptr;
    state::JsonStateStore m_stateStore;
    int m_jsonUpdateCount = 0;
};

} // namespace app
