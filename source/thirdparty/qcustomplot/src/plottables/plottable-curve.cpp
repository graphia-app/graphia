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

#include "plottable-curve.h"

#include "../painter.h"
#include "../core.h"
#include "../axis/axis.h"
#include "../layoutelements/layoutelement-axisrect.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPCurveData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPCurveData
  \brief Holds the data of one single data point for QCPCurve.
  
  The stored data is:
  \li \a t: the free ordering parameter of this curve point, like in the mathematical vector <em>(x(t), y(t))</em>. (This is the \a sortKey)
  \li \a key: coordinate on the key axis of this curve point (this is the \a mainKey)
  \li \a value: coordinate on the value axis of this curve point (this is the \a mainValue)
  
  The container for storing multiple data points is \ref QCPCurveDataContainer. It is a typedef for
  \ref QCPDataContainer with \ref QCPCurveData as the DataType template parameter. See the
  documentation there for an explanation regarding the data type's generic methods.
  
  \see QCPCurveDataContainer
*/

/* start documentation of inline functions */

/*! \fn double QCPCurveData::sortKey() const
  
  Returns the \a t member of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn static QCPCurveData QCPCurveData::fromSortKey(double sortKey)
  
  Returns a data point with the specified \a sortKey (assigned to the data point's \a t member).
  All other members are set to zero.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn static static bool QCPCurveData::sortKeyIsMainKey()
  
  Since the member \a key is the data point key coordinate and the member \a t is the data ordering
  parameter, this method returns false.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn double QCPCurveData::mainKey() const
  
  Returns the \a key member of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn double QCPCurveData::mainValue() const
  
  Returns the \a value member of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/*! \fn QCPRange QCPCurveData::valueRange() const
  
  Returns a QCPRange with both lower and upper boundary set to \a value of this data point.
  
  For a general explanation of what this method is good for in the context of the data container,
  see the documentation of \ref QCPDataContainer.
*/

/* end documentation of inline functions */

/*!
  Constructs a curve data point with t, key and value set to zero.
*/
QCPCurveData::QCPCurveData() :
  t(0),
  key(0),
  value(0)
{
}

/*!
  Constructs a curve data point with the specified \a t, \a key and \a value.
*/
QCPCurveData::QCPCurveData(double t, double key, double value) :
  t(t),
  key(key),
  value(value)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPCurve
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPCurve
  \brief A plottable representing a parametric curve in a plot.
  
  \image html QCPCurve.png
  
  Unlike QCPGraph, plottables of this type may have multiple points with the same key coordinate,
  so their visual representation can have \a loops. This is realized by introducing a third
  coordinate \a t, which defines the order of the points described by the other two coordinates \a
  x and \a y.

  To plot data, assign it with the \ref setData or \ref addData functions. Alternatively, you can
  also access and modify the curve's data via the \ref data method, which returns a pointer to the
  internal \ref QCPCurveDataContainer.
  
  Gaps in the curve can be created by adding data points with NaN as key and value
  (<tt>qQNaN()</tt> or <tt>std::numeric_limits<double>::quiet_NaN()</tt>) in between the two data points that shall be
  separated.
  
  \section qcpcurve-appearance Changing the appearance
  
  The appearance of the curve is determined by the pen and the brush (\ref setPen, \ref setBrush).
  
  \section qcpcurve-usage Usage
  
  Like all data representing objects in QCustomPlot, the QCPCurve is a plottable
  (QCPAbstractPlottable). So the plottable-interface of QCustomPlot applies
  (QCustomPlot::plottable, QCustomPlot::removePlottable, etc.)
  
  Usually, you first create an instance:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcurve-creation-1
  which registers it with the QCustomPlot instance of the passed axes. Note that this QCustomPlot instance takes
  ownership of the plottable, so do not delete it manually but use QCustomPlot::removePlottable() instead.
  The newly created plottable can be modified, e.g.:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcurve-creation-2
*/

/* start of documentation of inline functions */

/*! \fn QSharedPointer<QCPCurveDataContainer> QCPCurve::data() const
  
  Returns a shared pointer to the internal data storage of type \ref QCPCurveDataContainer. You may
  use it to directly manipulate the data, which may be more convenient and faster than using the
  regular \ref setData or \ref addData methods.
*/

/* end of documentation of inline functions */

/*!
  Constructs a curve which uses \a keyAxis as its key axis ("x") and \a valueAxis as its value
  axis ("y"). \a keyAxis and \a valueAxis must reside in the same QCustomPlot instance and not have
  the same orientation. If either of these restrictions is violated, a corresponding message is
  printed to the debug output (qDebug), the construction is not aborted, though.
  
  The created QCPCurve is automatically registered with the QCustomPlot instance inferred from \a
  keyAxis. This QCustomPlot instance takes ownership of the QCPCurve, so do not delete it manually
  but use QCustomPlot::removePlottable() instead.
*/
QCPCurve::QCPCurve(QCPAxis *keyAxis, QCPAxis *valueAxis) :
  QCPAbstractPlottable1D<QCPCurveData>(keyAxis, valueAxis),
  mScatterSkip{},
  mLineStyle{}
{
  // modify inherited properties from abstract plottable:
  setPen(QPen(Qt::blue, 0));
  setBrush(Qt::NoBrush);
  
  setScatterStyle(QCPScatterStyle());
  setLineStyle(lsLine);
  setScatterSkip(0);
}

QCPCurve::~QCPCurve()
{
}

/*! \overload
  
  Replaces the current data container with the provided \a data container.
  
  Since a QSharedPointer is used, multiple QCPCurves may share the same data container safely.
  Modifying the data in the container will then affect all curves that share the container. Sharing
  can be achieved by simply exchanging the data containers wrapped in shared pointers:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcurve-datasharing-1
  
  If you do not wish to share containers, but create a copy from an existing container, rather use
  the \ref QCPDataContainer<DataType>::set method on the curve's data container directly:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcurve-datasharing-2
  
  \see addData
*/
void QCPCurve::setData(QSharedPointer<QCPCurveDataContainer> data)
{
  mDataContainer = data;
}

/*! \overload
  
  Replaces the current data with the provided points in \a t, \a keys and \a values. The provided
  vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
  
  If you can guarantee that the passed data points are sorted by \a t in ascending order, you can
  set \a alreadySorted to true, to improve performance by saving a sorting run.
  
  \see addData
*/
void QCPCurve::setData(const QVector<double> &t, const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  mDataContainer->clear();
  addData(t, keys, values, alreadySorted);
}


/*! \overload
  
  Replaces the current data with the provided points in \a keys and \a values. The provided vectors
  should have equal length. Else, the number of added points will be the size of the smallest
  vector.
  
  The t parameter of each data point will be set to the integer index of the respective key/value
  pair.
  
  \see addData
*/
void QCPCurve::setData(const QVector<double> &keys, const QVector<double> &values)
{
  mDataContainer->clear();
  addData(keys, values);
}

