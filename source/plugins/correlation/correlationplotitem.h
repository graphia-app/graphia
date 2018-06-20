#ifndef CORRELATIONPLOTITEM_H
#define CORRELATIONPLOTITEM_H

#include "shared/utils/qmlenum.h"

#include "thirdparty/qcustomplot/qcustomplot_disable_warnings.h"
#include "thirdparty/qcustomplot/qcustomplot.h"
#include "thirdparty/qcustomplot/qcustomplot_enable_warnings.h"

#include <QQuickPaintedItem>
#include <QVector>
#include <QStringList>
#include <QThread>
#include <QTimer>

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

struct CorrelationPlotConfig
{
    size_t _columnCount = 0;
    size_t _rowCount = 0;

    QString _xAxisLabel;
    QString _yAxisLabel;

    int _elideLabelWidth = 120;
    QStringList _labelNames;
    QStringList _graphNames;

    const QVector<double>* _data = nullptr;
    QVector<int> _selectedRows;
    QVector<QColor> _rowColors;

    bool _showColumnNames = true;
    bool _showGridLines = true;
    bool _showLegend = false;

    double _scrollAmount = 0.0;

    PlotScaleType _plotScaleType = PlotScaleType::Raw;
    PlotAveragingType _plotAveragingType = PlotAveragingType::Individual;
    PlotDispersionType _plotDispersionType = PlotDispersionType::None;
    PlotDispersionVisualType _plotDispersionVisualType = PlotDispersionVisualType::Bars;

    bool operator!=(const CorrelationPlotConfig& other)
    {
        return
            _columnCount != other._columnCount ||
            _rowCount != other._rowCount ||
            _xAxisLabel != other._xAxisLabel ||
            _yAxisLabel != other._yAxisLabel ||
            _elideLabelWidth != other._elideLabelWidth ||
            _labelNames != other._labelNames ||
            _graphNames != other._graphNames ||
            _data != other._data ||
            _selectedRows != other._selectedRows ||
            _rowColors != other._rowColors ||
            _showColumnNames != other._showColumnNames ||
            _showGridLines != other._showGridLines ||
            _showLegend != other._showLegend ||
            _scrollAmount != other._scrollAmount ||
            _plotScaleType != other._plotScaleType ||
            _plotAveragingType != other._plotAveragingType ||
            _plotDispersionType != other._plotDispersionType ||
            _plotDispersionVisualType != other._plotDispersionVisualType;
    }
};

class CorrelationPlotWorker : public QObject
{
    Q_OBJECT

public:
    CorrelationPlotWorker();
    ~CorrelationPlotWorker() override;

    QPixmap pixmap() const;

    void queueUpdate();
    bool busy() const { return _numUpdatesQueued >= 1; }

    void routeMouseEvent(const QMouseEvent* event);
    void routeWheelEvent(const QWheelEvent* event);

    Q_INVOKABLE void savePlotImage(const QUrl& url, const QStringList& extensions);

private:
    QCustomPlot* _customPlot = nullptr;
    QPixmap _pixmap;
    mutable std::mutex _mutex;
    std::atomic<size_t> _numUpdatesQueued;

    QFont _defaultFont9Pt;

    CorrelationPlotConfig _config;
    bool _lastBuildIncomplete = true;
    int _width = -1;
    int _height = -1;

    const double LEGEND_WIDTH_FRACTION = 0.15;

    struct Tooltips
    {
        void initialise(QCustomPlot* plot, QCPLayer* layer);

        QCPAbstractPlottable* _hoverPlottable = nullptr;
        QPointF _hoverPoint;
        QCPItemText* _hoverLabel = nullptr;
        QCPItemRect* _hoverColorRect = nullptr;
        QCPItemTracer* _itemTracer = nullptr;
    };

    Tooltips _tooltips;

    QCPLayer* tooltipLayer();
    void showTooltip();
    void hideTooltip();

    double visibleHorizontalFraction() const;
    double columnLabelWidth() const;
    double columnAxisWidth() const;

    void scaleXAxis();

    void populateMeanLinePlot();
    void populateMedianLinePlot();
    void populateMeanHistogramPlot();
    void populateIQRPlot();
    void populateStdDevPlot();
    void populateStdErrorPlot();
    void plotDispersion(QVector<double> stdDevs, const QString& name);
    void populateLinePlot();

    void configureLegend();

    void buildPlot();

    void updateCompleted();
    bool otherUpdatesQueued() const { return _numUpdatesQueued > 1; }

private slots:
    void onReplot();

public slots:
    void update(CorrelationPlotConfig config, int width, int height);

    void onHoverMouseMove(const QPointF& position);
    void onHoverMouseLeave();

signals:
    void visibleHorizontalFractionChanged(double visibleHorizontalFraction);
    void updated();
    void busyChanged();
};

