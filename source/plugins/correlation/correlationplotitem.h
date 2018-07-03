#ifndef CORRELATIONPLOTITEM_H
#define CORRELATIONPLOTITEM_H

#include "shared/utils/qmlenum.h"

#include "thirdparty/qcustomplot/qcustomplot_disable_warnings.h"
#include "thirdparty/qcustomplot/qcustomplot.h"
#include "thirdparty/qcustomplot/qcustomplot_enable_warnings.h"

#include <QQuickPaintedItem>
#include <QVector>
#include <QMap>
#include <QStringList>
#include <QElapsedTimer>
#include <QThread>
#include <QPixmap>
#include <QOffscreenSurface>

#include <mutex>
#include <atomic>

DEFINE_QML_ENUM(Q_GADGET, PlotScaleType,
                Raw,
                Log,
                MeanCentre,
                UnitVariance,
                Pareto);

DEFINE_QML_ENUM(Q_GADGET, PlotAveragingType,
                Individual,
                MeanLine,
                MedianLine,
                MeanHistogram,
                IQRPlot);

DEFINE_QML_ENUM(Q_GADGET, PlotDispersionType,
                None,
                StdErr,
                StdDev);

DEFINE_QML_ENUM(Q_GADGET, PlotDispersionVisualType,
                Bars,
                Area,
                StdDev);

enum class CorrelationPlotUpdateType
{
    None,
    Render,
    RenderAndTooltips,
    ReplotAndRenderAndTooltips,
};

class CorrelationPlotWorker : public QObject
{
    Q_OBJECT

public:
    CorrelationPlotWorker(std::recursive_mutex& mutex,
        QCustomPlot& customPlot, QCPLayer& tooltipLayer);

    bool busy() const;

    Q_INVOKABLE void setWidth(int width);
    Q_INVOKABLE void setHeight(int height);
    Q_INVOKABLE void setXAxisRange(double min, double max);
    Q_INVOKABLE void updatePixmap(CorrelationPlotUpdateType updateType);

private:
    bool _debug = false;
    QElapsedTimer _replotTimer;

    mutable std::recursive_mutex* _mutex;
    std::atomic_bool _busy;

    QCustomPlot* _customPlot = nullptr;
    QOffscreenSurface* _surface = nullptr;
    QCPLayer* _tooltipLayer = nullptr;

    int _width = -1;
    int _height = -1;
    double _xAxisMin = 0.0;
    double _xAxisMax = 0.0;

    bool _updateQueued = false;
    CorrelationPlotUpdateType _updateType = CorrelationPlotUpdateType::None;

    Q_INVOKABLE void renderPixmap();

signals:
    void busyChanged();
    void pixmapUpdated(QPixmap pixmap);
};

class CorrelationPlotItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QVector<double> rawData MEMBER _data)
    Q_PROPERTY(double horizontalScrollPosition MEMBER _horizontalScrollPosition WRITE setHorizontalScrollPosition NOTIFY horizontalScrollPositionChanged)
    Q_PROPERTY(double visibleHorizontalFraction READ visibleHorizontalFraction NOTIFY visibleHorizontalFractionChanged)
    Q_PROPERTY(QVector<int> selectedRows MEMBER _selectedRows WRITE setSelectedRows)
    Q_PROPERTY(QVector<QColor> rowColors MEMBER _rowColors WRITE setRowColors)
    Q_PROPERTY(QStringList columnNames MEMBER _labelNames WRITE setLabelNames)
    Q_PROPERTY(QStringList rowNames MEMBER _graphNames)
    Q_PROPERTY(size_t columnCount MEMBER _columnCount WRITE setColumnCount)
    Q_PROPERTY(size_t rowCount MEMBER _rowCount)
    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelWidth WRITE setElideLabelWidth)
    Q_PROPERTY(bool showColumnNames MEMBER _showColumnNames WRITE setShowColumnNames)
    Q_PROPERTY(bool showGridLines MEMBER _showGridLines WRITE setShowGridLines NOTIFY plotOptionsChanged)
    Q_PROPERTY(bool showLegend MEMBER _showLegend WRITE setShowLegend NOTIFY plotOptionsChanged)
    Q_PROPERTY(int plotScaleType MEMBER _plotScaleType WRITE setPlotScaleType NOTIFY plotOptionsChanged)
    Q_PROPERTY(int plotAveragingType MEMBER _plotAveragingType WRITE setPlotAveragingType NOTIFY plotOptionsChanged)
    Q_PROPERTY(int plotDispersionType MEMBER _plotDispersionType WRITE setPlotDispersionType NOTIFY plotOptionsChanged)
    Q_PROPERTY(int plotDispersionVisualType MEMBER _plotDispersionVisualType WRITE setPlotDispersionVisualType NOTIFY plotOptionsChanged)
    Q_PROPERTY(QString xAxisLabel MEMBER _xAxisLabel WRITE setXAxisLabel NOTIFY plotOptionsChanged)
    Q_PROPERTY(QString yAxisLabel MEMBER _yAxisLabel WRITE setYAxisLabel NOTIFY plotOptionsChanged)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit CorrelationPlotItem(QQuickItem* parent = nullptr);
    ~CorrelationPlotItem() override;

    void paint(QPainter* painter) override;

    Q_INVOKABLE void savePlotImage(const QUrl& url, const QStringList& extensions);

    void setPlotScaleType(int plotScaleType);
    void setPlotDispersionType(int plotDispersionType);
    void setXAxisLabel(const QString& plotXAxisLabel);
    void setYAxisLabel(const QString& plotYAxisLabel);
    void setPlotAveragingType(int plotAveragingType);
    void setPlotDispersionVisualType(int plotDispersionVisualType);