/*!
  Sets the visual appearance of single data points in the plot. If set to \ref
  QCPScatterStyle::ssNone, no scatter points are drawn (e.g. for line-only plots with appropriate
  line style).
  
  \see QCPScatterStyle, setLineStyle
*/
void QCPCurve::setScatterStyle(const QCPScatterStyle &style)
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
void QCPCurve::setScatterSkip(int skip)
{
  mScatterSkip = qMax(0, skip);
}

/*!
  Sets how the single data points are connected in the plot or how they are represented visually
  apart from the scatter symbol. For scatter-only plots, set \a style to \ref lsNone and \ref
  setScatterStyle to the desired scatter style.
  
  \see setScatterStyle
*/
void QCPCurve::setLineStyle(QCPCurve::LineStyle style)
{
  mLineStyle = style;
}

/*! \overload
  
  Adds the provided points in \a t, \a keys and \a values to the current data. The provided vectors
  should have equal length. Else, the number of added points will be the size of the smallest
  vector.
  
  If you can guarantee that the passed data points are sorted by \a keys in ascending order, you
  can set \a alreadySorted to true, to improve performance by saving a sorting run.
  
  Alternatively, you can also access and modify the data directly via the \ref data method, which
  returns a pointer to the internal data container.
*/
void QCPCurve::addData(const QVector<double> &t, const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  if (t.size() != keys.size() || t.size() != values.size())
    qDebug() << Q_FUNC_INFO << "ts, keys and values have different sizes:" << t.size() << keys.size() << values.size();
  const int n = qMin(qMin(t.size(), keys.size()), values.size());
  QVector<QCPCurveData> tempData(n);
  QVector<QCPCurveData>::iterator it = tempData.begin();
  const QVector<QCPCurveData>::iterator itEnd = tempData.end();
  int i = 0;
  while (it != itEnd)
  {
    it->t = t[i];
    it->key = keys[i];
    it->value = values[i];
    ++it;
    ++i;
  }
  mDataContainer->add(tempData, alreadySorted); // don't modify tempData beyond this to prevent copy on write
}

/*! \overload
  
  Adds the provided points in \a keys and \a values to the current data. The provided vectors
  should have equal length. Else, the number of added points will be the size of the smallest
  vector.
  
  The t parameter of each data point will be set to the integer index of the respective key/value
  pair.
  
  Alternatively, you can also access and modify the data directly via the \ref data method, which
  returns a pointer to the internal data container.
*/
void QCPCurve::addData(const QVector<double> &keys, const QVector<double> &values)
{
  if (keys.size() != values.size())
    qDebug() << Q_FUNC_INFO << "keys and values have different sizes:" << keys.size() << values.size();
  const int n = qMin(keys.size(), values.size());
  double tStart;
  if (!mDataContainer->isEmpty())
    tStart = (mDataContainer->constEnd()-1)->t + 1.0;
  else
    tStart = 0;
  QVector<QCPCurveData> tempData(n);
  QVector<QCPCurveData>::iterator it = tempData.begin();
  const QVector<QCPCurveData>::iterator itEnd = tempData.end();
  int i = 0;
  while (it != itEnd)
  {
    it->t = tStart + i;
    it->key = keys[i];
    it->value = values[i];
    ++it;
    ++i;
  }
  mDataContainer->add(tempData, true); // don't modify tempData beyond this to prevent copy on write
}

/*! \overload
  Adds the provided data point as \a t, \a key and \a value to the current data.
  
  Alternatively, you can also access and modify the data directly via the \ref data method, which
  returns a pointer to the internal data container.
*/
void QCPCurve::addData(double t, double key, double value)
{
  mDataContainer->add(QCPCurveData(t, key, value));
}

/*! \overload
  
  Adds the provided data point as \a key and \a value to the current data.
  
  The t parameter is generated automatically by increments of 1 for each point, starting at the
  highest t of previously existing data or 0, if the curve data is empty.
  
  Alternatively, you can also access and modify the data directly via the \ref data method, which
  returns a pointer to the internal data container.
*/
void QCPCurve::addData(double key, double value)
{
  if (!mDataContainer->isEmpty())
    mDataContainer->add(QCPCurveData((mDataContainer->constEnd()-1)->t + 1.0, key, value));
  else
    mDataContainer->add(QCPCurveData(0.0, key, value));
}

/*!
  Implements a selectTest specific to this plottable's point geometry.

  If \a details is not 0, it will be set to a \ref QCPDataSelection, describing the closest data
  point to \a pos.
  
  \seebaseclassmethod \ref QCPAbstractPlottable::selectTest
*/
double QCPCurve::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if ((onlySelectable && mSelectable == QCP::stNone) || mDataContainer->isEmpty())
    return -1;
  if (!mKeyAxis || !mValueAxis)
    return -1;
  
  if (mKeyAxis.data()->axisRect()->rect().contains(pos.toPoint()) || mParentPlot->interactions().testFlag(QCP::iSelectPlottablesBeyondAxisRect))
  {
    QCPCurveDataContainer::const_iterator closestDataPoint = mDataContainer->constEnd();
    double result = pointDistance(pos, closestDataPoint);
    if (details)
    {
      int pointIndex = int( closestDataPoint-mDataContainer->constBegin() );
      details->setValue(QCPDataSelection(QCPDataRange(pointIndex, pointIndex+1)));
    }
    return result;
  } else
    return -1;
}

/* inherits documentation from base class */
QCPRange QCPCurve::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
  return mDataContainer->keyRange(foundRange, inSignDomain);
}

/* inherits documentation from base class */
QCPRange QCPCurve::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain, const QCPRange &inKeyRange) const
{
  return mDataContainer->valueRange(foundRange, inSignDomain, inKeyRange);
}

/* inherits documentation from base class */
void QCPCurve::draw(QCPPainter *painter)
{
  if (mDataContainer->isEmpty()) return;
  
  // allocate line vector:
  QVector<QPointF> lines, scatters;
  
  // loop over and draw segments of unselected/selected data:
  QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  allSegments << unselectedSegments << selectedSegments;
  for (int i=0; i<allSegments.size(); ++i)
  {
    bool isSelectedSegment = i >= unselectedSegments.size();
    
    // fill with curve data:
    QPen finalCurvePen = mPen; // determine the final pen already here, because the line optimization depends on its stroke width
    if (isSelectedSegment && mSelectionDecorator)
      finalCurvePen = mSelectionDecorator->pen();
    
    QCPDataRange lineDataRange = isSelectedSegment ? allSegments.at(i) : allSegments.at(i).adjusted(-1, 1); // unselected segments extend lines to bordering selected data point (safe to exceed total data bounds in first/last segment, getCurveLines takes care)
    getCurveLines(&lines, lineDataRange, finalCurvePen.widthF());
    
    // check data validity if flag set:
  #ifdef QCUSTOMPLOT_CHECK_DATA
    for (QCPCurveDataContainer::const_iterator it = mDataContainer->constBegin(); it != mDataContainer->constEnd(); ++it)
    {
      if (QCP::isInvalidData(it->t) ||
          QCP::isInvalidData(it->key, it->value))
        qDebug() << Q_FUNC_INFO << "Data point at" << it->key << "invalid." << "Plottable name:" << name();
    }
  #endif
    
    // draw curve fill:
    applyFillAntialiasingHint(painter);
    if (isSelectedSegment && mSelectionDecorator)
      mSelectionDecorator->applyBrush(painter);
    else
      painter->setBrush(mBrush);
    painter->setPen(Qt::NoPen);
    if (painter->brush().style() != Qt::NoBrush && painter->brush().color().alpha() != 0)
      painter->drawPolygon(QPolygonF(lines));
    
    // draw curve line:
    if (mLineStyle != lsNone)
    {
      painter->setPen(finalCurvePen);
      painter->setBrush(Qt::NoBrush);
      drawCurveLine(painter, lines);
    }
    
    // draw scatters:
    QCPScatterStyle finalScatterStyle = mScatterStyle;
    if (isSelectedSegment && mSelectionDecorator)
      finalScatterStyle = mSelectionDecorator->getFinalScatterStyle(mScatterStyle);
    if (!finalScatterStyle.isNone())
    {
      getScatters(&scatters, allSegments.at(i), finalScatterStyle.size());
      drawScatterPlot(painter, scatters, finalScatterStyle);
    }
  }
  
  // draw other selection decoration that isn't just line/scatter pens and brushes:
  if (mSelectionDecorator)
    mSelectionDecorator->drawDecoration(painter, selection());
}

