/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <QDebug>

#include <cmath>
#include <algorithm>
#include <numeric>

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

void CorrelationPlotWorker::setShowGridLines(bool showGridLines)
{
    _showGridLines = showGridLines;
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
        {
            auto numVisibleColumns = (_xAxisMax - _xAxisMin);
            bool columnsAreDense = numVisibleColumns > (axisRect->width() * 0.3);

            auto* axis = axisRect->axis(QCPAxis::atBottom);
            axis->setRange(_xAxisMin, _xAxisMax);
            axis->grid()->setVisible(_showGridLines && !columnsAreDense);
            axis->setTicks(!columnsAreDense);
        }
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
    _customPlot.setSelectionTolerance(24);

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
    int yDest = static_cast<int>(height()) - _pixmap.height();

    if(!isEnabled())
    {
        // Create a desaturated version of the pixmap
        auto image = _pixmap.toImage();
        const auto bytes = image.depth() >> 3;

        for(int y = 0; y < image.height(); y++)
        {
            auto* scanLine = image.scanLine(y);
            for(int x = 0; x < image.width(); x++)
            {
                auto* pixel = reinterpret_cast<QRgb*>(scanLine + (x * bytes));
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
    if(_rebuildRequired != RebuildRequired::None)
    {
        rebuildPlot(_rebuildRequired == RebuildRequired::Full ?
            InvalidateCache::Yes : InvalidateCache::No);
    }

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
    auto* newEvent = new QWheelEvent(event->position(), event->globalPosition(), event->pixelDelta(),
                                     event->angleDelta(), event->buttons(), event->modifiers(), event->phase(),
                                     event->inverted());
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

    updateTooltip();
}

void CorrelationPlotItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

QCPAbstractPlottable* CorrelationPlotItem::abstractPlottableUnderCursor(double& keyCoord)
{
    QCPAbstractPlottable* nearestPlottable = nullptr;

    if(_hoverPoint.x() >= 0.0 && _hoverPoint.y() >= 0.0)
    {
        QVariant details;

        const auto& axisRectUnderCursor = _customPlot.axisRectAt(_hoverPoint);
        auto minDistanceSq = std::numeric_limits<double>::max();

        for(int index = 0; index < _customPlot.plottableCount(); index++)
        {
            auto* plottable = _customPlot.plottable(index);

            if(plottable == nullptr)
                continue;

            if(plottable->selectable() == QCP::SelectionType::stNone)
                continue;

            const auto* plottablKeyAxisRect = plottable->keyAxis()->axisRect();
            if(plottablKeyAxisRect != axisRectUnderCursor)
                continue;

            const auto rect = plottablKeyAxisRect->rect().intersected(
                plottable->valueAxis()->axisRect()->rect());

            if(!rect.contains(_hoverPoint.toPoint()))
                continue;

            const auto* graph = dynamic_cast<QCPGraph*>(plottable);
            if(graph != nullptr)
            {
                double posKeyMin = 0.0, posKeyMax = 0.0, dummy = 0.0;

                QPointF tolerancePoint(_customPlot.selectionTolerance(),
                    _customPlot.selectionTolerance());

                plottable->pixelsToCoords(_hoverPoint - tolerancePoint, posKeyMin, dummy);
                plottable->pixelsToCoords(_hoverPoint + tolerancePoint, posKeyMax, dummy);

                if(posKeyMin > posKeyMax)
                    std::swap(posKeyMin, posKeyMax);

                QCPGraphDataContainer::const_iterator begin = graph->data()->findBegin(posKeyMin, true);
                QCPGraphDataContainer::const_iterator end = graph->data()->findEnd(posKeyMax, true);

                for(QCPGraphDataContainer::const_iterator it = begin; it != end; ++it)
                {
                    const auto distanceSq = QCPVector2D(plottable->coordsToPixels(
                        it->key, it->value) - _hoverPoint).lengthSquared();

                    if(distanceSq < minDistanceSq)
                    {
                        minDistanceSq = distanceSq;
                        nearestPlottable = plottable;

                        keyCoord = std::distance(graph->data()->constBegin(), it);
                    }
                }
            }
            else
            {
                const auto distanceSq = plottable->selectTest(_hoverPoint, false, &details);

                if(distanceSq >= 0.0 && distanceSq < minDistanceSq)
                {
                    minDistanceSq = distanceSq;
                    nearestPlottable = plottable;

                    auto dataSelection = qvariant_cast<QCPDataSelection>(details);
                    keyCoord = dataSelection.dataRange().begin();
                }
            }
        }

        double toleranceSq = _customPlot.selectionTolerance() * _customPlot.selectionTolerance();
        if(minDistanceSq > toleranceSq)
            return nullptr;
    }

    return nearestPlottable;
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

    double xCoord = -1.0;

    if(_hoverPoint.x() >= 0.0 && _hoverPoint.y() >= 0.0)
    {
        axisRectUnderCursor = _customPlot.axisRectAt(_hoverPoint);
        plottableUnderCursor = abstractPlottableUnderCursor(xCoord);
    }

    bool showTooltip = false;
    _hoverLabel->setText(QString());
    _itemTracer->setGraph(nullptr);

    if(plottableUnderCursor != nullptr || axisRectUnderCursor != nullptr)
    {
        if(axisRectUnderCursor == _mainAxisRect && plottableUnderCursor != nullptr)
        {
            if(auto* graph = dynamic_cast<QCPGraph*>(plottableUnderCursor))
            {
                _itemTracer->setGraph(graph);
                _itemTracer->setGraphKey(xCoord);
                showTooltip = true;
            }
            else if(auto* bars = dynamic_cast<QCPBars*>(plottableUnderCursor))
            {
                _itemTracer->position->setPixelPosition(bars->dataPixelPosition(static_cast<int>(xCoord)));
                showTooltip = true;
            }
            else if(auto* boxPlot = dynamic_cast<QCPStatisticalBox*>(plottableUnderCursor))
            {
                // Only show simple tooltips for now, can extend this later...
                _itemTracer->position->setPixelPosition(boxPlot->dataPixelPosition(static_cast<int>(xCoord)));
                showTooltip = true;
            }
        }
        else if(axisRectUnderCursor == _columnAnnotationsAxisRect)
        {
            if(axisRectUnderCursor->rect().contains(_hoverPoint.toPoint()))
            {
                auto point = _hoverPoint - axisRectUnderCursor->topLeft();
                auto* bottomAxis = axisRectUnderCursor->axis(QCPAxis::atBottom);
                const auto& bottomRange = bottomAxis->range();
                auto bottomSize = bottomRange.size();
                auto xf = bottomRange.lower + 0.5 +
                        (static_cast<double>(point.x() * bottomSize) / axisRectUnderCursor->width());

                auto x = static_cast<int>(xf);
                int y = static_cast<int>((point.y() * numVisibleColumnAnnotations()) /
                    static_cast<double>(axisRectUnderCursor->height()));

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
                    .arg(u::formatNumberScientific(_itemTracer->position->value())));
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

QVector<double> CorrelationPlotItem::meanAverageData(double& min, double& max, const QVector<int>& rows)
{
    // Use Average Calculation
    QVector<double> yDataAvg; yDataAvg.reserve(rows.size());

    for(size_t col = 0; col < _pluginInstance->numColumns(); col++)
    {
        double runningTotal = std::accumulate(rows.begin(), rows.end(), 0.0,
        [this, col](auto partial, auto row)
        {
            return partial + _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));
        });

        yDataAvg.append(runningTotal / rows.length());

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

static QColor colorForRows(const CorrelationPluginInstance* pluginInstance, const QVector<int>& rows)
{
    if(rows.isEmpty())
        return {};

    auto color = pluginInstance->nodeColorForRow(rows.at(0));

    auto colorsInconsistent = std::any_of(rows.begin(), rows.end(),
    [&](auto row)
    {
        return pluginInstance->nodeColorForRow(row) != color;
    });

    if(colorsInconsistent)
    {
        // The colours are not consistent, so just use black
        color = Qt::black;
    }

    return color;
}

template<typename Fn>
void addPlotPerAttributeValue(const CorrelationPluginInstance* pluginInstance,
    const QString& nameTemplate, const QString& attributeName,
    const QVector<int>& selectedRows, Fn&& addPlotFn)
{
    QMap<QString, QVector<int>> map;
    for(auto selectedRow : selectedRows)
    {
        const auto value = pluginInstance->attributeValueFor(attributeName, selectedRow);
        map[value].append(selectedRow); // clazy:exclude=reserve-candidates
    }

    const auto keys = map.keys();
    for(const auto& value : keys)
    {
        const auto& rows = map.value(value);
        auto color = colorForRows(pluginInstance, rows);

        addPlotFn(color, nameTemplate.arg(attributeName, value), rows);
    }
}

void CorrelationPlotItem::populateMeanLinePlot()
{
    if(_selectedRows.isEmpty())
        return;

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    auto addMeanPlot =
    [this, &minY, &maxY](const QColor& color, const QString& name, const QVector<int>& rows)
    {
        auto* graph = _customPlot.addGraph();
        graph->setPen(QPen(color, 2.0, Qt::DashLine));
        graph->setName(name);

        QVector<double> xData(static_cast<int>(_pluginInstance->numColumns()));
        // xData is just the column indices
        std::iota(std::begin(xData), std::end(xData), 0);

        // Use Average Calculation and set min / max
        QVector<double> yDataAvg = meanAverageData(minY, maxY, rows);

        graph->setData(xData, yDataAvg, true);

        _meanPlots.append(graph);
        populateDispersion(graph, minY, maxY, rows, yDataAvg);
    };

    if(!_averagingAttributeName.isEmpty())
    {
        addPlotPerAttributeValue(_pluginInstance, tr("Mean average of %1: %2"),
            _averagingAttributeName, _selectedRows, addMeanPlot);
    }
    else
    {
        addMeanPlot(colorForRows(_pluginInstance, _selectedRows),
            tr("Mean average of selection"), _selectedRows);
    }

    setYAxisRange(minY, maxY);
}

void CorrelationPlotItem::populateMedianLinePlot()
{
    if(_selectedRows.isEmpty())
        return;

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    auto addMedianPlot =
    [this, &minY, &maxY](const QColor& color, const QString& name, const QVector<int>& rows)
    {
        auto* graph = _customPlot.addGraph();
        graph->setPen(QPen(color, 2.0, Qt::DashLine));
        graph->setName(name);

        QVector<double> xData(static_cast<int>(_pluginInstance->numColumns()));
        // xData is just the column indices
        std::iota(std::begin(xData), std::end(xData), 0);

        QVector<double> rowsEntries(rows.length());
        QVector<double> yDataAvg(static_cast<int>(_pluginInstance->numColumns()));

        for(int col = 0; col < static_cast<int>(_pluginInstance->numColumns()); col++)
        {
            rowsEntries.clear();
            std::transform(rows.begin(), rows.end(), std::back_inserter(rowsEntries),
            [this, col](auto row)
            {
                return _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));
            });

            if(!rows.empty())
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

        _meanPlots.append(graph);
        populateDispersion(graph, minY, maxY, rows, yDataAvg);
    };

    if(!_averagingAttributeName.isEmpty())
    {
        addPlotPerAttributeValue(_pluginInstance, tr("Median average of %1: %2"),
            _averagingAttributeName, _selectedRows, addMedianPlot);
    }
    else
    {
        addMedianPlot(colorForRows(_pluginInstance, _selectedRows),
            tr("Median average of selection"), _selectedRows);
    }

    setYAxisRange(minY, maxY);
}

void CorrelationPlotItem::populateMeanHistogramPlot()
{
    if(_selectedRows.isEmpty())
        return;

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    auto addMeanBars =
    [this, &minY, &maxY](const QColor& color, const QString& name, const QVector<int>& rows)
    {
        QVector<double> xData(static_cast<int>(_pluginInstance->numColumns()));
        // xData is just the column indices
        std::iota(std::begin(xData), std::end(xData), 0);

        // Use Average Calculation and set min / max
        QVector<double> yDataAvg = meanAverageData(minY, maxY, rows);

        auto* histogramBars = new QCPBars(_mainXAxis, _mainYAxis);
        histogramBars->setName(name);
        histogramBars->setData(xData, yDataAvg, true);
        histogramBars->setPen(QPen(color.darker(150)));

        auto innerColor = color.lighter(110);

        // If the value is 0, i.e. the color is black, .lighter won't have
        // had any effect, so just pick an arbitrary higher value
        if(innerColor.value() == 0)
            innerColor.setHsv(innerColor.hue(), innerColor.saturation(), 92);

        histogramBars->setBrush(innerColor);

        setYAxisRange(minY, maxY);

        _meanPlots.append(histogramBars);
        populateDispersion(histogramBars, minY, maxY, rows, yDataAvg);
    };

    if(!_averagingAttributeName.isEmpty())
    {
        addPlotPerAttributeValue(_pluginInstance, tr("Median histogram of %1: %2"),
            _averagingAttributeName, _selectedRows, addMeanBars);

        auto* barsGroup = new QCPBarsGroup(&_customPlot);
        barsGroup->setSpacingType(QCPBarsGroup::stAbsolute);
        barsGroup->setSpacing(1.0);

        for(auto* plottable : std::as_const(_meanPlots))
        {
            auto* bars = dynamic_cast<QCPBars*>(plottable);
            bars->setWidth(bars->width() / _meanPlots.size());
            barsGroup->append(bars);
        }
    }
    else
    {
        addMeanBars(colorForRows(_pluginInstance, _selectedRows),
            tr("Mean histogram of selection"), _selectedRows);
    }

    setYAxisRange(minY, maxY);
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
        const auto& selectedRows = std::as_const(_selectedRows);
        std::transform(selectedRows.begin(), selectedRows.end(), std::back_inserter(rowsEntries),
        [this, col](auto row)
        {
            return _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));
        });

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

            for(auto row : std::as_const(rowsEntries))
            {
                // Find Maximum and minimum non-outliers
                if(row < thirdQuartile + (iqr * 1.5))
                    maxValue = std::max(maxValue, row);

                if(row > firstQuartile - (iqr * 1.5))
                    minValue = std::min(minValue, row);

                // Find outliers
                if(row > thirdQuartile + (iqr * 1.5) ||
                    row < firstQuartile - (iqr * 1.5))
                {
                    outliers.push_back(row);
                }

                maxY = std::max(maxY, row);
                minY = std::min(minY, row);
            }

            // Add data for each column individually because setData doesn't let us do outliers(??)
            statPlot->addData(col, minValue, firstQuartile, secondQuartile, thirdQuartile,
                              maxValue, outliers);
        }
    }

    setYAxisRange(minY, maxY);
}

void CorrelationPlotItem::plotDispersion(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<double>& stdDevs, const QString& name = QStringLiteral("Deviation"))
{
    auto visualType = static_cast<PlotDispersionVisualType>(_dispersionVisualType);
    if(visualType == PlotDispersionVisualType::Bars)
    {
        auto* stdDevBars = new QCPErrorBars(_mainXAxis, _mainYAxis);
        stdDevBars->setName(name);
        stdDevBars->setSelectable(QCP::SelectionType::stNone);
        stdDevBars->setAntialiased(false);
        stdDevBars->setDataPlottable(meanPlot);
        stdDevBars->setData(stdDevs);
    }
    else if(visualType == PlotDispersionVisualType::Area)
    {
        auto* devTop = new QCPGraph(_mainXAxis, _mainYAxis);
        auto* devBottom = new QCPGraph(_mainXAxis, _mainYAxis);
        devTop->setName(QStringLiteral("%1 Top").arg(name));
        devBottom->setName(QStringLiteral("%1 Bottom").arg(name));

        auto fillColour = meanPlot->pen().color();
        auto penColour = meanPlot->pen().color().lighter(150);
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
            topErr[i] = meanPlot->interface1D()->dataMainValue(i) + stdDevs[i];
            bottomErr[i] = meanPlot->interface1D()->dataMainValue(i) - stdDevs[i];
        }

        // xData is just the column indices
        QVector<double> xData(static_cast<int>(_pluginInstance->numColumns()));
        std::iota(std::begin(xData), std::end(xData), 0);

        devTop->setData(xData, topErr);
        devBottom->setData(xData, bottomErr);
    }

    for(int i = 0; i < static_cast<int>(_pluginInstance->numColumns()); i++)
    {
        minY = std::min(minY, meanPlot->interface1D()->dataMainValue(i) - stdDevs[i]);
        maxY = std::max(maxY, meanPlot->interface1D()->dataMainValue(i) + stdDevs[i]);
    }
}

