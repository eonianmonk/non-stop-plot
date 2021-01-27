#pragma once
#include <memory>
#include <QMainWindow>
#include <QSerialPort>
#include <QDebug>
#include "auxillary/decoder.h"
//#include "auxillary/usbeventlistener.h"

//#include <iomanip>
//#define HEX( x, bytew ) std::setw(bytew*2) << std::setfill('0') << std::hex << (int)( x )

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //DeviceEventFilter *def;

private:
    Ui::MainWindow *ui;

    QSerialPort *sport;
    tUart_Decoder *decoder;
    QByteArray *portData;
    tUart_PACKET *packet;

    uint32_t plotWidth = 10000;

    bool updateLock;
    bool oxSliderPressed;

    void updatePlot();

private slots:
    void portAvailability();
    void portChosen();
    void readData();
    void hscrolled(int);
    void comConnected(QString);
    void refreshPorts();

signals:
    void horizontalScrolled(int);
    void qualPlotScroll(int, bool);
    void createPoint(std::unique_ptr<QPair<uint16_t, QPointF*>>&);
};
