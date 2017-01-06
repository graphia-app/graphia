#include "correlationplotitem.h"

#include <QDesktopServices>

#include <random>

CorrelationPlotItem::CorrelationPlotItem(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    _customPlot.setOpenGl(true);
    _customPlot.addLayer("textLayer");

    _textLayer = _customPlot.layer("textLayer");
    _textLayer->setMode(QCPLayer::LayerMode::lmBuffered);

    QFont defaultFont10Pt;
    defaultFont10Pt.setPointSize(10);

    _defaultFont9Pt.setPointSize(9);

    _hoverLabel = new QCPItemText(&_customPlot);
    _hoverLabel->setLayer(_textLayer);
    _hoverLabel->setPositionAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    _hoverLabel->setFont(defaultFont10Pt);
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
    _plotModeTextElement->setFont(_defaultFont9Pt);
    _plotModeTextElement->setTextColor(Qt::black);
    _plotModeTextElement->setVisible(false);

    setFlag(QQuickItem::ItemHasContents, true);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &QQuickPaintedItem::widthChanged, this, &CorrelationPlotItem::updateCustomPlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &CorrelationPlotItem::updateCustomPlotSize);
    connect(&_customPlot, &QCustomPlot::afterReplot, this, &CorrelationPlotItem::onCustomReplot);
}

void CorrelationPlotItem::refresh()
{
    updateCustomPlotSize();
    buildGraphs();
    _customPlot.replot();
}

void CorrelationPlotItem::paint(QPainter* painter)
{
    QPixmap    picture(boundingRect().size().toSize());
    QCPPainter qcpPainter(&picture);

    _customPlot.toPainter(&qcpPainter);

    painter->drawPixmap(QPoint(), picture);
}

void CorrelationPlotItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvents(event);
}

void CorrelationPlotItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvents(event);
    if (event->button() == Qt::RightButton)
        emit rightClick();
}

void CorrelationPlotItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvents(event);
}

void CorrelationPlotItem::hoverMoveEvent(QHoverEvent* event)
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

void CorrelationPlotItem::hoverLeaveEvent(QHoverEvent*)
{
    hideTooltip();
}

void CorrelationPlotItem::mouseDoubleClickEvent(QMouseEvent* event)
{
    routeMouseEvents(event);
}

void CorrelationPlotItem::buildGraphs()
{
    _plotModeTextElement->setVisible(false);
    // If the legend is not cleared first this will cause a slowdown
    // when removing a large number of graphs
    _customPlot.legend->clear();
    _customPlot.clearGraphs();

    if(_customPlot.plotLayout()->rowCount() > 1)
    {
        _customPlot.plotLayout()->remove(_plotModeTextElement);
        _customPlot.plotLayout()->simplify();
    }

    if(_selectedRows.length() > MAX_SELECTED_ROWS_BEFORE_MEAN)
        populateMeanAverageGraphs();
    else
        populateRawGraphs();

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);
    _customPlot.xAxis->setTicker(categoryTicker);
    _customPlot.xAxis->setTickLabelRotation(90);

    // Populate Categories
    for(int i = 0; i < _labelNames.count(); ++i)
        categoryTicker->addTick(i, _labelNames[i]);
}