class CorrelationPlotItem : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QVector<double> rawData READ data WRITE setData NOTIFY configChanged)
    Q_PROPERTY(double visibleHorizontalFraction MEMBER _visibleHorizontalFraction NOTIFY visibleHorizontalFractionChanged)
    Q_PROPERTY(bool busy MEMBER _busy NOTIFY busyChanged)

    Q_PROPERTY(double scrollAmount READ scrollAmount WRITE setScrollAmount NOTIFY scrollAmountChanged)
    Q_PROPERTY(QVector<int> selectedRows READ selectedRows WRITE setSelectedRows NOTIFY configChanged)
    Q_PROPERTY(QVector<QColor> rowColors READ rowColors WRITE setRowColors NOTIFY configChanged)
    Q_PROPERTY(QStringList columnNames READ labelNames WRITE setLabelNames NOTIFY configChanged)
    Q_PROPERTY(QStringList rowNames READ graphNames WRITE setGraphNames NOTIFY configChanged)
    Q_PROPERTY(size_t columnCount READ columnCount WRITE setColumnCount NOTIFY configChanged)
    Q_PROPERTY(size_t rowCount READ rowCount WRITE setRowCount NOTIFY configChanged)
    Q_PROPERTY(int elideLabelWidth READ elideLabelWidth WRITE setElideLabelWidth NOTIFY configChanged)
    Q_PROPERTY(bool showColumnNames READ showColumnNames WRITE setShowColumnNames NOTIFY configChanged)
    Q_PROPERTY(bool showGridLines READ showGridLines WRITE setShowGridLines NOTIFY configChanged)
    Q_PROPERTY(bool showLegend READ showLegend WRITE setShowLegend NOTIFY configChanged)
    Q_PROPERTY(int plotScaleType READ plotScaleType WRITE setPlotScaleType NOTIFY configChanged)
    Q_PROPERTY(int plotAveragingType READ plotAveragingType WRITE setPlotAveragingType NOTIFY configChanged)
    Q_PROPERTY(int plotDispersionType READ plotDispersionType WRITE setPlotDispersionType NOTIFY configChanged)
    Q_PROPERTY(int plotDispersionVisualType READ plotDispersionVisualType WRITE setPlotDispersionVisualType NOTIFY configChanged)
    Q_PROPERTY(QString xAxisLabel READ xAxisLabel WRITE setXAxisLabel NOTIFY configChanged)
    Q_PROPERTY(QString yAxisLabel READ yAxisLabel WRITE setYAxisLabel NOTIFY configChanged)

public:
    explicit CorrelationPlotItem(QQuickItem* parent = nullptr);
    ~CorrelationPlotItem() override;

    void paint(QPainter* painter) override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void buildPlot();

private:
    QThread _plotBuildThread;
    CorrelationPlotWorker* _worker = nullptr;

    QPointF _hoverPosition;
    QTimer _tooltipTimer;

    QVector<double> _data;
    CorrelationPlotConfig _config;

    double _visibleHorizontalFraction = 0.0;
    bool _busy = false;
    QPixmap _lastRenderedPixmap;

public:
    Q_INVOKABLE void savePlotImage(const QUrl& url, const QStringList& extensions);

private:
    void setData(const QVector<double>& data);
    void setSelectedRows(const QVector<int>& selectedRows);
    void setRowColors(const QVector<QColor>& rowColors);
    void setLabelNames(const QStringList& labelNames);
    void setGraphNames(const QStringList& graphNames);
    void setElideLabelWidth(int elideLabelWidth);
    void setColumnCount(size_t columnCount);
    void setRowCount(size_t rowCount);
    void setShowColumnNames(bool showColumnNames);
    void setShowGridLines(bool showGridLines);
    void setShowLegend(bool showLegend);
    void setScrollAmount(double scrollAmount);
    void setPlotScaleType(int plotScaleType);
    void setPlotDispersionType(int plotDispersionType);
    void setXAxisLabel(const QString& plotXAxisLabel);
    void setYAxisLabel(const QString& plotYAxisLabel);
    void setPlotAveragingType(int plotAveragingType);
    void setPlotDispersionVisualType(int plotDispersionVisualType);

    QVector<double> data() const { return *_config._data; }
    QVector<int> selectedRows() const { return _config._selectedRows; }
    QVector<QColor> rowColors() const { return _config._rowColors; }
    QStringList labelNames() const { return _config._labelNames; }
    QStringList graphNames() const { return _config._graphNames; }
    int elideLabelWidth() const { return _config._elideLabelWidth; }
    size_t columnCount() const { return _config._columnCount; }
    size_t rowCount() const { return _config._rowCount; }
    bool showColumnNames() const { return _config._showColumnNames; }
    bool showGridLines() const { return _config._showGridLines; }
    bool showLegend() const { return _config._showLegend; }
    double scrollAmount() const { return _config._scrollAmount; }
    int plotScaleType() const { return static_cast<int>(_config._plotScaleType); }
    int plotDispersionType() const { return static_cast<int>(_config._plotDispersionType); }
    int plotAveragingType() const { return static_cast<int>(_config._plotAveragingType); }
    int plotDispersionVisualType() const { return static_cast<int>(_config._plotDispersionVisualType); }
    QString xAxisLabel() const { return _config._xAxisLabel; }
    QString yAxisLabel() const { return _config._yAxisLabel; }

private slots:
    void onVisibleHorizontalFractionChanged(double visibleHorizontalFraction);
    void onPlotUpdated();
    void updatePlotSize();

signals:
    void rightClick();
    void visibleHorizontalFractionChanged();
    void scrollAmountChanged();

    void configChanged(CorrelationPlotConfig config, int width = -1, int height = -1);

    void hoverMouseHover(const QPointF& position);
    void hoverMouseLeave();

    void busyChanged();
};
#endif // CORRELATIONPLOTITEM_H
