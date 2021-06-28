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

void CorrelationPlotItem::setContinousYAxisRange(double min, double max)
{
    if(_includeYZero)
    {
        if(min > 0.0)
            min = 0.0;
        else if(max < 0.0)
            max = 0.0;
    }

    _continuousYAxis->setRange(min, max);
}

void CorrelationPlotItem::setDispersionVisualType(int dispersionVisualType)
{
    if(_dispersionVisualType != dispersionVisualType)
    {
        _dispersionVisualType = dispersionVisualType;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setIncludeYZero(bool includeYZero)
{
    if(_includeYZero != includeYZero)
    {
        _includeYZero = includeYZero;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setScaleType(int scaleType)
{
    if(_scaleType != scaleType)
    {
        _scaleType = scaleType;
        emit plotOptionsChanged();
        rebuildPlot(InvalidateCache::Yes);
    }
}

void CorrelationPlotItem::setScaleByAttributeName(const QString& attributeName)
{
    if(_scaleByAttributeName != attributeName || _scaleType != static_cast<int>(PlotScaleType::ByAttribute))
    {
        _scaleByAttributeName = attributeName;
        _scaleType = static_cast<int>(PlotScaleType::ByAttribute);
        emit plotOptionsChanged();
        rebuildPlot(InvalidateCache::Yes);
    }
}

void CorrelationPlotItem::setAveragingType(int averagingType)
{
    if(_averagingType != averagingType)
    {
        _averagingType = averagingType;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setAveragingAttributeName(const QString& attributeName)
{
    if(_averagingAttributeName != attributeName)
    {
        _averagingAttributeName = attributeName;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setDispersionType(int dispersionType)
{
    if(_dispersionType != dispersionType)
    {
        _dispersionType = dispersionType;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

QVector<double> CorrelationPlotItem::meanAverageData(double& min, double& max, const QVector<int>& rows)
{
    // Use Average Calculation
    QVector<double> yDataAvg; yDataAvg.reserve(rows.size());

    for(size_t col = 0; col < _pluginInstance->numContinuousColumns(); col++)
    {
        double runningTotal = std::accumulate(rows.begin(), rows.end(), 0.0,
        [this, col](auto partial, auto row)
        {
            return partial + _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));
        });

        yDataAvg.append(runningTotal / rows.length());

        max = std::max(max, yDataAvg.back());
        min = std::min(min, yDataAvg.back());
    }

    return yDataAvg;
}

template<typename Fn>
void addPlotPerAttributeValue(const CorrelationPluginInstance* pluginInstance,
    const QString& nameTemplate, const QString& attributeName,
    const QVector<int>& selectedRows, Fn&& addPlotFn)
{
    QMap<QString, QVector<int>> map;
    for(auto selectedRow : selectedRows)
    {
        const auto value = pluginInstance->attributeValueFor(attributeName, selectedRow);
        map[value].append(selectedRow); // clazy:exclude=reserve-candidates
    }

    const auto keys = map.keys();
    for(const auto& value : keys)
    {
        const auto& rows = map.value(value);
        auto color = CorrelationPlotItem::colorForRows(pluginInstance, rows);

        addPlotFn(color, nameTemplate.arg(attributeName, value), rows);
    }
}

void CorrelationPlotItem::populateMeanLinePlot()
{
    if(_selectedRows.isEmpty())
        return;

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    auto addMeanPlot =
    [this, &minY, &maxY](const QColor& color, const QString& name, const QVector<int>& rows)
    {
        auto* graph = _customPlot.addGraph();
        graph->setPen(QPen(color, 2.0, Qt::DashLine));
        graph->setName(name);

        QVector<double> xData(static_cast<int>(_pluginInstance->numContinuousColumns()));
        // xData is just the column indices
        std::iota(std::begin(xData), std::end(xData), 0);

        // Use Average Calculation and set min / max
        QVector<double> yDataAvg = meanAverageData(minY, maxY, rows);

        graph->setData(xData, yDataAvg, true);

        _meanPlots.append(graph);
        populateDispersion(graph, minY, maxY, rows, yDataAvg);
    };

    if(!_averagingAttributeName.isEmpty())
    {
        addPlotPerAttributeValue(_pluginInstance, tr("Mean average of %1: %2"),
            _averagingAttributeName, _selectedRows, addMeanPlot);
    }
    else
    {
        addMeanPlot(CorrelationPlotItem::colorForRows(_pluginInstance, _selectedRows),
            tr("Mean average of selection"), _selectedRows);
    }

    setContinousYAxisRange(minY, maxY);
}

void CorrelationPlotItem::populateMedianLinePlot()
{
    if(_selectedRows.isEmpty())
        return;

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    auto addMedianPlot =
    [this, &minY, &maxY](const QColor& color, const QString& name, const QVector<int>& rows)
    {
        auto* graph = _customPlot.addGraph();
        graph->setPen(QPen(color, 2.0, Qt::DashLine));
        graph->setName(name);

        QVector<double> xData(static_cast<int>(_pluginInstance->numContinuousColumns()));
        // xData is just the column indices
        std::iota(std::begin(xData), std::end(xData), 0);

        QVector<double> rowsEntries(rows.length());
        QVector<double> yDataAvg(static_cast<int>(_pluginInstance->numContinuousColumns()));

        for(int col = 0; col < static_cast<int>(_pluginInstance->numContinuousColumns()); col++)
        {
            rowsEntries.clear();
            std::transform(rows.begin(), rows.end(), std::back_inserter(rowsEntries),
            [this, col](auto row)
            {
                return _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));
            });

            if(!rows.empty())
            {
                std::sort(rowsEntries.begin(), rowsEntries.end());
                double median = 0.0;
                if(rowsEntries.length()  % 2 == 0)
                    median = (rowsEntries[rowsEntries.length() / 2 - 1] + rowsEntries[rowsEntries.length() / 2]) / 2.0;
                else
                    median = rowsEntries[rowsEntries.length() / 2];

                yDataAvg[col] = median;

                maxY = std::max(maxY, yDataAvg[col]);
                minY = std::min(minY, yDataAvg[col]);
            }
        }

        graph->setData(xData, yDataAvg, true);

        _meanPlots.append(graph);
        populateDispersion(graph, minY, maxY, rows, yDataAvg);
    };

    if(!_averagingAttributeName.isEmpty())
    {
        addPlotPerAttributeValue(_pluginInstance, tr("Median average of %1: %2"),
            _averagingAttributeName, _selectedRows, addMedianPlot);
    }
    else
    {
        addMedianPlot(CorrelationPlotItem::colorForRows(_pluginInstance, _selectedRows),
            tr("Median average of selection"), _selectedRows);
    }

    setContinousYAxisRange(minY, maxY);
}

void CorrelationPlotItem::populateMeanHistogramPlot()
{
    if(_selectedRows.isEmpty())
        return;

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    auto addMeanBars =
    [this, &minY, &maxY](const QColor& color, const QString& name, const QVector<int>& rows)
    {
        QVector<double> xData(static_cast<int>(_pluginInstance->numContinuousColumns()));
        // xData is just the column indices
        std::iota(std::begin(xData), std::end(xData), 0);

        // Use Average Calculation and set min / max
        QVector<double> yDataAvg = meanAverageData(minY, maxY, rows);

        auto* histogramBars = new QCPBars(_continuousXAxis, _continuousYAxis);
        histogramBars->setName(name);
        histogramBars->setData(xData, yDataAvg, true);
        histogramBars->setPen(QPen(color.darker(150)));

        auto innerColor = color.lighter(110);

        // If the value is 0, i.e. the color is black, .lighter won't have
        // had any effect, so just pick an arbitrary higher value
        if(innerColor.value() == 0)
            innerColor.setHsv(innerColor.hue(), innerColor.saturation(), 92);

        histogramBars->setBrush(innerColor);

        setContinousYAxisRange(minY, maxY);

        _meanPlots.append(histogramBars);
        populateDispersion(histogramBars, minY, maxY, rows, yDataAvg);
    };

    if(!_averagingAttributeName.isEmpty())
    {
        addPlotPerAttributeValue(_pluginInstance, tr("Median histogram of %1: %2"),
            _averagingAttributeName, _selectedRows, addMeanBars);

        auto* barsGroup = new QCPBarsGroup(&_customPlot);
        barsGroup->setSpacingType(QCPBarsGroup::stAbsolute);
        barsGroup->setSpacing(1.0);

        for(auto* plottable : std::as_const(_meanPlots))
        {
            auto* bars = dynamic_cast<QCPBars*>(plottable);
            bars->setWidth(bars->width() / _meanPlots.size());
            barsGroup->append(bars);
        }
    }
    else
    {
        addMeanBars(colorForRows(_pluginInstance, _selectedRows),
            tr("Mean histogram of selection"), _selectedRows);
    }

    setContinousYAxisRange(minY, maxY);
}

static double medianOf(const QVector<double>& sortedData)
{
    if(sortedData.length() == 0)
        return 0.0;

    double median = 0.0;
    if(sortedData.length() % 2 == 0)
        median = (sortedData[sortedData.length() / 2 - 1] + sortedData[sortedData.length() / 2]) / 2.0;
    else
        median = sortedData[sortedData.length() / 2];

    return median;
}

void CorrelationPlotItem::populateIQRPlot()
{
    // Box-plots representing the IQR.
    // Whiskers represent the maximum and minimum non-outlier values
    // Outlier values are (< Q1 - 1.5IQR and > Q3 + 1.5IQR)

    auto* statPlot = new QCPStatisticalBox(_continuousXAxis, _continuousYAxis);
    statPlot->setName(tr("Median (IQR plots) of selection"));

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    QVector<double> rowsEntries(_selectedRows.length());
    QVector<double> outliers;

    // Calculate IQRs, outliers and ranges
    for(int col = 0; col < static_cast<int>(_pluginInstance->numContinuousColumns()); col++)
    {
        rowsEntries.clear();
        outliers.clear();
        const auto& selectedRows = std::as_const(_selectedRows);
        std::transform(selectedRows.begin(), selectedRows.end(), std::back_inserter(rowsEntries),
        [this, col](auto row)
        {
            return _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));
        });

        if(!_selectedRows.empty())
        {
            std::sort(rowsEntries.begin(), rowsEntries.end());
            double secondQuartile = medianOf(rowsEntries);
            double firstQuartile = secondQuartile;
            double thirdQuartile = secondQuartile;

            // Don't calculate medians if there's only one sample!
            if(rowsEntries.size() > 1)
            {
                if(rowsEntries.size() % 2 == 0)
                {
                    firstQuartile = medianOf(rowsEntries.mid(0, (rowsEntries.size() / 2)));
                    thirdQuartile = medianOf(rowsEntries.mid((rowsEntries.size() / 2)));
                }
                else
                {
                    firstQuartile = medianOf(rowsEntries.mid(0, ((rowsEntries.size() - 1) / 2)));
                    thirdQuartile = medianOf(rowsEntries.mid(((rowsEntries.size() + 1) / 2)));
                }
            }

            double iqr = thirdQuartile - firstQuartile;
            double minValue = secondQuartile;
            double maxValue = secondQuartile;

            for(auto row : std::as_const(rowsEntries))
            {
                // Find Maximum and minimum non-outliers
                if(row < thirdQuartile + (iqr * 1.5))
                    maxValue = std::max(maxValue, row);

                if(row > firstQuartile - (iqr * 1.5))
                    minValue = std::min(minValue, row);

                // Find outliers
                if(row > thirdQuartile + (iqr * 1.5) ||
                    row < firstQuartile - (iqr * 1.5))
                {
                    outliers.push_back(row);
                }

                maxY = std::max(maxY, row);
                minY = std::min(minY, row);
            }

            // Add data for each column individually because setData doesn't let us do outliers(??)
            statPlot->addData(col, minValue, firstQuartile, secondQuartile, thirdQuartile,
                              maxValue, outliers);
        }
    }

    setContinousYAxisRange(minY, maxY);
}

void CorrelationPlotItem::plotDispersion(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<double>& stdDevs, const QString& name = QStringLiteral("Deviation"))
{
    auto visualType = static_cast<PlotDispersionVisualType>(_dispersionVisualType);
    if(visualType == PlotDispersionVisualType::Bars)
    {
        auto* stdDevBars = new QCPErrorBars(_continuousXAxis, _continuousYAxis);
        stdDevBars->setName(name);
        stdDevBars->setSelectable(QCP::SelectionType::stNone);
        stdDevBars->setAntialiased(false);
        stdDevBars->setDataPlottable(meanPlot);
        stdDevBars->setData(stdDevs);
    }
    else if(visualType == PlotDispersionVisualType::Area)
    {
        auto* devTop = new QCPGraph(_continuousXAxis, _continuousYAxis);
        auto* devBottom = new QCPGraph(_continuousXAxis, _continuousYAxis);
        devTop->setName(QStringLiteral("%1 Top").arg(name));
        devBottom->setName(QStringLiteral("%1 Bottom").arg(name));

        auto fillColour = meanPlot->pen().color();
        auto penColour = meanPlot->pen().color().lighter(150);
        fillColour.setAlpha(50);
        penColour.setAlpha(120);

        devTop->setChannelFillGraph(devBottom);
        devTop->setBrush(QBrush(fillColour));
        devTop->setPen(QPen(penColour));

        devBottom->setPen(QPen(penColour));

        devBottom->setSelectable(QCP::SelectionType::stNone);
        devTop->setSelectable(QCP::SelectionType::stNone);

        auto topErr = QVector<double>(static_cast<int>(_pluginInstance->numContinuousColumns()));
        auto bottomErr = QVector<double>(static_cast<int>(_pluginInstance->numContinuousColumns()));

        for(int i = 0; i < static_cast<int>(_pluginInstance->numContinuousColumns()); i++)
        {
            topErr[i] = meanPlot->interface1D()->dataMainValue(i) + stdDevs[i];
            bottomErr[i] = meanPlot->interface1D()->dataMainValue(i) - stdDevs[i];
        }

        // xData is just the column indices
        QVector<double> xData(static_cast<int>(_pluginInstance->numContinuousColumns()));
        std::iota(std::begin(xData), std::end(xData), 0);

        devTop->setData(xData, topErr);
        devBottom->setData(xData, bottomErr);
    }

    for(int i = 0; i < static_cast<int>(_pluginInstance->numContinuousColumns()); i++)
    {
        minY = std::min(minY, meanPlot->interface1D()->dataMainValue(i) - stdDevs[i]);
        maxY = std::max(maxY, meanPlot->interface1D()->dataMainValue(i) + stdDevs[i]);
    }
}

void CorrelationPlotItem::populateStdDevPlot(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<int>& rows, QVector<double>& means)
{
    QVector<double> stdDevs(static_cast<int>(_pluginInstance->numContinuousColumns()));

    for(int col = 0; col < static_cast<int>(_pluginInstance->numContinuousColumns()); col++)
    {
        double stdDev = 0.0;
        for(auto row : rows)
        {
            auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col])) - means.at(col);
            stdDev += (value * value);
        }

        stdDev /= static_cast<double>(_pluginInstance->numContinuousColumns());
        stdDev = std::sqrt(stdDev);
        stdDevs[col] = stdDev;
    }

    plotDispersion(meanPlot, minY, maxY, stdDevs, QStringLiteral("Std Dev"));
}

void CorrelationPlotItem::populateStdErrorPlot(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<int>& rows, QVector<double>& means)
{
    QVector<double> stdErrs(static_cast<int>(_pluginInstance->numContinuousColumns()));

    for(int col = 0; col < static_cast<int>(_pluginInstance->numContinuousColumns()); col++)
    {
        double stdErr = 0.0;
        for(auto row : rows)
        {
            auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col])) - means.at(col);
            stdErr += (value * value);
        }

        stdErr /= static_cast<double>(_pluginInstance->numContinuousColumns());
        stdErr = std::sqrt(stdErr) / std::sqrt(static_cast<double>(rows.length()));
        stdErrs[col] = stdErr;
    }

    plotDispersion(meanPlot, minY, maxY, stdErrs, QStringLiteral("Std Err"));
}