void CorrelationPlotItem::populateStdDevPlot(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<int>& rows, QVector<double>& means)
{
    QVector<double> stdDevs(static_cast<int>(_pluginInstance->numColumns()));

    for(int col = 0; col < static_cast<int>(_pluginInstance->numColumns()); col++)
    {
        double stdDev = 0.0;
        for(auto row : rows)
        {
            auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col])) - means.at(col);
            stdDev += (value * value);
        }

        stdDev /= _pluginInstance->numColumns();
        stdDev = std::sqrt(stdDev);
        stdDevs[col] = stdDev;
    }

    plotDispersion(meanPlot, minY, maxY, stdDevs, QStringLiteral("Std Dev"));
}

void CorrelationPlotItem::populateStdErrorPlot(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<int>& rows, QVector<double>& means)
{
    QVector<double> stdErrs(static_cast<int>(_pluginInstance->numColumns()));

    for(int col = 0; col < static_cast<int>(_pluginInstance->numColumns()); col++)
    {
        double stdErr = 0.0;
        for(auto row : rows)
        {
            auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col])) - means.at(col);
            stdErr += (value * value);
        }

        stdErr /= _pluginInstance->numColumns();
        stdErr = std::sqrt(stdErr) / std::sqrt(static_cast<double>(rows.length()));
        stdErrs[col] = stdErr;
    }

    plotDispersion(meanPlot, minY, maxY, stdErrs, QStringLiteral("Std Err"));
}

