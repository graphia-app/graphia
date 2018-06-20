#include "correlationplotitem.h"

#include "shared/utils/thread.h"

#include <QDesktopServices>

#include <cmath>

CorrelationPlotWorker::CorrelationPlotWorker() :
    _numUpdatesQueued(0)
{
    _defaultFont9Pt.setPointSize(9);
}

CorrelationPlotWorker::~CorrelationPlotWorker()
{
    delete _customPlot;
    _customPlot = nullptr;
}

void CorrelationPlotWorker::queueUpdate()
{
    if(++_numUpdatesQueued == 1)
        emit busyChanged();
}

void CorrelationPlotWorker::updateCompleted()
{
    if(--_numUpdatesQueued == 0)
        emit busyChanged();
}

QPixmap CorrelationPlotWorker::pixmap() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock())
        return _pixmap;

    return {};
}

QCPLayer* CorrelationPlotWorker::tooltipLayer()
{
    Q_ASSERT(_customPlot != nullptr);
    return _customPlot->layer(QStringLiteral("tooltipLayer"));
}

void CorrelationPlotWorker::Tooltips::initialise(QCustomPlot* plot, QCPLayer* layer)
{
    QFont defaultFont10Pt;
    defaultFont10Pt.setPointSize(10);

    _hoverLabel = new QCPItemText(plot);
    _hoverLabel->setLayer(layer);
    _hoverLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    _hoverLabel->setFont(defaultFont10Pt);
    _hoverLabel->setPen(QPen(Qt::black));
    _hoverLabel->setBrush(QBrush(Qt::white));
    _hoverLabel->setPadding(QMargins(3, 3, 3, 3));
    _hoverLabel->setClipToAxisRect(false);
    _hoverLabel->setVisible(false);

    _hoverColorRect = new QCPItemRect(plot);
    _hoverColorRect->setLayer(layer);
    _hoverColorRect->topLeft->setParentAnchor(_hoverLabel->topRight);
    _hoverColorRect->setClipToAxisRect(false);
    _hoverColorRect->setVisible(false);

    _itemTracer = new QCPItemTracer(plot);
    _itemTracer->setBrush(QBrush(Qt::white));
    _itemTracer->setLayer(layer);
    _itemTracer->setInterpolating(false);
    _itemTracer->setVisible(true);
    _itemTracer->setStyle(QCPItemTracer::TracerStyle::tsCircle);
    _itemTracer->setClipToAxisRect(false);
}

void CorrelationPlotWorker::update(CorrelationPlotConfig config, int width, int height)
{
    if(_customPlot == nullptr)
    {
        u::setCurrentThreadName(QStringLiteral("CorrPlotUpdate"));

        _customPlot = new QCustomPlot;

        _customPlot->setOpenGl(true);
        _customPlot->setAutoAddPlottableToLegend(false);

        _customPlot->addLayer(QStringLiteral("tooltipLayer"));
        tooltipLayer()->setMode(QCPLayer::LayerMode::lmBuffered);
        _tooltips.initialise(_customPlot, tooltipLayer());

        connect(_customPlot, &QCustomPlot::afterReplot, this, &CorrelationPlotWorker::onReplot);
    }

    if(width > 0 && height > 0)
    {
        _width = width;
        _height = height;
    }

    if(!otherUpdatesQueued())
    {
        std::unique_lock<std::mutex> lock(_mutex);

        _customPlot->setGeometry(0, 0, _width, _height);
        scaleXAxis();

        if(config != _config || _lastBuildIncomplete)
        {
            _config = config;
            buildPlot();
        }

        if(!otherUpdatesQueued())
            _customPlot->replot(QCustomPlot::rpImmediateRefresh);
    }

    updateCompleted();
}

void CorrelationPlotWorker::onHoverMouseMove(const QPointF& position)
{
    _tooltips._hoverPoint = position;

    auto* plottableUnderCursor = _customPlot->plottableAt(position, true);
    if(_tooltips._hoverPlottable != plottableUnderCursor)
        _tooltips._hoverPlottable = plottableUnderCursor;

    if(_tooltips._hoverPlottable != nullptr)
        showTooltip();
    else
        hideTooltip();
}

void CorrelationPlotWorker::onHoverMouseLeave()
{
    hideTooltip();
}

double CorrelationPlotWorker::visibleHorizontalFraction() const
{
    if(_config._showColumnNames)
        return (columnAxisWidth() / (columnLabelWidth() * _config._columnCount));

    return 1.0;
}

double CorrelationPlotWorker::columnLabelWidth() const
{
    QFontMetrics metrics(_defaultFont9Pt);
    const unsigned int columnPadding = 1;
    return metrics.height() + columnPadding;
}

double CorrelationPlotWorker::columnAxisWidth() const
{
    const auto& margins = _customPlot->axisRect()->margins();
    const unsigned int axisWidth = margins.left() + margins.right();

    const auto axisFraction = _config._showLegend ? 1.0 - LEGEND_WIDTH_FRACTION : 1.0;

    return (_customPlot->geometry().width() - axisWidth) * axisFraction;
}

