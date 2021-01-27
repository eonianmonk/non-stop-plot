#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMetaEnum>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    portData = new QByteArray(8, Qt::Initialization());
    //packet = new tUart_PACKET();
    decoder = new tUart_Decoder(this);

    //def = new DeviceEventFilter();

    plotWidth = 5000;
    ui->thePlot->init(plotWidth,10,ui->oxAccessibleArea->value());
    ui->oxScaleBox->setValue(plotWidth);
    //ui->thePlot->setRanges(plotWidth,100);
    ui->plotScrollBar->setPageStep(plotWidth);

    // adding available ports to combobox
    ui->portBox->clear();
    refreshPorts();
    const QMetaEnum bitrates = QMetaEnum::fromType<QSerialPort::BaudRate>();
    for(int i = 0; i < bitrates.keyCount() - 1; i++)
    {
        ui->bitrateBox->addItem(QString::number(bitrates.value(i)));
    }
    // signals - slots
    connect(this, &MainWindow::createPoint, ui->thePlot, &MyPlot::addPoint);
    connect(this, &MainWindow::qualPlotScroll, ui->thePlot, &MyPlot::plotHorizontalScrollBarValueChange);
    connect(ui->oxScaleBox, QOverload<int>::of(&QSpinBox::valueChanged), ui->thePlot, &MyPlot::rangeChanged);
    connect(ui->oxAccessibleArea, QOverload<int>::of(&QSpinBox::valueChanged), ui->thePlot, &MyPlot::areaChanged);
    connect(this, &MainWindow::horizontalScrolled, ui->thePlot, &MyPlot::plotHorizontalScrollBarValueChange);

    // scrollbar
    connect(ui->plotScrollBar, &QScrollBar::valueChanged, this,  &MainWindow::hscrolled);
    connect(ui->plotScrollBar, &QScrollBar::sliderPressed, this, [this](){this->oxSliderPressed = true;}); // works
    connect(ui->plotScrollBar, &QScrollBar::sliderReleased, this, [this](){this->oxSliderPressed = false;}); // works

    // plot press lock
    connect(ui->plotScrollBar, &QScrollBar::sliderPressed, ui->thePlot, &MyPlot::pressLock);
    connect(ui->plotScrollBar, &QScrollBar::sliderReleased, ui->thePlot, &MyPlot::pressLock);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshPorts()
{
    for(auto& port : QSerialPortInfo::availablePorts())
    {
        ui->portBox->addItem(port.portName(), port.systemLocation());
    }
}

void MainWindow::portAvailability()
{
    //if(ui->bitrateBox->currentText() != "" && ui->portBox->currentText() != "")
    if(ui->bitrateBox->currentText() != "" && ui->portBox->currentText() != "")
    {
        ui->portPushButton->setEnabled(true);
    }
    else
    {
        ui->portPushButton->setEnabled(false);
    }
}

void MainWindow::portChosen()
{
    if(ui->bitrateBox->isEnabled())
    {
        ui->portBox->setEnabled(false);
        ui->bitrateBox->setEnabled(false);
        ui->portPushButton->setText("Choose other");

        sport = new QSerialPort(this);
        sport->setPortName(ui->portBox->currentText());
        sport->setBaudRate(ui->bitrateBox->currentText().toInt());
        sport->setStopBits(QSerialPort::OneStop);
        sport->setFlowControl(QSerialPort::NoFlowControl);
        sport->setParity(QSerialPort::NoParity);

        if(sport->open(QIODevice::ReadOnly))
        {
            qDebug() << "port opened succesfully";
            connect(sport, &QSerialPort::readyRead, this, &MainWindow::readData);
        }
        else
        {
            qDebug() << "no device found: " << sport->error();
        }
    }
    else
    {
        sport->close();

        ui->portBox->setEnabled(true);
        ui->bitrateBox->setEnabled(true);
        ui->portPushButton->setText("Choose port");

        ui->plotDataCleaner->setDisabled(true);

        disconnect(sport, &QSerialPort::readyRead, this, &MainWindow::readData);

        delete sport;
    }
}

void MainWindow::readData()
{
    const int packet_size = sizeof(tUart_PACKET);
    if(sport->bytesAvailable() >= packet_size && sport->bytesAvailable()%packet_size == 0)
    {
        tUart_PACKET packet_temp;
        int total_bytes = sport->bytesAvailable();
        *portData = sport->readAll();
        for(int i = 0; i < total_bytes/packet_size; i++)
        {
            decoder->decode(portData->mid(i*packet_size,packet_size), &packet_temp);
            data.append(packet_temp);
        }
        for(auto& pack : data)
        {
            //qDebug() << pack.time<< pack.data;
            tUartPlotData_uqptr p(new tUart_plotData(pack.type, new QPointF(pack.time, pack.data)));
            emit this->createPoint(p);
        }
        data.clear();
        portData->clear();

        updatePlot();
        this->ui->plotDataCleaner->setEnabled(true);
    }
    else
    {
        qDebug()<< "Not reading: bytes available:" << sport->bytesAvailable();
    }
}

void MainWindow::hscrolled(int newValue)
{
    if(oxSliderPressed)
    {
        emit this->horizontalScrolled(newValue);
    }
}

void MainWindow::updatePlot()
{
    int32_t pos = (uint32_t)ui->plotScrollBar->value(), barWidth, plottingArea;
    bool freeze = ui->thePlot->mreplot(pos, barWidth, plottingArea);

    if(barWidth > plottingArea)
    {
        ui->plotScrollBar->setDisabled(true);
    }
    else
    {
        if(!ui->plotScrollBar->isEnabled()) ui->plotScrollBar->setDisabled(false);

        if(freeze) ui->plotScrollBar->setValue(pos);
        //if(freeze) qDebug() << "Outer value:"<< ui->plotScrollBar->value();
        ui->plotScrollBar->setSingleStep(barWidth);
        ui->plotScrollBar->setMaximum(plottingArea);
    }
    //ui->plotScrollBar->setRange(width<plotWidth?0:width-plotWidth, width);
}

void MainWindow::comConnected(QString comName)
{
    qDebug() << comName;
}