void CorrelationPlotItem::populateDispersion(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<int>& rows, QVector<double>& means)
{
    auto averagingType = static_cast<PlotAveragingType>(_averagingType);
    auto dispersionType = static_cast<PlotDispersionType>(_dispersionType);

    if(averagingType == PlotAveragingType::Individual || averagingType == PlotAveragingType::IQRPlot)
        return;

    if(dispersionType == PlotDispersionType::StdDev)
        populateStdDevPlot(meanPlot, minY, maxY, rows, means);
    else if(dispersionType == PlotDispersionType::StdErr)
        populateStdErrorPlot(meanPlot, minY, maxY, rows, means);
}

void CorrelationPlotItem::populateLinePlot()
{
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    QVector<double> yData; yData.reserve(_selectedRows.size());
    QVector<double> xData; xData.reserve(static_cast<int>(_pluginInstance->numColumns()));

    // Plot each row individually
    for(auto row : std::as_const(_selectedRows))
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
                rowSum += _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));

            double rowMean = rowSum / _pluginInstance->numColumns();

            double variance = 0.0;
            for(size_t col = 0; col < _pluginInstance->numColumns(); col++)
            {
                auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col])) - rowMean;
                variance += (value * value);
            }

            variance /= _pluginInstance->numColumns();
            double stdDev = std::sqrt(variance);
            double pareto = std::sqrt(stdDev);

            double attributeValue = 1.0;

            if(static_cast<PlotScaleType>(_scaleType) == PlotScaleType::ByAttribute && !_scaleByAttributeName.isEmpty())
            {
                attributeValue = u::toNumber(_pluginInstance->attributeValueFor(_scaleByAttributeName, row));

                if(attributeValue == 0.0 || !std::isfinite(attributeValue))
                    attributeValue = 1.0;
            }

            yData.clear();
            xData.clear();

            for(size_t col = 0; col < _pluginInstance->numColumns(); col++)
            {
                auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));

                switch(static_cast<PlotScaleType>(_scaleType))
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
                case PlotScaleType::ByAttribute:
                    value /= attributeValue;
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

    auto* caXAxis = _columnAnnotationsAxisRect->axis(QCPAxis::atBottom);
    auto* caYAxis = _columnAnnotationsAxisRect->axis(QCPAxis::atLeft);

    auto h = columnAnnotaionsHeight(_columnAnnotationSelectionModeEnabled);
    _columnAnnotationsAxisRect->setMinimumSize(0, h);
    _columnAnnotationsAxisRect->setMaximumSize(QWIDGETSIZE_MAX, h);

    QSharedPointer<QCPAxisTickerText> columnAnnotationTicker(new QCPAxisTickerText);
    size_t y = numColumnAnnotations - 1;
    size_t offset = 0;

    auto* qcpColumnAnnotations = new QCPColumnAnnotations(caXAxis, caYAxis);

    for(const auto& columnAnnotation : columnAnnotations)
    {
        auto selected = u::contains(_visibleColumnAnnotationNames, columnAnnotation.name());
        bool visible = selected || _columnAnnotationSelectionModeEnabled;

        if(visible)
        {
            qcpColumnAnnotations->setData(y, _sortMap, selected, offset, &columnAnnotation);

            QString prefix;
            QString postfix;

            if(!_columnSortOrders.empty())
            {
                const auto& columnSortOrder = _columnSortOrders.first();
                auto type = static_cast<PlotColumnSortType>(columnSortOrder["type"].toInt());
                auto text = columnSortOrder["text"].toString();
                auto order = static_cast<Qt::SortOrder>(columnSortOrder["order"].toInt());

                if(type == PlotColumnSortType::ColumnAnnotation && text == columnAnnotation.name())
                {
                    prefix += order == Qt::AscendingOrder ?
                        QStringLiteral(u"▻ ") : QStringLiteral(u"◅ ");
                }
            }

            if(_columnAnnotationSelectionModeEnabled)
                postfix += selected ? QStringLiteral(u" ☑") : QStringLiteral(u" ☐");

            double tickPosition = static_cast<double>(y) + 0.5;
            columnAnnotationTicker->addTick(tickPosition, prefix + columnAnnotation.name() + postfix);

            y--;
        }

        offset++;
    }

    caYAxis->setTickPen(QPen(Qt::transparent)); // NOLINT clang-analyzer-cplusplus.NewDeleteLeaks
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
    const auto* axisRect = _customPlot.axisRectAt(pos);

    if(axisRect == nullptr)
        return;

    auto rectHeight = axisRect->bottom() - axisRect->top();
    auto x = pos.x() - axisRect->left();
    auto y = pos.y() - axisRect->top();

    if(y >= rectHeight)
    {
        // Click is below either/both axes
        sortBy(static_cast<int>(PlotColumnSortType::ColumnName));
        return;
    }

    if(axisRect != _columnAnnotationsAxisRect)
    {
        if(x < 0)
        {
            // Click is to the left of the main axis
            sortBy(static_cast<int>(PlotColumnSortType::Natural));
        }

        return;
    }

    const auto& columnAnnotations = _pluginInstance->columnAnnotations();
    auto index = (y * numVisibleColumnAnnotations()) / (rectHeight);
    if(index >= columnAnnotations.size())
        return;

    std::vector<QString> annotationNames;
    std::transform(columnAnnotations.begin(), columnAnnotations.end(),
        std::back_inserter(annotationNames), [](const auto& v) { return v.name(); });

    if(!_columnAnnotationSelectionModeEnabled)
    {
        // Remove any annotations not currently visible, so
        // that looking up by index works
        annotationNames.erase(std::remove_if(annotationNames.begin(), annotationNames.end(),
        [this](const auto& v)
        {
            return !u::contains(_visibleColumnAnnotationNames, v);
        }));
    }

    const auto& name = annotationNames.at(index);

    if(_columnAnnotationSelectionModeEnabled && x < 0)
    {
        // Click is on the annotation name itself (with checkbox)
        if(u::contains(_visibleColumnAnnotationNames, name))
            _visibleColumnAnnotationNames.erase(name);
        else
            _visibleColumnAnnotationNames.insert(name);
    }
    else if(_columnAnnotationSelectionModeEnabled &&
        !u::contains(_visibleColumnAnnotationNames, name))
    {
        // Clicking anywhere else enables a column annotation
        // when it's disabled...
        _visibleColumnAnnotationNames.insert(name);
    }
    else
    {
        // ...or selects it as the sort annotation otherwise
        sortBy(static_cast<int>(PlotColumnSortType::ColumnAnnotation), name);
        return;
    }

    emit plotOptionsChanged();
    rebuildPlot();
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

