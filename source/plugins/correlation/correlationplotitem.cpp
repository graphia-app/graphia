#include "correlationplotitem.h"

#include "shared/utils/scope_exit.h"

#include <QDesktopServices>

#include <cmath>

CorrelationPlotWorker::CorrelationPlotWorker(std::recursive_mutex& mutex,
    QCustomPlot& customPlot, QCPLayer& tooltipLayer) :
    _debug(qEnvironmentVariableIntValue("QCUSTOMPLOT_DEBUG") != 0),
    _mutex(&mutex), _busy(false),
    _customPlot(&customPlot), _tooltipLayer(&tooltipLayer)
{
    QSurfaceFormat proposedSurfaceFormat;
    proposedSurfaceFormat.setSamples(16);

    // Qt requires that this is created on the UI thread, so we do so here
    // Note QCustomPlot takes ownership of the memory, so it does not need
    // to be deleted
    _surface = new QOffscreenSurface;
    _surface->setFormat(proposedSurfaceFormat);
    _surface->create();

    if(_debug)
    {
        connect(_customPlot, &QCustomPlot::beforeReplot, [this] { _replotTimer.restart(); });
        connect(_customPlot, &QCustomPlot::afterReplot, [this] { qDebug() << "replot" << _replotTimer.elapsed() << "ms"; });
    }
}

bool CorrelationPlotWorker::busy() const
{
    return _busy;
}

void CorrelationPlotWorker::setWidth(int width)
{
    _width = width;
}

void CorrelationPlotWorker::setHeight(int height)
{
    _height = height;
}

void CorrelationPlotWorker::setXAxisRange(double min, double max)
{
    _xAxisMin = min;
    _xAxisMax = max;
}

void CorrelationPlotWorker::updatePixmap(CorrelationPlotUpdateType updateType)
{
    if(updateType > _updateType)
        _updateType = updateType;

    // Avoid queueing up multiple redundant updates
    if(_updateQueued)
        return;

    QMetaObject::invokeMethod(this, "renderPixmap", Qt::QueuedConnection);
    _updateQueued = true;
}

void CorrelationPlotWorker::renderPixmap()
{
    _updateQueued = false;

    std::unique_lock<std::recursive_mutex> lock(*_mutex);

    // Don't indicate business if we're just updating the tooltip
    if(_updateType != CorrelationPlotUpdateType::RenderAndTooltips)
    {
        _busy = true;
        emit busyChanged();
    }

    auto atExit = std::experimental::make_scope_exit([this]
    {
        if(_busy)
        {
            _busy = false;
            emit busyChanged();
        }
    });

    if(_width < 0 || _height < 0)
        return;

    // OpenGL must be enabled on the thread which does the updates,
    // so that the context is created with the correct affinity
    if(!_customPlot->openGl())
    {
        _customPlot->setOpenGl(true, _surface->format().samples(), _surface);

        // We don't need to use the surface any more, so give up access to it
        _surface = nullptr;
    }

    _customPlot->setGeometry(0, 0, _width, _height);
    _customPlot->xAxis->setRange(_xAxisMin, _xAxisMax);

    if(_updateType >= CorrelationPlotUpdateType::RenderAndTooltips)
        _tooltipLayer->replot();

    if(_updateType >= CorrelationPlotUpdateType::ReplotAndRenderAndTooltips)
        _customPlot->replot(QCustomPlot::rpImmediateRefresh);

    _updateType = CorrelationPlotUpdateType::None;

    QElapsedTimer _pixmapTimer;
    _pixmapTimer.start();
    QPixmap pixmap = _customPlot->toPixmap();

    if(_debug)
        qDebug() << "render" << _pixmapTimer.elapsed() << "ms";

    // Ensure lock is released before receivers of pixmapUpdated are notified
    lock.unlock();

    emit pixmapUpdated(pixmap);
}

