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

#include "shared/utils/color.h"

#include "shared/ui/visualisations/colorpalette.h"
#include "shared/ui/visualisations/defaultpalettes.h"

#include <map>

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

    std::map<QString, QVector<double>> yData;

    QVector<double> columnTotals(_pluginInstance->numDiscreteColumns());

    for(auto row : std::as_const(_selectedRows))
    {
        for(size_t column = 0; column < _pluginInstance->numDiscreteColumns(); column++)
        {
            const auto& value = _pluginInstance->discreteDataAt(row, static_cast<int>(_sortMap[column]));
            auto& dataRow = yData[value];

            if(dataRow.isEmpty())
                dataRow.resize(_pluginInstance->numDiscreteColumns());

            dataRow[column] += 1.0;
            columnTotals[column] += 1.0;
        }
    }

    double maxY = *std::max_element(columnTotals.begin(), columnTotals.end());
    QCPBars* last = nullptr;

    ColorPalette colorPalette(Defaults::PALETTE);
    int index = 0;

    for(const auto& [value, row] : yData)
    {
        QVector<double> xData(static_cast<int>(_pluginInstance->numDiscreteColumns()));
        // xData is just the column indices
        std::iota(std::begin(xData), std::end(xData), 0);

        auto* bars = new QCPBars(_discreteXAxis, _discreteYAxis);
        bars->setData(xData, row, true);

        if(last != nullptr)
            bars->moveAbove(last);

        last = bars;

        bars->setAntialiased(false);

        bars->setStackingGap(-2.0);
        bars->setPen(QPen({}, 0.0));

        auto color = colorPalette.get(value, index++);
        bars->setBrush(color);
    }

    _discreteYAxis->setRange(0.0, maxY);

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