void CorrelationPlotItem::rebuildPlot(InvalidateCache invalidateCache)
{
    std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);

    if(!lock.owns_lock())
    {
        if(invalidateCache == InvalidateCache::Yes)
            _rebuildRequired = RebuildRequired::Full;
        else if(_rebuildRequired != RebuildRequired::Full)
            _rebuildRequired = RebuildRequired::Partial;

        return;
    }
    else
        _rebuildRequired = RebuildRequired::None;

    if(invalidateCache == InvalidateCache::Yes)
    {
        // Invalidate the line graph cache
        for(auto v : std::as_const(_lineGraphCache))
            _customPlot.removeGraph(v._graph);

        _lineGraphCache.clear();
    }

    QElapsedTimer buildTimer;
    buildTimer.start();

    updateSortMap();

    for(auto v : std::as_const(_lineGraphCache))
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

    _meanPlots.clear();
    _columnAnnotationsAxisRect = nullptr;

    // Return the plot layout to its immediate post-construction state
    removeAllExcept(_mainAxisLayout, _mainAxisRect);
    removeAllExcept(_customPlot.plotLayout(), _mainAxisLayout);

    auto plotAveragingType = static_cast<PlotAveragingType>(_averagingType);
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

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);

    auto* xAxis = _mainXAxis;

    xAxis->setLabel({});
    xAxis->setTicker(categoryTicker);

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