CorrelationPlotItem::CorrelationPlotItem(QQuickItem* parent) :
    QQuickPaintedItem(parent),
    _debug(qEnvironmentVariableIntValue("QCUSTOMPLOT_DEBUG") != 0)
{
    setRenderTarget(RenderTarget::FramebufferObject);

    _customPlot.addLayer(QStringLiteral("tooltipLayer"));
    _customPlot.setAutoAddPlottableToLegend(false);

    _tooltipLayer = _customPlot.layer(QStringLiteral("tooltipLayer"));
    _tooltipLayer->setMode(QCPLayer::LayerMode::lmBuffered);

    QFont defaultFont10Pt;
    defaultFont10Pt.setPointSize(10);

    _defaultFont9Pt.setPointSize(9);

    _hoverLabel = new QCPItemText(&_customPlot);
    _hoverLabel->setLayer(_tooltipLayer);
    _hoverLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    _hoverLabel->setFont(defaultFont10Pt);
    _hoverLabel->setPen(QPen(Qt::black));
    _hoverLabel->setBrush(QBrush(Qt::white));
    _hoverLabel->setPadding(QMargins(3, 3, 3, 3));
    _hoverLabel->setClipToAxisRect(false);
    _hoverLabel->setVisible(false);

    _hoverColorRect = new QCPItemRect(&_customPlot);
    _hoverColorRect->setLayer(_tooltipLayer);
    _hoverColorRect->topLeft->setParentAnchor(_hoverLabel->topRight);
    _hoverColorRect->setClipToAxisRect(false);
    _hoverColorRect->setVisible(false);

    _itemTracer = new QCPItemTracer(&_customPlot);
    _itemTracer->setBrush(QBrush(Qt::white));
    _itemTracer->setLayer(_tooltipLayer);
    _itemTracer->setInterpolating(false);
    _itemTracer->setVisible(true);
    _itemTracer->setStyle(QCPItemTracer::TracerStyle::tsCircle);
    _itemTracer->setClipToAxisRect(false);

    setFlag(QQuickItem::ItemHasContents, true);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    qRegisterMetaType<CorrelationPlotUpdateType>("CorrelationPlotUpdateType");

    _worker = new CorrelationPlotWorker(_mutex, _customPlot, *_tooltipLayer);
    _worker->moveToThread(&_plotRenderThread);
    connect(&_plotRenderThread, &QThread::finished, _worker, &QObject::deleteLater);

    connect(this, &QQuickPaintedItem::widthChanged, [this] { QMetaObject::invokeMethod(_worker, "setWidth", Q_ARG(int, width())); });
    connect(this, &QQuickPaintedItem::heightChanged, [this] { QMetaObject::invokeMethod(_worker, "setHeight", Q_ARG(int, height())); });

    connect(_worker, &CorrelationPlotWorker::pixmapUpdated, this, &CorrelationPlotItem::onPixmapUpdated);
    connect(_worker, &CorrelationPlotWorker::busyChanged, this, &CorrelationPlotItem::busyChanged);

    connect(&_customPlot, &QCustomPlot::afterReplot, [this]
    {
        // The constructor of QCustomPlot does an initial replot scheduled in the UI event loop
        // We wait for it to complete before starting the render thread, otherwise we get into
        // the situation where the initial replot can potentially use the GL context created
        // on the render thread, which is obviously bad
        if(!_plotRenderThread.isRunning())
            _plotRenderThread.start();
    });

    connect(this, &QQuickPaintedItem::widthChanged, this, &CorrelationPlotItem::updatePlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &CorrelationPlotItem::updatePlotSize);
    connect(this, &QQuickPaintedItem::widthChanged, this, &CorrelationPlotItem::visibleHorizontalFractionChanged);
}

CorrelationPlotItem::~CorrelationPlotItem()
{
    _plotRenderThread.quit();
    _plotRenderThread.wait();
}

void CorrelationPlotItem::updatePixmap(CorrelationPlotUpdateType updateType)
{
    QMetaObject::invokeMethod(_worker, "updatePixmap", Q_ARG(CorrelationPlotUpdateType, updateType));
}

void CorrelationPlotItem::paint(QPainter* painter)
{
    if(_pixmap.isNull())
        return;

    painter->fillRect(0, 0, width(), height(), Qt::white);

    // Render the plot in the bottom left; that way when its container
    // is resized, it doesn't hop around vertically, as it would if
    // it had been rendered from the top left
    painter->drawPixmap(0, height() - _pixmap.height(), _pixmap);
}

void CorrelationPlotItem::onPixmapUpdated(const QPixmap& pixmap)
{
    if(!pixmap.isNull())
    {
        _pixmap = pixmap;
        update();
    }

    // A tooltip update was attempted during the render, so
    // complete it now, now that the render has finished
    if(_tooltipNeedsUpdate)
        updateTooltip();
}

void CorrelationPlotItem::routeMouseEvent(QMouseEvent* event)
{
    auto* newEvent = new QMouseEvent(event->type(), event->localPos(),
                                     event->button(), event->buttons(),
                                     event->modifiers());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void CorrelationPlotItem::routeWheelEvent(QWheelEvent* event)
{
    auto* newEvent = new QWheelEvent(event->pos(), event->delta(),
                                     event->buttons(), event->modifiers(),
                                     event->orientation());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void CorrelationPlotItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void CorrelationPlotItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::RightButton)
        emit rightClick();
}

void CorrelationPlotItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void CorrelationPlotItem::updateTooltip()
{
    std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);

    if(!lock.owns_lock())
    {
        _tooltipNeedsUpdate = true;
        return;
    }

    _tooltipNeedsUpdate = false;

    QCPAbstractPlottable* plottableUnderCursor = nullptr;

    if(_hoverPoint.x() >= 0.0 && _hoverPoint.y() >= 0.0)
        plottableUnderCursor = _customPlot.plottableAt(_hoverPoint, true);

    if(plottableUnderCursor != nullptr)
    {
        _itemTracer->setGraph(nullptr);
        if(auto graph = dynamic_cast<QCPGraph*>(plottableUnderCursor))
        {
            _itemTracer->setGraph(graph);
            _itemTracer->setGraphKey(_customPlot.xAxis->pixelToCoord(_hoverPoint.x()));
        }
        else if(auto bars = dynamic_cast<QCPBars*>(plottableUnderCursor))
        {
            auto xCoord = std::lround(_customPlot.xAxis->pixelToCoord(_hoverPoint.x()));
            _itemTracer->position->setPixelPosition(bars->dataPixelPosition(xCoord));
        }
        else if(auto boxPlot = dynamic_cast<QCPStatisticalBox*>(plottableUnderCursor))
        {
            // Only show simple tooltips for now, can extend this later...
            auto xCoord = std::lround(_customPlot.xAxis->pixelToCoord(_hoverPoint.x()));
            _itemTracer->position->setPixelPosition(boxPlot->dataPixelPosition(xCoord));
        }

        _itemTracer->setVisible(true);
        _itemTracer->setInterpolating(false);
        _itemTracer->updatePosition();
        auto itemTracerPosition = _itemTracer->anchor(QStringLiteral("position"))->pixelPosition();

        _hoverLabel->setVisible(true);
        _hoverLabel->setText(QStringLiteral("%1, %2: %3")
                         .arg(plottableUnderCursor->name(),
                              _labelNames[static_cast<int>(_itemTracer->position->key())])
                         .arg(_itemTracer->position->value()));

        const auto COLOR_RECT_WIDTH = 10.0;
        const auto HOVER_MARGIN = 10.0;
        auto hoverlabelWidth = _hoverLabel->right->pixelPosition().x() - _hoverLabel->left->pixelPosition().x();
        auto hoverlabelHeight = _hoverLabel->bottom->pixelPosition().y() - _hoverLabel->top->pixelPosition().y();
        auto hoverLabelRightX = itemTracerPosition.x() + hoverlabelWidth + HOVER_MARGIN + COLOR_RECT_WIDTH;
        auto xBounds = clipRect().width();
        QPointF targetPosition(itemTracerPosition.x() + HOVER_MARGIN, itemTracerPosition.y());

        // If it falls out of bounds, clip to bounds and move label above marker
        if(hoverLabelRightX > xBounds)
        {
            targetPosition.rx() = xBounds - hoverlabelWidth - COLOR_RECT_WIDTH - 1.0;

            // If moving the label above marker is less than 0, clip to 0 + labelHeight/2;
            if(targetPosition.y() - (hoverlabelHeight * 0.5) - HOVER_MARGIN * 2.0 < 0.0)
                targetPosition.setY(hoverlabelHeight * 0.5);
            else
                targetPosition.ry() -= HOVER_MARGIN * 2.0;
        }

        _hoverLabel->position->setPixelPosition(targetPosition);

        _hoverColorRect->setVisible(true);
        _hoverColorRect->setBrush(QBrush(plottableUnderCursor->pen().color()));
        _hoverColorRect->bottomRight->setPixelPosition(
            {_hoverLabel->bottomRight->pixelPosition().x() + COLOR_RECT_WIDTH,
            _hoverLabel->bottomRight->pixelPosition().y()});
    }
    else if(_itemTracer->visible())
    {
        _hoverLabel->setVisible(false);
        _hoverColorRect->setVisible(false);
        _itemTracer->setVisible(false);
    }
    else
    {
        // Nothing changed
        return;
    }

    updatePixmap(CorrelationPlotUpdateType::RenderAndTooltips);
}

void CorrelationPlotItem::hoverMoveEvent(QHoverEvent* event)
{
    if(event->posF() == _hoverPoint)
        return;

    _hoverPoint = event->posF();
    updateTooltip();
}

void CorrelationPlotItem::hoverLeaveEvent(QHoverEvent*)
{
    _hoverPoint = {-1.0, -1.0};
    updateTooltip();
}

void CorrelationPlotItem::mouseDoubleClickEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void CorrelationPlotItem::wheelEvent(QWheelEvent* event)
{
    routeWheelEvent(event);
}

QVector<double> CorrelationPlotItem::meanAverageData(double& min, double& max)
{
    min = 0.0;
    max = 0.0;

    // Use Average Calculation
    QVector<double> yDataAvg; yDataAvg.reserve(_selectedRows.size());

    for(size_t col = 0; col < _columnCount; col++)
    {
        double runningTotal = 0.0;
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            runningTotal += _data.at(static_cast<int>(index));
        }
        yDataAvg.append(runningTotal / _selectedRows.length());

        max = std::max(max, yDataAvg.back());
        min = std::min(min, yDataAvg.back());
    }
    return yDataAvg;
}

void CorrelationPlotItem::populateMeanLinePlot()
{
    double maxY = 0.0;
    double minY = 0.0;

    auto* graph = _customPlot.addGraph();
    graph->setPen(QPen(Qt::black));
    graph->setName(tr("Mean average of selection"));

    QVector<double> xData(static_cast<int>(_columnCount));
    // xData is just the column indices
    std::iota(std::begin(xData), std::end(xData), 0);

    // Use Average Calculation and set min / max
    QVector<double> yDataAvg = meanAverageData(minY, maxY);

    graph->setData(xData, yDataAvg, true);

    auto* plotModeTextElement = new QCPTextElement(&_customPlot);
    plotModeTextElement->setLayer(_tooltipLayer);
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Mean average plot of %1 rows"))
                .arg(_selectedRows.length()));
    plotModeTextElement->setVisible(true);

    _customPlot.plotLayout()->insertRow(1);
    _customPlot.plotLayout()->addElement(1, 0, plotModeTextElement);

    _customPlot.yAxis->setRange(minY, maxY);
}

