/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011-2022 Emanuel Eichhammer                            **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: https://www.qcustomplot.com/                         **
**             Date: 06.11.22                                             **
**          Version: 2.1.1                                                **
****************************************************************************/
/*! \file */
#ifndef QCP_PLOTTABLE_GRAPH_H
#define QCP_PLOTTABLE_GRAPH_H

#include "../global.h"
#include "../axis/range.h"
#include "../plottable1d.h"
#include "../painter.h"
#include "../datacontainer.h"

class QCPPainter;
class QCPAxis;

class QCP_LIB_DECL QCPGraphData
{
public:
  QCPGraphData();
  QCPGraphData(double key, double value);
  
  inline double sortKey() const { return key; }
  inline static QCPGraphData fromSortKey(double sortKey) { return QCPGraphData(sortKey, 0); }
  inline static bool sortKeyIsMainKey() { return true; }
  
  inline double mainKey() const { return key; }
  inline double mainValue() const { return value; }
  
  inline QCPRange valueRange() const { return QCPRange(value, value); }
  
  double key, value;
};
Q_DECLARE_TYPEINFO(QCPGraphData, Q_PRIMITIVE_TYPE);


/*! \typedef QCPGraphDataContainer
  
  Container for storing \ref QCPGraphData points. The data is stored sorted by \a key.
  
  This template instantiation is the container in which QCPGraph holds its data. For details about
  the generic container, see the documentation of the class template \ref QCPDataContainer.
  
  \see QCPGraphData, QCPGraph::setData
*/
typedef QCPDataContainer<QCPGraphData> QCPGraphDataContainer;

class QCP_LIB_DECL QCPGraph : public QCPAbstractPlottable1D<QCPGraphData>
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(LineStyle lineStyle READ lineStyle WRITE setLineStyle)
  Q_PROPERTY(QCPScatterStyle scatterStyle READ scatterStyle WRITE setScatterStyle)
  Q_PROPERTY(int scatterSkip READ scatterSkip WRITE setScatterSkip)
  Q_PROPERTY(QCPGraph* channelFillGraph READ channelFillGraph WRITE setChannelFillGraph)
  Q_PROPERTY(bool adaptiveSampling READ adaptiveSampling WRITE setAdaptiveSampling)
  /// \endcond
public:
  /*!
    Defines how the graph's line is represented visually in the plot. The line is drawn with the
    current pen of the graph (\ref setPen).
    \see setLineStyle
  */
  enum LineStyle { lsNone        ///< data points are not connected with any lines (e.g. data only represented
                                 ///< with symbols according to the scatter style, see \ref setScatterStyle)
                   ,lsLine       ///< data points are connected by a straight line
                   ,lsStepLeft   ///< line is drawn as steps where the step height is the value of the left data point
                   ,lsStepRight  ///< line is drawn as steps where the step height is the value of the right data point
                   ,lsStepCenter ///< line is drawn as steps where the step is in between two data points
                   ,lsImpulse    ///< each data point is represented by a line parallel to the value axis, which reaches from the data point to the zero-value-line
                 };
  Q_ENUM(LineStyle)
  
  explicit QCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);
  virtual ~QCPGraph() Q_DECL_OVERRIDE;
  
  // getters:
  QSharedPointer<QCPGraphDataContainer> data() const { return mDataContainer; }
  LineStyle lineStyle() const { return mLineStyle; }
  QCPScatterStyle scatterStyle() const { return mScatterStyle; }
  int scatterSkip() const { return mScatterSkip; }
  QCPGraph *channelFillGraph() const { return mChannelFillGraph.data(); }
  bool adaptiveSampling() const { return mAdaptiveSampling; }
  
  // setters:
  void setData(QSharedPointer<QCPGraphDataContainer> data);
  void setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void setLineStyle(LineStyle ls);
  void setScatterStyle(const QCPScatterStyle &style);
  void setScatterSkip(int skip);
  void setChannelFillGraph(QCPGraph *targetGraph);
  void setAdaptiveSampling(bool enabled);
  
  // non-property methods:
  void addData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void addData(double key, double value);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  virtual QCPRange getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth) const Q_DECL_OVERRIDE;
  virtual QCPRange getValueRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth, const QCPRange &inKeyRange=QCPRange()) const Q_DECL_OVERRIDE;
  
