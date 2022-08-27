#include <QApplication>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
qApp->setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.resize(800,600);
    w.show();

    return app.exec();
}
