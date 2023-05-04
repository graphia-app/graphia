/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CORRELATIONPLOTITEM_H
#define CORRELATIONPLOTITEM_H

#include "correlationplugin.h"

#include "columnannotation.h"

#include "shared/utils/qmlenum.h"

#include <qcustomplot.h>

#include <QQuickPaintedItem>
#include <QVector>
#include <QMap>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QElapsedTimer>
#include <QThread>
#include <QPixmap>
#include <QOffscreenSurface>

#include <vector>
#include <set>
#include <map>
#include <mutex>
#include <atomic>

class QCPColumnAnnotations;

DEFINE_QML_ENUM(
    Q_GADGET, PlotScaleType,
    Raw,
    Log,
    MeanCentre,
    UnitVariance,
    Pareto,
    ByAttribute);

DEFINE_QML_ENUM(
    Q_GADGET, PlotAveragingType,
    Individual,
    MeanLine,
    MedianLine,
    MeanHistogram,
    IQR);

DEFINE_QML_ENUM(
    Q_GADGET, PlotDispersionType,
    None,
    StdErr,
    StdDev);

DEFINE_QML_ENUM(
    Q_GADGET, PlotDispersionVisualType,
    Bars,
    Area,
    StdDev);

DEFINE_QML_ENUM(
    Q_GADGET, PlotColumnSortType,
    Natural,
    ColumnName,
    ColumnAnnotation,
    DataValue,
    HierarchicalClustering);

DEFINE_QML_ENUM(
    Q_GADGET, PlotMode,
    Normal,
    ColumnAnnotationSelection);

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
        QCustomPlot& customPlot);

    bool busy() const;

    bool zoomed() const;
    void updateZoomed();

    Q_INVOKABLE void setShowGridLines(bool showGridLines);
    Q_INVOKABLE void setWidth(int width);
    Q_INVOKABLE void setHeight(int height);
    Q_INVOKABLE void setXAxisRange(double min, double max);
    Q_INVOKABLE void setAxisRange(QCPAxis* axis, double min, double max);
    Q_INVOKABLE void zoom(QCPAxis* axis, double centre, int direction);
    Q_INVOKABLE void resetZoom();
    Q_INVOKABLE void pan(QCPAxis* axis, double delta);
    Q_INVOKABLE void updatePixmap(CorrelationPlotUpdateType updateType);

    void clone(CorrelationPlotWorker& target) const;

private:
    bool _debug = false;
    QElapsedTimer _replotTimer;

    mutable std::recursive_mutex* _mutex;
    std::atomic_bool _busy;
    int _threadId = -1;

    QCustomPlot* _customPlot = nullptr;
    QOffscreenSurface* _surface = nullptr;

    int _width = -1;
    int _height = -1;
    double _xAxisMin = 0.0;
    double _xAxisMax = 0.0;
    bool _showGridLines = true;

    struct AxisParameters
    {
        double _min = 0.0;
        double _max = 0.0;

        double _zoomedMin = 0.0;
        double _zoomedMax = 0.0;

        void resetZoom()
        {
            _zoomedMin = std::numeric_limits<double>::max();
            _zoomedMax = std::numeric_limits<double>::lowest();
        }

        AxisParameters() { resetZoom(); }

        bool zoomed() const { return _zoomedMin < _zoomedMax && (_zoomedMin > _min || _zoomedMax < _max); }
    };

    std::map<QCPAxis*, AxisParameters> _axisParameters;

    std::atomic_bool _zoomed = false;

    std::atomic_bool _updateQueued = false;
    CorrelationPlotUpdateType _updateType = CorrelationPlotUpdateType::None;

    Q_INVOKABLE void renderPixmap();

signals:
    void busyChanged();
    void pixmapUpdated(QPixmap pixmap);
    void zoomedChanged();
};

class CorrelationPlotItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(CorrelationPluginInstance* model MEMBER _pluginInstance WRITE setPluginInstance)
    Q_PROPERTY(double horizontalScrollPosition MEMBER _horizontalScrollPosition
        WRITE setHorizontalScrollPosition NOTIFY horizontalScrollPositionChanged)
    Q_PROPERTY(double visibleHorizontalFraction READ visibleHorizontalFraction NOTIFY visibleHorizontalFractionChanged)
    Q_PROPERTY(QVector<int> selectedRows MEMBER _selectedRows WRITE setSelectedRows NOTIFY selectedRowsChanged)

    Q_PROPERTY(QStringList visibleColumnAnnotationNames READ visibleColumnAnnotationNames
        WRITE setVisibleColumnAnnotationNames NOTIFY visibleColumnAnnotationNamesChanged)
    Q_PROPERTY(int plotMode READ plotMode WRITE setPlotMode NOTIFY plotModeChanged)
    Q_PROPERTY(bool groupByAnnotation MEMBER _groupByAnnotation WRITE setGroupByAnnotation NOTIFY plotOptionsChanged)
    Q_PROPERTY(QString colorGroupByAnnotationName MEMBER _colorGroupByAnnotationName
        WRITE setColorGroupByAnnotationName NOTIFY plotOptionsChanged)

    Q_PROPERTY(std::vector<size_t> selectedColumns READ selectedColumns NOTIFY selectedColumnsChanged)

    Q_PROPERTY(int elideLabelWidth MEMBER _elideLabelWidth WRITE setElideLabelWidth)
    Q_PROPERTY(bool showColumnNames MEMBER _showColumnNames WRITE setShowColumnNames NOTIFY plotOptionsChanged)
    Q_PROPERTY(bool showGridLines MEMBER _showGridLines WRITE setShowGridLines NOTIFY plotOptionsChanged)
    Q_PROPERTY(bool showLegend MEMBER _showLegend WRITE setShowLegend NOTIFY plotOptionsChanged)
    Q_PROPERTY(int scaleType MEMBER _scaleType WRITE setScaleType NOTIFY plotOptionsChanged)
    Q_PROPERTY(QString scaleByAttributeName MEMBER _scaleByAttributeName
        WRITE setScaleByAttributeName NOTIFY plotOptionsChanged)
    Q_PROPERTY(int averagingType MEMBER _averagingType WRITE setAveragingType NOTIFY plotOptionsChanged)
    Q_PROPERTY(QString averagingAttributeName MEMBER _averagingAttributeName
        WRITE setAveragingAttributeName NOTIFY plotOptionsChanged)
    Q_PROPERTY(int dispersionType MEMBER _dispersionType WRITE setDispersionType NOTIFY plotOptionsChanged)
    Q_PROPERTY(int dispersionVisualType MEMBER _dispersionVisualType
        WRITE setDispersionVisualType NOTIFY plotOptionsChanged)
    Q_PROPERTY(QVector<QVariantMap> columnSortOrders MEMBER _columnSortOrders
        WRITE setColumnSortOrders NOTIFY plotOptionsChanged)
    Q_PROPERTY(QString xAxisLabel MEMBER _xAxisLabel WRITE setXAxisLabel NOTIFY plotOptionsChanged)
    Q_PROPERTY(QString yAxisLabel MEMBER _yAxisLabel WRITE setYAxisLabel NOTIFY plotOptionsChanged)
    Q_PROPERTY(int rightPadding MEMBER _rightPadding WRITE setRightPadding NOTIFY plotOptionsChanged)
    Q_PROPERTY(int bottomPadding MEMBER _bottomPadding WRITE setBottomPadding NOTIFY plotOptionsChanged)
    Q_PROPERTY(bool includeYZero MEMBER _includeYZero WRITE setIncludeYZero NOTIFY plotOptionsChanged)
    Q_PROPERTY(bool showIqrOutliers MEMBER _showIqrOutliers WRITE setShowIqrOutliers NOTIFY plotOptionsChanged)
    Q_PROPERTY(bool showAllColumns MEMBER _showAllColumns WRITE setShowAllColumns NOTIFY plotOptionsChanged)
    Q_PROPERTY(bool isWide READ isWide NOTIFY isWideChanged)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool zoomed READ zoomed NOTIFY zoomedChanged)

    Q_PROPERTY(double minimumHeight READ minimumHeight NOTIFY minimumHeightChanged)

public:
    explicit CorrelationPlotItem(QQuickItem* parent = nullptr);
    ~CorrelationPlotItem() override;

    void paint(QPainter* painter) override;

    Q_INVOKABLE void savePlotImage(const QUrl& url, const QString& extension);
    Q_INVOKABLE void savePlotImageByAttribute(const QUrl& url,
        const QString& extension, const QString& attributeName);
    Q_INVOKABLE void sortBy(int type, const QString& text = {});
    Q_INVOKABLE void resetZoom();

    Q_INVOKABLE void showAllColumnAnnotations();
    Q_INVOKABLE void hideAllColumnAnnotations();

    void setSelectedRows(const QVector<int>& selectedRows);
    void setScaleType(int scaleType);
    void setScaleByAttributeName(const QString& attributeName);
    void setDispersionType(int dispersionType);
    void setXAxisLabel(const QString& plotXAxisLabel);
    void setYAxisLabel(const QString& plotYAxisLabel);
    void setIncludeYZero(bool includeYZero);
    void setShowIqrOutliers(bool showIqrOutliers);
    void setShowAllColumns(bool showAllColumns);
    void setAveragingType(int averagingType);
    void setAveragingAttributeName(const QString& attributeName);
    void setDispersionVisualType(int dispersionVisualType);
    void setGroupByAnnotation(bool groupByAnnotation);
    void setColorGroupByAnnotationName(const QString& annotationName);

    static bool axisRectIsColumnAnnotations(const QCPAxisRect* axisRect);
    static QColor colorForRows(const CorrelationPluginInstance* pluginInstance,
        const QVector<int>& rows);

    void clone(CorrelationPlotItem& target) const;
    void savePlotImage(const QString& filename);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    enum class InvalidateCache { No, Yes };

    void rebuildPlot(InvalidateCache invalidateCache = InvalidateCache::No);