void CorrelationPlotItem::setDispersionVisualType(int dispersionVisualType)
{
    if(_dispersionVisualType != dispersionVisualType)
    {
        _dispersionVisualType = dispersionVisualType;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setYAxisLabel(const QString& plotYAxisLabel)
{
    if(_yAxisLabel != plotYAxisLabel)
    {
        _yAxisLabel = plotYAxisLabel;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setIncludeYZero(bool includeYZero)
{
    if(_includeYZero != includeYZero)
    {
        _includeYZero = includeYZero;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setShowAllColumns(bool showAllColumns)
{
    if(_showAllColumns != showAllColumns)
    {
        _showAllColumns = showAllColumns;
        emit visibleHorizontalFractionChanged();
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setXAxisLabel(const QString& plotXAxisLabel)
{
    if(_xAxisLabel != plotXAxisLabel)
    {
        _xAxisLabel = plotXAxisLabel;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setScaleType(int scaleType)
{
    if(_scaleType != scaleType)
    {
        _scaleType = scaleType;
        emit plotOptionsChanged();
        rebuildPlot(InvalidateCache::Yes);
    }
}

void CorrelationPlotItem::setScaleByAttributeName(const QString& attributeName)
{
    if(_scaleByAttributeName != attributeName || _scaleType != static_cast<int>(PlotScaleType::ByAttribute))
    {
        _scaleByAttributeName = attributeName;
        _scaleType = static_cast<int>(PlotScaleType::ByAttribute);
        emit plotOptionsChanged();
        rebuildPlot(InvalidateCache::Yes);
    }
}

void CorrelationPlotItem::setAveragingType(int averagingType)
{
    if(_averagingType != averagingType)
    {
        _averagingType = averagingType;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setAveragingAttributeName(const QString& attributeName)
{
    if(_averagingAttributeName != attributeName)
    {
        _averagingAttributeName = attributeName;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setDispersionType(int dispersionType)
{
    if(_dispersionType != dispersionType)
    {
        _dispersionType = dispersionType;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setShowLegend(bool showLegend)
{
    if(_showLegend != showLegend)
    {
        _showLegend = showLegend;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setPluginInstance(CorrelationPluginInstance* pluginInstance)
{
    _pluginInstance = pluginInstance;

    connect(_pluginInstance, &CorrelationPluginInstance::nodeColorsChanged,
        this, [this] { rebuildPlot(); });
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
    if(_showGridLines != showGridLines)
    {
        _showGridLines = showGridLines;
        QMetaObject::invokeMethod(_worker, "setShowGridLines", Q_ARG(bool, showGridLines));
        rebuildPlot();
    }
}

void CorrelationPlotItem::setHorizontalScrollPosition(double horizontalScrollPosition)
{
    _horizontalScrollPosition = std::clamp(horizontalScrollPosition, 0.0, 1.0);
    computeXAxisRange();

    updatePixmap(CorrelationPlotUpdateType::Render);
}

void CorrelationPlotItem::setXAxisPadding(int padding)
{
    if(_xAxisPadding != padding)
    {
        _xAxisPadding = padding;
        rebuildPlot();
    }
}

void CorrelationPlotItem::updateSortMap()
{
    _sortMap.clear();

    for(size_t i = 0U; i < _pluginInstance->numColumns(); i++)
        _sortMap.push_back(i);

    QCollator collator;
    collator.setNumericMode(true);

    // Convert the javascript/QML object array into something
    // efficient to access from inside the sort lambda function
    struct ColumnSortOrder
    {
        PlotColumnSortType _type = PlotColumnSortType::Natural;
        Qt::SortOrder _order = Qt::AscendingOrder;
        const ColumnAnnotation* _annotation = nullptr;
    };

    std::vector<ColumnSortOrder> columnSortOrders;
    columnSortOrders.reserve(_columnSortOrders.size());

    for(const auto& qmlColumnSortOrder : _columnSortOrders)
    {
        Q_ASSERT(u::containsAllOf(qmlColumnSortOrder, {"type", "text", "order"}));

        ColumnSortOrder columnSortOrder;
        columnSortOrder._type = static_cast<PlotColumnSortType>(qmlColumnSortOrder["type"].toInt());
        columnSortOrder._order = static_cast<Qt::SortOrder>(qmlColumnSortOrder["order"].toInt());

        if(columnSortOrder._type == PlotColumnSortType::ColumnAnnotation)
        {
            const auto& columnAnnotations = _pluginInstance->columnAnnotations();
            auto columnAnnotationName = qmlColumnSortOrder["text"].toString();
            auto it = std::find_if(columnAnnotations.begin(), columnAnnotations.end(),
                [&columnAnnotationName](const auto& v) { return v.name() == columnAnnotationName; });

            Q_ASSERT(it != columnAnnotations.end());
            if(it != columnAnnotations.end())
                columnSortOrder._annotation = &(*it);
        }

        columnSortOrders.emplace_back(columnSortOrder);
    }

    std::sort(_sortMap.begin(), _sortMap.end(),
    [this, &columnSortOrders, &collator](size_t a, size_t b)
    {
        for(const auto& columnSortOrder : columnSortOrders)
        {
            switch(columnSortOrder._type)
            {
            default:
            case PlotColumnSortType::Natural:
                return columnSortOrder._order == Qt::AscendingOrder ?
                    a < b : b < a;

            case PlotColumnSortType::ColumnName:
            {
                auto columnNameA = _pluginInstance->columnName(static_cast<int>(a));
                auto columnNameB = _pluginInstance->columnName(static_cast<int>(b));

                if(columnNameA == columnNameB)
                    continue;

                return columnSortOrder._order == Qt::AscendingOrder ?
                    collator.compare(columnNameA, columnNameB) < 0 :
                    collator.compare(columnNameB, columnNameA) < 0;
            }

            case PlotColumnSortType::ColumnAnnotation:
            {
                auto annotationValueA = columnSortOrder._annotation->valueAt(static_cast<int>(a));
                auto annotationValueB = columnSortOrder._annotation->valueAt(static_cast<int>(b));

                if(annotationValueA == annotationValueB)
                    continue;

                return columnSortOrder._order == Qt::AscendingOrder ?
                    collator.compare(annotationValueA, annotationValueB) < 0 :
                    collator.compare(annotationValueB, annotationValueA) < 0;
            }

            }
        }

        // If all else fails, just use natural order
        return a < b;
    });
}

void CorrelationPlotItem::sortBy(int type, const QString& text)
{
    auto order = Qt::AscendingOrder;

    auto existing = std::find_if(_columnSortOrders.begin(), _columnSortOrders.end(),
    [type, &text](const auto& value)
    {
        bool sameType = (value["type"].toInt() == type);
        bool sameText = (value["text"].toString() == text);
        bool typeIsColumnAnnotation =
            (type == static_cast<int>(PlotColumnSortType::ColumnAnnotation));

        return sameType && (!typeIsColumnAnnotation || sameText);
    });

    // If the column has been sorted on before, remove it so
    // that adding it brings it to the front
    if(existing != _columnSortOrders.end())
    {
        order = static_cast<Qt::SortOrder>((*existing)["order"].toInt());

        if(existing == _columnSortOrders.begin())
        {
            // If the thing we're sorting is on the front
            // of the list already, flip the sort order
            order = order == Qt::AscendingOrder ?
                Qt::DescendingOrder : Qt::AscendingOrder;
        }

        _columnSortOrders.erase(existing);
    }

    QVariantMap newSortOrder;
    newSortOrder["type"] = type;
    newSortOrder["text"] = text;
    newSortOrder["order"] = order;

    _columnSortOrders.push_front(newSortOrder);

    emit plotOptionsChanged();
    rebuildPlot(InvalidateCache::Yes);
}

void CorrelationPlotItem::setColumnSortOrders(const QVector<QVariantMap> columnSortOrders)
{
    if(_columnSortOrders != columnSortOrders)
    {
        _columnSortOrders = columnSortOrders;
        emit plotOptionsChanged();

        rebuildPlot(InvalidateCache::Yes);
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
    const unsigned int marginWidth = margins.left() + margins.right();

    //FIXME This value is wrong when the legend is enabled
    return width() - marginWidth;
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