void CorrelationPlotItem::populateDispersion(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<int>& rows, QVector<double>& means)
{
    auto averagingType = static_cast<PlotAveragingType>(_averagingType);
    auto dispersionType = static_cast<PlotDispersionType>(_dispersionType);

    if(averagingType == PlotAveragingType::Individual || averagingType == PlotAveragingType::IQRPlot)
        return;

    if(dispersionType == PlotDispersionType::StdDev)
        populateStdDevPlot(meanPlot, minY, maxY, rows, means);
    else if(dispersionType == PlotDispersionType::StdErr)
        populateStdErrorPlot(meanPlot, minY, maxY, rows, means);
}

void CorrelationPlotItem::populateLinePlot()
{
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    QVector<double> yData; yData.reserve(_selectedRows.size());
    QVector<double> xData; xData.reserve(static_cast<int>(_pluginInstance->numContinuousColumns()));

    // Plot each row individually
    for(auto row : std::as_const(_selectedRows))
    {
        QCPGraph* graph = nullptr;
        double rowMinY = std::numeric_limits<double>::max();
        double rowMaxY = std::numeric_limits<double>::lowest();

        if(!_lineGraphCache.contains(row))
        {
            graph = _customPlot.addGraph(_continuousXAxis, _continuousYAxis);
            graph->setLayer(_lineGraphLayer);

            double rowSum = 0.0;
            for(size_t col = 0; col < _pluginInstance->numContinuousColumns(); col++)
                rowSum += _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));

            double rowMean = rowSum / _pluginInstance->numContinuousColumns();

            double variance = 0.0;
            for(size_t col = 0; col < _pluginInstance->numContinuousColumns(); col++)
            {
                auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col])) - rowMean;
                variance += (value * value);
            }

            variance /= _pluginInstance->numContinuousColumns();
            double stdDev = std::sqrt(variance);
            double pareto = std::sqrt(stdDev);

            double attributeValue = 1.0;

            if(static_cast<PlotScaleType>(_scaleType) == PlotScaleType::ByAttribute && !_scaleByAttributeName.isEmpty())
            {
                attributeValue = u::toNumber(_pluginInstance->attributeValueFor(_scaleByAttributeName, row));

                if(attributeValue == 0.0 || !std::isfinite(attributeValue))
                    attributeValue = 1.0;
            }

            yData.clear();
            xData.clear();

            for(size_t col = 0; col < _pluginInstance->numContinuousColumns(); col++)
            {
                auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap[col]));

                switch(static_cast<PlotScaleType>(_scaleType))
                {
                case PlotScaleType::Log:
                {
                    // LogY(x+c) where c is EPSILON
                    // This prevents LogY(0) which is -inf
                    // Log2(0+c) = -1057
                    // Document this!
                    const double EPSILON = std::nextafter(0.0, 1.0);
                    value = std::log(value + EPSILON);
                }
                    break;
                case PlotScaleType::MeanCentre:
                    value -= rowMean;
                    break;
                case PlotScaleType::UnitVariance:
                    value -= rowMean;
                    value /= stdDev;
                    break;
                case PlotScaleType::Pareto:
                    value -= rowMean;
                    value /= pareto;
                    break;
                case PlotScaleType::ByAttribute:
                    value /= attributeValue;
                    break;
                default:
                    break;
                }

                xData.append(static_cast<double>(col));
                yData.append(value);

                rowMinY = std::min(rowMinY, value);
                rowMaxY = std::max(rowMaxY, value);
            }

            graph->setData(xData, yData, true);

            _lineGraphCache.insert(row, {graph, rowMinY, rowMaxY});
        }
        else
        {
            const auto& v = _lineGraphCache.value(row);
            graph = v._graph;
            rowMinY = v._minY;
            rowMaxY = v._maxY;
        }

        minY = std::min(minY, rowMinY);
        maxY = std::max(maxY, rowMaxY);

        graph->setVisible(true);
        graph->setSelectable(QCP::SelectionType::stWhole);

        graph->setPen(_pluginInstance->nodeColorForRow(row));
        graph->setName(_pluginInstance->rowName(row));
    }

    setContinousYAxisRange(minY, maxY);
}

