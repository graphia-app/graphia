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

#include "plottable-graph.h"

#include "../painter.h"
#include "../core.h"
#include "../axis/axis.h"
#include "../layoutelements/layoutelement-axisrect.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPGraphData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPGraphData
  \brief Holds the data of one single data point for QCPGraph.
  
  The stored data is:
  \li \a key: coordinate on the key axis of this data point (this is the \a mainKey and the \a sortKey)
  \li \a value: coordinate on the value axis of this data point (this is the \a mainValue)
  
  The container for storing multiple data points is \ref QCPGraphDataContainer. It is a typedef for
  \ref QCPDataContainer with \ref QCPGraphData as the DataType template parameter. See the
  documentation there for an explanation regarding the data type's generic methods.
  
  \see QCPGraphDataContainer
*/

/* start documentation of inline functions */

/*! \fn double QCPGraphData::sortKey() const
  
  Returns the \a key member of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn static QCPGraphData QCPGraphData::fromSortKey(double sortKey)
  
  Returns a data point with the specified \a sortKey. All other members are set to zero.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn static static bool QCPGraphData::sortKeyIsMainKey()
  
  Since the member \a key is both the data point key coordinate and the data ordering parameter,
  this method returns true.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn double QCPGraphData::mainKey() const
  
  Returns the \a key member of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn double QCPGraphData::mainValue() const
  
  Returns the \a value member of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn QCPRange QCPGraphData::valueRange() const
  
  Returns a QCPRange with both lower and upper boundary set to \a value of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/* end documentation of inline functions */

/*!
  Constructs a data point with key and value set to zero.
*/
QCPGraphData::QCPGraphData() :
  key(0),
  value(0)
{
}

/*!
  Constructs a data point with the specified \a key and \a value.
*/
QCPGraphData::QCPGraphData(double key, double value) :
  key(key),
  value(value)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPGraph
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPGraph
  \brief A plottable representing a graph in a plot.

  \image html QCPGraph.png
  
  Usually you create new graphs by calling QCustomPlot::addGraph. The resulting instance can be
  accessed via QCustomPlot::graph.

  To plot data, assign it with the \ref setData or \ref addData functions. Alternatively, you can
  also access and modify the data via the \ref data method, which returns a pointer to the internal
  \ref QCPGraphDataContainer.
  
  Graphs are used to display single-valued data. Single-valued means that there should only be one
  data point per unique key coordinate. In other words, the graph can't have \a loops. If you do
  want to plot non-single-valued curves, rather use the QCPCurve plottable.
  
  Gaps in the graph line can be created by adding data points with NaN as value
  (<tt>qQNaN()</tt> or <tt>std::numeric_limits<double>::quiet_NaN()</tt>) in between the two data points that shall be
  separated.
  
  \section qcpgraph-appearance Changing the appearance
  
  The appearance of the graph is mainly determined by the line style, scatter style, brush and pen
  of the graph (\ref setLineStyle, \ref setScatterStyle, \ref setBrush, \ref setPen).
  
  \subsection filling Filling under or between graphs
  
  QCPGraph knows two types of fills: Normal graph fills towards the zero-value-line parallel to
  the key axis of the graph, and fills between two graphs, called channel fills. To enable a fill,
  just set a brush with \ref setBrush which is neither Qt::NoBrush nor fully transparent.
  
  By default, a normal fill towards the zero-value-line will be drawn. To set up a channel fill
  between this graph and another one, call \ref setChannelFillGraph with the other graph as
  parameter.

  \see QCustomPlot::addGraph, QCustomPlot::graph
*/

/* start of documentation of inline functions */

/*! \fn QSharedPointer<QCPGraphDataContainer> QCPGraph::data() const
  
  Returns a shared pointer to the internal data storage of type \ref QCPGraphDataContainer. You may
  use it to directly manipulate the data, which may be more convenient and faster than using the
  regular \ref setData or \ref addData methods.
*/

/* end of documentation of inline functions */

/*!
  Constructs a graph which uses \a keyAxis as its key axis ("x") and \a valueAxis as its value
  axis ("y"). \a keyAxis and \a valueAxis must reside in the same QCustomPlot instance and not have
  the same orientation. If either of these restrictions is violated, a corresponding message is
  printed to the debug output (qDebug), the construction is not aborted, though.
  
  The created QCPGraph is automatically registered with the QCustomPlot instance inferred from \a
  keyAxis. This QCustomPlot instance takes ownership of the QCPGraph, so do not delete it manually
  but use QCustomPlot::removePlottable() instead.
  
  To directly create a graph inside a plot, you can also use the simpler QCustomPlot::addGraph function.
*/
QCPGraph::QCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) :
  QCPAbstractPlottable1D<QCPGraphData>(keyAxis, valueAxis),
  mLineStyle{},
  mScatterSkip{},
  mAdaptiveSampling{}
{
  // special handling for QCPGraphs to maintain the simple graph interface:
  mParentPlot->registerGraph(this);

  setPen(QPen(Qt::blue, 0));
  setBrush(Qt::NoBrush);
  
  setLineStyle(lsLine);
  setScatterSkip(0);
  setChannelFillGraph(nullptr);
  setAdaptiveSampling(true);
}

QCPGraph::~QCPGraph()
{
}

/*! \overload
  
  Replaces the current data container with the provided \a data container.
  
  Since a QSharedPointer is used, multiple QCPGraphs may share the same data container safely.
  Modifying the data in the container will then affect all graphs that share the container. Sharing
  can be achieved by simply exchanging the data containers wrapped in shared pointers:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpgraph-datasharing-1
  
  If you do not wish to share containers, but create a copy from an existing container, rather use
  the \ref QCPDataContainer<DataType>::set method on the graph's data container directly:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpgraph-datasharing-2
  
  \see addData
*/
void QCPGraph::setData(QSharedPointer<QCPGraphDataContainer> data)
{
  mDataContainer = data;
}

/*! \overload
  
  Replaces the current data with the provided points in \a keys and \a values. The provided
  vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
  
  If you can guarantee that the passed data points are sorted by \a keys in ascending order, you
  can set \a alreadySorted to true, to improve performance by saving a sorting run.
  
  \see addData
*/
void QCPGraph::setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  mDataContainer->clear();
  addData(keys, values, alreadySorted);
}

/*!
  Sets how the single data points are connected in the plot. For scatter-only plots, set \a ls to
  \ref lsNone and \ref setScatterStyle to the desired scatter style.
  
  \see setScatterStyle
*/
void QCPGraph::setLineStyle(LineStyle ls)
{
  mLineStyle = ls;
}

/*!
  Sets the visual appearance of single data points in the plot. If set to \ref QCPScatterStyle::ssNone, no scatter points
  are drawn (e.g. for line-only-plots with appropriate line style).
  
  \see QCPScatterStyle, setLineStyle
*/
void QCPGraph::setScatterStyle(const QCPScatterStyle &style)
{
  mScatterStyle = style;
}

/*!
  If scatters are displayed (scatter style not \ref QCPScatterStyle::ssNone), \a skip number of
  scatter points are skipped/not drawn after every drawn scatter point.

  This can be used to make the data appear sparser while for example still having a smooth line,
  and to improve performance for very high density plots.

  If \a skip is set to 0 (default), all scatter points are drawn.

  \see setScatterStyle
*/
void QCPGraph::setScatterSkip(int skip)
{
  mScatterSkip = qMax(0, skip);
}

/*!
  Sets the target graph for filling the area between this graph and \a targetGraph with the current
  brush (\ref setBrush).
  
  When \a targetGraph is set to 0, a normal graph fill to the zero-value-line will be shown. To
  disable any filling, set the brush to Qt::NoBrush.

  \see setBrush
*/
void QCPGraph::setChannelFillGraph(QCPGraph *targetGraph)
{
  // prevent setting channel target to this graph itself:
  if (targetGraph == this)
  {
    qDebug() << Q_FUNC_INFO << "targetGraph is this graph itself";
    mChannelFillGraph = nullptr;
    return;
  }
  // prevent setting channel target to a graph not in the plot:
  if (targetGraph && targetGraph->mParentPlot != mParentPlot)
  {
    qDebug() << Q_FUNC_INFO << "targetGraph not in same plot";
    mChannelFillGraph = nullptr;
    return;
  }
  
  mChannelFillGraph = targetGraph;
}

