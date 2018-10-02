#include "graphsizeestimateplotitem.h"

#include <QVariantMap>
#include <QVector>

GraphSizeEstimatePlotItem::GraphSizeEstimatePlotItem(QQuickItem* parent) :
    QQuickPaintedItem(parent)
{
    setRenderTarget(RenderTarget::FramebufferObject);

    _customPlot.setOpenGl(true);

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

void GraphSizeEstimatePlotItem::setThreshold(double threshold_)
{
    if(threshold_ != _threshold)
    {
        _threshold = threshold_;
        emit thresholdChanged();
        buildPlot();
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

    auto thresholdIndicator = new QCPItemStraightLine(&_customPlot);

    QPen indicatorPen;
    indicatorPen.setStyle(Qt::DashLine);
    indicatorPen.setWidth(2);
    thresholdIndicator->setPen(indicatorPen);

    thresholdIndicator->point1->setCoords(_threshold, 0.0);
    thresholdIndicator->point2->setCoords(_threshold, 1.0);

    _customPlot.yAxis->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    _customPlot.yAxis->setTicker(logTicker);
    _customPlot.yAxis->setNumberFormat(QStringLiteral("eb"));
    _customPlot.yAxis->setNumberPrecision(0);

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
}

void GraphSizeEstimatePlotItem::onReplot()
{
    update();
}