void CorrelationPlotItem::configureContinuousAxisRect()
{
    if(_continuousAxisRect == nullptr)
    {
        _continuousAxisRect = new QCPAxisRect(&_customPlot);
        _continuousXAxis = _continuousAxisRect->axis(QCPAxis::atBottom);
        _continuousYAxis = _continuousAxisRect->axis(QCPAxis::atLeft);

        for(auto& axis : _continuousAxisRect->axes())
        {
            axis->setLayer(QStringLiteral("axes"));
            axis->grid()->setLayer(QStringLiteral("grid"));
        }

        _axesLayoutGrid->addElement(_continuousAxisRect);

        // Layer to keep individual line plots separate from everything else
        _customPlot.addLayer(QStringLiteral("lineGraphLayer"));
        _lineGraphLayer = _customPlot.layer(QStringLiteral("lineGraphLayer"));

        // Don't show an emphasised vertical zero line
        _continuousXAxis->grid()->setZeroLinePen(_continuousXAxis->grid()->pen());
    }

    auto plotAveragingType = static_cast<PlotAveragingType>(_averagingType);

    switch(plotAveragingType)
    {
    case PlotAveragingType::MeanLine:       populateMeanLinePlot(); break;
    case PlotAveragingType::MedianLine:     populateMedianLinePlot(); break;
    case PlotAveragingType::MeanHistogram:  populateMeanHistogramPlot(); break;
    case PlotAveragingType::IQRPlot:        populateIQRPlot(); break;
    default:                                populateLinePlot(); break;
    }

    _continuousXAxis->grid()->setVisible(_showGridLines);
    _continuousYAxis->grid()->setVisible(_showGridLines);

    if(_discreteAxisRect == nullptr)
        _continuousYAxis->setLabel(_yAxisLabel);

    auto* xAxis = configureColumnAnnotations(_continuousAxisRect);

    xAxis->setTickLabelRotation(90);
    xAxis->setTickLabels(_showColumnNames && (_elideLabelWidth > 0));

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);

    for(size_t x = 0U; x < _pluginInstance->numContinuousColumns(); x++)
    {
        auto labelName = elideLabel(_pluginInstance->columnName(static_cast<int>(_sortMap[x])));
        categoryTicker->addTick(x, labelName);
    }

    xAxis->setTicker(categoryTicker);
    xAxis->setPadding(_xAxisLabel.isEmpty() ? _xAxisPadding : 0);
}

bool CorrelationPlotItem::continuousTooltip(const QCPAxisRect* axisRect,
    const QCPAbstractPlottable* plottable, double xCoord)
{
    if(axisRect != _continuousAxisRect || plottable == nullptr)
        return false;

    if(const auto* graph = dynamic_cast<const QCPGraph*>(plottable))
    {
        _itemTracer->setGraph(const_cast<QCPGraph*>(graph)); // NOLINT cppcoreguidelines-pro-type-const-cast
        _itemTracer->setGraphKey(xCoord);
        return true;
    }

    if(const auto* bars = dynamic_cast<const QCPBars*>(plottable))
    {
        _itemTracer->position->setPixelPosition(bars->dataPixelPosition(static_cast<int>(xCoord)));
        return true;
    }

    if(const auto* boxPlot = dynamic_cast<const QCPStatisticalBox*>(plottable))
    {
        // Only show simple tooltips for now, can extend this later...
        _itemTracer->position->setPixelPosition(boxPlot->dataPixelPosition(static_cast<int>(xCoord)));
        return true;
    }

    return false;
}