protected:
    void routeMouseEvent(QMouseEvent* event);
    void routeWheelEvent(QWheelEvent* event);

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void rebuildPlot();

private:
    bool _debug = false;

    bool _tooltipNeedsUpdate = false;
    QCPLayer* _tooltipLayer = nullptr;
    QPointF _hoverPoint{-1.0, -1.0};
    QCPItemText* _hoverLabel = nullptr;
    QCPItemRect* _hoverColorRect = nullptr;
    QCPItemTracer* _itemTracer = nullptr;
    QFont _defaultFont9Pt;

    QCustomPlot _customPlot;
    size_t _columnCount = 0;
    size_t _rowCount = 0;
    int _elideLabelWidth = 120;
    QStringList _labelNames;
    QStringList _graphNames;
    QVector<double> _data;
    QVector<int> _selectedRows;
    QVector<QColor> _rowColors;
    bool _showColumnNames = true;
    bool _showGridLines = true;
    bool _showLegend = false;
    int _plotScaleType = static_cast<int>(PlotScaleType::Raw);
    int _plotAveragingType = static_cast<int>(PlotAveragingType::Individual);
    int _plotDispersionType = static_cast<int>(PlotDispersionType::None);
    int _plotDispersionVisualType = static_cast<int>(PlotDispersionVisualType::Bars);
    double _horizontalScrollPosition = 0.0;
    QString _xAxisLabel;
    QString _yAxisLabel;

    QCPLayer* _lineGraphLayer = nullptr;

    struct CacheEntry
    {
        QCPGraph* _graph = nullptr;
        double _minY = std::numeric_limits<double>::max();
        double _maxY = std::numeric_limits<double>::min();
    };

    QMap<int, CacheEntry> _lineGraphCache;

    std::recursive_mutex _mutex;
    QThread _plotRenderThread;
    CorrelationPlotWorker* _worker = nullptr;

    QPixmap _pixmap;

    void populateMeanLinePlot();
    void populateMedianLinePlot();
    void populateLinePlot();
    void populateMeanHistogramPlot();
    void populateIQRPlot();
    void populateStdDevPlot();
    void populateStdErrorPlot();
    void plotDispersion(QVector<double> stdDevs, const QString& name);

    bool busy() const { return _worker != nullptr ? _worker->busy() : false; }

private:
    void setSelectedRows(const QVector<int>& selectedRows);
    void setRowColors(const QVector<QColor>& rowColors);
    void setLabelNames(const QStringList& labelNames);
    void setElideLabelWidth(int elideLabelWidth);
    void setColumnCount(size_t columnCount);
    void setShowColumnNames(bool showColumnNames);
    void setShowGridLines(bool showGridLines);
    void setShowLegend(bool showLegend);
    void setHorizontalScrollPosition(double horizontalScrollPosition);

    void computeXAxisRange();
    QVector<double> meanAverageData(double& min, double& max);

    double visibleHorizontalFraction();
    double columnLabelWidth();
    double columnAxisWidth();

    void configureLegend();

    void invalidateLineGraphCache();

    void updatePixmap(CorrelationPlotUpdateType updateType);

private slots:
    void onPixmapUpdated(const QPixmap& pixmap);
    void updatePlotSize();
    void updateTooltip();

signals:
    void rightClick();
    void horizontalScrollPositionChanged();
    void visibleHorizontalFractionChanged();
    void plotOptionsChanged();
    void busyChanged();
};
#endif // CORRELATIONPLOTITEM_H
