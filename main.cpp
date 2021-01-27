#include "mainwindow.h"
//#include <auxillary/usbeventlistener.h>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //a.installNativeEventFilter(w.def);
    //a.installEventFilter(w.def);
    //a.eventDispatcher()

    w.show();
    return a.exec();
}
