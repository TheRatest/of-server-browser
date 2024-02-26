#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow hWnd;
    hWnd.show();
    return a.exec();
}

