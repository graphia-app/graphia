#include "correlationplotitem.h"

#include "correlationplugin.h"

#include "qcpcolumnannotations.h"

#include "shared/utils/scope_exit.h"
#include "shared/utils/thread.h"
#include "shared/utils/utils.h"
#include "shared/utils/random.h"
#include "shared/utils/color.h"
#include "shared/utils/container.h"
#include "shared/utils/string.h"

#include <QDesktopServices>
#include <QSet>
#include <QCollator>

#include <cmath>
#include <algorithm>

CorrelationPlotWorker::CorrelationPlotWorker(std::recursive_mutex& mutex,
    QCustomPlot& customPlot, QCPLayer& tooltipLayer) :
    _debug(qEnvironmentVariableIntValue("QCUSTOMPLOT_DEBUG") != 0),
    _mutex(&mutex), _busy(false),
    _customPlot(&customPlot), _tooltipLayer(&tooltipLayer)
{
    // Qt requires that this is created on the UI thread, so we do so here
    // Note QCustomPlot takes ownership of the memory, so it does not need
    // to be deleted
    _surface = new QOffscreenSurface;
    _surface->setFormat(QSurfaceFormat::defaultFormat());
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

        u::setCurrentThreadName(QStringLiteral("CorrPlotRender"));
    }

    _customPlot->setGeometry(0, 0, _width, _height);

    // Since QCustomPlot is a QWidget, it is never technically visible, so never generates
    // a resizeEvent, so its viewport never gets set, so we must do so manually
    _customPlot->setViewport(_customPlot->geometry());

    auto elements = _customPlot->plotLayout()->elements(true);
    for(auto* element : elements)
    {
        auto* axisRect = dynamic_cast<QCPAxisRect*>(element);
        if(axisRect != nullptr)
            axisRect->axis(QCPAxis::atBottom)->setRange(_xAxisMin, _xAxisMax);
    }

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
    //FIXME commented until QTBUG-70148 is fixed
    //setRenderTarget(RenderTarget::FramebufferObject);

    // Discard the defaults...
    _customPlot.plotLayout()->clear();

    // ...and manage our own layout
    _mainAxisLayout = new QCPLayoutGrid;
    _customPlot.plotLayout()->addElement(_mainAxisLayout);

    _mainAxisRect = new QCPAxisRect(&_customPlot);
    _mainAxisLayout->addElement(_mainAxisRect);
    _mainXAxis = _mainAxisRect->axis(QCPAxis::atBottom);
    _mainYAxis = _mainAxisRect->axis(QCPAxis::atLeft);

    for(auto& axis : _mainAxisRect->axes())
    {
        axis->setLayer(QStringLiteral("axes"));
        axis->grid()->setLayer(QStringLiteral("grid"));
    }

    // Layer to keep individual line plots separate from everything else
    _customPlot.addLayer(QStringLiteral("lineGraphLayer"));
    _lineGraphLayer = _customPlot.layer(QStringLiteral("lineGraphLayer"));

    _customPlot.addLayer(QStringLiteral("tooltipLayer"));
    _tooltipLayer = _customPlot.layer(QStringLiteral("tooltipLayer"));
    _tooltipLayer->setMode(QCPLayer::LayerMode::lmBuffered);

    _customPlot.setAutoAddPlottableToLegend(false);

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

    connect(this, &QQuickPaintedItem::widthChanged, [this]
    {
        QMetaObject::invokeMethod(_worker, "setWidth", Q_ARG(int, width()));
    });

    connect(this, &QQuickPaintedItem::heightChanged, [this]
    {
        QMetaObject::invokeMethod(_worker, "setHeight",
            Q_ARG(int, std::max(static_cast<int>(height()), minimumHeight())));
    });

    connect(_worker, &CorrelationPlotWorker::pixmapUpdated, this, &CorrelationPlotItem::onPixmapUpdated);
    connect(this, &CorrelationPlotItem::enabledChanged, [this] { update(); });
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
    connect(this, &QQuickPaintedItem::widthChanged, this, &CorrelationPlotItem::isWideChanged);
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

    // Render the plot in the bottom left; that way when its container
    // is resized, it doesn't hop around vertically, as it would if
    // it had been rendered from the top left
    int yDest = height() - _pixmap.height();

    if(!isEnabled())
    {
        // Create a desaturated version of the pixmap
        auto image = _pixmap.toImage();
        const auto bytes = image.depth() >> 3;

        for(int y = 0; y < image.height(); y++)
        {
            auto scanLine = image.scanLine(y);
            for(int x = 0; x < image.width(); x++)
            {
                auto pixel = reinterpret_cast<QRgb*>(scanLine + (x * bytes));
                const int gray = qGray(*pixel);
                const int alpha = qAlpha(*pixel);
                *pixel = QColor(gray, gray, gray, alpha).rgba();
            }
        }

        // The pixmap that QCustomPlot creates is a mixture of premultipled
        // pixels and pixels with an alpha value, so to keep things simple
        // we just use an alpha value in the destination buffer instead
        painter->setCompositionMode(QPainter::CompositionMode_DestinationOver);

        auto backgroundColor = QColor(Qt::white);
        backgroundColor.setAlpha(127);

        painter->fillRect(0, 0, width(), height(), backgroundColor);
        painter->drawPixmap(0, yDest, QPixmap::fromImage(image));
    }
    else
    {
        painter->fillRect(0, 0, width(), height(), Qt::white);
        painter->drawPixmap(0, yDest, _pixmap);
    }
}

