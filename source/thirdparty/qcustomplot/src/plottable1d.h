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

#ifndef QCP_PLOTTABLE1D_H
#define QCP_PLOTTABLE1D_H

#include "global.h"
#include "painter.h"
#include "core.h"
#include "datacontainer.h"
#include "plottable.h"

class QCPPlottableInterface1D
{
public:
  virtual ~QCPPlottableInterface1D() = default;
  // introduced pure virtual methods:
  virtual int dataCount() const = 0;
  virtual double dataMainKey(int index) const = 0;
  virtual double dataSortKey(int index) const = 0;
  virtual double dataMainValue(int index) const = 0;
  virtual QCPRange dataValueRange(int index) const = 0;
  virtual QPointF dataPixelPosition(int index) const = 0;
  virtual bool sortKeyIsMainKey() const = 0;
  virtual QCPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable) const = 0;
  virtual int findBegin(double sortKey, bool expandedRange=true) const = 0;
  virtual int findEnd(double sortKey, bool expandedRange=true) const = 0;
};

template <class DataType>
class QCPAbstractPlottable1D : public QCPAbstractPlottable, public QCPPlottableInterface1D // no QCP_LIB_DECL, template class ends up in header (cpp included below)
{
  // No Q_OBJECT macro due to template class
  
public:
  QCPAbstractPlottable1D(QCPAxis *keyAxis, QCPAxis *valueAxis);
  virtual ~QCPAbstractPlottable1D() Q_DECL_OVERRIDE;
  
  // virtual methods of 1d plottable interface:
  virtual int dataCount() const Q_DECL_OVERRIDE;
  virtual double dataMainKey(int index) const Q_DECL_OVERRIDE;
  virtual double dataSortKey(int index) const Q_DECL_OVERRIDE;
  virtual double dataMainValue(int index) const Q_DECL_OVERRIDE;
  virtual QCPRange dataValueRange(int index) const Q_DECL_OVERRIDE;
  virtual QPointF dataPixelPosition(int index) const Q_DECL_OVERRIDE;
  virtual bool sortKeyIsMainKey() const Q_DECL_OVERRIDE;
  virtual QCPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable) const Q_DECL_OVERRIDE;
  virtual int findBegin(double sortKey, bool expandedRange=true) const Q_DECL_OVERRIDE;
  virtual int findEnd(double sortKey, bool expandedRange=true) const Q_DECL_OVERRIDE;
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  virtual QCPPlottableInterface1D *interface1D() Q_DECL_OVERRIDE { return this; }
  
protected:
  // property members:
  QSharedPointer<QCPDataContainer<DataType> > mDataContainer;
  
  // helpers for subclasses:
  void getDataSegments(QList<QCPDataRange> &selectedSegments, QList<QCPDataRange> &unselectedSegments) const;
  void drawPolyline(QCPPainter *painter, const QVector<QPointF> &lineData) const;

private:
  Q_DISABLE_COPY(QCPAbstractPlottable1D)
  
};



// include implementation in header since it is a class template:
////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPlottableInterface1D
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPlottableInterface1D
  \brief Defines an abstract interface for one-dimensional plottables

  This class contains only pure virtual methods which define a common interface to the data
  of one-dimensional plottables.

  For example, it is implemented by the template class \ref QCPAbstractPlottable1D (the preferred
  base class for one-dimensional plottables). So if you use that template class as base class of
  your one-dimensional plottable, you won't have to care about implementing the 1d interface
  yourself.

  If your plottable doesn't derive from \ref QCPAbstractPlottable1D but still wants to provide a 1d
  interface (e.g. like \ref QCPErrorBars does), you should inherit from both \ref
  QCPAbstractPlottable and \ref QCPPlottableInterface1D and accordingly reimplement the pure
  virtual methods of the 1d interface, matching your data container. Also, reimplement \ref
  QCPAbstractPlottable::interface1D to return the \c this pointer.

  If you have a \ref QCPAbstractPlottable pointer, you can check whether it implements this
  interface by calling \ref QCPAbstractPlottable::interface1D and testing it for a non-zero return
  value. If it indeed implements this interface, you may use it to access the plottable's data
  without needing to know the exact type of the plottable or its data point type.
*/

/* start documentation of pure virtual functions */

/*! \fn virtual int QCPPlottableInterface1D::dataCount() const = 0;
  
  Returns the number of data points of the plottable.
*/

