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
#include "qcpcolumnannotations.h"

#include "shared/utils/container.h"

void CorrelationPlotItem::updateColumnAnnotationVisibility()
{
    auto mainPlotHeight = height() - columnAnnotaionsHeight(_columnAnnotationSelectionModeEnabled);
    bool showColumnAnnotations = mainPlotHeight >= minimumHeight();

    if(showColumnAnnotations != _showColumnAnnotations)
    {
        _showColumnAnnotations = showColumnAnnotations;

        // If we can't show column annotations, we also can't be in selection mode
        if(!_showColumnAnnotations)
            setColumnAnnotationSelectionModeEnabled(false);

        rebuildPlot();
    }
}

bool CorrelationPlotItem::canShowColumnAnnotationSelection() const
{
    auto mainPlotHeight = height() - columnAnnotaionsHeight(true);
    return mainPlotHeight >= minimumHeight();
}

QCPAxis* CorrelationPlotItem::configureColumnAnnotations(QCPAxisRect* axisRect)
{
    auto* xAxis = axisRect->axis(QCPAxis::atBottom);

    auto* layoutGrid = dynamic_cast<QCPLayoutGrid*>(axisRect->layout());
    Q_ASSERT(layoutGrid != nullptr);

    // Find the column in which axisRect exists
    int layoutColumn = 0;
    for(; layoutColumn < layoutGrid->columnCount(); layoutColumn++)
    {
        if(layoutGrid->element(0, layoutColumn) == axisRect)
            break;
    }

    Q_ASSERT(layoutColumn < layoutGrid->columnCount());

    auto* layoutElement = layoutGrid->hasElement(1, layoutColumn) ? layoutGrid->element(1, layoutColumn) : nullptr;

    const auto& columnAnnotations = _pluginInstance->columnAnnotations();
    bool invisible = !_columnAnnotationSelectionModeEnabled && _visibleColumnAnnotationNames.empty();

    if(columnAnnotations.empty() || !_showColumnAnnotations || invisible)
    {
        if(layoutElement != nullptr)
        {
            layoutGrid->remove(layoutElement);
            layoutGrid->simplify();
        }

        axisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msTop|QCP::msBottom);
        axisRect->setMargins({});
        axisRect->setMarginGroup(QCP::msLeft|QCP::msRight, nullptr);
        xAxis->setTickLabels(true);

        return xAxis;
    }

    QCPAxisRect* columnAnnotationsAxisRect = nullptr;

    if(layoutElement == nullptr)
    {
        columnAnnotationsAxisRect = new QCPAxisRect(&_customPlot);
        layoutGrid->addElement(1, layoutColumn, columnAnnotationsAxisRect);
    }
    else
    {
        columnAnnotationsAxisRect = dynamic_cast<QCPAxisRect*>(layoutElement);
        Q_ASSERT(columnAnnotationsAxisRect != nullptr);
    }

    const auto separation = 8;
    axisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msTop);
    axisRect->setMargins(QMargins(0, 0, 0, separation));
    columnAnnotationsAxisRect->setAutoMargins(QCP::msLeft|QCP::msRight|QCP::msBottom);
    columnAnnotationsAxisRect->setMargins({0, 0, separation, 0});

    // Align the left and right hand sides of the axes
    auto* group = new QCPMarginGroup(&_customPlot);
    axisRect->setMarginGroup(QCP::msLeft|QCP::msRight, group);
    columnAnnotationsAxisRect->setMarginGroup(QCP::msLeft|QCP::msRight, group);

    xAxis->setTickLabels(false);

    auto* caXAxis = columnAnnotationsAxisRect->axis(QCPAxis::atBottom);
    auto* caYAxis = columnAnnotationsAxisRect->axis(QCPAxis::atLeft);

    auto h = static_cast<int>(columnAnnotaionsHeight(_columnAnnotationSelectionModeEnabled));
    columnAnnotationsAxisRect->setMinimumSize(0, h);
    columnAnnotationsAxisRect->setMaximumSize(QWIDGETSIZE_MAX, h);

    size_t numColumnAnnotations = numVisibleColumnAnnotations();

    auto forEachColumnAnnotation = [this, numColumnAnnotations, &columnAnnotations](auto&& fn)
    {
        size_t y = numColumnAnnotations - 1;
        size_t offset = 0;

        for(const auto& columnAnnotation : columnAnnotations)
        {
            auto selected = u::contains(_visibleColumnAnnotationNames, columnAnnotation.name());
            bool visible = selected || _columnAnnotationSelectionModeEnabled;

            if(visible)
            {
                fn(columnAnnotation, selected, y, offset);
                y--;
            }

            offset++;
        }
    };

    // This gets removed (and thus delete'd) for every call to rebuildPlot
    auto* qcpColumnAnnotations = new QCPColumnAnnotations(caXAxis, caYAxis);

    std::vector<size_t> indices;

    if(_groupByAnnotation)
    {
        indices.reserve(_annotationGroupMap.size());
        for(const auto& columnMap : _annotationGroupMap)
            indices.push_back(columnMap.front());
    }
    else
        indices = _sortMap;

    forEachColumnAnnotation([&indices, qcpColumnAnnotations](const ColumnAnnotation& columnAnnotation,
        bool selected, size_t y, size_t offset)
    {
        qcpColumnAnnotations->setData(y, indices, selected, offset, &columnAnnotation);
    });

    qcpColumnAnnotations->resolveRects();

    if(_groupByAnnotation && !_visibleColumnAnnotationNames.empty())
        populateIQRAnnotationPlot(qcpColumnAnnotations);

    // We only want the ticker on the left most column annotation QCPAxisRect
    if(layoutColumn == 0)
    {
        // NOLINTNEXTLINE clang-analyzer-cplusplus.NewDeleteLeaks
        QSharedPointer<QCPAxisTickerText> columnAnnotationTicker(new QCPAxisTickerText);

        forEachColumnAnnotation([this, columnAnnotationTicker](const ColumnAnnotation& columnAnnotation,
            bool selected, size_t y, size_t)
        {
            QString prefix;
            QString postfix;

            if(!_columnSortOrders.empty())
            {
                const auto& columnSortOrder = _columnSortOrders.first();
                auto type = NORMALISE_QML_ENUM(PlotColumnSortType, columnSortOrder[QStringLiteral("type")].toInt());
                auto text = columnSortOrder[QStringLiteral("text")].toString();
                auto order = static_cast<Qt::SortOrder>(columnSortOrder[QStringLiteral("order")].toInt());

                if(type == PlotColumnSortType::ColumnAnnotation && text == columnAnnotation.name())
                {
                    prefix += order == Qt::AscendingOrder ?
                        QStringLiteral(u"▻ ") : QStringLiteral(u"◅ ");
                }
            }

            if(_columnAnnotationSelectionModeEnabled)
                postfix += selected ? QStringLiteral(u" ☑") : QStringLiteral(u" ☐");

            double tickPosition = static_cast<double>(y) + 0.5;
            columnAnnotationTicker->addTick(tickPosition, prefix + columnAnnotation.name() + postfix);
        });

        caYAxis->setTicker(columnAnnotationTicker);
    }
    else
        caYAxis->setTicker(nullptr);

    caYAxis->setTickPen(QPen(Qt::transparent)); // NOLINT clang-analyzer-cplusplus.NewDeleteLeaks
    caYAxis->setRange(0.0, static_cast<double>(numColumnAnnotations));

    caXAxis->setTickPen(QPen(Qt::transparent));
    caXAxis->setBasePen(QPen(Qt::transparent));
    caYAxis->setBasePen(QPen(Qt::transparent));

    caXAxis->grid()->setVisible(false);
    caYAxis->grid()->setVisible(false);

    xAxis = caXAxis;

    return xAxis;
}

