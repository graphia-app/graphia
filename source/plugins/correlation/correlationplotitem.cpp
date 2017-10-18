#include "correlationplotitem.h"

#include <QDesktopServices>

CorrelationPlotItem::CorrelationPlotItem(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    setRenderTarget(RenderTarget::FramebufferObject);

    _customPlot.setOpenGl(true);
    _customPlot.addLayer(QStringLiteral("textLayer"));

    _textLayer = _customPlot.layer(QStringLiteral("textLayer"));
    _textLayer->setMode(QCPLayer::LayerMode::lmBuffered);

    QFont defaultFont10Pt;
    defaultFont10Pt.setPointSize(10);

    _defaultFont9Pt.setPointSize(9);

    _hoverLabel = new QCPItemText(&_customPlot);
    _hoverLabel->setLayer(_textLayer);
    _hoverLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    _hoverLabel->setFont(defaultFont10Pt);
    _hoverLabel->setPen(QPen(Qt::black));
    _hoverLabel->setBrush(QBrush(Qt::white));
    _hoverLabel->setPadding(QMargins(3, 3, 3, 3));
    _hoverLabel->setClipToAxisRect(false);
    _hoverLabel->setVisible(false);

    _hoverColorRect = new QCPItemRect(&_customPlot);
    _hoverColorRect->setLayer(_textLayer);
    _hoverColorRect->topLeft->setParentAnchor(_hoverLabel->topRight);
    _hoverColorRect->setClipToAxisRect(false);
    _hoverColorRect->setVisible(false);

    _itemTracer = new QCPItemTracer(&_customPlot);
    _itemTracer->setBrush(QBrush(Qt::white));
    _itemTracer->setLayer(_textLayer);
    _itemTracer->setInterpolating(false);
    _itemTracer->setVisible(true);
    _itemTracer->setStyle(QCPItemTracer::TracerStyle::tsCircle);
    _itemTracer->setClipToAxisRect(false);

    setFlag(QQuickItem::ItemHasContents, true);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &QQuickPaintedItem::widthChanged, this, &CorrelationPlotItem::updatePlotSize);
    connect(this, &QQuickPaintedItem::widthChanged, this, &CorrelationPlotItem::rangeSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &CorrelationPlotItem::updatePlotSize);
    connect(&_customPlot, &QCustomPlot::afterReplot, this, &CorrelationPlotItem::onCustomReplot);
}

void CorrelationPlotItem::refresh()
{
    updatePlotSize();
    buildPlot();
    _customPlot.replot();
}

void CorrelationPlotItem::paint(QPainter* painter)
{
    QPixmap picture(boundingRect().size().toSize());
    QCPPainter qcpPainter(&picture);

    _customPlot.toPainter(&qcpPainter);

    painter->drawPixmap(QPoint(), picture);
}

void CorrelationPlotItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void CorrelationPlotItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::RightButton)
        emit rightClick();
}

void CorrelationPlotItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void CorrelationPlotItem::hoverMoveEvent(QHoverEvent* event)
{
    _hoverPoint = event->posF();

    auto* currentPlottable = _customPlot.plottableAt(event->posF(), true);
    if(_hoverPlottable != currentPlottable)
    {
        _hoverPlottable = currentPlottable;
        hideTooltip();
    }

    if(_hoverPlottable != nullptr)
        showTooltip();
}

void CorrelationPlotItem::hoverLeaveEvent(QHoverEvent*)
{
    hideTooltip();
}

void CorrelationPlotItem::mouseDoubleClickEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void CorrelationPlotItem::wheelEvent(QWheelEvent* event)
{
    routeWheelEvent(event);
}