void CorrelationPlotItem::onPixmapUpdated(const QPixmap& pixmap)
{
    if(!pixmap.isNull())
    {
        _pixmap = pixmap;
        update();
    }

    // Updates were attempted during the render, so perform
    // them now, now that the render has finished
    if(_rebuildRequired)
        rebuildPlot();

    if(_tooltipUpdateRequired)
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
    switch(event->button())
    {
    case Qt::LeftButton:
        onLeftClick(event->pos());
        break;

    case Qt::RightButton:
        emit rightClick();
        break;

    default:
        break;
    }
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
        _tooltipUpdateRequired = true;
        return;
    }

    _tooltipUpdateRequired = false;

    QCPAbstractPlottable* plottableUnderCursor = nullptr;
    QCPAxisRect* axisRectUnderCursor = nullptr;

    if(_hoverPoint.x() >= 0.0 && _hoverPoint.y() >= 0.0)
    {
        axisRectUnderCursor = _customPlot.axisRectAt(_hoverPoint);
        plottableUnderCursor = _customPlot.plottableAt(_hoverPoint, true);

        if(plottableUnderCursor != nullptr &&
            plottableUnderCursor->keyAxis()->axisRect() != axisRectUnderCursor)
        {
            // The plottable under the cursor is not on the axisRect
            // under the cursor (!?)
            plottableUnderCursor = nullptr;
        }
    }

    bool showTooltip = false;
    _hoverLabel->setText(QString());
    _itemTracer->setGraph(nullptr);

    if(plottableUnderCursor != nullptr || axisRectUnderCursor != nullptr)
    {
        if(axisRectUnderCursor == _mainAxisRect)
        {
            if(auto graph = dynamic_cast<QCPGraph*>(plottableUnderCursor))
            {
                _itemTracer->setGraph(graph);
                _itemTracer->setGraphKey(_mainXAxis->pixelToCoord(_hoverPoint.x()));
                showTooltip = true;
            }
            else if(auto bars = dynamic_cast<QCPBars*>(plottableUnderCursor))
            {
                auto xCoord = std::lround(_mainXAxis->pixelToCoord(_hoverPoint.x()));
                _itemTracer->position->setPixelPosition(bars->dataPixelPosition(xCoord));
                showTooltip = true;
            }
            else if(auto boxPlot = dynamic_cast<QCPStatisticalBox*>(plottableUnderCursor))
            {
                // Only show simple tooltips for now, can extend this later...
                auto xCoord = std::lround(_mainXAxis->pixelToCoord(_hoverPoint.x()));
                _itemTracer->position->setPixelPosition(boxPlot->dataPixelPosition(xCoord));
                showTooltip = true;
            }
        }
        else if(axisRectUnderCursor == _columnAnnotationsAxisRect)
        {
            if(axisRectUnderCursor->rect().contains(_hoverPoint.toPoint()))
            {
                auto point = _hoverPoint - axisRectUnderCursor->topLeft();
                auto bottomAxis = axisRectUnderCursor->axis(QCPAxis::atBottom);
                const auto& bottomRange = bottomAxis->range();
                auto bottomSize = bottomRange.size();
                auto xf = bottomRange.lower + 0.5 +
                        (static_cast<double>(point.x() * bottomSize) / axisRectUnderCursor->width());

                auto x = static_cast<int>(xf);
                int y = (point.y() * numVisibleColumnAnnotations()) / axisRectUnderCursor->height();

                auto text = columnAnnotationValueAt(x, y);

                if(!text.isEmpty())
                {
                    _itemTracer->position->setPixelPosition(_hoverPoint);
                    _hoverLabel->setText(text);

                    showTooltip = true;
                }
            }
        }
    }

    if(showTooltip)
    {
        _itemTracer->setVisible(true);
        _itemTracer->setInterpolating(false);
        _itemTracer->updatePosition();
        auto itemTracerPosition = _itemTracer->anchor(QStringLiteral("position"))->pixelPosition();

        _hoverLabel->setVisible(true);

        auto key = _itemTracer->position->key();

        if(_hoverLabel->text().isEmpty() && plottableUnderCursor != nullptr && key >= 0.0)
        {
            auto index = static_cast<size_t>(key);

            if(index < _pluginInstance->numColumns())
            {
                auto mappedCol = static_cast<int>(_sortMap.at(index));
                _hoverLabel->setText(QStringLiteral("%1, %2: %3")
                    .arg(plottableUnderCursor->name(), _pluginInstance->columnName(mappedCol))
                    .arg(_itemTracer->position->value()));
            }
        }

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

        if(plottableUnderCursor != nullptr)
        {
            _hoverColorRect->setVisible(true);

            QColor color = plottableUnderCursor->pen().color();

            _hoverColorRect->setBrush(QBrush(color));
            _hoverColorRect->bottomRight->setPixelPosition(
                {_hoverLabel->bottomRight->pixelPosition().x() + COLOR_RECT_WIDTH,
                _hoverLabel->bottomRight->pixelPosition().y()});
        }
        else
            _hoverColorRect->setVisible(false);
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
    min = std::numeric_limits<double>::max();
    max = std::numeric_limits<double>::lowest();

    // Use Average Calculation
    QVector<double> yDataAvg; yDataAvg.reserve(_selectedRows.size());

    for(size_t col = 0; col < _pluginInstance->numColumns(); col++)
    {
        double runningTotal = 0.0;
        for(auto row : qAsConst(_selectedRows))
            runningTotal += _pluginInstance->dataAt(row, static_cast<int>(_sortMap[col]));

        yDataAvg.append(runningTotal / _selectedRows.length());

        max = std::max(max, yDataAvg.back());
        min = std::min(min, yDataAvg.back());
    }
    return yDataAvg;
}

