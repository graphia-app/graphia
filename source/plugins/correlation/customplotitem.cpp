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

    _hoverLabel = new QCPItemText(&_customPlot);
    _hoverLabel->setLayer(_textLayer);
    _hoverLabel->setPositionAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    _hoverLabel->setFont(QFont("Arial", 10));
    _hoverLabel->setPen(QPen(Qt::black));
    _hoverLabel->setBrush(QBrush(Qt::white));
    _hoverLabel->setPadding(QMargins(3, 3, 3, 3));
    _hoverLabel->setClipToAxisRect(false);
    _hoverLabel->setVisible(false);

    _hoverColorRect = new QCPItemRect(&_customPlot);
    _hoverColorRect->setLayer(_textLayer);
    _hoverColorRect->bottomRight->setParentAnchor(_hoverLabel->bottomLeft);
    _hoverColorRect->setClipToAxisRect(false);
    _hoverColorRect->setVisible(false);

    _itemTracer = new QCPItemTracer(&_customPlot);
    _itemTracer->setBrush(QBrush(Qt::white));
    _itemTracer->setLayer(_textLayer);
    _itemTracer->setInterpolating(false);
    _itemTracer->setVisible(true);
    _itemTracer->setStyle(QCPItemTracer::TracerStyle::tsCircle);
    _itemTracer->setClipToAxisRect(false);

    _plotModeTextElement = new QCPTextElement(&_customPlot);
    _plotModeTextElement->setLayer(_textLayer);
    _plotModeTextElement->setTextFlags(Qt::AlignLeft);
    _plotModeTextElement->setFont(QFont("Arial", 9));
    _plotModeTextElement->setTextColor(Qt::black);
    _plotModeTextElement->setVisible(false);

    _customPlot.plotLayout()->insertRow(0);
    _customPlot.plotLayout()->addElement(0,0, _plotModeTextElement);

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
    if (event->button() == Qt::RightButton)
        emit rightClick();
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
    _plotModeTextElement->setVisible(false);
    // If the legend is not cleared first this will cause a slowdown
    // when removing a large number of graphs
    _customPlot.legend->clear();
    _customPlot.clearGraphs();

    if(_selectedRows.length() > MAX_SELECTED_ROWS_BEFORE_MEAN)
        populateMeanAvgGraphs();
    else
        populateRawGraphs();

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);
    _customPlot.xAxis->setTicker(categoryTicker);
    _customPlot.xAxis->setTickLabelRotation(90);

    // Populate Categories
    for(int i = 0; i < _labelNames.count(); ++i)
        categoryTicker->addTick(i, _labelNames[i]);
}

void CustomPlotItem::populateMeanAvgGraphs()
{
    double maxX = _colCount;
    double maxY = 0;

    std::random_device randomDevice;
    std::mt19937 mTwister(randomDevice());
    std::uniform_int_distribution<> randomColorDist(0, 255);

    auto* graph = _customPlot.addGraph();
    mTwister.seed(_selectedRows.count()*1000);
    QColor randomColor = QColor::fromHsl(randomColorDist(mTwister), 210, 130);
    graph->setPen(QPen(randomColor));
    graph->setName("Mean Avg of Selection");

    // Use Average Calculation
    QVector<double> yDataAvg;
    QVector<double> xData;

    for(int col=0; col<_colCount; col++)
    {
        double runningTotal = 0.0f;
        for(auto row : _selectedRows)
        {
            int index = (row * _colCount) + col;
            runningTotal += _data[index];
        }
        xData.append(col);
        yDataAvg.append(runningTotal / _selectedRows.length());

        maxY = qMax(maxY, yDataAvg.back());
    }
    graph->setData(xData, yDataAvg, true);

    _plotModeTextElement->setText("*Mean Avg plot of " + QString::number(_selectedRows.length()) +
                            " nodes. Maximum node count for individual plots is " + QString::number(MAX_SELECTED_ROWS_BEFORE_MEAN));
    _plotModeTextElement->setVisible(true);

    _customPlot.xAxis->setRange(0, maxX);
    _customPlot.yAxis->setRange(0, maxY);
}

void CustomPlotItem::populateRawGraphs()
{
    double maxX = _colCount;
    double maxY = 0;

    std::random_device randomDevice;
    std::mt19937 mTwister(randomDevice());
    std::uniform_int_distribution<> randomColorDist(0, 255);

    // Plot each graph individually
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

    _hoverLabel->setVisible(true);
    _hoverLabel->position->setCoords(
                _customPlot.xAxis->pixelToCoord(_hoverPoint.x()),
                _customPlot.yAxis->pixelToCoord(_hoverPoint.y()));
    _hoverLabel->setText(_hoverPlottable->name() +
                        " " + QString::number(_itemTracer->position->value()));

    _hoverColorRect->setVisible(true);
    _hoverColorRect->setBrush(QBrush(_hoverPlottable->pen().color()));
    _hoverColorRect->topLeft->setPixelPosition(QPointF(_hoverLabel->topLeft->pixelPosition().x() - 10,
                                                   _hoverLabel->topLeft->pixelPosition().y()));

    _textLayer->replot();

    update();
}

void CustomPlotItem::hideTooltip()
{
    _hoverLabel->setVisible(false);
    _hoverColorRect->setVisible(false);
    _itemTracer->setVisible(false);
    _textLayer->replot();
    update();
}

void CustomPlotItem::saveGraphImage(QUrl path)
{
    _customPlot.savePng(path.toLocalFile());
}

void CustomPlotItem::onCustomReplot()
{
    update();
}
