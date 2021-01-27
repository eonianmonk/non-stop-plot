#include "myplot.h"

// vpps - vertical points per screen(plot)
// hpps - horizontalpoints per screen(plot)
// tt - total plotting width
void MyPlot::init(uint32_t hr, uint32_t vr, uint32_t tt)
{
    hRange = hr;
    vRange = vr;
    if(tt > hr*2)
    {
        plottingRange = tt;
    }
    else plottingRange = hr*3;

    approximationAreaV = hRange/130;
    approximationAreaH = vRange/130;

    this->xAxis->setRange(0, hRange);
    this->yAxis->setRange(400, 500);
    this->xAxis->setLabel("X");
    this->yAxis->setLabel("Y");

    connect(this, &MyPlot::mousePress, this, &MyPlot::plotPress);

    freeze = false;

    XRange = QCPRange();
    XRange.lower = std::numeric_limits<double>::max();

    // loop through enums to get iterator to reserved red color
    QMetaEnum e = QMetaEnum::fromType<Qt::GlobalColor>();
    colorList = QList<Qt::GlobalColor>();

    for(int i = 0; i < e.keyCount();i++)
    {
        colorList.push_back((Qt::GlobalColor)e.value(i));
        if((Qt::GlobalColor)e.value(i) == Qt::red)
        {
            redInd = i+1;
        }
    }
}

void MyPlot::setRanges(uint32_t hr, uint32_t vr)
{
    vRange = vr;
    hRange = hr;

    approximationAreaV = vr/130;
    approximationAreaH = hr/130;
}

MyPlot::~MyPlot()
{
    graphs.clear();
}

void MyPlot::displayPointLines(const QCPGraphData* point, const QPen &pen, const QCPRange &xrange, bool vComments)
{
    if(point == nullptr)
    {
        return;
    }
    if(!this->point_lines.first.isNull())
    {
        this->removeItem(this->point_lines.first);
        this->removeItem(this->point_lines.second);
    }

    point_lines.first = this->displayLine(point->key, point->value, true, pen);
    point_lines.second = this->displayLine(point->key, point->value, false, pen);

    if(vComments)
    {
        pointCoordsText = new QCPItemText(this);
        this->defaultText(pointCoordsText);
        pointCoordsText->setText("(" + QString::number(point->key) + ", " + QString::number(point->value) + ")");
        pointCoordsText->setColor(pen.color());

        if(point->key > xrange.lower + 15)
        {
            pointCoordsText->position->setCoords(xrange.lower, point->value);
        }
        else
        {
            pointCoordsText->position->setCoords(point->key, point->value);
        }
        //point_text.second->position->setCoords(point->key + 1, point->value + 1);
    }

}

// returns displayed line
QPointer<QCPItemLine> MyPlot::displayLine(double x, double y, bool horizontal, const QPen &pen)
{
   QCPItemLine* line = new QCPItemLine(this);
   line->setPen(pen);
   line->setTail(QCPLineEnding::esNone);
   line->setHead(QCPLineEnding::esNone);

   if(horizontal)
   {
       line->start->setCoords(this->xAxis->range().lower, y);
       line->end->setCoords(this->xAxis->range().upper, y);
    }
    else
   {
       line->start->setCoords(x, this->yAxis->range().lower);
       line->end->setCoords(x, this->yAxis->range().upper);
   }
   return line;
}

void MyPlot::defaultText(QPointer<QCPItemText> object)
{
    object->position->setType(QCPItemPosition::ptPlotCoords);
    object->setPositionAlignment(Qt::AlignLeft|Qt::AlignBottom);
    //object->position->setCoords(gd_CONTAINER->begin()->key, (gd_CONTAINER->end()-1)->value); // lower right corner of axis rect
    object->setText("Points of fixed\nphase define\nphase velocity vp");
    object->setTextAlignment(Qt::AlignLeft);

    QFont *font = new QFont("ISOCPEUR", 12, 2, true);
    object->setFont(*font);
    object->setPadding(QMargins(4,0,0,2));
}