/*!
  Sets whether adaptive sampling shall be used when plotting this graph. QCustomPlot's adaptive
  sampling technique can drastically improve the replot performance for graphs with a larger number
  of points (e.g. above 10,000), without notably changing the appearance of the graph.
  
  By default, adaptive sampling is enabled. Even if enabled, QCustomPlot decides whether adaptive
  sampling shall actually be used on a per-graph basis. So leaving adaptive sampling enabled has no
  disadvantage in almost all cases.
  
  \image html adaptive-sampling-line.png "A line plot of 500,000 points without and with adaptive sampling"
  
  As can be seen, line plots experience no visual degradation from adaptive sampling. Outliers are
  reproduced reliably, as well as the overall shape of the data set. The replot time reduces
  dramatically though. This allows QCustomPlot to display large amounts of data in realtime.
  
  \image html adaptive-sampling-scatter.png "A scatter plot of 100,000 points without and with adaptive sampling"
  
  Care must be taken when using high-density scatter plots in combination with adaptive sampling.
  The adaptive sampling algorithm treats scatter plots more carefully than line plots which still
  gives a significant reduction of replot times, but not quite as much as for line plots. This is
  because scatter plots inherently need more data points to be preserved in order to still resemble
  the original, non-adaptive-sampling plot. As shown above, the results still aren't quite
  identical, as banding occurs for the outer data points. This is in fact intentional, such that
  the boundaries of the data cloud stay visible to the viewer. How strong the banding appears,
  depends on the point density, i.e. the number of points in the plot.
  
  For some situations with scatter plots it might thus be desirable to manually turn adaptive
  sampling off. For example, when saving the plot to disk. This can be achieved by setting \a
  enabled to false before issuing a command like \ref QCustomPlot::savePng, and setting \a enabled
  back to true afterwards.
*/
void QCPGraph::setAdaptiveSampling(bool enabled)
{
  mAdaptiveSampling = enabled;
}

/*! \overload
  
  Adds the provided points in \a keys and \a values to the current data. The provided vectors
  should have equal length. Else, the number of added points will be the size of the smallest
  vector.
  
  If you can guarantee that the passed data points are sorted by \a keys in ascending order, you
  can set \a alreadySorted to true, to improve performance by saving a sorting run.
  
  Alternatively, you can also access and modify the data directly via the \ref data method, which
  returns a pointer to the internal data container.
*/
void QCPGraph::addData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  if (keys.size() != values.size())
    qDebug() << Q_FUNC_INFO << "keys and values have different sizes:" << keys.size() << values.size();
  const int n = qMin(keys.size(), values.size());
  QVector<QCPGraphData> tempData(n);
  QVector<QCPGraphData>::iterator it = tempData.begin();
  const QVector<QCPGraphData>::iterator itEnd = tempData.end();
  int i = 0;
  while (it != itEnd)
  {
    it->key = keys[i];
    it->value = values[i];
    ++it;
    ++i;
  }
  mDataContainer->add(tempData, alreadySorted); // don't modify tempData beyond this to prevent copy on write
}

/*! \overload
  
  Adds the provided data point as \a key and \a value to the current data.
  
  Alternatively, you can also access and modify the data directly via the \ref data method, which
  returns a pointer to the internal data container.
*/
void QCPGraph::addData(double key, double value)
{
  mDataContainer->add(QCPGraphData(key, value));
}

/*!
  Implements a selectTest specific to this plottable's point geometry.

  If \a details is not 0, it will be set to a \ref QCPDataSelection, describing the closest data
  point to \a pos.
  
  \seebaseclassmethod \ref QCPAbstractPlottable::selectTest
*/
double QCPGraph::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if ((onlySelectable && mSelectable == QCP::stNone) || mDataContainer->isEmpty())
    return -1;
  if (!mKeyAxis || !mValueAxis)
    return -1;
  
  if (mKeyAxis.data()->axisRect()->rect().contains(pos.toPoint()) || mParentPlot->interactions().testFlag(QCP::iSelectPlottablesBeyondAxisRect))
  {
    QCPGraphDataContainer::const_iterator closestDataPoint = mDataContainer->constEnd();
    double result = pointDistance(pos, closestDataPoint);
    if (details)
    {
      int pointIndex = int(closestDataPoint-mDataContainer->constBegin());
      details->setValue(QCPDataSelection(QCPDataRange(pointIndex, pointIndex+1)));
    }
    return result;
  } else
    return -1;
}

/* inherits documentation from base class */
QCPRange QCPGraph::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
  return mDataContainer->keyRange(foundRange, inSignDomain);
}

/* inherits documentation from base class */
QCPRange QCPGraph::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain, const QCPRange &inKeyRange) const
{
  return mDataContainer->valueRange(foundRange, inSignDomain, inKeyRange);
}

/* inherits documentation from base class */
void QCPGraph::draw(QCPPainter *painter)
{
  if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  if (mKeyAxis.data()->range().size() <= 0 || mDataContainer->isEmpty()) return;
  if (mLineStyle == lsNone && mScatterStyle.isNone()) return;
  
  QVector<QPointF> lines, scatters; // line and (if necessary) scatter pixel coordinates will be stored here while iterating over segments
  
  // loop over and draw segments of unselected/selected data:
  QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  allSegments << unselectedSegments << selectedSegments;
  for (int i=0; i<allSegments.size(); ++i)
  {
    bool isSelectedSegment = i >= unselectedSegments.size();
    // get line pixel points appropriate to line style:
    QCPDataRange lineDataRange = isSelectedSegment ? allSegments.at(i) : allSegments.at(i).adjusted(-1, 1); // unselected segments extend lines to bordering selected data point (safe to exceed total data bounds in first/last segment, getLines takes care)
    getLines(&lines, lineDataRange);
    
    // check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA
    QCPGraphDataContainer::const_iterator it;
    for (it = mDataContainer->constBegin(); it != mDataContainer->constEnd(); ++it)
    {
      if (QCP::isInvalidData(it->key, it->value))
        qDebug() << Q_FUNC_INFO << "Data point at" << it->key << "invalid." << "Plottable name:" << name();
    }
#endif
    
    // draw fill of graph:
    if (isSelectedSegment && mSelectionDecorator)
      mSelectionDecorator->applyBrush(painter);
    else
      painter->setBrush(mBrush);
    painter->setPen(Qt::NoPen);
    drawFill(painter, &lines);
    
    // draw line:
    if (mLineStyle != lsNone)
    {
      if (isSelectedSegment && mSelectionDecorator)
        mSelectionDecorator->applyPen(painter);
      else
        painter->setPen(mPen);
      painter->setBrush(Qt::NoBrush);
      if (mLineStyle == lsImpulse)
        drawImpulsePlot(painter, lines);
      else
        drawLinePlot(painter, lines); // also step plots can be drawn as a line plot
    }
    
    // draw scatters:
    QCPScatterStyle finalScatterStyle = mScatterStyle;
    if (isSelectedSegment && mSelectionDecorator)
      finalScatterStyle = mSelectionDecorator->getFinalScatterStyle(mScatterStyle);
    if (!finalScatterStyle.isNone())
    {
      getScatters(&scatters, allSegments.at(i));
      drawScatterPlot(painter, scatters, finalScatterStyle);
    }
  }
  
  // draw other selection decoration that isn't just line/scatter pens and brushes:
  if (mSelectionDecorator)
    mSelectionDecorator->drawDecoration(painter, selection());
}

/* inherits documentation from base class */
void QCPGraph::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
  // draw fill:
  if (mBrush.style() != Qt::NoBrush)
  {
    applyFillAntialiasingHint(painter);
    painter->fillRect(QRectF(rect.left(), rect.top()+rect.height()/2.0, rect.width(), rect.height()/3.0), mBrush);
  }
  // draw line vertically centered:
  if (mLineStyle != lsNone)
  {
    applyDefaultAntialiasingHint(painter);
    painter->setPen(mPen);
    painter->drawLine(QLineF(rect.left(), rect.top()+rect.height()/2.0, rect.right()+5, rect.top()+rect.height()/2.0)); // +5 on x2 else last segment is missing from dashed/dotted pens
  }
  // draw scatter symbol:
  if (!mScatterStyle.isNone())
  {
    applyScattersAntialiasingHint(painter);
    // scale scatter pixmap if it's too large to fit in legend icon rect:
    if (mScatterStyle.shape() == QCPScatterStyle::ssPixmap && (mScatterStyle.pixmap().size().width() > rect.width() || mScatterStyle.pixmap().size().height() > rect.height()))
    {
      QCPScatterStyle scaledStyle(mScatterStyle);
      scaledStyle.setPixmap(scaledStyle.pixmap().scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
      scaledStyle.applyTo(painter, mPen);
      scaledStyle.drawShape(painter, QRectF(rect).center());
    } else
    {
      mScatterStyle.applyTo(painter, mPen);
      mScatterStyle.drawShape(painter, QRectF(rect).center());
    }
  }
}

