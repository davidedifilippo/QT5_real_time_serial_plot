#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QString>
#include "settingsdialog.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void realTimeDataSlot();
    void openSerialPort();
    void closeSerialPort();
    void readData();

private:
    void initActionsConnections();

private:
    void showStatusMessage(const QString &message);

private:
    Ui::MainWindow *ui;
    QVector<double> x, y; // initialize with entries 0..100
    QTimer dataTimer;
    SettingsDialog *settings;
    QSerialPort *serial;
    QString lastDataAv;
    QStringList datalist;
    QList<QList<double> > parseddatalist;
    QString lastdatalistline;
    QString firstdatalistline;
    QString initchar;
    bool     isfirst;

};

#endif // MAINWINDOW_H
