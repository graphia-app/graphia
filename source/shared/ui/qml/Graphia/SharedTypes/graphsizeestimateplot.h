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

#ifndef GRAPHSIZEESTIMATEPLOT_H
#define GRAPHSIZEESTIMATEPLOT_H

#include <qcustomplotquickitem.h>

#include <QObject>
#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QVector>

class GraphSizeEstimatePlot : public QCustomPlotQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantMap graphSizeEstimate READ graphSizeEstimate WRITE setGraphSizeEstimate) // clazy:exclude=qproperty-without-notify
    Q_PROPERTY(double threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)
    Q_PROPERTY(bool uniqueEdgesOnly MEMBER _uniqueEdgesOnly NOTIFY uniqueEdgesOnlyChanged)
    Q_PROPERTY(bool integralThreshold MEMBER _integralThreshold)

public:
    explicit GraphSizeEstimatePlot(QQuickItem* parent = nullptr);

private:
    QCPItemStraightLine* _thresholdIndicator = nullptr;

    QVector<double> _keys;
    QVector<double> _numNodes;
    QVector<double> _numEdges;
    QVector<double> _numUniqueEdges;

    double _threshold = 0.0;
    bool _uniqueEdgesOnly = false;
    bool _integralThreshold = false;
    bool _dragging = false;

    double threshold() const;
    void setThreshold(double threshold);

    QVariantMap graphSizeEstimate() const { return {}; } // Silence AutoMoc warning
    void setGraphSizeEstimate(const QVariantMap& graphSizeEstimate);

    void updateThresholdIndicator();
    void buildPlot() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

signals:
    void thresholdChanged();
    void uniqueEdgesOnlyChanged();
};

#endif // GRAPHSIZEESTIMATEPLOT_H
