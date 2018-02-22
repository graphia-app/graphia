#include "enrichmentheatmapitem.h"
#include <set>

EnrichmentHeatmapItem::EnrichmentHeatmapItem(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    setRenderTarget(RenderTarget::FramebufferObject);

    _customPlot.setOpenGl(true);

    _colorMap = new QCPColorMap(_customPlot.xAxis, _customPlot.yAxis);
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

    connect(this, &EnrichmentHeatmapItem::tableModelChanged, this, &EnrichmentHeatmapItem::update);
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

void EnrichmentHeatmapItem::update()
{
    QSharedPointer<QCPAxisTickerText> xCategoryTicker(new QCPAxisTickerText);
    QSharedPointer<QCPAxisTickerText> yCategoryTicker(new QCPAxisTickerText);

    _customPlot.xAxis->setTicker(xCategoryTicker);
    _customPlot.xAxis->setTickLabelRotation(90);
    _customPlot.yAxis->setTicker(yCategoryTicker);

    std::set<QString> attributeValueSetA;
    std::set<QString> attributeValueSetB;
    for (int i = 0; i < _tableModel->rowCount(); ++i)
    {
        attributeValueSetA.insert(_tableModel->data(i, "Attribute Group").toString());
        attributeValueSetB.insert(_tableModel->data(i, "Selection").toString());
    }
    int pos = 0;
    for(auto& value: attributeValueSetA)
    {
        xCategoryTicker->addTick(pos++, value);
    }
    pos = 0;
    for(auto& value: attributeValueSetB)
    {
        yCategoryTicker->addTick(pos++, value);
    }

    _colorMap->data()->setSize(attributeValueSetA.size(), attributeValueSetB.size());
    _colorMap->data()->setRange(QCPRange(0, attributeValueSetA.size()-1), QCPRange(0, attributeValueSetB.size()-1));
    _customPlot.xAxis->setRange(-0.5, attributeValueSetA.size()-0.5);
    _customPlot.yAxis->setRange(-0.5, attributeValueSetB.size()-0.5);

    for(int i=0; i<_tableModel->rowCount(); i++)
    {
        _colorMap->data()->setCell(xCategoryTicker->ticks().key(_tableModel->data(i, "Attribute Group").toString()) + 0.5,
                                   yCategoryTicker->ticks().key(_tableModel->data(i, "Selection").toString()) + 0.5,
                                   _tableModel->data(i, "Fishers").toFloat());
    }
    _colorMap->rescaleDataRange(true);
}
