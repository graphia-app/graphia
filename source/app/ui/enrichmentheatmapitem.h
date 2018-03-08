#ifndef ENRICHMENTHEATMAPITEM_H
#define ENRICHMENTHEATMAPITEM_H

#include <QQuickPaintedItem>
#include "thirdparty/qcustomplot/qcustomplot.h"
#include "attributes/enrichmentcalculator.h"

class EnrichmentHeatmapItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(EnrichmentTableModel* model MEMBER _tableModel NOTIFY tableModelChanged)
    Q_PROPERTY(double scrollXAmount MEMBER _scrollXAmount WRITE setScrollXAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(double scrollYAmount MEMBER _scrollYAmount WRITE setScrollYAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(double horizontalRangeSize READ horizontalRangeSize NOTIFY horizontalRangeSizeChanged)
    Q_PROPERTY(double verticalRangeSize READ verticalRangeSize NOTIFY verticalRangeSizeChanged)
    Q_PROPERTY(double scrollYAmount MEMBER _scrollYAmount WRITE setScrollYAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelWidth WRITE setElideLabelWidth)

private:
    QCPColorMap* _colorMap = nullptr;
    QCustomPlot _customPlot;
    QStringList _xAttributeValues;
    QStringList _yAttributeValues;
    EnrichmentTableModel* _tableModel = nullptr;
    QFont _defaultFont9Pt;
    int _attributeACount = 0;
    int _attributeBCount = 0;
    double _scrollXAmount = 0.0;
    double _scrollYAmount = 0.0;
    int _elideLabelWidth = 120;
    const double _HEATMAP_OFFSET = 0.5;

public:
    explicit EnrichmentHeatmapItem(QQuickItem* parent = nullptr);
    void setData(EnrichmentCalculator::Table table);

public:
    void paint(QPainter *painter);
    void updatePlotSize();
    double horizontalRangeSize();
    double verticalRangeSize();
    double columnAxisWidth();
    double columnLabelSize();
    void scaleXAxis();
    void scaleYAxis();
    double columnAxisHeight();
    void setScrollXAmount(double scrollAmount);
    void setScrollYAmount(double scrollAmount);
    void setElideLabelWidth(int elideLabelWidth);

    Q_INVOKABLE void buildPlot();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void routeMouseEvent(QMouseEvent *event);

signals:
    void tableModelChanged();
    void scrollAmountChanged();
    void horizontalRangeSizeChanged();
    void verticalRangeSizeChanged();

public slots:
    void onCustomReplot();
};

#endif // ENRICHMENTHEATMAPITEM_H
