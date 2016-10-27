#include "customplotitem.h"

#include <random>

#include <QDebug>

CustomPlotItem::CustomPlotItem(QQuickItem* parent) : QQuickPaintedItem(parent),
    _customPlot()
{
    _customPlot.setOpenGl(true);
    _customPlot.addLayer("textLayer");

    _textLayer = _customPlot.layer("textLayer");
    _textLayer->setMode(QCPLayer::LayerMode::lmBuffered);

    _textLabel = new QCPItemText(&_customPlot);
    _textLabel->setLayer(_textLayer);
    _textLabel->setPositionAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    _textLabel->setFont(QFont("Arial", 10));
    _textLabel->setPen(QPen(Qt::black));
    _textLabel->setBrush(QBrush(Qt::white));
    _textLabel->setPadding(QMargins(3, 3, 3, 3));
    _textLabel->setClipToAxisRect(false);
    _textLabel->setVisible(false);

    _labelColor = new QCPItemRect(&_customPlot);
    _labelColor->setLayer(_textLayer);
    _labelColor->bottomRight->setParentAnchor(_textLabel->bottomLeft);
    _labelColor->setClipToAxisRect(false);
    _labelColor->setVisible(false);

    _itemTracer = new QCPItemTracer(&_customPlot);
    _itemTracer->setLayer(_textLayer);
    _itemTracer->setInterpolating(false);
    _itemTracer->setVisible(true);
    _itemTracer->setStyle(QCPItemTracer::TracerStyle::tsCircle);

    setFlag(QQuickItem::ItemHasContents, true);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &QQuickPaintedItem::widthChanged, this, &CustomPlotItem::updateCustomPlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &CustomPlotItem::updateCustomPlotSize);
    connect(&_customPlot, &QCustomPlot::afterReplot, this, &CustomPlotItem::onCustomReplot);
}

void CustomPlotItem::initCustomPlot()
{
    updateCustomPlotSize();
    buildGraphs();
    _customPlot.replot();
}


void CustomPlotItem::paint(QPainter* painter)
{
    QPixmap    picture(boundingRect().size().toSize());
    QCPPainter qcpPainter(&picture);

    _customPlot.toPainter(&qcpPainter);

    painter->drawPixmap(QPoint(), picture);
}

void CustomPlotItem::mousePressEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    routeMouseEvents(event);
}

void CustomPlotItem::mouseReleaseEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    routeMouseEvents(event);
}

void CustomPlotItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvents(event);
}

void CustomPlotItem::hoverMoveEvent(QHoverEvent *event)
{
    _hoverPoint = event->posF();

    auto* currentPlottable = _customPlot.plottableAt(event->posF());
    if(_hoverPlottable != currentPlottable)
    {
        _hoverPlottable = currentPlottable;
        hideTooltip();
    }

    if(_hoverPlottable != nullptr)
        showTooltip();
}

void CustomPlotItem::hoverLeaveEvent(QHoverEvent *event)
{
    Q_UNUSED(event);
    hideTooltip();
}

void CustomPlotItem::mouseDoubleClickEvent(QMouseEvent* event)
{
    qDebug() << Q_FUNC_INFO;
    routeMouseEvents(event);
}

void CustomPlotItem::buildGraphs()
{
    // If the legend is not cleared first this will cause a slowdown
    // when removing a large number of graphs
    _customPlot.legend->clear();
    _customPlot.clearGraphs();

    double maxX = _colCount;
    double maxY = 0;

    std::random_device randomDevice;
    std::mt19937 mTwister(randomDevice());
    std::uniform_int_distribution<> randomColorDist(0, 255);

    for(auto row : _selectedRows)
    {
        auto* graph = _customPlot.addGraph();
        mTwister.seed(row*1000);
        QColor randomColor = QColor::fromHsl(randomColorDist(mTwister), 210, 130);
        graph->setPen(QPen(randomColor));
        graph->setName(_graphNames[row]);

        QVector<double> yData;
        QVector<double> xData;

        for(int col=0; col<_colCount; col++)
        {
            int index = (row * _colCount) + col;
            xData.append(col);
            yData.append(_data[index]);

            maxY = qMax(maxY, _data[index]);
        }
        graph->setData(xData, yData, true);
    }

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);
    _customPlot.xAxis->setTicker(categoryTicker);
    _customPlot.xAxis->setTickLabelRotation(90);

    // Populate Categories
    for(int i = 0; i < _labelNames.count(); ++i)
        categoryTicker->addTick(i, _labelNames[i]);

    _customPlot.xAxis->setRange(0, maxX);
    _customPlot.yAxis->setRange(0, maxY);
}

void CustomPlotItem::setLabelNames(const QStringList &labelNames)
{
    QFont arialFont("Arial", 9);
    QFontMetrics arialMetrics(arialFont);
    _labelNames.clear();

    for(auto name : labelNames)
        _labelNames.append(arialMetrics.elidedText(name, Qt::ElideRight, _elideLabelSizePixels));
}

void CustomPlotItem::graphClicked(QCPAbstractPlottable* plottable)
{
    qDebug() << Q_FUNC_INFO << QString("Clicked on graph '%1 ").arg(plottable->name());
}

void CustomPlotItem::routeMouseEvents(QMouseEvent* event)
{
    QMouseEvent* newEvent = new QMouseEvent(event->type(), event->localPos(), event->button(), event->buttons(), event->modifiers());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void CustomPlotItem::updateCustomPlotSize()
{
    _customPlot.setGeometry(0, 0, width(), height());
}

void CustomPlotItem::showTooltip()
{

    QCPGraph* graph = dynamic_cast<QCPGraph*>(_hoverPlottable);

    _itemTracer->setGraph(graph);
    _itemTracer->setVisible(true);
    _itemTracer->setInterpolating(false);
    _itemTracer->setGraphKey(_customPlot.xAxis->pixelToCoord(_hoverPoint.x()));

    _textLabel->setVisible(true);
    _textLabel->position->setCoords(
                _customPlot.xAxis->pixelToCoord(_hoverPoint.x()),
                _customPlot.yAxis->pixelToCoord(_hoverPoint.y()));
    _textLabel->setText(_hoverPlottable->name() +
                        " " + QString::number(_itemTracer->position->value()));

    _labelColor->setVisible(true);
    _labelColor->setBrush(QBrush(_hoverPlottable->pen().color()));
    _labelColor->topLeft->setPixelPosition(QPointF(_textLabel->topLeft->pixelPosition().x() - 10,
                                                   _textLabel->topLeft->pixelPosition().y()));

    _textLayer->replot();

    update();
}

void CustomPlotItem::hideTooltip()
{
    _textLabel->setVisible(false);
    _labelColor->setVisible(false);
    _itemTracer->setVisible(false);
    _textLayer->replot();
    update();
}

void CustomPlotItem::onCustomReplot()
{
    update();
}