void CorrelationPlotItem::buildPlot()
{
    _customPlot.legend->setVisible(false);

    // If the legend is not cleared first this will cause a slowdown
    // when removing a large number of graphs
    _customPlot.legend->clear();
    _customPlot.clearGraphs();
    _customPlot.clearPlottables();

    while(_customPlot.plotLayout()->rowCount() > 1)
    {
        _customPlot.plotLayout()->removeAt(_customPlot.plotLayout()->rowColToIndex(1, 0));
        _customPlot.plotLayout()->simplify();
    }

    auto plotAveragingType = static_cast<PlotAveragingType>(_plotAveragingType);
    if(plotAveragingType == PlotAveragingType::MeanLine ||
            _selectedRows.length() > MAX_SELECTED_ROWS_BEFORE_MEAN)
        populateMeanLinePlot();
    else if(plotAveragingType == PlotAveragingType::MedianLine)
        populateMedianLinePlot();
    else if(plotAveragingType == PlotAveragingType::MeanHistogram)
        populateMeanHistogramPlot();
    else if(plotAveragingType == PlotAveragingType::IQRPlot)
        populateIQRPlot();
    else
        populateLinePlot();

    auto plotDeviationType = static_cast<PlotDeviationType>(_plotDeviationType);
    if(plotAveragingType != PlotAveragingType::Individual)
    {
        if(plotDeviationType == PlotDeviationType::StdDev)
            populateStdDevPlot();
        else if(plotDeviationType == PlotDeviationType::StdErr)
            populateStdErrorPlot();
    }

    if(_customPlot.graphCount() > 0 || _customPlot.plottableCount() > 0)
        _customPlot.legend->setVisible(_showLegend);

    QSharedPointer<QCPAxisTickerText> categoryTicker(new QCPAxisTickerText);
    _customPlot.xAxis->setTicker(categoryTicker);
    _customPlot.xAxis->setTickLabelRotation(90);

    bool columnNamesSuppressed = false;

    if(_showColumnNames)
    {
        if(_elideLabelWidth > 0)
        {
            QFontMetrics metrics(_defaultFont9Pt);
            int column = 0;

            for(auto& labelName : _labelNames)
                categoryTicker->addTick(column++, metrics.elidedText(labelName, Qt::ElideRight, _elideLabelWidth));
        }
        else
        {
            auto* text = new QCPTextElement(&_customPlot);
            text->setLayer(_textLayer);
            text->setTextFlags(Qt::AlignLeft);
            text->setFont(_defaultFont9Pt);
            text->setTextColor(Qt::gray);
            text->setText(tr("Resize To Expose Column Names"));
            text->setVisible(true);

            _customPlot.plotLayout()->insertRow(1);
            _customPlot.plotLayout()->addElement(1, 0, text);

            columnNamesSuppressed = true;
        }
    }

    if(columnNamesSuppressed && _customPlot.plotLayout()->rowCount() > 1)
    {
        auto margins = _customPlot.axisRect()->margins();
        margins.setBottom(0);
        _customPlot.axisRect()->setAutoMargins(QCP::MarginSide::msLeft|
                                               QCP::MarginSide::msRight|
                                               QCP::MarginSide::msTop);
        _customPlot.axisRect()->setMargins(margins);
    }
    else
        _customPlot.axisRect()->setAutoMargins(QCP::MarginSide::msAll);

    _customPlot.setBackground(Qt::white);
}

void CorrelationPlotItem::setPlotScaleType(const int &plotScaleType)
{
    _plotScaleType = plotScaleType;
    emit plotOptionsChanged();
    refresh();
}

void CorrelationPlotItem::setPlotAveragingType(const int &plotAveragingType)
{
    _plotAveragingType = plotAveragingType;
    emit plotOptionsChanged();
    refresh();
}

void CorrelationPlotItem::setPlotDeviationType(const int &plotDeviationType)
{
    _plotDeviationType = plotDeviationType;
    emit plotOptionsChanged();
    refresh();
}

void CorrelationPlotItem::setShowLegend(bool showLegend)
{
    _showLegend = showLegend;
    emit plotOptionsChanged();
    refresh();
}

void CorrelationPlotItem::populateMeanLinePlot()
{
    double maxY = 0.0;
    double minY = 0.0;

    auto* graph = _customPlot.addGraph();
    graph->setPen(QPen(Qt::black));
    graph->setName(tr("Mean average of selection"));

    QVector<double> xData(static_cast<int>(_columnCount));
    // xData is just the column indecides
    std::iota(std::begin(xData), std::end(xData), 0);

    // Use Average Calculation and set min / max
    QVector<double> yDataAvg = meanAverageData(minY, maxY);

    graph->setData(xData, yDataAvg, true);

    auto* plotModeTextElement = new QCPTextElement(&_customPlot);
    plotModeTextElement->setLayer(_textLayer);
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Mean average plot of %1 rows (maximum row count for individual plots is %2)"))
                .arg(_selectedRows.length())
                .arg(MAX_SELECTED_ROWS_BEFORE_MEAN));
    plotModeTextElement->setVisible(true);

    _customPlot.plotLayout()->insertRow(1);
    _customPlot.plotLayout()->addElement(1, 0, plotModeTextElement);

    scaleXAxis();
    _customPlot.yAxis->setRange(minY, maxY);
}