/*! \fn virtual QCPDataSelection QCPPlottableInterface1D::selectTestRect(const QRectF &rect, bool onlySelectable) const = 0;
  
  Returns a data selection containing all the data points of this plottable which are contained (or
  hit by) \a rect. This is used mainly in the selection rect interaction for data selection (\ref
  dataselection "data selection mechanism").
  
  If \a onlySelectable is true, an empty QCPDataSelection is returned if this plottable is not
  selectable (i.e. if \ref QCPAbstractPlottable::setSelectable is \ref QCP::stNone).
  
  \note \a rect must be a normalized rect (positive or zero width and height). This is especially
  important when using the rect of \ref QCPSelectionRect::accepted, which is not necessarily
  normalized. Use <tt>QRect::normalized()</tt> when passing a rect which might not be normalized.
*/

/*! \fn virtual double QCPPlottableInterface1D::dataMainKey(int index) const = 0
  
  Returns the main key of the data point at the given \a index.
  
  What the main key is, is defined by the plottable's data type. See the \ref
  qcpdatacontainer-datatype "QCPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual double QCPPlottableInterface1D::dataSortKey(int index) const = 0
  
  Returns the sort key of the data point at the given \a index.
  
  What the sort key is, is defined by the plottable's data type. See the \ref
  qcpdatacontainer-datatype "QCPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual double QCPPlottableInterface1D::dataMainValue(int index) const = 0
  
  Returns the main value of the data point at the given \a index.
  
  What the main value is, is defined by the plottable's data type. See the \ref
  qcpdatacontainer-datatype "QCPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual QCPRange QCPPlottableInterface1D::dataValueRange(int index) const = 0
  
  Returns the value range of the data point at the given \a index.
  
  What the value range is, is defined by the plottable's data type. See the \ref
  qcpdatacontainer-datatype "QCPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual QPointF QCPPlottableInterface1D::dataPixelPosition(int index) const = 0

  Returns the pixel position on the widget surface at which the data point at the given \a index
  appears.

  Usually this corresponds to the point of \ref dataMainKey/\ref dataMainValue, in pixel
  coordinates. However, depending on the plottable, this might be a different apparent position
  than just a coord-to-pixel transform of those values. For example, \ref QCPBars apparent data
  values can be shifted depending on their stacking, bar grouping or configured base value.
*/

/*! \fn virtual bool QCPPlottableInterface1D::sortKeyIsMainKey() const = 0

  Returns whether the sort key (\ref dataSortKey) is identical to the main key (\ref dataMainKey).

  What the sort and main keys are, is defined by the plottable's data type. See the \ref
  qcpdatacontainer-datatype "QCPDataContainer DataType" documentation for details about this naming
  convention.
*/

/*! \fn virtual int QCPPlottableInterface1D::findBegin(double sortKey, bool expandedRange) const = 0

  Returns the index of the data point with a (sort-)key that is equal to, just below, or just above
  \a sortKey. If \a expandedRange is true, the data point just below \a sortKey will be considered,
  otherwise the one just above.

  This can be used in conjunction with \ref findEnd to iterate over data points within a given key
  range, including or excluding the bounding data points that are just beyond the specified range.

  If \a expandedRange is true but there are no data points below \a sortKey, 0 is returned.

  If the container is empty, returns 0 (in that case, \ref findEnd will also return 0, so a loop
  using these methods will not iterate over the index 0).

  \see findEnd, QCPDataContainer::findBegin
*/

/*! \fn virtual int QCPPlottableInterface1D::findEnd(double sortKey, bool expandedRange) const = 0

  Returns the index one after the data point with a (sort-)key that is equal to, just above, or
  just below \a sortKey. If \a expandedRange is true, the data point just above \a sortKey will be
  considered, otherwise the one just below.

  This can be used in conjunction with \ref findBegin to iterate over data points within a given
  key range, including the bounding data points that are just below and above the specified range.

  If \a expandedRange is true but there are no data points above \a sortKey, the index just above the
  highest data point is returned.

  If the container is empty, returns 0.

  \see findBegin, QCPDataContainer::findEnd
*/

/* end documentation of pure virtual functions */


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAbstractPlottable1D
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPAbstractPlottable1D
  \brief A template base class for plottables with one-dimensional data

  This template class derives from \ref QCPAbstractPlottable and from the abstract interface \ref
  QCPPlottableInterface1D. It serves as a base class for all one-dimensional data (i.e. data with
  one key dimension), such as \ref QCPGraph and QCPCurve.

  The template parameter \a DataType is the type of the data points of this plottable (e.g. \ref
  QCPGraphData or \ref QCPCurveData). The main purpose of this base class is to provide the member
  \a mDataContainer (a shared pointer to a \ref QCPDataContainer "QCPDataContainer<DataType>") and
  implement the according virtual methods of the \ref QCPPlottableInterface1D, such that most
  subclassed plottables don't need to worry about this anymore.

  Further, it provides a convenience method for retrieving selected/unselected data segments via
  \ref getDataSegments. This is useful when subclasses implement their \ref draw method and need to
  draw selected segments with a different pen/brush than unselected segments (also see \ref
  QCPSelectionDecorator).

  This class implements basic functionality of \ref QCPAbstractPlottable::selectTest and \ref
  QCPPlottableInterface1D::selectTestRect, assuming point-like data points, based on the 1D data
  interface. In spite of that, most plottable subclasses will want to reimplement those methods
  again, to provide a more accurate hit test based on their specific data visualization geometry.
