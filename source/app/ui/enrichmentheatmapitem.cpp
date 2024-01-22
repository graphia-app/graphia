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

#include "enrichmentheatmapitem.h"

#include "shared/utils/utils.h"
#include "shared/utils/string.h"
#include "shared/utils/container.h"
#include "shared/utils/static_block.h"
#include "shared/rendering/multisamples.h"

#include <QQmlEngine>
#include <QtGlobal>

#include <set>
#include <iterator>

using namespace Qt::Literals::StringLiterals;

static const double HEATMAP_OFFSET = 0.5;

EnrichmentHeatmapItem::EnrichmentHeatmapItem(QQuickItem* parent) :
    QCustomPlotQuickItem(multisamples(), parent),
    _colorMap(new QCPColorMap(customPlot().xAxis, customPlot().yAxis2)),
    _colorScale(new QCPColorScale(&customPlot())),
    _hoverLabel(new QCPItemText(&customPlot()))
{
    customPlot().addLayer(u"textLayer"_s);
    customPlot().plotLayout()->setAutoMargins(QCP::MarginSide::msTop | QCP::MarginSide::msLeft);

    _colorScale->setLabel(tr("Bonferroni Adjusted P-Value"));
    _colorScale->setType(QCPAxis::atBottom);
    customPlot().plotLayout()->addElement(1, 0, _colorScale);
    _colorScale->setMinimumMargins(QMargins(6, 0, 6, 0));

    for(auto* axis : _colorScale->axis()->axisRect()->axes()) // clazy:exclude=range-loop-detach
        axis->setBasePen(QPen(penColor()));

    _colorScale->axis()->setTickPen(QPen(penColor()));
    _colorScale->axis()->setSubTickPen(QPen(penColor()));
    _colorScale->axis()->setTickLabelColor(penColor());
    _colorScale->axis()->setLabelColor(penColor());

    _textLayer = customPlot().layer(u"textLayer"_s);
    _textLayer->setMode(QCPLayer::LayerMode::lmBuffered);

    customPlot().yAxis2->setVisible(true);
    customPlot().yAxis->setVisible(false);

    auto colorScaleTicker = QSharedPointer<QCPAxisTickerText>::create();
    _colorScale->axis()->setTicker(colorScaleTicker);
  
    colorScaleTicker->addTick(0, u"0"_s);
    colorScaleTicker->addTick(0.01, u"0.01"_s);
    colorScaleTicker->addTick(0.02, u"0.02"_s);
    colorScaleTicker->addTick(0.03, u"0.03"_s);
    colorScaleTicker->addTick(0.04, u"0.04"_s);
    colorScaleTicker->addTick(0.05, u"0.05"_s);

    QCPColorGradient gradient;
    auto insignificantColor = QColor(Qt::gray);
    auto verySignificantColor = QColor(Qt::yellow);
    auto significantColor = QColor(Qt::red);
    gradient.setColorStopAt(0, verySignificantColor);
    gradient.setColorStopAt(5.0 / 6.0, significantColor);
    gradient.setColorStopAt(5.0 / 6.0 + 0.001, insignificantColor);
    gradient.setColorStopAt(1.0, insignificantColor);

    _colorMap->setInterpolate(false);
    _colorMap->setColorScale(_colorScale);
    _colorMap->setGradient(gradient);
    _colorMap->setTightBoundary(true);

    QFont defaultFont10Pt;
    defaultFont10Pt.setPointSize(10);

    _defaultFont9Pt.setPointSize(9);

    _hoverLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    _hoverLabel->setLayer(_textLayer);
    _hoverLabel->setFont(defaultFont10Pt);
    _hoverLabel->setPen(QPen(Qt::black));
    _hoverLabel->setBrush(QBrush(Qt::white));
    _hoverLabel->setPadding(QMargins(3, 3, 3, 3));
    _hoverLabel->setClipToAxisRect(false);
    _hoverLabel->setVisible(false);

    connect(this, &EnrichmentHeatmapItem::tableModelChanged, this, &EnrichmentHeatmapItem::buildPlot);
    connect(this, &EnrichmentHeatmapItem::tableModelChanged, this, &QQuickPaintedItem::widthChanged);
    connect(this, &EnrichmentHeatmapItem::tableModelChanged, this, &QQuickPaintedItem::heightChanged);

    connect(this, &QQuickPaintedItem::widthChanged, this, &EnrichmentHeatmapItem::horizontalRangeSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &EnrichmentHeatmapItem::verticalRangeSizeChanged);
    connect(this, &QQuickPaintedItem::widthChanged, this, &EnrichmentHeatmapItem::scaleXAxis);
    connect(this, &QQuickPaintedItem::heightChanged, this, &EnrichmentHeatmapItem::scaleYAxis);
}

void EnrichmentHeatmapItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        auto xCoord = static_cast<int>(std::round(customPlot().xAxis->pixelToCoord(event->pos().x())));
        auto yCoord = static_cast<int>(std::round(customPlot().yAxis2->pixelToCoord(event->pos().y())));
        emit plotValueClicked(_tableModel->rowFromAttributeSets(_xAxisToFullLabel[xCoord], _yAxisToFullLabel[yCoord]));
    }
}

void EnrichmentHeatmapItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    hideTooltip();
    if(event->button() == Qt::RightButton)
        emit rightClick();
}

void EnrichmentHeatmapItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void EnrichmentHeatmapItem::hoverMoveEvent(QHoverEvent *event)
{
    _hoverPoint = event->position();

    auto* currentPlottable = customPlot().plottableAt(event->position(), true);
    if(_hoverPlottable != currentPlottable)
    {
        _hoverPlottable = currentPlottable;
        hideTooltip();
    }

    if(_hoverPlottable != nullptr)
        showTooltip();
}

void EnrichmentHeatmapItem::hoverLeaveEvent(QHoverEvent *event)
{
    hideTooltip();
    Q_UNUSED(event);
}

void EnrichmentHeatmapItem::buildPlot()
{
    if(_tableModel == nullptr)
        return;

    const QSharedPointer<QCPAxisTickerText> xCategoryTicker(new QCPAxisTickerText);
    const QSharedPointer<QCPAxisTickerText> yCategoryTicker(new QCPAxisTickerText);

    customPlot().xAxis->setTicker(xCategoryTicker);
    customPlot().xAxis->setTickLabelRotation(90);

    customPlot().xAxis->setBasePen(QPen(penColor()));
    customPlot().xAxis->setTickPen(QPen(penColor()));
    customPlot().xAxis->setSubTickPen(QPen(penColor()));
    customPlot().xAxis->setTickLabelColor(penColor());

    customPlot().yAxis2->setTicker(yCategoryTicker);
    customPlot().yAxis2->setTickLabelColor(penColor());

    customPlot().yAxis2->setBasePen(QPen(penColor()));
    customPlot().yAxis2->setTickPen(QPen(penColor()));
    customPlot().yAxis2->setSubTickPen(QPen(penColor()));
    customPlot().yAxis2->setTickLabelColor(penColor());

    customPlot().plotLayout()->setMargins(QMargins(0, 0, 0, 0));

    if(!_xAxisLabel.isEmpty())
    {
        customPlot().xAxis->setLabel(_xAxisLabel);
        customPlot().xAxis->setLabelColor(penColor());
    }

    if(!_yAxisLabel.isEmpty())
    {
        customPlot().yAxis2->setLabel(_yAxisLabel);
        customPlot().yAxis2->setLabelColor(penColor());
    }

    std::set<QString> attributeValueSetA;
    std::set<QString> attributeValueSetB;
    std::map<QString, int> fullLabelToXAxis;
    std::map<QString, int> fullLabelToYAxis;

    _xAxisToFullLabel.clear();
    _yAxisToFullLabel.clear();
    _colorMapKeyValueToTableIndex.clear();

    for(int i = 1; i < _tableModel->rowCount(); ++i)
    {
        attributeValueSetA.insert(_tableModel->data(i, EnrichmentTableModel::Results::SelectionA).toString());
        attributeValueSetB.insert(_tableModel->data(i, EnrichmentTableModel::Results::SelectionB).toString());
    }

    // Sensible sort strings using numbers
    auto compare = [](const auto& a, const auto& b) { return u::numericCompare(a, b) < 0; };
    std::vector<QString> sortAttributeValueSetA(attributeValueSetA.begin(), attributeValueSetA.end());
    std::vector<QString> sortAttributeValueSetB(attributeValueSetB.begin(), attributeValueSetB.end());
    std::sort(sortAttributeValueSetA.begin(), sortAttributeValueSetA.end(), compare);
    std::sort(sortAttributeValueSetB.rbegin(), sortAttributeValueSetB.rend(), compare);

    const QFontMetrics metrics(_defaultFont9Pt);

    int column = 0;
    for(const auto& labelName: sortAttributeValueSetA)
    {
        fullLabelToXAxis[labelName] = column;
        _xAxisToFullLabel[column] = labelName;
        if(_elideLabelWidth > 0)
            xCategoryTicker->addTick(column++, metrics.elidedText(labelName, Qt::ElideRight, _elideLabelWidth));
        else
            xCategoryTicker->addTick(column++, labelName);
    }
    column = 0;
    for(const auto& labelName: sortAttributeValueSetB)
    {
        fullLabelToYAxis[labelName] = column;
        _yAxisToFullLabel[column] = labelName;
        if(_elideLabelWidth > 0)
            yCategoryTicker->addTick(column++, metrics.elidedText(labelName, Qt::ElideRight, _elideLabelWidth));
        else
            yCategoryTicker->addTick(column++, labelName);
    }

    // WARNING: Colour maps seem to overdraw the map size, this means hover events won't be triggered on the
    // overdrawn edges. As a fix I add a 1 cell margin on all sides of the map, offset the data by 1 cell
    // and range it to match
    _colorMap->data()->setSize(static_cast<int>(attributeValueSetA.size() + 2), static_cast<int>(attributeValueSetB.size() + 2));
    _colorMap->data()->setRange(
        QCPRange(-1.0, static_cast<double>(attributeValueSetA.size())),
        QCPRange(-1.0, static_cast<double>(attributeValueSetB.size())));

    _attributeACount = static_cast<int>(attributeValueSetA.size());
    _attributeBCount = static_cast<int>(attributeValueSetB.size());

    for(int i = 1; i < _tableModel->rowCount(); i++)
    {
        // The data is offset by 1 to account for the empty margin
        // Set the data of the cell
        auto xValue = fullLabelToXAxis[_tableModel->data(i, EnrichmentTableModel::Results::SelectionA).toString()];
        auto yValue = fullLabelToYAxis[_tableModel->data(i, EnrichmentTableModel::Results::SelectionB).toString()];

        auto pValue = _tableModel->data(i, EnrichmentTableModel::Results::BonferroniAdjusted).toDouble();

        _colorMapKeyValueToTableIndex.emplace(std::make_pair(xValue, yValue), i);

        if(_showOnlyEnriched)
        {
            auto overRep = _tableModel->data(i, EnrichmentTableModel::Results::OverRep).toDouble();
            if(overRep <= 1.0)
            {
                // Set a value that will map to grey, so that the heatmap matches the table
                pValue = 1.0;
            }
        }

        _colorMap->data()->setCell(xValue + 1, yValue + 1, pValue);

        // Ugly hack: Colors blend from margin cells. I recolour them to match adjacent cells so you can't tell
        // 200 IQ fix really...
        if(xValue == 0)
        {
            _colorMap->data()->setCell(xValue, yValue + 1,
                _colorMap->data()->cell(xValue + 1, yValue + 1));
        }

        if(yValue == static_cast<int>(attributeValueSetB.size()) - 1)
        {
            _colorMap->data()->setCell(xValue + 1, yValue + 2,
                _colorMap->data()->cell(xValue + 1, yValue + 1));
        }
    }
    _colorScale->setDataRange(QCPRange(0, 0.06));

    customPlot().replot(QCustomPlot::rpQueuedReplot);
}