void MyPlot::flushNonGraph()
{
    if(!pointCoordsText.isNull())
    {
        this->removeItem(this->pointCoordsText);
        this->pointCoordsText.clear();
    }
    if(!point_lines.first.isNull())
    {
        this->removeItem(this->point_lines.first);
        this->removeItem(this->point_lines.second);
        this->point_lines.second.clear();
        this->point_lines.first.clear();
    }
}

void MyPlot::clearData()
{
    this->graphs.clear();
    totalGraphs = 0;
}

const QCPGraphData* MyPlot::findGraphElement(QPointF *point)
{
    QMapIterator<uint16_t, QSharedPointer<QCPGraphDataContainer>> it(graphs);
    QCPGraphDataContainer::const_iterator gdc_it, bestHit = nullptr;
    double closestDiff = std::numeric_limits<double>::max(), diff;

    // loop through all graphs
    while(it.hasNext())
    {
        gdc_it = findApproximation(point, *it.value(), diff);

        if(gdc_it != nullptr && closestDiff > diff)
        {
            closestDiff = diff;
            bestHit = gdc_it;
        }
        it.next();
    }

    qDebug()<<"Point not found: "<< QString::number(point->x());
    return bestHit;
}

// doesn't scale
void MyPlot::plotVerticalScrollBarValueChange(int newValue)
{
    this->yAxis->setRange(QCPRange(newValue, newValue + vRange));
    if(!this->point_lines.first.isNull())
    {
        this->removeItem(this->point_lines.first);
        this->removeItem(this->point_lines.second);
    }
}