*/

/* start documentation of inline functions */

/*! \fn QCPPlottableInterface1D *QCPAbstractPlottable1D::interface1D()
  
  Returns a \ref QCPPlottableInterface1D pointer to this plottable, providing access to its 1D
  interface.
  
  \seebaseclassmethod
*/

/* end documentation of inline functions */

/*!
  Forwards \a keyAxis and \a valueAxis to the \ref QCPAbstractPlottable::QCPAbstractPlottable
  "QCPAbstractPlottable" constructor and allocates the \a mDataContainer.
*/
template <class DataType>
QCPAbstractPlottable1D<DataType>::QCPAbstractPlottable1D(QCPAxis *keyAxis, QCPAxis *valueAxis) :
  QCPAbstractPlottable(keyAxis, valueAxis),
  mDataContainer(new QCPDataContainer<DataType>)
{
}

template <class DataType>
QCPAbstractPlottable1D<DataType>::~QCPAbstractPlottable1D()
{
}

/*!
  \copydoc QCPPlottableInterface1D::dataCount
*/
template <class DataType>
int QCPAbstractPlottable1D<DataType>::dataCount() const
{
  return mDataContainer->size();
}

/*!
  \copydoc QCPPlottableInterface1D::dataMainKey
*/
template <class DataType>
double QCPAbstractPlottable1D<DataType>::dataMainKey(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    return (mDataContainer->constBegin()+index)->mainKey();
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return 0;
  }
}

/*!
  \copydoc QCPPlottableInterface1D::dataSortKey
*/
template <class DataType>
double QCPAbstractPlottable1D<DataType>::dataSortKey(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    return (mDataContainer->constBegin()+index)->sortKey();
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return 0;
  }
}

/*!
  \copydoc QCPPlottableInterface1D::dataMainValue
*/
template <class DataType>
double QCPAbstractPlottable1D<DataType>::dataMainValue(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    return (mDataContainer->constBegin()+index)->mainValue();
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return 0;
  }
}

/*!
  \copydoc QCPPlottableInterface1D::dataValueRange
*/
template <class DataType>
QCPRange QCPAbstractPlottable1D<DataType>::dataValueRange(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    return (mDataContainer->constBegin()+index)->valueRange();
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return QCPRange(0, 0);
  }
}

/*!
  \copydoc QCPPlottableInterface1D::dataPixelPosition
*/
template <class DataType>
QPointF QCPAbstractPlottable1D<DataType>::dataPixelPosition(int index) const
{
  if (index >= 0 && index < mDataContainer->size())
  {
    const typename QCPDataContainer<DataType>::const_iterator it = mDataContainer->constBegin()+index;
    return coordsToPixels(it->mainKey(), it->mainValue());
  } else
  {
    qDebug() << Q_FUNC_INFO << "Index out of bounds" << index;
    return QPointF();
  }
}

/*!
  \copydoc QCPPlottableInterface1D::sortKeyIsMainKey
*/
template <class DataType>
bool QCPAbstractPlottable1D<DataType>::sortKeyIsMainKey() const
{
  return DataType::sortKeyIsMainKey();
}

