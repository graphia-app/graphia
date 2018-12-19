#ifndef ENRICHMENTHEATMAPITEM_H
#define ENRICHMENTHEATMAPITEM_H

#include "attributes/enrichmentcalculator.h"

#include <qcustomplot.h>

#include <QQuickPaintedItem>

class EnrichmentHeatmapItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(EnrichmentTableModel* model MEMBER _tableModel NOTIFY tableModelChanged)
    Q_PROPERTY(double scrollXAmount MEMBER _scrollXAmount WRITE setScrollXAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(double scrollYAmount MEMBER _scrollYAmount WRITE setScrollYAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(double horizontalRangeSize READ horizontalRangeSize NOTIFY horizontalRangeSizeChanged)
    Q_PROPERTY(double verticalRangeSize READ verticalRangeSize NOTIFY verticalRangeSizeChanged)
    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelWidth WRITE setElideLabelWidth)
    Q_PROPERTY(int xAxisPadding MEMBER _xAxisPadding WRITE setXAxisPadding NOTIFY scrollAmountChanged)
    Q_PROPERTY(int yAxisPadding MEMBER _yAxisPadding WRITE setYAxisPadding NOTIFY scrollAmountChanged)
 
    Q_PROPERTY(bool showOnlyEnriched MEMBER _showOnlyEnriched WRITE setShowOnlyEnriched)

private:
    QCPLayer* _textLayer = nullptr;
    QCPColorMap* _colorMap = nullptr;
    QCPColorScale* _colorScale = nullptr;
    QCPItemText* _hoverLabel = nullptr;
    QCPAbstractPlottable* _hoverPlottable = nullptr;
    QPointF _hoverPoint;
    QCustomPlot _customPlot;
    std::map<int, QString> _xAxisToFullLabel;
    std::map<int, QString> _yAxisToFullLabel;
    std::map<std::pair<int, int>, int> _colorMapKeyValueToTableIndex;

    EnrichmentTableModel* _tableModel = nullptr;
    QFont _defaultFont9Pt;
    int _attributeACount = 0;
    int _attributeBCount = 0;
    double _scrollXAmount = 0.0;
    double _scrollYAmount = 0.0;
    int _elideLabelWidth = 120;
    const double _HEATMAP_OFFSET = 0.5;
    int _xAxisPadding = 0;
    int _yAxisPadding = 0;

    bool _showOnlyEnriched = false;

public:
    explicit EnrichmentHeatmapItem(QQuickItem* parent = nullptr);
    void setData(EnrichmentTableModel::Table table);

public:
    void paint(QPainter *painter) override;
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
    void setXAxisPadding(int padding);
    void setYAxisPadding(int padding);
    void setShowOnlyEnriched(bool showOnlyEnriched);

    Q_INVOKABLE void buildPlot();
    Q_INVOKABLE void savePlotImage(const QUrl &url, const QStringList &extensions);
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void routeMouseEvent(QMouseEvent *event);

signals:
    void rightClick();
    void tableModelChanged();
    void scrollAmountChanged();
    void horizontalRangeSizeChanged();
    void verticalRangeSizeChanged();
    void plotValueClicked(int row);

public slots:
    void onCustomReplot();
    void showTooltip();
    void hideTooltip();
};

#endif // ENRICHMENTHEATMAPITEM_H
