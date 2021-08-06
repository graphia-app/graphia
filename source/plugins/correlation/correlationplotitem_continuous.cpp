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

#include "shared/utils/statistics.h"
#include "shared/utils/container_randomsample.h"

// NOLINTNEXTLINE readability-make-member-function-const
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

void CorrelationPlotItem::setContinousYAxisRangeForSelection()
{
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for(auto row : std::as_const(_selectedRows))
    {
        for(size_t column = 0; column < _pluginInstance->numContinuousColumns(); column++)
        {
            auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap.at(column)));
            maxY = std::max(maxY, value);
            minY = std::min(minY, value);
        }
    }

    setContinousYAxisRange(minY, maxY);
}

void CorrelationPlotItem::setDispersionVisualType(int dispersionVisualType)
{
    dispersionVisualType = static_cast<int>(NORMALISE_QML_ENUM(PlotDispersionVisualType, dispersionVisualType));

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
    scaleType = static_cast<int>(NORMALISE_QML_ENUM(PlotScaleType, scaleType));

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
    averagingType = static_cast<int>(NORMALISE_QML_ENUM(PlotAveragingType, averagingType));

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

void CorrelationPlotItem::setGroupByAnnotation(bool groupByAnnotation)
{
    if(_groupByAnnotation != groupByAnnotation)
    {
        _groupByAnnotation = groupByAnnotation;
        emit plotOptionsChanged();
        rebuildPlot();
    }
}

void CorrelationPlotItem::setDispersionType(int dispersionType)
{
    dispersionType = static_cast<int>(NORMALISE_QML_ENUM(PlotDispersionType, dispersionType));

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
            return partial + _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap.at(col)));
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
                return _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap.at(col)));
            });

            if(!rows.empty())
            {
                yDataAvg[col] = u::medianOf(rowsEntries);

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

static void addIQRBoxPlotTo(QCPAxis* keyAxis, QCPAxis* valueAxis, size_t column, QVector<double> values)
{
    // Box-plots representing the InterQuatile Range
    // Whiskers represent the maximum and minimum non-outlier values
    // Outlier values are (< Q1 - 1.5IQR and > Q3 + 1.5IQR)

    if(values.empty())
        return;

    auto* statisticalBox = new QCPStatisticalBox(keyAxis, valueAxis);
    statisticalBox->setName(QObject::tr("IQR"));

    // Partial (efficient) sort which is suitable for IQR calculation
    auto const Q1 = values.size() / 4;
    auto const Q2 = values.size() / 2;
    auto const Q3 = Q1 + Q2;

    std::nth_element(values.begin(),          values.begin() + Q1, values.end());
    std::nth_element(values.begin() + Q1 + 1, values.begin() + Q2, values.end());
    std::nth_element(values.begin() + Q2 + 1, values.begin() + Q3, values.end());

    double secondQuartile = u::medianOf(values);
    double firstQuartile = secondQuartile;
    double thirdQuartile = secondQuartile;

    // Don't calculate medians if there's only one sample
    if(values.size() > 1)
    {
        if(values.size() % 2 == 0)
        {
            firstQuartile = u::medianOf(values.mid(0, (values.size() / 2)));
            thirdQuartile = u::medianOf(values.mid((values.size() / 2)));
        }
        else
        {
            firstQuartile = u::medianOf(values.mid(0, ((values.size() - 1) / 2)));
            thirdQuartile = u::medianOf(values.mid(((values.size() + 1) / 2)));
        }
    }

    double iqr = thirdQuartile - firstQuartile;
    double minValue = secondQuartile;
    double maxValue = secondQuartile;
    QVector<double> outliers;
    outliers.reserve(values.size());

    for(auto value : std::as_const(values))
    {
        // Find Maximum and minimum non-outliers
        if(value < thirdQuartile + (iqr * 1.5))
            maxValue = std::max(maxValue, value);

        if(value > firstQuartile - (iqr * 1.5))
            minValue = std::min(minValue, value);

        // Find outliers
        if(value > thirdQuartile + (iqr * 1.5) || value < firstQuartile - (iqr * 1.5))
            outliers.push_back(value);
    }

    outliers.shrink_to_fit();

    const size_t maxOutliers = 100;
    if(static_cast<size_t>(outliers.size()) > maxOutliers)
        outliers = u::randomSample(outliers, maxOutliers);

    statisticalBox->addData(static_cast<int>(column), minValue, firstQuartile,
        secondQuartile, thirdQuartile, maxValue, outliers);
}

void CorrelationPlotItem::populateIQRPlot()
{
    for(int column = 0; column < static_cast<int>(_pluginInstance->numContinuousColumns()); column++)
    {
        QVector<double> values;

        const auto& selectedRows = std::as_const(_selectedRows);
        std::transform(selectedRows.begin(), selectedRows.end(), std::back_inserter(values),
        [this, column](auto row)
        {
            return _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap.at(column)));
        });

        addIQRBoxPlotTo(_continuousXAxis, _continuousYAxis, column, std::move(values));
    }

    setContinousYAxisRangeForSelection();
}

