#include "mainwindow.h"
#include "draw.h"

#include <QApplication>
#include "drawwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

     MainWindow w;

    w.setWindowTitle("Simulation");
    w.showMaximized();
    w.showFullScreen();
    w.show();
    return a.exec();
}
