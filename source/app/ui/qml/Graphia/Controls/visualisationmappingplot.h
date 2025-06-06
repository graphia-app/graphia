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

#ifndef VISUALISATIONMAPPINGPLOT_H
#define VISUALISATIONMAPPINGPLOT_H

#include <qcustomplotquickitem.h>

#include "shared/utils/statistics.h"

#include <QObject>
#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QList>

class VisualisationMappingPlot : public QCustomPlotQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    Q_PROPERTY(QList<double> values MEMBER _values WRITE setValues)
    Q_PROPERTY(bool invert MEMBER _invert WRITE setInvert)
    Q_PROPERTY(double exponent MEMBER _exponent WRITE setExponent)
    Q_PROPERTY(double minimum MEMBER _min WRITE setMinimum NOTIFY minimumChanged)
    Q_PROPERTY(double maximum MEMBER _max WRITE setMaximum NOTIFY maximumChanged)

    explicit VisualisationMappingPlot(QQuickItem* parent = nullptr);

    Q_INVOKABLE void setRangeToMinMax();
    Q_INVOKABLE void setRangeToStddev();

private:
    QList<double> _values;
    u::Statistics _statistics;

    bool _invert = false;
    double _exponent = 1.0;
    double _min = 0.0;
    double _max = 1.0;

    void setValues(const QList<double>& values);
    void setInvert(bool invert);
    void setExponent(double exponent);
    void setMinimum(double min);
    void setMaximum(double max);

    void buildPlot() override;

    enum class DragType
    {
        None,
        Min,
        Max,
        Move
    };

    DragType _dragType = DragType::None;
    double _clickPosition = 0.0;
    double _clickMin = 0.0;
    double _clickMax = 0.0;

    template<typename Event>
    DragType dragTypeForEvent(const Event* event) const
    {
        auto minPixel = customPlot().yAxis->coordToPixel(_min);
        auto maxPixel = customPlot().yAxis->coordToPixel(_max);

        auto yPos = static_cast<double>(event->position().y());

        const double hoverThreshold = 5.0;
        const bool overMin = std::abs(minPixel - yPos) < hoverThreshold;
        const bool overMax = std::abs(maxPixel - yPos) < hoverThreshold;

        if(overMin)
            return DragType::Min;

        if(overMax)
            return DragType::Max;

        if(yPos < minPixel && yPos > maxPixel)
            return DragType::Move;

        return DragType::None;
    }

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void hoverMoveEvent(QHoverEvent* event) override;

signals:
    void minimumChanged();
    void maximumChanged();

    void manualChangeToMinMax();
};

#endif // VISUALISATIONMAPPINGPLOT_H
