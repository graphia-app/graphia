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
#ifndef QCP_PLOTTABLE_CURVE_H
#define QCP_PLOTTABLE_CURVE_H

#include "../global.h"
#include "../axis/range.h"
#include "../plottable1d.h"
#include "../painter.h"
#include "../datacontainer.h"

class QCPPainter;
class QCPAxis;

class QCP_LIB_DECL QCPCurveData
{
public:
  QCPCurveData();
  QCPCurveData(double t, double key, double value);
  
  inline double sortKey() const { return t; }
  inline static QCPCurveData fromSortKey(double sortKey) { return QCPCurveData(sortKey, 0, 0); }
  inline static bool sortKeyIsMainKey() { return false; }
  
  inline double mainKey() const { return key; }
  inline double mainValue() const { return value; }
  
  inline QCPRange valueRange() const { return QCPRange(value, value); }
  
  double t, key, value;
};
Q_DECLARE_TYPEINFO(QCPCurveData, Q_PRIMITIVE_TYPE);


/*! \typedef QCPCurveDataContainer
  
  Container for storing \ref QCPCurveData points. The data is stored sorted by \a t, so the \a
  sortKey() (returning \a t) is different from \a mainKey() (returning \a key).
  
  This template instantiation is the container in which QCPCurve holds its data. For details about
  the generic container, see the documentation of the class template \ref QCPDataContainer.
  
  \see QCPCurveData, QCPCurve::setData
*/
typedef QCPDataContainer<QCPCurveData> QCPCurveDataContainer;

class QCP_LIB_DECL QCPCurve : public QCPAbstractPlottable1D<QCPCurveData>
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QCPScatterStyle scatterStyle READ scatterStyle WRITE setScatterStyle)
  Q_PROPERTY(int scatterSkip READ scatterSkip WRITE setScatterSkip)
  Q_PROPERTY(LineStyle lineStyle READ lineStyle WRITE setLineStyle)
  /// \endcond
public:
  /*!
    Defines how the curve's line is represented visually in the plot. The line is drawn with the
    current pen of the curve (\ref setPen).
    \see setLineStyle
  */
  enum LineStyle { lsNone  ///< No line is drawn between data points (e.g. only scatters)
                   ,lsLine ///< Data points are connected with a straight line
                 };
  Q_ENUM(LineStyle)
  
  explicit QCPCurve(QCPAxis *keyAxis, QCPAxis *valueAxis);
  virtual ~QCPCurve() Q_DECL_OVERRIDE;
  
  // getters:
  QSharedPointer<QCPCurveDataContainer> data() const { return mDataContainer; }
  QCPScatterStyle scatterStyle() const { return mScatterStyle; }
  int scatterSkip() const { return mScatterSkip; }
  LineStyle lineStyle() const { return mLineStyle; }
  
  // setters:
  void setData(QSharedPointer<QCPCurveDataContainer> data);
  void setData(const QVector<double> &t, const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void setData(const QVector<double> &keys, const QVector<double> &values);
  void setScatterStyle(const QCPScatterStyle &style);
  void setScatterSkip(int skip);
  void setLineStyle(LineStyle style);
  
  // non-property methods:
  void addData(const QVector<double> &t, const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void addData(const QVector<double> &keys, const QVector<double> &values);
  void addData(double t, double key, double value);
  void addData(double key, double value);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  virtual QCPRange getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth) const Q_DECL_OVERRIDE;
  virtual QCPRange getValueRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth, const QCPRange &inKeyRange=QCPRange()) const Q_DECL_OVERRIDE;
  
protected:
  // property members:
  QCPScatterStyle mScatterStyle;
  int mScatterSkip;
  LineStyle mLineStyle;
  
  // reimplemented virtual methods:
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const Q_DECL_OVERRIDE;
  
  // introduced virtual methods:
  virtual void drawCurveLine(QCPPainter *painter, const QVector<QPointF> &lines) const;
  virtual void drawScatterPlot(QCPPainter *painter, const QVector<QPointF> &points, const QCPScatterStyle &style) const;
  
  // non-virtual methods:
  void getCurveLines(QVector<QPointF> *lines, const QCPDataRange &dataRange, double penWidth) const;
  void getScatters(QVector<QPointF> *scatters, const QCPDataRange &dataRange, double scatterWidth) const;
  int getRegion(double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const;
  QPointF getOptimizedPoint(int otherRegion, double otherKey, double otherValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const;
  QVector<QPointF> getOptimizedCornerPoints(int prevRegion, int currentRegion, double prevKey, double prevValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const;
  bool mayTraverse(int prevRegion, int currentRegion) const;
  bool getTraverse(double prevKey, double prevValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin, QPointF &crossA, QPointF &crossB) const;
  void getTraverseCornerPoints(int prevRegion, int currentRegion, double keyMin, double valueMax, double keyMax, double valueMin, QVector<QPointF> &beforeTraverse, QVector<QPointF> &afterTraverse) const;
  double pointDistance(const QPointF &pixelPoint, QCPCurveDataContainer::const_iterator &closestData) const;
  
  friend class QCustomPlot;
  friend class QCPLegend;
};
Q_DECLARE_METATYPE(QCPCurve::LineStyle)

#endif // QCP_PLOTTABLE_CURVE_H