void CorrelationPlotItem::populateMedianLinePlot()
{
    double maxY = 0.0;
    double minY = 0.0;

    auto* graph = _customPlot.addGraph();
    graph->setPen(QPen(Qt::black));
    graph->setName(tr("Median average of selection"));

    QVector<double> xData(static_cast<int>(_columnCount));
    // xData is just the column indices
    std::iota(std::begin(xData), std::end(xData), 0);

    QVector<double> rowsEntries(_selectedRows.length());
    QVector<double> yDataAvg(static_cast<int>(_columnCount));

    for(int col = 0; col < static_cast<int>(_columnCount); col++)
    {
        rowsEntries.clear();
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            rowsEntries.push_back(_data[static_cast<int>(index)]);
        }

        if(!_selectedRows.empty())
        {
            std::sort(rowsEntries.begin(), rowsEntries.end());
            double median = 0.0;
            if(rowsEntries.length()  % 2 == 0)
                median = (rowsEntries[rowsEntries.length() / 2 - 1] + rowsEntries[rowsEntries.length() / 2]) / 2.0;
            else
                median = rowsEntries[rowsEntries.length() / 2];

            yDataAvg[col] = median;

            maxY = std::max(maxY, yDataAvg[col]);
            minY = std::min(minY, yDataAvg[col]);
        }
    }

    graph->setData(xData, yDataAvg, true);

    auto* plotModeTextElement = new QCPTextElement(&_customPlot);
    plotModeTextElement->setLayer(_tooltipLayer);
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Median average plot of %1 rows")
                .arg(_selectedRows.length())));
    plotModeTextElement->setVisible(true);

    _customPlot.plotLayout()->insertRow(1);
    _customPlot.plotLayout()->addElement(1, 0, plotModeTextElement);

    _customPlot.yAxis->setRange(minY, maxY);
}

void CorrelationPlotItem::populateMeanHistogramPlot()
{
    if(_selectedRows.isEmpty())
        return;

    double maxY = 0.0;
    double minY = 0.0;

    QVector<double> xData(static_cast<int>(_columnCount));
    // xData is just the column indices
    std::iota(std::begin(xData), std::end(xData), 0);

    // Use Average Calculation and set min / max
    QVector<double> yDataAvg = meanAverageData(minY, maxY);

    auto* histogramBars = new QCPBars(_customPlot.xAxis, _customPlot.yAxis);
    histogramBars->setName(tr("Mean histogram of selection"));
    histogramBars->setData(xData, yDataAvg, true);

    auto* plotModeTextElement = new QCPTextElement(&_customPlot);
    plotModeTextElement->setLayer(_tooltipLayer);
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Mean histogram of %1 rows"))
                .arg(_selectedRows.length()));
    plotModeTextElement->setVisible(true);

    _customPlot.plotLayout()->insertRow(1);
    _customPlot.plotLayout()->addElement(1, 0, plotModeTextElement);

    _customPlot.yAxis->setRange(minY, maxY);
}

static double medianOf(const QVector<double>& sortedData)
{
    if(sortedData.length() == 0)
        return 0.0;

    double median = 0.0;
    if(sortedData.length() % 2 == 0)
        median = (sortedData[sortedData.length() / 2 - 1] + sortedData[sortedData.length() / 2]) / 2.0;
    else
        median = sortedData[sortedData.length() / 2];
    return median;
}

