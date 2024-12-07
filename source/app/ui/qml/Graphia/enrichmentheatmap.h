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

#ifndef ENRICHMENTHEATMAP_H
#define ENRICHMENTHEATMAP_H

#include "app/attributes/enrichmentcalculator.h"

#include <qcustomplotquickitem.h>

#include <QObject>
#include <QQmlEngine>

class EnrichmentHeatmap : public QCustomPlotQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(EnrichmentTableModel* model MEMBER _tableModel NOTIFY tableModelChanged)
    Q_PROPERTY(double scrollXAmount MEMBER _scrollXAmount WRITE setScrollXAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(double scrollYAmount MEMBER _scrollYAmount WRITE setScrollYAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(double horizontalRangeSize READ horizontalRangeSize NOTIFY horizontalRangeSizeChanged)
    Q_PROPERTY(double verticalRangeSize READ verticalRangeSize NOTIFY verticalRangeSizeChanged)
    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelWidth WRITE setElideLabelWidth)
    Q_PROPERTY(QString xAxisLabel MEMBER _xAxisLabel WRITE setXAxisLabel)
    Q_PROPERTY(QString yAxisLabel MEMBER _yAxisLabel WRITE setYAxisLabel)

private:
    QCPLayer* _textLayer = nullptr;
    QCPColorMap* _colorMap = nullptr;
    QCPColorScale* _colorScale = nullptr;
    QCPItemText* _hoverLabel = nullptr;
    QCPAbstractPlottable* _hoverPlottable = nullptr;
    QPointF _hoverPoint;
    std::map<int, QString> _xAxisToFullLabel;
    std::map<int, QString> _yAxisToFullLabel;
    std::map<std::pair<int, int>, int> _colorMapKeyValueToTableIndex;

    EnrichmentTableModel* _tableModel = nullptr;
    QFont _defaultFont9Pt;
    int _attributeACount = 0;
    int _attributeBCount = 0;
    double _scrollXAmount = 0.0;
    double _scrollYAmount = 0.0;
    int _elideLabelWidth = 120;
    QString _xAxisLabel;
    QString _yAxisLabel;

    double horizontalRangeSize();
    double verticalRangeSize();
    double columnAxisWidth();
    double columnLabelSize();
    void scaleXAxis();
    void scaleYAxis();
    double columnAxisHeight();
    void setScrollXAmount(double scrollAmount);
    void setScrollYAmount(double scrollAmount);
    void setElideLabelWidth(int elideLabelWidth);
    void setXAxisLabel(const QString& xAxisLabel);
    void setYAxisLabel(const QString& yAxisLabel);
    void scaleAxes();

public:
    explicit EnrichmentHeatmap(QQuickItem* parent = nullptr);

    Q_INVOKABLE void buildPlot() override;
    Q_INVOKABLE void savePlotImage(const QUrl &url, const QStringList& extensions);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;

signals:
    void rightClick();
    void tableModelChanged();
    void scrollAmountChanged();
    void horizontalRangeSizeChanged();
    void verticalRangeSizeChanged();
    void plotValueClicked(int row);

public slots:
    void showTooltip();
    void hideTooltip();
};

#endif // ENRICHMENTHEATMAP_H
