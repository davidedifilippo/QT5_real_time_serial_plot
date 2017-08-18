#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);





    MainWindow w;
    w.setWindowTitle("Serial Port Plotter");
    QIcon appIcon(":/Icons/exe_icon.ico");
    w.setWindowIcon(appIcon);
    w.show();

    return a.exec();  //main event loop
}