void CorrelationPlotWorker::scaleXAxis()
{
    auto maxX = static_cast<double>(_config._columnCount);
    if(_config._showColumnNames)
    {
        double numVisibleColumns = columnAxisWidth() / columnLabelWidth();

        double position = (_config._columnCount - numVisibleColumns) *
            _config._scrollAmount;

        if(position + numVisibleColumns <= maxX)
            _customPlot->xAxis->setRange(position, position + numVisibleColumns);
        else
            _customPlot->xAxis->setRange(0, maxX);
    }
    else
        _customPlot->xAxis->setRange(0, maxX);
}

static QVector<double> meanAverageData(const CorrelationPlotConfig& config, double& min, double& max)
{
    min = std::numeric_limits<double>::max();
    max = std::numeric_limits<double>::min();

    // Use Average Calculation
    QVector<double> yDataAvg;
    yDataAvg.reserve(config._selectedRows.size());

    for(size_t col = 0; col < config._columnCount; col++)
    {
        double runningTotal = 0.0;
        for(auto row : qAsConst(config._selectedRows))
        {
            auto index = (row * config._columnCount) + col;
            runningTotal += config._data->at(static_cast<int>(index));
        }

        yDataAvg.append(runningTotal / config._selectedRows.length());

        max = std::max(max, yDataAvg.back());
        min = std::min(min, yDataAvg.back());
    }

    return yDataAvg;
}

void CorrelationPlotWorker::populateMeanLinePlot()
{
    double maxY = 0.0;
    double minY = 0.0;

    auto* graph = _customPlot->addGraph();
    graph->setPen(QPen(Qt::black));
    graph->setName(tr("Mean average of selection"));

    QVector<double> xData(static_cast<int>(_config._columnCount));
    // xData is just the column indices
    std::iota(std::begin(xData), std::end(xData), 0);

    // Use Average Calculation and set min / max
    QVector<double> yDataAvg = meanAverageData(_config, minY, maxY);

    graph->setData(xData, yDataAvg, true);

    auto* plotModeTextElement = new QCPTextElement(_customPlot);
    plotModeTextElement->setLayer(tooltipLayer());
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(QString(tr("*Mean average plot of %1 rows"))
        .arg(_config._selectedRows.length()));
    plotModeTextElement->setVisible(true);

    _customPlot->plotLayout()->insertRow(1);
    _customPlot->plotLayout()->addElement(1, 0, plotModeTextElement);

    scaleXAxis();
    _customPlot->yAxis->setRange(minY, maxY);
}

void CorrelationPlotWorker::populateMedianLinePlot()
{
    double maxY = 0.0;
    double minY = 0.0;

    auto* graph = _customPlot->addGraph();
    graph->setPen(QPen(Qt::black));
    graph->setName(tr("Median average of selection"));

    QVector<double> xData(static_cast<int>(_config._columnCount));
    // xData is just the column indices
    std::iota(std::begin(xData), std::end(xData), 0);

    QVector<double> rowsEntries(_config._selectedRows.length());
    QVector<double> yDataAvg(static_cast<int>(_config._columnCount));

    for(int col = 0; col < static_cast<int>(_config._columnCount); col++)
    {
        if(otherUpdatesQueued())
            return;

        rowsEntries.clear();
        for(auto row : qAsConst(_config._selectedRows))
        {
            auto index = (row * _config._columnCount) + col;
            rowsEntries.push_back(_config._data->at(static_cast<int>(index)));
        }

        if(!_config._selectedRows.empty())
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

    auto* plotModeTextElement = new QCPTextElement(_customPlot);
    plotModeTextElement->setLayer(tooltipLayer());
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Median average plot of %1 rows")
                .arg(_config._selectedRows.length())));
    plotModeTextElement->setVisible(true);

    _customPlot->plotLayout()->insertRow(1);
    _customPlot->plotLayout()->addElement(1, 0, plotModeTextElement);

    scaleXAxis();
    _customPlot->yAxis->setRange(minY, maxY);
}

void CorrelationPlotWorker::populateMeanHistogramPlot()
{
    if(_config._selectedRows.isEmpty())
        return;

    double maxY = 0.0;
    double minY = 0.0;

    QVector<double> xData(static_cast<int>(_config._columnCount));
    // xData is just the column indices
    std::iota(std::begin(xData), std::end(xData), 0);

    // Use Average Calculation and set min / max
    QVector<double> yDataAvg = meanAverageData(_config, minY, maxY);

    auto* histogramBars = new QCPBars(_customPlot->xAxis, _customPlot->yAxis);
    histogramBars->setName(tr("Mean histogram of selection"));
    histogramBars->setData(xData, yDataAvg, true);

    auto* plotModeTextElement = new QCPTextElement(_customPlot);
    plotModeTextElement->setLayer(tooltipLayer());
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Mean histogram of %1 rows"))
                .arg(_config._selectedRows.length()));
    plotModeTextElement->setVisible(true);

    _customPlot->plotLayout()->insertRow(1);
    _customPlot->plotLayout()->addElement(1, 0, plotModeTextElement);

    scaleXAxis();
    _customPlot->yAxis->setRange(minY, maxY);
}