/*! \internal

  This method retrieves an optimized set of data points via \ref getOptimizedLineData, and branches
  out to the line style specific functions such as \ref dataToLines, \ref dataToStepLeftLines, etc.
  according to the line style of the graph.

  \a lines will be filled with points in pixel coordinates, that can be drawn with the according
  draw functions like \ref drawLinePlot and \ref drawImpulsePlot. The points returned in \a lines
  aren't necessarily the original data points. For example, step line styles require additional
  points to form the steps when drawn. If the line style of the graph is \ref lsNone, the \a
  lines vector will be empty.

  \a dataRange specifies the beginning and ending data indices that will be taken into account for
  conversion. In this function, the specified range may exceed the total data bounds without harm:
  a correspondingly trimmed data range will be used. This takes the burden off the user of this
  function to check for valid indices in \a dataRange, e.g. when extending ranges coming from \ref
  getDataSegments.

  \see getScatters
*/
void QCPGraph::getLines(QVector<QPointF> *lines, const QCPDataRange &dataRange) const
{
  if (!lines) return;
  QCPGraphDataContainer::const_iterator begin, end;
  getVisibleDataBounds(begin, end, dataRange);
  if (begin == end)
  {
    lines->clear();
    return;
  }
  
  QVector<QCPGraphData> lineData;
  if (mLineStyle != lsNone)
    getOptimizedLineData(&lineData, begin, end);
  
  if (mKeyAxis->rangeReversed() != (mKeyAxis->orientation() == Qt::Vertical)) // make sure key pixels are sorted ascending in lineData (significantly simplifies following processing)
    std::reverse(lineData.begin(), lineData.end());

  switch (mLineStyle)
  {
    case lsNone: lines->clear(); break;
    case lsLine: *lines = dataToLines(lineData); break;
    case lsStepLeft: *lines = dataToStepLeftLines(lineData); break;
    case lsStepRight: *lines = dataToStepRightLines(lineData); break;
    case lsStepCenter: *lines = dataToStepCenterLines(lineData); break;
    case lsImpulse: *lines = dataToImpulseLines(lineData); break;
  }
}

/*! \internal

  This method retrieves an optimized set of data points via \ref getOptimizedScatterData and then
  converts them to pixel coordinates. The resulting points are returned in \a scatters, and can be
  passed to \ref drawScatterPlot.

  \a dataRange specifies the beginning and ending data indices that will be taken into account for
  conversion. In this function, the specified range may exceed the total data bounds without harm:
  a correspondingly trimmed data range will be used. This takes the burden off the user of this
  function to check for valid indices in \a dataRange, e.g. when extending ranges coming from \ref
  getDataSegments.
*/
void QCPGraph::getScatters(QVector<QPointF> *scatters, const QCPDataRange &dataRange) const
{
  if (!scatters) return;
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; scatters->clear(); return; }
  
  QCPGraphDataContainer::const_iterator begin, end;
  getVisibleDataBounds(begin, end, dataRange);
  if (begin == end)
  {
    scatters->clear();
    return;
  }
  
  QVector<QCPGraphData> data;
  getOptimizedScatterData(&data, begin, end);
  
  if (mKeyAxis->rangeReversed() != (mKeyAxis->orientation() == Qt::Vertical)) // make sure key pixels are sorted ascending in data (significantly simplifies following processing)
    std::reverse(data.begin(), data.end());
  
  scatters->resize(data.size());
  if (keyAxis->orientation() == Qt::Vertical)
  {
    for (int i=0; i<data.size(); ++i)
    {
      if (!qIsNaN(data.at(i).value))
      {
        (*scatters)[i].setX(valueAxis->coordToPixel(data.at(i).value));
        (*scatters)[i].setY(keyAxis->coordToPixel(data.at(i).key));
      }
    }
  } else
  {
    for (int i=0; i<data.size(); ++i)
    {
      if (!qIsNaN(data.at(i).value))
      {
        (*scatters)[i].setX(keyAxis->coordToPixel(data.at(i).key));
        (*scatters)[i].setY(valueAxis->coordToPixel(data.at(i).value));
      }
    }
  }
}

/*! \internal

  Takes raw data points in plot coordinates as \a data, and returns a vector containing pixel
  coordinate points which are suitable for drawing the line style \ref lsLine.
  
  The source of \a data is usually \ref getOptimizedLineData, and this method is called in \a
  getLines if the line style is set accordingly.

  \see dataToStepLeftLines, dataToStepRightLines, dataToStepCenterLines, dataToImpulseLines, getLines, drawLinePlot
*/
QVector<QPointF> QCPGraph::dataToLines(const QVector<QCPGraphData> &data) const
{
  QVector<QPointF> result;
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return result; }

  result.resize(data.size());
  
  // transform data points to pixels:
  if (keyAxis->orientation() == Qt::Vertical)
  {
    for (int i=0; i<data.size(); ++i)
    {
      result[i].setX(valueAxis->coordToPixel(data.at(i).value));
      result[i].setY(keyAxis->coordToPixel(data.at(i).key));
    }
  } else // key axis is horizontal
  {
    for (int i=0; i<data.size(); ++i)
    {
      result[i].setX(keyAxis->coordToPixel(data.at(i).key));
      result[i].setY(valueAxis->coordToPixel(data.at(i).value));
    }
  }
  return result;
}

/*! \internal

  Takes raw data points in plot coordinates as \a data, and returns a vector containing pixel
  coordinate points which are suitable for drawing the line style \ref lsStepLeft.
  
  The source of \a data is usually \ref getOptimizedLineData, and this method is called in \a
  getLines if the line style is set accordingly.

  \see dataToLines, dataToStepRightLines, dataToStepCenterLines, dataToImpulseLines, getLines, drawLinePlot
*/
QVector<QPointF> QCPGraph::dataToStepLeftLines(const QVector<QCPGraphData> &data) const
{
  QVector<QPointF> result;
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return result; }
  
  result.resize(data.size()*2);
  
  // calculate steps from data and transform to pixel coordinates:
  if (keyAxis->orientation() == Qt::Vertical)
  {
    double lastValue = valueAxis->coordToPixel(data.first().value);
    for (int i=0; i<data.size(); ++i)
    {
      const double key = keyAxis->coordToPixel(data.at(i).key);
      result[i*2+0].setX(lastValue);
      result[i*2+0].setY(key);
      lastValue = valueAxis->coordToPixel(data.at(i).value);
      result[i*2+1].setX(lastValue);
      result[i*2+1].setY(key);
    }
  } else // key axis is horizontal
  {
    double lastValue = valueAxis->coordToPixel(data.first().value);
    for (int i=0; i<data.size(); ++i)
    {
      const double key = keyAxis->coordToPixel(data.at(i).key);
      result[i*2+0].setX(key);
      result[i*2+0].setY(lastValue);
      lastValue = valueAxis->coordToPixel(data.at(i).value);
      result[i*2+1].setX(key);
      result[i*2+1].setY(lastValue);
    }
  }
  return result;
}

/*! \internal

  Takes raw data points in plot coordinates as \a data, and returns a vector containing pixel
  coordinate points which are suitable for drawing the line style \ref lsStepRight.
  
  The source of \a data is usually \ref getOptimizedLineData, and this method is called in \a
  getLines if the line style is set accordingly.

  \see dataToLines, dataToStepLeftLines, dataToStepCenterLines, dataToImpulseLines, getLines, drawLinePlot
*/
QVector<QPointF> QCPGraph::dataToStepRightLines(const QVector<QCPGraphData> &data) const
{
  QVector<QPointF> result;
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return result; }
  
  result.resize(data.size()*2);
  
  // calculate steps from data and transform to pixel coordinates:
  if (keyAxis->orientation() == Qt::Vertical)
  {
    double lastKey = keyAxis->coordToPixel(data.first().key);
    for (int i=0; i<data.size(); ++i)
    {
      const double value = valueAxis->coordToPixel(data.at(i).value);
      result[i*2+0].setX(value);
      result[i*2+0].setY(lastKey);
      lastKey = keyAxis->coordToPixel(data.at(i).key);
      result[i*2+1].setX(value);
      result[i*2+1].setY(lastKey);
    }
  } else // key axis is horizontal
  {
    double lastKey = keyAxis->coordToPixel(data.first().key);
    for (int i=0; i<data.size(); ++i)
    {
      const double value = valueAxis->coordToPixel(data.at(i).value);
      result[i*2+0].setX(lastKey);
      result[i*2+0].setY(value);
      lastKey = keyAxis->coordToPixel(data.at(i).key);
      result[i*2+1].setX(lastKey);
      result[i*2+1].setY(value);
    }
  }
  return result;
}