/* inherits documentation from base class */
void QCPCurve::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
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

/*!  \internal

  Draws lines between the points in \a lines, given in pixel coordinates.

  \see drawScatterPlot, getCurveLines
*/
void QCPCurve::drawCurveLine(QCPPainter *painter, const QVector<QPointF> &lines) const
{
  if (painter->pen().style() != Qt::NoPen && painter->pen().color().alpha() != 0)
  {
    applyDefaultAntialiasingHint(painter);
    drawPolyline(painter, lines);
  }
}

/*! \internal

  Draws scatter symbols at every point passed in \a points, given in pixel coordinates. The
  scatters will be drawn with \a painter and have the appearance as specified in \a style.

  \see drawCurveLine, getCurveLines
*/
void QCPCurve::drawScatterPlot(QCPPainter *painter, const QVector<QPointF> &points, const QCPScatterStyle &style) const
{
  // draw scatter point symbols:
  applyScattersAntialiasingHint(painter);
  style.applyTo(painter, mPen);
  foreach (const QPointF &point, points)
    if (!qIsNaN(point.x()) && !qIsNaN(point.y()))
      style.drawShape(painter,  point);
}

/*! \internal

  Called by \ref draw to generate points in pixel coordinates which represent the line of the
  curve.

  Line segments that aren't visible in the current axis rect are handled in an optimized way. They
  are projected onto a rectangle slightly larger than the visible axis rect and simplified
  regarding point count. The algorithm makes sure to preserve appearance of lines and fills inside
  the visible axis rect by generating new temporary points on the outer rect if necessary.

  \a lines will be filled with points in pixel coordinates, that can be drawn with \ref
  drawCurveLine.

  \a dataRange specifies the beginning and ending data indices that will be taken into account for
  conversion. In this function, the specified range may exceed the total data bounds without harm:
  a correspondingly trimmed data range will be used. This takes the burden off the user of this
  function to check for valid indices in \a dataRange, e.g. when extending ranges coming from \ref
  getDataSegments.

  \a penWidth specifies the pen width that will be used to later draw the lines generated by this
  function. This is needed here to calculate an accordingly wider margin around the axis rect when
  performing the line optimization.

  Methods that are also involved in the algorithm are: \ref getRegion, \ref getOptimizedPoint, \ref
  getOptimizedCornerPoints \ref mayTraverse, \ref getTraverse, \ref getTraverseCornerPoints.

  \see drawCurveLine, drawScatterPlot
*/
void QCPCurve::getCurveLines(QVector<QPointF> *lines, const QCPDataRange &dataRange, double penWidth) const
{
  if (!lines) return;
  lines->clear();
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  // add margins to rect to compensate for stroke width
  const double strokeMargin = qMax(qreal(1.0), qreal(penWidth*0.75)); // stroke radius + 50% safety
  const double keyMin = keyAxis->pixelToCoord(keyAxis->coordToPixel(keyAxis->range().lower)-strokeMargin*keyAxis->pixelOrientation());
  const double keyMax = keyAxis->pixelToCoord(keyAxis->coordToPixel(keyAxis->range().upper)+strokeMargin*keyAxis->pixelOrientation());
  const double valueMin = valueAxis->pixelToCoord(valueAxis->coordToPixel(valueAxis->range().lower)-strokeMargin*valueAxis->pixelOrientation());
  const double valueMax = valueAxis->pixelToCoord(valueAxis->coordToPixel(valueAxis->range().upper)+strokeMargin*valueAxis->pixelOrientation());
  QCPCurveDataContainer::const_iterator itBegin = mDataContainer->constBegin();
  QCPCurveDataContainer::const_iterator itEnd = mDataContainer->constEnd();
  mDataContainer->limitIteratorsToDataRange(itBegin, itEnd, dataRange);
  if (itBegin == itEnd)
    return;
  QCPCurveDataContainer::const_iterator it = itBegin;
  QCPCurveDataContainer::const_iterator prevIt = itEnd-1;
  int prevRegion = getRegion(prevIt->key, prevIt->value, keyMin, valueMax, keyMax, valueMin);
  QVector<QPointF> trailingPoints; // points that must be applied after all other points (are generated only when handling first point to get virtual segment between last and first point right)
  while (it != itEnd)
  {
    const int currentRegion = getRegion(it->key, it->value, keyMin, valueMax, keyMax, valueMin);
    if (currentRegion != prevRegion) // changed region, possibly need to add some optimized edge points or original points if entering R
    {
      if (currentRegion != 5) // segment doesn't end in R, so it's a candidate for removal
      {
        QPointF crossA, crossB;
        if (prevRegion == 5) // we're coming from R, so add this point optimized
        {
          lines->append(getOptimizedPoint(currentRegion, it->key, it->value, prevIt->key, prevIt->value, keyMin, valueMax, keyMax, valueMin));
          // in the situations 5->1/7/9/3 the segment may leave R and directly cross through two outer regions. In these cases we need to add an additional corner point
          *lines << getOptimizedCornerPoints(prevRegion, currentRegion, prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin);
        } else if (mayTraverse(prevRegion, currentRegion) &&
                   getTraverse(prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin, crossA, crossB))
        {
          // add the two cross points optimized if segment crosses R and if segment isn't virtual zeroth segment between last and first curve point:
          QVector<QPointF> beforeTraverseCornerPoints, afterTraverseCornerPoints;
          getTraverseCornerPoints(prevRegion, currentRegion, keyMin, valueMax, keyMax, valueMin, beforeTraverseCornerPoints, afterTraverseCornerPoints);
          if (it != itBegin)
          {
            *lines << beforeTraverseCornerPoints;
            lines->append(crossA);
            lines->append(crossB);
            *lines << afterTraverseCornerPoints;
          } else
          {
            lines->append(crossB);
            *lines << afterTraverseCornerPoints;
            trailingPoints << beforeTraverseCornerPoints << crossA ;
          }
        } else // doesn't cross R, line is just moving around in outside regions, so only need to add optimized point(s) at the boundary corner(s)
        {
          *lines << getOptimizedCornerPoints(prevRegion, currentRegion, prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin);
        }
      } else // segment does end in R, so we add previous point optimized and this point at original position
      {
        if (it == itBegin) // it is first point in curve and prevIt is last one. So save optimized point for adding it to the lineData in the end
          trailingPoints << getOptimizedPoint(prevRegion, prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin);
        else
          lines->append(getOptimizedPoint(prevRegion, prevIt->key, prevIt->value, it->key, it->value, keyMin, valueMax, keyMax, valueMin));
        lines->append(coordsToPixels(it->key, it->value));
      }
    } else // region didn't change
    {
      if (currentRegion == 5) // still in R, keep adding original points
      {
        lines->append(coordsToPixels(it->key, it->value));
      } else // still outside R, no need to add anything
      {
        // see how this is not doing anything? That's the main optimization...
      }
    }
    prevIt = it;
    prevRegion = currentRegion;
    ++it;
  }
  *lines << trailingPoints;
}