void CorrelationPlotItem::populateMedianLinePlot()
{
    double maxY = 0.0;
    double minY = 0.0;

    auto* graph = _customPlot.addGraph();
    graph->setPen(QPen(Qt::black));
    graph->setName(tr("Median average of selection"));

    QVector<double> xData(static_cast<int>(_columnCount));
    // xData is just the column indecides
    std::iota(std::begin(xData), std::end(xData), 0);

    QVector<double> rowsEntries(_selectedRows.length());
    QVector<double> yDataAvg(static_cast<int>(_columnCount));

    for(int col = 0; col < static_cast<int>(_columnCount); col++)
    {
        rowsEntries.clear();
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            rowsEntries.push_back(_data[static_cast<int>(index)]);
        }
        if(_selectedRows.size() > 0)
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

    auto* plotModeTextElement = new QCPTextElement(&_customPlot);
    plotModeTextElement->setLayer(_textLayer);
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Median average plot of %1 rows")
                .arg(_selectedRows.length())));
    plotModeTextElement->setVisible(true);

    _customPlot.plotLayout()->insertRow(1);
    _customPlot.plotLayout()->addElement(1, 0, plotModeTextElement);

    scaleXAxis();
    _customPlot.yAxis->setRange(minY, maxY);
}

void CorrelationPlotItem::populateMeanHistogramPlot()
{
    if(_selectedRows.size() == 0)
        return;

    double maxY = 0.0;
    double minY = 0.0;

    QVector<double> xData(static_cast<int>(_columnCount));
    // xData is just the column indecides
    std::iota(std::begin(xData), std::end(xData), 0);

    // Use Average Calculation and set min / max
    QVector<double> yDataAvg = meanAverageData(minY, maxY);

    QCPBars *histogramBars = new QCPBars(_customPlot.xAxis, _customPlot.yAxis);
    histogramBars->setName(tr("Mean histogram of selection"));
    histogramBars->setData(xData, yDataAvg, true);

    auto* plotModeTextElement = new QCPTextElement(&_customPlot);
    plotModeTextElement->setLayer(_textLayer);
    plotModeTextElement->setTextFlags(Qt::AlignLeft);
    plotModeTextElement->setFont(_defaultFont9Pt);
    plotModeTextElement->setTextColor(Qt::gray);
    plotModeTextElement->setText(
        QString(tr("*Mean histogram of %1 rows"))
                .arg(_selectedRows.length()));
    plotModeTextElement->setVisible(true);

    _customPlot.plotLayout()->insertRow(1);
    _customPlot.plotLayout()->addElement(1, 0, plotModeTextElement);

    scaleXAxis();
    _customPlot.yAxis->setRange(minY, maxY);
}