/*! \internal

  Takes raw data points in plot coordinates as \a data, and returns a vector containing pixel
  coordinate points which are suitable for drawing the line style \ref lsStepCenter.
  
  The source of \a data is usually \ref getOptimizedLineData, and this method is called in \a
  getLines if the line style is set accordingly.

  \see dataToLines, dataToStepLeftLines, dataToStepRightLines, dataToImpulseLines, getLines, drawLinePlot
*/
QVector<QPointF> QCPGraph::dataToStepCenterLines(const QVector<QCPGraphData> &data) const
{
  QVector<QPointF> result;
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return result; }
  
  result.resize(data.size()*2);
  
  // calculate steps from data and transform to pixel coordinates:
  if (keyAxis->orientation() == Qt::Vertical)
  {
    double lastKey = keyAxis->coordToPixel(data.first().key);
    double lastValue = valueAxis->coordToPixel(data.first().value);
    result[0].setX(lastValue);
    result[0].setY(lastKey);
    for (int i=1; i<data.size(); ++i)
    {
      const double key = (keyAxis->coordToPixel(data.at(i).key)+lastKey)*0.5;
      result[i*2-1].setX(lastValue);
      result[i*2-1].setY(key);
      lastValue = valueAxis->coordToPixel(data.at(i).value);
      lastKey = keyAxis->coordToPixel(data.at(i).key);
      result[i*2+0].setX(lastValue);
      result[i*2+0].setY(key);
    }
    result[data.size()*2-1].setX(lastValue);
    result[data.size()*2-1].setY(lastKey);
  } else // key axis is horizontal
  {
    double lastKey = keyAxis->coordToPixel(data.first().key);
    double lastValue = valueAxis->coordToPixel(data.first().value);
    result[0].setX(lastKey);
    result[0].setY(lastValue);
    for (int i=1; i<data.size(); ++i)
    {
      const double key = (keyAxis->coordToPixel(data.at(i).key)+lastKey)*0.5;
      result[i*2-1].setX(key);
      result[i*2-1].setY(lastValue);
      lastValue = valueAxis->coordToPixel(data.at(i).value);
      lastKey = keyAxis->coordToPixel(data.at(i).key);
      result[i*2+0].setX(key);
      result[i*2+0].setY(lastValue);
    }
    result[data.size()*2-1].setX(lastKey);
    result[data.size()*2-1].setY(lastValue);
  }
  return result;
}

/*! \internal

  Takes raw data points in plot coordinates as \a data, and returns a vector containing pixel
  coordinate points which are suitable for drawing the line style \ref lsImpulse.
  
  The source of \a data is usually \ref getOptimizedLineData, and this method is called in \a
  getLines if the line style is set accordingly.

  \see dataToLines, dataToStepLeftLines, dataToStepRightLines, dataToStepCenterLines, getLines, drawImpulsePlot
*/
QVector<QPointF> QCPGraph::dataToImpulseLines(const QVector<QCPGraphData> &data) const
{
  QVector<QPointF> result;
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return result; }
  
  result.resize(data.size()*2);
  
  // transform data points to pixels:
  if (keyAxis->orientation() == Qt::Vertical)
  {
    for (int i=0; i<data.size(); ++i)
    {
      const QCPGraphData &current = data.at(i);
      if (!qIsNaN(current.value))
      {
        const double key = keyAxis->coordToPixel(current.key);
        result[i*2+0].setX(valueAxis->coordToPixel(0));
        result[i*2+0].setY(key);
        result[i*2+1].setX(valueAxis->coordToPixel(current.value));
        result[i*2+1].setY(key);
      } else
      {
        result[i*2+0] = QPointF(0, 0);
        result[i*2+1] = QPointF(0, 0);
      }
    }
  } else // key axis is horizontal
  {
    for (int i=0; i<data.size(); ++i)
    {
      const QCPGraphData &current = data.at(i);
      if (!qIsNaN(current.value))
      {
        const double key = keyAxis->coordToPixel(data.at(i).key);
        result[i*2+0].setX(key);
        result[i*2+0].setY(valueAxis->coordToPixel(0));
        result[i*2+1].setX(key);
        result[i*2+1].setY(valueAxis->coordToPixel(data.at(i).value));
      } else
      {
        result[i*2+0] = QPointF(0, 0);
        result[i*2+1] = QPointF(0, 0);
      }
    }
  }
  return result;
}

/*! \internal
  
  Draws the fill of the graph using the specified \a painter, with the currently set brush.
  
  Depending on whether a normal fill or a channel fill (\ref setChannelFillGraph) is needed, \ref
  getFillPolygon or \ref getChannelFillPolygon are used to find the according fill polygons.
  
  In order to handle NaN Data points correctly (the fill needs to be split into disjoint areas),
  this method first determines a list of non-NaN segments with \ref getNonNanSegments, on which to
  operate. In the channel fill case, \ref getOverlappingSegments is used to consolidate the non-NaN
  segments of the two involved graphs, before passing the overlapping pairs to \ref
  getChannelFillPolygon.
  
  Pass the points of this graph's line as \a lines, in pixel coordinates.

  \see drawLinePlot, drawImpulsePlot, drawScatterPlot
*/
void QCPGraph::drawFill(QCPPainter *painter, QVector<QPointF> *lines) const
{
  if (mLineStyle == lsImpulse) return; // fill doesn't make sense for impulse plot
  if (painter->brush().style() == Qt::NoBrush || painter->brush().color().alpha() == 0) return;
  
  applyFillAntialiasingHint(painter);
  const QVector<QCPDataRange> segments = getNonNanSegments(lines, keyAxis()->orientation());
  if (!mChannelFillGraph)
  {
    // draw base fill under graph, fill goes all the way to the zero-value-line:
    foreach (QCPDataRange segment, segments)
      painter->drawPolygon(getFillPolygon(lines, segment));
  } else
  {
    // draw fill between this graph and mChannelFillGraph:
    QVector<QPointF> otherLines;
    mChannelFillGraph->getLines(&otherLines, QCPDataRange(0, mChannelFillGraph->dataCount()));
    if (!otherLines.isEmpty())
    {
      QVector<QCPDataRange> otherSegments = getNonNanSegments(&otherLines, mChannelFillGraph->keyAxis()->orientation());
      QVector<QPair<QCPDataRange, QCPDataRange> > segmentPairs = getOverlappingSegments(segments, lines, otherSegments, &otherLines);
      for (int i=0; i<segmentPairs.size(); ++i)
        painter->drawPolygon(getChannelFillPolygon(lines, segmentPairs.at(i).first, &otherLines, segmentPairs.at(i).second));
    }
  }
}

/*! \internal

  Draws scatter symbols at every point passed in \a scatters, given in pixel coordinates. The
  scatters will be drawn with \a painter and have the appearance as specified in \a style.

  \see drawLinePlot, drawImpulsePlot
*/
void QCPGraph::drawScatterPlot(QCPPainter *painter, const QVector<QPointF> &scatters, const QCPScatterStyle &style) const
{
  applyScattersAntialiasingHint(painter);
  style.applyTo(painter, mPen);
  foreach (const QPointF &scatter, scatters)
    style.drawShape(painter, scatter.x(), scatter.y());
}

/*!  \internal
  
  Draws lines between the points in \a lines, given in pixel coordinates.
  
  \see drawScatterPlot, drawImpulsePlot, QCPAbstractPlottable1D::drawPolyline
*/
void QCPGraph::drawLinePlot(QCPPainter *painter, const QVector<QPointF> &lines) const
{
  if (painter->pen().style() != Qt::NoPen && painter->pen().color().alpha() != 0)
  {
    applyDefaultAntialiasingHint(painter);
    drawPolyline(painter, lines);
  }
}

/*! \internal

  Draws impulses from the provided data, i.e. it connects all line pairs in \a lines, given in
  pixel coordinates. The \a lines necessary for impulses are generated by \ref dataToImpulseLines
  from the regular graph data points.

  \see drawLinePlot, drawScatterPlot
*/
void QCPGraph::drawImpulsePlot(QCPPainter *painter, const QVector<QPointF> &lines) const
{
  if (painter->pen().style() != Qt::NoPen && painter->pen().color().alpha() != 0)
  {
    applyDefaultAntialiasingHint(painter);
    QPen oldPen = painter->pen();
    QPen newPen = painter->pen();
    newPen.setCapStyle(Qt::FlatCap); // so impulse line doesn't reach beyond zero-line
    painter->setPen(newPen);
    painter->drawLines(lines);
    painter->setPen(oldPen);
  }
}

