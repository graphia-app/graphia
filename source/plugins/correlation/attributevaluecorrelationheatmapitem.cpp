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

#include "attributevaluecorrelationheatmapitem.h"

#include "shared/rendering/multisamples.h"

#include <vector>

AttributeValueCorrelationHeatmapItem::AttributeValueCorrelationHeatmapItem(QQuickItem* parent) :
    QCustomPlotQuickItem(multisamples(), parent),
    _colorMap(new QCPColorMap(customPlot().xAxis, customPlot().yAxis)),
    _tooltipLabel(new QCPItemText(&customPlot()))
{
    QCPColorGradient gradient;
    gradient.setColorStopAt(0.0, QColor(Qt::black));
    gradient.setColorStopAt(0.4, QColor(Qt::darkMagenta));
    gradient.setColorStopAt(0.6, QColor(Qt::red));
    gradient.setColorStopAt(0.8, QColor(Qt::yellow));
    gradient.setColorStopAt(1.0, QColor(Qt::white));

    _colorMap->setInterpolate(false);
    _colorMap->setGradient(gradient);
    _colorMap->setTightBoundary(true);

    customPlot().addLayer(u"textLayer"_s);
    _textLayer = customPlot().layer(u"textLayer"_s);
    _textLayer->setMode(QCPLayer::LayerMode::lmBuffered);

    QFont defaultFont10Pt;
    defaultFont10Pt.setPointSize(10);

    _tooltipLabel->setPositionAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    _tooltipLabel->setLayer(_textLayer);
    _tooltipLabel->setFont(defaultFont10Pt);
    _tooltipLabel->setPen(QPen(Qt::black));
    _tooltipLabel->setBrush(QBrush(Qt::white));
    _tooltipLabel->setPadding(QMargins(3, 3, 3, 3));
    _tooltipLabel->setClipToAxisRect(false);
    _tooltipLabel->setVisible(false);

    connect(this, &AttributeValueCorrelationHeatmapItem::widthChanged,
        this, &AttributeValueCorrelationHeatmapItem::updateLabelVisibility);
    connect(this, &AttributeValueCorrelationHeatmapItem::heightChanged,
        this, &AttributeValueCorrelationHeatmapItem::updateLabelVisibility);
}

void AttributeValueCorrelationHeatmapItem::setMaxNumAttributeValues(int maxNumAttributeValues)
{
    const bool changed = (_maxNumAttributeValues != maxNumAttributeValues);
    _maxNumAttributeValues = maxNumAttributeValues;

    if(changed)
        rebuild();
}

int AttributeValueCorrelationHeatmapItem::numAttributeValues() const
{
    return static_cast<int>(_data._values.size());
}

void AttributeValueCorrelationHeatmapItem::setElideLabelWidth(int elideLabelWidth)
{
    const bool changed = (_elideLabelWidth != elideLabelWidth);
    _elideLabelWidth = elideLabelWidth;

    if(changed)
        rebuild();
}

void AttributeValueCorrelationHeatmapItem::updateLabelVisibility()
{
    size_t maxNumAttributeValues = std::min(static_cast<size_t>(_maxNumAttributeValues), _data._values.size());

    QFont defaultFont9Pt;
    defaultFont9Pt.setPointSize(9);
    const QFontMetrics fontMetrics(defaultFont9Pt);

    auto w = width() - customPlot().axisRect()->margins().left();
    auto h = height() - customPlot().axisRect()->margins().top();
    bool showHorizontalLabels = (w / static_cast<double>(maxNumAttributeValues)) >
        static_cast<double>(fontMetrics.height());
    bool showVerticalLabels = (h / static_cast<double>(maxNumAttributeValues)) >
        static_cast<double>(fontMetrics.height());

    customPlot().xAxis->setTicks(showHorizontalLabels);
    customPlot().yAxis->setTicks(showVerticalLabels);
}

bool AttributeValueCorrelationHeatmapItem::valid() const
{
    return !_data._values.empty();
}

void AttributeValueCorrelationHeatmapItem::showTooltip()
{
    double key = 0.0, value = 0.0;
    _colorMap->pixelsToCoords(_hoverPoint, key, value);
    auto k = static_cast<size_t>(key);
    auto v = static_cast<size_t>(value);

    if(k > _data._values.size() || v > _data._values.size())
    {
        qDebug() << "AttributeValueCorrelationHeatmapItem::showTooltip() index out of bounds";
        return;
    }

    const auto dataValue = _data._heatmap.valueAt(_ordering.at(k), _ordering.at(v));
    const auto& keyLabel = _data._values.at(_ordering.at(k))._value;
    const auto& valueLabel = _data._values.at(_ordering.at(v))._value;

    const auto tooltipText = QObject::tr("%1 vs %2: %3")
        .arg(keyLabel, valueLabel, QString::number(dataValue, 'g', 3));

    _tooltipLabel->setVisible(true);
    _tooltipLabel->setText(tooltipText);

    QPointF targetPosition(_hoverPoint.x(), _hoverPoint.y());

    _tooltipLabel->position->setPixelPosition(targetPosition);

    update();
}

void AttributeValueCorrelationHeatmapItem::hideTooltip()
{
    _tooltipLabel->setVisible(false);
    update();
}