// returns oy min-max range in area
unique_rng_ptr MyPlot::findLocalOYRange(const QCPRange& oxrange)
{
    QCPGraphDataContainer::const_iterator it;
    unique_rng_ptr oyrange = nullptr, res = nullptr; // =.=

    for(auto& pmapk : graphs.keys())
    {

        // graph entry check
        // based on fact that all are oredered
        // | - x border, / - data container range
        if(graphs[pmapk]->begin()->key < oxrange.lower && (graphs[pmapk]->end() - 1)->key > oxrange.lower && (graphs[pmapk]->end() - 1)->key < oxrange.upper)
        {
            // /---|--/---|
            // decide which border is closer
            // if graph inside is smaller
            if(oxrange.lower - graphs[pmapk]->begin()->key >= (graphs[pmapk]->end()-1)->key - oxrange.lower)
            {
                it = graphs[pmapk]->constEnd() - 1;
                while(true)
                {
                    if(it->key < oxrange.lower)
                    {
                        break;
                    }
                    it--;
                }
                it++;
                oyrange = actuallyFindOYRange(it, graphs[pmapk]->constEnd());
            }
            //graph outside is smaller
            else
            {
                it = graphs[pmapk]->constBegin();
                while(true)
                {
                    if(it->key < oxrange.lower)
                    {
                        break;
                    }
                    it++;
                }
                oyrange = actuallyFindOYRange(it,graphs[pmapk]->constEnd());
            }

        }
        else if(graphs[pmapk]->begin()->key > oxrange.lower && (graphs[pmapk]->end() - 1)->key > oxrange.upper && graphs[pmapk]->begin()->key < oxrange.upper)
        {
            // |---/--|---/
            // decide where border is closer
            // if graph outside is smaller
            if(oxrange.upper - graphs[pmapk]->begin()->key >= (graphs[pmapk]->end()-1)->key - oxrange.upper)
            {
                // |--/
                it = graphs[pmapk]->constEnd() - 1;
                while(true)
                {
                    if(it->key < oxrange.upper)
                    {
                        break;
                    }
                    it--;
                }
                it++;
                oyrange = actuallyFindOYRange(graphs[pmapk]->constBegin(), it);

            }
            //graph inside is smaller
            else
            {
                // /--|
                it = graphs[pmapk]->constBegin();
                while(true)
                {
                    if(it->key < oxrange.lower)
                    {
                        break;
                    }
                    it--;
                }
                it++;
                oyrange = actuallyFindOYRange(it, graphs[pmapk]->constBegin());
            }
        }
        else if(graphs[pmapk]->begin()->key < oxrange.lower && (graphs[pmapk]->end() - 1)->key > oxrange.upper)
        {
            // /--|-----|--/
            uint32_t offset = graphs[pmapk]->size()/2;
            QCPGraphDataContainer::const_iterator it_end;
            it = graphs[pmapk]->constBegin()+offset;

            // searching first hit of key inside oxrange
            while((it->key < oxrange.lower || it->key > oxrange.upper) && offset)
            {
                offset /= 2;
                if(it->key < oxrange.lower) it -= offset;
                else it += offset;
            }
            // if it's value is still below range, going up until edge or collision
            if(it->key < oxrange.lower)
            {
                while(it->key < oxrange.lower && it != graphs[pmapk]->constEnd()) it++;
            }
            // if it value is bigger, than range upper, going down until edge
            else if(it->key > oxrange.upper)
            {
                while(it->key > oxrange.upper && it != graphs[pmapk]->constBegin()) it--;
                it++;
            }
            // still couldn't find
            if(it->key < oxrange.lower || it->key > oxrange.upper)
            {
                it_end = it;
            }
            // found (it(var) is in range)
            // it is now somewhere inside range
            else
            {
                it_end = it;
                // finding beginning
                while(it != graphs[pmapk]->constBegin() && it->key >= oxrange.lower) it--;
                // now it is at the first point of the graph inside the range
                it++;

                while(it_end != graphs[pmapk]->constEnd() && it_end->key <= oxrange.upper) it_end++;
            }

            oyrange = actuallyFindOYRange(it,it_end);
        }
        else if(graphs[pmapk]->begin()->key > oxrange.lower && (graphs[pmapk]->end() - 1)->key < oxrange.upper)
        {
            // |--/----/--|
            oyrange = actuallyFindOYRange(graphs[pmapk]->constBegin(), graphs[pmapk]->constEnd());
        }
        if(oyrange != nullptr)
        {
            if(res == nullptr)
            {
                res = std::move(oyrange);
            }
            else
            {
                if(oyrange->lower < res->lower)
                {
                    res->lower = oyrange->lower;
                }
                if(oyrange->upper > res->upper)
                {
                    res->upper = oyrange->upper;
                }
            }

        }
    }
    return res;
}

//finds local oy range
unique_rng_ptr MyPlot::actuallyFindOYRange(QCPGraphDataContainer::const_iterator begin, QCPGraphDataContainer::const_iterator end)
{
    unique_rng_ptr oyrange(new QCPRange());
    if(begin != end)
    {
        ///....
        int c = 0;
        if(begin->value < 1 || (end-1)->value < 1)
        {
            c = 2+3;
        }
        if(c == 5) qDebug()<<c;
        ///
        oyrange->lower = std::min_element(begin, end, [](QCPGraphData a, QCPGraphData b)
            { return a.value < b.value; } )->value;

        oyrange->upper = std::max_element(begin, end, [](QCPGraphData a, QCPGraphData b)
            { return a.value < b.value; } )->value;
    }
    else // begin == end
    {
        oyrange->lower = begin->value;
        oyrange->upper = begin->value;
    }
    return oyrange;
}