void CorrelationPlotItem::populateIQRPlot()
{
    // Box-plots representing the IQR.
    // Whiskers represent the maximum and minimum non-outlier values
    // Outlier values are (< Q1 - 1.5IQR and > Q3 + 1.5IQR)

    auto* statPlot = new QCPStatisticalBox(_customPlot.xAxis, _customPlot.yAxis);
    statPlot->setName(tr("Median (IQR plots) of selection"));

    double maxY = 0.0;
    double minY = 0.0;

    QVector<double> rowsEntries(_selectedRows.length());
    QVector<double> outliers;

    // Calculate IQRs, outliers and ranges
    for(int col = 0; col < static_cast<int>(_columnCount); col++)
    {
        rowsEntries.clear();
        outliers.clear();
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            rowsEntries.push_back(_data[static_cast<int>(index)]);
        }

        if(!_selectedRows.empty())
        {
            std::sort(rowsEntries.begin(), rowsEntries.end());
            double secondQuartile = medianOf(rowsEntries);
            double firstQuartile = secondQuartile;
            double thirdQuartile = secondQuartile;

            // Don't calculate medians if there's only one sample!
            if(rowsEntries.size() > 1)
            {
                if(rowsEntries.size() % 2 == 0)
                {
                    firstQuartile = medianOf(
                                rowsEntries.mid(0, (rowsEntries.size() / 2)));
                    thirdQuartile = medianOf(
                                rowsEntries.mid((rowsEntries.size() / 2)));
                }
                else
                {
                    firstQuartile = medianOf(
                                rowsEntries.mid(0, ((rowsEntries.size() - 1) / 2)));
                    thirdQuartile = medianOf(
                                rowsEntries.mid(((rowsEntries.size() + 1) / 2)));
                }
            }

            double iqr = thirdQuartile - firstQuartile;
            double minValue = secondQuartile;
            double maxValue = secondQuartile;

            for(auto row : qAsConst(rowsEntries))
            {
                // Find Maximum and minimum non-outliers
                if(row < thirdQuartile + (iqr * 1.5))
                    maxValue = std::max(maxValue, row);
                if(row > firstQuartile - (iqr * 1.5))
                    minValue = std::min(minValue, row);

                // Find outliers
                if(row > thirdQuartile + (iqr * 1.5))
                    outliers.push_back(row);
                else if(row < firstQuartile - (iqr * 1.5))
                    outliers.push_back(row);

                maxY = std::max(maxY, row);
                minY = std::min(minY, row);
            }

            // Add data for each column individually because setData doesn't let us do outliers(??)
            statPlot->addData(col, minValue, firstQuartile, secondQuartile, thirdQuartile,
                              maxValue, outliers);
        }
    }

    auto* plotModeTextElement = new QCPTextElement(&_customPlot);
    plotModeTextElement->setLayer(_tooltipLayer);
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Median IQR box plots of %1 rows"))
                .arg(_selectedRows.length()));
    plotModeTextElement->setVisible(true);

    _customPlot.plotLayout()->insertRow(1);
    _customPlot.plotLayout()->addElement(1, 0, plotModeTextElement);

    _customPlot.yAxis->setRange(minY, maxY);
}

void CorrelationPlotItem::plotDispersion(QVector<double> stdDevs, const QString& name = QStringLiteral("Deviation"))
{
    auto visualType = static_cast<PlotDispersionVisualType>(_plotDispersionVisualType);
    if(visualType == PlotDispersionVisualType::Bars)
    {
        auto* stdDevBars = new QCPErrorBars(_customPlot.xAxis, _customPlot.yAxis);
        stdDevBars->setName(name);
        stdDevBars->setSelectable(QCP::SelectionType::stNone);
        stdDevBars->setAntialiased(false);
        stdDevBars->setDataPlottable(_customPlot.plottable(0));
        stdDevBars->setData(stdDevs);
    }
    else if(visualType == PlotDispersionVisualType::Area)
    {
        auto* devTop = new QCPGraph(_customPlot.xAxis, _customPlot.yAxis);
        auto* devBottom = new QCPGraph(_customPlot.xAxis, _customPlot.yAxis);
        devTop->setName(QStringLiteral("%1 Top").arg(name));
        devBottom->setName(QStringLiteral("%1 Bottom").arg(name));

        auto fillColour = _customPlot.plottable(0)->pen().color();
        auto penColour = _customPlot.plottable(0)->pen().color().lighter(150);
        fillColour.setAlpha(50);
        penColour.setAlpha(120);

        devTop->setChannelFillGraph(devBottom);
        devTop->setBrush(QBrush(fillColour));
        devTop->setPen(QPen(penColour));

        devBottom->setPen(QPen(penColour));

        devBottom->setSelectable(QCP::SelectionType::stNone);
        devTop->setSelectable(QCP::SelectionType::stNone);

        auto topErr = QVector<double>(static_cast<int>(_columnCount));
        auto bottomErr = QVector<double>(static_cast<int>(_columnCount));

        for(int i = 0; i < static_cast<int>(_columnCount); ++i)
        {
            topErr[i] = _customPlot.plottable(0)->interface1D()->dataMainValue(i) + stdDevs[i];
            bottomErr[i] = _customPlot.plottable(0)->interface1D()->dataMainValue(i) - stdDevs[i];
        }

        // xData is just the column indices
        QVector<double> xData(static_cast<int>(_columnCount));
        std::iota(std::begin(xData), std::end(xData), 0);

        devTop->setData(xData, topErr);
        devBottom->setData(xData, bottomErr);
    }
}

void CorrelationPlotItem::populateStdDevPlot()
{
    if(_selectedRows.isEmpty())
        return;

    double min = 0, max = 0;

    QVector<double> stdDevs(static_cast<int>(_columnCount));
    QVector<double> means(static_cast<int>(_columnCount));

    for(int col = 0; col < static_cast<int>(_columnCount); col++)
    {
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            means[col] += _data.at(static_cast<int>(index));
        }
        means[col] /= _selectedRows.count();

        double stdDev = 0.0;
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            stdDev += (_data.at(static_cast<int>(index)) - means.at(col)) *
                    (_data.at(static_cast<int>(index)) - means.at(col));
        }
        stdDev /= _columnCount;
        stdDev = std::sqrt(stdDev);
        stdDevs[col] = stdDev;

        min = std::min(min, means.at(col) - stdDev);
        max = std::max(max, means.at(col) + stdDev);
    }

    plotDispersion(stdDevs, QStringLiteral("Std Dev"));
    _customPlot.yAxis->setRange(min, max);
}