/*! \internal

  Called by \ref draw to generate points in pixel coordinates which represent the scatters of the
  curve. If a scatter skip is configured (\ref setScatterSkip), the returned points are accordingly
  sparser.

  Scatters that aren't visible in the current axis rect are optimized away.

  \a scatters will be filled with points in pixel coordinates, that can be drawn with \ref
  drawScatterPlot.

  \a dataRange specifies the beginning and ending data indices that will be taken into account for
  conversion.

  \a scatterWidth specifies the scatter width that will be used to later draw the scatters at pixel
  coordinates generated by this function. This is needed here to calculate an accordingly wider
  margin around the axis rect when performing the data point reduction.

  \see draw, drawScatterPlot
*/
void QCPCurve::getScatters(QVector<QPointF> *scatters, const QCPDataRange &dataRange, double scatterWidth) const
{
  if (!scatters) return;
  scatters->clear();
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  QCPCurveDataContainer::const_iterator begin = mDataContainer->constBegin();
  QCPCurveDataContainer::const_iterator end = mDataContainer->constEnd();
  mDataContainer->limitIteratorsToDataRange(begin, end, dataRange);
  if (begin == end)
    return;
  const int scatterModulo = mScatterSkip+1;
  const bool doScatterSkip = mScatterSkip > 0;
  int endIndex = int( end-mDataContainer->constBegin() );
  
  QCPRange keyRange = keyAxis->range();
  QCPRange valueRange = valueAxis->range();
  // extend range to include width of scatter symbols:
  keyRange.lower = keyAxis->pixelToCoord(keyAxis->coordToPixel(keyRange.lower)-scatterWidth*keyAxis->pixelOrientation());
  keyRange.upper = keyAxis->pixelToCoord(keyAxis->coordToPixel(keyRange.upper)+scatterWidth*keyAxis->pixelOrientation());
  valueRange.lower = valueAxis->pixelToCoord(valueAxis->coordToPixel(valueRange.lower)-scatterWidth*valueAxis->pixelOrientation());
  valueRange.upper = valueAxis->pixelToCoord(valueAxis->coordToPixel(valueRange.upper)+scatterWidth*valueAxis->pixelOrientation());
  
  QCPCurveDataContainer::const_iterator it = begin;
  int itIndex = int( begin-mDataContainer->constBegin() );
  while (doScatterSkip && it != end && itIndex % scatterModulo != 0) // advance begin iterator to first non-skipped scatter
  {
    ++itIndex;
    ++it;
  }
  if (keyAxis->orientation() == Qt::Vertical)
  {
    while (it != end)
    {
      if (!qIsNaN(it->value) && keyRange.contains(it->key) && valueRange.contains(it->value))
        scatters->append(QPointF(valueAxis->coordToPixel(it->value), keyAxis->coordToPixel(it->key)));
      
      // advance iterator to next (non-skipped) data point:
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
  } else
  {
    while (it != end)
    {
      if (!qIsNaN(it->value) && keyRange.contains(it->key) && valueRange.contains(it->value))
        scatters->append(QPointF(keyAxis->coordToPixel(it->key), valueAxis->coordToPixel(it->value)));
      
      // advance iterator to next (non-skipped) data point:
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
  }
}

/*! \internal

  This function is part of the curve optimization algorithm of \ref getCurveLines.

  It returns the region of the given point (\a key, \a value) with respect to a rectangle defined
  by \a keyMin, \a keyMax, \a valueMin, and \a valueMax.

  The regions are enumerated from top to bottom (\a valueMin to \a valueMax) and left to right (\a
  keyMin to \a keyMax):

  <table style="width:10em; text-align:center">
    <tr><td>1</td><td>4</td><td>7</td></tr>
    <tr><td>2</td><td style="border:1px solid black">5</td><td>8</td></tr>
    <tr><td>3</td><td>6</td><td>9</td></tr>
  </table>

  With the rectangle being region 5, and the outer regions extending infinitely outwards. In the
  curve optimization algorithm, region 5 is considered to be the visible portion of the plot.
*/
int QCPCurve::getRegion(double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const
{
  if (key < keyMin) // region 123
  {
    if (value > valueMax)
      return 1;
    else if (value < valueMin)
      return 3;
    else
      return 2;
  } else if (key > keyMax) // region 789
  {
    if (value > valueMax)
      return 7;
    else if (value < valueMin)
      return 9;
    else
      return 8;
  } else // region 456
  {
    if (value > valueMax)
      return 4;
    else if (value < valueMin)
      return 6;
    else
      return 5;
  }
}

/*! \internal
  
  This function is part of the curve optimization algorithm of \ref getCurveLines.
  
  This method is used in case the current segment passes from inside the visible rect (region 5,
  see \ref getRegion) to any of the outer regions (\a otherRegion). The current segment is given by
  the line connecting (\a key, \a value) with (\a otherKey, \a otherValue).
  
  It returns the intersection point of the segment with the border of region 5.
  
  For this function it doesn't matter whether (\a key, \a value) is the point inside region 5 or
  whether it's (\a otherKey, \a otherValue), i.e. whether the segment is coming from region 5 or
  leaving it. It is important though that \a otherRegion correctly identifies the other region not
  equal to 5.
*/
QPointF QCPCurve::getOptimizedPoint(int otherRegion, double otherKey, double otherValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const
{
  // The intersection point interpolation here is done in pixel coordinates, so we don't need to
  // differentiate between different axis scale types. Note that the nomenclature
  // top/left/bottom/right/min/max is with respect to the rect in plot coordinates, wich may be
  // different in pixel coordinates (horz/vert key axes, reversed ranges)
  
  const double keyMinPx = mKeyAxis->coordToPixel(keyMin);
  const double keyMaxPx = mKeyAxis->coordToPixel(keyMax);
  const double valueMinPx = mValueAxis->coordToPixel(valueMin);
  const double valueMaxPx = mValueAxis->coordToPixel(valueMax);
  const double otherValuePx = mValueAxis->coordToPixel(otherValue);
  const double valuePx = mValueAxis->coordToPixel(value);
  const double otherKeyPx = mKeyAxis->coordToPixel(otherKey);
  const double keyPx = mKeyAxis->coordToPixel(key);
  double intersectKeyPx = keyMinPx; // initial key just a fail-safe
  double intersectValuePx = valueMinPx; // initial value just a fail-safe
  switch (otherRegion)
  {
    case 1: // top and left edge
    {
      intersectValuePx = valueMaxPx;
      intersectKeyPx = otherKeyPx + (keyPx-otherKeyPx)/(valuePx-otherValuePx)*(intersectValuePx-otherValuePx);
      if (intersectKeyPx < qMin(keyMinPx, keyMaxPx) || intersectKeyPx > qMax(keyMinPx, keyMaxPx)) // check whether top edge is not intersected, then it must be left edge (qMin/qMax necessary since axes may be reversed)
      {
        intersectKeyPx = keyMinPx;
        intersectValuePx = otherValuePx + (valuePx-otherValuePx)/(keyPx-otherKeyPx)*(intersectKeyPx-otherKeyPx);
      }
      break;
    }
    case 2: // left edge
    {
      intersectKeyPx = keyMinPx;
      intersectValuePx = otherValuePx + (valuePx-otherValuePx)/(keyPx-otherKeyPx)*(intersectKeyPx-otherKeyPx);
      break;
    }
    case 3: // bottom and left edge
    {
      intersectValuePx = valueMinPx;
      intersectKeyPx = otherKeyPx + (keyPx-otherKeyPx)/(valuePx-otherValuePx)*(intersectValuePx-otherValuePx);
      if (intersectKeyPx < qMin(keyMinPx, keyMaxPx) || intersectKeyPx > qMax(keyMinPx, keyMaxPx)) // check whether bottom edge is not intersected, then it must be left edge (qMin/qMax necessary since axes may be reversed)
      {
        intersectKeyPx = keyMinPx;
        intersectValuePx = otherValuePx + (valuePx-otherValuePx)/(keyPx-otherKeyPx)*(intersectKeyPx-otherKeyPx);
      }
      break;
    }
    case 4: // top edge
    {
      intersectValuePx = valueMaxPx;
      intersectKeyPx = otherKeyPx + (keyPx-otherKeyPx)/(valuePx-otherValuePx)*(intersectValuePx-otherValuePx);
      break;
    }
    case 5:
    {
      break; // case 5 shouldn't happen for this function but we add it anyway to prevent potential discontinuity in branch table
    }
    case 6: // bottom edge
    {
      intersectValuePx = valueMinPx;
      intersectKeyPx = otherKeyPx + (keyPx-otherKeyPx)/(valuePx-otherValuePx)*(intersectValuePx-otherValuePx);
      break;
    }
    case 7: // top and right edge
    {
      intersectValuePx = valueMaxPx;
      intersectKeyPx = otherKeyPx + (keyPx-otherKeyPx)/(valuePx-otherValuePx)*(intersectValuePx-otherValuePx);
      if (intersectKeyPx < qMin(keyMinPx, keyMaxPx) || intersectKeyPx > qMax(keyMinPx, keyMaxPx)) // check whether top edge is not intersected, then it must be right edge (qMin/qMax necessary since axes may be reversed)
      {
        intersectKeyPx = keyMaxPx;
        intersectValuePx = otherValuePx + (valuePx-otherValuePx)/(keyPx-otherKeyPx)*(intersectKeyPx-otherKeyPx);
      }
      break;
    }
    case 8: // right edge
    {
      intersectKeyPx = keyMaxPx;
      intersectValuePx = otherValuePx + (valuePx-otherValuePx)/(keyPx-otherKeyPx)*(intersectKeyPx-otherKeyPx);
      break;
    }
    case 9: // bottom and right edge
    {
      intersectValuePx = valueMinPx;
      intersectKeyPx = otherKeyPx + (keyPx-otherKeyPx)/(valuePx-otherValuePx)*(intersectValuePx-otherValuePx);
      if (intersectKeyPx < qMin(keyMinPx, keyMaxPx) || intersectKeyPx > qMax(keyMinPx, keyMaxPx)) // check whether bottom edge is not intersected, then it must be right edge (qMin/qMax necessary since axes may be reversed)
      {
        intersectKeyPx = keyMaxPx;
        intersectValuePx = otherValuePx + (valuePx-otherValuePx)/(keyPx-otherKeyPx)*(intersectKeyPx-otherKeyPx);
      }
      break;
    }
  }
  if (mKeyAxis->orientation() == Qt::Horizontal)
    return {intersectKeyPx, intersectValuePx};
  else
    return {intersectValuePx, intersectKeyPx};
}

/*! \internal
  
  This function is part of the curve optimization algorithm of \ref getCurveLines.
  
  In situations where a single segment skips over multiple regions it might become necessary to add
  extra points at the corners of region 5 (see \ref getRegion) such that the optimized segment
  doesn't unintentionally cut through the visible area of the axis rect and create plot artifacts.
  This method provides these points that must be added, assuming the original segment doesn't
  start, end, or traverse region 5. (Corner points where region 5 is traversed are calculated by
  \ref getTraverseCornerPoints.)
  
  For example, consider a segment which directly goes from region 4 to 2 but originally is far out
  to the top left such that it doesn't cross region 5. Naively optimizing these points by
  projecting them on the top and left borders of region 5 will create a segment that surely crosses
  5, creating a visual artifact in the plot. This method prevents this by providing extra points at
  the top left corner, making the optimized curve correctly pass from region 4 to 1 to 2 without
  traversing 5.
*/
QVector<QPointF> QCPCurve::getOptimizedCornerPoints(int prevRegion, int currentRegion, double prevKey, double prevValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin) const
{
  QVector<QPointF> result;
  switch (prevRegion)
  {
    case 1:
    {
      switch (currentRegion)
      {
        case 2: { result << coordsToPixels(keyMin, valueMax); break; }
        case 4: { result << coordsToPixels(keyMin, valueMax); break; }
        case 3: { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMin, valueMin); break; }
        case 7: { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMax, valueMax); break; }
        case 6: { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMin, valueMin); result.append(result.last()); break; }
        case 8: { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMax, valueMax); result.append(result.last()); break; }
        case 9: { // in this case we need another distinction of cases: segment may pass below or above rect, requiring either bottom right or top left corner points
          if ((value-prevValue)/(key-prevKey)*(keyMin-key)+value < valueMin) // segment passes below R
          { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMin, valueMin); result.append(result.last()); result << coordsToPixels(keyMax, valueMin); }
          else
          { result << coordsToPixels(keyMin, valueMax) << coordsToPixels(keyMax, valueMax); result.append(result.last()); result << coordsToPixels(keyMax, valueMin); }
          break;
        }
      }
      break;
    }
    case 2:
    {
      switch (currentRegion)
      {
        case 1: { result << coordsToPixels(keyMin, valueMax); break; }
        case 3: { result << coordsToPixels(keyMin, valueMin); break; }
        case 4: { result << coordsToPixels(keyMin, valueMax); result.append(result.last()); break; }
        case 6: { result << coordsToPixels(keyMin, valueMin); result.append(result.last()); break; }
        case 7: { result << coordsToPixels(keyMin, valueMax); result.append(result.last()); result << coordsToPixels(keyMax, valueMax); break; }
        case 9: { result << coordsToPixels(keyMin, valueMin); result.append(result.last()); result << coordsToPixels(keyMax, valueMin); break; }
      }
      break;
    }
    case 3:
    {
      switch (currentRegion)
      {
        case 2: { result << coordsToPixels(keyMin, valueMin); break; }
        case 6: { result << coordsToPixels(keyMin, valueMin); break; }
        case 1: { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMin, valueMax); break; }
        case 9: { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMax, valueMin); break; }
        case 4: { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMin, valueMax); result.append(result.last()); break; }
        case 8: { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMax, valueMin); result.append(result.last()); break; }
        case 7: { // in this case we need another distinction of cases: segment may pass below or above rect, requiring either bottom right or top left corner points
          if ((value-prevValue)/(key-prevKey)*(keyMax-key)+value < valueMin) // segment passes below R
          { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMax, valueMin); result.append(result.last()); result << coordsToPixels(keyMax, valueMax); }
          else
          { result << coordsToPixels(keyMin, valueMin) << coordsToPixels(keyMin, valueMax); result.append(result.last()); result << coordsToPixels(keyMax, valueMax); }
          break;
        }
      }
      break;
    }
    case 4:
    {
      switch (currentRegion)
      {
        case 1: { result << coordsToPixels(keyMin, valueMax); break; }
        case 7: { result << coordsToPixels(keyMax, valueMax); break; }
        case 2: { result << coordsToPixels(keyMin, valueMax); result.append(result.last()); break; }
        case 8: { result << coordsToPixels(keyMax, valueMax); result.append(result.last()); break; }
        case 3: { result << coordsToPixels(keyMin, valueMax); result.append(result.last()); result << coordsToPixels(keyMin, valueMin); break; }
        case 9: { result << coordsToPixels(keyMax, valueMax); result.append(result.last()); result << coordsToPixels(keyMax, valueMin); break; }
      }
      break;
    }
    case 5:
    {
      switch (currentRegion)
      {
        case 1: { result << coordsToPixels(keyMin, valueMax); break; }
        case 7: { result << coordsToPixels(keyMax, valueMax); break; }
        case 9: { result << coordsToPixels(keyMax, valueMin); break; }
        case 3: { result << coordsToPixels(keyMin, valueMin); break; }
      }
      break;
    }
    case 6:
    {
      switch (currentRegion)
      {
        case 3: { result << coordsToPixels(keyMin, valueMin); break; }
        case 9: { result << coordsToPixels(keyMax, valueMin); break; }
        case 2: { result << coordsToPixels(keyMin, valueMin); result.append(result.last()); break; }
        case 8: { result << coordsToPixels(keyMax, valueMin); result.append(result.last()); break; }
        case 1: { result << coordsToPixels(keyMin, valueMin); result.append(result.last()); result << coordsToPixels(keyMin, valueMax); break; }
        case 7: { result << coordsToPixels(keyMax, valueMin); result.append(result.last()); result << coordsToPixels(keyMax, valueMax); break; }
      }
      break;
    }
    case 7:
    {
      switch (currentRegion)
      {
        case 4: { result << coordsToPixels(keyMax, valueMax); break; }
        case 8: { result << coordsToPixels(keyMax, valueMax); break; }
        case 1: { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMin, valueMax); break; }
        case 9: { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMax, valueMin); break; }
        case 2: { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMin, valueMax); result.append(result.last()); break; }
        case 6: { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMax, valueMin); result.append(result.last()); break; }
        case 3: { // in this case we need another distinction of cases: segment may pass below or above rect, requiring either bottom right or top left corner points
          if ((value-prevValue)/(key-prevKey)*(keyMax-key)+value < valueMin) // segment passes below R
          { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMax, valueMin); result.append(result.last()); result << coordsToPixels(keyMin, valueMin); }
          else
          { result << coordsToPixels(keyMax, valueMax) << coordsToPixels(keyMin, valueMax); result.append(result.last()); result << coordsToPixels(keyMin, valueMin); }
          break;
        }
      }
      break;
    }
    case 8:
    {
      switch (currentRegion)
      {
        case 7: { result << coordsToPixels(keyMax, valueMax); break; }
        case 9: { result << coordsToPixels(keyMax, valueMin); break; }
        case 4: { result << coordsToPixels(keyMax, valueMax); result.append(result.last()); break; }
        case 6: { result << coordsToPixels(keyMax, valueMin); result.append(result.last()); break; }
        case 1: { result << coordsToPixels(keyMax, valueMax); result.append(result.last()); result << coordsToPixels(keyMin, valueMax); break; }
        case 3: { result << coordsToPixels(keyMax, valueMin); result.append(result.last()); result << coordsToPixels(keyMin, valueMin); break; }
      }
      break;
    }
    case 9:
    {
      switch (currentRegion)
      {
        case 6: { result << coordsToPixels(keyMax, valueMin); break; }
        case 8: { result << coordsToPixels(keyMax, valueMin); break; }
        case 3: { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMin, valueMin); break; }
        case 7: { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMax, valueMax); break; }
        case 2: { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMin, valueMin); result.append(result.last()); break; }
        case 4: { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMax, valueMax); result.append(result.last()); break; }
        case 1: { // in this case we need another distinction of cases: segment may pass below or above rect, requiring either bottom right or top left corner points
          if ((value-prevValue)/(key-prevKey)*(keyMin-key)+value < valueMin) // segment passes below R
          { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMin, valueMin); result.append(result.last()); result << coordsToPixels(keyMin, valueMax); }
          else
          { result << coordsToPixels(keyMax, valueMin) << coordsToPixels(keyMax, valueMax); result.append(result.last()); result << coordsToPixels(keyMin, valueMax); }
          break;
        }
      }
      break;
    }
  }
  return result;
}

