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

#include "graphsizeestimateplot.h"

#include "shared/utils/utils.h"
#include "shared/utils/string.h"
#include "shared/rendering/multisamples.h"

#include <QVariantMap>
#include <QVector>
#include <QGuiApplication>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

GraphSizeEstimatePlot::GraphSizeEstimatePlot(QQuickItem* parent) :
    QCustomPlotQuickItem(multisamples(), parent)
{
    connect(this, &GraphSizeEstimatePlot::uniqueEdgesOnlyChanged, [this] { buildPlot(); });
}

double GraphSizeEstimatePlot::threshold() const
{
    return _threshold;
}

void GraphSizeEstimatePlot::setThreshold(double threshold)
{
    if(_integralThreshold)
        threshold = std::round(threshold);

    if(threshold != _threshold)
    {
        _threshold = threshold;
        emit thresholdChanged();

        updateThresholdIndicator();
        customPlot().replot(QCustomPlot::rpQueuedReplot);
    }
}

void GraphSizeEstimatePlot::setGraphSizeEstimate(const QVariantMap& graphSizeEstimate)
{
    if(!graphSizeEstimate.contains(u"keys"_s))
        return;

    if(!graphSizeEstimate.contains(u"numNodes"_s))
        return;

    if(!graphSizeEstimate.contains(u"numEdges"_s))
        return;

    if(!graphSizeEstimate.contains(u"numUniqueEdges"_s))
        return;

    _keys = graphSizeEstimate.value(u"keys"_s).value<QVector<double>>();
    _numNodes = graphSizeEstimate.value(u"numNodes"_s).value<QVector<double>>();
    _numEdges = graphSizeEstimate.value(u"numEdges"_s).value<QVector<double>>();
    _numUniqueEdges = graphSizeEstimate.value(u"numUniqueEdges"_s).value<QVector<double>>();

    buildPlot();
}

void GraphSizeEstimatePlot::updateThresholdIndicator()
{
    if(_thresholdIndicator == nullptr || _keys.isEmpty())
        return;

    // Clamp the indicator x coordinate to within the bounds of the
    // plot by a small margin, so that it's always visible
    auto firstKey = std::as_const(_keys).first();
    auto lastKey = std::as_const(_keys).last();
    auto [smallestKey, largestKey] = std::minmax(firstKey, lastKey);

    const double keepVisibleOffset = (largestKey - smallestKey) * 0.002;

    auto x = std::clamp(_threshold,
        smallestKey + keepVisibleOffset,
        largestKey - keepVisibleOffset);

    _thresholdIndicator->point1->setCoords(x, 0.0);
    _thresholdIndicator->point2->setCoords(x, 1.0);

    int index = 0;
    double minDiff = std::numeric_limits<double>::max();
    for(int i = 0; i < _keys.size(); i++)
    {
        auto diff = std::abs(_keys.at(i) - _threshold);
        if(diff < minDiff)
        {
            minDiff = diff;
            index = i;
        }
    }

    size_t numNodes = 0;
    size_t numEdges = 0;

    if(index < _keys.size())
    {
        numNodes = static_cast<size_t>(_numNodes.at(index));
        numEdges = _uniqueEdgesOnly ?
            static_cast<size_t>(_numUniqueEdges.at(index)) :
            static_cast<size_t>(_numEdges.at(index));
    }

    customPlot().xAxis->setLabel(tr("Estimated Graph Size: %1 Nodes, %2 Edges")
        .arg(u::formatNumberSIPostfix(static_cast<double>(numNodes)),
        u::formatNumberSIPostfix(static_cast<double>(numEdges))));
}

void GraphSizeEstimatePlot::buildPlot()
{
    if(_keys.isEmpty())
        return;

    customPlot().clearItems();
    customPlot().clearPlottables();

    auto* nodesGraph = customPlot().addGraph();
    auto* edgesGraph = customPlot().addGraph();

    auto nodesColor = QColor(Qt::red);
    auto edgesColor = QColor(Qt::blue);

    if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
    {
        nodesColor = nodesColor.lighter();
        edgesColor = edgesColor.lighter();
    }

    nodesGraph->setData(_keys, _numNodes, true);
    nodesGraph->setPen(QPen(nodesColor));
    nodesGraph->setName(tr("Nodes"));
    edgesGraph->setData(_keys, _uniqueEdgesOnly ? _numUniqueEdges : _numEdges, true);
    edgesGraph->setPen(QPen(edgesColor));
    edgesGraph->setName(tr("Edges"));

    _thresholdIndicator = new QCPItemStraightLine(&customPlot());

    QPen indicatorPen;
    indicatorPen.setStyle(Qt::DashLine);
    indicatorPen.setColor(penColor());
    _thresholdIndicator->setPen(indicatorPen);

    updateThresholdIndicator();

    customPlot().yAxis->setScaleType(QCPAxis::stLogarithmic);
    const QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    logTicker->setLogBase(10);
    logTicker->setSubTickCount(3);
    customPlot().yAxis->setTicker(logTicker);
    customPlot().yAxis->setNumberFormat(u"eb"_s);
    customPlot().yAxis->setNumberPrecision(0);
    customPlot().yAxis->grid()->setSubGridVisible(true);

    auto smallestKey = std::as_const(_keys).first();
    auto largestKey = std::as_const(_keys).last();

    customPlot().xAxis->setRange(smallestKey, largestKey);
    customPlot().xAxis->setRangeReversed(largestKey < smallestKey);

    customPlot().yAxis->rescale();

    customPlot().xAxis->setBasePen(QPen(penColor()));
    customPlot().xAxis->setTickPen(QPen(penColor()));
    customPlot().xAxis->setSubTickPen(QPen(penColor()));
    customPlot().xAxis->setTickLabelColor(penColor());
    auto xAxisGridPen = customPlot().xAxis->grid()->pen();
    xAxisGridPen.setColor(lightPenColor());
    customPlot().xAxis->grid()->setPen(xAxisGridPen);
    customPlot().xAxis->setLabelColor(penColor());

    customPlot().yAxis->setBasePen(QPen(penColor()));
    customPlot().yAxis->setTickPen(QPen(penColor()));
    customPlot().yAxis->setSubTickPen(QPen(penColor()));
    customPlot().yAxis->setTickLabelColor(penColor());
    auto yAxisGridPen = customPlot().yAxis->grid()->pen();
    yAxisGridPen.setColor(lightPenColor());
    customPlot().yAxis->grid()->setPen(yAxisGridPen);

    customPlot().legend->setVisible(true);
    customPlot().legend->setBrush(QBrush(QColor(255, 255, 255, 127)));
    customPlot().axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignBottom);

    customPlot().replot(QCustomPlot::rpQueuedReplot);
}

void GraphSizeEstimatePlot::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        _dragging = true;
        auto xValue = customPlot().xAxis->pixelToCoord(event->pos().x());
        setThreshold(xValue);
    }
}

void GraphSizeEstimatePlot::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
        _dragging = false;
}

void GraphSizeEstimatePlot::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(_dragging)
    {
        auto xValue = customPlot().xAxis->pixelToCoord(event->pos().x());
        setThreshold(xValue);
    }
}