void CorrelationPlotItem::populateIQRAnnotationPlot(const QCPColumnAnnotations* qcpColumnAnnotations)
{
    const auto* columnAnnotation = _pluginInstance->columnAnnotationByName(_colorGroupByAnnotationName);

    for(size_t column = 0; column < _annotationGroupMap.size(); column++)
    {
        QVector<double> values;

        for(auto row : std::as_const(_selectedRows))
        {
            for(size_t groupedColumn : _annotationGroupMap.at(column))
                values.append(_pluginInstance->continuousDataAt(row, static_cast<int>(groupedColumn)));
        }

        if(_scaleType == static_cast<int>(PlotScaleType::Log))
            logScale(values);

        QColor color;

        if(columnAnnotation != nullptr)
        {
            const auto* rect = qcpColumnAnnotations->rectAt(column, *columnAnnotation);
            color = rect->_color;
        }

        addIQRBoxPlotTo(_continuousXAxis, _continuousYAxis, column, std::move(values), color);
    }

    setContinousYAxisRangeForSelection();
}

QStringList CorrelationPlotItem::visibleColumnAnnotationNames() const
{
    QStringList list;
    list.reserve(static_cast<int>(_visibleColumnAnnotationNames.size()));

    for(const auto& columnAnnotaionName : _visibleColumnAnnotationNames)
        list.append(columnAnnotaionName);

    return list;
}

