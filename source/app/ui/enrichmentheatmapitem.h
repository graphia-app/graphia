#ifndef ENRICHMENTHEATMAPITEM_H
#define ENRICHMENTHEATMAPITEM_H

#include <QQuickPaintedItem>
#include "thirdparty/qcustomplot/qcustomplot.h"
#include "attributes/enrichmentcalculator.h"

class EnrichmentHeatmapItem : public QQuickPaintedItem
{
    Q_PROPERTY(EnrichmentTableModel* model MEMBER _tableModel NOTIFY tableModelChanged)

    Q_OBJECT
private:
    QCPColorMap* _colorMap = nullptr;
    QCustomPlot _customPlot;
    QStringList _xAttributeValues;
    QStringList _yAttributeValues;
    EnrichmentTableModel* _tableModel;
public:
    explicit EnrichmentHeatmapItem(QQuickItem* parent = nullptr);
    void setData(EnrichmentCalculator::Table table);
public:
    void paint(QPainter *painter);
    void updatePlotSize();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void routeMouseEvent(QMouseEvent *event);
signals:
    void tableModelChanged();
public slots:
    void update();
};

#endif // ENRICHMENTHEATMAPITEM_H