void CorrelationPlotItem::populateStdErrorPlot()
{
    if(_selectedRows.isEmpty())
        return;

    double min = 0, max = 0;

    QVector<double> stdErrs(static_cast<int>(_columnCount));
    QVector<double> means(static_cast<int>(_columnCount));

    for(int col = 0; col < static_cast<int>(_columnCount); col++)
    {
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            means[col] += _data.at(static_cast<int>(index));
        }
        means[col] /= _selectedRows.count();

        double stdErr = 0.0;
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            stdErr += (_data.at(static_cast<int>(index)) - means.at(col)) *
                    (_data.at(static_cast<int>(index)) - means.at(col));
        }
        stdErr /= _columnCount;
        stdErr = std::sqrt(stdErr) / std::sqrt(static_cast<double>(_selectedRows.length()));
        stdErrs[col] = stdErr;

        min = std::min(min, means.at(col) - stdErr);
        max = std::max(max, means.at(col) + stdErr);
    }

    plotDispersion(stdErrs, QStringLiteral("Std Err"));
    _customPlot.yAxis->setRange(min, max);
}

void CorrelationPlotItem::populateLinePlot()
{
    double maxY = 0.0;
    double minY = 0.0;

    QVector<double> yData; yData.reserve(_selectedRows.size());
    QVector<double> xData; xData.reserve(static_cast<int>(_columnCount));

    // Plot each row individually
    for(auto row : qAsConst(_selectedRows))
    {
        auto* graph = _customPlot.addGraph();
        graph->setPen(_rowColors.at(row));
        graph->setName(_graphNames[row]);

        yData.clear();
        xData.clear();

        double rowMean = 0;

        for(size_t col = 0; col < _columnCount; col++)
        {
            auto index = (row * _columnCount) + col;
            rowMean += _data.at(static_cast<int>(index));
        }
        rowMean /= _columnCount;

        double stdDev = 0;
        for(size_t col = 0; col < _columnCount; col++)
        {
            auto index = (row * _columnCount) + col;
            stdDev += (_data.at(static_cast<int>(index)) - rowMean) *
                    (_data.at(static_cast<int>(index)) - rowMean);
        }
        stdDev /= _columnCount;
        stdDev = std::sqrt(stdDev);
        double pareto = std::sqrt(stdDev);

        for(size_t col = 0; col < _columnCount; col++)
        {
            auto index = (row * _columnCount) + col;
            auto data = _data.at(static_cast<int>(index));
            switch(static_cast<PlotScaleType>(_plotScaleType))
            {
            case PlotScaleType::Log:
            {
                // LogY(x+c) where c is EPSILON
                // This prevents LogY(0) which is -inf
                // Log2(0+c) = -1057
                // Document this!
                const double EPSILON = std::nextafter(0.0, 1.0);
                data = std::log(data + EPSILON);
            }
                break;
            case PlotScaleType::MeanCentre:
                data -= rowMean;
                break;
            case PlotScaleType::UnitVariance:
                data -= rowMean;
                data /= stdDev;
                break;
            case PlotScaleType::Pareto:
                data -= rowMean;
                data /= pareto;
                break;
            default:
                break;
            }

            xData.append(static_cast<double>(col));
            yData.append(data);

            maxY = std::max(maxY, data);
            minY = std::min(minY, data);
        }
        graph->setData(xData, yData, true);
    }

    _customPlot.yAxis->setRange(minY, maxY);
}