double EnrichmentHeatmapItem::columnAxisWidth()
{
    const auto& margins = customPlot().axisRect()->margins();
    const auto axisWidth = margins.left() + margins.right();

    return width() - axisWidth;
}

double EnrichmentHeatmapItem::columnAxisHeight()
{
    const auto& margins = customPlot().axisRect()->margins();
    const auto axisHeight = margins.top() + margins.bottom();

    return height() - axisHeight;
}

void EnrichmentHeatmapItem::scaleXAxis()
{
    auto maxX = static_cast<double>(_attributeACount);
    const double visiblePlotWidth = columnAxisWidth();
    const double textHeight = columnLabelSize();

    const double position = (_attributeACount - (visiblePlotWidth / textHeight)) * _scrollXAmount;

    if(position + (visiblePlotWidth / textHeight) <= maxX)
        customPlot().xAxis->setRange(position - HEATMAP_OFFSET, position + (visiblePlotWidth / textHeight) - HEATMAP_OFFSET);
    else
        customPlot().xAxis->setRange(-HEATMAP_OFFSET, maxX - HEATMAP_OFFSET);
}

void EnrichmentHeatmapItem::scaleYAxis()
{
    auto maxY = static_cast<double>(_attributeBCount);
    const double visiblePlotHeight = columnAxisHeight();
    const double textHeight = columnLabelSize();

    const double position = (_attributeBCount - (visiblePlotHeight / textHeight)) * (1.0-_scrollYAmount);

    if((visiblePlotHeight / textHeight) <= maxY)
        customPlot().yAxis2->setRange(position - HEATMAP_OFFSET, position + (visiblePlotHeight / textHeight) - HEATMAP_OFFSET);
    else
        customPlot().yAxis2->setRange(-HEATMAP_OFFSET, maxY - HEATMAP_OFFSET);
}