void CorrelationPlotItem::setVisibleColumnAnnotationNames(const QStringList& columnAnnotations)
{
    std::set<QString> newVisibleColumnAnnotationNames(columnAnnotations.begin(), columnAnnotations.end());

    if(_visibleColumnAnnotationNames != newVisibleColumnAnnotationNames)
    {
        _visibleColumnAnnotationNames = newVisibleColumnAnnotationNames;
        emit visibleColumnAnnotationNamesChanged();
    }
}

bool CorrelationPlotItem::columnAnnotationSelectionModeEnabled() const
{
    return _columnAnnotationSelectionModeEnabled;
}

void CorrelationPlotItem::setColumnAnnotationSelectionModeEnabled(bool enabled)
{
    // Don't set it if we can't enter selection mode
    if(enabled && !canShowColumnAnnotationSelection())
        return;

    if(_columnAnnotationSelectionModeEnabled != enabled)
    {
        _columnAnnotationSelectionModeEnabled = enabled;
        emit columnAnnotationSelectionModeEnabledChanged();

        // Disable group by annotation, if no annotations are visible
        if(!enabled && _groupByAnnotation && _visibleColumnAnnotationNames.empty())
        {
            _groupByAnnotation = false;
            emit plotOptionsChanged();
        }

        rebuildPlot();
    }
}

size_t CorrelationPlotItem::numVisibleColumnAnnotations() const
{
    if(_columnAnnotationSelectionModeEnabled)
        return _pluginInstance->columnAnnotations().size();

    return _visibleColumnAnnotationNames.size();
}

QString CorrelationPlotItem::columnAnnotationValueAt(size_t x, size_t y) const
{
    const auto& columnAnnotations = _pluginInstance->columnAnnotations();

    struct RowIndex { size_t _index; bool _enabled; };
    std::vector<RowIndex> visibleRowIndices;
    visibleRowIndices.reserve(columnAnnotations.size());

    size_t index = 0;
    for(const auto& columnAnnotation : columnAnnotations)
    {
        auto enabled = u::contains(_visibleColumnAnnotationNames, columnAnnotation.name());
        if(_columnAnnotationSelectionModeEnabled || enabled)
            visibleRowIndices.push_back({index, enabled});

        index++;
    }

    auto rowIndex = visibleRowIndices.at(y);
    const auto& columnAnnotation = columnAnnotations.at(rowIndex._index);

    if(_groupByAnnotation && !rowIndex._enabled)
    {
        std::set<QString> uniqueValues;

        for(auto column : _annotationGroupMap.at(x))
            uniqueValues.emplace(columnAnnotation.valueAt(static_cast<int>(column)));

        if(uniqueValues.size() == 1)
            return *uniqueValues.begin();

        return tr("%1 unique values").arg(uniqueValues.size());
    }

    auto column = _groupByAnnotation ? _annotationGroupMap.at(x).front() : _sortMap.at(x);
    return columnAnnotation.valueAt(static_cast<int>(column));
}

