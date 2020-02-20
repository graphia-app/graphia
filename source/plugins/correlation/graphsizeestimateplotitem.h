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

#ifndef GRAPHSIZEESTIMATEPLOTITEM_H
#define GRAPHSIZEESTIMATEPLOTITEM_H

#include <qcustomplot.h>

#include <QObject>
#include <QQuickPaintedItem>
#include <QVector>

class GraphSizeEstimatePlotItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap graphSizeEstimate WRITE setGraphSizeEstimate)
    Q_PROPERTY(double threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)

public:
    explicit GraphSizeEstimatePlotItem(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

private:
    QCustomPlot _customPlot;
    QCPItemStraightLine* _thresholdIndicator = nullptr;

    QVector<double> _keys;
    QVector<double> _numNodes;
    QVector<double> _numEdges;

    double _threshold = 0.0;
    bool _dragging = false;

    double threshold() const;
    void setThreshold(double threshold);

    void setGraphSizeEstimate(const QVariantMap& graphSizeEstimate);

    void updateThresholdIndicator();
    void buildPlot();
    void updatePlotSize();

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void routeMouseEvent(QMouseEvent* event);

private slots:
    void onReplot();

signals:
    void thresholdChanged();
};

#endif // GRAPHSIZEESTIMATEPLOTITEM_H