void MyPlot::resizeOY(const QCPRange& oxrange)
{
    unique_rng_ptr oyrange = findLocalOYRange(oxrange);

    if(oyrange != nullptr)
    {
        // if values REALLY greatly vary - going smoothly
        // on next iterations they probably going to change if
        // plot is updating dynamically
        // If plot is being manually scrolled - thats probably not the way
        if(oyrange->lower <= yAxis->range().lower/10)
        {
            oyrange->lower = yAxis->range().lower/9.9;
        }
        else if(oyrange->lower >= yAxis->range().lower*10)
        {
            oyrange->lower = oyrange->lower/1.33;
        }
        if(oyrange->upper >= yAxis->range().upper*9.9)
        {
            oyrange->upper = oyrange->upper/1.33;
        }
        else if(oyrange->upper <= yAxis->range().upper/10)
        {
            oyrange->upper = yAxis->range().upper/9.9;
        }
        //qDebug() <<"currently found:" << *oyrange << "previous:" << yAxis->range();
        //possible new vertical range
        uint32_t temp = oyrange->upper - oyrange->lower + (oyrange->upper/14) + (oyrange->lower/14);

        if(abs((int)vRange - (int)temp) > (int)vRange/10)
        {
            vRange = temp;
            yAxis->setRange(oyrange->lower - oyrange->lower/14, oyrange->upper + oyrange->upper/14);
        }
    }
}


QCPGraphDataContainer::const_iterator MyPlot::findApproximation(QPointF *point, const QCPGraphDataContainer &container, double& closestDiff)
{
    int offset = container.size()/2;
    closestDiff = std::numeric_limits<double>::max();
    double diff = 0;
    QCPGraphDataContainer::const_iterator it = container.constEnd() - offset, closest = nullptr;

    while(offset)
    {
        // check if horizontally approximate
        if(it->key <= point->x() + approximationAreaH && it->key >= point->x() - approximationAreaH)
        {
            // goto first possible iterator in area
            while(it != container.constBegin() && (it - 1)->key >= point->x() - approximationAreaH)
            {
                it--;
            }

            // loop through all possible horizontally approximate keys and look for
            // vertical approximation
            for(;(it+1)->key <= point->x() + approximationAreaH;it++)
            {
                // calculating 2d direct distance       cannot use this beacause ox and oy are diproportionate
                diff = fabs(it->value - point->y()); // sqrt(pow(point->x() - it->key, 2) + pow(point->y() - , 2));

                if(diff < approximationAreaV && diff < closestDiff && diff < approximationAreaV)
                {
                    closestDiff = diff;
                    closest = it;
                }
            }
            offset = 0;
        }
        else
        {
            offset /= 2;

            if(it->key > point->x())
            {
                it -= offset;
            }
            else
            {
                it += offset;
            }
        }
    }
    if(closest == nullptr)
    {
        return nullptr;
    }
    return closest;
}

// use point to add point
void MyPlot::addPoint(std::unique_ptr<QPair<uint16_t, QPointF*>> &pData)
{
    if(pData->first == 0xdead)
    {
        return;
    }
    if(pData->second->x() > XRange.upper)
    {
        XRange.upper = pData->second->x();
    }
    if(pData->second->x() < XRange.lower)
    {
        XRange.lower = pData->second->x();
    }
    if(!graphs.keys().contains(pData->first))
    {
        graphs[pData->first] = QSharedPointer<QCPGraphDataContainer>(new QCPGraphDataContainer());
        graphs[pData->first]->add(QCPGraphData(pData->second->x(), pData->second->y()));

        addGraph();
        if((redInd+totalGraphs)%colorList.size() == redInd-1)
        {
            throw std::invalid_argument("out of colors");
        }
        else
        {
            graph(totalGraphs)->setPen(QPen(colorList.at((redInd+totalGraphs)%colorList.size())));
            graph(totalGraphs)->setData(graphs[pData->first]);
        }
        totalGraphs++;
    }
    else
    {
        graphs[pData->first]->add(QCPGraphData(pData->second->x(), pData->second->y()));
    }
}

