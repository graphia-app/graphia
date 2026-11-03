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

#ifndef QCP_POLARGRAPH_H
#define QCP_POLARGRAPH_H

#include "../global.h"
#include "../vector2d.h"
#include "../axis/range.h"
#include "../layer.h"
#include "../selection.h"
#include "../scatterstyle.h"
#include "../plottables/plottable-graph.h"
#include "layoutelement-angularaxis.h"
#include "../layoutelements/layoutelement-legend.h"
#include "radialaxis.h"

class QCPPainter;
class QCPPlottableInterface1D;
class QCPLegend;
class QCPPolarGraph;


class QCP_LIB_DECL QCPPolarLegendItem : public QCPAbstractLegendItem
{
  Q_OBJECT
public:
  QCPPolarLegendItem(QCPLegend *parent, QCPPolarGraph *graph);
  
  // getters:
  QCPPolarGraph *polarGraph() { return mPolarGraph; }
  
protected:
  // property members:
  QCPPolarGraph *mPolarGraph;
  
  // reimplemented virtual methods:
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  virtual QSize minimumOuterSizeHint() const Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  QPen getIconBorderPen() const;
  QColor getTextColor() const;
  QFont getFont() const;
};


class QCP_LIB_DECL QCPPolarGraph : public QCPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  
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
                 };
  Q_ENUM(LineStyle)
  
  QCPPolarGraph(QCPPolarAxisAngular *keyAxis, QCPPolarAxisRadial *valueAxis);
  virtual ~QCPPolarGraph();
  
  // getters:
  QString name() const { return mName; }
  bool antialiasedFill() const { return mAntialiasedFill; }
  bool antialiasedScatters() const { return mAntialiasedScatters; }
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  bool periodic() const { return mPeriodic; }
  QCPPolarAxisAngular *keyAxis() const { return mKeyAxis.data(); }
  QCPPolarAxisRadial *valueAxis() const { return mValueAxis.data(); }
  QCP::SelectionType selectable() const { return mSelectable; }
  bool selected() const { return !mSelection.isEmpty(); }
  QCPDataSelection selection() const { return mSelection; }
  //QCPSelectionDecorator *selectionDecorator() const { return mSelectionDecorator; }
  QSharedPointer<QCPGraphDataContainer> data() const { return mDataContainer; }
  LineStyle lineStyle() const { return mLineStyle; }
  QCPScatterStyle scatterStyle() const { return mScatterStyle; }
  
  // setters:
  void setName(const QString &name);
  void setAntialiasedFill(bool enabled);
  void setAntialiasedScatters(bool enabled);
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setPeriodic(bool enabled);
  void setKeyAxis(QCPPolarAxisAngular *axis);
  void setValueAxis(QCPPolarAxisRadial *axis);
  Q_SLOT void setSelectable(QCP::SelectionType selectable);
  Q_SLOT void setSelection(QCPDataSelection selection);
  //void setSelectionDecorator(QCPSelectionDecorator *decorator);
  void setData(QSharedPointer<QCPGraphDataContainer> data);
  void setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void setLineStyle(LineStyle ls);
  void setScatterStyle(const QCPScatterStyle &style);

  // non-property methods:
  void addData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
  void addData(double key, double value);
  void coordsToPixels(double key, double value, double &x, double &y) const;
  const QPointF coordsToPixels(double key, double value) const;
  void pixelsToCoords(double x, double y, double &key, double &value) const;
  void pixelsToCoords(const QPointF &pixelPos, double &key, double &value) const;
  void rescaleAxes(bool onlyEnlarge=false) const;
  void rescaleKeyAxis(bool onlyEnlarge=false) const;
  void rescaleValueAxis(bool onlyEnlarge=false, bool inKeyRange=false) const;
  bool addToLegend(QCPLegend *legend);
  bool addToLegend();
  bool removeFromLegend(QCPLegend *legend) const;
  bool removeFromLegend() const;
  
  // introduced virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const; // actually introduced in QCPLayerable as non-pure, but we want to force reimplementation for plottables
  virtual QCPPlottableInterface1D *interface1D() { return 0; } // TODO: return this later, when QCPAbstractPolarPlottable is created
  virtual QCPRange getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth) const;
  virtual QCPRange getValueRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth, const QCPRange &inKeyRange=QCPRange()) const;
  
signals:
  void selectionChanged(bool selected);
  void selectionChanged(const QCPDataSelection &selection);
  void selectableChanged(QCP::SelectionType selectable);
  
protected:
  // property members:
  QSharedPointer<QCPGraphDataContainer> mDataContainer;
  LineStyle mLineStyle;
  QCPScatterStyle mScatterStyle;
  QString mName;
  bool mAntialiasedFill, mAntialiasedScatters;
  QPen mPen;
  QBrush mBrush;
  bool mPeriodic;
  QPointer<QCPPolarAxisAngular> mKeyAxis;
  QPointer<QCPPolarAxisRadial> mValueAxis;
  QCP::SelectionType mSelectable;
  QCPDataSelection mSelection;
  //QCPSelectionDecorator *mSelectionDecorator;
  
  // introduced virtual methods (later reimplemented TODO from QCPAbstractPolarPlottable):
  virtual QRect clipRect() const;
  virtual void draw(QCPPainter *painter);
  virtual QCP::Interaction selectionCategory() const;
  void applyDefaultAntialiasingHint(QCPPainter *painter) const;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged);
  virtual void deselectEvent(bool *selectionStateChanged);
  // virtual drawing helpers:
  virtual void drawLinePlot(QCPPainter *painter, const QVector<QPointF> &lines) const;
  virtual void drawFill(QCPPainter *painter, QVector<QPointF> *lines) const;
  virtual void drawScatterPlot(QCPPainter *painter, const QVector<QPointF> &scatters, const QCPScatterStyle &style) const;
  
  // introduced virtual methods:
  virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const;
  
  // non-virtual methods:
  void applyFillAntialiasingHint(QCPPainter *painter) const;
  void applyScattersAntialiasingHint(QCPPainter *painter) const;
  double pointDistance(const QPointF &pixelPoint, QCPGraphDataContainer::const_iterator &closestData) const;
  // drawing helpers:
  virtual int dataCount() const;
  void getDataSegments(QList<QCPDataRange> &selectedSegments, QList<QCPDataRange> &unselectedSegments) const;
  void drawPolyline(QCPPainter *painter, const QVector<QPointF> &lineData) const;
  void getVisibleDataBounds(QCPGraphDataContainer::const_iterator &begin, QCPGraphDataContainer::const_iterator &end, const QCPDataRange &rangeRestriction) const;
  void getLines(QVector<QPointF> *lines, const QCPDataRange &dataRange) const;
  void getScatters(QVector<QPointF> *scatters, const QCPDataRange &dataRange) const;
  void getOptimizedLineData(QVector<QCPGraphData> *lineData, const QCPGraphDataContainer::const_iterator &begin, const QCPGraphDataContainer::const_iterator &end) const;
  void getOptimizedScatterData(QVector<QCPGraphData> *scatterData, QCPGraphDataContainer::const_iterator begin, QCPGraphDataContainer::const_iterator end) const;
  QVector<QPointF> dataToLines(const QVector<QCPGraphData> &data) const;

private:
  Q_DISABLE_COPY(QCPPolarGraph)
  
  friend class QCPPolarLegendItem;
};

#endif // QCP_POLARGRAPH_H
