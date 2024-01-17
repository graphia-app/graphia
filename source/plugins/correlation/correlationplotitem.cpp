/* Copyright © 2013-2023 Graphia Technologies Ltd.
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
#include "correlationplotsaveimagecommand.h"

#include "qcpcolumnannotations.h"

#include "shared/utils/scope_exit.h"
#include "shared/utils/thread.h"
#include "shared/utils/utils.h"
#include "shared/utils/random.h"
#include "shared/utils/color.h"
#include "shared/utils/container.h"
#include "shared/utils/string.h"
#include "shared/utils/flags.h"
#include "shared/utils/fatalerror.h"

#include <QDesktopServices>
#include <QSet>
#include <QCollator>
#include <QQuickWindow>
#include <QDebug>

#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>
#include <map>

using namespace Qt::Literals::StringLiterals;

CorrelationPlotWorker::CorrelationPlotWorker(std::recursive_mutex& mutex,
    QCustomPlot& customPlot) :
    _debug(qEnvironmentVariableIntValue("QCUSTOMPLOT_DEBUG") != 0),
    _mutex(&mutex), _busy(false),
    _customPlot(&customPlot),
    _surface(new QOffscreenSurface)

{
    // Qt requires that this is created on the UI thread, so we do so here
    // Note QCustomPlot takes ownership of the memory, so it does not need
    // to be deleted
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

bool CorrelationPlotWorker::zoomed() const
{
    return _zoomed;
}

void CorrelationPlotWorker::updateZoomed()
{
    auto zoomed = std::any_of(_axisParameters.begin(), _axisParameters.end(), [](const auto& v)
    {
        return v.second.zoomed();
    });

    if(_zoomed != zoomed)
    {
        _zoomed = zoomed;
        emit zoomedChanged();
    }
}

void CorrelationPlotWorker::setShowGridLines(bool showGridLines)
{
    _showGridLines = showGridLines;
}

void CorrelationPlotWorker::setDevicePixelRatio(double devicePixelRatio)
{
    _devicePixelRatio = devicePixelRatio;
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

void CorrelationPlotWorker::setAxisRange(QCPAxis* axis, double min, double max)
{
    auto& parameters = _axisParameters[axis];

    if(!parameters.zoomed())
    {
        parameters._zoomedMin = min;
        parameters._zoomedMax = max;
    }
    else
    {
        parameters._zoomedMin = std::max(min, parameters._zoomedMin);
        parameters._zoomedMax = std::min(max, parameters._zoomedMax);
    }

    parameters._min = min;
    parameters._max = max;

    updateZoomed();
}

void CorrelationPlotWorker::zoom(QCPAxis* axis, double centre, int direction)
{
    constexpr double maxScale = 50.0;
    constexpr double zoomStepFactor = 0.8;

    auto& parameters = _axisParameters[axis];

    // Can't zoom if no range has been established
    if(parameters._min > parameters._max)
        return;

    auto zoomFactor = zoomStepFactor;
    if(direction < 0)
        zoomFactor = 1.0 / zoomFactor;

    centre = parameters._zoomedMin + ((parameters._zoomedMax - parameters._zoomedMin) * centre);
    centre = std::clamp(centre, parameters._min, parameters._max);

    auto newZoomedMin = ((parameters._zoomedMin - centre) * zoomFactor) + centre;
    auto newZoomedMax = ((parameters._zoomedMax - centre) * zoomFactor) + centre;

    if((parameters._max - parameters._min) / (newZoomedMax - newZoomedMin) > maxScale)
        return;

    parameters._zoomedMin = std::max(parameters._min, newZoomedMin);
    parameters._zoomedMax = std::min(parameters._max, newZoomedMax);

    updateZoomed();
}

void CorrelationPlotWorker::resetZoom()
{
    for(auto& [axis, parameters] : _axisParameters)
    {
        parameters._zoomedMin = parameters._min;
        parameters._zoomedMax = parameters._max;
    }

    updateZoomed();
}

void CorrelationPlotWorker::pan(QCPAxis* axis, double delta)
{
    auto& parameters = _axisParameters[axis];

    if(delta >= 0.0)
        delta = std::min(delta, parameters._max - parameters._zoomedMax);
    else
        delta = std::max(delta, parameters._min - parameters._zoomedMin);

    parameters._zoomedMin = std::max(parameters._min, parameters._zoomedMin + delta);
    parameters._zoomedMax = std::min(parameters._max, parameters._zoomedMax + delta);

    updateZoomed();
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

void CorrelationPlotWorker::clone(CorrelationPlotWorker& target) const
{
    target._devicePixelRatio    = _devicePixelRatio;
    target._width               = _width;
    target._height              = _height;
    target._xAxisMin            = _xAxisMin;
    target._xAxisMax            = _xAxisMax;
    target._showGridLines       = _showGridLines;
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
    if(_threadId == std::thread::id())
    {
        _customPlot->setOpenGl(true, _surface->format().samples(), _surface);

        // We don't need to use the surface any more, so give up access to it
        _surface = nullptr;

        u::setCurrentThreadName(u"CorrPlotRender"_s);
        _threadId = std::this_thread::get_id();
    }

    if(_threadId != std::this_thread::get_id())
        FATAL_ERROR(CorrPlotOnWrongThread);

    _customPlot->setGeometry(0, 0, _width, _height);

    // Since QCustomPlot is a QWidget, it is never technically visible, so never generates
    // a resizeEvent, so its viewport never gets set, so we must do so manually
    _customPlot->setViewport(_customPlot->geometry());

    // Force a layout so that any QCPAxisRects' dimensions are correct, before
    // calculating if the columns are considered to be dense
    _customPlot->plotLayout()->update(QCPLayoutElement::upLayout);

    const auto elements = _customPlot->plotLayout()->elements(true);
    for(const auto* element : elements)
    {
        const auto* axisRect = dynamic_cast<const QCPAxisRect*>(element);
        if(axisRect != nullptr)
        {
            auto numVisibleColumns = (_xAxisMax - _xAxisMin);
            const bool columnsAreDense = numVisibleColumns > (axisRect->width() * 0.15);

            auto* xAxis = axisRect->axis(QCPAxis::atBottom);
            xAxis->setRange(_xAxisMin, _xAxisMax);
            xAxis->grid()->setVisible(_showGridLines && !columnsAreDense &&
                xAxis->basePen() != QPen(Qt::transparent));
            xAxis->setTicks(!columnsAreDense);
            xAxis->setSubTicks(false);
        }
    }

    for(auto& [axis, parameters] : _axisParameters)
        axis->setRange(parameters._zoomedMin, parameters._zoomedMax);

    auto* tooltipLayer = _customPlot->layer(u"tooltipLayer"_s);
    if(tooltipLayer != nullptr && _updateType >= CorrelationPlotUpdateType::RenderAndTooltips)
        tooltipLayer->replot();

    if(_updateType >= CorrelationPlotUpdateType::ReplotAndRenderAndTooltips)
        _customPlot->replot(QCustomPlot::rpImmediateRefresh);

    _updateType = CorrelationPlotUpdateType::None;

    QElapsedTimer _pixmapTimer;
    _pixmapTimer.start();

    // This assumes that the Qt platform has the QPlatformIntegration::ThreadedPixmaps capability,
    // which should be true on the desktop, but a console warning will be shown if it isn't
    QPixmap pixmap(
        static_cast<int>(_width * _devicePixelRatio),
        static_cast<int>(_height * _devicePixelRatio));

    pixmap.fill(Qt::transparent);
    QCPPainter painter(&pixmap);
    painter.scale(_devicePixelRatio, _devicePixelRatio);
    _customPlot->toPainter(&painter);

    if(_debug)
        qDebug() << "render" << _pixmapTimer.elapsed() << "ms";

    // Ensure lock is released before receivers of pixmapUpdated are notified
    lock.unlock();

    emit pixmapUpdated(pixmap);
}

CorrelationPlotItem::CorrelationPlotItem(QQuickItem* parent) :
    QQuickPaintedItem(parent),
    _debug(qEnvironmentVariableIntValue("QCUSTOMPLOT_DEBUG") != 0),
    _mainLayoutGrid(new QCPLayoutGrid),
    _worker(new CorrelationPlotWorker(_mutex, _customPlot))
{
    // Discard the defaults...
    _customPlot.plotLayout()->clear();

    // ...and manage our own layout
    _customPlot.plotLayout()->addElement(_mainLayoutGrid);

    _customPlot.setAutoAddPlottableToLegend(false);

    _defaultFont9Pt.setPointSize(9);

    _customPlot.setSelectionTolerance(24);
    _customPlot.setBackground(Qt::transparent);

    // This is the default setting, and should never be set
    // to anything else since using the QCustomPlot interaction
    // mechanisms can cause replots outside of the render thread
    _customPlot.setInteractions(QCP::iNone);

    setFlag(QQuickItem::ItemHasContents, true);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    qRegisterMetaType<CorrelationPlotUpdateType>("CorrelationPlotUpdateType");

    _worker->moveToThread(&_plotRenderThread);
    connect(&_plotRenderThread, &QThread::finished, _worker, &QObject::deleteLater);

    connect(this, &QQuickPaintedItem::widthChanged, [this]
    {
        QMetaObject::invokeMethod(_worker, "setWidth", Qt::QueuedConnection,
            Q_ARG(int, static_cast<int>(width())));
    });

    connect(this, &QQuickPaintedItem::heightChanged, [this]
    {
        QMetaObject::invokeMethod(_worker, "setHeight", Qt::QueuedConnection,
            Q_ARG(int, std::max(static_cast<int>(height()), static_cast<int>(minimumHeight()))));
    });

    connect(this, &QQuickPaintedItem::windowChanged, [this]
    {
        if(window() == nullptr)
            return;

        auto devicePixelRatio = window()->devicePixelRatio();
        QMetaObject::invokeMethod(_worker, "setDevicePixelRatio", Qt::QueuedConnection,
            Q_ARG(double, devicePixelRatio));
    });

    connect(_worker, &CorrelationPlotWorker::pixmapUpdated, this, &CorrelationPlotItem::onPixmapUpdated);
    connect(_worker, &CorrelationPlotWorker::pixmapUpdated, this, &CorrelationPlotItem::pixmapUpdated);
    connect(this, &CorrelationPlotItem::enabledChanged, [this] { update(); });
    connect(_worker, &CorrelationPlotWorker::busyChanged, this, &CorrelationPlotItem::busyChanged);
    connect(_worker, &CorrelationPlotWorker::zoomedChanged, this, &CorrelationPlotItem::zoomedChanged);

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

    connect(this, &CorrelationPlotItem::numVisibleColumnsChanged, this, &CorrelationPlotItem::visibleHorizontalFractionChanged);

    connect(this, &CorrelationPlotItem::visibleColumnAnnotationNamesChanged, this, &CorrelationPlotItem::minimumHeightChanged);
    connect(this, &CorrelationPlotItem::plotModeChanged, this, &CorrelationPlotItem::minimumHeightChanged);
    connect(this, &CorrelationPlotItem::columnSortOrderPinnedChanged, [this] { if(!_columnSortOrderPinned) rebuildPlot(); });
}

CorrelationPlotItem::~CorrelationPlotItem() // NOLINT modernize-use-equals-default
{
    _plotRenderThread.quit();
    _plotRenderThread.wait();
}

void CorrelationPlotItem::updatePixmap(CorrelationPlotUpdateType updateType)
{
    QMetaObject::invokeMethod(_worker, "updatePixmap", Qt::QueuedConnection,
        Q_ARG(CorrelationPlotUpdateType, updateType));
}

bool CorrelationPlotItem::event(QEvent* event)
{
    if(event->type() == QEvent::ApplicationPaletteChange)
        rebuildPlot();

    return QQuickPaintedItem::event(event);
}

void CorrelationPlotItem::paint(QPainter* painter)
{
    if(_pixmap.isNull())
        return;

    auto devicePixelRatio = window()->devicePixelRatio();

    // Render the plot in the bottom left; that way when its container
    // is resized, it doesn't hop around vertically, as it would if
    // it had been rendered from the top left
    const int yOffset = static_cast<int>(height() - (_pixmap.height() / devicePixelRatio));

    const QRect target
    {
        0,
        yOffset,
        static_cast<int>(width()),
        static_cast<int>(height()) - yOffset
    };

    const QRect source
    {
        0, 0,
        static_cast<int>(width() * devicePixelRatio),
        static_cast<int>((height() - yOffset) * devicePixelRatio)
    };

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
                auto* pixel = reinterpret_cast<QRgb*>(scanLine + static_cast<ptrdiff_t>(x * bytes));
                const int gray = qGray(*pixel);
                const int alpha = qAlpha(*pixel);
                *pixel = QColor(gray, gray, gray, alpha).rgba();
            }
        }

        // The pixmap that QCustomPlot creates is a mixture of premultipled
        // pixels and pixels with an alpha value, so to keep things simple
        // we just use an alpha value in the destination buffer instead
        painter->setCompositionMode(QPainter::CompositionMode_DestinationOver);

        auto alphaBackgroundColor = backgroundColor();
        alphaBackgroundColor.setAlpha(127);

        painter->fillRect(QRectF{0.0, 0.0, width(), height()}, alphaBackgroundColor);
        painter->drawPixmap(target, QPixmap::fromImage(image), source);
    }
    else
    {
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->fillRect(QRectF{0.0, 0.0, width(), height()}, backgroundColor());
        painter->drawPixmap(target, _pixmap, source);
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

void CorrelationPlotItem::mousePressEvent(QMouseEvent* event)
{
    _clickMousePosition = _lastMousePosition = event->pos();
    event->accept();
}

void CorrelationPlotItem::mouseReleaseEvent(QMouseEvent* event)
{
    auto delta = _clickMousePosition - event->pos();

    // Ignore drags
    if(delta.manhattanLength() > 3)
        return;

    onClick(event);

    if(event->button() == Qt::RightButton)
        emit rightClick();

    _tooltipUpdateRequired = true;
}

void CorrelationPlotItem::mouseMoveEvent(QMouseEvent* event)
{
    auto* axisRectUnderCursor = _customPlot.axisRectAt(event->pos());

    if(axisRectUnderCursor == _continuousAxisRect)
    {
        auto* axis = axisRectUnderCursor->axis(QCPAxis::atLeft);
        auto delta = axis->pixelToCoord(_lastMousePosition.y()) - axis->pixelToCoord(event->pos().y());

        QMetaObject::invokeMethod(_worker, "pan", Qt::QueuedConnection, Q_ARG(QCPAxis*, axis), Q_ARG(double, delta));

        // Hide tooltips when paning
        _hoverPoint = {-1.0, -1.0};

        if(!updateTooltip()) // If tooltip update fails...
            updatePixmap(CorrelationPlotUpdateType::Render);

        _lastMousePosition = event->pos();
        event->accept();
    }
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

            if(plottable == nullptr || plottable->keyAxis() == nullptr)
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

                const QPointF tolerancePoint(_customPlot.selectionTolerance(),
                    _customPlot.selectionTolerance());

                plottable->pixelsToCoords(_hoverPoint - tolerancePoint, posKeyMin, dummy);
                plottable->pixelsToCoords(_hoverPoint + tolerancePoint, posKeyMax, dummy);

                if(posKeyMin > posKeyMax)
                    std::swap(posKeyMin, posKeyMax);

                const QCPGraphDataContainer::const_iterator begin = graph->data()->findBegin(posKeyMin, true);
                const QCPGraphDataContainer::const_iterator end = graph->data()->findEnd(posKeyMax, true);

                for(QCPGraphDataContainer::const_iterator it = begin; it != end; ++it)
                {
                    const auto distanceSq = QCPVector2D(plottable->coordsToPixels(
                        it->key, it->value) - _hoverPoint).lengthSquared();

                    if(distanceSq < minDistanceSq)
                    {
                        minDistanceSq = distanceSq;
                        nearestPlottable = plottable;

                        keyCoord = static_cast<double>(std::distance(graph->data()->constBegin(), it));
                    }
                }
            }
            else if(plottable != nullptr) // Placate MSVC C28182
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

        const double toleranceSq = _customPlot.selectionTolerance() * _customPlot.selectionTolerance();
        if(minDistanceSq > toleranceSq)
            return nullptr;
    }

    return nearestPlottable;
}

bool CorrelationPlotItem::updateTooltip()
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);

    if(!lock.owns_lock())
    {
        _tooltipUpdateRequired = true;
        return false;
    }

    _tooltipUpdateRequired = false;

    if(_tooltipLayer == nullptr)
        return false;

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
        showTooltip =
            discreteTooltip(axisRectUnderCursor, plottableUnderCursor, xCoord) ||
            continuousTooltip(axisRectUnderCursor, plottableUnderCursor, xCoord) ||
            columnAnnotationTooltip(axisRectUnderCursor);
    }

    if(showTooltip)
    {
        _itemTracer->setVisible(true);
        _itemTracer->setInterpolating(false);
        _itemTracer->updatePosition();
        auto itemTracerPosition = _itemTracer->anchor(u"position"_s)->pixelPosition();

        _hoverLabel->setVisible(true);

        auto key = std::round(_itemTracer->position->key());

        if(_hoverLabel->text().isEmpty() && plottableUnderCursor != nullptr && key >= 0)
        {
            auto index = static_cast<size_t>(key);

            if(index < numColumns())
            {
                double value = 0.0;

                const auto* plottable1D = dynamic_cast<const QCPPlottableInterface1D*>(plottableUnderCursor);
                if(plottable1D != nullptr)
                {
                    auto dataIndex = plottable1D->dataCount() == 1 ? 0 : static_cast<int>(key);
                    value = plottable1D->dataMainValue(dataIndex);
                }

                auto columnName = _groupByAnnotation ?
                    u::pluralise(_annotationGroupMap.at(index).size(), tr("column"), tr("columns")) :
                    _pluginInstance->columnName(_sortMap.at(index));

                _hoverLabel->setText(tr("%1, %2: %3")
                    .arg(plottableUnderCursor->name(),
                    columnName, u::formatNumberScientific(value)));
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

            const QColor color = plottableUnderCursor->brush().color();

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
        return false;
    }

    updatePixmap(CorrelationPlotUpdateType::RenderAndTooltips);

    return true;
}

void CorrelationPlotItem::hoverMoveEvent(QHoverEvent* event)
{
    if(event->position() == _hoverPoint)
        return;

    if(event->position() == event->oldPosF())
        return;

    _hoverPoint = event->position();
    updateTooltip();
}

void CorrelationPlotItem::hoverLeaveEvent(QHoverEvent*)
{
    if(_hoverPoint.x() < 0.0 && _hoverPoint.y() < 0.0)
        return;

    _hoverPoint = {-1.0, -1.0};
    updateTooltip();
}

void CorrelationPlotItem::wheelEvent(QWheelEvent* event)
{
    auto* axisRectUnderCursor = _customPlot.axisRectAt(event->position());

    if(event->angleDelta().y() != 0.0 && axisRectUnderCursor == _continuousAxisRect)
    {
        event->accept();

        auto* axis = axisRectUnderCursor->axis(QCPAxis::atLeft);
        auto f = 1.0 - ((event->position().y() - axisRectUnderCursor->top()) / axisRectUnderCursor->height());
        f = std::clamp(f, 0.0, 1.0);

        QMetaObject::invokeMethod(_worker, "zoom", Qt::QueuedConnection,
            Q_ARG(QCPAxis*, axis), Q_ARG(double, f),
            Q_ARG(int, event->angleDelta().y() > 0.0 ? 1 : -1));

        // Hide tooltips when zooming
        _hoverPoint = {-1.0, -1.0};

        if(!updateTooltip()) // If tooltip update fails...
            updatePixmap(CorrelationPlotUpdateType::Render);
    }
    else
        event->ignore();
}

void CorrelationPlotItem::configureLegend()
{
    if(_selectedRows.empty() || !_showLegend)
    {
        if(_legendLayoutGrid != nullptr)
        {
            _customPlot.plotLayout()->remove(_legendLayoutGrid);
            _customPlot.plotLayout()->simplify();
            _customPlot.plotLayout()->setColumnStretchFactor(0, 1.0);
            _legendLayoutGrid = nullptr;
        }

        return;
    }

    if(_legendLayoutGrid == nullptr)
    {
        // Create a subLayout to position the Legend
        _legendLayoutGrid = new QCPLayoutGrid;
        _customPlot.plotLayout()->insertColumn(1);
        _customPlot.plotLayout()->addElement(0, 1, _legendLayoutGrid);

        // Surround the legend row in two empty rows that are stretched maximally, and
        // stretch the legend itself minimally, thus centreing the legend vertically
        _legendLayoutGrid->insertRow(0);
        _legendLayoutGrid->setRowStretchFactor(0, 1.0);
        _legendLayoutGrid->addElement(1, 0, new QCPLegend);
        _legendLayoutGrid->setRowStretchFactor(1, std::numeric_limits<double>::min());
        _legendLayoutGrid->insertRow(2);
        _legendLayoutGrid->setRowStretchFactor(2, 1.0);
    }

    auto* legend = dynamic_cast<QCPLegend*>(_legendLayoutGrid->elementAt(
        _legendLayoutGrid->rowColToIndex(1, 0)));

    legend->setLayer(u"legend"_s);
    legend->clear();
    legend->setBorderPen(QPen(penColor()));
    legend->setBrush(backgroundColor());
    legend->setTextColor(penColor());

    const int marginSize = 5;
    legend->setMargins(QMargins(marginSize, marginSize, marginSize, marginSize));
    _legendLayoutGrid->setMargins(QMargins(0, marginSize, marginSize, marginSize + _bottomPadding));

    // BIGGEST HACK
    // Layouts and sizes aren't done until a replot, and layout is performed on another
    // thread which means it's too late to add or remove elements from the legend.
    // The anticipated sizes for the legend layout are calculated here but will break
    // if any additional rows are added to the plotLayout as the legend height is
    // estimated using the total height of the QQuickItem, not the (unknowable) plot height

    // See QCPPlottableLegendItem::draw for the reasoning behind this value
    const auto legendElementHeight =
        std::max(QFontMetrics(legend->font()).height(), legend->iconSize().height());

    const auto totalExternalMargins = _legendLayoutGrid->margins().top() + _legendLayoutGrid->margins().bottom();
    const auto totalInternalMargins = legend->margins().top() + legend->margins().bottom();
    const auto maxLegendHeight = _customPlot.height() - (totalExternalMargins + totalInternalMargins);

    size_t maxNumberOfElementsToDraw = 0;
    int accumulatedHeight = legendElementHeight;
    while(accumulatedHeight < maxLegendHeight)
    {
        accumulatedHeight += (legend->rowSpacing() + legendElementHeight);
        maxNumberOfElementsToDraw++;
    }

    std::map<QString, QCPAbstractPlottable*> plottablesMap;

    for(int i = 0; i < _customPlot.plottableCount(); i++)
    {
        auto* plottable = _customPlot.plottable(i);

        // Don't add invisible plots to the legend
        if(!plottable->visible() || plottable->name().isEmpty())
            continue;

        if(plottable->valueAxis() != _continuousYAxis && plottable->valueAxis() != _discreteYAxis)
            continue;

        plottablesMap[plottable->name()] = plottable;
    }

    // Not sure why it would be, at this point, but let's be safe
    if(plottablesMap.empty())
        return;

    std::vector<QCPAbstractPlottable*> plottables;

    std::transform(plottablesMap.begin(), plottablesMap.end(), std::back_inserter(plottables),
        [](auto& pair){ return pair.second; });
    std::sort(plottables.begin(), plottables.end(),
        [](const auto& a, const auto& b) { return a->name() < b->name(); });

    size_t numTruncated = 0;

    if(plottables.size() > maxNumberOfElementsToDraw)
    {
        auto maxMinusOne = maxNumberOfElementsToDraw - 1;
        numTruncated = plottables.size() - maxMinusOne;

        // Remove the excess plottables and another to make room for the overflow text
        plottables.resize(maxMinusOne);
    }

    for(auto* plottable : plottables)
        plottable->addToLegend(legend);

    if(numTruncated > 0)
    {
        auto* moreText = new QCPTextElement(&_customPlot);
        moreText->setMargins(QMargins());
        moreText->setLayer(_tooltipLayer);
        moreText->setTextFlags(Qt::AlignLeft);
        moreText->setFont(legend->font());
        moreText->setTextColor(Qt::gray);
        moreText->setText(tr("…and %1 more").arg(numTruncated));
        moreText->setVisible(true);

        legend->addElement(moreText);

        // When we're overflowing, hackily enlarge the bottom margin to
        // compensate for QCP's layout algorithm being a bit rubbish
        auto margins = legend->margins();
        margins.setBottom(margins.bottom() * 2);
        legend->setMargins(margins);
    }

    // Make the plot take 85% of the width, and the legend the remaining 15%
    _customPlot.plotLayout()->setColumnStretchFactor(0, 0.85);
    _customPlot.plotLayout()->setColumnStretchFactor(1, 0.15);

    legend->setVisible(true);
}

void CorrelationPlotItem::onClick(const QMouseEvent* event)
{
    if(event->button() != Qt::LeftButton)
        return;

    const auto* axisRect = _customPlot.axisRectAt(event->pos());

    if(axisRect == nullptr)
        return;

    auto point = event->pos() - axisRect->topLeft();

    auto selectColumn = [&]
    {
        if(point.x() < 0)
            return;

        auto* axis = axisRect->axis(QCPAxis::atBottom);
        auto column = static_cast<size_t>(std::round(axis->pixelToCoord(event->pos().x())));

        const bool toggle = event->modifiers().testFlag(Qt::ControlModifier);

        if(!toggle)
            _selectedColumns.clear();

        auto index = _sortMap.at(column);
        if(toggle && _selectedColumns.contains(index))
            _selectedColumns.erase(index);
        else
            _selectedColumns.insert(index);

        emit selectedColumnsChanged();
        rebuildPlot();
    };

    if(point.y() >= axisRect->height() && _showColumnNames)
    {
        if(point.x() >= 0)
        {
            if(_plotMode == PlotMode::RowsOfInterestColumnSelection)
                selectColumn();
            else
                sortBy(static_cast<int>(PlotColumnSortType::ColumnName));
        }
        else
        {
            // Click is in bottom left
            sortBy(static_cast<int>(PlotColumnSortType::Natural));
        }
    }
    else if(CorrelationPlotItem::axisRectIsColumnAnnotations(axisRect))
        onClickColumnAnnotation(axisRect, event);
    else if(_plotMode == PlotMode::RowsOfInterestColumnSelection)
        selectColumn();
}

void CorrelationPlotItem::rebuildPlot(InvalidateCache invalidateCache)
{
    if(_pluginInstance == nullptr)
        return;

    const std::unique_lock<std::recursive_mutex> lock(_mutex, std::try_to_lock);

    if(!lock.owns_lock())
    {
        if(invalidateCache == InvalidateCache::Yes)
            _rebuildRequired = RebuildRequired::Full;
        else if(_rebuildRequired != RebuildRequired::Full)
            _rebuildRequired = RebuildRequired::Partial;

        return;
    }

    _rebuildRequired = RebuildRequired::None;

    auto clearLineGraphCache = [this]
    {
        for(auto v : std::as_const(_lineGraphCache))
            _customPlot.removeGraph(v._graph);

        _lineGraphCache.clear();
    };

    if(invalidateCache == InvalidateCache::Yes)
        clearLineGraphCache();

    QElapsedTimer buildTimer;
    buildTimer.start();

    if(updateSortMap())
        clearLineGraphCache();

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

    configureAxisRects();

    _customPlot.plotLayout()->setMargins(QMargins(0, 0, _rightPadding, 0));

    // Hide tooltips after rebuild
    _hoverPoint = {-1.0, -1.0};

    if(_debug)
        qDebug() << "buildPlot" << buildTimer.elapsed() << "ms";

    updatePixmap(CorrelationPlotUpdateType::ReplotAndRenderAndTooltips);
}

size_t CorrelationPlotItem::numColumns() const
{
    if(_pluginInstance == nullptr)
        return 0;

    //FIXME: this will hit when we get around to dealing with the mixed case
    // Note that things that call this function might need a rethink at that point
    Q_ASSERT(_pluginInstance->numDiscreteColumns() == 0 ||
        _pluginInstance->numContinuousColumns() == 0);

    return _pluginInstance->numDiscreteColumns() +
        _pluginInstance->numContinuousColumns();
}

size_t CorrelationPlotItem::numVisibleColumns() const
{
    if(_groupByAnnotation)
        return _annotationGroupMap.size();

    return numColumns();
}

void CorrelationPlotItem::computeXAxisRange()
{
    auto min = 0.0;
    auto max = static_cast<double>(numVisibleColumns() - 1);
    auto maxVisibleColumns = columnAxisWidth() / minColumnWidth();
    auto numHiddenColumns = max - maxVisibleColumns;

    if(numHiddenColumns > 0.0)
    {
        const double position = numHiddenColumns * _horizontalScrollPosition;
        min = position;
        max = position + maxVisibleColumns;
    }

    const auto padding = 0.5;
    min -= padding;
    max += padding;

    QMetaObject::invokeMethod(_worker, "setXAxisRange", Qt::QueuedConnection,
        Q_ARG(double, min), Q_ARG(double, max));
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

void CorrelationPlotItem::setShowAllColumns(bool showAllColumns)
{
    if(_showAllColumns != showAllColumns)
    {
        _showAllColumns = showAllColumns;
        computeXAxisRange();
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

void CorrelationPlotItem::setShowLegend(bool showLegend)
{
    if(_showLegend != showLegend)
    {
        _showLegend = showLegend;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

double CorrelationPlotItem::minimumHeight() const
{
    return 150.0 + columnAnnotationsHeight();
}

void CorrelationPlotItem::setPluginInstance(CorrelationPluginInstance* pluginInstance)
{
    _pluginInstance = pluginInstance;

    connect(_pluginInstance, &CorrelationPluginInstance::visualsChanged, // clazy:exclude=connect-non-signal
        this, [this](VisualChangeFlags nodeChange, VisualChangeFlags)
    {
        if(::Flags<VisualChangeFlags>(nodeChange).test(VisualChangeFlags::Color))
            rebuildPlot();
    });
}

void CorrelationPlotItem::setSelectedRows(const QList<int>& selectedRows)
{
    _selectedRows = selectedRows;
    emit selectedRowsChanged();

    rebuildPlot();
}

void CorrelationPlotItem::setElideLabelWidth(int elideLabelWidth)
{
    const bool changed = (_elideLabelWidth != elideLabelWidth);
    _elideLabelWidth = elideLabelWidth;

    if(changed && showColumnNames())
        rebuildPlot();
}

bool CorrelationPlotItem::showColumnNames() const
{
    if(!_showColumnNames)
        return false;

    if(_groupByAnnotation)
    {
        // Only show column names if there are no groups
        return numColumns() == numVisibleColumns();
    }

    return true;
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
        QMetaObject::invokeMethod(_worker, "setShowGridLines", Qt::QueuedConnection,
            Q_ARG(bool, showGridLines));
        rebuildPlot();
    }
}

void CorrelationPlotItem::setHorizontalScrollPosition(double horizontalScrollPosition)
{
    auto newHorizontalScrollPosition = std::clamp(horizontalScrollPosition, 0.0, 1.0);

    if(newHorizontalScrollPosition != _horizontalScrollPosition)
    {
        _horizontalScrollPosition = newHorizontalScrollPosition;
        computeXAxisRange();
        updatePixmap(CorrelationPlotUpdateType::Render);
    }
}

void CorrelationPlotItem::setRightPadding(int padding)
{
    if(_rightPadding != padding)
    {
        _rightPadding = padding;
        rebuildPlot();
    }
}

void CorrelationPlotItem::setBottomPadding(int padding)
{
    if(_bottomPadding != padding)
    {
        _bottomPadding = padding;
        rebuildPlot();
    }
}

bool CorrelationPlotItem::updateSortMap()
{
    auto previousSortmap = _sortMap;

    _sortMap.resize(numColumns());
    std::iota(_sortMap.begin(), _sortMap.end(), 0);

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
    columnSortOrders.reserve(static_cast<size_t>(_columnSortOrders.size()));

    for(const auto& qmlColumnSortOrder : std::as_const(_columnSortOrders))
    {
        Q_ASSERT(u::containsAllOf(qmlColumnSortOrder, {"type", "text", "order"}));

        ColumnSortOrder columnSortOrder;
        columnSortOrder._type = normaliseQmlEnum<PlotColumnSortType>(qmlColumnSortOrder[u"type"_s].toInt());
        columnSortOrder._order = static_cast<Qt::SortOrder>(qmlColumnSortOrder[u"order"_s].toInt());

        if(columnSortOrder._type == PlotColumnSortType::ColumnAnnotation)
        {
            auto columnAnnotationName = qmlColumnSortOrder[u"text"_s].toString();
            columnSortOrder._annotation = _pluginInstance->columnAnnotationByName(columnAnnotationName);
        }

        // If grouping by annotation, sorting by data value doesn't really make sense
        if(_groupByAnnotation && columnSortOrder._type == PlotColumnSortType::DataValue)
            continue;

        columnSortOrders.emplace_back(columnSortOrder);
    }

    if(_columnDataValueSortOrder.empty())
        _columnDataValueSortOrder.resize(numColumns());

    if(!_columnSortOrderPinned && std::any_of(columnSortOrders.begin(), columnSortOrders.end(),
        [](const auto& cso) { return cso._type == PlotColumnSortType::DataValue; }))
    {
        std::vector<double> columnValues;
        columnValues.resize(numColumns(), std::numeric_limits<double>::lowest());

        for(size_t col = 0; col < _pluginInstance->numDiscreteColumns(); col++)
        {
            columnValues.at(col) = 0.0;

            for(auto row : std::as_const(_selectedRows))
            {
                if(!_pluginInstance->discreteDataAt(static_cast<size_t>(row), col).isEmpty())
                    columnValues.at(col) += 1.0;
            }
        }

        auto plotAveragingType = normaliseQmlEnum<PlotAveragingType>(_averagingType);
        auto offset = _pluginInstance->numDiscreteColumns();

        for(size_t col = 0; col < _pluginInstance->numContinuousColumns(); col++)
        {
            double total = 0.0;
            std::vector<double> values;
            values.reserve(static_cast<size_t>(_selectedRows.size()));

            for(auto row : std::as_const(_selectedRows))
            {
                auto value = _pluginInstance->continuousDataAt(static_cast<size_t>(row), col);

                switch(plotAveragingType)
                {
                case PlotAveragingType::MeanLine:
                case PlotAveragingType::MeanHistogram:
                    total += value;
                    break;

                case PlotAveragingType::MedianLine:
                case PlotAveragingType::IQR:
                    values.push_back(value);
                    break;

                default:
                    columnValues.at(col + offset) = std::max(columnValues.at(col + offset), value);
                    break;
                }
            }

            switch(plotAveragingType)
            {
            case PlotAveragingType::MeanLine:
            case PlotAveragingType::MeanHistogram:
                columnValues.at(col + offset) = total / static_cast<double>(_selectedRows.size());
                break;

            case PlotAveragingType::MedianLine:
            case PlotAveragingType::IQR:
                columnValues.at(col + offset) = u::medianOf(values);
                break;

            default:
                break;
            }
        }

        std::vector<size_t> inverseDataValueOrdering(numColumns());
        std::iota(inverseDataValueOrdering.begin(), inverseDataValueOrdering.end(), 0);
        std::sort(inverseDataValueOrdering.begin(), inverseDataValueOrdering.end(),
        [&columnValues](size_t a, size_t b)
        {
            return columnValues.at(a) < columnValues.at(b);
        });

        for(size_t i = 0; i < inverseDataValueOrdering.size(); i++)
            _columnDataValueSortOrder[inverseDataValueOrdering.at(i)] = i;
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
                auto columnNameA = _pluginInstance->columnName(a);
                auto columnNameB = _pluginInstance->columnName(b);

                if(columnNameA == columnNameB)
                    continue;

                return columnSortOrder._order == Qt::AscendingOrder ?
                    collator.compare(columnNameA, columnNameB) < 0 :
                    collator.compare(columnNameB, columnNameA) < 0;
            }

            case PlotColumnSortType::DataValue:
                return columnSortOrder._order == Qt::AscendingOrder ?
                    _columnDataValueSortOrder.at(a) < _columnDataValueSortOrder.at(b) :
                    _columnDataValueSortOrder.at(b) < _columnDataValueSortOrder.at(a);

            case PlotColumnSortType::ColumnAnnotation:
            {
                auto annotationValueA = columnSortOrder._annotation->valueAt(a);
                auto annotationValueB = columnSortOrder._annotation->valueAt(b);

                if(annotationValueA == annotationValueB)
                    continue;

                if(columnSortOrder._annotation->isNumeric() && !annotationValueA.isEmpty() && !annotationValueB.isEmpty())
                {
                    return columnSortOrder._order == Qt::AscendingOrder ?
                        u::toNumber(annotationValueA) < u::toNumber(annotationValueB) :
                        u::toNumber(annotationValueB) < u::toNumber(annotationValueA);
                }

                return columnSortOrder._order == Qt::AscendingOrder ?
                    collator.compare(annotationValueA, annotationValueB) < 0 :
                    collator.compare(annotationValueB, annotationValueA) < 0;
            }

            case PlotColumnSortType::HierarchicalClustering:
                return columnSortOrder._order == Qt::AscendingOrder ?
                    _pluginInstance->hcColumn(a) < _pluginInstance->hcColumn(b) :
                    _pluginInstance->hcColumn(b) < _pluginInstance->hcColumn(a);
            }
        }

        // If all else fails, just use natural order
        return a < b;
    });

    const size_t oldSize = _annotationGroupMap.size();
    _annotationGroupMap.clear();

    if(_groupByAnnotation)
    {
        std::vector<QString> lastValueColumn;

        for(size_t i = 0U; i < numColumns(); i++)
        {
            auto column = _sortMap.at(i);

            std::vector<QString> valueColumn;
            valueColumn.reserve(_visibleColumnAnnotationNames.size());

            for(const auto& name : _visibleColumnAnnotationNames)
            {
                const auto* annotation = _pluginInstance->columnAnnotationByName(name);
                valueColumn.emplace_back(annotation->valueAt(column));
            }

            if(valueColumn != lastValueColumn || lastValueColumn.empty())
            {
                lastValueColumn = valueColumn;
                _annotationGroupMap.emplace_back();
            }

            _annotationGroupMap.back().push_back(column);
        }
    }

    if(_annotationGroupMap.size() != oldSize)
    {
        computeXAxisRange();
        emit numVisibleColumnsChanged();
    }

    return _sortMap != previousSortmap;
}

void CorrelationPlotItem::sortBy(int type, const QString& text)
{
    const bool columnSortOrderCouldBePinned = columnSortOrderCanBePinned();
    const bool typeIsColumnAnnotation =
        (type == static_cast<int>(PlotColumnSortType::ColumnAnnotation));

    auto order = Qt::AscendingOrder;

    auto existing = std::find_if(_columnSortOrders.cbegin(), _columnSortOrders.cend(),
    [type, &text, typeIsColumnAnnotation](const auto& value)
    {
        const bool sameType = (value[u"type"_s].toInt() == type);
        const bool sameText = (value[u"text"_s].toString() == text);

        return sameType && (!typeIsColumnAnnotation || sameText);
    });

    // If the type has been sorted on before, remove it so
    // that adding it brings it to the front
    if(existing != _columnSortOrders.cend())
    {
        order = static_cast<Qt::SortOrder>((*existing)[u"order"_s].toInt());

        if(existing == _columnSortOrders.cbegin())
        {
            // If the thing we're sorting is on the front
            // of the list already, flip the sort order
            order = order == Qt::AscendingOrder ?
                Qt::DescendingOrder : Qt::AscendingOrder;
        }

        _columnSortOrders.erase(existing);
    }

    if(!typeIsColumnAnnotation)
    {
        _columnSortOrders.erase(std::remove_if(_columnSortOrders.begin(), _columnSortOrders.end(),  // clazy:exclude=strict-iterators,detaching-member
        [](const auto& value)
        {
            return static_cast<PlotColumnSortType>(value[u"type"_s].toInt()) !=
                PlotColumnSortType::ColumnAnnotation;
        }), _columnSortOrders.end());  // clazy:exclude=strict-iterators,detaching-member
    }

    QVariantMap newSortOrder;
    newSortOrder[u"type"_s] = type;
    newSortOrder[u"text"_s] = text;
    newSortOrder[u"order"_s] = order;

    _columnSortOrders.push_front(newSortOrder);

    emit plotOptionsChanged();

    if(columnSortOrderCouldBePinned != columnSortOrderCanBePinned())
        emit columnSortOrderCanBePinnedChanged();

    rebuildPlot(InvalidateCache::Yes);
}

void CorrelationPlotItem::resetZoom()
{
    if(_worker == nullptr)
        return;

    QMetaObject::invokeMethod(_worker, "resetZoom", Qt::QueuedConnection);
    updatePixmap(CorrelationPlotUpdateType::Render);
}

void CorrelationPlotItem::setColumnSortOrders(const QVector<QVariantMap>& columnSortOrders) // clazy:exclude=qproperty-type-mismatch
{
    if(_columnSortOrders != columnSortOrders)
    {
        _columnSortOrders = columnSortOrders;
        emit plotOptionsChanged();

        rebuildPlot(InvalidateCache::Yes);
    }
}

bool CorrelationPlotItem::columnSortOrderCanBePinned() const
{
    return std::any_of(_columnSortOrders.cbegin(), _columnSortOrders.cend(), [](const auto& value)
    {
        return static_cast<PlotColumnSortType>(value[u"type"_s].toInt()) ==
            PlotColumnSortType::DataValue;
    });
}

QString CorrelationPlotItem::elideLabel(const QString& label)
{
    if(_elideLabelWidth <= 0)
        return {};

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

double CorrelationPlotItem::visibleHorizontalFraction() const
{
    if(_pluginInstance == nullptr)
        return 1.0;

    auto f = (columnAxisWidth() / (minColumnWidth() * static_cast<double>(numVisibleColumns())));

    return std::min(f, 1.0);
}

double CorrelationPlotItem::labelHeight() const
{
    const int columnPadding = 1;
    return static_cast<double>(_defaultFontMetrics.height() + columnPadding);
}

constexpr double minColumnPixelWidth = 1.0;
constexpr double minColumnBoxPlotWidth = 10.0;

bool CorrelationPlotItem::isWide() const
{
    return (static_cast<double>(numVisibleColumns()) * minColumnPixelWidth) > columnAxisWidth();
}

double CorrelationPlotItem::minColumnWidth() const
{
    if(showColumnNames())
        return labelHeight();

    if(_showAllColumns)
        return columnAxisWidth() / static_cast<double>(numVisibleColumns());

    if(normaliseQmlEnum<PlotAveragingType>(_averagingType) == PlotAveragingType::IQR || _groupByAnnotation)
        return minColumnBoxPlotWidth;

    if(normaliseQmlEnum<PlotDispersionType>(_dispersionType) != PlotDispersionType::None)
        return minColumnBoxPlotWidth;

    return minColumnPixelWidth;
}

double CorrelationPlotItem::columnAxisWidth() const
{
    int marginWidth = 0;

    if(_discreteAxisRect != nullptr)
    {
        marginWidth += (_discreteAxisRect->margins().left() +
            _discreteAxisRect->margins().right());
    }

    if(_continuousAxisRect != nullptr)
    {
        marginWidth += (_continuousAxisRect->margins().left() +
            _continuousAxisRect->margins().right());
    }

    //FIXME This value is wrong when the legend is enabled
    return width() - marginWidth;
}

double CorrelationPlotItem::columnAnnotationsHeight() const
{
    if(_plotMode == PlotMode::ColumnAnnotationSelection)
        return static_cast<double>(_pluginInstance->columnAnnotations().size()) * labelHeight();

    return static_cast<double>(_visibleColumnAnnotationNames.size()) * labelHeight();
}

std::vector<size_t> CorrelationPlotItem::selectedColumns() const
{
    std::vector<size_t> v;
    v.reserve(_selectedColumns.size());

    for(auto column : _selectedColumns)
        v.push_back(column);

    return v;
}

void CorrelationPlotItem::createTooltip()
{
    if(_tooltipLayer != nullptr)
        return;

    _customPlot.addLayer(u"tooltipLayer"_s);
    _tooltipLayer = _customPlot.layer(u"tooltipLayer"_s);
    _tooltipLayer->setMode(QCPLayer::LayerMode::lmBuffered);

    QFont defaultFont10Pt;
    defaultFont10Pt.setPointSize(10);

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
}

void CorrelationPlotItem::configureAxisRects()
{
    if(_axesLayoutGrid == nullptr)
    {
        _axesLayoutGrid = new QCPLayoutGrid;
        _mainLayoutGrid->addElement(_axesLayoutGrid);
        _axesLayoutGrid->setRowSpacing(0);
        _axesLayoutGrid->setFillOrder(QCPLayoutGrid::foRowsFirst);
    }

    if(_pluginInstance->numDiscreteColumns() > 0)
        configureDiscreteAxisRect();

    if(_pluginInstance->numContinuousColumns() > 0)
        configureContinuousAxisRect();

    QString xAxisLabel = _xAxisLabel;

    if(_elideLabelWidth <= 0)
    {
        // There is no room to display labels, so show a warning instead
        QString warning;

        if(!_visibleColumnAnnotationNames.empty())
            warning = tr("Resize To Expose Column Information");
        else if(_showColumnNames)
            warning = tr("Resize To Expose Column Names");

        if(!warning.isEmpty())
        {
            if(!xAxisLabel.isEmpty())
                xAxisLabel = tr("%1 (%2)").arg(xAxisLabel, warning);
            else
                xAxisLabel = warning;
        }
    }

    if(!xAxisLabel.isEmpty())
    {
        if(_xAxisLabelTextElement == nullptr)
        {
            _xAxisLabelTextElement = new QCPTextElement(&_customPlot);
            _mainLayoutGrid->addElement(1, 0, _xAxisLabelTextElement);
        }

        _xAxisLabelTextElement->setText(xAxisLabel);
        _xAxisLabelTextElement->setTextColor(penColor());
        _xAxisLabelTextElement->setMargins({0, 0, 0, _bottomPadding});
    }
    else if(_xAxisLabelTextElement != nullptr)
    {
        _mainLayoutGrid->remove(_xAxisLabelTextElement);
        _mainLayoutGrid->simplify();
        _xAxisLabelTextElement = nullptr;
    }

    configureLegend();
    createTooltip();
}

void CorrelationPlotItem::updatePlotSize()
{
    computeXAxisRange();
    updatePixmap(CorrelationPlotUpdateType::Render);
}

void CorrelationPlotItem::clone(CorrelationPlotItem& target) const
{
    target._pluginInstance                  = _pluginInstance;
    target._horizontalScrollPosition        = _horizontalScrollPosition;
    target._selectedRows                    = _selectedRows;

    target._groupByAnnotation               = _groupByAnnotation;
    target._colorGroupByAnnotationName      = _colorGroupByAnnotationName;
    target._selectedColumns                 = _selectedColumns;

    target._elideLabelWidth                 = _elideLabelWidth;
    target._showColumnNames                 = _showColumnNames;
    target._showGridLines                   = _showGridLines;
    target._showLegend                      = _showLegend;
    target._scaleType                       = _scaleType;
    target._scaleByAttributeName            = _scaleByAttributeName;
    target._averagingType                   = _averagingType;
    target._averagingAttributeName          = _averagingAttributeName;
    target._dispersionType                  = _dispersionType;
    target._dispersionVisualType            = _dispersionVisualType;
    target._columnSortOrders                = _columnSortOrders;
    target._xAxisLabel                      = _xAxisLabel;
    target._yAxisLabel                      = _yAxisLabel;
    target._rightPadding                    = _rightPadding;
    target._bottomPadding                   = _bottomPadding;
    target._includeYZero                    = _includeYZero;
    target._showIqrOutliers                 = _showIqrOutliers;
    target._showAllColumns                  = _showAllColumns;

    target._sortMap                         = _sortMap;
    target._visibleColumnAnnotationNames    = _visibleColumnAnnotationNames;
    target._annotationGroupMap              = _annotationGroupMap;

    _worker->clone(*target._worker);
}

void CorrelationPlotItem::savePlotImage(const QUrl& url, const QString& extension)
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    auto cpsic = std::make_unique<CorrelationPlotSaveImageCommand>(*this,
        url.toLocalFile(), extension);

    cpsic->addImageConfiguration({}, _selectedRows);

    _pluginInstance->commandManager()->execute(ExecutePolicy::Once, std::move(cpsic));
}

void CorrelationPlotItem::savePlotImageByRow(const QUrl& url, const QString& extension)
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    auto cpsic = std::make_unique<CorrelationPlotSaveImageCommand>(*this,
        url.toLocalFile(), extension);

    for(auto selectedRow : std::as_const(_selectedRows))
    {
        auto rowName = _pluginInstance->rowName(static_cast<size_t>(selectedRow));
        const static QRegularExpression re(QStringLiteral(R"(\s+)"));
        rowName.replace(re, u"_"_s);

        cpsic->addImageConfiguration(rowName, {selectedRow});
    }

    _pluginInstance->commandManager()->execute(ExecutePolicy::Once, std::move(cpsic));
}

void CorrelationPlotItem::savePlotImageByAttribute(const QUrl& url, const QString& extension,
    const QString& attributeName)
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    auto cpsic = std::make_unique<CorrelationPlotSaveImageCommand>(*this,
        url.toLocalFile(), extension);

    std::map<QString, QVector<int>> images;

    for(auto selectedRow : std::as_const(_selectedRows))
    {
        auto attributeValue = _pluginInstance->attributeValueFor(attributeName,
            static_cast<size_t>(selectedRow));
        const static QRegularExpression re(QStringLiteral(R"(\s+)"));
        attributeValue.replace(re, u"_"_s);
        images[attributeValue].append(selectedRow); // clazy:exclude=reserve-candidates
    }

    for(const auto& [label, rows] : images)
        cpsic->addImageConfiguration(label, rows);

    _pluginInstance->commandManager()->execute(ExecutePolicy::Once, std::move(cpsic));
}

void CorrelationPlotItem::savePlotImage(const QString& filename)
{
    const QFileInfo fileInfo(filename);

    if(fileInfo.suffix() == u"png"_s)
        _customPlot.savePng(filename);
    else if(fileInfo.suffix() == u"pdf"_s)
        _customPlot.savePdf(filename);
    else if(fileInfo.suffix() == u"jpg"_s)
        _customPlot.saveJpg(filename);
    else
        qDebug() << "CorrelationPlotItem::savePlotImage unknown suffix:" << fileInfo.suffix();
}
