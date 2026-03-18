#include <QApplication>

#include "app/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app::MainWindow mainWindow;
    mainWindow.show();
    mainWindow.start();

    return app.exec();
}