static double medianOf(const QVector<double>& sortedData)
{
    if(sortedData.length() == 0)
        return 0.0;

    double median = 0.0;
    if(sortedData.length() % 2 == 0)
        median = (sortedData[(sortedData.length() / 2) - 1] + sortedData[sortedData.length() / 2]) / 2.0;
    else
        median = sortedData[sortedData.length() / 2];

    return median;
}

void CorrelationPlotWorker::populateIQRPlot()
{
    // Box-plots representing the IQR.
    // Whiskers represent the maximum and minimum non-outlier values
    // Outlier values are (< Q1 - 1.5IQR and > Q3 + 1.5IQR)

    auto* statPlot = new QCPStatisticalBox(_customPlot->xAxis, _customPlot->yAxis);
    statPlot->setName(tr("Median (IQR plots) of selection"));

    double maxY = 0.0;
    double minY = 0.0;

    QVector<double> rowsEntries(_config._selectedRows.length());
    QVector<double> outliers;

    // Calculate IQRs, outliers and ranges
    for(int col = 0; col < static_cast<int>(_config._columnCount); col++)
    {
        if(otherUpdatesQueued())
            return;

        rowsEntries.clear();
        outliers.clear();
        for(auto row : qAsConst(_config._selectedRows))
        {
            auto index = (row * _config._columnCount) + col;
            rowsEntries.push_back(_config._data->at(static_cast<int>(index)));
        }

        if(!_config._selectedRows.empty())
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

            for(auto row : qAsConst(rowsEntries))
            {
                // Find Maximum and minimum non-outliers
                if(row < thirdQuartile + (iqr * 1.5))
                    maxValue = std::max(maxValue, row);
                if(row > firstQuartile - (iqr * 1.5))
                    minValue = std::min(minValue, row);

                // Find outliers
                if(row > thirdQuartile + (iqr * 1.5))
                    outliers.push_back(row);
                else if(row < firstQuartile - (iqr * 1.5))
                    outliers.push_back(row);

                maxY = std::max(maxY, row);
                minY = std::min(minY, row);
            }

            // Add data for each column individually because setData doesn't let us do outliers(??)
            statPlot->addData(col, minValue, firstQuartile, secondQuartile, thirdQuartile,
                              maxValue, outliers);
        }
    }

    auto* plotModeTextElement = new QCPTextElement(_customPlot);
    plotModeTextElement->setLayer(tooltipLayer());
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Median IQR box plots of %1 rows"))
                .arg(_config._selectedRows.length()));
    plotModeTextElement->setVisible(true);

    _customPlot->plotLayout()->insertRow(1);
    _customPlot->plotLayout()->addElement(1, 0, plotModeTextElement);

    _customPlot->yAxis->setRange(minY, maxY);
    scaleXAxis();
}

void CorrelationPlotWorker::plotDispersion(QVector<double> stdDevs, const QString& name = QStringLiteral("Deviation"))
{
    if(_config._plotDispersionVisualType == PlotDispersionVisualType::Bars)
    {
        auto* stdDevBars = new QCPErrorBars(_customPlot->xAxis, _customPlot->yAxis);
        stdDevBars->setName(name);
        stdDevBars->setSelectable(QCP::SelectionType::stNone);
        stdDevBars->setAntialiased(false);
        stdDevBars->setDataPlottable(_customPlot->plottable(0));
        stdDevBars->setData(stdDevs);
    }
    else if(_config._plotDispersionVisualType == PlotDispersionVisualType::Area)
    {
        auto* devTop = new QCPGraph(_customPlot->xAxis, _customPlot->yAxis);
        auto* devBottom = new QCPGraph(_customPlot->xAxis, _customPlot->yAxis);
        devTop->setName(QStringLiteral("%1 Top").arg(name));
        devBottom->setName(QStringLiteral("%1 Bottom").arg(name));

        auto fillColour = _customPlot->plottable(0)->pen().color();
        auto penColour = _customPlot->plottable(0)->pen().color().lighter(150);
        fillColour.setAlpha(50);
        penColour.setAlpha(120);

        devTop->setChannelFillGraph(devBottom);
        devTop->setBrush(QBrush(fillColour));
        devTop->setPen(QPen(penColour));

        devBottom->setPen(QPen(penColour));

        devBottom->setSelectable(QCP::SelectionType::stNone);
        devTop->setSelectable(QCP::SelectionType::stNone);

        auto topErr = QVector<double>(static_cast<int>(_config._columnCount));
        auto bottomErr = QVector<double>(static_cast<int>(_config._columnCount));

        for(int i = 0; i < static_cast<int>(_config._columnCount); ++i)
        {
            topErr[i] = _customPlot->plottable(0)->interface1D()->dataMainValue(i) + stdDevs[i];
            bottomErr[i] = _customPlot->plottable(0)->interface1D()->dataMainValue(i) - stdDevs[i];
        }

        // xData is just the column indices
        QVector<double> xData(static_cast<int>(_config._columnCount));
        std::iota(std::begin(xData), std::end(xData), 0);

        devTop->setData(xData, topErr);
        devBottom->setData(xData, bottomErr);
    }
}

