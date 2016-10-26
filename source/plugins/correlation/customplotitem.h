#ifndef CUSTOMPLOTITEM_H
#define CUSTOMPLOTITEM_H

#include "thirdparty/qcustomplot.h"

#include <QtQuick>


class QCustomPlot;

class CustomPlotItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QVector<double> data MEMBER _data NOTIFY dataChanged)
    Q_PROPERTY(QVector<int> selectedRows MEMBER _selectedRows NOTIFY selectedRowsChanged)
    Q_PROPERTY(QStringList columnNames MEMBER _labelNames WRITE setLabelNames NOTIFY columnNamesChanged)
    Q_PROPERTY(QStringList rowNames MEMBER _graphNames)
    Q_PROPERTY(int columnCount MEMBER _colCount)
    Q_PROPERTY(int rowCount MEMBER _rowCount)
    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelSizePixels)

public:
    CustomPlotItem( QQuickItem* parent = 0 );

    void paint( QPainter* painter );

    Q_INVOKABLE void initCustomPlot();

    void setLabelNames(const QStringList &labelNames);

protected:
    void routeMouseEvents( QMouseEvent* event );

    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void hoverMoveEvent( QHoverEvent* event );
    virtual void hoverLeaveEvent( QHoverEvent* event );
    virtual void mouseDoubleClickEvent( QMouseEvent* event );

    void buildGraphs();

private:
    QCPAbstractPlottable* _hoverPlottable;
    QPointF _hoverPoint;
    QCPLayer* _textLayer;
    QColor _hoverColor;
    QCPItemText* _textLabel;

    QCustomPlot _customPlot;
    int _colCount;
    int _rowCount;
    int _elideLabelSizePixels = 120;
    QStringList _labelNames;
    QStringList _graphNames;
    QVector<double> _data;
    QVector<int> _selectedRows;

private slots:
    void graphClicked( QCPAbstractPlottable* plottable );
    void onCustomReplot();
    void updateCustomPlotSize();
    void showTooltip();
    void hideTooltip();
signals:
    void dataChanged();
    void selectedRowsChanged();
    void columnNamesChanged();

};
#endif // CUSTOMPLOTITEM_H
