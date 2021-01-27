#pragma once
#include <memory>
#include <QMainWindow>
#include <QSerialPort>
#include <QDebug>
#include "auxillary/decoder.h"
//#include "auxillary/usbeventlistener.h"

//#include <iomanip>
//#define HEX( x, bytew ) std::setw(bytew*2) << std::setfill('0') << std::hex << (int)( x )

typedef std::unique_ptr<QPair<uint16_t, QPointF*>> tUartPlotData_uqptr;
typedef QPair<uint16_t, QPointF*> tUart_plotData;

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
    //tUart_PACKET *packet;

    uint32_t plotWidth = 10000;
    uint32_t lpAmount = 0;
    QVector<tUart_PACKET> data;

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
