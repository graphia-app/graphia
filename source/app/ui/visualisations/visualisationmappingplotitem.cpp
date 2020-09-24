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

#include "visualisationmappingplotitem.h"

#include <cmath>

VisualisationMappingPlotItem::VisualisationMappingPlotItem(QQuickItem* parent) :
    QQuickPaintedItem(parent)
{
    setRenderTarget(RenderTarget::FramebufferObject);

    _customPlot.setOpenGl(true);

    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &QQuickPaintedItem::widthChanged, this, &VisualisationMappingPlotItem::updatePlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &VisualisationMappingPlotItem::updatePlotSize);
    connect(&_customPlot, &QCustomPlot::afterReplot, this, &VisualisationMappingPlotItem::onReplot);

    buildPlot();
}

void VisualisationMappingPlotItem::paint(QPainter* painter)
{
    painter->drawPixmap(0, 0, _customPlot.toPixmap());
}

void VisualisationMappingPlotItem::setRangeToMinMax()
{
    _min = _statistics._min;
    _max = _statistics._max;
    emit minimumChanged();
    emit maximumChanged();
    buildPlot();
}

void VisualisationMappingPlotItem::setRangeToStddev()
{
    _min = _statistics._mean - (0.5 * _statistics._stddev);
    _max = _statistics._mean + (0.5 * _statistics._stddev);
    emit minimumChanged();
    emit maximumChanged();
    buildPlot();
}

void VisualisationMappingPlotItem::setValues(const QVector<double>& values)
{
    _values = values;
    _statistics = u::findStatisticsFor(_values);
    buildPlot();
}

void VisualisationMappingPlotItem::setInvert(bool invert)
{
    _invert = invert;
    buildPlot();
}

void VisualisationMappingPlotItem::setExponent(double exponent)
{
    _exponent = exponent;
    buildPlot();
}

void VisualisationMappingPlotItem::setMinimum(double min)
{
    _min = std::clamp(min, _statistics._min, _statistics._max);
    _min = std::min(_min, _max);

    if(_min == _max)
        _min = _max - std::numeric_limits<double>::epsilon();

    emit minimumChanged();
    buildPlot();
}

void VisualisationMappingPlotItem::setMaximum(double max)
{
    _max = std::clamp(max, _statistics._min, _statistics._max);
    _max = std::max(_max, _min);

    if(_max == _min)
        _max = _min + std::numeric_limits<double>::epsilon();

    emit maximumChanged();
    buildPlot();
}

void VisualisationMappingPlotItem::updatePlotSize()
{
    // QML does some spurious resizing, which can result in odd
    // sizes that things get upset with
    if(width() <= 0.0 || height() <= 0.0)
        return;

    _customPlot.setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));

    // Since QCustomPlot is a QWidget, it is never technically visible, so never generates
    // a resizeEvent, so its viewport never gets set, so we must do so manually
    _customPlot.setViewport(_customPlot.geometry());
}

