#include "enrichmentheatmapitem.h"
#include <set>
#include <iterator>

EnrichmentHeatmapItem::EnrichmentHeatmapItem(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    setRenderTarget(RenderTarget::FramebufferObject);

    _customPlot.setOpenGl(true);

    _colorMap = new QCPColorMap(_customPlot.xAxis, _customPlot.yAxis2);
    _customPlot.yAxis2->setVisible(true);
    _customPlot.yAxis->setVisible(false);
    _colorMap->setInterpolate(false);
    _colorMap->setGradient(QCPColorGradient::gpHot);
    _colorMap->data()->setSize(10, 10);
    // Offsets required as the cells are centered on the datapoints
    _colorMap->data()->setRange(QCPRange(0.5,9.5), QCPRange(0.5,9.5));
    for(int i=0; i<10; i++)
    {
        for(int j=0; j<10; j++)
        {
            _colorMap->data()->setCell(i,j, i+j);
        }
    }
    _colorMap->rescaleDataRange(true);
    _colorMap->setTightBoundary(false);

    _defaultFont9Pt.setPointSize(9);

    setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &EnrichmentHeatmapItem::tableModelChanged, this, &EnrichmentHeatmapItem::buildPlot);
    connect(this, &QQuickPaintedItem::widthChanged, this, &EnrichmentHeatmapItem::horizontalRangeSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &EnrichmentHeatmapItem::verticalRangeSizeChanged);
    connect(this, &QQuickPaintedItem::widthChanged, this, &EnrichmentHeatmapItem::updatePlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &EnrichmentHeatmapItem::updatePlotSize);
    connect(&_customPlot, &QCustomPlot::afterReplot, this, &EnrichmentHeatmapItem::onCustomReplot);
}

void EnrichmentHeatmapItem::paint(QPainter *painter)
{
    QPixmap picture(boundingRect().size().toSize());
    QCPPainter qcpPainter(&picture);

    _customPlot.toPainter(&qcpPainter);

    painter->drawPixmap(QPoint(), picture);
}

void EnrichmentHeatmapItem::mousePressEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void EnrichmentHeatmapItem::mouseReleaseEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void EnrichmentHeatmapItem::mouseMoveEvent(QMouseEvent* event)
{
    routeMouseEvent(event);
}

void EnrichmentHeatmapItem::routeMouseEvent(QMouseEvent* event)
{
    auto* newEvent = new QMouseEvent(event->type(), event->localPos(),
                                     event->button(), event->buttons(),
                                     event->modifiers());
    QCoreApplication::postEvent(&_customPlot, newEvent);
}

void EnrichmentHeatmapItem::buildPlot()
{
    if(_tableModel == nullptr)
        return;

    QSharedPointer<QCPAxisTickerText> xCategoryTicker(new QCPAxisTickerText);
    QSharedPointer<QCPAxisTickerText> yCategoryTicker(new QCPAxisTickerText);

    _customPlot.xAxis->setTicker(xCategoryTicker);
    _customPlot.xAxis->setTickLabelRotation(90);
    _customPlot.yAxis2->setTicker(yCategoryTicker);

    std::set<QString> attributeValueSetA;
    std::set<QString> attributeValueSetB;
    std::map<QString, int> attributeSetNameAToAxisX;
    std::map<QString, int> attributeSetNameBToAxisY;
    for (int i = 0; i < _tableModel->rowCount(); ++i)
    {
        attributeValueSetA.insert(_tableModel->data(i, "Attribute Group").toString());
        attributeValueSetB.insert(_tableModel->data(i, "Selection").toString());
    }

    // Sensible sort strings using numbers
    QCollator collator;
    collator.setNumericMode(true);
    std::vector<QString> sortAttributeValueSetA(attributeValueSetA.begin(), attributeValueSetA.end());
    std::vector<QString> sortAttributeValueSetB(attributeValueSetB.begin(), attributeValueSetB.end());
    std::sort(sortAttributeValueSetA.begin(), sortAttributeValueSetA.end(), collator);
    std::sort(sortAttributeValueSetB.begin(), sortAttributeValueSetB.end(), collator);

    QFontMetrics metrics(_defaultFont9Pt);

    int column = 0;
    for(auto& labelName: sortAttributeValueSetA)
    {
        attributeSetNameAToAxisX[labelName] = column;
        if(_elideLabelWidth > 0)
            xCategoryTicker->addTick(column++, metrics.elidedText(labelName, Qt::ElideRight, _elideLabelWidth));
        else
            xCategoryTicker->addTick(column++, labelName);
    }
    column = 0;
    for(auto& labelName: sortAttributeValueSetB)
    {
        attributeSetNameBToAxisY[labelName] = column;
        if(_elideLabelWidth > 0)
            yCategoryTicker->addTick(column++, metrics.elidedText(labelName, Qt::ElideRight, _elideLabelWidth));
        else
            yCategoryTicker->addTick(column++, labelName);
    }

    _colorMap->data()->setSize(static_cast<int>(attributeValueSetA.size()), static_cast<int>(attributeValueSetB.size()));
    _colorMap->data()->setRange(QCPRange(0, attributeValueSetA.size()-1), QCPRange(0, attributeValueSetB.size()-1));

    _attributeACount = static_cast<int>(attributeValueSetA.size());
    _attributeBCount = static_cast<int>(attributeValueSetB.size());

    for(int i=0; i<_tableModel->rowCount(); i++)
    {
        _colorMap->data()->setCell(attributeSetNameAToAxisX[_tableModel->data(i, "Attribute Group").toString()] + 0.5,
                                   attributeSetNameBToAxisY[_tableModel->data(i, "Selection").toString()] + 0.5,
                                   _tableModel->data(i, "Fishers").toFloat());
    }
    _colorMap->rescaleDataRange(true);
}