void CorrelationPlotWorker::populateStdDevPlot()
{
    if(_config._selectedRows.isEmpty())
        return;

    double min = 0, max = 0;

    QVector<double> stdDevs(static_cast<int>(_config._columnCount));
    QVector<double> means(static_cast<int>(_config._columnCount));

    for(int col = 0; col < static_cast<int>(_config._columnCount); col++)
    {
        if(otherUpdatesQueued())
            return;

        for(auto row : qAsConst(_config._selectedRows))
        {
            auto index = (row * _config._columnCount) + col;
            means[col] += _config._data->at(static_cast<int>(index));
        }
        means[col] /= _config._selectedRows.count();

        double stdDev = 0.0;
        for(auto row : qAsConst(_config._selectedRows))
        {
            auto index = (row * _config._columnCount) + col;
            stdDev += (_config._data->at(static_cast<int>(index)) - means.at(col)) *
                    (_config._data->at(static_cast<int>(index)) - means.at(col));
        }
        stdDev /= _config._columnCount;
        stdDev = std::sqrt(stdDev);
        stdDevs[col] = stdDev;

        min = std::min(min, means.at(col) - stdDev);
        max = std::max(max, means.at(col) + stdDev);
    }

    plotDispersion(stdDevs, QStringLiteral("Std Dev"));
    _customPlot->yAxis->setRange(min, max);
}

void CorrelationPlotWorker::populateStdErrorPlot()
{
    if(_config._selectedRows.isEmpty())
        return;

    double min = 0, max = 0;

    QVector<double> stdErrs(static_cast<int>(_config._columnCount));
    QVector<double> means(static_cast<int>(_config._columnCount));

    for(int col = 0; col < static_cast<int>(_config._columnCount); col++)
    {
        if(otherUpdatesQueued())
            return;

        for(auto row : qAsConst(_config._selectedRows))
        {
            auto index = (row * _config._columnCount) + col;
            means[col] += _config._data->at(static_cast<int>(index));
        }
        means[col] /= _config._selectedRows.count();

        double stdErr = 0.0;
        for(auto row : qAsConst(_config._selectedRows))
        {
            auto index = (row * _config._columnCount) + col;
            stdErr += (_config._data->at(static_cast<int>(index)) - means.at(col)) *
                    (_config._data->at(static_cast<int>(index)) - means.at(col));
        }
        stdErr /= _config._columnCount;
        stdErr = std::sqrt(stdErr) / std::sqrt(static_cast<double>(_config._selectedRows.length()));
        stdErrs[col] = stdErr;

        min = std::min(min, means.at(col) - stdErr);
        max = std::max(max, means.at(col) + stdErr);
    }

    plotDispersion(stdErrs, QStringLiteral("Std Err"));
    _customPlot->yAxis->setRange(min, max);
}

void CorrelationPlotWorker::populateLinePlot()
{
    double maxY = 0.0;
    double minY = 0.0;

    QVector<double> yData; yData.reserve(_config._selectedRows.size());
    QVector<double> xData; xData.reserve(static_cast<int>(_config._columnCount));

    // Plot each row individually
    for(auto row : qAsConst(_config._selectedRows))
    {
        if(otherUpdatesQueued())
            return;

        auto* graph = _customPlot->addGraph();
        graph->setPen(_config._rowColors.at(row));
        graph->setName(_config._graphNames[row]);

        yData.clear();
        xData.clear();

        double rowMean = 0;

        for(size_t col = 0; col < _config._columnCount; col++)
        {
            auto index = (row * _config._columnCount) + col;
            rowMean += _config._data->at(static_cast<int>(index));
        }
        rowMean /= _config._columnCount;

        double stdDev = 0;
        for(size_t col = 0; col < _config._columnCount; col++)
        {
            auto index = (row * _config._columnCount) + col;
            stdDev += (_config._data->at(static_cast<int>(index)) - rowMean) *
                (_config._data->at(static_cast<int>(index)) - rowMean);
        }
        stdDev /= _config._columnCount;
        stdDev = std::sqrt(stdDev);
        double pareto = std::sqrt(stdDev);

        for(size_t col = 0; col < _config._columnCount; col++)
        {
            auto index = (row * _config._columnCount) + col;
            auto data = _config._data->at(static_cast<int>(index));
            switch(_config._plotScaleType)
            {
            case PlotScaleType::Log:
            {
                // LogY(x+c) where c is EPSILON
                // This prevents LogY(0) which is -inf
                // Log2(0+c) = -1057
                // Document this!
                const double EPSILON = std::nextafter(0.0, 1.0);
                data = std::log(data + EPSILON);
            }
                break;
            case PlotScaleType::MeanCentre:
                data -= rowMean;
                break;
            case PlotScaleType::UnitVariance:
                data -= rowMean;
                data /= stdDev;
                break;
            case PlotScaleType::Pareto:
                data -= rowMean;
                data /= pareto;
                break;
            default:
                break;
            }

            xData.append(static_cast<double>(col));
            yData.append(data);

            maxY = std::max(maxY, data);
            minY = std::min(minY, data);
        }

        graph->setData(xData, yData, true);
    }

    scaleXAxis();
    _customPlot->yAxis->setRange(minY, maxY);
}