void VisualisationMappingPlotItem::buildPlot()
{
    _customPlot.clearItems();
    _customPlot.clearPlottables();
    _customPlot.plotLayout()->clear();

    auto* mainAxisLayout = new QCPLayoutGrid;
    _customPlot.plotLayout()->addElement(0, 1, mainAxisLayout);

    auto* mainAxisRect = new QCPAxisRect(&_customPlot);
    mainAxisLayout->addElement(mainAxisRect);
    auto* mainXAxis = mainAxisRect->axis(QCPAxis::atBottom);
    auto* mainYAxis = mainAxisRect->axis(QCPAxis::atLeft);

    double rangeMargin = _statistics._range * 0.05;
    double min = _statistics._min - rangeMargin;
    double max = _statistics._max + rangeMargin;

    mainXAxis->setRange(0.0, 1.0);
    mainYAxis->setRange(min, max);

    mainXAxis->setTickLabels(false);
    mainXAxis->setTicks(false);

    auto* line = new QCPGraph(mainXAxis, mainYAxis);
    line->setPen(QPen(Qt::black));

    QVector<double> x;
    QVector<double> y;

    const int numSamples = 250;
    x.reserve(numSamples);
    y.reserve(numSamples);

    for(int i = 0; i <= numSamples; i++)
    {
        double n = static_cast<double>(i) / static_cast<double>(numSamples);
        x.append(n);

        if(_invert)
            n = 1.0 - n;

        auto e = std::pow(n, _exponent);
        auto value = _min + (e * (_max - _min));
        value = std::clamp(value, _statistics._min, _statistics._max);
        y.append(value);
    }

    line->setData(x, y);

    auto limitPen = QPen(Qt::black, 1.0, Qt::DashLine);

    auto* upperLimit = new QCPGraph(mainXAxis, mainYAxis);
    upperLimit->setPen(limitPen);
    upperLimit->setData({0.0, 1.0}, {_max, _max});

    auto* lowerLimit = new QCPGraph(mainXAxis, mainYAxis);
    lowerLimit->setPen(limitPen);
    lowerLimit->setData({0.0, 1.0}, {_min, _min});

    auto* valuesAxisLayout = new QCPLayoutGrid;
    _customPlot.plotLayout()->addElement(0, 0, valuesAxisLayout);

    auto* valuesAxisRect = new QCPAxisRect(&_customPlot);
    valuesAxisRect->setMinimumSize(10, 0);
    valuesAxisRect->setMaximumSize(10, 10000);
    valuesAxisLayout->addElement(valuesAxisRect);
    auto* valuesXAxis = valuesAxisRect->axis(QCPAxis::atBottom);
    auto* valuesYAxis = valuesAxisRect->axis(QCPAxis::atLeft);
    valuesXAxis->setVisible(false);
    valuesYAxis->setVisible(false);

    valuesXAxis->setRange(0.0, 1.0);
    valuesYAxis->setRange(min, max);

    auto* points = new QCPGraph(valuesXAxis, valuesYAxis);

    QVector<double> px;
    QVector<double> py;

    px.reserve(_values.size());
    py.reserve(_values.size());

    for(int i = 0 ; i < _values.size(); i++)
    {
        px.append(0.5);
        py.append(_values.at(i));
    }

    auto pointColor = QColor(0, 0, 255, 53);
    points->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle,
        pointColor, pointColor, 5.0));
    points->setLineStyle(QCPGraph::lsNone);
    points->setData(px, py);

    QCPMarginGroup *marginGroup = new QCPMarginGroup(&_customPlot);
    mainAxisRect->setMarginGroup(QCP::msBottom, marginGroup);
    valuesAxisRect->setMarginGroup(QCP::msBottom, marginGroup);

    for(auto* axis : {mainXAxis, mainYAxis, valuesXAxis, valuesYAxis})
    {
        axis->setLayer("axes");
        axis->grid()->setLayer("grid");
    }

    _customPlot.replot(QCustomPlot::rpQueuedReplot);
}

void VisualisationMappingPlotItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        _clickPosition = _customPlot.yAxis->pixelToCoord(event->pos().y());
        _clickMin = _min;
        _clickMax = _max;
        _dragType = dragTypeForEvent(event);
    }
}

void VisualisationMappingPlotItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
        _dragType = DragType::None;
}

void VisualisationMappingPlotItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(_dragType != DragType::None)
    {
        auto position = _customPlot.yAxis->pixelToCoord(event->pos().y());

        switch(_dragType)
        {
        case DragType::Min:
            setMinimum(position);
            emit manualChangeToMinMax();
            break;

        case DragType::Max:
            setMaximum(position);
            emit manualChangeToMinMax();
            break;

        case DragType::Move:
        {
            auto diff = position - _clickPosition;
            setMinimum(_clickMin + diff);
            setMaximum(_clickMax + diff);
            emit manualChangeToMinMax();
            break;
        }

        default:
            break;
        }
    }
}

void VisualisationMappingPlotItem::hoverMoveEvent(QHoverEvent* event)
{
    auto dragType = dragTypeForEvent(event);

    if(dragType == DragType::Min || dragType == DragType::Max)
        setCursor(Qt::SizeVerCursor);
    else if(dragType == DragType::Move)
        setCursor(Qt::SizeAllCursor);
    else
        setCursor({});
}

void VisualisationMappingPlotItem::routeMouseEvent(QMouseEvent* event)
{
    auto* newEvent = new QMouseEvent(event->type(), event->localPos(),
        event->button(), event->buttons(), event->modifiers());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void VisualisationMappingPlotItem::onReplot()
{
    update();
}
