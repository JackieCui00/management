#include <QApplication>

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    ::QApplication app(argc, argv);

    ::self::MainWindow window;
    if (!window.start()) {
        return -1;
    }

    window.show();

    return app.exec();
}