void CorrelationPlotItem::populateMeanAverageGraphs()
{
    double maxX = _columnCount;
    double maxY = 0.0;

    std::random_device randomDevice;
    std::mt19937 mTwister(randomDevice());
    std::uniform_int_distribution<> randomColorDist(0, 255);

    auto* graph = _customPlot.addGraph();
    mTwister.seed(_selectedRows.count() * 1000);
    QColor randomColor = QColor::fromHsl(randomColorDist(mTwister), 210, 130);
    graph->setPen(QPen(randomColor));
    graph->setName(tr("Mean average of selection"));

    // Use Average Calculation
    QVector<double> yDataAvg;
    QVector<double> xData;

    for(int col = 0; col < _columnCount; col++)
    {
        double runningTotal = 0.0;
        for(auto row : _selectedRows)
        {
            int index = (row * _columnCount) + col;
            runningTotal += _data[index];
        }
        xData.append(col);
        yDataAvg.append(runningTotal / _selectedRows.length());

        maxY = qMax(maxY, yDataAvg.back());
    }
    graph->setData(xData, yDataAvg, true);

    _customPlot.plotLayout()->insertRow(1);
    _customPlot.plotLayout()->addElement(1, 0, _plotModeTextElement);
    _plotModeTextElement->setText(
        QString(tr("*Mean average plot of %1 rows (maximum row count for individual plots is %2)"))
                .arg(_selectedRows.length())
                .arg(MAX_SELECTED_ROWS_BEFORE_MEAN));
    _plotModeTextElement->setVisible(true);

    _customPlot.xAxis->setRange(0, maxX);
    _customPlot.yAxis->setRange(0, maxY);
}

void CorrelationPlotItem::populateRawGraphs()
{
    double maxX = _columnCount;
    double maxY = 0.0;

    std::random_device randomDevice;
    std::mt19937 mTwister(randomDevice());
    std::uniform_int_distribution<> randomColorDist(0, 255);

    // Plot each graph individually
    for(auto row : _selectedRows)
    {
        auto* graph = _customPlot.addGraph();
        mTwister.seed(row * 1000);
        QColor randomColor = QColor::fromHsl(randomColorDist(mTwister), 210, 130);
        graph->setPen(QPen(randomColor));
        graph->setName(_graphNames[row]);

        QVector<double> yData;
        QVector<double> xData;

        for(int col = 0; col < _columnCount; col++)
        {
            int index = (row * _columnCount) + col;
            xData.append(col);
            yData.append(_data[index]);

            maxY = qMax(maxY, _data[index]);
        }
        graph->setData(xData, yData, true);
    }

    _customPlot.xAxis->setRange(0.0, maxX);
    _customPlot.yAxis->setRange(0.0, maxY);
}

void CorrelationPlotItem::setLabelNames(const QStringList &labelNames)
{
    QFontMetrics metrics(_defaultFont9Pt);
    _labelNames.clear();

    for(auto name : labelNames)
        _labelNames.append(metrics.elidedText(name, Qt::ElideRight, _elideLabelSizePixels));
}

void CorrelationPlotItem::routeMouseEvents(QMouseEvent* event)
{
    QMouseEvent* newEvent = new QMouseEvent(event->type(), event->localPos(), event->button(), event->buttons(), event->modifiers());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void CorrelationPlotItem::updateCustomPlotSize()
{
    _customPlot.setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));
}

void CorrelationPlotItem::showTooltip()
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
    _hoverLabel->setText(QString("%1 %2")
                         .arg(_hoverPlottable->name())
                         .arg(_itemTracer->position->value()));

    _hoverColorRect->setVisible(true);
    _hoverColorRect->setBrush(QBrush(_hoverPlottable->pen().color()));
    _hoverColorRect->topLeft->setPixelPosition(QPointF(_hoverLabel->topLeft->pixelPosition().x() - 10.0,
                                                   _hoverLabel->topLeft->pixelPosition().y()));

    _textLayer->replot();

    update();
}

void CorrelationPlotItem::hideTooltip()
{
    _hoverLabel->setVisible(false);
    _hoverColorRect->setVisible(false);
    _itemTracer->setVisible(false);
    _textLayer->replot();
    update();
}

void CorrelationPlotItem::savePlotImage(const QUrl& url, const QString& format)
{
    if(format.contains("png"))
        _customPlot.savePng(url.toLocalFile());
    else if(format.contains("pdf"))
        _customPlot.savePdf(url.toLocalFile());
    else if(format.contains("jpg"))
        _customPlot.saveJpg(url.toLocalFile());

    QDesktopServices::openUrl(url);
}

void CorrelationPlotItem::onCustomReplot()
{
    update();
}
