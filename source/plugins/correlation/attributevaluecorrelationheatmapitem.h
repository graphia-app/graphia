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

#ifndef ATTRIBUTEVALUECORRELATIONHEATMAPITEM_H
#define ATTRIBUTEVALUECORRELATIONHEATMAPITEM_H

#include "attributevaluecorrelationheatmapworker.h"

#include "shared/graph/covariancematrix.h"

#include <qcustomplotquickitem.h>

#include <QObject>

#include <vector>

class AttributeValueCorrelationHeatmapItem : public QCustomPlotQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int maxNumAttributeValues MEMBER _maxNumAttributeValues WRITE setMaxNumAttributeValues)
    Q_PROPERTY(int numAttributeValues READ numAttributeValues NOTIFY numAttributeValuesChanged)
    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelWidth WRITE setElideLabelWidth)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

private:
    AttributeValueCorrelationHeatmapResult _data;
    std::vector<size_t> _ordering;
    int _maxNumAttributeValues = 20;
    int _elideLabelWidth = 120;

    QCPColorMap* _colorMap = nullptr;
    QCPLayer* _textLayer = nullptr;
    QCPItemText* _tooltipLabel = nullptr;
    QCPAbstractPlottable* _tooltipPlottable = nullptr;
    QPointF _hoverPoint;

    void setMaxNumAttributeValues(int maxNumAttributeValues);
    int numAttributeValues() const;
    void setElideLabelWidth(int elideLabelWidth);

    bool valid() const;

    void showTooltip();
    void hideTooltip();

private slots:
    void updateLabelVisibility();

public:
    explicit AttributeValueCorrelationHeatmapItem(QQuickItem* parent = nullptr);

    Q_INVOKABLE void rebuild(const AttributeValueCorrelationHeatmapResult* data = nullptr);
    Q_INVOKABLE void reset();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;

signals:
    void numAttributeValuesChanged();
    void validChanged();

    void cellClicked(const QString& a, const QString& b);
};

#endif // ATTRIBUTEVALUECORRELATIONHEATMAPITEM_H
