#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),x(101),y(101), dataTimer(new QTimer(this)),lastDataAv {""}, initchar{"!"}
{
    ui->setupUi(this);

    serial = new QSerialPort(this);
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    settings = new SettingsDialog;

    initActionsConnections();


    ui->customplot->addGraph(); // blue line
    ui->customplot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->customplot->addGraph(); // red line
    ui->customplot->graph(1)->setPen(QPen(QColor(255, 110, 40)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->customplot->xAxis->setTicker(timeTicker);
    ui->customplot->axisRect()->setupFullAxesBox();
    ui->customplot->yAxis->setRange(0, 100);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customplot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customplot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customplot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customplot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realTimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible

    lastdatalistline = "";
    isfirst = true;

//    for (int i=0; i<101; ++i)
//    {
//      x[i] = i/5.0 - 10; // x goes from -1 to 1
//      y[i] = x[i]*x[i]*x[i]; // let's plot a quadratic function
//    }

//    ui->customplot->addGraph();
//    ui->customplot->graph(0)->setData(x,y);

//    ui->customplot->xAxis->setLabel("x");
//    ui->customplot->xAxis->setLabel("y");

//    ui->customplot->xAxis->setRange(-11, 11);
//    ui->customplot->yAxis->setRange(-1200,1200);
//    ui->customplot->replot();

}
MainWindow::~MainWindow()
{
    delete ui;
    delete settings;


}

void MainWindow::realTimeDataSlot()
{
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;

    //Trattamento dati ricevuti


    if(!lastDataAv.isEmpty()) {
        datalist.clear();
        lastDataAv.remove("\n");
        lastDataAv.remove("\r");
        datalist.append(lastDataAv.split("#"));
        //qDebug("dtalistlength = %d", datalist.length());
        //qDebug(qUtf8Printable(lastDataAv));

        //  a questo punto datalist contiene varie stringhe di dati
        //  se isfirst=false il primo e l'ultimo tuttavia potrebbero essere
        //  lastdatalistline = data2 /t data3 /t data4
        //  firstdatalistline =#  /t data1 /t
        //  o simile

        firstdatalistline = datalist.takeFirst();

        if(!isfirst) {
            datalist.insert(0, lastdatalistline + firstdatalistline);

            //  ricostruisce la stringa prendendo i due pezzi
            //  finale della lettura precedente
            //  iniziale della lettura presente
        }

        isfirst = false;
        lastdatalistline = datalist.takeLast();
        //salva l'ultimo pezzo della lettura presente rimuovendolo dalla lista
        //da adesso in poi sono solo tanti pezzi interi di dati del tipo # .. /t .. /t .. /t ..



         QList<QString>::ConstIterator i;

         for (i = datalist.begin(); i != datalist.end(); ++i){

            QStringList chunks = (*i).split("\t"); //si separano i dati sulla base del carattere di separazione

            if(chunks.at(0) == initchar) {
            chunks.removeFirst();

            QList<double> currentdataset;

            QList<QString>::ConstIterator k;

            for (k = chunks.begin(); k != chunks.end(); ++k){
                currentdataset.append((*k).toFloat());
                 //qDebug("float = %f", (*k).toFloat());
            }
            parseddatalist.append(currentdataset); //ogni currentdataset contiene solo un chunk mentre parsedatalist tutti i chunks
            //qDebug("currentdataset = %d", currentdataset.length());
        }
    }

    }



    //if (key-lastPointKey > 0.002) // at most add point every 2 ms
    {
     foreach (QList<double> parseddata, parseddatalist ) {
     ui->customplot->graph(1)->addData(key, parseddata.at(1));
     //ui->customplot->graph(0)->addData(key, parseddata.at(1));

        }
      // add data to lines:



      // rescale value (vertical) axis to fit ;the current data:
      //ui->customPlot->graph(0)->rescaleValueAxis();
      //ui->customPlot->graph(1)->rescaleValueAxis(true);
      lastPointKey = key;
    }

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customplot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->customplot->replot();
    lastDataAv.clear();
    parseddatalist.clear();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
      ui->statusBar->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(ui->customplot->graph(0)->data()->size()+ui->customplot->graph(1)->data()->size())
            , 0);
      lastFpsKey = key;
      frameCount = 0;

    }
}

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionConfigure, &QAction::triggered, settings, &SettingsDialog::show);
}

void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);

    //Tentativo di apertura della seriale scelta

    if (serial->open(QIODevice::ReadWrite)) {

        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        //La seriale non può essere aperta per qualche motivo
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}

void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}

void MainWindow::readData()
{
    //quint64 dataAvalaible = serial->bytesAvailable();

    QByteArray data = serial->readAll(); //legge tutti i dati in attesa

    lastDataAv.append(data);

    //qDebug(qUtf8Printable(lastDataAv));

    }





void MainWindow::showStatusMessage(const QString &message)
{
   ui->statusBar->showMessage(message);
}