void CorrelationPlotItem::populateIQRAnnotationPlot()
{
    for(size_t column = 0; column < _annotationGroupMap.size(); column++)
    {
        QVector<double> values;

        for(auto row : std::as_const(_selectedRows))
        {
            for(size_t groupedColumn : _annotationGroupMap.at(column))
                values.append(_pluginInstance->continuousDataAt(row, static_cast<int>(groupedColumn)));
        }

        addIQRBoxPlotTo(_continuousXAxis, _continuousYAxis, column, std::move(values));
    }

    setContinousYAxisRangeForSelection();
}

void CorrelationPlotItem::plotDispersion(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<double>& stdDevs, const QString& name = tr("Deviation"))
{
    auto visualType = NORMALISE_QML_ENUM(PlotDispersionVisualType, _dispersionVisualType);
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
        devTop->setName(tr("%1 Top").arg(name));
        devBottom->setName(tr("%1 Bottom").arg(name));

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
            auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap.at(col))) - means.at(col);
            stdDev += (value * value);
        }

        stdDev /= static_cast<double>(_pluginInstance->numContinuousColumns());
        stdDev = std::sqrt(stdDev);
        stdDevs[col] = stdDev;
    }

    plotDispersion(meanPlot, minY, maxY, stdDevs, tr("Std Dev"));
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
            auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap.at(col))) - means.at(col);
            stdErr += (value * value);
        }

        stdErr /= static_cast<double>(_pluginInstance->numContinuousColumns());
        stdErr = std::sqrt(stdErr) / std::sqrt(static_cast<double>(rows.length()));
        stdErrs[col] = stdErr;
    }

    plotDispersion(meanPlot, minY, maxY, stdErrs, tr("Std Err"));
}

void CorrelationPlotItem::populateDispersion(QCPAbstractPlottable* meanPlot,
    double& minY, double& maxY,
    const QVector<int>& rows, QVector<double>& means)
{
    if(_groupByAnnotation)
        return;

    auto averagingType = NORMALISE_QML_ENUM(PlotAveragingType, _averagingType);
    auto dispersionType = NORMALISE_QML_ENUM(PlotDispersionType, _dispersionType);

    if(averagingType == PlotAveragingType::Individual || averagingType == PlotAveragingType::IQR)
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
                rowSum += _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap.at(col)));

            double rowMean = rowSum / static_cast<double>(_pluginInstance->numContinuousColumns());

            double variance = 0.0;
            for(size_t col = 0; col < _pluginInstance->numContinuousColumns(); col++)
            {
                auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap.at(col))) - rowMean;
                variance += (value * value);
            }

            variance /= static_cast<double>(_pluginInstance->numContinuousColumns());
            double stdDev = std::sqrt(variance);
            double pareto = std::sqrt(stdDev);

            double attributeValue = 1.0;

            if(NORMALISE_QML_ENUM(PlotScaleType, _scaleType) == PlotScaleType::ByAttribute && !_scaleByAttributeName.isEmpty())
            {
                attributeValue = u::toNumber(_pluginInstance->attributeValueFor(_scaleByAttributeName, row));

                if(attributeValue == 0.0 || !std::isfinite(attributeValue))
                    attributeValue = 1.0;
            }

            yData.clear();
            xData.clear();

            for(size_t col = 0; col < _pluginInstance->numContinuousColumns(); col++)
            {
                auto value = _pluginInstance->continuousDataAt(row, static_cast<int>(_sortMap.at(col)));

                switch(NORMALISE_QML_ENUM(PlotScaleType, _scaleType))
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

    auto plotAveragingType = NORMALISE_QML_ENUM(PlotAveragingType, _averagingType);

    if(!_groupByAnnotation || _visibleColumnAnnotationNames.empty())
    {
        switch(plotAveragingType)
        {
        case PlotAveragingType::MeanLine:       populateMeanLinePlot(); break;
        case PlotAveragingType::MedianLine:     populateMedianLinePlot(); break;
        case PlotAveragingType::MeanHistogram:  populateMeanHistogramPlot(); break;
        case PlotAveragingType::IQR:            populateIQRPlot(); break;
        default:                                populateLinePlot(); break;
        }
    }
    else
        populateIQRAnnotationPlot();

    _continuousXAxis->grid()->setVisible(_showGridLines);
    _continuousYAxis->grid()->setVisible(_showGridLines);

    if(_discreteAxisRect == nullptr)
        _continuousYAxis->setLabel(_yAxisLabel);

    auto* xAxis = configureColumnAnnotations(_continuousAxisRect);

    xAxis->setTickLabelRotation(90);
    xAxis->setTickLabels(showColumnNames() && (_elideLabelWidth > 0));

    if(xAxis->tickLabels())
    {
        QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);

        for(size_t x = 0U; x < _pluginInstance->numContinuousColumns(); x++)
        {
            auto labelName = elideLabel(_pluginInstance->columnName(static_cast<int>(_sortMap.at(x))));
            categoryTicker->addTick(static_cast<double>(x), labelName);
        }

        xAxis->setTicker(categoryTicker);
    }

    xAxis->setPadding(_xAxisLabel.isEmpty() ? _xAxisPadding : 0);
}

bool CorrelationPlotItem::continuousTooltip(const QCPAxisRect* axisRect,
    const QCPAbstractPlottable* plottable, double xCoord)
{
    if(axisRect != _continuousAxisRect || plottable == nullptr)
        return false;

    if(const auto* graph = dynamic_cast<const QCPGraph*>(plottable))
    {
        _itemTracer->setGraph(const_cast<QCPGraph*>(graph));
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