void MyPlot::plotPress(QMouseEvent* e)
{
    if(point_lines.first.isNull()){
        QPointF *point = new QPointF(e->x(), e->y());

        if(point->x() > xAxis->range().lower || point->x() < xAxis->range().upper)
        {
            //removeItem(maxYLine);
            flushNonGraph();
            displayPointLines(findGraphElement(point), QPen(Qt::red), xAxis->range(), true);
        }
        delete point;
        freeze = true;
    }
    else
    {
        flushNonGraph();
        freeze = false;
    }

    if(!pressLocked) replot();

}

// assuming data is unsigned
// 1 - possible ox scrollbar value
// 2 - possible ox scrollbar page step
// 3 - total ox sb width
bool MyPlot::mreplot(int32_t& psbvalue, int32_t& psbpage_step, int32_t& psbwidth)
{
    if(!freeze) psbvalue = -1;
    psbpage_step = -1;
    psbwidth = -1;
    // assuming data is ordered
    double minx = std::numeric_limits<double>::max(), maxx = 00;
    bool to_resize_oy = true;
    for(auto& key : graphs.keys())
    {
        if(graphs[key]->begin()->key < minx)
            minx = graphs[key]->begin()->key;

        if((graphs[key]->end()-1)->key > maxx)
            maxx = (graphs[key]->end()-1)->key;
    }
    if(maxx <= minx)
    {
        if(maxx != minx) std::swap(minx, maxx);
        psbvalue = 0;
    }

    //setting ox step
    psbpage_step = plottingRange/hRange;
    // setting scrollbar width
    psbwidth = maxx - minx;
    psbwidth = static_cast<uint32_t>(psbwidth) > plottingRange ? plottingRange : psbwidth;

    if(!freeze)
    {
        // axelerate ox
        // if data fits into single range align center
        if(fabs(maxx - minx) < hRange)
        {
            double avg = (maxx + minx)/2;
            xAxis->setRange(avg - hRange/2, avg + hRange/2);
        }
        else
        {
            if(maxx > static_cast<int>(hRange))
            {
                xAxis->setRange(maxx - hRange, maxx);
            }
            else
            {
                xAxis->setRange(maxx - hRange/2, maxx + hRange/2);
            }
        }
        psbvalue = psbwidth;
    }
    else // totally broken
    {
        // if cursor position is at lowest position move range so
        // that scrollbar(and position) stays the same, but plot is moved
        // define if scrollbar at lowest position
        double plotStart = maxx - (XRange.size() > plottingRange ? plottingRange : XRange.size());
        double delta = maxx - xAxis->range().upper;

        if(xAxis->range().lower <= plotStart)
        {
            xAxis->setRange(plotStart + delta, plotStart + hRange + delta);
            psbvalue = 0;
        }
        // at right edge - unfreeze
        //else if()
        // scrollbar at any other non-edge position
        else
        {
            // do not change xaxis, just scrtollbar values
            //psbvalue = maxx - xAxis->range().lower;
            to_resize_oy = false;
        }
    }
    // axelerate oy
    if(to_resize_oy)
    {
        resizeOY(xAxis->range());
        if(!pressLocked) replot();
    }
    return !freeze;
}

void MyPlot::plotHorizontalScrollBarValueChange(int newValue)
{
    freeze = true;

    if(!this->point_lines.first.isNull())
    {
        this->removeItem(this->point_lines.first);
        this->removeItem(this->point_lines.second);
    }
    // new value -> x coordinate
    double lowerEdge = XRange.size() < plottingRange ? XRange.lower : XRange.upper - plottingRange;
    this->xAxis->setRange(QCPRange(lowerEdge + newValue, lowerEdge + newValue + hRange));
    // replotting & rescaling OY
    resizeOY(xAxis->range());
    replot();
    qDebug() << newValue<<": Xrange:"<<xAxis->range()<<": Yrange:"<<yAxis->range();
}

void MyPlot::rangeChanged(int newRange)
{
    hRange = newRange;
}

void MyPlot::areaChanged(int newRange)
{
    plottingRange = newRange;
}

void MyPlot::pressLock()
{
    pressLocked = !pressLocked;
}
