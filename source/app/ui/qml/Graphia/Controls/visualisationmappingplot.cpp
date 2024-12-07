/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "visualisationmappingplot.h"

#include "shared/rendering/multisamples.h"

#include <QQmlEngine>

#include <cmath>

using namespace Qt::Literals::StringLiterals;

VisualisationMappingPlot::VisualisationMappingPlot(QQuickItem* parent) :
    QCustomPlotQuickItem(multisamples(), parent)
{}

void VisualisationMappingPlot::setRangeToMinMax()
{
    _min = _statistics._min;
    _max = _statistics._max;
    emit minimumChanged();
    emit maximumChanged();
    buildPlot();
}

void VisualisationMappingPlot::setRangeToStddev()
{
    _min = _statistics._mean - _statistics._stddev;
    _max = _statistics._mean + _statistics._stddev;
    emit minimumChanged();
    emit maximumChanged();
    buildPlot();
}

void VisualisationMappingPlot::setValues(const QVector<double>& values)
{
    _values = values;
    _statistics = u::findStatisticsFor(_values);
    buildPlot();
}

void VisualisationMappingPlot::setInvert(bool invert)
{
    _invert = invert;
    buildPlot();
}

void VisualisationMappingPlot::setExponent(double exponent)
{
    _exponent = exponent;
    buildPlot();
}

void VisualisationMappingPlot::setMinimum(double min)
{
    if(!_statistics._values.empty())
        _min = std::clamp(min, _statistics._min, _statistics._max);
    else
        _min = min;

    _min = std::min(_min, _max);

    if(_min == _max)
        _min = _max - std::numeric_limits<double>::epsilon();

    emit minimumChanged();
    buildPlot();
}

void VisualisationMappingPlot::setMaximum(double max)
{
    if(!_statistics._values.empty())
        _max = std::clamp(max, _statistics._min, _statistics._max);
    else
        _max = max;

    _max = std::max(_max, _min);

    if(_max == _min)
        _max = _min + std::numeric_limits<double>::epsilon();

    emit maximumChanged();
    buildPlot();
}

void VisualisationMappingPlot::buildPlot()
{
    if(_values.isEmpty())
        return;

    customPlot().clearItems();
    customPlot().clearPlottables();
    customPlot().plotLayout()->clear();

    auto* mainAxisLayout = new QCPLayoutGrid;
    customPlot().plotLayout()->addElement(0, 1, mainAxisLayout);

    auto* mainAxisRect = new QCPAxisRect(&customPlot());
    mainAxisLayout->addElement(mainAxisRect);
    auto* mainXAxis = mainAxisRect->axis(QCPAxis::atBottom);
    auto* mainYAxis = mainAxisRect->axis(QCPAxis::atLeft);

    const double rangeMargin = _statistics._range * 0.05;
    const double min = _statistics._min - rangeMargin;
    const double max = _statistics._max + rangeMargin;

    mainXAxis->setRange(0.0, 1.0);
    mainYAxis->setRange(min, max);

    mainXAxis->setTickLabels(false);
    mainXAxis->setTicks(false);

    mainXAxis->setBasePen(QPen(penColor()));
    mainXAxis->setTickPen(QPen(penColor()));
    mainXAxis->setSubTickPen(QPen(penColor()));
    auto xAxisGridPen = mainXAxis->grid()->pen();
    xAxisGridPen.setColor(lightPenColor());
    mainXAxis->grid()->setPen(xAxisGridPen);

    mainYAxis->setBasePen(QPen(penColor()));
    mainYAxis->setTickPen(QPen(penColor()));
    mainYAxis->setSubTickPen(QPen(penColor()));
    auto yAxisGridPen = mainYAxis->grid()->pen();
    yAxisGridPen.setColor(lightPenColor());
    mainYAxis->grid()->setPen(yAxisGridPen);

    mainYAxis->setTickLabelColor(penColor());

    auto* line = new QCPGraph(mainXAxis, mainYAxis);
    line->setPen(QPen(penColor()));

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

    auto limitPen = QPen(penColor(), 1.0, Qt::DashLine);

    auto* upperLimit = new QCPGraph(mainXAxis, mainYAxis);
    upperLimit->setPen(limitPen);
    upperLimit->setData({0.0, 1.0}, {_max, _max});

    auto* lowerLimit = new QCPGraph(mainXAxis, mainYAxis);
    lowerLimit->setPen(limitPen);
    lowerLimit->setData({0.0, 1.0}, {_min, _min});

    auto* valuesAxisLayout = new QCPLayoutGrid;
    customPlot().plotLayout()->addElement(0, 0, valuesAxisLayout);

    auto* valuesAxisRect = new QCPAxisRect(&customPlot());
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

    for(auto value : std::as_const(_values))
    {
        px.append(0.5);
        py.append(value);
    }

    auto pointColor = QColor(0, 0, 255, 53);
    points->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle,
        pointColor, pointColor, 5.0));
    points->setLineStyle(QCPGraph::lsNone);
    points->setData(px, py);

    auto* marginGroup = new QCPMarginGroup(&customPlot());
    mainAxisRect->setMarginGroup(QCP::msBottom, marginGroup);
    valuesAxisRect->setMarginGroup(QCP::msBottom, marginGroup);

    for(auto* axis : {mainXAxis, mainYAxis, valuesXAxis, valuesYAxis})
    {
        axis->setLayer(u"axes"_s);
        axis->grid()->setLayer(u"grid"_s);
    }

    customPlot().replot(QCustomPlot::rpQueuedReplot);
}

void VisualisationMappingPlot::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        _clickPosition = customPlot().yAxis->pixelToCoord(event->pos().y());
        _clickMin = _min;
        _clickMax = _max;
        _dragType = dragTypeForEvent(event);
    }
}

void VisualisationMappingPlot::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
        _dragType = DragType::None;
}

void VisualisationMappingPlot::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(_dragType != DragType::None)
    {
        auto position = customPlot().yAxis->pixelToCoord(event->pos().y());

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

void VisualisationMappingPlot::hoverMoveEvent(QHoverEvent* event)
{
    auto dragType = dragTypeForEvent(event);

    if(dragType == DragType::Min || dragType == DragType::Max)
        setCursor(Qt::SizeVerCursor);
    else if(dragType == DragType::Move)
        setCursor(Qt::SizeAllCursor);
    else
        setCursor({});
}
