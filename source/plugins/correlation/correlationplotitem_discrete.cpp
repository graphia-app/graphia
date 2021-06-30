/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

    std::vector<size_t> columnTotals(_pluginInstance->numDiscreteColumns());
    std::vector<std::map<QString, size_t>> columnData(_pluginInstance->numDiscreteColumns());

    for(size_t column = 0; column < _pluginInstance->numDiscreteColumns(); column++)
    {
        auto& m = columnData[column];

        for(auto row : std::as_const(_selectedRows))
        {
            const auto& value = _pluginInstance->discreteDataAt(row, static_cast<int>(_sortMap[column]));

            if(value.isEmpty())
                continue;

            m[value]++;
            columnTotals[column]++;
        }
    }

    double maxY = *std::max_element(columnTotals.begin(), columnTotals.end());

    ColorPalette colorPalette(Defaults::PALETTE);

    for(size_t column = 0; column < _pluginInstance->numDiscreteColumns(); column++)
    {
        const auto& m = columnData[column];
        QCPBars* last = nullptr;

        auto addBars = [&](const QString& value, double size, QColor color = {})
        {
            auto* bars = new QCPBars(_discreteXAxis, _discreteYAxis);
            bars->addData(static_cast<double>(column), size);

            if(last != nullptr)
                bars->moveAbove(last);

            last = bars;

            bars->setAntialiased(false);
            bars->setName(value);

            if(!color.isValid())
            {
                auto index = _pluginInstance->discreteDataValueIndex(value);
                color = colorPalette.get(value, index);
            }

            bars->setPen(QPen(color.darker(150)));

            auto innerColor = color.lighter(110);

            // If the value is 0, i.e. the color is black, .lighter won't have
            // had any effect, so just pick an arbitrary higher value
            if(innerColor.value() == 0)
                innerColor.setHsv(innerColor.hue(), innerColor.saturation(), 92);

            bars->setBrush(innerColor);
        };

        // When the number of possible values is large, for an arbitrary value of large,
        // performance suffers given the way we're using QCPBars, so in that case just
        // display a single bar that's the sum of all the values it represents
        if(m.size() >= 16)
        {
            size_t totalSize = 0;
            for(const auto& [value, size] : m)
                totalSize += size;

            const auto& keys = u::keysFor(m);
            auto value = QStringLiteral("%1, %2 and %3 more…")
                .arg(keys.at(0), keys.at(1))
                .arg(m.size() - 2);

            addBars(value, totalSize, Qt::black);
        }
        else
        {
            for(const auto& [value, size] : m)
                addBars(value, size);
        }
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
    xAxis->setPadding(_xAxisLabel.isEmpty() ? _xAxisPadding : 0);
}

bool CorrelationPlotItem::discreteTooltip(const QCPAxisRect* axisRect,
    const QCPAbstractPlottable* plottable, double xCoord)
{
    if(axisRect != _discreteAxisRect || plottable == nullptr)
        return false;

    if(const auto* bars = dynamic_cast<const QCPBars*>(plottable))
    {
        _itemTracer->position->setPixelPosition(bars->dataPixelPosition(static_cast<int>(xCoord)));
        return true;
    }

    return false;
}