/*! \internal
  
  This function is part of the curve optimization algorithm of \ref getCurveLines.
  
  This method returns whether a segment going from \a prevRegion to \a currentRegion (see \ref
  getRegion) may traverse the visible region 5. This function assumes that neither \a prevRegion
  nor \a currentRegion is 5 itself.
  
  If this method returns false, the segment for sure doesn't pass region 5. If it returns true, the
  segment may or may not pass region 5 and a more fine-grained calculation must be used (\ref
  getTraverse).
*/
bool QCPCurve::mayTraverse(int prevRegion, int currentRegion) const
{
  switch (prevRegion)
  {
    case 1:
    {
      switch (currentRegion)
      {
        case 4:
        case 7:
        case 2:
        case 3: return false;
        default: return true;
      }
    }
    case 2:
    {
      switch (currentRegion)
      {
        case 1:
        case 3: return false;
        default: return true;
      }
    }
    case 3:
    {
      switch (currentRegion)
      {
        case 1:
        case 2:
        case 6:
        case 9: return false;
        default: return true;
      }
    }
    case 4:
    {
      switch (currentRegion)
      {
        case 1:
        case 7: return false;
        default: return true;
      }
    }
    case 5: return false; // should never occur
    case 6:
    {
      switch (currentRegion)
      {
        case 3:
        case 9: return false;
        default: return true;
      }
    }
    case 7:
    {
      switch (currentRegion)
      {
        case 1:
        case 4:
        case 8:
        case 9: return false;
        default: return true;
      }
    }
    case 8:
    {
      switch (currentRegion)
      {
        case 7:
        case 9: return false;
        default: return true;
      }
    }
    case 9:
    {
      switch (currentRegion)
      {
        case 3:
        case 6:
        case 8:
        case 7: return false;
        default: return true;
      }
    }
    default: return true;
  }
}


