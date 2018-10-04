#ifndef GRAPHSIZEESTIMATEPLOTITEM_H
#define GRAPHSIZEESTIMATEPLOTITEM_H

#include "thirdparty/qcustomplot/qcustomplot_disable_warnings.h"
#include "thirdparty/qcustomplot/qcustomplot.h"
#include "thirdparty/qcustomplot/qcustomplot_enable_warnings.h"

#include <QObject>
#include <QQuickPaintedItem>
#include <QVector>

class GraphSizeEstimatePlotItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap graphSizeEstimate WRITE setGraphSizeEstimate)
    Q_PROPERTY(double threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)

public:
    explicit GraphSizeEstimatePlotItem(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

private:
    QCustomPlot _customPlot;
    QCPItemStraightLine* _thresholdIndicator = nullptr;

    QVector<double> _keys;
    QVector<double> _numNodes;
    QVector<double> _numEdges;

    double _threshold = 0.0;
    bool _dragging = false;

    double threshold() const;
    void setThreshold(double threshold);

    void setGraphSizeEstimate(const QVariantMap& graphSizeEstimate);

    void updateThresholdIndicator();
    void buildPlot();
    void updatePlotSize();

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void routeMouseEvent(QMouseEvent* event);

private slots:
    void onReplot();

signals:
    void thresholdChanged();
};

#endif // GRAPHSIZEESTIMATEPLOTITEM_H