void AttributeValueCorrelationHeatmapItem::rebuild(const AttributeValueCorrelationHeatmapResult* data)
{
    if(data != nullptr)
        _data = *data;

    if(!valid())
    {
        emit validChanged();
        return;
    }

    emit numAttributeValuesChanged();

    const QSharedPointer<QCPAxisTickerText> xCategoryTicker(new QCPAxisTickerText);
    const QSharedPointer<QCPAxisTickerText> yCategoryTicker(new QCPAxisTickerText);

    customPlot().xAxis->setTicker(xCategoryTicker);
    customPlot().xAxis->setTickLabelRotation(90);
    customPlot().yAxis->setTicker(yCategoryTicker);

    customPlot().plotLayout()->setMargins(QMargins(0, 0, 0, 0));

    size_t maxNumAttributeValues = std::min(static_cast<size_t>(_maxNumAttributeValues), _data._values.size());

    _ordering.resize(_data._values.size());
    std::iota(_ordering.begin(), _ordering.end(), 0);

    QCollator collator;
    collator.setNumericMode(true);

    std::sort(_ordering.begin(), _ordering.end(), [this, &collator](size_t a, size_t b)
    {
        auto aSize = _data._values.at(a)._rows.size();
        auto bSize = _data._values.at(b)._rows.size();

        if(aSize == bSize)
        {
            const auto& aValue = _data._values.at(a)._value;
            const auto& bValue = _data._values.at(b)._value;

            return collator.compare(aValue, bValue) < 0;
        }

        return bSize < aSize;
    });

    customPlot().xAxis->setRange(0, static_cast<int>(maxNumAttributeValues));
    customPlot().yAxis->setRange(0, static_cast<int>(maxNumAttributeValues));

    QFont defaultFont9Pt;
    defaultFont9Pt.setPointSize(9);
    const QFontMetrics fontMetrics(defaultFont9Pt);

    for(size_t i = 0; i < maxNumAttributeValues; i++)
    {
        const auto p = static_cast<double>(i) + 0.5;
        const auto& value = _data._values.at(_ordering.at(i))._value;

        if(_elideLabelWidth > 0)
        {
            xCategoryTicker->addTick(p, fontMetrics.elidedText(value, Qt::ElideRight, _elideLabelWidth));
            yCategoryTicker->addTick(p, fontMetrics.elidedText(value, Qt::ElideRight, _elideLabelWidth));
        }
        else
        {
            xCategoryTicker->addTick(p, value);
            yCategoryTicker->addTick(p, value);
        }
    }

    customPlot().xAxis->setTickPen(QPen(Qt::transparent));
    customPlot().yAxis->setTickPen(QPen(Qt::transparent));

    updateLabelVisibility();

    // Over-allocate a border which will be populated with values clamped to the
    // actual visible cells; this avoids non data cell colours bleeding into adjacent ones
    _colorMap->data()->setSize(
        static_cast<int>(maxNumAttributeValues) + 2,
        static_cast<int>(maxNumAttributeValues) + 2);

    _colorMap->data()->setRange(
        QCPRange(-0.5, static_cast<double>(maxNumAttributeValues) + 0.5),
        QCPRange(-0.5, static_cast<double>(maxNumAttributeValues) + 0.5));

    for(int y = 0; y < _colorMap->data()->valueSize(); y++)
    {
        for(int x = 0; x < _colorMap->data()->keySize(); x++)
        {
            auto column = std::clamp(static_cast<size_t>(x - 1), static_cast<size_t>(0), maxNumAttributeValues - 1);
            auto row = std::clamp(static_cast<size_t>(y - 1), static_cast<size_t>(0), maxNumAttributeValues - 1);

            auto value = _data._heatmap.valueAt(_ordering.at(column), _ordering.at(row));
            _colorMap->data()->setCell(x, y, value);
        }
    }

    _colorMap->rescaleDataRange(true);

    customPlot().replot(QCustomPlot::rpQueuedReplot);
    emit validChanged();
}

void AttributeValueCorrelationHeatmapItem::reset()
{
    _data = {};
    emit validChanged();
}

void AttributeValueCorrelationHeatmapItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton)
    {
        double key = 0.0, value = 0.0;
        _colorMap->pixelsToCoords(event->pos(), key, value);
        auto k = static_cast<size_t>(key);
        auto v = static_cast<size_t>(value);

        if(k > _data._values.size() || v > _data._values.size())
            return;

        const auto& keyLabel = _data._values.at(_ordering.at(k))._value;
        const auto& valueLabel = _data._values.at(_ordering.at(v))._value;

        emit cellClicked(keyLabel, valueLabel);
    }
}

void AttributeValueCorrelationHeatmapItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
    hideTooltip();
}

void AttributeValueCorrelationHeatmapItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void AttributeValueCorrelationHeatmapItem::hoverMoveEvent(QHoverEvent* event)
{
    _hoverPoint = event->position();

    auto* currentPlottable = customPlot().plottableAt(event->position(), true);
    if(_tooltipPlottable != currentPlottable)
    {
        _tooltipPlottable = currentPlottable;
        hideTooltip();
    }

    if(_tooltipPlottable != nullptr)
        showTooltip();
}

void AttributeValueCorrelationHeatmapItem::hoverLeaveEvent(QHoverEvent*)
{
    hideTooltip();
}