void CorrelationPlotItem::updateColumnAnnotationVisibility()
{
    auto mainPlotHeight = height() - columnAnnotaionsHeight(_columnAnnotationSelectionModeEnabled);
    bool showColumnAnnotations = mainPlotHeight >= minimumHeight();

    if(showColumnAnnotations != _showColumnAnnotations)
    {
        _showColumnAnnotations = showColumnAnnotations;

        // If we can't show column annotations, we also can't be in selection mode
        if(!_showColumnAnnotations)
            setColumnAnnotationSelectionModeEnabled(false);

        rebuildPlot();
    }
}

bool CorrelationPlotItem::canShowColumnAnnotationSelection() const
{
    auto mainPlotHeight = height() - columnAnnotaionsHeight(true);
    return mainPlotHeight >= minimumHeight();
}

void CorrelationPlotItem::populateMeanLinePlot()
{
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    auto* graph = _customPlot.addGraph();
    graph->setPen(QPen(Qt::black, 2.0, Qt::DashLine));
    graph->setName(tr("Mean average of selection"));

    QVector<double> xData(static_cast<int>(_pluginInstance->numColumns()));
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

    _mainAxisLayout->addElement(_mainAxisLayout->rowCount(), 0, plotModeTextElement);

    setYAxisRange(minY, maxY);

    _meanPlot = graph;
}

