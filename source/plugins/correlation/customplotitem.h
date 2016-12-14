#ifndef CUSTOMPLOTITEM_H
#define CUSTOMPLOTITEM_H

#include "thirdparty/qcustomplot/qcustomplot.h"

#include <QtQuick>

class CustomPlotItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QVector<double> data MEMBER _data NOTIFY dataChanged)
    Q_PROPERTY(QVector<int> selectedRows MEMBER _selectedRows NOTIFY selectedRowsChanged)
    Q_PROPERTY(QStringList columnNames MEMBER _labelNames WRITE setLabelNames NOTIFY columnNamesChanged)
    Q_PROPERTY(QStringList rowNames MEMBER _graphNames)
    Q_PROPERTY(int columnCount MEMBER _columnCount)
    Q_PROPERTY(int rowCount MEMBER _rowCount)
    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelSizePixels)

public:
    CustomPlotItem(QQuickItem* parent = nullptr);
    void paint(QPainter* painter);
    void setLabelNames(const QStringList& labelNames);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void savePlotImage(const QUrl& path, const QString& format);

protected:
    void routeMouseEvents(QMouseEvent* event);

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void hoverMoveEvent(QHoverEvent* event);
    virtual void hoverLeaveEvent(QHoverEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);

    void buildGraphs();

private:
    const int MAX_SELECTED_ROWS_BEFORE_MEAN = 1000;

    QCPLayer* _textLayer = nullptr;
    QCPAbstractPlottable* _hoverPlottable = nullptr;
    QPointF _hoverPoint;
    QCPItemText* _hoverLabel = nullptr;
    QCPItemRect* _hoverColorRect = nullptr;
    QCPItemTracer* _itemTracer = nullptr;
    QCPTextElement* _plotModeTextElement = nullptr;

    QFont _defaultFont9Pt;

    QCustomPlot _customPlot;
    int _columnCount;
    int _rowCount;
    int _elideLabelSizePixels = 120;
    QStringList _labelNames;
    QStringList _graphNames;
    QVector<double> _data;
    QVector<int> _selectedRows;

    void populateMeanAverageGraphs();
    void populateRawGraphs();

private slots:
    void onCustomReplot();
    void updateCustomPlotSize();
    void showTooltip();
    void hideTooltip();

signals:
    void dataChanged();
    void selectedRowsChanged();
    void columnNamesChanged();
    void rightClick();

};
#endif // CUSTOMPLOTITEM_H