/*! \internal
  
  This function is part of the curve optimization algorithm of \ref getCurveLines.
  
  This method assumes that the \ref mayTraverse test has returned true, so there is a chance the
  segment defined by (\a prevKey, \a prevValue) and (\a key, \a value) goes through the visible
  region 5.
  
  The return value of this method indicates whether the segment actually traverses region 5 or not.
  
  If the segment traverses 5, the output parameters \a crossA and \a crossB indicate the entry and
  exit points of region 5. They will become the optimized points for that segment.
*/
bool QCPCurve::getTraverse(double prevKey, double prevValue, double key, double value, double keyMin, double valueMax, double keyMax, double valueMin, QPointF &crossA, QPointF &crossB) const
{
  // The intersection point interpolation here is done in pixel coordinates, so we don't need to
  // differentiate between different axis scale types. Note that the nomenclature
  // top/left/bottom/right/min/max is with respect to the rect in plot coordinates, wich may be
  // different in pixel coordinates (horz/vert key axes, reversed ranges)
  
  QList<QPointF> intersections;
  const double valueMinPx = mValueAxis->coordToPixel(valueMin);
  const double valueMaxPx = mValueAxis->coordToPixel(valueMax);
  const double keyMinPx = mKeyAxis->coordToPixel(keyMin);
  const double keyMaxPx = mKeyAxis->coordToPixel(keyMax);
  const double keyPx = mKeyAxis->coordToPixel(key);
  const double valuePx = mValueAxis->coordToPixel(value);
  const double prevKeyPx = mKeyAxis->coordToPixel(prevKey);
  const double prevValuePx = mValueAxis->coordToPixel(prevValue);
  if (qFuzzyIsNull(keyPx-prevKeyPx)) // line is parallel to value axis
  {
    // due to region filter in mayTraverse(), if line is parallel to value or key axis, region 5 is traversed here
    intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyPx, valueMinPx) : QPointF(valueMinPx, keyPx)); // direction will be taken care of at end of method
    intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyPx, valueMaxPx) : QPointF(valueMaxPx, keyPx));
  } else if (qFuzzyIsNull(valuePx-prevValuePx)) // line is parallel to key axis
  {
    // due to region filter in mayTraverse(), if line is parallel to value or key axis, region 5 is traversed here
    intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyMinPx, valuePx) : QPointF(valuePx, keyMinPx)); // direction will be taken care of at end of method
    intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyMaxPx, valuePx) : QPointF(valuePx, keyMaxPx));
  } else // line is skewed
  {
    double gamma;
    double keyPerValuePx = (keyPx-prevKeyPx)/(valuePx-prevValuePx);
    // check top of rect:
    gamma = prevKeyPx + (valueMaxPx-prevValuePx)*keyPerValuePx;
    if (gamma >= qMin(keyMinPx, keyMaxPx) && gamma <= qMax(keyMinPx, keyMaxPx)) // qMin/qMax necessary since axes may be reversed
      intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(gamma, valueMaxPx) : QPointF(valueMaxPx, gamma));
    // check bottom of rect:
    gamma = prevKeyPx + (valueMinPx-prevValuePx)*keyPerValuePx;
    if (gamma >= qMin(keyMinPx, keyMaxPx) && gamma <= qMax(keyMinPx, keyMaxPx)) // qMin/qMax necessary since axes may be reversed
      intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(gamma, valueMinPx) : QPointF(valueMinPx, gamma));
    const double valuePerKeyPx = 1.0/keyPerValuePx;
    // check left of rect:
    gamma = prevValuePx + (keyMinPx-prevKeyPx)*valuePerKeyPx;
    if (gamma >= qMin(valueMinPx, valueMaxPx) && gamma <= qMax(valueMinPx, valueMaxPx)) // qMin/qMax necessary since axes may be reversed
      intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyMinPx, gamma) : QPointF(gamma, keyMinPx));
    // check right of rect:
    gamma = prevValuePx + (keyMaxPx-prevKeyPx)*valuePerKeyPx;
    if (gamma >= qMin(valueMinPx, valueMaxPx) && gamma <= qMax(valueMinPx, valueMaxPx)) // qMin/qMax necessary since axes may be reversed
      intersections.append(mKeyAxis->orientation() == Qt::Horizontal ? QPointF(keyMaxPx, gamma) : QPointF(gamma, keyMaxPx));
  }
  
  // handle cases where found points isn't exactly 2:
  if (intersections.size() > 2)
  {
    // line probably goes through corner of rect, and we got duplicate points there. single out the point pair with greatest distance in between:
    double distSqrMax = 0;
    QPointF pv1, pv2;
    for (int i=0; i<intersections.size()-1; ++i)
    {
      for (int k=i+1; k<intersections.size(); ++k)
      {
        QPointF distPoint = intersections.at(i)-intersections.at(k);
        double distSqr = distPoint.x()*distPoint.x()+distPoint.y()+distPoint.y();
        if (distSqr > distSqrMax)
        {
          pv1 = intersections.at(i);
          pv2 = intersections.at(k);
          distSqrMax = distSqr;
        }
      }
    }
    intersections = QList<QPointF>() << pv1 << pv2;
  } else if (intersections.size() != 2)
  {
    // one or even zero points found (shouldn't happen unless line perfectly tangent to corner), no need to draw segment
    return false;
  }
  
  // possibly re-sort points so optimized point segment has same direction as original segment:
  double xDelta = keyPx-prevKeyPx;
  double yDelta = valuePx-prevValuePx;
  if (mKeyAxis->orientation() != Qt::Horizontal)
    qSwap(xDelta, yDelta);
  if (xDelta*(intersections.at(1).x()-intersections.at(0).x()) + yDelta*(intersections.at(1).y()-intersections.at(0).y()) < 0) // scalar product of both segments < 0 -> opposite direction
    intersections.move(0, 1);
  crossA = intersections.at(0);
  crossB = intersections.at(1);
  return true;
}