/*! \internal

  Returns via \a lineData the data points that need to be visualized for this graph when plotting
  graph lines, taking into consideration the currently visible axis ranges and, if \ref
  setAdaptiveSampling is enabled, local point densities. The considered data can be restricted
  further by \a begin and \a end, e.g. to only plot a certain segment of the data (see \ref
  getDataSegments).

  This method is used by \ref getLines to retrieve the basic working set of data.

  \see getOptimizedScatterData
*/
void QCPGraph::getOptimizedLineData(QVector<QCPGraphData> *lineData, const QCPGraphDataContainer::const_iterator &begin, const QCPGraphDataContainer::const_iterator &end) const
{
  if (!lineData) return;
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  if (begin == end) return;
  
  int dataCount = int(end-begin);
  int maxCount = (std::numeric_limits<int>::max)();
  if (mAdaptiveSampling)
  {
    double keyPixelSpan = qAbs(keyAxis->coordToPixel(begin->key)-keyAxis->coordToPixel((end-1)->key));
    if (2*keyPixelSpan+2 < static_cast<double>((std::numeric_limits<int>::max)()))
      maxCount = int(2*keyPixelSpan+2);
  }
  
  if (mAdaptiveSampling && dataCount >= maxCount) // use adaptive sampling only if there are at least two points per pixel on average
  {
    QCPGraphDataContainer::const_iterator it = begin;
    double minValue = it->value;
    double maxValue = it->value;
    QCPGraphDataContainer::const_iterator currentIntervalFirstPoint = it;
    int reversedFactor = keyAxis->pixelOrientation(); // is used to calculate keyEpsilon pixel into the correct direction
    int reversedRound = reversedFactor==-1 ? 1 : 0; // is used to switch between floor (normal) and ceil (reversed) rounding of currentIntervalStartKey
    double currentIntervalStartKey = keyAxis->pixelToCoord(int(keyAxis->coordToPixel(begin->key)+reversedRound));
    double lastIntervalEndKey = currentIntervalStartKey;
    double keyEpsilon = qAbs(currentIntervalStartKey-keyAxis->pixelToCoord(keyAxis->coordToPixel(currentIntervalStartKey)+1.0*reversedFactor)); // interval of one pixel on screen when mapped to plot key coordinates
    bool keyEpsilonVariable = keyAxis->scaleType() == QCPAxis::stLogarithmic; // indicates whether keyEpsilon needs to be updated after every interval (for log axes)
    int intervalDataCount = 1;
    ++it; // advance iterator to second data point because adaptive sampling works in 1 point retrospect
    while (it != end)
    {
      if (it->key < currentIntervalStartKey+keyEpsilon) // data point is still within same pixel, so skip it and expand value span of this cluster if necessary
      {
        if (it->value < minValue)
          minValue = it->value;
        else if (it->value > maxValue)
          maxValue = it->value;
        ++intervalDataCount;
      } else // new pixel interval started
      {
        if (intervalDataCount >= 2) // last pixel had multiple data points, consolidate them to a cluster
        {
          if (lastIntervalEndKey < currentIntervalStartKey-keyEpsilon) // last point is further away, so first point of this cluster must be at a real data point
            lineData->append(QCPGraphData(currentIntervalStartKey+keyEpsilon*0.2, currentIntervalFirstPoint->value));
          lineData->append(QCPGraphData(currentIntervalStartKey+keyEpsilon*0.25, minValue));
          lineData->append(QCPGraphData(currentIntervalStartKey+keyEpsilon*0.75, maxValue));
          if (it->key > currentIntervalStartKey+keyEpsilon*2) // new pixel started further away from previous cluster, so make sure the last point of the cluster is at a real data point
            lineData->append(QCPGraphData(currentIntervalStartKey+keyEpsilon*0.8, (it-1)->value));
        } else
          lineData->append(QCPGraphData(currentIntervalFirstPoint->key, currentIntervalFirstPoint->value));
        lastIntervalEndKey = (it-1)->key;
        minValue = it->value;
        maxValue = it->value;
        currentIntervalFirstPoint = it;
        currentIntervalStartKey = keyAxis->pixelToCoord(int(keyAxis->coordToPixel(it->key)+reversedRound));
        if (keyEpsilonVariable)
          keyEpsilon = qAbs(currentIntervalStartKey-keyAxis->pixelToCoord(keyAxis->coordToPixel(currentIntervalStartKey)+1.0*reversedFactor));
        intervalDataCount = 1;
      }
      ++it;
    }
    // handle last interval:
    if (intervalDataCount >= 2) // last pixel had multiple data points, consolidate them to a cluster
    {
      if (lastIntervalEndKey < currentIntervalStartKey-keyEpsilon) // last point wasn't a cluster, so first point of this cluster must be at a real data point
        lineData->append(QCPGraphData(currentIntervalStartKey+keyEpsilon*0.2, currentIntervalFirstPoint->value));
      lineData->append(QCPGraphData(currentIntervalStartKey+keyEpsilon*0.25, minValue));
      lineData->append(QCPGraphData(currentIntervalStartKey+keyEpsilon*0.75, maxValue));
    } else
      lineData->append(QCPGraphData(currentIntervalFirstPoint->key, currentIntervalFirstPoint->value));
    
  } else // don't use adaptive sampling algorithm, transfer points one-to-one from the data container into the output
  {
    lineData->resize(dataCount);
    std::copy(begin, end, lineData->begin());
  }
}

/*! \internal

  Returns via \a scatterData the data points that need to be visualized for this graph when
  plotting scatter points, taking into consideration the currently visible axis ranges and, if \ref
  setAdaptiveSampling is enabled, local point densities. The considered data can be restricted
  further by \a begin and \a end, e.g. to only plot a certain segment of the data (see \ref
  getDataSegments).

  This method is used by \ref getScatters to retrieve the basic working set of data.

  \see getOptimizedLineData
*/
void QCPGraph::getOptimizedScatterData(QVector<QCPGraphData> *scatterData, QCPGraphDataContainer::const_iterator begin, QCPGraphDataContainer::const_iterator end) const
{
  if (!scatterData) return;
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  const int scatterModulo = mScatterSkip+1;
  const bool doScatterSkip = mScatterSkip > 0;
  int beginIndex = int(begin-mDataContainer->constBegin());
  int endIndex = int(end-mDataContainer->constBegin());
  while (doScatterSkip && begin != end && beginIndex % scatterModulo != 0) // advance begin iterator to first non-skipped scatter
  {
    ++beginIndex;
    ++begin;
  }
  if (begin == end) return;
  int dataCount = int(end-begin);
  int maxCount = (std::numeric_limits<int>::max)();
  if (mAdaptiveSampling)
  {
    int keyPixelSpan = int(qAbs(keyAxis->coordToPixel(begin->key)-keyAxis->coordToPixel((end-1)->key)));
    maxCount = 2*keyPixelSpan+2;
  }
  
  if (mAdaptiveSampling && dataCount >= maxCount) // use adaptive sampling only if there are at least two points per pixel on average
  {
    double valueMaxRange = valueAxis->range().upper;
    double valueMinRange = valueAxis->range().lower;
    QCPGraphDataContainer::const_iterator it = begin;
    int itIndex = int(beginIndex);
    double minValue = it->value;
    double maxValue = it->value;
    QCPGraphDataContainer::const_iterator minValueIt = it;
    QCPGraphDataContainer::const_iterator maxValueIt = it;
    QCPGraphDataContainer::const_iterator currentIntervalStart = it;
    int reversedFactor = keyAxis->pixelOrientation(); // is used to calculate keyEpsilon pixel into the correct direction
    int reversedRound = reversedFactor==-1 ? 1 : 0; // is used to switch between floor (normal) and ceil (reversed) rounding of currentIntervalStartKey
    double currentIntervalStartKey = keyAxis->pixelToCoord(int(keyAxis->coordToPixel(begin->key)+reversedRound));
    double keyEpsilon = qAbs(currentIntervalStartKey-keyAxis->pixelToCoord(keyAxis->coordToPixel(currentIntervalStartKey)+1.0*reversedFactor)); // interval of one pixel on screen when mapped to plot key coordinates
    bool keyEpsilonVariable = keyAxis->scaleType() == QCPAxis::stLogarithmic; // indicates whether keyEpsilon needs to be updated after every interval (for log axes)
    int intervalDataCount = 1;
    // advance iterator to second (non-skipped) data point because adaptive sampling works in 1 point retrospect:
    if (!doScatterSkip)
      ++it;
    else
    {
      itIndex += scatterModulo;
      if (itIndex < endIndex) // make sure we didn't jump over end
        it += scatterModulo;
      else
      {
        it = end;
        itIndex = endIndex;
      }
    }
    // main loop over data points:
    while (it != end)
    {
      if (it->key < currentIntervalStartKey+keyEpsilon) // data point is still within same pixel, so skip it and expand value span of this pixel if necessary
      {
        if (it->value < minValue && it->value > valueMinRange && it->value < valueMaxRange)
        {
          minValue = it->value;
          minValueIt = it;
        } else if (it->value > maxValue && it->value > valueMinRange && it->value < valueMaxRange)
        {
          maxValue = it->value;
          maxValueIt = it;
        }
        ++intervalDataCount;
      } else // new pixel started
      {
        if (intervalDataCount >= 2) // last pixel had multiple data points, consolidate them
        {
          // determine value pixel span and add as many points in interval to maintain certain vertical data density (this is specific to scatter plot):
          double valuePixelSpan = qAbs(valueAxis->coordToPixel(minValue)-valueAxis->coordToPixel(maxValue));
          int dataModulo = qMax(1, qRound(intervalDataCount/(valuePixelSpan/4.0))); // approximately every 4 value pixels one data point on average
          QCPGraphDataContainer::const_iterator intervalIt = currentIntervalStart;
          int c = 0;
          while (intervalIt != it)
          {
            if ((c % dataModulo == 0 || intervalIt == minValueIt || intervalIt == maxValueIt) && intervalIt->value > valueMinRange && intervalIt->value < valueMaxRange)
              scatterData->append(*intervalIt);
            ++c;
            if (!doScatterSkip)
              ++intervalIt;
            else
              intervalIt += scatterModulo; // since we know indices of "currentIntervalStart", "intervalIt" and "it" are multiples of scatterModulo, we can't accidentally jump over "it" here
          }
        } else if (currentIntervalStart->value > valueMinRange && currentIntervalStart->value < valueMaxRange)
          scatterData->append(*currentIntervalStart);
        minValue = it->value;
        maxValue = it->value;
        currentIntervalStart = it;
        currentIntervalStartKey = keyAxis->pixelToCoord(int(keyAxis->coordToPixel(it->key)+reversedRound));
        if (keyEpsilonVariable)
          keyEpsilon = qAbs(currentIntervalStartKey-keyAxis->pixelToCoord(keyAxis->coordToPixel(currentIntervalStartKey)+1.0*reversedFactor));
        intervalDataCount = 1;
      }
      // advance to next data point:
      if (!doScatterSkip)
        ++it;
      else
      {
        itIndex += scatterModulo;
        if (itIndex < endIndex) // make sure we didn't jump over end
          it += scatterModulo;
        else
        {
          it = end;
          itIndex = endIndex;
        }
      }
    }
    // handle last interval:
    if (intervalDataCount >= 2) // last pixel had multiple data points, consolidate them
    {
      // determine value pixel span and add as many points in interval to maintain certain vertical data density (this is specific to scatter plot):
      double valuePixelSpan = qAbs(valueAxis->coordToPixel(minValue)-valueAxis->coordToPixel(maxValue));
      int dataModulo = qMax(1, qRound(intervalDataCount/(valuePixelSpan/4.0))); // approximately every 4 value pixels one data point on average
      QCPGraphDataContainer::const_iterator intervalIt = currentIntervalStart;
      int intervalItIndex = int(intervalIt-mDataContainer->constBegin());
      int c = 0;
      while (intervalIt != it)
      {
        if ((c % dataModulo == 0 || intervalIt == minValueIt || intervalIt == maxValueIt) && intervalIt->value > valueMinRange && intervalIt->value < valueMaxRange)
          scatterData->append(*intervalIt);
        ++c;
        if (!doScatterSkip)
          ++intervalIt;
        else // here we can't guarantee that adding scatterModulo doesn't exceed "it" (because "it" is equal to "end" here, and "end" isn't scatterModulo-aligned), so check via index comparison:
        {
          intervalItIndex += scatterModulo;
          if (intervalItIndex < itIndex)
            intervalIt += scatterModulo;
          else
          {
            intervalIt = it;
            intervalItIndex = itIndex;
          }
        }
      }
    } else if (currentIntervalStart->value > valueMinRange && currentIntervalStart->value < valueMaxRange)
      scatterData->append(*currentIntervalStart);
    
  } else // don't use adaptive sampling algorithm, transfer points one-to-one from the data container into the output
  {
    QCPGraphDataContainer::const_iterator it = begin;
    int itIndex = beginIndex;
    scatterData->reserve(dataCount);
    while (it != end)
    {
      scatterData->append(*it);
      // advance to next data point:
      if (!doScatterSkip)
        ++it;
      else
      {
        itIndex += scatterModulo;
        if (itIndex < endIndex)
          it += scatterModulo;
        else
        {
          it = end;
          itIndex = endIndex;
        }
      }
    }
  }
}

