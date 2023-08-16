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

#include "correlationplotitem.h"

#include "correlationplugin.h"
#include "qcpcolumnannotations.h"

#include "shared/utils/container.h"

using namespace Qt::Literals::StringLiterals;

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
    const bool invisible = _plotMode != PlotMode::ColumnAnnotationSelection && _visibleColumnAnnotationNames.empty();

    if(columnAnnotations.empty() || invisible)
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

    auto h = static_cast<int>(columnAnnotationsHeight());
    columnAnnotationsAxisRect->setMinimumSize(0, h);
    columnAnnotationsAxisRect->setMaximumSize(QWIDGETSIZE_MAX, h);

    const size_t numColumnAnnotations = numVisibleColumnAnnotations();

    auto forEachColumnAnnotation = [this, numColumnAnnotations, &columnAnnotations](auto&& fn)
    {
        size_t y = numColumnAnnotations - 1;
        size_t offset = 0;

        for(const auto& columnAnnotation : columnAnnotations)
        {
            auto selected = u::contains(_visibleColumnAnnotationNames, columnAnnotation.name());
            const bool visible = selected || _plotMode == PlotMode::ColumnAnnotationSelection;

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
                auto type = normaliseQmlEnum<PlotColumnSortType>(columnSortOrder[u"type"_s].toInt());
                auto text = columnSortOrder[u"text"_s].toString();
                auto order = static_cast<Qt::SortOrder>(columnSortOrder[u"order"_s].toInt());

                if(type == PlotColumnSortType::ColumnAnnotation && text == columnAnnotation.name())
                {
                    prefix += order == Qt::AscendingOrder ? u"▷ "_s : u"◁ "_s;
                }
            }

            if(_plotMode == PlotMode::ColumnAnnotationSelection)
                postfix += selected ? u" ☑"_s : u" ☐"_s;

            const double tickPosition = static_cast<double>(y) + 0.5;
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

    auto minY = std::numeric_limits<double>::max();
    auto maxY = std::numeric_limits<double>::lowest();

    for(size_t column = 0; column < _annotationGroupMap.size(); column++)
    {
        QVector<double> values;

        for(auto row : std::as_const(_selectedRows))
        {
            for(const size_t groupedColumn : _annotationGroupMap.at(column))
                values.append(_pluginInstance->continuousDataAt(static_cast<size_t>(row), groupedColumn));
        }

        if(_scaleType == static_cast<int>(PlotScaleType::Log))
            logScale(values);

        QColor color;
        QString value;

        if(columnAnnotation != nullptr)
        {
            const auto* rect = qcpColumnAnnotations->rectAt(column, *columnAnnotation);
            color = rect->_color;
            value = rect->_value;
        }

        auto minmax = addIQRBoxPlotTo(_continuousXAxis, _continuousYAxis, column,
            std::move(values), _showIqrOutliers, color, value);

        minY = std::min(minY, minmax.first);
        maxY = std::max(maxY, minmax.second);
    }

    setContinousYAxisRange(minY, maxY);
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
    const std::set<QString> newVisibleColumnAnnotationNames(columnAnnotations.begin(), columnAnnotations.end());

    if(_visibleColumnAnnotationNames != newVisibleColumnAnnotationNames)
    {
        _visibleColumnAnnotationNames = newVisibleColumnAnnotationNames;
        emit visibleColumnAnnotationNamesChanged();
    }
}