void CorrelationPlotWorker::configureLegend()
{
    if(_customPlot->plottableCount() <= 0 || !_config._showLegend)
        return;

    // Create a subLayout to position the Legend
    auto* subLayout = new QCPLayoutGrid;
    _customPlot->plotLayout()->insertColumn(1);
    _customPlot->plotLayout()->addElement(0, 1, subLayout);

    // Surround the legend row in two empty rows that are stretched maximally, and
    // stretch the legend itself minimally, thus centreing the legend vertically
    subLayout->insertRow(0);
    subLayout->setRowStretchFactor(0, 1.0);
    subLayout->addElement(1, 0, _customPlot->legend);
    subLayout->setRowStretchFactor(1, std::numeric_limits<double>::min());
    subLayout->insertRow(2);
    subLayout->setRowStretchFactor(2, 1.0);

    const int marginSize = 5;
    subLayout->setMargins(QMargins(0, marginSize, marginSize, marginSize));
    _customPlot->legend->setMargins(QMargins(marginSize, marginSize, marginSize, marginSize));

    // BIGGEST HACK
    // Layouts and sizes aren't done until a replot, and layout is performed on another
    // thread which means it's too late to add or remove elements from the legend.
    // The anticipated sizes for the legend layout are calculated here but will break
    // if any additional rows are added to the plotLayout as the legend height is
    // estimated using the total height of the QQuickItem, not the (unknowable) plot height

    // See QCPPlottableLegendItem::draw for the reasoning behind this value
    const auto legendElementHeight = std::max(QFontMetrics(_customPlot->legend->font()).height(),
                                              _customPlot->legend->iconSize().height());

    const auto totalExternalMargins = subLayout->margins().top() + subLayout->margins().bottom();
    const auto totalInternalMargins = _customPlot->legend->margins().top() + _customPlot->legend->margins().bottom();
    const auto maxLegendHeight = _customPlot->height() - (totalExternalMargins + totalInternalMargins);

    int maxNumberOfElementsToDraw = 0;
    int accumulatedHeight = legendElementHeight;
    while(accumulatedHeight < maxLegendHeight)
    {
        accumulatedHeight += (_customPlot->legend->rowSpacing() + legendElementHeight);
        maxNumberOfElementsToDraw++;
    };

    const auto numberOfElementsToDraw = std::min(_customPlot->plottableCount(), maxNumberOfElementsToDraw);

    if(numberOfElementsToDraw > 0)
    {
        // Populate the legend
        _customPlot->legend->clear();
        for(int i = 0; i < numberOfElementsToDraw; i++)
            _customPlot->plottable(i)->addToLegend(_customPlot->legend);

        // Cap the legend count to only those visible
        if(_customPlot->plottableCount() > maxNumberOfElementsToDraw)
        {
            auto* moreText = new QCPTextElement(_customPlot);
            moreText->setMargins(QMargins());
            moreText->setLayer(tooltipLayer());
            moreText->setTextFlags(Qt::AlignLeft);
            moreText->setFont(_customPlot->legend->font());
            moreText->setTextColor(Qt::gray);
            moreText->setText(QString(tr("...and %1 more"))
                .arg(_customPlot->plottableCount() - maxNumberOfElementsToDraw + 1));
            moreText->setVisible(true);

            auto lastElementIndex = _customPlot->legend->rowColToIndex(_customPlot->legend->rowCount() - 1, 0);
            _customPlot->legend->removeAt(lastElementIndex);
            _customPlot->legend->addElement(moreText);

            // When we're overflowing, hackily enlarge the bottom margin to
            // compensate for QCP's layout algorithm being a bit rubbish
            auto margins = _customPlot->legend->margins();
            margins.setBottom(margins.bottom() * 3);
            _customPlot->legend->setMargins(margins);
        }

        _customPlot->plotLayout()->setColumnStretchFactor(0, 1.0 - LEGEND_WIDTH_FRACTION);
        _customPlot->plotLayout()->setColumnStretchFactor(1, LEGEND_WIDTH_FRACTION);

        _customPlot->legend->setVisible(true);
    }
}