/*!
  This method outputs the currently visible data range via \a begin and \a end. The returned range
  will also never exceed \a rangeRestriction.

  This method takes into account that the drawing of data lines at the axis rect border always
  requires the points just outside the visible axis range. So \a begin and \a end may actually
  indicate a range that contains one additional data point to the left and right of the visible
  axis range.
*/
void QCPGraph::getVisibleDataBounds(QCPGraphDataContainer::const_iterator &begin, QCPGraphDataContainer::const_iterator &end, const QCPDataRange &rangeRestriction) const
{
  if (rangeRestriction.isEmpty())
  {
    end = mDataContainer->constEnd();
    begin = end;
  } else
  {
    QCPAxis *keyAxis = mKeyAxis.data();
    QCPAxis *valueAxis = mValueAxis.data();
    if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
    // get visible data range:
    begin = mDataContainer->findBegin(keyAxis->range().lower);
    end = mDataContainer->findEnd(keyAxis->range().upper);
    // limit lower/upperEnd to rangeRestriction:
    mDataContainer->limitIteratorsToDataRange(begin, end, rangeRestriction); // this also ensures rangeRestriction outside data bounds doesn't break anything
  }
}

/*!  \internal
  
  This method goes through the passed points in \a lineData and returns a list of the segments
  which don't contain NaN data points.
  
  \a keyOrientation defines whether the \a x or \a y member of the passed QPointF is used to check
  for NaN. If \a keyOrientation is \c Qt::Horizontal, the \a y member is checked, if it is \c
  Qt::Vertical, the \a x member is checked.
  
  \see getOverlappingSegments, drawFill
*/
QVector<QCPDataRange> QCPGraph::getNonNanSegments(const QVector<QPointF> *lineData, Qt::Orientation keyOrientation) const
{
  QVector<QCPDataRange> result;
  const int n = lineData->size();
  
  QCPDataRange currentSegment(-1, -1);
  int i = 0;
  
  if (keyOrientation == Qt::Horizontal)
  {
    while (i < n)
    {
      while (i < n && qIsNaN(lineData->at(i).y())) // seek next non-NaN data point
        ++i;
      if (i == n)
        break;
      currentSegment.setBegin(i++);
      while (i < n && !qIsNaN(lineData->at(i).y())) // seek next NaN data point or end of data
        ++i;
      currentSegment.setEnd(i++);
      result.append(currentSegment);
    }
  } else // keyOrientation == Qt::Vertical
  {
    while (i < n)
    {
      while (i < n && qIsNaN(lineData->at(i).x())) // seek next non-NaN data point
        ++i;
      if (i == n)
        break;
      currentSegment.setBegin(i++);
      while (i < n && !qIsNaN(lineData->at(i).x())) // seek next NaN data point or end of data
        ++i;
      currentSegment.setEnd(i++);
      result.append(currentSegment);
    }
  }
  return result;
}

/*!  \internal
  
  This method takes two segment lists (e.g. created by \ref getNonNanSegments) \a thisSegments and
  \a otherSegments, and their associated point data \a thisData and \a otherData.

  It returns all pairs of segments (the first from \a thisSegments, the second from \a
  otherSegments), which overlap in plot coordinates.
  
  This method is useful in the case of a channel fill between two graphs, when only those non-NaN
  segments which actually overlap in their key coordinate shall be considered for drawing a channel
  fill polygon.
  
  It is assumed that the passed segments in \a thisSegments are ordered ascending by index, and
  that the segments don't overlap themselves. The same is assumed for the segments in \a
  otherSegments. This is fulfilled when the segments are obtained via \ref getNonNanSegments.
  
  \see getNonNanSegments, segmentsIntersect, drawFill, getChannelFillPolygon
*/
QVector<QPair<QCPDataRange, QCPDataRange> > QCPGraph::getOverlappingSegments(QVector<QCPDataRange> thisSegments, const QVector<QPointF> *thisData, QVector<QCPDataRange> otherSegments, const QVector<QPointF> *otherData) const
{
  QVector<QPair<QCPDataRange, QCPDataRange> > result;
  if (thisData->isEmpty() || otherData->isEmpty() || thisSegments.isEmpty() || otherSegments.isEmpty())
    return result;
  
  int thisIndex = 0;
  int otherIndex = 0;
  const bool verticalKey = mKeyAxis->orientation() == Qt::Vertical;
  while (thisIndex < thisSegments.size() && otherIndex < otherSegments.size())
  {
    if (thisSegments.at(thisIndex).size() < 2) // segments with fewer than two points won't have a fill anyhow
    {
      ++thisIndex;
      continue;
    }
    if (otherSegments.at(otherIndex).size() < 2) // segments with fewer than two points won't have a fill anyhow
    {
      ++otherIndex;
      continue;
    }
    double thisLower, thisUpper, otherLower, otherUpper;
    if (!verticalKey)
    {
      thisLower = thisData->at(thisSegments.at(thisIndex).begin()).x();
      thisUpper = thisData->at(thisSegments.at(thisIndex).end()-1).x();
      otherLower = otherData->at(otherSegments.at(otherIndex).begin()).x();
      otherUpper = otherData->at(otherSegments.at(otherIndex).end()-1).x();
    } else
    {
      thisLower = thisData->at(thisSegments.at(thisIndex).begin()).y();
      thisUpper = thisData->at(thisSegments.at(thisIndex).end()-1).y();
      otherLower = otherData->at(otherSegments.at(otherIndex).begin()).y();
      otherUpper = otherData->at(otherSegments.at(otherIndex).end()-1).y();
    }
    
    int bPrecedence;
    if (segmentsIntersect(thisLower, thisUpper, otherLower, otherUpper, bPrecedence))
      result.append(QPair<QCPDataRange, QCPDataRange>(thisSegments.at(thisIndex), otherSegments.at(otherIndex)));
    
    if (bPrecedence <= 0) // otherSegment doesn't reach as far as thisSegment, so continue with next otherSegment, keeping current thisSegment
      ++otherIndex;
    else // otherSegment reaches further than thisSegment, so continue with next thisSegment, keeping current otherSegment
      ++thisIndex;
  }
  
  return result;
}