void EnrichmentHeatmapItem::setElideLabelWidth(int elideLabelWidth)
{
    const bool changed = (_elideLabelWidth != elideLabelWidth);
    _elideLabelWidth = elideLabelWidth;

    if(changed)
    {
        updatePlotSize();
        buildPlot();
    }
}

void EnrichmentHeatmapItem::setXAxisLabel(const QString& xAxisLabel)
{
    if(xAxisLabel != _xAxisLabel)
    {
        _xAxisLabel = xAxisLabel;
        buildPlot();
    }
}

void EnrichmentHeatmapItem::setYAxisLabel(const QString& yAxisLabel)
{
    if(yAxisLabel != _yAxisLabel)
    {
        _yAxisLabel = yAxisLabel;
        buildPlot();
    }
}

void EnrichmentHeatmapItem::setShowOnlyEnriched(bool showOnlyEnriched)
{
    if(showOnlyEnriched != _showOnlyEnriched)
    {
        _showOnlyEnriched = showOnlyEnriched;
        buildPlot();
        emit showOnlyEnrichedChanged();
    }
}

void EnrichmentHeatmapItem::setScrollXAmount(double scrollAmount)
{
    _scrollXAmount = scrollAmount;
    scaleXAxis();
    customPlot().replot();
}

void EnrichmentHeatmapItem::setScrollYAmount(double scrollAmount)
{
    _scrollYAmount = scrollAmount;
    scaleYAxis();
    customPlot().replot();
    update();
}

double EnrichmentHeatmapItem::columnLabelSize()
{
    const QFontMetrics metrics(_defaultFont9Pt);
    const auto columnPadding = 1;
    return metrics.height() + columnPadding;
}

double EnrichmentHeatmapItem::horizontalRangeSize()
{
    return (columnAxisWidth() / (columnLabelSize() * _attributeACount));
}

double EnrichmentHeatmapItem::verticalRangeSize()
{
    return (columnAxisHeight() / (columnLabelSize() * _attributeBCount));
}

void EnrichmentHeatmapItem::showTooltip()
{
    double key = 0.0, value = 0.0;
    _colorMap->pixelsToCoords(_hoverPoint, key, value);

    const std::pair<int, int> colorMapIndexPair = {std::round(key), std::round(value)};
    if(!u::containsKey(_colorMapKeyValueToTableIndex, colorMapIndexPair))
        return;

    auto tableIndex = _colorMapKeyValueToTableIndex.at(colorMapIndexPair);

    // Bounds check the row
    if(tableIndex >= _tableModel->rowCount() || tableIndex < 0)
        return;

    _hoverLabel->setVisible(true);

    auto pValue = _tableModel->data(tableIndex,
        EnrichmentTableModel::Results::BonferroniAdjusted).toDouble();

    _hoverLabel->setText(tr("Adj. P-value: %1").arg(u::formatNumberScientific(pValue)));

    const auto COLOR_RECT_WIDTH = 10.0;
    const auto HOVER_MARGIN = 10.0;
    auto hoverlabelWidth = _hoverLabel->right->pixelPosition().x() -
            _hoverLabel->left->pixelPosition().x();
    auto hoverlabelHeight = _hoverLabel->bottom->pixelPosition().y() -
            _hoverLabel->top->pixelPosition().y();
    auto hoverLabelRightX = _hoverPoint.x() +
            hoverlabelWidth + HOVER_MARGIN + COLOR_RECT_WIDTH;
    auto xBounds = clipRect().width();
    QPointF targetPosition(_hoverPoint.x() + HOVER_MARGIN,
                           _hoverPoint.y());

    // If it falls out of bounds, clip to bounds and move label above marker
    // yAxisPadding accounts for scrollbar spacing
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

    update();
}

void EnrichmentHeatmapItem::savePlotImage(const QUrl& url, const QStringList& extensions)
{
    if(extensions.contains(u"png"_s))
        customPlot().savePng(url.toLocalFile());
    else if(extensions.contains(u"pdf"_s))
        customPlot().savePdf(url.toLocalFile());
    else if(extensions.contains(u"jpg"_s))
        customPlot().saveJpg(url.toLocalFile());

#ifndef Q_OS_WASM
    QDesktopServices::openUrl(url);
#endif
}

void EnrichmentHeatmapItem::hideTooltip()
{
    _hoverLabel->setVisible(false);
    update();
}

static_block
{
    qmlRegisterType<EnrichmentHeatmapItem>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "EnrichmentHeatmap");
}
