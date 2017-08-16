#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QIcon appIcon(":/Ico/Icons/serial_port_icon.icns");                       // Get the icon for the right corner
    w.setWindowIcon(appIcon);
    w.setWindowTitle("Serial Port Plotter");
    w.show();

    return a.exec();  //main event loop
}