double calculateMedian(const QVector<double>& sortedData)
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

    QCPStatisticalBox *statPlot = new QCPStatisticalBox(_customPlot.xAxis, _customPlot.yAxis);
    statPlot->setName(tr("IQR of selection"));

    double maxY = 0.0;
    double minY = 0.0;

    QVector<double> rowsEntries(_selectedRows.length());
    QVector<double> outliers;

    for(int col = 0; col < static_cast<int>(_columnCount); col++)
    {
        rowsEntries.clear();
        outliers.clear();
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            rowsEntries.push_back(_data[static_cast<int>(index)]);
        }
        if(_selectedRows.size() > 0)
        {
            std::sort(rowsEntries.begin(), rowsEntries.end());
            double secondQuartile = calculateMedian(rowsEntries);
            double firstQuartile = secondQuartile;
            double thirdQuartile = secondQuartile;

            if(rowsEntries.size() > 1)
            {
                if(rowsEntries.size() % 2 == 0)
                {
                    firstQuartile = calculateMedian(
                                rowsEntries.mid(0, (rowsEntries.size() / 2)));
                    thirdQuartile = calculateMedian(
                                rowsEntries.mid((rowsEntries.size() / 2)));
                }
                else
                {
                    firstQuartile = calculateMedian(
                                rowsEntries.mid(0, ((rowsEntries.size() - 1) / 2)));
                    thirdQuartile = calculateMedian(
                                rowsEntries.mid(((rowsEntries.size() + 1) / 2)));
                }
            }

            double iqr = thirdQuartile - firstQuartile;
            double minValue = secondQuartile;
            double maxValue = secondQuartile;

            for(auto row : rowsEntries)
            {
                // Find Maximum and minimum non-outliers
                if(row < thirdQuartile + (iqr * 1.5))
                {
                    maxValue = std::max(maxValue, row);
                }
                if(row > firstQuartile - (iqr * 1.5))
                {
                    minValue = std::min(minValue, row);
                }

                // Find outliers
                if(row > thirdQuartile + (iqr * 1.5))
                {
                    outliers.push_back(row);
                }
                else if(row < firstQuartile - (iqr * 1.5))
                {
                    outliers.push_back(row);
                }

                maxY = std::max(maxY, row);
                minY = std::min(minY, row);
            }

            statPlot->addData(col, minValue, firstQuartile, secondQuartile, thirdQuartile, maxValue, outliers);
        }
    }

    _customPlot.yAxis->setRange(minY, maxY);
}

void CorrelationPlotItem::populateStdDevPlot()
{
    double min = 0, max = 0;

    QCPErrorBars* stdDevBars = new QCPErrorBars(_customPlot.xAxis, _customPlot.yAxis);
    stdDevBars->removeFromLegend();
    stdDevBars->setAntialiased(false);
    stdDevBars->setSelectable(QCP::SelectionType::stNone);
    stdDevBars->setDataPlottable(_customPlot.plottable(0));

    QVector<double> stdDevs(static_cast<int>(_columnCount));
    QVector<double> means(static_cast<int>(_columnCount));

    for(int col = 0; col < static_cast<int>(_columnCount); col++)
    {
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            means[col] += _data[static_cast<int>(index)];
        }
        means[col] /= _selectedRows.count();

        double stdDev = 0.0;
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            stdDev += (_data[static_cast<int>(index)] - means[col]) *
                    (_data[static_cast<int>(index)] - means[col]);
        }
        stdDev /= _columnCount;
        stdDev = std::sqrt(stdDev);
        stdDevs[col] = stdDev;

        min = std::min(min, means[col] - stdDev);
        max = std::max(max, means[col] + stdDev);
    }
    stdDevBars->setData(stdDevs);
    _customPlot.yAxis->setRange(min, max);
}

void CorrelationPlotItem::populateStdErrorPlot()
{
    double min = 0, max = 0;

    QCPErrorBars* stdErrBars = new QCPErrorBars(_customPlot.xAxis, _customPlot.yAxis);
    stdErrBars->removeFromLegend();
    stdErrBars->setSelectable(QCP::SelectionType::stNone);
    stdErrBars->setAntialiased(false);
    stdErrBars->setDataPlottable(_customPlot.plottable(0));

    QVector<double> stdErrs(static_cast<int>(_columnCount));
    QVector<double> means(static_cast<int>(_columnCount));

    for(int col = 0; col < static_cast<int>(_columnCount); col++)
    {
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            means[col] += _data[static_cast<int>(index)];
        }
        means[col] /= _selectedRows.count();

        double stdErr = 0.0;
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            stdErr += (_data[static_cast<int>(index)] - means[col]) *
                    (_data[static_cast<int>(index)] - means[col]);
        }
        stdErr /= _columnCount;
        stdErr = std::sqrt(stdErr) / std::sqrt(static_cast<double>(_selectedRows.length()));
        stdErrs[col] = stdErr;

        min = std::min(min, means[col] - stdErr);
        max = std::max(max, means[col] + stdErr);
    }
    stdErrBars->setData(stdErrs);
    _customPlot.yAxis->setRange(min, max);
}