/*!
  Implements a rect-selection algorithm assuming the data (accessed via the 1D data interface) is
  point-like. Most subclasses will want to reimplement this method again, to provide a more
  accurate hit test based on the true data visualization geometry.

  \seebaseclassmethod
*/
template <class DataType>
QCPDataSelection QCPAbstractPlottable1D<DataType>::selectTestRect(const QRectF &rect, bool onlySelectable) const
{
  QCPDataSelection result;
  if ((onlySelectable && mSelectable == QCP::stNone) || mDataContainer->isEmpty())
    return result;
  if (!mKeyAxis || !mValueAxis)
    return result;
  
  // convert rect given in pixels to ranges given in plot coordinates:
  double key1, value1, key2, value2;
  pixelsToCoords(rect.topLeft(), key1, value1);
  pixelsToCoords(rect.bottomRight(), key2, value2);
  QCPRange keyRange(key1, key2); // QCPRange normalizes internally so we don't have to care about whether key1 < key2
  QCPRange valueRange(value1, value2);
  typename QCPDataContainer<DataType>::const_iterator begin = mDataContainer->constBegin();
  typename QCPDataContainer<DataType>::const_iterator end = mDataContainer->constEnd();
  if (DataType::sortKeyIsMainKey()) // we can assume that data is sorted by main key, so can reduce the searched key interval:
  {
    begin = mDataContainer->findBegin(keyRange.lower, false);
    end = mDataContainer->findEnd(keyRange.upper, false);
  }
  if (begin == end)
    return result;
  
  int currentSegmentBegin = -1; // -1 means we're currently not in a segment that's contained in rect
  for (typename QCPDataContainer<DataType>::const_iterator it=begin; it!=end; ++it)
  {
    if (currentSegmentBegin == -1)
    {
      if (valueRange.contains(it->mainValue()) && keyRange.contains(it->mainKey())) // start segment
        currentSegmentBegin = int(it-mDataContainer->constBegin());
    } else if (!valueRange.contains(it->mainValue()) || !keyRange.contains(it->mainKey())) // segment just ended
    {
      result.addDataRange(QCPDataRange(currentSegmentBegin, int(it-mDataContainer->constBegin())), false);
      currentSegmentBegin = -1;
    }
  }
  // process potential last segment:
  if (currentSegmentBegin != -1)
    result.addDataRange(QCPDataRange(currentSegmentBegin, int(end-mDataContainer->constBegin())), false);
  
  result.simplify();
  return result;
}

/*!
  \copydoc QCPPlottableInterface1D::findBegin
*/
template <class DataType>
int QCPAbstractPlottable1D<DataType>::findBegin(double sortKey, bool expandedRange) const
{
  return int(mDataContainer->findBegin(sortKey, expandedRange)-mDataContainer->constBegin());
}

/*!
  \copydoc QCPPlottableInterface1D::findEnd
*/
template <class DataType>
int QCPAbstractPlottable1D<DataType>::findEnd(double sortKey, bool expandedRange) const
{
  return int(mDataContainer->findEnd(sortKey, expandedRange)-mDataContainer->constBegin());
}

/*!
  Implements a point-selection algorithm assuming the data (accessed via the 1D data interface) is
  point-like. Most subclasses will want to reimplement this method again, to provide a more
  accurate hit test based on the true data visualization geometry.

  If \a details is not 0, it will be set to a \ref QCPDataSelection, describing the closest data point
  to \a pos.
  
  \seebaseclassmethod
*/
template <class DataType>
double QCPAbstractPlottable1D<DataType>::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if ((onlySelectable && mSelectable == QCP::stNone) || mDataContainer->isEmpty())
    return -1;
  if (!mKeyAxis || !mValueAxis)
    return -1;
  
  QCPDataSelection selectionResult;
  double minDistSqr = (std::numeric_limits<double>::max)();
  int minDistIndex = mDataContainer->size();
  
  typename QCPDataContainer<DataType>::const_iterator begin = mDataContainer->constBegin();
  typename QCPDataContainer<DataType>::const_iterator end = mDataContainer->constEnd();
  if (DataType::sortKeyIsMainKey()) // we can assume that data is sorted by main key, so can reduce the searched key interval:
  {
    // determine which key range comes into question, taking selection tolerance around pos into account:
    double posKeyMin, posKeyMax, dummy;
    pixelsToCoords(pos-QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMin, dummy);
    pixelsToCoords(pos+QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMax, dummy);
    if (posKeyMin > posKeyMax)
      qSwap(posKeyMin, posKeyMax);
    begin = mDataContainer->findBegin(posKeyMin, true);
    end = mDataContainer->findEnd(posKeyMax, true);
  }
  if (begin == end)
    return -1;
  QCPRange keyRange(mKeyAxis->range());
  QCPRange valueRange(mValueAxis->range());
  for (typename QCPDataContainer<DataType>::const_iterator it=begin; it!=end; ++it)
  {
    const double mainKey = it->mainKey();
    const double mainValue = it->mainValue();
    if (keyRange.contains(mainKey) && valueRange.contains(mainValue)) // make sure data point is inside visible range, for speedup in cases where sort key isn't main key and we iterate over all points
    {
      const double currentDistSqr = QCPVector2D(coordsToPixels(mainKey, mainValue)-pos).lengthSquared();
      if (currentDistSqr < minDistSqr)
      {
        minDistSqr = currentDistSqr;
        minDistIndex = int(it-mDataContainer->constBegin());
      }
    }
  }
  if (minDistIndex != mDataContainer->size())
    selectionResult.addDataRange(QCPDataRange(minDistIndex, minDistIndex+1), false);
  
  selectionResult.simplify();
  if (details)
    details->setValue(selectionResult);
  return qSqrt(minDistSqr);
}

