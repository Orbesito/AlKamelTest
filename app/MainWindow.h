#pragma once

#include <QMainWindow>
#include <QString>

class QLabel;
class QPlainTextEdit;

namespace network
{
class AlkamelClient;
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
    network::AlkamelClient *m_client = nullptr;
};

} // namespace app
