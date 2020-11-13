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

#include "graphsizeestimateplotitem.h"

#include "shared/utils/utils.h"
#include "shared/utils/string.h"

#include <QVariantMap>
#include <QVector>

#include <algorithm>

GraphSizeEstimatePlotItem::GraphSizeEstimatePlotItem(QQuickItem* parent) :
    QQuickPaintedItem(parent)
{
    _customPlot.setOpenGl(true);

    setAcceptedMouseButtons(Qt::AllButtons);
    setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &QQuickPaintedItem::widthChanged, this, &GraphSizeEstimatePlotItem::updatePlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &GraphSizeEstimatePlotItem::updatePlotSize);
    connect(&_customPlot, &QCustomPlot::afterReplot, this, &GraphSizeEstimatePlotItem::onReplot);
}

void GraphSizeEstimatePlotItem::paint(QPainter* painter)
{
    if(_keys.isEmpty())
        return;

    painter->drawPixmap(0, 0, _customPlot.toPixmap());
}

double GraphSizeEstimatePlotItem::threshold() const
{
    return _threshold;
}

void GraphSizeEstimatePlotItem::setThreshold(double threshold)
{
    if(threshold != _threshold)
    {
        if(!_keys.isEmpty())
            threshold = std::clamp(threshold, std::as_const(_keys).first(), 1.0);

        _threshold = threshold;
        emit thresholdChanged();

        updateThresholdIndicator();
        _customPlot.replot(QCustomPlot::rpQueuedReplot);
    }
}

void GraphSizeEstimatePlotItem::setGraphSizeEstimate(const QVariantMap& graphSizeEstimate)
{
    if(!graphSizeEstimate.contains(QStringLiteral("keys")))
        return;

    if(!graphSizeEstimate.contains(QStringLiteral("numNodes")))
        return;

    if(!graphSizeEstimate.contains(QStringLiteral("numEdges")))
        return;

    _keys = graphSizeEstimate.value(QStringLiteral("keys")).value<QVector<double>>();
    _numNodes = graphSizeEstimate.value(QStringLiteral("numNodes")).value<QVector<double>>();
    _numEdges = graphSizeEstimate.value(QStringLiteral("numEdges")).value<QVector<double>>();

    buildPlot();
}

void GraphSizeEstimatePlotItem::updateThresholdIndicator()
{
    if(_thresholdIndicator == nullptr || _keys.isEmpty())
        return;

    _thresholdIndicator->point1->setCoords(_threshold, 0.0);
    _thresholdIndicator->point2->setCoords(_threshold, 1.0);

    int index = 0;
    while(_keys.at(index) < _threshold && index < _keys.size() - 1)
        index++;

    index++;

    size_t numNodes = 0;
    size_t numEdges = 0;

    if(index < _keys.size())
    {
        numNodes = static_cast<size_t>(_numNodes.at(index));
        numEdges = static_cast<size_t>(_numEdges.at(index));
    }

    _customPlot.xAxis->setLabel(tr("Estimated Graph Size: %1 Nodes, %2 Edges")
        .arg(u::formatNumberSIPostfix(numNodes), u::formatNumberSIPostfix(numEdges)));
}

void GraphSizeEstimatePlotItem::buildPlot()
{
    if(_keys.isEmpty())
        return;

    _customPlot.clearItems();
    _customPlot.clearPlottables();

    auto* nodesGraph = _customPlot.addGraph();
    auto* edgesGraph = _customPlot.addGraph();

    nodesGraph->setData(_keys, _numNodes, true);
    nodesGraph->setPen(QPen(Qt::red));
    nodesGraph->setName(tr("Nodes"));
    edgesGraph->setData(_keys, _numEdges, true);
    edgesGraph->setPen(QPen(Qt::blue));
    edgesGraph->setName(tr("Edges"));

    _thresholdIndicator = new QCPItemStraightLine(&_customPlot);

    QPen indicatorPen;
    indicatorPen.setStyle(Qt::DashLine);
    _thresholdIndicator->setPen(indicatorPen);

    updateThresholdIndicator();

    _customPlot.yAxis->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    logTicker->setLogBase(10);
    logTicker->setSubTickCount(3);
    _customPlot.yAxis->setTicker(logTicker);
    _customPlot.yAxis->setNumberFormat(QStringLiteral("eb"));
    _customPlot.yAxis->setNumberPrecision(0);
    _customPlot.yAxis->grid()->setSubGridVisible(true);

    _customPlot.xAxis->setRange(std::as_const(_keys).first(),
        // If the threshold is 1.0, stretch the X axis a little,
        // so that the marker is always visible
        qFuzzyCompare(_threshold, 1.0) ? 1.001 : 1.0);

    _customPlot.yAxis->rescale();

    _customPlot.legend->setVisible(true);
    _customPlot.legend->setBrush(QBrush(QColor(255, 255, 255, 127)));
    _customPlot.axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignBottom);

    _customPlot.replot(QCustomPlot::rpQueuedReplot);
}

void GraphSizeEstimatePlotItem::updatePlotSize()
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

void GraphSizeEstimatePlotItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        _dragging = true;
        auto xValue = _customPlot.xAxis->pixelToCoord(event->pos().x());
        setThreshold(xValue);
    }
}

void GraphSizeEstimatePlotItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
        _dragging = false;
}

void GraphSizeEstimatePlotItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(_dragging)
    {
        auto xValue = _customPlot.xAxis->pixelToCoord(event->pos().x());
        setThreshold(xValue);
    }
}

void GraphSizeEstimatePlotItem::routeMouseEvent(QMouseEvent* event)
{
    auto* newEvent = new QMouseEvent(event->type(), event->localPos(),
                                     event->button(), event->buttons(),
                                     event->modifiers());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void GraphSizeEstimatePlotItem::onReplot()
{
    update();
}