/*! \internal
  
  This function is part of the curve optimization algorithm of \ref getCurveLines.
  
  This method assumes that the \ref getTraverse test has returned true, so the segment definitely
  traverses the visible region 5 when going from \a prevRegion to \a currentRegion.
  
  In certain situations it is not sufficient to merely generate the entry and exit points of the
  segment into/out of region 5, as \ref getTraverse provides. It may happen that a single segment, in
  addition to traversing region 5, skips another region outside of region 5, which makes it
  necessary to add an optimized corner point there (very similar to the job \ref
  getOptimizedCornerPoints does for segments that are completely in outside regions and don't
  traverse 5).
  
  As an example, consider a segment going from region 1 to region 6, traversing the lower left
  corner of region 5. In this configuration, the segment additionally crosses the border between
  region 1 and 2 before entering region 5. This makes it necessary to add an additional point in
  the top left corner, before adding the optimized traverse points. So in this case, the output
  parameter \a beforeTraverse will contain the top left corner point, and \a afterTraverse will be
  empty.
  
  In some cases, such as when going from region 1 to 9, it may even be necessary to add additional
  corner points before and after the traverse. Then both \a beforeTraverse and \a afterTraverse
  return the respective corner points.
*/
void QCPCurve::getTraverseCornerPoints(int prevRegion, int currentRegion, double keyMin, double valueMax, double keyMax, double valueMin, QVector<QPointF> &beforeTraverse, QVector<QPointF> &afterTraverse) const
{
  switch (prevRegion)
  {
    case 1:
    {
      switch (currentRegion)
      {
        case 6: { beforeTraverse << coordsToPixels(keyMin, valueMax); break; }
        case 9: { beforeTraverse << coordsToPixels(keyMin, valueMax); afterTraverse << coordsToPixels(keyMax, valueMin); break; }
        case 8: { beforeTraverse << coordsToPixels(keyMin, valueMax); break; }
      }
      break;
    }
    case 2:
    {
      switch (currentRegion)
      {
        case 7: { afterTraverse << coordsToPixels(keyMax, valueMax); break; }
        case 9: { afterTraverse << coordsToPixels(keyMax, valueMin); break; }
      }
      break;
    }
    case 3:
    {
      switch (currentRegion)
      {
        case 4: { beforeTraverse << coordsToPixels(keyMin, valueMin); break; }
        case 7: { beforeTraverse << coordsToPixels(keyMin, valueMin); afterTraverse << coordsToPixels(keyMax, valueMax); break; }
        case 8: { beforeTraverse << coordsToPixels(keyMin, valueMin); break; }
      }
      break;
    }
    case 4:
    {
      switch (currentRegion)
      {
        case 3: { afterTraverse << coordsToPixels(keyMin, valueMin); break; }
        case 9: { afterTraverse << coordsToPixels(keyMax, valueMin); break; }
      }
      break;
    }
    case 5: { break; } // shouldn't happen because this method only handles full traverses
    case 6:
    {
      switch (currentRegion)
      {
        case 1: { afterTraverse << coordsToPixels(keyMin, valueMax); break; }
        case 7: { afterTraverse << coordsToPixels(keyMax, valueMax); break; }
      }
      break;
    }
    case 7:
    {
      switch (currentRegion)
      {
        case 2: { beforeTraverse << coordsToPixels(keyMax, valueMax); break; }
        case 3: { beforeTraverse << coordsToPixels(keyMax, valueMax); afterTraverse << coordsToPixels(keyMin, valueMin); break; }
        case 6: { beforeTraverse << coordsToPixels(keyMax, valueMax); break; }
      }
      break;
    }
    case 8:
    {
      switch (currentRegion)
      {
        case 1: { afterTraverse << coordsToPixels(keyMin, valueMax); break; }
        case 3: { afterTraverse << coordsToPixels(keyMin, valueMin); break; }
      }
      break;
    }
    case 9:
    {
      switch (currentRegion)
      {
        case 2: { beforeTraverse << coordsToPixels(keyMax, valueMin); break; }
        case 1: { beforeTraverse << coordsToPixels(keyMax, valueMin); afterTraverse << coordsToPixels(keyMin, valueMax); break; }
        case 4: { beforeTraverse << coordsToPixels(keyMax, valueMin); break; }
      }
      break;
    }
  }
}

