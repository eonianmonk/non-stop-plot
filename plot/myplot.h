#pragma once
#include "qcustomplot.h"
#include <utility>
#include <algorithm>
#include <functional>
#include <QPointer>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <limits>
//#include <QEvent>

typedef std::unique_ptr<QCPRange> unique_rng_ptr;

QT_BEGIN_NAMESPACE
namespace Ui { class MyPlot; }
QT_END_NAMESPACE

class MyPlot : public QCustomPlot
{
    Q_OBJECT
    ~MyPlot();
    using QCustomPlot::QCustomPlot;
public:
    //explicit MyPlot(QObject *parent = nullptr);
    void init(uint32_t, uint32_t, uint32_t);
    void setRanges(uint32_t, uint32_t);

    uint32_t hRange;
    uint32_t vRange;
    uint32_t plottingRange;
    bool freeze;

    //QSharedPointer<QCPGraphDataContainer> graph_container;
    // uint16_t - data type
    QMap<uint16_t, QSharedPointer<QCPGraphDataContainer>> graphs;
    QPair<QPointer<QCPItemLine>, QPointer<QCPItemLine>> point_lines; // 1 - horiz, 2 - vertic


    const QCPGraphData* findGraphElement(QPointF*);
    // lines pointing the found line
    QPointer<QCPItemText> pointCoordsText;
    QPointer<QCPItemLine> displayLine(double, double, bool, const QPen&);
    void displayPointLines(const QCPGraphData*, const QPen&, const QCPRange&, bool);
    void flushNonGraph();

    // returns whether scrollbar should be changed
    // 1 - possible bar value, 2 - bar width(page step), 3 - total width(maximum - minimum)
    bool mreplot(int32_t&, int32_t&, int32_t&);

public slots:
    void plotHorizontalScrollBarValueChange(int);
    void plotVerticalScrollBarValueChange(int); // unimplemented
    void addPoint(std::unique_ptr<QPair<uint16_t, QPointF*>>&);
    void plotPress(QMouseEvent*);
    void rangeChanged(int);
    void areaChanged(int);
    void clearData();
    void pressLock();

signals:
    void mouseHoverEvent(QMouseEvent*, QPointF*);
    void plotMoved(QCPRange* oy, QCPRange* ox);

private:
    bool pressLocked = false;

    // finds closed(by approximation) point in container
    // supposing graph data is sorted (!)
    QCPGraphDataContainer::const_iterator findApproximation(QPointF*, const QCPGraphDataContainer&, double&);

    // replots OY axis based on minimums&maximums of all graphs in selected OX range
    void resizeOY(const QCPRange&);

    // finds local oy limits
    // param - ox range
    unique_rng_ptr findLocalOYRange(const QCPRange&);
    unique_rng_ptr actuallyFindOYRange(QCPGraphDataContainer::const_iterator, QCPGraphDataContainer::const_iterator);

    //default text
    void defaultText(QPointer<QCPItemText> object);

    uint16_t totalGraphs = 0;
    double approximationAreaV;
    double approximationAreaH;
    QCPRange XRange;
    //QCPRange YRange;

    // colors
    QList<Qt::GlobalColor> colorList;
    int redInd;
};

