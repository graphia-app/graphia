#include "graphsizeestimateplotitem.h"

#include "shared/utils/utils.h"
#include "shared/utils/string.h"

#include <QVariantMap>
#include <QVector>

GraphSizeEstimatePlotItem::GraphSizeEstimatePlotItem(QQuickItem* parent) :
    QQuickPaintedItem(parent)
{
    setRenderTarget(RenderTarget::FramebufferObject);

    _customPlot.setOpenGl(true);

    setAcceptedMouseButtons(Qt::AllButtons);
    setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &QQuickPaintedItem::widthChanged, this, &GraphSizeEstimatePlotItem::updatePlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &GraphSizeEstimatePlotItem::updatePlotSize);
    connect(&_customPlot, &QCustomPlot::afterReplot, this, &GraphSizeEstimatePlotItem::onReplot);
}

void GraphSizeEstimatePlotItem::paint(QPainter* painter)
{
    painter->drawPixmap(0, 0, _customPlot.toPixmap());
}

double GraphSizeEstimatePlotItem::threshold() const
{
    return _threshold;
}

void GraphSizeEstimatePlotItem::setThreshold(double threshold)
{
    if(threshold != _threshold)
    {
        if(!_keys.isEmpty())
            threshold = u::clamp(qAsConst(_keys).first(), 1.0, threshold);

        _threshold = threshold;
        emit thresholdChanged();

        updateThresholdIndicator();
        _customPlot.replot(QCustomPlot::rpQueuedReplot);
    }
}

void GraphSizeEstimatePlotItem::setGraphSizeEstimate(const QVariantMap& graphSizeEstimate)
{
    if(!graphSizeEstimate.contains(QStringLiteral("keys")))
        return;

    if(!graphSizeEstimate.contains(QStringLiteral("numNodes")))
        return;

    if(!graphSizeEstimate.contains(QStringLiteral("numEdges")))
        return;

    _keys = graphSizeEstimate.value(QStringLiteral("keys")).value<QVector<double>>();
    _numNodes = graphSizeEstimate.value(QStringLiteral("numNodes")).value<QVector<double>>();
    _numEdges = graphSizeEstimate.value(QStringLiteral("numEdges")).value<QVector<double>>();

    buildPlot();
}

void GraphSizeEstimatePlotItem::updateThresholdIndicator()
{
    if(_thresholdIndicator == nullptr || _keys.isEmpty())
        return;

    _thresholdIndicator->point1->setCoords(_threshold, 0.0);
    _thresholdIndicator->point2->setCoords(_threshold, 1.0);

    int index = 0;
    while(_keys.at(index) < _threshold && index < _keys.size() - 1)
        index++;

    index++;

    int numNodes = 0;
    int numEdges = 0;

    if(index < _keys.size())
    {
        numNodes = _numNodes.at(index);
        numEdges = _numEdges.at(index);
    }

    _customPlot.xAxis->setLabel(tr("Estimated Graph Size: %1 Nodes, %2 Edges")
        .arg(u::formatNumberSIPostfix(numNodes))
        .arg(u::formatNumberSIPostfix(numEdges)));
}

void GraphSizeEstimatePlotItem::buildPlot()
{
    if(_keys.isEmpty())
        return;

    _customPlot.clearItems();
    _customPlot.clearPlottables();

    auto nodesGraph = _customPlot.addGraph();
    auto edgesGraph = _customPlot.addGraph();

    nodesGraph->setData(_keys, _numNodes, true);
    nodesGraph->setPen(QPen(Qt::red));
    nodesGraph->setName(tr("Nodes"));
    edgesGraph->setData(_keys, _numEdges, true);
    edgesGraph->setPen(QPen(Qt::blue));
    edgesGraph->setName(tr("Edges"));

    _thresholdIndicator = new QCPItemStraightLine(&_customPlot);

    QPen indicatorPen;
    indicatorPen.setStyle(Qt::DashLine);
    _thresholdIndicator->setPen(indicatorPen);

    updateThresholdIndicator();

    _customPlot.yAxis->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    logTicker->setLogBase(10);
    logTicker->setSubTickCount(3);
    _customPlot.yAxis->setTicker(logTicker);
    _customPlot.yAxis->setNumberFormat(QStringLiteral("eb"));
    _customPlot.yAxis->setNumberPrecision(0);
    _customPlot.yAxis->grid()->setSubGridVisible(true);

    _customPlot.xAxis->setRange(qAsConst(_keys).first(), 1.0);
    _customPlot.yAxis->rescale();

    _customPlot.legend->setVisible(true);
    _customPlot.legend->setBrush(QBrush(QColor(255, 255, 255, 127)));
    _customPlot.axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignBottom);

    _customPlot.replot(QCustomPlot::rpQueuedReplot);
}

void GraphSizeEstimatePlotItem::updatePlotSize()
{
    _customPlot.setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));

    // Since QCustomPlot is a QWidget, it is never technically visible, so never generates
    // a resizeEvent, so its viewport never gets set, so we must do so manually
    _customPlot.setViewport(_customPlot.geometry());
}

void GraphSizeEstimatePlotItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        _dragging = true;
        auto xValue = _customPlot.xAxis->pixelToCoord(event->pos().x());
        setThreshold(xValue);
    }
}

void GraphSizeEstimatePlotItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
        _dragging = false;
}

void GraphSizeEstimatePlotItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(_dragging)
    {
        auto xValue = _customPlot.xAxis->pixelToCoord(event->pos().x());
        setThreshold(xValue);
    }
}

void GraphSizeEstimatePlotItem::routeMouseEvent(QMouseEvent* event)
{
    auto* newEvent = new QMouseEvent(event->type(), event->localPos(),
                                     event->button(), event->buttons(),
                                     event->modifiers());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void GraphSizeEstimatePlotItem::onReplot()
{
    update();
}