bool CorrelationPlotItem::axisRectIsColumnAnnotations(const QCPAxisRect* axisRect)
{
    int numPlottables = axisRect->plottables().size();

    if(numPlottables == 0)
        return false;

    int numColumnAnnotations = 0;

    const auto& plottables = axisRect->plottables();
    for(const auto* plottable : plottables)
    {
        const auto* columnAnnotations =
            dynamic_cast<const QCPColumnAnnotations*>(plottable);

        if(columnAnnotations != nullptr)
            numColumnAnnotations++;
    }

    return numPlottables == numColumnAnnotations;
}

bool CorrelationPlotItem::columnAnnotationTooltip(const QCPAxisRect* axisRect)
{
    if(!axisRectIsColumnAnnotations(axisRect))
        return false;

    if(!axisRect->rect().contains(_hoverPoint.toPoint()))
        return false;

    auto rectPoint = _hoverPoint - axisRect->topLeft();

    auto* bottomAxis = axisRect->axis(QCPAxis::atBottom);
    const auto& bottomRange = bottomAxis->range();
    auto bottomSize = bottomRange.size();
    auto xf = bottomRange.lower + 0.5 +
        (static_cast<double>(rectPoint.x() * bottomSize) / axisRect->width());

    auto x = static_cast<int>(xf);
    int y = static_cast<int>((rectPoint.y() * static_cast<double>(numVisibleColumnAnnotations())) /
        static_cast<double>(axisRect->height()));

    auto text = columnAnnotationValueAt(x, y);
    if(text.isEmpty())
        return false;

    _itemTracer->position->setPixelPosition(_hoverPoint);
    _hoverLabel->setText(text);

    return true;
}

void CorrelationPlotItem::onLeftClickColumnAnnotation(const QCPAxisRect* axisRect, const QPoint& pos)
{
    const auto& columnAnnotations = _pluginInstance->columnAnnotations();
    auto index = (pos.y() * numVisibleColumnAnnotations()) / (axisRect->height());
    if(index >= columnAnnotations.size())
        return;

    std::vector<QString> annotationNames;
    std::transform(columnAnnotations.begin(), columnAnnotations.end(),
        std::back_inserter(annotationNames), [](const auto& v) { return v.name(); });

    if(!_columnAnnotationSelectionModeEnabled)
    {
        // Remove any annotations not currently visible, so
        // that looking up by index works
        annotationNames.erase(std::remove_if(annotationNames.begin(), annotationNames.end(),
        [this](const auto& v)
        {
            return !u::contains(_visibleColumnAnnotationNames, v);
        }), annotationNames.end());
    }

    const auto& name = annotationNames.at(index);

    if(_columnAnnotationSelectionModeEnabled && pos.x() < 0)
    {
        // Click is on the annotation name itself (with checkbox)
        if(u::contains(_visibleColumnAnnotationNames, name))
            _visibleColumnAnnotationNames.erase(name);
        else
            _visibleColumnAnnotationNames.insert(name);

        emit visibleColumnAnnotationNamesChanged();
    }
    else if(_columnAnnotationSelectionModeEnabled &&
        !u::contains(_visibleColumnAnnotationNames, name))
    {
        // Clicking anywhere else enables a column annotation
        // when it's disabled...
        _visibleColumnAnnotationNames.insert(name);

        emit visibleColumnAnnotationNamesChanged();
    }
    else
    {
        // ...or selects it as the sort annotation otherwise
        sortBy(static_cast<int>(PlotColumnSortType::ColumnAnnotation), name);
        return;
    }

    emit plotOptionsChanged(); // NOLINT
    rebuildPlot();
}