void EnrichmentHeatmapItem::updatePlotSize()
{
    _customPlot.setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));
    scaleXAxis();
    scaleYAxis();
}

double EnrichmentHeatmapItem::columnAxisWidth()
{
    const auto& margins = _customPlot.axisRect()->margins();
    const unsigned int axisWidth = margins.left() + margins.right();

    //FIXME This value is wrong when the legend is enabled
    return width() - axisWidth;
}

double EnrichmentHeatmapItem::columnAxisHeight()
{
    const auto& margins = _customPlot.axisRect()->margins();
    const unsigned int axisHeight = margins.top() + margins.bottom();

    //FIXME This value is wrong when the legend is enabled
    return height() - axisHeight;
}

void EnrichmentHeatmapItem::scaleXAxis()
{
    auto maxX = static_cast<double>(_attributeACount);
    double visiblePlotWidth = columnAxisWidth();
    double textHeight = columnLabelSize();

    double position = (_attributeACount - (visiblePlotWidth / textHeight)) * _scrollXAmount;

    if(position + (visiblePlotWidth / textHeight) <= maxX)
        _customPlot.xAxis->setRange(position - _HEATMAP_OFFSET, position + (visiblePlotWidth / textHeight) - _HEATMAP_OFFSET);
    else
        _customPlot.xAxis->setRange(-_HEATMAP_OFFSET, maxX - _HEATMAP_OFFSET);
}

void EnrichmentHeatmapItem::scaleYAxis()
{
    auto maxY = static_cast<double>(_attributeBCount);
    double visiblePlotHeight = columnAxisHeight();
    double textHeight = columnLabelSize();

    double position = (_attributeBCount - (visiblePlotHeight / textHeight)) * (1.0-_scrollYAmount);

    if((visiblePlotHeight / textHeight) <= maxY)
        _customPlot.yAxis2->setRange(position - _HEATMAP_OFFSET, position + (visiblePlotHeight / textHeight) - _HEATMAP_OFFSET);
    else
        _customPlot.yAxis2->setRange(-_HEATMAP_OFFSET, maxY - _HEATMAP_OFFSET);
}

void EnrichmentHeatmapItem::setElideLabelWidth(int elideLabelWidth)
{
    bool changed = (_elideLabelWidth != elideLabelWidth);
    _elideLabelWidth = elideLabelWidth;

    if(changed)
    {
        updatePlotSize();
        buildPlot();
        _customPlot.replot(QCustomPlot::rpQueuedReplot);
    }
}


void EnrichmentHeatmapItem::setScrollXAmount(double scrollAmount)
{
    _scrollXAmount = scrollAmount;
    scaleXAxis();
    _customPlot.replot();
}

void EnrichmentHeatmapItem::setScrollYAmount(double scrollAmount)
{
    _scrollYAmount = scrollAmount;
    scaleYAxis();
    _customPlot.replot();
    update();
}

double EnrichmentHeatmapItem::columnLabelSize()
{
    QFontMetrics metrics(_defaultFont9Pt);
    const unsigned int columnPadding = 1;
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

void EnrichmentHeatmapItem::onCustomReplot()
{
    update();
}