/*!  \internal
  
  Returns whether the segments defined by the coordinates (aLower, aUpper) and (bLower, bUpper)
  have overlap.
  
  The output parameter \a bPrecedence indicates whether the \a b segment reaches farther than the
  \a a segment or not. If \a bPrecedence returns 1, segment \a b reaches the farthest to higher
  coordinates (i.e. bUpper > aUpper). If it returns -1, segment \a a reaches the farthest. Only if
  both segment's upper bounds are identical, 0 is returned as \a bPrecedence.
  
  It is assumed that the lower bounds always have smaller or equal values than the upper bounds.
  
  \see getOverlappingSegments
*/
bool QCPGraph::segmentsIntersect(double aLower, double aUpper, double bLower, double bUpper, int &bPrecedence) const
{
  bPrecedence = 0;
  if (aLower > bUpper)
  {
    bPrecedence = -1;
    return false;
  } else if (bLower > aUpper)
  {
    bPrecedence = 1;
    return false;
  } else
  {
    if (aUpper > bUpper)
      bPrecedence = -1;
    else if (aUpper < bUpper)
      bPrecedence = 1;
    
    return true;
  }
}

/*! \internal
  
  Returns the point which closes the fill polygon on the zero-value-line parallel to the key axis.
  The logarithmic axis scale case is a bit special, since the zero-value-line in pixel coordinates
  is in positive or negative infinity. So this case is handled separately by just closing the fill
  polygon on the axis which lies in the direction towards the zero value.

  \a matchingDataPoint will provide the key (in pixels) of the returned point. Depending on whether
  the key axis of this graph is horizontal or vertical, \a matchingDataPoint will provide the x or
  y value of the returned point, respectively.
*/
QPointF QCPGraph::getFillBasePoint(QPointF matchingDataPoint) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return {}; }
  
  QPointF result;
  if (valueAxis->scaleType() == QCPAxis::stLinear)
  {
    if (keyAxis->orientation() == Qt::Horizontal)
    {
      result.setX(matchingDataPoint.x());
      result.setY(valueAxis->coordToPixel(0));
    } else // keyAxis->orientation() == Qt::Vertical
    {
      result.setX(valueAxis->coordToPixel(0));
      result.setY(matchingDataPoint.y());
    }
  } else // valueAxis->mScaleType == QCPAxis::stLogarithmic
  {
    // In logarithmic scaling we can't just draw to value 0 so we just fill all the way
    // to the axis which is in the direction towards 0
    if (keyAxis->orientation() == Qt::Vertical)
    {
      if ((valueAxis->range().upper < 0 && !valueAxis->rangeReversed()) ||
          (valueAxis->range().upper > 0 && valueAxis->rangeReversed())) // if range is negative, zero is on opposite side of key axis
        result.setX(keyAxis->axisRect()->right());
      else
        result.setX(keyAxis->axisRect()->left());
      result.setY(matchingDataPoint.y());
    } else if (keyAxis->axisType() == QCPAxis::atTop || keyAxis->axisType() == QCPAxis::atBottom)
    {
      result.setX(matchingDataPoint.x());
      if ((valueAxis->range().upper < 0 && !valueAxis->rangeReversed()) ||
          (valueAxis->range().upper > 0 && valueAxis->rangeReversed())) // if range is negative, zero is on opposite side of key axis
        result.setY(keyAxis->axisRect()->top());
      else
        result.setY(keyAxis->axisRect()->bottom());
    }
  }
  return result;
}

/*! \internal
  
  Returns the polygon needed for drawing normal fills between this graph and the key axis.
  
  Pass the graph's data points (in pixel coordinates) as \a lineData, and specify the \a segment
  which shall be used for the fill. The collection of \a lineData points described by \a segment
  must not contain NaN data points (see \ref getNonNanSegments).
  
  The returned fill polygon will be closed at the key axis (the zero-value line) for linear value
  axes. For logarithmic value axes the polygon will reach just beyond the corresponding axis rect
  side (see \ref getFillBasePoint).

  For increased performance (due to implicit sharing), keep the returned QPolygonF const.
  
  \see drawFill, getNonNanSegments
*/
const QPolygonF QCPGraph::getFillPolygon(const QVector<QPointF> *lineData, QCPDataRange segment) const
{
  if (segment.size() < 2)
    return QPolygonF();
  QPolygonF result(segment.size()+2);
  
  result[0] = getFillBasePoint(lineData->at(segment.begin()));
  std::copy(lineData->constBegin()+segment.begin(), lineData->constBegin()+segment.end(), result.begin()+1);
  result[result.size()-1] = getFillBasePoint(lineData->at(segment.end()-1));
  
  return result;
}