private:
    bool _debug = false;

    enum class RebuildRequired
    {
        None,
        Partial,
        Full
    };

    RebuildRequired _rebuildRequired = RebuildRequired::None;

    QPoint _clickMousePosition;
    QPoint _lastMousePosition;
    bool _tooltipUpdateRequired = false;
    QCPLayer* _tooltipLayer = nullptr;
    QPointF _hoverPoint{-1.0, -1.0};
    QCPItemText* _hoverLabel = nullptr;
    QCPItemRect* _hoverColorRect = nullptr;
    QCPItemTracer* _itemTracer = nullptr;
    QFont _defaultFont9Pt;
    QFontMetrics _defaultFontMetrics{_defaultFont9Pt};

    QCustomPlot _customPlot;
    QCPLayoutGrid* _mainLayoutGrid = nullptr;
    QCPLayoutGrid* _axesLayoutGrid = nullptr;
    QCPLayoutGrid* _legendLayoutGrid = nullptr;

    QCPAxisRect* _discreteAxisRect = nullptr;
    QCPAxis* _discreteXAxis = nullptr;
    QCPAxis* _discreteYAxis = nullptr;

    QCPAxisRect* _continuousAxisRect = nullptr;
    QCPAxis* _continuousXAxis = nullptr;
    QCPAxis* _continuousYAxis = nullptr;
    QVector<QCPAbstractPlottable*> _meanPlots;

    PlotMode _plotMode = PlotMode::Normal;

    QCPAxisRect* _columnAnnotationsAxisRect = nullptr;
    bool _groupByAnnotation = false;
    QString _colorGroupByAnnotationName;

    std::set<size_t> _selectedColumns;

    QCPTextElement* _xAxisLabelTextElement = nullptr;

    CorrelationPluginInstance* _pluginInstance = nullptr;

    int _elideLabelWidth = 120;
    QVector<int> _selectedRows;
    bool _showColumnNames = true;
    bool _showGridLines = true;
    bool _showLegend = false;
    int _scaleType = static_cast<int>(PlotScaleType::Raw);
    QString _scaleByAttributeName;
    int _averagingType = static_cast<int>(PlotAveragingType::Individual);
    QString _averagingAttributeName;
    int _dispersionType = static_cast<int>(PlotDispersionType::None);
    int _dispersionVisualType = static_cast<int>(PlotDispersionVisualType::Bars);
    QVector<QVariantMap> _columnSortOrders;
    double _horizontalScrollPosition = 0.0;
    QString _xAxisLabel;
    QString _yAxisLabel;
    bool _includeYZero = false;
    bool _showIqrOutliers = true;
    bool _showAllColumns = false;
    int _rightPadding = 0;
    int _bottomPadding = 0;

    std::vector<size_t> _sortMap;

    std::set<QString> _visibleColumnAnnotationNames;
    std::vector<std::vector<size_t>> _annotationGroupMap;

    QCPLayer* _lineGraphLayer = nullptr;

    struct LineCacheEntry
    {
        QCPGraph* _graph = nullptr;
        double _minY = std::numeric_limits<double>::max();
        double _maxY = std::numeric_limits<double>::lowest();
    };

    QMap<int, LineCacheEntry> _lineGraphCache;

    using LabelElisionCacheEntry = QMap<int, QString>;
    QMap<QString, LabelElisionCacheEntry> _labelElisionCache;

    std::recursive_mutex _mutex;
    QThread _plotRenderThread;
    CorrelationPlotWorker* _worker = nullptr;

    QPixmap _pixmap;

    size_t numColumns() const;
    size_t numVisibleColumns() const;

    void populateMeanLinePlot();
    void populateMedianLinePlot();
    void populateLinePlot();
    void populateMeanHistogramPlot();
    void populateIQRPlot();
    void populateIQRAnnotationPlot(const QCPColumnAnnotations* qcpColumnAnnotations);
    void plotDispersion(QCPAbstractPlottable* meanPlot,
        double& minY, double& maxY,
        const QVector<double>& stdDevs, const QString& name);
    void populateStdDevPlot(QCPAbstractPlottable* meanPlot,
        double& minY, double& maxY,
        const QVector<int>& rows, QVector<double>& means);
    void populateStdErrorPlot(QCPAbstractPlottable* meanPlot,
        double& minY, double& maxY,
        const QVector<int>& rows, QVector<double>& means);
    void populateDispersion(QCPAbstractPlottable* meanPlot,
        double& minY, double& maxY,
        const QVector<int>& rows, QVector<double>& means);

    bool busy() const { return _worker != nullptr ? _worker->busy() : false; }
    bool zoomed() const { return _worker != nullptr ? _worker->zoomed() : false; }

    double minimumHeight() const;

    void setPluginInstance(CorrelationPluginInstance* pluginInstance);

    void setElideLabelWidth(int elideLabelWidth);
    bool showColumnNames() const;
    void setShowColumnNames(bool showColumnNames);
    void setShowGridLines(bool showGridLines);
    void setShowLegend(bool showLegend);
    void setHorizontalScrollPosition(double horizontalScrollPosition);
    void setRightPadding(int padding);
    void setBottomPadding(int padding);

    bool updateSortMap();
    void setColumnSortOrders(const QVector<QVariantMap>& columnSortOrders); // clazy:exclude=qproperty-type-mismatch

    QString elideLabel(const QString& label);

    int plotMode() const;
    void setPlotMode(int plotModeInt);

    std::vector<size_t> selectedColumns() const;

    QStringList visibleColumnAnnotationNames() const;
    void setVisibleColumnAnnotationNames(const QStringList& columnAnnotations);
    size_t numVisibleColumnAnnotations() const;
    QString columnAnnotationValueAt(size_t x, size_t y) const;

    void computeXAxisRange();
    void setContinousYAxisRange(double min, double max);
    void setContinousYAxisRangeForSelection();
    QVector<double> meanAverageData(double& min, double& max, const QVector<int>& rows);

    double visibleHorizontalFraction() const;
    bool isWide() const;
    double labelHeight() const;
    double minColumnWidth() const;
    double columnAxisWidth() const;
    double columnAnnotationsHeight() const;

    void createTooltip();

    void configureDiscreteAxisRect();
    void configureContinuousAxisRect();
    void configureAxisRects();

    QCPAxis* configureColumnAnnotations(QCPAxisRect* axisRect);
    void configureLegend();

    void onClick(const QMouseEvent* event);
    void onClickColumnAnnotation(const QCPAxisRect* axisRect, const QMouseEvent* event);

    void updatePixmap(CorrelationPlotUpdateType updateType);

    QPoint columnAnnotationPositionForPixel(const QCPAxisRect* axisRect, const QPointF& position);
    QCPAbstractPlottable* abstractPlottableUnderCursor(double& keyCoord);
    bool discreteTooltip(const QCPAxisRect* axisRect,
        const QCPAbstractPlottable* plottable, double xCoord);
    bool continuousTooltip(const QCPAxisRect* axisRect,
        const QCPAbstractPlottable* plottable, double xCoord);
    bool columnAnnotationTooltip(const QCPAxisRect* axisRect);

    static double logScale(double value, double epsilon = std::nextafter(0.0, 1.0));

    template<typename C>
    void logScale(C& values, double epsilon = std::nextafter(0.0, 1.0))
    {
        for(auto& value : values)
            value = logScale(value, epsilon);
    }

    static std::pair<double, double> addIQRBoxPlotTo(QCPAxis* keyAxis, QCPAxis* valueAxis,
        size_t column, QVector<double> values, bool showOutliers,
        const QColor& color = {}, const QString& text = {});

private slots:
    void onPixmapUpdated(const QPixmap& pixmap);
    void updatePlotSize();
    bool updateTooltip();

signals:
    void rightClick();
    void horizontalScrollPositionChanged();
    void selectedRowsChanged();
    void selectedColumnsChanged();
    void numVisibleColumnsChanged();
    void visibleHorizontalFractionChanged();
    void isWideChanged();
    void plotOptionsChanged();
    void visibleColumnAnnotationNamesChanged();
    void busyChanged();
    void zoomedChanged();
    void minimumHeightChanged();
    void pixmapUpdated();

    void plotModeChanged();
};
#endif // CORRELATIONPLOTITEM_H