void CorrelationPlotItem::populateMedianLinePlot()
{
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    auto* graph = _customPlot.addGraph();
    graph->setPen(QPen(Qt::black, 2.0, Qt::DashLine));
    graph->setName(tr("Median average of selection"));

    QVector<double> xData(static_cast<int>(_pluginInstance->numColumns()));
    // xData is just the column indices
    std::iota(std::begin(xData), std::end(xData), 0);

    QVector<double> rowsEntries(_selectedRows.length());
    QVector<double> yDataAvg(static_cast<int>(_pluginInstance->numColumns()));

    for(int col = 0; col < static_cast<int>(_pluginInstance->numColumns()); col++)
    {
        rowsEntries.clear();
        for(auto row : qAsConst(_selectedRows))
            rowsEntries.push_back(_pluginInstance->dataAt(row, static_cast<int>(_sortMap[col])));

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

    _mainAxisLayout->addElement(_mainAxisLayout->rowCount(), 0, plotModeTextElement);

    setYAxisRange(minY, maxY);

    _meanPlot = graph;
}

void CorrelationPlotItem::populateMeanHistogramPlot()
{
    if(_selectedRows.isEmpty())
        return;

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    QVector<double> xData(static_cast<int>(_pluginInstance->numColumns()));
    // xData is just the column indices
    std::iota(std::begin(xData), std::end(xData), 0);

    // Use Average Calculation and set min / max
    QVector<double> yDataAvg = meanAverageData(minY, maxY);

    auto* histogramBars = new QCPBars(_mainXAxis, _mainYAxis);
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

    _mainAxisLayout->addElement(_mainAxisLayout->rowCount(), 0, plotModeTextElement);

    setYAxisRange(minY, maxY);

    _meanPlot = histogramBars;
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

    auto* statPlot = new QCPStatisticalBox(_mainXAxis, _mainYAxis);
    statPlot->setName(tr("Median (IQR plots) of selection"));

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    QVector<double> rowsEntries(_selectedRows.length());
    QVector<double> outliers;

    // Calculate IQRs, outliers and ranges
    for(int col = 0; col < static_cast<int>(_pluginInstance->numColumns()); col++)
    {
        rowsEntries.clear();
        outliers.clear();
        for(auto row : qAsConst(_selectedRows))
            rowsEntries.push_back(_pluginInstance->dataAt(row, static_cast<int>(_sortMap[col])));

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
                    firstQuartile = medianOf(rowsEntries.mid(0, (rowsEntries.size() / 2)));
                    thirdQuartile = medianOf(rowsEntries.mid((rowsEntries.size() / 2)));
                }
                else
                {
                    firstQuartile = medianOf(rowsEntries.mid(0, ((rowsEntries.size() - 1) / 2)));
                    thirdQuartile = medianOf(rowsEntries.mid(((rowsEntries.size() + 1) / 2)));
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

    _mainAxisLayout->addElement(_mainAxisLayout->rowCount(), 0, plotModeTextElement);

    setYAxisRange(minY, maxY);
}

void CorrelationPlotItem::plotDispersion(QVector<double> stdDevs, const QString& name = QStringLiteral("Deviation"))
{
    auto visualType = static_cast<PlotDispersionVisualType>(_plotDispersionVisualType);
    if(visualType == PlotDispersionVisualType::Bars)
    {
        auto* stdDevBars = new QCPErrorBars(_mainXAxis, _mainYAxis);
        stdDevBars->setName(name);
        stdDevBars->setSelectable(QCP::SelectionType::stNone);
        stdDevBars->setAntialiased(false);
        stdDevBars->setDataPlottable(_meanPlot);
        stdDevBars->setData(stdDevs);
    }
    else if(visualType == PlotDispersionVisualType::Area)
    {
        auto* devTop = new QCPGraph(_mainXAxis, _mainYAxis);
        auto* devBottom = new QCPGraph(_mainXAxis, _mainYAxis);
        devTop->setName(QStringLiteral("%1 Top").arg(name));
        devBottom->setName(QStringLiteral("%1 Bottom").arg(name));

        auto fillColour = _meanPlot->pen().color();
        auto penColour = _meanPlot->pen().color().lighter(150);
        fillColour.setAlpha(50);
        penColour.setAlpha(120);

        devTop->setChannelFillGraph(devBottom);
        devTop->setBrush(QBrush(fillColour));
        devTop->setPen(QPen(penColour));

        devBottom->setPen(QPen(penColour));

        devBottom->setSelectable(QCP::SelectionType::stNone);
        devTop->setSelectable(QCP::SelectionType::stNone);

        auto topErr = QVector<double>(static_cast<int>(_pluginInstance->numColumns()));
        auto bottomErr = QVector<double>(static_cast<int>(_pluginInstance->numColumns()));

        for(int i = 0; i < static_cast<int>(_pluginInstance->numColumns()); i++)
        {
            topErr[i] = _meanPlot->interface1D()->dataMainValue(i) + stdDevs[i];
            bottomErr[i] = _meanPlot->interface1D()->dataMainValue(i) - stdDevs[i];
        }

        // xData is just the column indices
        QVector<double> xData(static_cast<int>(_pluginInstance->numColumns()));
        std::iota(std::begin(xData), std::end(xData), 0);

        devTop->setData(xData, topErr);
        devBottom->setData(xData, bottomErr);
    }

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for(int i = 0; i < static_cast<int>(_pluginInstance->numColumns()); i++)
    {
        minY = std::min(minY, _meanPlot->interface1D()->dataMainValue(i) - stdDevs[i]);
        maxY = std::max(maxY, _meanPlot->interface1D()->dataMainValue(i) + stdDevs[i]);
    }

    setYAxisRange(minY, maxY);
}

void CorrelationPlotItem::setPluginInstance(CorrelationPluginInstance* pluginInstance)
{
    _pluginInstance = pluginInstance;

    connect(_pluginInstance, &CorrelationPluginInstance::nodeColorsChanged,
        this, &CorrelationPlotItem::rebuildPlot);
}

void CorrelationPlotItem::populateStdDevPlot()
{
    if(_selectedRows.isEmpty())
        return;

    QVector<double> stdDevs(static_cast<int>(_pluginInstance->numColumns()));
    QVector<double> means(static_cast<int>(_pluginInstance->numColumns()));

    for(int col = 0; col < static_cast<int>(_pluginInstance->numColumns()); col++)
    {
        for(auto row : qAsConst(_selectedRows))
            means[col] += _pluginInstance->dataAt(row, static_cast<int>(_sortMap[col]));

        means[col] /= _selectedRows.count();

        double stdDev = 0.0;
        for(auto row : qAsConst(_selectedRows))
        {
            auto value = _pluginInstance->dataAt(row, static_cast<int>(_sortMap[col])) - means.at(col);
            stdDev += (value * value);
        }

        stdDev /= _pluginInstance->numColumns();
        stdDev = std::sqrt(stdDev);
        stdDevs[col] = stdDev;
    }

    plotDispersion(stdDevs, QStringLiteral("Std Dev"));
}

void CorrelationPlotItem::populateStdErrorPlot()
{
    if(_selectedRows.isEmpty())
        return;

    QVector<double> stdErrs(static_cast<int>(_pluginInstance->numColumns()));
    QVector<double> means(static_cast<int>(_pluginInstance->numColumns()));

    for(int col = 0; col < static_cast<int>(_pluginInstance->numColumns()); col++)
    {
        for(auto row : qAsConst(_selectedRows))
            means[col] += _pluginInstance->dataAt(row, static_cast<int>(_sortMap[col]));

        means[col] /= _selectedRows.count();

        double stdErr = 0.0;
        for(auto row : qAsConst(_selectedRows))
        {
            auto value = _pluginInstance->dataAt(row, static_cast<int>(_sortMap[col])) - means.at(col);
            stdErr += (value * value);
        }

        stdErr /= _pluginInstance->numColumns();
        stdErr = std::sqrt(stdErr) / std::sqrt(static_cast<double>(_selectedRows.length()));
        stdErrs[col] = stdErr;
    }

    plotDispersion(stdErrs, QStringLiteral("Std Err"));
}

void CorrelationPlotItem::populateLinePlot()
{
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    QVector<double> yData; yData.reserve(_selectedRows.size());
    QVector<double> xData; xData.reserve(static_cast<int>(_pluginInstance->numColumns()));

    // Plot each row individually
    for(auto row : qAsConst(_selectedRows))
    {
        QCPGraph* graph = nullptr;
        double rowMinY = std::numeric_limits<double>::max();
        double rowMaxY = std::numeric_limits<double>::lowest();

        if(!_lineGraphCache.contains(row))
        {
            graph = _customPlot.addGraph(_mainXAxis, _mainYAxis);
            graph->setLayer(_lineGraphLayer);

            double rowSum = 0.0;
            for(size_t col = 0; col < _pluginInstance->numColumns(); col++)
                rowSum += _pluginInstance->dataAt(row, static_cast<int>(_sortMap[col]));

            double rowMean = rowSum / _pluginInstance->numColumns();

            double variance = 0.0;
            for(size_t col = 0; col < _pluginInstance->numColumns(); col++)
            {
                auto value = _pluginInstance->dataAt(row, static_cast<int>(_sortMap[col])) - rowMean;
                variance += (value * value);
            }

            variance /= _pluginInstance->numColumns();
            double stdDev = std::sqrt(variance);
            double pareto = std::sqrt(stdDev);

            yData.clear();
            xData.clear();

            for(size_t col = 0; col < _pluginInstance->numColumns(); col++)
            {
                auto value = _pluginInstance->dataAt(row, static_cast<int>(_sortMap[col]));

                switch(static_cast<PlotScaleType>(_plotScaleType))
                {
                case PlotScaleType::Log:
                {
                    // LogY(x+c) where c is EPSILON
                    // This prevents LogY(0) which is -inf
                    // Log2(0+c) = -1057
                    // Document this!
                    const double EPSILON = std::nextafter(0.0, 1.0);
                    value = std::log(value + EPSILON);
                }
                    break;
                case PlotScaleType::MeanCentre:
                    value -= rowMean;
                    break;
                case PlotScaleType::UnitVariance:
                    value -= rowMean;
                    value /= stdDev;
                    break;
                case PlotScaleType::Pareto:
                    value -= rowMean;
                    value /= pareto;
                    break;
                default:
                    break;
                }

                xData.append(static_cast<double>(col));
                yData.append(value);

                rowMinY = std::min(rowMinY, value);
                rowMaxY = std::max(rowMaxY, value);
            }

            graph->setData(xData, yData, true);

            _lineGraphCache.insert(row, {graph, rowMinY, rowMaxY});
        }
        else
        {
            const auto& v = _lineGraphCache.value(row);
            graph = v._graph;
            rowMinY = v._minY;
            rowMaxY = v._maxY;
        }

        minY = std::min(minY, rowMinY);
        maxY = std::max(maxY, rowMaxY);

        graph->setVisible(true);
        graph->setSelectable(QCP::SelectionType::stWhole);

        graph->setPen(_pluginInstance->nodeColorForRow(row));
        graph->setName(_pluginInstance->rowName(row));
    }

    setYAxisRange(minY, maxY);
}

QCPAxis* CorrelationPlotItem::configureColumnAnnotations(QCPAxis* xAxis)
{
    const auto& columnAnnotations = _pluginInstance->columnAnnotations();

    if(columnAnnotations.empty())
        return xAxis;

    if(!_showColumnAnnotations)
        return xAxis;

    if(!_columnAnnotationSelectionModeEnabled && _visibleColumnAnnotationNames.empty())
        return xAxis;

    size_t numColumnAnnotations = numVisibleColumnAnnotations();

    _columnAnnotationsAxisRect = new QCPAxisRect(&_customPlot);
    _mainAxisLayout->addElement(_mainAxisLayout->rowCount(), 0, _columnAnnotationsAxisRect);

    const auto separation = 8;
    _mainAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msTop);
    _mainAxisRect->setMargins(QMargins(0, 0, 0, separation));
    _columnAnnotationsAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    _columnAnnotationsAxisRect->setMargins(QMargins(0, 0, separation, 0));

    // Align the left and right hand sides of the axes
    auto* group = new QCPMarginGroup(&_customPlot);
    _mainAxisRect->setMarginGroup(QCP::msLeft|QCP::msRight, group);
    _columnAnnotationsAxisRect->setMarginGroup(QCP::msLeft|QCP::msRight, group);

    xAxis->setTickLabels(false);

    auto caXAxis = _columnAnnotationsAxisRect->axis(QCPAxis::atBottom);
    auto caYAxis = _columnAnnotationsAxisRect->axis(QCPAxis::atLeft);

    auto h = columnAnnotaionsHeight(_columnAnnotationSelectionModeEnabled);
    _columnAnnotationsAxisRect->setMinimumSize(0, h);
    _columnAnnotationsAxisRect->setMaximumSize(QWIDGETSIZE_MAX, h);

    QSharedPointer<QCPAxisTickerText> columnAnnotationTicker(new QCPAxisTickerText);
    size_t y = numColumnAnnotations - 1;

    auto* qcpColumnAnnotations = new QCPColumnAnnotations(caXAxis, caYAxis);

    for(const auto& columnAnnotation : columnAnnotations)
    {
        auto selected = u::contains(_visibleColumnAnnotationNames, columnAnnotation.name());
        bool visible = selected || _columnAnnotationSelectionModeEnabled;

        if(visible)
        {
            qcpColumnAnnotations->setData(y, _sortMap, selected, &columnAnnotation);

            QString postfix;

            if(_columnAnnotationSelectionModeEnabled)
                postfix = selected ? QStringLiteral(u" ☑") : QStringLiteral(u" ☐");

            double tickPosition = static_cast<double>(y) + 0.5;
            columnAnnotationTicker->addTick(tickPosition, columnAnnotation.name() + postfix);

            y--;
        }
    }

    caYAxis->setTickPen(QPen(Qt::transparent));
    caYAxis->setTicker(columnAnnotationTicker);
    caYAxis->setRange(0.0, numColumnAnnotations);

    caXAxis->setTickPen(QPen(Qt::transparent));
    caXAxis->setBasePen(QPen(Qt::transparent));
    caYAxis->setBasePen(QPen(Qt::transparent));

    caXAxis->grid()->setVisible(false);
    caYAxis->grid()->setVisible(false);

    xAxis = caXAxis;

    return xAxis;
}

void CorrelationPlotItem::configureLegend()
{
    if(_selectedRows.empty() || !_showLegend)
        return;

    // Create a subLayout to position the Legend
    auto* subLayout = new QCPLayoutGrid;
    _customPlot.plotLayout()->insertColumn(1);
    _customPlot.plotLayout()->addElement(0, 1, subLayout);

    auto* legend = new QCPLegend;

    // Surround the legend row in two empty rows that are stretched maximally, and
    // stretch the legend itself minimally, thus centreing the legend vertically
    subLayout->insertRow(0);
    subLayout->setRowStretchFactor(0, 1.0);
    subLayout->addElement(1, 0, legend);
    subLayout->setRowStretchFactor(1, std::numeric_limits<double>::min());
    subLayout->insertRow(2);
    subLayout->setRowStretchFactor(2, 1.0);

    legend->setLayer(QStringLiteral("legend"));

    const int marginSize = 5;
    legend->setMargins(QMargins(marginSize, marginSize, marginSize, marginSize));
    subLayout->setMargins(QMargins(0, marginSize, marginSize, marginSize));

    // BIGGEST HACK
    // Layouts and sizes aren't done until a replot, and layout is performed on another
    // thread which means it's too late to add or remove elements from the legend.
    // The anticipated sizes for the legend layout are calculated here but will break
    // if any additional rows are added to the plotLayout as the legend height is
    // estimated using the total height of the QQuickItem, not the (unknowable) plot height

    // See QCPPlottableLegendItem::draw for the reasoning behind this value
    const auto legendElementHeight = std::max(QFontMetrics(legend->font()).height(),
                                              legend->iconSize().height());

    const auto totalExternalMargins = subLayout->margins().top() + subLayout->margins().bottom();
    const auto totalInternalMargins = legend->margins().top() + legend->margins().bottom();
    const auto maxLegendHeight = _customPlot.height() - (totalExternalMargins + totalInternalMargins);

    int maxNumberOfElementsToDraw = 0;
    int accumulatedHeight = legendElementHeight;
    while(accumulatedHeight < maxLegendHeight)
    {
        accumulatedHeight += (legend->rowSpacing() + legendElementHeight);
        maxNumberOfElementsToDraw++;
    };

    auto numberOfElementsToDraw = std::min(_selectedRows.size(), maxNumberOfElementsToDraw);

    if(numberOfElementsToDraw > 0)
    {
        // Populate the legend
        legend->clear();
        for(int i = 0; i < _customPlot.plottableCount() && numberOfElementsToDraw > 0; i++)
        {
            auto* plottable = _customPlot.plottable(i);

            // Don't add invisible plots to the legend
            if(!plottable->visible() || plottable->valueAxis() != _mainYAxis)
                continue;

            plottable->addToLegend(legend);
            numberOfElementsToDraw--;
        }

        // Cap the legend count to only those visible
        if(_selectedRows.size() > maxNumberOfElementsToDraw)
        {
            auto* moreText = new QCPTextElement(&_customPlot);
            moreText->setMargins(QMargins());
            moreText->setLayer(_tooltipLayer);
            moreText->setTextFlags(Qt::AlignLeft);
            moreText->setFont(legend->font());
            moreText->setTextColor(Qt::gray);
            moreText->setText(QString(tr("…and %1 more"))
                .arg(_selectedRows.size() - maxNumberOfElementsToDraw + 1));
            moreText->setVisible(true);

            auto lastElementIndex = legend->rowColToIndex(legend->rowCount() - 1, 0);
            legend->removeAt(lastElementIndex);
            legend->addElement(moreText);

            // When we're overflowing, hackily enlarge the bottom margin to
            // compensate for QCP's layout algorithm being a bit rubbish
            auto margins = legend->margins();
            margins.setBottom(margins.bottom() * 3);
            legend->setMargins(margins);
        }

        // Make the plot take 85% of the width, and the legend the remaining 15%
        _customPlot.plotLayout()->setColumnStretchFactor(0, 0.85);
        _customPlot.plotLayout()->setColumnStretchFactor(1, 0.15);

        legend->setVisible(true);
    }
}

void CorrelationPlotItem::onLeftClick(const QPoint& pos)
{
    auto* axisRect = _customPlot.axisRectAt(pos);

    if(_columnAnnotationSelectionModeEnabled && axisRect == _columnAnnotationsAxisRect)
    {
        const auto& columnAnnotations = _pluginInstance->columnAnnotations();

        auto rectHeight = axisRect->bottom() - axisRect->top();
        auto y = pos.y() - axisRect->top();

        if(y < rectHeight)
        {
            auto index = (y * columnAnnotations.size()) / (rectHeight);

            if(index < columnAnnotations.size())
            {
                const auto& columnAnnotation = columnAnnotations.at(index);
                const auto& name = columnAnnotation.name();

                if(u::contains(_visibleColumnAnnotationNames, name))
                    _visibleColumnAnnotationNames.erase(name);
                else
                    _visibleColumnAnnotationNames.insert(name);

                emit plotOptionsChanged();

                rebuildPlot();
            }
        }
    }
}

void CorrelationPlotItem::invalidateLineGraphCache()
{
    for(auto v : qAsConst(_lineGraphCache))
        _customPlot.removeGraph(v._graph);

    _lineGraphCache.clear();
}

static void removeAllExcept(QCPLayoutGrid* layout, QCPLayoutElement* except)
{
    for(int i = layout->elementCount() - 1; i >= 0; --i)
    {
        auto* element = layout->elementAt(i);
        if(element != nullptr && element != except)
            layout->removeAt(i);
    }
    layout->simplify();
}

void CorrelationPlotItem::rebuildPlot()
{
    std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);

    if(!lock.owns_lock())
    {
        _rebuildRequired = true;
        return;
    }

    _rebuildRequired = false;

    QElapsedTimer buildTimer;
    buildTimer.start();

    updateSortMap();

    for(auto v : qAsConst(_lineGraphCache))
    {
        v._graph->setVisible(false);

        // For some (stupid) reason, QCustomPlot::plottableAt returns lines that
        // are invisible, but if we make them not selectable, it ignores them
        v._graph->setSelectable(QCP::SelectionType::stNone);
    }

    for(int i = _customPlot.plottableCount() - 1; i >= 0; --i)
    {
        auto* plottable = _customPlot.plottable(i);

        // Don't remove anything that's on the line graph layer,
        // as these are cached to avoid costly recreation
        if(plottable->layer() == _lineGraphLayer)
            continue;

        _customPlot.removePlottable(plottable);
    }

    _meanPlot = nullptr;
    _columnAnnotationsAxisRect = nullptr;

    // Return the plot layout to its immediate post-construction state
    removeAllExcept(_mainAxisLayout, _mainAxisRect);
    removeAllExcept(_customPlot.plotLayout(), _mainAxisLayout);

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

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);

    auto* xAxis = _mainXAxis;

    xAxis->setLabel({});
    xAxis->setTicker(categoryTicker);
    xAxis->setTicks(minColumnWidth() >= 1.0);

    xAxis->grid()->setVisible(_showGridLines);
    _mainYAxis->grid()->setVisible(_showGridLines);

    // Don't show an emphasised vertical zero line
    xAxis->grid()->setZeroLinePen(xAxis->grid()->pen());

    _mainYAxis->setLabel(_yAxisLabel);

    _mainAxisLayout->setRowSpacing(0);
    _mainAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msTop|QCP::msBottom);
    _mainAxisRect->setMargins(QMargins(0, 0, 0, 0));

    xAxis = configureColumnAnnotations(xAxis);

    configureLegend();

    xAxis->setTicker(categoryTicker);
    xAxis->setTickLabelRotation(90);
    xAxis->setTickLabels(_showColumnNames && (_elideLabelWidth > 0));

    xAxis->setLabel(_xAxisLabel);

    xAxis->setPadding(_xAxisPadding);

    for(size_t x = 0U; x < _pluginInstance->numColumns(); x++)
    {
        auto labelName = elideLabel(_pluginInstance->columnName(static_cast<int>(_sortMap[x])));
        categoryTicker->addTick(x, labelName);
    }

    if(_elideLabelWidth <= 0)
    {
        // There is no room to display labels, so show a warning instead
        QString warning;

        if(!_showColumnAnnotations && !_visibleColumnAnnotationNames.empty())
            warning = tr("Resize To Expose Column Information");
        else if(_showColumnNames)
            warning = tr("Resize To Expose Column Names");

        if(!_xAxisLabel.isEmpty() && !warning.isEmpty())
            xAxis->setLabel(QString(QStringLiteral("%1 (%2)")).arg(_xAxisLabel, warning));
        else
            xAxis->setLabel(warning);
    }

    _customPlot.setBackground(Qt::transparent);

    if(_debug)
        qDebug() << "buildPlot" << buildTimer.elapsed() << "ms";

    updatePixmap(CorrelationPlotUpdateType::ReplotAndRenderAndTooltips);
}