void CorrelationPlotItem::populateLinePlot()
{
    double maxY = 0.0;
    double minY = 0.0;

    QVector<double> yData; yData.reserve(_selectedRows.size());
    QVector<double> xData; xData.reserve(static_cast<int>(_columnCount));

    // Plot each row individually
    for(auto row : qAsConst(_selectedRows))
    {
        auto* graph = _customPlot.addGraph();
        graph->setPen(_rowColors.at(row));
        graph->setName(_graphNames[row]);

        yData.clear();
        xData.clear();

        double rowMean = 0;

        for(size_t col = 0; col < _columnCount; col++)
        {
            auto index = (row * _columnCount) + col;
            rowMean += _data[static_cast<int>(index)];
        }
        rowMean /= _columnCount;

        double stdDev = 0;
        for(size_t col = 0; col < _columnCount; col++)
        {
            auto index = (row * _columnCount) + col;
            stdDev += (_data[static_cast<int>(index)] - rowMean) *
                    (_data[static_cast<int>(index)] - rowMean);
        }
        stdDev /= _columnCount;
        stdDev = std::sqrt(stdDev);
        double pareto = std::sqrt(stdDev);

        for(size_t col = 0; col < _columnCount; col++)
        {
            auto index = (row * _columnCount) + col;
            auto data = _data[static_cast<int>(index)];
            switch(static_cast<PlotScaleType>(_plotScaleType))
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
    _customPlot.yAxis->setRange(minY, maxY);
}

void CorrelationPlotItem::setSelectedRows(const QVector<int>& selectedRows)
{
    _selectedRows = selectedRows;
    refresh();
}

void CorrelationPlotItem::setRowColors(const QVector<QColor>& rowColors)
{
    _rowColors = rowColors;
    refresh();
}

void CorrelationPlotItem::setLabelNames(const QStringList& labelNames)
{
    _labelNames = labelNames;
}

void CorrelationPlotItem::setElideLabelWidth(int elideLabelWidth)
{
    bool changed = (_elideLabelWidth != elideLabelWidth);
    _elideLabelWidth = elideLabelWidth;

    if(changed && _showColumnNames)
        refresh();
}

void CorrelationPlotItem::setColumnCount(size_t columnCount)
{
    _columnCount = columnCount;
}

void CorrelationPlotItem::setShowColumnNames(bool showColumnNames)
{
    bool changed = (_showColumnNames != showColumnNames);
    _showColumnNames = showColumnNames;

    if(changed)
    {
        emit rangeSizeChanged();
        refresh();
    }
}

void CorrelationPlotItem::setShowGridLines(bool showGridLines)
{
    _showGridLines = showGridLines;
    _customPlot.xAxis->grid()->setVisible(_showGridLines);
    _customPlot.yAxis->grid()->setVisible(_showGridLines);
    refresh();
}

void CorrelationPlotItem::setScrollAmount(double scrollAmount)
{
    _scrollAmount = scrollAmount;
    scaleXAxis();
    _customPlot.replot();
}

void CorrelationPlotItem::scaleXAxis()
{
    auto maxX = static_cast<double>(_columnCount);
    if(_showColumnNames)
    {
        double visiblePlotWidth = columnAxisWidth();
        double textHeight = columnLabelSize();

        double position = (_columnCount - (visiblePlotWidth / textHeight)) * _scrollAmount;

        if(position + (visiblePlotWidth / textHeight) <= maxX)
            _customPlot.xAxis->setRange(position, position + (visiblePlotWidth / textHeight));
        else
            _customPlot.xAxis->setRange(0, maxX);
    }
    else
    {
        _customPlot.xAxis->setRange(0, maxX);
    }
}

QVector<double> CorrelationPlotItem::meanAverageData(double &min, double &max)
{
    min = 0.0f;
    max = 0.0f;

    // Use Average Calculation
    QVector<double> yDataAvg; yDataAvg.reserve(_selectedRows.size());

    for(size_t col = 0; col < _columnCount; col++)
    {
        double runningTotal = 0.0;
        for(auto row : qAsConst(_selectedRows))
        {
            auto index = (row * _columnCount) + col;
            runningTotal += _data[static_cast<int>(index)];
        }
        yDataAvg.append(runningTotal / _selectedRows.length());

        max = std::max(max, yDataAvg.back());
        min = std::min(min, yDataAvg.back());
    }
    return yDataAvg;
}

double CorrelationPlotItem::rangeSize()
{
    if(_showColumnNames)
        return (columnAxisWidth() / (columnLabelSize() * _columnCount));
    else
        return 1.0;
}

double CorrelationPlotItem::columnLabelSize()
{
    QFontMetrics metrics(_defaultFont9Pt);
    const unsigned int columnPadding = 1;
    return metrics.height() + columnPadding;
}

double CorrelationPlotItem::columnAxisWidth()
{
    const auto& margins = _customPlot.axisRect()->margins();
    const unsigned int axisWidth = margins.left() + margins.right();

    return width() - axisWidth;
}

void CorrelationPlotItem::routeMouseEvent(QMouseEvent* event)
{
    auto* newEvent = new QMouseEvent(event->type(), event->localPos(),
                                     event->button(), event->buttons(),
                                     event->modifiers());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void CorrelationPlotItem::routeWheelEvent(QWheelEvent* event)
{
    auto* newEvent = new QWheelEvent(event->pos(), event->delta(),
                                     event->buttons(), event->modifiers(),
                                     event->orientation());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void CorrelationPlotItem::updatePlotSize()
{
    _customPlot.setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));
    scaleXAxis();
}

void CorrelationPlotItem::showTooltip()
{
    _itemTracer->setGraph(nullptr);
    if(auto graph = dynamic_cast<QCPGraph*>(_hoverPlottable))
    {
        _itemTracer->setGraph(graph);
        _itemTracer->setGraphKey(_customPlot.xAxis->pixelToCoord(_hoverPoint.x()));
    }
    else if(auto bars = dynamic_cast<QCPBars*>(_hoverPlottable))
    {
        auto xCoord = _customPlot.xAxis->pixelToCoord(_hoverPoint.x()) + 0.5f;
        _itemTracer->position->setPixelPosition(bars->dataPixelPosition(xCoord));
    }

    _itemTracer->setVisible(true);
    _itemTracer->setInterpolating(false);

    _hoverLabel->setVisible(true);
    _hoverLabel->setText(QStringLiteral("%1, %2: %3")
                     .arg(_hoverPlottable->name(),
                          _labelNames[static_cast<int>(_itemTracer->position->key())])
                     .arg(_itemTracer->position->value()));

    const auto COLOR_RECT_WIDTH = 10.0;
    const auto HOVER_MARGIN = 10.0;
    auto hoverlabelWidth = _hoverLabel->right->pixelPosition().x() -
            _hoverLabel->left->pixelPosition().x();
    auto hoverlabelHeight = _hoverLabel->bottom->pixelPosition().y() -
            _hoverLabel->top->pixelPosition().y();
    auto hoverLabelRightX = _itemTracer->anchor(QStringLiteral("position"))->pixelPosition().x() +
            hoverlabelWidth + HOVER_MARGIN + COLOR_RECT_WIDTH;
    auto xBounds = clipRect().width();
    QPointF targetPosition(_itemTracer->anchor(QStringLiteral("position"))->pixelPosition().x() + HOVER_MARGIN,
                           _itemTracer->anchor(QStringLiteral("position"))->pixelPosition().y());

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

    _hoverLabel->position->setPixelPosition(targetPosition);

    _hoverColorRect->setVisible(true);
    _hoverColorRect->setBrush(QBrush(_hoverPlottable->pen().color()));
    _hoverColorRect->bottomRight->setPixelPosition(QPointF(_hoverLabel->bottomRight->pixelPosition().x() + COLOR_RECT_WIDTH,
                                                   _hoverLabel->bottomRight->pixelPosition().y()));

    _textLayer->replot();

    update();
}

void CorrelationPlotItem::hideTooltip()
{
    _hoverLabel->setVisible(false);
    _hoverColorRect->setVisible(false);
    _itemTracer->setVisible(false);
    _textLayer->replot();
    update();
}

void CorrelationPlotItem::savePlotImage(const QUrl& url, const QStringList& extensions)
{
    if(extensions.contains(QStringLiteral("png")))
        _customPlot.savePng(url.toLocalFile());
    else if(extensions.contains(QStringLiteral("pdf")))
        _customPlot.savePdf(url.toLocalFile());
    else if(extensions.contains(QStringLiteral("jpg")))
        _customPlot.saveJpg(url.toLocalFile());

    QDesktopServices::openUrl(url);
}

void CorrelationPlotItem::onCustomReplot()
{
    update();
}