void CorrelationPlotWorker::buildPlot()
{
    _lastBuildIncomplete = true;
    _customPlot->legend->setVisible(false);

    _customPlot->clearGraphs();
    _customPlot->clearPlottables();

    while(_customPlot->plotLayout()->rowCount() > 1)
    {
        _customPlot->plotLayout()->removeAt(_customPlot->plotLayout()->rowColToIndex(1, 0));
        _customPlot->plotLayout()->simplify();
    }

    while(_customPlot->plotLayout()->columnCount() > 1)
    {
        // Save the legend from getting destroyed
        _customPlot->axisRect()->insetLayout()->addElement(_customPlot->legend, Qt::AlignRight);
        _customPlot->plotLayout()->removeAt(_customPlot->plotLayout()->rowColToIndex(0, 1));
        // Destroy the extra legend column
        _customPlot->plotLayout()->simplify();
    }

    switch(_config._plotAveragingType)
    {
    case PlotAveragingType::MeanLine:
        populateMeanLinePlot();
        break;

    case PlotAveragingType::MedianLine:
        populateMedianLinePlot();
        break;

    case PlotAveragingType::MeanHistogram:
        populateMeanHistogramPlot();
        break;

    case PlotAveragingType::IQRPlot:
        populateIQRPlot();
        break;

    default:
        populateLinePlot();
        break;
    }

    if(otherUpdatesQueued())
        return;

    if(_config._plotAveragingType != PlotAveragingType::Individual &&
        _config._plotAveragingType != PlotAveragingType::IQRPlot)
    {
        if(_config._plotDispersionType == PlotDispersionType::StdDev)
            populateStdDevPlot();
        else if(_config._plotDispersionType == PlotDispersionType::StdErr)
            populateStdErrorPlot();
    }

    if(otherUpdatesQueued())
        return;

    configureLegend();

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);
    _customPlot->xAxis->setTicker(categoryTicker);
    _customPlot->xAxis->setTickLabelRotation(90);

    _customPlot->xAxis->setLabel(_config._xAxisLabel);
    _customPlot->yAxis->setLabel(_config._yAxisLabel);

    _customPlot->xAxis->grid()->setVisible(_config._showGridLines);
    _customPlot->yAxis->grid()->setVisible(_config._showGridLines);

    if(_config._showColumnNames)
    {
        if(_config._elideLabelWidth > 0)
        {
            QFontMetrics metrics(_defaultFont9Pt);
            int column = 0;

            for(auto& labelName : _config._labelNames)
                categoryTicker->addTick(column++, metrics.elidedText(labelName, Qt::ElideRight, _config._elideLabelWidth));
        }
        else
        {
            // There is no room to display labels, so show a warning instead
            QString warning = tr("Resize To Expose Column Names");
            if(!_config._xAxisLabel.isEmpty())
                _customPlot->xAxis->setLabel(QString(QStringLiteral("%1 (%2)")).arg(_config._xAxisLabel, warning));
            else
                _customPlot->xAxis->setLabel(warning);
        }
    }

    _customPlot->setBackground(Qt::white);

    _lastBuildIncomplete = false;
}

void CorrelationPlotWorker::onReplot()
{
    if(_width > 0 && _height > 0)
    {
        _pixmap = _customPlot->toPixmap();
        emit visibleHorizontalFractionChanged(visibleHorizontalFraction());
        emit updated();
    }
}

void CorrelationPlotWorker::showTooltip()
{
    _tooltips._itemTracer->setGraph(nullptr);
    if(auto graph = dynamic_cast<QCPGraph*>(_tooltips._hoverPlottable))
    {
        _tooltips._itemTracer->setGraph(graph);
        _tooltips._itemTracer->setGraphKey(_customPlot->xAxis->pixelToCoord(_tooltips._hoverPoint.x()));
        _tooltips._itemTracer->updatePosition();
    }
    else if(auto bars = dynamic_cast<QCPBars*>(_tooltips._hoverPlottable))
    {
        auto xCoord = std::lround(_customPlot->xAxis->pixelToCoord(_tooltips._hoverPoint.x()));
        _tooltips._itemTracer->position->setPixelPosition(bars->dataPixelPosition(xCoord));
    }
    else if(auto boxPlot = dynamic_cast<QCPStatisticalBox*>(_tooltips._hoverPlottable))
    {
        auto xCoord = std::lround(_customPlot->xAxis->pixelToCoord(_tooltips._hoverPoint.x()));
        _tooltips._itemTracer->position->setPixelPosition(boxPlot->dataPixelPosition(xCoord));
    }

    auto tracerPosition = _tooltips._itemTracer->anchor(QStringLiteral("position"))->pixelPosition();

    _tooltips._itemTracer->setVisible(true);
    _tooltips._itemTracer->setInterpolating(false);

    _tooltips._hoverLabel->setVisible(true);
    _tooltips._hoverLabel->setText(QStringLiteral("%1, %2: %3")
                     .arg(_tooltips._hoverPlottable->name(),
                          _config._labelNames[static_cast<int>(_tooltips._itemTracer->position->key())])
                     .arg(_tooltips._itemTracer->position->value()));

    const auto COLOR_RECT_WIDTH = 10.0;
    const auto HOVER_MARGIN = 10.0;
    auto hoverlabelWidth = _tooltips._hoverLabel->right->pixelPosition().x() -
            _tooltips._hoverLabel->left->pixelPosition().x();
    auto hoverlabelHeight = _tooltips._hoverLabel->bottom->pixelPosition().y() -
            _tooltips._hoverLabel->top->pixelPosition().y();
    auto hoverLabelRightX = tracerPosition.x() + hoverlabelWidth + HOVER_MARGIN + COLOR_RECT_WIDTH;
    auto xBounds = _customPlot->width();
    QPointF targetPosition(tracerPosition.x() + HOVER_MARGIN, tracerPosition.y());

    // If it falls out of bounds, clip to bounds and move label above marker
    if(hoverLabelRightX > xBounds)
    {
        targetPosition.rx() = xBounds - hoverlabelWidth - COLOR_RECT_WIDTH - 1.0;

        // If moving the label above marker is less than 0, clip to 0 + labelHeight/2;
        if(targetPosition.y() - (hoverlabelHeight * 0.5) - HOVER_MARGIN * 2.0 < 0.0)
            targetPosition.setY(hoverlabelHeight * 0.5);
        else
            targetPosition.ry() -= HOVER_MARGIN * 2.0;
    }

    _tooltips._hoverLabel->position->setPixelPosition(targetPosition);

    _tooltips._hoverColorRect->setVisible(true);
    _tooltips._hoverColorRect->setBrush(QBrush(_tooltips._hoverPlottable->pen().color()));
    _tooltips._hoverColorRect->bottomRight->setPixelPosition(
        QPointF(_tooltips._hoverLabel->bottomRight->pixelPosition().x() + COLOR_RECT_WIDTH,
        _tooltips._hoverLabel->bottomRight->pixelPosition().y()));

    tooltipLayer()->replot();
    _customPlot->replot(QCustomPlot::rpImmediateRefresh);
}