void CorrelationPlotItem::showAllColumnAnnotations()
{
    for(const auto& columnAnnotation : _pluginInstance->columnAnnotations())
        _visibleColumnAnnotationNames.insert(columnAnnotation.name());

    emit visibleColumnAnnotationNamesChanged();
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::hideAllColumnAnnotations()
{
    _visibleColumnAnnotationNames.clear();

    emit visibleColumnAnnotationNamesChanged();
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::showColumnAnnotations(const QStringList& annotations)
{
    for(const auto& annotation : annotations)
    {
        Q_ASSERT(_pluginInstance->columnAnnotationNames().contains(annotation));
        _visibleColumnAnnotationNames.insert(annotation);
    }

    emit visibleColumnAnnotationNamesChanged();
    emit plotOptionsChanged();
    rebuildPlot();
}

void CorrelationPlotItem::hideColumnAnnotations(const QStringList& annotations)
{
    for(const auto& annotation : annotations)
    {
        Q_ASSERT(_visibleColumnAnnotationNames.contains(annotation));
        _visibleColumnAnnotationNames.erase(annotation);
    }

    emit visibleColumnAnnotationNamesChanged();
    emit plotOptionsChanged();
    rebuildPlot();
}

int CorrelationPlotItem::plotMode() const
{
    return static_cast<int>(_plotMode);
}

void CorrelationPlotItem::setPlotMode(int plotModeInt)
{
    auto plotMode = normaliseQmlEnum<PlotMode>(plotModeInt);

    if(_plotMode != plotMode)
    {
        _plotMode = plotMode;
        emit plotModeChanged();

        // Disable group by annotation, if no annotations are visible
        if(_plotMode != PlotMode::ColumnAnnotationSelection &&
           _groupByAnnotation && _visibleColumnAnnotationNames.empty())
        {
            _groupByAnnotation = false;
            emit plotOptionsChanged();
        }

        if(_plotMode != PlotMode::RowsOfInterestColumnSelection)
        {
            _selectedColumns.clear();
            emit selectedColumnsChanged();
        }

        rebuildPlot();
    }
}

size_t CorrelationPlotItem::numVisibleColumnAnnotations() const
{
    if(_plotMode == PlotMode::ColumnAnnotationSelection)
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
        if(_plotMode == PlotMode::ColumnAnnotationSelection || enabled)
            visibleRowIndices.push_back({index, enabled});

        index++;
    }

    if(y >= visibleRowIndices.size())
        return {};

    auto rowIndex = visibleRowIndices.at(y);
    const auto& columnAnnotation = columnAnnotations.at(rowIndex._index);

    size_t column;

    if(_groupByAnnotation)
    {
        if(x >= _annotationGroupMap.size())
            return {};

        if(!rowIndex._enabled)
        {
            std::set<QString> uniqueValues;

            for(auto groupColumn : _annotationGroupMap.at(x))
                uniqueValues.emplace(columnAnnotation.valueAt(groupColumn));

            if(uniqueValues.size() == 1)
                return *uniqueValues.begin();

            return tr("%1 unique values").arg(uniqueValues.size());
        }

        column = _annotationGroupMap.at(x).front();
    }
    else
    {
        if(x >= _sortMap.size())
            return {};

        column = _sortMap.at(x);
    }

    return columnAnnotation.valueAt(column);
}

bool CorrelationPlotItem::axisRectIsColumnAnnotations(const QCPAxisRect* axisRect)
{
    auto numPlottables = static_cast<int>(axisRect->plottables().size());

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

QPoint CorrelationPlotItem::columnAnnotationPositionForPixel(const QCPAxisRect* axisRect, const QPointF& position)
{
    auto* bottomAxis = axisRect->axis(QCPAxis::atBottom);
    const auto& bottomRange = bottomAxis->range();
    auto xf = bottomRange.lower + 0.5 +
        (static_cast<double>(position.x() * bottomRange.size()) / axisRect->width());

    auto x = static_cast<int>(xf);
    auto y = static_cast<int>((position.y() * static_cast<double>(numVisibleColumnAnnotations())) /
        static_cast<double>(axisRect->height()));

    if(y >= static_cast<int>(numVisibleColumnAnnotations()))
        y = -1;

    return {x, y};
}

bool CorrelationPlotItem::columnAnnotationTooltip(const QCPAxisRect* axisRect)
{
    if(!axisRectIsColumnAnnotations(axisRect))
        return false;

    if(!axisRect->rect().contains(_hoverPoint.toPoint()))
        return false;

    auto rectPoint = _hoverPoint - axisRect->topLeft();
    auto p = columnAnnotationPositionForPixel(axisRect, rectPoint);

    if(p.x() < 0 || p.y() < 0)
        return false;

    auto text = columnAnnotationValueAt(static_cast<size_t>(p.x()), static_cast<size_t>(p.y()));
    if(text.isEmpty())
        return false;

    _itemTracer->position->setPixelPosition(_hoverPoint);
    _hoverLabel->setText(text);

    return true;
}

void CorrelationPlotItem::onClickColumnAnnotation(const QCPAxisRect* axisRect, const QMouseEvent* event)
{
    std::vector<const ColumnAnnotation*> columnAnnotations;
    std::transform(_pluginInstance->columnAnnotations().begin(), _pluginInstance->columnAnnotations().end(),
        std::back_inserter(columnAnnotations), [](const auto& v) { return &v; });

    if(_plotMode != PlotMode::ColumnAnnotationSelection)
    {
        // Remove any annotations not currently visible, so that looking up by index works
        columnAnnotations.erase(std::remove_if(columnAnnotations.begin(), columnAnnotations.end(),
        [this](const auto* v)
        {
            return !u::contains(_visibleColumnAnnotationNames, v->name());
        }), columnAnnotations.end());
    }

    auto pos = event->pos() - axisRect->topLeft();
    bool clickOnName = pos.x() < 0;
    auto p = columnAnnotationPositionForPixel(axisRect, pos.toPointF());

    if(p.y() < 0)
        return;

    const auto& name = columnAnnotations.at(static_cast<size_t>(p.y()))->name();

    if(clickOnName && _plotMode != PlotMode::ColumnAnnotationSelection)
    {
        sortBy(static_cast<int>(PlotColumnSortType::ColumnAnnotation), name);
        return;
    }

    switch(_plotMode)
    {
    default:
    case PlotMode::Normal:
        return;

    case PlotMode::ColumnAnnotationSelection:
        if(clickOnName)
        {
            // Click is on the annotation name itself (with checkbox)
            if(u::contains(_visibleColumnAnnotationNames, name))
                hideColumnAnnotations({name});
            else
                showColumnAnnotations({name});
        }
        else if(!u::contains(_visibleColumnAnnotationNames, name))
        {
            // Clicking anywhere else enables a column annotation
            // when it's disabled...
            showColumnAnnotations({name});
        }
        else
        {
            // ...or selects it as the sort annotation otherwise
            sortBy(static_cast<int>(PlotColumnSortType::ColumnAnnotation), name);
            return;
        }
        break;

    case PlotMode::RowsOfInterestColumnSelection:
        const auto& plottable = axisRect->plottables().at(0);
        const auto* qcpColumnAnnotations = dynamic_cast<const QCPColumnAnnotations*>(plottable);
        const auto* rect = qcpColumnAnnotations->rectAt(
            static_cast<size_t>(p.x()), *columnAnnotations.at(static_cast<size_t>(p.y())));

        bool toggle = event->modifiers().testFlag(Qt::ControlModifier);

        if(!toggle)
            _selectedColumns.clear();

        std::vector<size_t> indices;

        for(size_t i = rect->_x; i < rect->_x + rect->_width; i++)
        {
            if(_groupByAnnotation)
            {
                for(auto j : _annotationGroupMap.at(i))
                    indices.push_back(j);
            }
            else
                indices.push_back(_sortMap.at(i));
        }

        bool selected = std::all_of(indices.begin(), indices.end(),
            [this](size_t index) { return _selectedColumns.contains(index); });

        for(auto index : indices)
        {
            if(toggle && selected)
                _selectedColumns.erase(index);
            else
                _selectedColumns.insert(index);
        }

        emit selectedColumnsChanged();
        break;
    }

    rebuildPlot();
}