protected:
  // property members:
  LineStyle mLineStyle;
  QCPScatterStyle mScatterStyle;
  int mScatterSkip;
  QPointer<QCPGraph> mChannelFillGraph;
  bool mAdaptiveSampling;
  
  // reimplemented virtual methods:
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const Q_DECL_OVERRIDE;
  
  // introduced virtual methods:
  virtual void drawFill(QCPPainter *painter, QVector<QPointF> *lines) const;
  virtual void drawScatterPlot(QCPPainter *painter, const QVector<QPointF> &scatters, const QCPScatterStyle &style) const;
  virtual void drawLinePlot(QCPPainter *painter, const QVector<QPointF> &lines) const;
  virtual void drawImpulsePlot(QCPPainter *painter, const QVector<QPointF> &lines) const;
  
  virtual void getOptimizedLineData(QVector<QCPGraphData> *lineData, const QCPGraphDataContainer::const_iterator &begin, const QCPGraphDataContainer::const_iterator &end) const;
  virtual void getOptimizedScatterData(QVector<QCPGraphData> *scatterData, QCPGraphDataContainer::const_iterator begin, QCPGraphDataContainer::const_iterator end) const;
  
  // non-virtual methods:
  void getVisibleDataBounds(QCPGraphDataContainer::const_iterator &begin, QCPGraphDataContainer::const_iterator &end, const QCPDataRange &rangeRestriction) const;
  void getLines(QVector<QPointF> *lines, const QCPDataRange &dataRange) const;
  void getScatters(QVector<QPointF> *scatters, const QCPDataRange &dataRange) const;
  QVector<QPointF> dataToLines(const QVector<QCPGraphData> &data) const;
  QVector<QPointF> dataToStepLeftLines(const QVector<QCPGraphData> &data) const;
  QVector<QPointF> dataToStepRightLines(const QVector<QCPGraphData> &data) const;
  QVector<QPointF> dataToStepCenterLines(const QVector<QCPGraphData> &data) const;
  QVector<QPointF> dataToImpulseLines(const QVector<QCPGraphData> &data) const;
  QVector<QCPDataRange> getNonNanSegments(const QVector<QPointF> *lineData, Qt::Orientation keyOrientation) const;
  QVector<QPair<QCPDataRange, QCPDataRange> > getOverlappingSegments(QVector<QCPDataRange> thisSegments, const QVector<QPointF> *thisData, QVector<QCPDataRange> otherSegments, const QVector<QPointF> *otherData) const;
  bool segmentsIntersect(double aLower, double aUpper, double bLower, double bUpper, int &bPrecedence) const;
  QPointF getFillBasePoint(QPointF matchingDataPoint) const;
  const QPolygonF getFillPolygon(const QVector<QPointF> *lineData, QCPDataRange segment) const;
  const QPolygonF getChannelFillPolygon(const QVector<QPointF> *thisData, QCPDataRange thisSegment, const QVector<QPointF> *otherData, QCPDataRange otherSegment) const;
  int findIndexBelowX(const QVector<QPointF> *data, double x) const;
  int findIndexAboveX(const QVector<QPointF> *data, double x) const;
  int findIndexBelowY(const QVector<QPointF> *data, double y) const;
  int findIndexAboveY(const QVector<QPointF> *data, double y) const;
  double pointDistance(const QPointF &pixelPoint, QCPGraphDataContainer::const_iterator &closestData) const;
  
  friend class QCustomPlot;
  friend class QCPLegend;
};
Q_DECLARE_METATYPE(QCPGraph::LineStyle)

#endif // QCP_PLOTTABLE_GRAPH_H