void CorrelationPlotWorker::hideTooltip()
{
    if(!_tooltips._itemTracer->visible())
        return;

    _tooltips._hoverLabel->setVisible(false);
    _tooltips._hoverColorRect->setVisible(false);
    _tooltips._itemTracer->setVisible(false);

    tooltipLayer()->replot();
    _customPlot->replot(QCustomPlot::rpImmediateRefresh);
}

void CorrelationPlotWorker::routeMouseEvent(const QMouseEvent* event)
{
    auto* newEvent = new QMouseEvent(event->type(), event->localPos(),
                                     event->button(), event->buttons(),
                                     event->modifiers());
    QCoreApplication::postEvent(_customPlot, newEvent);
}

void CorrelationPlotWorker::routeWheelEvent(const QWheelEvent* event)
{
    auto* newEvent = new QWheelEvent(event->pos(), event->delta(),
                                     event->buttons(), event->modifiers(),
                                     event->orientation());
    QCoreApplication::postEvent(_customPlot, newEvent);
}

void CorrelationPlotItem::updatePlotSize()
{
    emit configChanged(_config, static_cast<int>(width()), static_cast<int>(height()));
}

void CorrelationPlotWorker::savePlotImage(const QUrl& url, const QStringList& extensions)
{
    if(extensions.contains(QStringLiteral("png")))
        _customPlot->savePng(url.toLocalFile());
    else if(extensions.contains(QStringLiteral("pdf")))
        _customPlot->savePdf(url.toLocalFile());
    else if(extensions.contains(QStringLiteral("jpg")))
        _customPlot->saveJpg(url.toLocalFile());

    QDesktopServices::openUrl(url);
}

CorrelationPlotItem::CorrelationPlotItem(QQuickItem* parent) :
    QQuickPaintedItem(parent), _tooltipTimer(this)
{
    setRenderTarget(RenderTarget::FramebufferObject);

    setFlag(QQuickItem::ItemHasContents, true);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &QQuickPaintedItem::widthChanged, this, &CorrelationPlotItem::updatePlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &CorrelationPlotItem::updatePlotSize);

    qRegisterMetaType<CorrelationPlotConfig>("CorrelationPlotConfig");

    _worker = new CorrelationPlotWorker;
    _worker->moveToThread(&_plotBuildThread);
    connect(&_plotBuildThread, &QThread::finished, _worker, &QObject::deleteLater);
    connect(this, &CorrelationPlotItem::configChanged, [this] { _worker->queueUpdate(); });
    connect(this, &CorrelationPlotItem::configChanged, _worker, &CorrelationPlotWorker::update);
    connect(_worker, &CorrelationPlotWorker::visibleHorizontalFractionChanged,
        this, &CorrelationPlotItem::onVisibleHorizontalFractionChanged);
    connect(_worker, &CorrelationPlotWorker::updated, this, &CorrelationPlotItem::onPlotUpdated);

    connect(this, &CorrelationPlotItem::hoverMouseHover, _worker, &CorrelationPlotWorker::onHoverMouseMove);
    connect(this, &CorrelationPlotItem::hoverMouseLeave, _worker, &CorrelationPlotWorker::onHoverMouseLeave);

    connect(_worker, &CorrelationPlotWorker::busyChanged,
    [this]
    {
        _busy = _worker->busy();
        emit busyChanged();
    });

    _tooltipTimer.setSingleShot(true);
    _tooltipTimer.setInterval(100);
    connect(&_tooltipTimer, &QTimer::timeout, [this] { emit hoverMouseHover(_hoverPosition); });

    _plotBuildThread.start();
}

CorrelationPlotItem::~CorrelationPlotItem()
{
    _plotBuildThread.quit();
    _plotBuildThread.wait();
}