/*! \internal
  
  Calculates the (minimum) distance (in pixels) the curve's representation has from the given \a
  pixelPoint in pixels. This is used to determine whether the curve was clicked or not, e.g. in
  \ref selectTest. The closest data point to \a pixelPoint is returned in \a closestData. Note that
  if the curve has a line representation, the returned distance may be smaller than the distance to
  the \a closestData point, since the distance to the curve line is also taken into account.
  
  If either the curve has no data or if the line style is \ref lsNone and the scatter style's shape
  is \ref QCPScatterStyle::ssNone (i.e. there is no visual representation of the curve), returns
  -1.0.
*/
double QCPCurve::pointDistance(const QPointF &pixelPoint, QCPCurveDataContainer::const_iterator &closestData) const
{
  closestData = mDataContainer->constEnd();
  if (mDataContainer->isEmpty())
    return -1.0;
  if (mLineStyle == lsNone && mScatterStyle.isNone())
    return -1.0;
  
  if (mDataContainer->size() == 1)
  {
    QPointF dataPoint = coordsToPixels(mDataContainer->constBegin()->key, mDataContainer->constBegin()->value);
    closestData = mDataContainer->constBegin();
    return QCPVector2D(dataPoint-pixelPoint).length();
  }
  
  // calculate minimum distances to curve data points and find closestData iterator:
  double minDistSqr = (std::numeric_limits<double>::max)();
  // iterate over found data points and then choose the one with the shortest distance to pos:
  QCPCurveDataContainer::const_iterator begin = mDataContainer->constBegin();
  QCPCurveDataContainer::const_iterator end = mDataContainer->constEnd();
  for (QCPCurveDataContainer::const_iterator it=begin; it!=end; ++it)
  {
    const double currentDistSqr = QCPVector2D(coordsToPixels(it->key, it->value)-pixelPoint).lengthSquared();
    if (currentDistSqr < minDistSqr)
    {
      minDistSqr = currentDistSqr;
      closestData = it;
    }
  }
  
  // calculate distance to line if there is one (if so, will probably be smaller than distance to closest data point):
  if (mLineStyle != lsNone)
  {
    QVector<QPointF> lines;
    getCurveLines(&lines, QCPDataRange(0, dataCount()), mParentPlot->selectionTolerance()*1.2); // optimized lines outside axis rect shouldn't respond to clicks at the edge, so use 1.2*tolerance as pen width
    for (int i=0; i<lines.size()-1; ++i)
    {
      double currentDistSqr = QCPVector2D(pixelPoint).distanceSquaredToLine(lines.at(i), lines.at(i+1));
      if (currentDistSqr < minDistSqr)
        minDistSqr = currentDistSqr;
    }
  }
  
  return qSqrt(minDistSqr);
}