void CorrelationPlotItem::configureLegend()
{
    if(_customPlot.plottableCount() <= 0 || !_showLegend)
        return;

    // Create a subLayout to position the Legend
    auto* subLayout = new QCPLayoutGrid;
    _customPlot.plotLayout()->insertColumn(1);
    _customPlot.plotLayout()->addElement(0, 1, subLayout);

    // Surround the legend row in two empty rows that are stretched maximally, and
    // stretch the legend itself minimally, thus centreing the legend vertically
    subLayout->insertRow(0);
    subLayout->setRowStretchFactor(0, 1.0);
    subLayout->addElement(1, 0, _customPlot.legend);
    subLayout->setRowStretchFactor(1, std::numeric_limits<double>::min());
    subLayout->insertRow(2);
    subLayout->setRowStretchFactor(2, 1.0);

    const int marginSize = 5;
    subLayout->setMargins(QMargins(0, marginSize, marginSize, marginSize));
    _customPlot.legend->setMargins(QMargins(marginSize, marginSize, marginSize, marginSize));

    // BIGGEST HACK
    // Layouts and sizes aren't done until a replot, and layout is performed on another
    // thread which means it's too late to add or remove elements from the legend.
    // The anticipated sizes for the legend layout are calculated here but will break
    // if any additional rows are added to the plotLayout as the legend height is
    // estimated using the total height of the QQuickItem, not the (unknowable) plot height

    // See QCPPlottableLegendItem::draw for the reasoning behind this value
    const auto legendElementHeight = std::max(QFontMetrics(_customPlot.legend->font()).height(),
                                              _customPlot.legend->iconSize().height());

    const auto totalExternalMargins = subLayout->margins().top() + subLayout->margins().bottom();
    const auto totalInternalMargins = _customPlot.legend->margins().top() + _customPlot.legend->margins().bottom();
    const auto maxLegendHeight = _customPlot.height() - (totalExternalMargins + totalInternalMargins);

    int maxNumberOfElementsToDraw = 0;
    int accumulatedHeight = legendElementHeight;
    while(accumulatedHeight < maxLegendHeight)
    {
        accumulatedHeight += (_customPlot.legend->rowSpacing() + legendElementHeight);
        maxNumberOfElementsToDraw++;
    };

    const auto numberOfElementsToDraw = std::min(_customPlot.plottableCount(), maxNumberOfElementsToDraw);

    if(numberOfElementsToDraw > 0)
    {
        // Populate the legend
        _customPlot.legend->clear();
        for(int i = 0; i < numberOfElementsToDraw; i++)
            _customPlot.plottable(i)->addToLegend(_customPlot.legend);

        // Cap the legend count to only those visible
        if(_customPlot.plottableCount() > maxNumberOfElementsToDraw)
        {
            auto* moreText = new QCPTextElement(&_customPlot);
            moreText->setMargins(QMargins());
            moreText->setLayer(_tooltipLayer);
            moreText->setTextFlags(Qt::AlignLeft);
            moreText->setFont(_customPlot.legend->font());
            moreText->setTextColor(Qt::gray);
            moreText->setText(QString(tr("...and %1 more"))
                .arg(_customPlot.plottableCount() - maxNumberOfElementsToDraw + 1));
            moreText->setVisible(true);

            auto lastElementIndex = _customPlot.legend->rowColToIndex(_customPlot.legend->rowCount() - 1, 0);
            _customPlot.legend->removeAt(lastElementIndex);
            _customPlot.legend->addElement(moreText);

            // When we're overflowing, hackily enlarge the bottom margin to
            // compensate for QCP's layout algorithm being a bit rubbish
            auto margins = _customPlot.legend->margins();
            margins.setBottom(margins.bottom() * 3);
            _customPlot.legend->setMargins(margins);
        }

        // Make the plot take 85% of the width, and the legend the remaining 15%
        _customPlot.plotLayout()->setColumnStretchFactor(0, 0.85);
        _customPlot.plotLayout()->setColumnStretchFactor(1, 0.15);

        _customPlot.legend->setVisible(true);
    }
}

void CorrelationPlotItem::rebuildPlot()
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    QElapsedTimer buildTimer;
    buildTimer.start();

    _customPlot.legend->setVisible(false);

    _customPlot.clearGraphs();
    _customPlot.clearPlottables();

    while(_customPlot.plotLayout()->rowCount() > 1)
    {
        _customPlot.plotLayout()->removeAt(_customPlot.plotLayout()->rowColToIndex(1, 0));
        _customPlot.plotLayout()->simplify();
    }

    while(_customPlot.plotLayout()->columnCount() > 1)
    {
        // Save the legend from getting destroyed
        _customPlot.axisRect()->insetLayout()->addElement(_customPlot.legend, Qt::AlignRight);
        _customPlot.plotLayout()->removeAt(_customPlot.plotLayout()->rowColToIndex(0, 1));
        // Destroy the extra legend column
        _customPlot.plotLayout()->simplify();
    }

    auto plotAveragingType = static_cast<PlotAveragingType>(_plotAveragingType);
    if(plotAveragingType == PlotAveragingType::MeanLine)
        populateMeanLinePlot();
    else if(plotAveragingType == PlotAveragingType::MedianLine)
        populateMedianLinePlot();
    else if(plotAveragingType == PlotAveragingType::MeanHistogram)
        populateMeanHistogramPlot();
    else if(plotAveragingType == PlotAveragingType::IQRPlot)
        populateIQRPlot();
    else
        populateLinePlot();

    auto plotDispersionType = static_cast<PlotDispersionType>(_plotDispersionType);
    if(plotAveragingType != PlotAveragingType::Individual &&
            plotAveragingType != PlotAveragingType::IQRPlot)
    {
        if(plotDispersionType == PlotDispersionType::StdDev)
            populateStdDevPlot();
        else if(plotDispersionType == PlotDispersionType::StdErr)
            populateStdErrorPlot();
    }

    configureLegend();

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);
    _customPlot.xAxis->setTicker(categoryTicker);
    _customPlot.xAxis->setTickLabelRotation(90);

    _customPlot.xAxis->setLabel(_xAxisLabel);
    _customPlot.yAxis->setLabel(_yAxisLabel);

    _customPlot.xAxis->grid()->setVisible(_showGridLines);
    _customPlot.yAxis->grid()->setVisible(_showGridLines);

    if(_showColumnNames)
    {
        if(_elideLabelWidth > 0)
        {
            QFontMetrics metrics(_defaultFont9Pt);
            int column = 0;

            for(auto& labelName : _labelNames)
                categoryTicker->addTick(column++, metrics.elidedText(labelName, Qt::ElideRight, _elideLabelWidth));
        }
        else
        {
            // There is no room to display labels, so show a warning instead
            QString warning = tr("Resize To Expose Column Names");
            if(!_xAxisLabel.isEmpty())
                _customPlot.xAxis->setLabel(QString(QStringLiteral("%1 (%2)")).arg(_xAxisLabel, warning));
            else
                _customPlot.xAxis->setLabel(warning);
        }
    }

    _customPlot.setBackground(Qt::transparent);

    if(_debug)
        qDebug() << "buildPlot" << buildTimer.elapsed() << "ms";

    updatePixmap(CorrelationPlotUpdateType::ReplotAndRenderAndTooltips);
}