/*!
  Splits all data into selected and unselected segments and outputs them via \a selectedSegments
  and \a unselectedSegments, respectively.

  This is useful when subclasses implement their \ref draw method and need to draw selected
  segments with a different pen/brush than unselected segments (also see \ref
  QCPSelectionDecorator).

  \see setSelection
*/
template <class DataType>
void QCPAbstractPlottable1D<DataType>::getDataSegments(QList<QCPDataRange> &selectedSegments, QList<QCPDataRange> &unselectedSegments) const
{
  selectedSegments.clear();
  unselectedSegments.clear();
  if (mSelectable == QCP::stWhole) // stWhole selection type draws the entire plottable with selected style if mSelection isn't empty
  {
    if (selected())
      selectedSegments << QCPDataRange(0, dataCount());
    else
      unselectedSegments << QCPDataRange(0, dataCount());
  } else
  {
    QCPDataSelection sel(selection());
    sel.simplify();
    selectedSegments = sel.dataRanges();
    unselectedSegments = sel.inverse(QCPDataRange(0, dataCount())).dataRanges();
  }
}

/*!
  A helper method which draws a line with the passed \a painter, according to the pixel data in \a
  lineData. NaN points create gaps in the line, as expected from QCustomPlot's plottables (this is
  the main difference to QPainter's regular drawPolyline, which handles NaNs by lagging or
  crashing).

  Further it uses a faster line drawing technique based on \ref QCPPainter::drawLine rather than \c
  QPainter::drawPolyline if the configured \ref QCustomPlot::setPlottingHints() and \a painter
  style allows.
*/
template <class DataType>
void QCPAbstractPlottable1D<DataType>::drawPolyline(QCPPainter *painter, const QVector<QPointF> &lineData) const
{
  // if drawing lines in plot (instead of PDF), reduce 1px lines to cosmetic, because at least in
  // Qt6 drawing of "1px" width lines is much slower even though it has same appearance apart from
  // High-DPI. In High-DPI cases people must set a pen width slightly larger than 1.0 to get
  // correct DPI scaling of width, but of course with performance penalty.
  if (!painter->modes().testFlag(QCPPainter::pmVectorized) &&
      qFuzzyCompare(painter->pen().widthF(), 1.0))
  {
    QPen newPen = painter->pen();
    newPen.setWidth(0);
    painter->setPen(newPen);
  }

  // if drawing solid line and not in PDF, use much faster line drawing instead of polyline:
  if (mParentPlot->plottingHints().testFlag(QCP::phFastPolylines) &&
      painter->pen().style() == Qt::SolidLine &&
      !painter->modes().testFlag(QCPPainter::pmVectorized) &&
      !painter->modes().testFlag(QCPPainter::pmNoCaching))
  {
    int i = 0;
    bool lastIsNan = false;
    const int lineDataSize = lineData.size();
    while (i < lineDataSize && (qIsNaN(lineData.at(i).y()) || qIsNaN(lineData.at(i).x()))) // make sure first point is not NaN
      ++i;
    ++i; // because drawing works in 1 point retrospect
    while (i < lineDataSize)
    {
      if (!qIsNaN(lineData.at(i).y()) && !qIsNaN(lineData.at(i).x())) // NaNs create a gap in the line
      {
        if (!lastIsNan)
          painter->drawLine(lineData.at(i-1), lineData.at(i));
        else
          lastIsNan = false;
      } else
        lastIsNan = true;
      ++i;
    }
  } else
  {
    int segmentStart = 0;
    int i = 0;
    const int lineDataSize = lineData.size();
    while (i < lineDataSize)
    {
      if (qIsNaN(lineData.at(i).y()) || qIsNaN(lineData.at(i).x()) || qIsInf(lineData.at(i).y())) // NaNs create a gap in the line. Also filter Infs which make drawPolyline block
      {
        painter->drawPolyline(lineData.constData()+segmentStart, i-segmentStart); // i, because we don't want to include the current NaN point
        segmentStart = i+1;
      }
      ++i;
    }
    // draw last segment:
    painter->drawPolyline(lineData.constData()+segmentStart, lineDataSize-segmentStart);
  }
}


#endif // QCP_PLOTTABLE1D_H