void CorrelationPlotItem::computeXAxisRange()
{
    auto min = 0.0;
    auto max = static_cast<double>(_pluginInstance->numColumns() - 1);
    auto maxVisibleColumns = columnAxisWidth() / minColumnWidth();
    auto numHiddenColumns = max - maxVisibleColumns;

    if(numHiddenColumns > 0.0)
    {
        double position = numHiddenColumns * _horizontalScrollPosition;
        min = position;
        max = position + maxVisibleColumns;
    }

    const auto padding = 0.5;
    min -= padding;
    max += padding;

    QMetaObject::invokeMethod(_worker, "setXAxisRange",
        Q_ARG(double, min), Q_ARG(double, max));
}

void CorrelationPlotItem::setYAxisRange(double min, double max)
{
    if(_includeYZero)
    {
        if(min > 0.0)
            min = 0.0;
        else if(max < 0.0)
            max = 0.0;
    }

    _mainYAxis->setRange(min, max);
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

void CorrelationPlotItem::setIncludeYZero(bool includeYZero)
{
    _includeYZero = includeYZero;
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::setShowAllColumns(bool showAllColumns)
{
    _showAllColumns = showAllColumns;
    emit visibleHorizontalFractionChanged();
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
    invalidateLineGraphCache();
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

void CorrelationPlotItem::setElideLabelWidth(int elideLabelWidth)
{
    bool changed = (_elideLabelWidth != elideLabelWidth);
    _elideLabelWidth = elideLabelWidth;

    if(changed && _showColumnNames)
        rebuildPlot();
}

void CorrelationPlotItem::setShowColumnNames(bool showColumnNames)
{
    if(_showColumnNames != showColumnNames)
    {
        _showColumnNames = showColumnNames;
        computeXAxisRange();
        emit visibleHorizontalFractionChanged();
        emit plotOptionsChanged();
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
    _horizontalScrollPosition = std::clamp(horizontalScrollPosition, 0.0, 1.0);
    computeXAxisRange();

    updatePixmap(CorrelationPlotUpdateType::Render);
}

void CorrelationPlotItem::setXAxisPadding(int padding)
{
    bool changed = _xAxisPadding != padding;
    _xAxisPadding = padding;

    if(changed)
        rebuildPlot();
}

void CorrelationPlotItem::updateSortMap()
{
    _sortMap.clear();

    for(size_t i = 0U; i < _pluginInstance->numColumns(); i++)
        _sortMap.push_back(i);

    QCollator collator;
    collator.setNumericMode(true);

    switch(static_cast<PlotColumnSortType>(_columnSortType))
    {
    default:
    case PlotColumnSortType::Natural:
        return;

    case PlotColumnSortType::ColumnName:
    {
        std::sort(_sortMap.begin(), _sortMap.end(),
        [this, &collator](size_t a, size_t b)
        {
            return collator.compare(
                _pluginInstance->columnName(static_cast<int>(a)),
                _pluginInstance->columnName(static_cast<int>(b))) < 0;
        });

        break;
    }

    case PlotColumnSortType::ColumnAnnotation:
    {
        const auto& columnAnnotations = _pluginInstance->columnAnnotations();

        if(columnAnnotations.empty() || _columnSortAnnotation.isEmpty())
            return;

        auto it = std::find_if(columnAnnotations.begin(), columnAnnotations.end(),
            [this](const auto& v) { return v.name() == _columnSortAnnotation; });

        Q_ASSERT(it != columnAnnotations.end());
        if(it == columnAnnotations.end())
            return;

        const auto& columnAnnotation = *it;

        std::sort(_sortMap.begin(), _sortMap.end(),
        [&collator, &columnAnnotation](size_t a, size_t b)
        {
            return collator.compare(
                columnAnnotation.valueAt(static_cast<int>(a)),
                columnAnnotation.valueAt(static_cast<int>(b))) < 0;
        });

        break;
    }

    }
}

void CorrelationPlotItem::setColumnSortType(int columnSortType)
{
    if(_columnSortType != columnSortType)
    {
        _columnSortType = columnSortType;
        emit plotOptionsChanged();

        invalidateLineGraphCache();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setColumnSortAnnotation(const QString& columnSortAnnotation)
{
    if(_columnSortAnnotation != columnSortAnnotation)
    {
        _columnSortAnnotation = columnSortAnnotation;
        emit plotOptionsChanged();

        if(static_cast<PlotColumnSortType>(_columnSortType) == PlotColumnSortType::ColumnAnnotation)
        {
            invalidateLineGraphCache();
            rebuildPlot();
        }
    }
}

QString CorrelationPlotItem::elideLabel(const QString& label)
{
    auto cacheEntry = _labelElisionCache.constFind(label);
    if(cacheEntry != _labelElisionCache.constEnd())
    {
        if(cacheEntry->contains(_elideLabelWidth))
            return cacheEntry->value(_elideLabelWidth);
    }

    auto elidedLabel = _defaultFontMetrics.elidedText(label, Qt::ElideRight, _elideLabelWidth);

    _labelElisionCache[label].insert(_elideLabelWidth, elidedLabel);

    return elidedLabel;
}

QStringList CorrelationPlotItem::visibleColumnAnnotationNames() const
{
    QStringList list;
    list.reserve(static_cast<int>(_visibleColumnAnnotationNames.size()));

    for(const auto& columnAnnotaionName : _visibleColumnAnnotationNames)
        list.append(columnAnnotaionName);

    return list;
}

void CorrelationPlotItem::setVisibleColumnAnnotationNames(const QStringList& columnAnnotations)
{
    _visibleColumnAnnotationNames.clear();
    for(const auto& columnAnnotation : columnAnnotations)
        _visibleColumnAnnotationNames.emplace(columnAnnotation);

    emit plotOptionsChanged();
}

bool CorrelationPlotItem::columnAnnotationSelectionModeEnabled() const
{
    return _columnAnnotationSelectionModeEnabled;
}

void CorrelationPlotItem::setColumnAnnotationSelectionModeEnabled(bool enabled)
{
    // Don't set it if we can't enter selection mode
    if(enabled && !canShowColumnAnnotationSelection())
        return;

    if(_columnAnnotationSelectionModeEnabled != enabled)
    {
        _columnAnnotationSelectionModeEnabled = enabled;
        emit columnAnnotationSelectionModeEnabledChanged();

        rebuildPlot();
    }
}

size_t CorrelationPlotItem::numVisibleColumnAnnotations() const
{
    if(_columnAnnotationSelectionModeEnabled)
        return _pluginInstance->columnAnnotations().size();

    return _visibleColumnAnnotationNames.size();
}

QString CorrelationPlotItem::columnAnnotationValueAt(size_t x, size_t y) const
{
    const auto& columnAnnotations = _pluginInstance->columnAnnotations();

    std::vector<size_t> visibleRowIndices;

    size_t index = 0;
    for(const auto& columnAnnotation : columnAnnotations)
    {
        if(_columnAnnotationSelectionModeEnabled ||
            u::contains(_visibleColumnAnnotationNames, columnAnnotation.name()))
        {
            visibleRowIndices.push_back(index);
        }

        index++;
    }

    const auto& columnAnnotation = columnAnnotations.at(visibleRowIndices.at(y));

    return columnAnnotation.valueAt(static_cast<int>(_sortMap[x]));
}

double CorrelationPlotItem::visibleHorizontalFraction() const
{
    if(_pluginInstance == nullptr)
        return 1.0;

    auto f = (columnAxisWidth() / (minColumnWidth() * _pluginInstance->numColumns()));

    return std::min(f, 1.0);
}

double CorrelationPlotItem::labelHeight() const
{
    const unsigned int columnPadding = 1;
    return _defaultFontMetrics.height() + columnPadding;
}

const double minColumnPixelWidth = 1.0;

bool CorrelationPlotItem::isWide() const
{
    return (_pluginInstance->numColumns() * minColumnPixelWidth) > columnAxisWidth();
}

double CorrelationPlotItem::minColumnWidth() const
{
    if(_showColumnNames)
        return labelHeight();

    if(_showAllColumns)
        return columnAxisWidth() / _pluginInstance->numColumns();

    return minColumnPixelWidth;
}

double CorrelationPlotItem::columnAxisWidth() const
{
    const auto& margins = _mainAxisRect->margins();
    const unsigned int axisWidth = margins.left() + margins.right();

    //FIXME This value is wrong when the legend is enabled
    return width() - axisWidth;
}

double CorrelationPlotItem::columnAnnotaionsHeight(bool allAttributes) const
{
    if(allAttributes)
        return _pluginInstance->columnAnnotations().size() * labelHeight();

    return _visibleColumnAnnotationNames.size() * labelHeight();
}

void CorrelationPlotItem::updatePlotSize()
{
    computeXAxisRange();
    updateColumnAnnotationVisibility();
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