void CorrelationPlotItem::paint(QPainter* painter)
{
    if(_worker != nullptr)
    {
        QPixmap pixmap = _worker->pixmap();

        if(pixmap.width() == 0)
        {
            if(_lastRenderedPixmap.width() > 0)
            {
                pixmap = _lastRenderedPixmap.scaled(
                    static_cast<int>(width()), static_cast<int>(height()),
                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
        }
        else
            _lastRenderedPixmap = pixmap;

        if(pixmap.width() > 0)
            painter->drawPixmap(0, 0, pixmap);
    }
}

void CorrelationPlotItem::mousePressEvent(QMouseEvent* event)
{
    _worker->routeMouseEvent(event);
}

void CorrelationPlotItem::mouseReleaseEvent(QMouseEvent* event)
{
     _worker->routeMouseEvent(event);
    if(event->button() == Qt::RightButton)
        emit rightClick();
}

void CorrelationPlotItem::mouseMoveEvent(QMouseEvent* event)
{
     _worker->routeMouseEvent(event);
}

void CorrelationPlotItem::hoverMoveEvent(QHoverEvent* event)
{
    if(_hoverPosition == event->posF())
        return;

    _hoverPosition = event->posF();
    _tooltipTimer.start();
}

void CorrelationPlotItem::hoverLeaveEvent(QHoverEvent*)
{
    emit hoverMouseLeave();
}

void CorrelationPlotItem::mouseDoubleClickEvent(QMouseEvent* event)
{
     _worker->routeMouseEvent(event);
}

void CorrelationPlotItem::wheelEvent(QWheelEvent* event)
{
     _worker->routeWheelEvent(event);
}

void CorrelationPlotItem::setPlotDispersionVisualType(int plotDispersionVisualType)
{
    _config._plotDispersionVisualType = static_cast<PlotDispersionVisualType>(plotDispersionVisualType);
    emit configChanged(_config);
}

void CorrelationPlotItem::onVisibleHorizontalFractionChanged(double visibleHorizontalFraction)
{
    _visibleHorizontalFraction = visibleHorizontalFraction;
    emit visibleHorizontalFractionChanged();
}

void CorrelationPlotItem::setYAxisLabel(const QString& plotYAxisLabel)
{
    _config._yAxisLabel = plotYAxisLabel;
    emit configChanged(_config);
}

void CorrelationPlotItem::setXAxisLabel(const QString& plotXAxisLabel)
{
    _config._xAxisLabel = plotXAxisLabel;
    emit configChanged(_config);
}

void CorrelationPlotItem::setPlotScaleType(int plotScaleType)
{
    _config._plotScaleType = static_cast<PlotScaleType>(plotScaleType);
    emit configChanged(_config);
}

void CorrelationPlotItem::setPlotAveragingType(int plotAveragingType)
{
    _config._plotAveragingType = static_cast<PlotAveragingType>(plotAveragingType);
    emit configChanged(_config);
}

void CorrelationPlotItem::setPlotDispersionType(int plotDispersionType)
{
    _config._plotDispersionType = static_cast<PlotDispersionType>(plotDispersionType);
    emit configChanged(_config);
}

void CorrelationPlotItem::setShowLegend(bool showLegend)
{
    _config._showLegend = showLegend;
    emit configChanged(_config);
}

void CorrelationPlotItem::setData(const QVector<double>& data)
{
    _data = data;
    _config._data = &_data;
    emit configChanged(_config);
}

void CorrelationPlotItem::setSelectedRows(const QVector<int>& selectedRows)
{
    _config._selectedRows = selectedRows;
    emit configChanged(_config);
}

void CorrelationPlotItem::setRowColors(const QVector<QColor>& rowColors)
{
    _config._rowColors = rowColors;
    emit configChanged(_config);
}

void CorrelationPlotItem::setLabelNames(const QStringList& labelNames)
{
    _config._labelNames = labelNames;
    emit configChanged(_config);
}

void CorrelationPlotItem::setGraphNames(const QStringList& graphNames)
{
    _config._graphNames = graphNames;
    emit configChanged(_config);
}

void CorrelationPlotItem::setElideLabelWidth(int elideLabelWidth)
{
    bool changed = (_config._elideLabelWidth != elideLabelWidth);
    _config._elideLabelWidth = elideLabelWidth;

    if(changed && _config._showColumnNames)
        emit configChanged(_config);
}

void CorrelationPlotItem::setColumnCount(size_t columnCount)
{
    _config._columnCount = columnCount;
}

void CorrelationPlotItem::setRowCount(size_t rowCount)
{
    _config._rowCount = rowCount;
}

void CorrelationPlotItem::setShowColumnNames(bool showColumnNames)
{
    bool changed = (_config._showColumnNames != showColumnNames);
    _config._showColumnNames = showColumnNames;

    if(changed)
    {
        emit visibleHorizontalFractionChanged();
        emit configChanged(_config);
    }
}

void CorrelationPlotItem::setShowGridLines(bool showGridLines)
{
    _config._showGridLines = showGridLines;
    emit configChanged(_config);
}

void CorrelationPlotItem::setScrollAmount(double scrollAmount)
{
    _config._scrollAmount = scrollAmount;
    emit configChanged(_config);
    emit scrollAmountChanged();
}

void CorrelationPlotItem::savePlotImage(const QUrl& url, const QStringList& extensions)
{
    QMetaObject::invokeMethod(_worker, "savePlotImage",
        Q_ARG(QUrl, url), Q_ARG(QStringList, extensions));
}

void CorrelationPlotItem::onPlotUpdated()
{
    update();
}