void CorrelationPlotItem::computeXAxisRange()
{
    auto min = 0.0;
    auto max = static_cast<double>(_columnCount);
    if(_showColumnNames)
    {
        auto maxVisibleColumns = columnAxisWidth() / columnLabelWidth();
        auto numHiddenColumns = max - maxVisibleColumns;

        double position = numHiddenColumns * _horizontalScrollPosition;

        if(position + maxVisibleColumns <= max)
        {
            min = position;
            max = position + maxVisibleColumns;
        }
    }

    QMetaObject::invokeMethod(_worker, "setXAxisRange",
        Q_ARG(double, min), Q_ARG(double, max));
}

void CorrelationPlotItem::setPlotDispersionVisualType(int plotDispersionVisualType)
{
    _plotDispersionVisualType = plotDispersionVisualType;
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::setYAxisLabel(const QString& plotYAxisLabel)
{
    _yAxisLabel = plotYAxisLabel;
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::setXAxisLabel(const QString& plotXAxisLabel)
{
    _xAxisLabel = plotXAxisLabel;
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::setPlotScaleType(int plotScaleType)
{
    _plotScaleType = plotScaleType;
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::setPlotAveragingType(int plotAveragingType)
{
    _plotAveragingType = plotAveragingType;
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::setPlotDispersionType(int plotDispersionType)
{
    _plotDispersionType = plotDispersionType;
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::setShowLegend(bool showLegend)
{
    _showLegend = showLegend;
    emit plotOptionsChanged();
    rebuildPlot();
}
void CorrelationPlotItem::setSelectedRows(const QVector<int>& selectedRows)
{
    _selectedRows = selectedRows;
    rebuildPlot();
}

void CorrelationPlotItem::setRowColors(const QVector<QColor>& rowColors)
{
    _rowColors = rowColors;
    rebuildPlot();
}

void CorrelationPlotItem::setLabelNames(const QStringList& labelNames)
{
    _labelNames = labelNames;
}

void CorrelationPlotItem::setElideLabelWidth(int elideLabelWidth)
{
    bool changed = (_elideLabelWidth != elideLabelWidth);
    _elideLabelWidth = elideLabelWidth;

    if(changed && _showColumnNames)
        rebuildPlot();
}

void CorrelationPlotItem::setColumnCount(size_t columnCount)
{
    _columnCount = columnCount;
}

void CorrelationPlotItem::setShowColumnNames(bool showColumnNames)
{
    bool changed = (_showColumnNames != showColumnNames);
    _showColumnNames = showColumnNames;

    if(changed)
    {
        emit visibleHorizontalFractionChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setShowGridLines(bool showGridLines)
{
    _showGridLines = showGridLines;
    rebuildPlot();
}

void CorrelationPlotItem::setHorizontalScrollPosition(double horizontalScrollPosition)
{
    _horizontalScrollPosition = horizontalScrollPosition;
    computeXAxisRange();

    updatePixmap(CorrelationPlotUpdateType::Render);
}

double CorrelationPlotItem::visibleHorizontalFraction()
{
    if(_showColumnNames)
        return (columnAxisWidth() / (columnLabelWidth() * _columnCount));

    return 1.0;
}

double CorrelationPlotItem::columnLabelWidth()
{
    QFontMetrics metrics(_defaultFont9Pt);
    const unsigned int columnPadding = 1;
    return metrics.height() + columnPadding;
}

double CorrelationPlotItem::columnAxisWidth()
{
    const auto& margins = _customPlot.axisRect()->margins();
    const unsigned int axisWidth = margins.left() + margins.right();

    //FIXME This value is wrong when the legend is enabled
    return width() - axisWidth;
}

void CorrelationPlotItem::updatePlotSize()
{
    computeXAxisRange();
    updatePixmap(CorrelationPlotUpdateType::Render);
}

void CorrelationPlotItem::savePlotImage(const QUrl& url, const QStringList& extensions)
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(extensions.contains(QStringLiteral("png")))
        _customPlot.savePng(url.toLocalFile());
    else if(extensions.contains(QStringLiteral("pdf")))
        _customPlot.savePdf(url.toLocalFile());
    else if(extensions.contains(QStringLiteral("jpg")))
        _customPlot.saveJpg(url.toLocalFile());

    QDesktopServices::openUrl(url);
}
