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

#include "correlationplotitem.h"

#include "correlationplugin.h"

void CorrelationPlotItem::configureDiscreteAxisRect()
{
    if(_discreteAxisRect == nullptr)
    {
        _discreteAxisRect = new QCPAxisRect(&_customPlot);
        _discreteXAxis = _discreteAxisRect->axis(QCPAxis::atBottom);
        _discreteYAxis = _discreteAxisRect->axis(QCPAxis::atLeft);

        for(auto& axis : _discreteAxisRect->axes())
        {
            axis->setLayer(QStringLiteral("axes"));
            axis->grid()->setLayer(QStringLiteral("grid"));
        }

        _axesLayoutGrid->addElement(_discreteAxisRect);

        // Don't show an emphasised vertical zero line
        _discreteXAxis->grid()->setZeroLinePen(_discreteXAxis->grid()->pen());
    }

    _discreteXAxis->grid()->setVisible(_showGridLines);
    _discreteYAxis->grid()->setVisible(_showGridLines);

    if(_continuousAxisRect == nullptr)
        _discreteYAxis->setLabel(_yAxisLabel);

    auto* xAxis = configureColumnAnnotations(_discreteAxisRect);

    xAxis->setTickLabelRotation(90);
    xAxis->setTickLabels(_showColumnNames && (_elideLabelWidth > 0));

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);

    for(size_t x = 0U; x < _pluginInstance->numDiscreteColumns(); x++)
    {
        auto labelName = elideLabel(_pluginInstance->columnName(static_cast<int>(_sortMap[x])));
        categoryTicker->addTick(x, labelName);
    }

    xAxis->setTicker(categoryTicker);
}

bool CorrelationPlotItem::discreteTooltip(const QCPAxisRect* axisRect,
    const QCPAbstractPlottable* plottable, double xCoord)
{
    return false;
}