/*! \internal
  
  Returns the polygon needed for drawing (partial) channel fills between this graph and the graph
  specified by \ref setChannelFillGraph.
  
  The data points of this graph are passed as pixel coordinates via \a thisData, the data of the
  other graph as \a otherData. The returned polygon will be calculated for the specified data
  segments \a thisSegment and \a otherSegment, pertaining to the respective \a thisData and \a
  otherData, respectively.
  
  The passed \a thisSegment and \a otherSegment should correspond to the segment pairs returned by
  \ref getOverlappingSegments, to make sure only segments that actually have key coordinate overlap
  need to be processed here.
  
  For increased performance due to implicit sharing, keep the returned QPolygonF const.
  
  \see drawFill, getOverlappingSegments, getNonNanSegments
*/
const QPolygonF QCPGraph::getChannelFillPolygon(const QVector<QPointF> *thisData, QCPDataRange thisSegment, const QVector<QPointF> *otherData, QCPDataRange otherSegment) const
{
  if (!mChannelFillGraph)
    return QPolygonF();
  
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return QPolygonF(); }
  if (!mChannelFillGraph.data()->mKeyAxis) { qDebug() << Q_FUNC_INFO << "channel fill target key axis invalid"; return QPolygonF(); }
  
  if (mChannelFillGraph.data()->mKeyAxis.data()->orientation() != keyAxis->orientation())
    return QPolygonF(); // don't have same axis orientation, can't fill that (Note: if keyAxis fits, valueAxis will fit too, because it's always orthogonal to keyAxis)
  
  if (thisData->isEmpty()) return QPolygonF();
  QVector<QPointF> thisSegmentData(thisSegment.size());
  QVector<QPointF> otherSegmentData(otherSegment.size());
  std::copy(thisData->constBegin()+thisSegment.begin(), thisData->constBegin()+thisSegment.end(), thisSegmentData.begin());
  std::copy(otherData->constBegin()+otherSegment.begin(), otherData->constBegin()+otherSegment.end(), otherSegmentData.begin());
  // pointers to be able to swap them, depending which data range needs cropping:
  QVector<QPointF> *staticData = &thisSegmentData;
  QVector<QPointF> *croppedData = &otherSegmentData;
  
  // crop both vectors to ranges in which the keys overlap (which coord is key, depends on axisType):
  if (keyAxis->orientation() == Qt::Horizontal)
  {
    // x is key
    // crop lower bound:
    if (staticData->first().x() < croppedData->first().x()) // other one must be cropped
      qSwap(staticData, croppedData);
    const int lowBound = findIndexBelowX(croppedData, staticData->first().x());
    if (lowBound == -1) return QPolygonF(); // key ranges have no overlap
    croppedData->remove(0, lowBound);
    // set lowest point of cropped data to fit exactly key position of first static data point via linear interpolation:
    if (croppedData->size() < 2) return QPolygonF(); // need at least two points for interpolation
    double slope;
    if (!qFuzzyCompare(croppedData->at(1).x(), croppedData->at(0).x()))
      slope = (croppedData->at(1).y()-croppedData->at(0).y())/(croppedData->at(1).x()-croppedData->at(0).x());
    else
      slope = 0;
    (*croppedData)[0].setY(croppedData->at(0).y()+slope*(staticData->first().x()-croppedData->at(0).x()));
    (*croppedData)[0].setX(staticData->first().x());
    
    // crop upper bound:
    if (staticData->last().x() > croppedData->last().x()) // other one must be cropped
      qSwap(staticData, croppedData);
    int highBound = findIndexAboveX(croppedData, staticData->last().x());
    if (highBound == -1) return QPolygonF(); // key ranges have no overlap
    croppedData->remove(highBound+1, croppedData->size()-(highBound+1));
    // set highest point of cropped data to fit exactly key position of last static data point via linear interpolation:
    if (croppedData->size() < 2) return QPolygonF(); // need at least two points for interpolation
    const int li = croppedData->size()-1; // last index
    if (!qFuzzyCompare(croppedData->at(li).x(), croppedData->at(li-1).x()))
      slope = (croppedData->at(li).y()-croppedData->at(li-1).y())/(croppedData->at(li).x()-croppedData->at(li-1).x());
    else
      slope = 0;
    (*croppedData)[li].setY(croppedData->at(li-1).y()+slope*(staticData->last().x()-croppedData->at(li-1).x()));
    (*croppedData)[li].setX(staticData->last().x());
  } else // mKeyAxis->orientation() == Qt::Vertical
  {
    // y is key
    // crop lower bound:
    if (staticData->first().y() < croppedData->first().y()) // other one must be cropped
      qSwap(staticData, croppedData);
    int lowBound = findIndexBelowY(croppedData, staticData->first().y());
    if (lowBound == -1) return QPolygonF(); // key ranges have no overlap
    croppedData->remove(0, lowBound);
    // set lowest point of cropped data to fit exactly key position of first static data point via linear interpolation:
    if (croppedData->size() < 2) return QPolygonF(); // need at least two points for interpolation
    double slope;
    if (!qFuzzyCompare(croppedData->at(1).y(), croppedData->at(0).y())) // avoid division by zero in step plots
      slope = (croppedData->at(1).x()-croppedData->at(0).x())/(croppedData->at(1).y()-croppedData->at(0).y());
    else
      slope = 0;
    (*croppedData)[0].setX(croppedData->at(0).x()+slope*(staticData->first().y()-croppedData->at(0).y()));
    (*croppedData)[0].setY(staticData->first().y());
    
    // crop upper bound:
    if (staticData->last().y() > croppedData->last().y()) // other one must be cropped
      qSwap(staticData, croppedData);
    int highBound = findIndexAboveY(croppedData, staticData->last().y());
    if (highBound == -1) return QPolygonF(); // key ranges have no overlap
    croppedData->remove(highBound+1, croppedData->size()-(highBound+1));
    // set highest point of cropped data to fit exactly key position of last static data point via linear interpolation:
    if (croppedData->size() < 2) return QPolygonF(); // need at least two points for interpolation
    int li = croppedData->size()-1; // last index
    if (!qFuzzyCompare(croppedData->at(li).y(), croppedData->at(li-1).y())) // avoid division by zero in step plots
      slope = (croppedData->at(li).x()-croppedData->at(li-1).x())/(croppedData->at(li).y()-croppedData->at(li-1).y());
    else
      slope = 0;
    (*croppedData)[li].setX(croppedData->at(li-1).x()+slope*(staticData->last().y()-croppedData->at(li-1).y()));
    (*croppedData)[li].setY(staticData->last().y());
  }
  
  // return joined:
  for (int i=otherSegmentData.size()-1; i>=0; --i) // insert reversed, otherwise the polygon will be twisted
    thisSegmentData << otherSegmentData.at(i);
  return QPolygonF(thisSegmentData);
}

/*! \internal
  
  Finds the smallest index of \a data, whose points x value is just above \a x. Assumes x values in
  \a data points are ordered ascending, as is ensured by \ref getLines/\ref getScatters if the key
  axis is horizontal.

  Used to calculate the channel fill polygon, see \ref getChannelFillPolygon.
*/
int QCPGraph::findIndexAboveX(const QVector<QPointF> *data, double x) const
{
  for (int i=data->size()-1; i>=0; --i)
  {
    if (data->at(i).x() < x)
    {
      if (i<data->size()-1)
        return i+1;
      else
        return data->size()-1;
    }
  }
  return -1;
}

/*! \internal
  
  Finds the highest index of \a data, whose points x value is just below \a x. Assumes x values in
  \a data points are ordered ascending, as is ensured by \ref getLines/\ref getScatters if the key
  axis is horizontal.
  
  Used to calculate the channel fill polygon, see \ref getChannelFillPolygon.
*/
int QCPGraph::findIndexBelowX(const QVector<QPointF> *data, double x) const
{
  for (int i=0; i<data->size(); ++i)
  {
    if (data->at(i).x() > x)
    {
      if (i>0)
        return i-1;
      else
        return 0;
    }
  }
  return -1;
}

/*! \internal
  
  Finds the smallest index of \a data, whose points y value is just above \a y. Assumes y values in
  \a data points are ordered ascending, as is ensured by \ref getLines/\ref getScatters if the key
  axis is vertical.
  
  Used to calculate the channel fill polygon, see \ref getChannelFillPolygon.
*/
int QCPGraph::findIndexAboveY(const QVector<QPointF> *data, double y) const
{
  for (int i=data->size()-1; i>=0; --i)
  {
    if (data->at(i).y() < y)
    {
      if (i<data->size()-1)
        return i+1;
      else
        return data->size()-1;
    }
  }
  return -1;
}

/*! \internal
  
  Calculates the minimum distance in pixels the graph's representation has from the given \a
  pixelPoint. This is used to determine whether the graph was clicked or not, e.g. in \ref
  selectTest. The closest data point to \a pixelPoint is returned in \a closestData. Note that if
  the graph has a line representation, the returned distance may be smaller than the distance to
  the \a closestData point, since the distance to the graph line is also taken into account.
  
  If either the graph has no data or if the line style is \ref lsNone and the scatter style's shape
  is \ref QCPScatterStyle::ssNone (i.e. there is no visual representation of the graph), returns -1.0.
*/
double QCPGraph::pointDistance(const QPointF &pixelPoint, QCPGraphDataContainer::const_iterator &closestData) const
{
  closestData = mDataContainer->constEnd();
  if (mDataContainer->isEmpty())
    return -1.0;
  if (mLineStyle == lsNone && mScatterStyle.isNone())
    return -1.0;
  
  // calculate minimum distances to graph data points and find closestData iterator:
  double minDistSqr = (std::numeric_limits<double>::max)();
  // determine which key range comes into question, taking selection tolerance around pos into account:
  double posKeyMin, posKeyMax, dummy;
  pixelsToCoords(pixelPoint-QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMin, dummy);
  pixelsToCoords(pixelPoint+QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMax, dummy);
  if (posKeyMin > posKeyMax)
    qSwap(posKeyMin, posKeyMax);
  // iterate over found data points and then choose the one with the shortest distance to pos:
  QCPGraphDataContainer::const_iterator begin = mDataContainer->findBegin(posKeyMin, true);
  QCPGraphDataContainer::const_iterator end = mDataContainer->findEnd(posKeyMax, true);
  for (QCPGraphDataContainer::const_iterator it=begin; it!=end; ++it)
  {
    const double currentDistSqr = QCPVector2D(coordsToPixels(it->key, it->value)-pixelPoint).lengthSquared();
    if (currentDistSqr < minDistSqr)
    {
      minDistSqr = currentDistSqr;
      closestData = it;
    }
  }
    
  // calculate distance to graph line if there is one (if so, will probably be smaller than distance to closest data point):
  if (mLineStyle != lsNone)
  {
    // line displayed, calculate distance to line segments:
    QVector<QPointF> lineData;
    getLines(&lineData, QCPDataRange(0, dataCount())); // don't limit data range further since with sharp data spikes, line segments may be closer to test point than segments with closer key coordinate
    QCPVector2D p(pixelPoint);
    const int step = mLineStyle==lsImpulse ? 2 : 1; // impulse plot differs from other line styles in that the lineData points are only pairwise connected
    for (int i=0; i<lineData.size()-1; i+=step)
    {
      const double currentDistSqr = p.distanceSquaredToLine(lineData.at(i), lineData.at(i+1));
      if (currentDistSqr < minDistSqr)
        minDistSqr = currentDistSqr;
    }
  }
  
  return qSqrt(minDistSqr);
}

/*! \internal
  
  Finds the highest index of \a data, whose points y value is just below \a y. Assumes y values in
  \a data points are ordered ascending, as is ensured by \ref getLines/\ref getScatters if the key
  axis is vertical.

  Used to calculate the channel fill polygon, see \ref getChannelFillPolygon.
*/
int QCPGraph::findIndexBelowY(const QVector<QPointF> *data, double y) const
{
  for (int i=0; i<data->size(); ++i)
  {
    if (data->at(i).y() > y)
    {
      if (i>0)
        return i-1;
      else
        return 0;
    }
  }
  return -1;
}
