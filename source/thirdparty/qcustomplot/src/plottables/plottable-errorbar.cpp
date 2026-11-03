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

#include "plottable-errorbar.h"

#include "../painter.h"
#include "../core.h"
#include "../axis/axis.h"
#include "../layoutelements/layoutelement-axisrect.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPErrorBarsData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPErrorBarsData
  \brief Holds the data of one single error bar for QCPErrorBars.

  The stored data is:
  \li \a errorMinus: how much the error bar extends towards negative coordinates from the data
  point position
  \li \a errorPlus: how much the error bar extends towards positive coordinates from the data point
  position

  The container for storing the error bar information is \ref QCPErrorBarsDataContainer. It is a
  typedef for <tt>QVector<\ref QCPErrorBarsData></tt>.

  \see QCPErrorBarsDataContainer
*/

/*!
  Constructs an error bar with errors set to zero.
*/
QCPErrorBarsData::QCPErrorBarsData() :
  errorMinus(0),
  errorPlus(0)
{
}

/*!
  Constructs an error bar with equal \a error in both negative and positive direction.
*/
QCPErrorBarsData::QCPErrorBarsData(double error) :
  errorMinus(error),
  errorPlus(error)
{
}

/*!
  Constructs an error bar with negative and positive errors set to \a errorMinus and \a errorPlus,
  respectively.
*/
QCPErrorBarsData::QCPErrorBarsData(double errorMinus, double errorPlus) :
  errorMinus(errorMinus),
  errorPlus(errorPlus)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPErrorBars
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPErrorBars
  \brief A plottable that adds a set of error bars to other plottables.

  \image html QCPErrorBars.png

  The \ref QCPErrorBars plottable can be attached to other one-dimensional plottables (e.g. \ref
  QCPGraph, \ref QCPCurve, \ref QCPBars, etc.) and equips them with error bars.

  Use \ref setDataPlottable to define for which plottable the \ref QCPErrorBars shall display the
  error bars. The orientation of the error bars can be controlled with \ref setErrorType.

  By using \ref setData, you can supply the actual error data, either as symmetric error or
  plus/minus asymmetric errors. \ref QCPErrorBars only stores the error data. The absolute
  key/value position of each error bar will be adopted from the configured data plottable. The
  error data of the \ref QCPErrorBars are associated one-to-one via their index to the data points
  of the data plottable. You can directly access and manipulate the error bar data via \ref data.

  Set either of the plus/minus errors to NaN (<tt>qQNaN()</tt> or
  <tt>std::numeric_limits<double>::quiet_NaN()</tt>) to not show the respective error bar on the data point at
  that index.

  \section qcperrorbars-appearance Changing the appearance

  The appearance of the error bars is defined by the pen (\ref setPen), and the width of the
  whiskers (\ref setWhiskerWidth). Further, the error bar backbones may leave a gap around the data
  point center to prevent that error bars are drawn too close to or even through scatter points.
  This gap size can be controlled via \ref setSymbolGap.
*/

/* start of documentation of inline functions */

/*! \fn QSharedPointer<QCPErrorBarsDataContainer> QCPErrorBars::data() const

  Returns a shared pointer to the internal data storage of type \ref QCPErrorBarsDataContainer. You
  may use it to directly manipulate the error values, which may be more convenient and faster than
  using the regular \ref setData methods.
*/

/* end of documentation of inline functions */

/*!
  Constructs an error bars plottable which uses \a keyAxis as its key axis ("x") and \a valueAxis as its value
  axis ("y"). \a keyAxis and \a valueAxis must reside in the same QCustomPlot instance and not have
  the same orientation. If either of these restrictions is violated, a corresponding message is
  printed to the debug output (qDebug), the construction is not aborted, though.

  It is also important that the \a keyAxis and \a valueAxis are the same for the error bars
  plottable and the data plottable that the error bars shall be drawn on (\ref setDataPlottable).

  The created \ref QCPErrorBars is automatically registered with the QCustomPlot instance inferred
  from \a keyAxis. This QCustomPlot instance takes ownership of the \ref QCPErrorBars, so do not
  delete it manually but use \ref QCustomPlot::removePlottable() instead.
*/
QCPErrorBars::QCPErrorBars(QCPAxis *keyAxis, QCPAxis *valueAxis) :
  QCPAbstractPlottable(keyAxis, valueAxis),
  mDataContainer(new QVector<QCPErrorBarsData>),
  mErrorType(etValueError),
  mWhiskerWidth(9),
  mSymbolGap(10)
{
  setPen(QPen(Qt::black, 0));
  setBrush(Qt::NoBrush);
}

QCPErrorBars::~QCPErrorBars()
{
}

/*! \overload

  Replaces the current data container with the provided \a data container.

  Since a QSharedPointer is used, multiple \ref QCPErrorBars instances may share the same data
  container safely. Modifying the data in the container will then affect all \ref QCPErrorBars
  instances that share the container. Sharing can be achieved by simply exchanging the data
  containers wrapped in shared pointers:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcperrorbars-datasharing-1

  If you do not wish to share containers, but create a copy from an existing container, assign the
  data containers directly:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcperrorbars-datasharing-2
  (This uses different notation compared with other plottables, because the \ref QCPErrorBars
  uses a \c QVector<QCPErrorBarsData> as its data container, instead of a \ref QCPDataContainer.)

  \see addData
*/
void QCPErrorBars::setData(QSharedPointer<QCPErrorBarsDataContainer> data)
{
  mDataContainer = data;
}

/*! \overload

  Sets symmetrical error values as specified in \a error. The errors will be associated one-to-one
  by the data point index to the associated data plottable (\ref setDataPlottable).

  You can directly access and manipulate the error bar data via \ref data.

  \see addData
*/
void QCPErrorBars::setData(const QVector<double> &error)
{
  mDataContainer->clear();
  addData(error);
}

/*! \overload

  Sets asymmetrical errors as specified in \a errorMinus and \a errorPlus. The errors will be
  associated one-to-one by the data point index to the associated data plottable (\ref
  setDataPlottable).

  You can directly access and manipulate the error bar data via \ref data.

  \see addData
*/
void QCPErrorBars::setData(const QVector<double> &errorMinus, const QVector<double> &errorPlus)
{
  mDataContainer->clear();
  addData(errorMinus, errorPlus);
}

/*!
  Sets the data plottable to which the error bars will be applied. The error values specified e.g.
  via \ref setData will be associated one-to-one by the data point index to the data points of \a
  plottable. This means that the error bars will adopt the key/value coordinates of the data point
  with the same index.

  The passed \a plottable must be a one-dimensional plottable, i.e. it must implement the \ref
  QCPPlottableInterface1D. Further, it must not be a \ref QCPErrorBars instance itself. If either
  of these restrictions is violated, a corresponding qDebug output is generated, and the data
  plottable of this \ref QCPErrorBars instance is set to zero.

  For proper display, care must also be taken that the key and value axes of the \a plottable match
  those configured for this \ref QCPErrorBars instance.
*/
void QCPErrorBars::setDataPlottable(QCPAbstractPlottable *plottable)
{
  if (plottable && qobject_cast<QCPErrorBars*>(plottable))
  {
    mDataPlottable = nullptr;
    qDebug() << Q_FUNC_INFO << "can't set another QCPErrorBars instance as data plottable";
    return;
  }
  if (plottable && !plottable->interface1D())
  {
    mDataPlottable = nullptr;
    qDebug() << Q_FUNC_INFO << "passed plottable doesn't implement 1d interface, can't associate with QCPErrorBars";
    return;
  }
  
  mDataPlottable = plottable;
}

/*!
  Sets in which orientation the error bars shall appear on the data points. If your data needs both
  error dimensions, create two \ref QCPErrorBars with different \a type.
*/
void QCPErrorBars::setErrorType(ErrorType type)
{
  mErrorType = type;
}

/*!
  Sets the width of the whiskers (the short bars at the end of the actual error bar backbones) to
  \a pixels.
*/
void QCPErrorBars::setWhiskerWidth(double pixels)
{
  mWhiskerWidth = pixels;
}

/*!
  Sets the gap diameter around the data points that will be left out when drawing the error bar
  backbones. This gap prevents that error bars are drawn too close to or even through scatter
  points.
*/
void QCPErrorBars::setSymbolGap(double pixels)
{
  mSymbolGap = pixels;
}

/*! \overload

  Adds symmetrical error values as specified in \a error. The errors will be associated one-to-one
  by the data point index to the associated data plottable (\ref setDataPlottable).

  You can directly access and manipulate the error bar data via \ref data.

  \see setData
*/
void QCPErrorBars::addData(const QVector<double> &error)
{
  addData(error, error);
}

/*! \overload

  Adds asymmetrical errors as specified in \a errorMinus and \a errorPlus. The errors will be
  associated one-to-one by the data point index to the associated data plottable (\ref
  setDataPlottable).

  You can directly access and manipulate the error bar data via \ref data.

  \see setData
*/
void QCPErrorBars::addData(const QVector<double> &errorMinus, const QVector<double> &errorPlus)
{
  if (errorMinus.size() != errorPlus.size())
    qDebug() << Q_FUNC_INFO << "minus and plus error vectors have different sizes:" << errorMinus.size() << errorPlus.size();
  const int n = qMin(errorMinus.size(), errorPlus.size());
  mDataContainer->reserve(n);
  for (int i=0; i<n; ++i)
    mDataContainer->append(QCPErrorBarsData(errorMinus.at(i), errorPlus.at(i)));
}

/*! \overload

  Adds a single symmetrical error bar as specified in \a error. The errors will be associated
  one-to-one by the data point index to the associated data plottable (\ref setDataPlottable).

  You can directly access and manipulate the error bar data via \ref data.

  \see setData
*/
void QCPErrorBars::addData(double error)
{
  mDataContainer->append(QCPErrorBarsData(error));
}

/*! \overload

  Adds a single asymmetrical error bar as specified in \a errorMinus and \a errorPlus. The errors
  will be associated one-to-one by the data point index to the associated data plottable (\ref
  setDataPlottable).

  You can directly access and manipulate the error bar data via \ref data.

  \see setData
*/
void QCPErrorBars::addData(double errorMinus, double errorPlus)
{
  mDataContainer->append(QCPErrorBarsData(errorMinus, errorPlus));
}

/* inherits documentation from base class */
int QCPErrorBars::dataCount() const
{
  return mDataContainer->size();
}

/* inherits documentation from base class */
double QCPErrorBars::dataMainKey(int index) const
{
  if (mDataPlottable)
    return mDataPlottable->interface1D()->dataMainKey(index);
  else
    qDebug() << Q_FUNC_INFO << "no data plottable set";
  return 0;
}

/* inherits documentation from base class */
double QCPErrorBars::dataSortKey(int index) const
{
  if (mDataPlottable)
    return mDataPlottable->interface1D()->dataSortKey(index);
  else
    qDebug() << Q_FUNC_INFO << "no data plottable set";
  return 0;
}

/* inherits documentation from base class */
double QCPErrorBars::dataMainValue(int index) const
{
  if (mDataPlottable)
    return mDataPlottable->interface1D()->dataMainValue(index);
  else
    qDebug() << Q_FUNC_INFO << "no data plottable set";
  return 0;
}

/* inherits documentation from base class */
QCPRange QCPErrorBars::dataValueRange(int index) const
{
  if (mDataPlottable)
  {
    const double value = mDataPlottable->interface1D()->dataMainValue(index);
    if (index >= 0 && index < mDataContainer->size() && mErrorType == etValueError)
      return {value-mDataContainer->at(index).errorMinus, value+mDataContainer->at(index).errorPlus};
    else
      return {value, value};
  } else
  {
    qDebug() << Q_FUNC_INFO << "no data plottable set";
    return {};
  }
}

/* inherits documentation from base class */
QPointF QCPErrorBars::dataPixelPosition(int index) const
{
  if (mDataPlottable)
    return mDataPlottable->interface1D()->dataPixelPosition(index);
  else
    qDebug() << Q_FUNC_INFO << "no data plottable set";
  return {};
}

/* inherits documentation from base class */
bool QCPErrorBars::sortKeyIsMainKey() const
{
  if (mDataPlottable)
  {
    return mDataPlottable->interface1D()->sortKeyIsMainKey();
  } else
  {
    qDebug() << Q_FUNC_INFO << "no data plottable set";
    return true;
  }
}

/*!
  \copydoc QCPPlottableInterface1D::selectTestRect
*/
QCPDataSelection QCPErrorBars::selectTestRect(const QRectF &rect, bool onlySelectable) const
{
  QCPDataSelection result;
  if (!mDataPlottable)
    return result;
  if ((onlySelectable && mSelectable == QCP::stNone) || mDataContainer->isEmpty())
    return result;
  if (!mKeyAxis || !mValueAxis)
    return result;
  
  QCPErrorBarsDataContainer::const_iterator visibleBegin, visibleEnd;
  getVisibleDataBounds(visibleBegin, visibleEnd, QCPDataRange(0, dataCount()));
  
  QVector<QLineF> backbones, whiskers;
  for (QCPErrorBarsDataContainer::const_iterator it=visibleBegin; it!=visibleEnd; ++it)
  {
    backbones.clear();
    whiskers.clear();
    getErrorBarLines(it, backbones, whiskers);
    foreach (const QLineF &backbone, backbones)
    {
      if (rectIntersectsLine(rect, backbone))
      {
        result.addDataRange(QCPDataRange(int(it-mDataContainer->constBegin()), int(it-mDataContainer->constBegin()+1)), false);
        break;
      }
    }
  }
  result.simplify();
  return result;
}

/* inherits documentation from base class */
int QCPErrorBars::findBegin(double sortKey, bool expandedRange) const
{
  if (mDataPlottable)
  {
    if (mDataContainer->isEmpty())
      return 0;
    int beginIndex = mDataPlottable->interface1D()->findBegin(sortKey, expandedRange);
    if (beginIndex >= mDataContainer->size())
      beginIndex = mDataContainer->size()-1;
    return beginIndex;
  } else
    qDebug() << Q_FUNC_INFO << "no data plottable set";
  return 0;
}

/* inherits documentation from base class */
int QCPErrorBars::findEnd(double sortKey, bool expandedRange) const
{
  if (mDataPlottable)
  {
    if (mDataContainer->isEmpty())
      return 0;
    int endIndex = mDataPlottable->interface1D()->findEnd(sortKey, expandedRange);
    if (endIndex > mDataContainer->size())
      endIndex = mDataContainer->size();
    return endIndex;
  } else
    qDebug() << Q_FUNC_INFO << "no data plottable set";
  return 0;
}

/*!
  Implements a selectTest specific to this plottable's point geometry.

  If \a details is not 0, it will be set to a \ref QCPDataSelection, describing the closest data
  point to \a pos.
  
  \seebaseclassmethod \ref QCPAbstractPlottable::selectTest
*/
double QCPErrorBars::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if (!mDataPlottable) return -1;
  
  if ((onlySelectable && mSelectable == QCP::stNone) || mDataContainer->isEmpty())
    return -1;
  if (!mKeyAxis || !mValueAxis)
    return -1;
  
  if (mKeyAxis.data()->axisRect()->rect().contains(pos.toPoint()) || mParentPlot->interactions().testFlag(QCP::iSelectPlottablesBeyondAxisRect))
  {
    QCPErrorBarsDataContainer::const_iterator closestDataPoint = mDataContainer->constEnd();
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
void QCPErrorBars::draw(QCPPainter *painter)
{
  if (!mDataPlottable) return;
  if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  if (mKeyAxis.data()->range().size() <= 0 || mDataContainer->isEmpty()) return;
  
  // if the sort key isn't the main key, we must check the visibility for each data point/error bar individually
  // (getVisibleDataBounds applies range restriction, but otherwise can only return full data range):
  bool checkPointVisibility = !mDataPlottable->interface1D()->sortKeyIsMainKey();
      
    // check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA
  QCPErrorBarsDataContainer::const_iterator it;
  for (it = mDataContainer->constBegin(); it != mDataContainer->constEnd(); ++it)
  {
    if (QCP::isInvalidData(it->errorMinus, it->errorPlus))
      qDebug() << Q_FUNC_INFO << "Data point at index" << it-mDataContainer->constBegin() << "invalid." << "Plottable name:" << name();
  }
#endif
  
  applyDefaultAntialiasingHint(painter);
  painter->setBrush(Qt::NoBrush);
  // loop over and draw segments of unselected/selected data:
  QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  allSegments << unselectedSegments << selectedSegments;
  QVector<QLineF> backbones, whiskers;
  for (int i=0; i<allSegments.size(); ++i)
  {
    QCPErrorBarsDataContainer::const_iterator begin, end;
    getVisibleDataBounds(begin, end, allSegments.at(i));
    if (begin == end)
      continue;
    
    bool isSelectedSegment = i >= unselectedSegments.size();
    if (isSelectedSegment && mSelectionDecorator)
      mSelectionDecorator->applyPen(painter);
    else
      painter->setPen(mPen);
    if (painter->pen().capStyle() == Qt::SquareCap)
    {
      QPen capFixPen(painter->pen());
      capFixPen.setCapStyle(Qt::FlatCap);
      painter->setPen(capFixPen);
    }
    backbones.clear();
    whiskers.clear();
    for (QCPErrorBarsDataContainer::const_iterator it=begin; it!=end; ++it)
    {
      if (!checkPointVisibility || errorBarVisible(int(it-mDataContainer->constBegin())))
        getErrorBarLines(it, backbones, whiskers);
    }
    painter->drawLines(backbones);
    painter->drawLines(whiskers);
  }
  
  // draw other selection decoration that isn't just line/scatter pens and brushes:
  if (mSelectionDecorator)
    mSelectionDecorator->drawDecoration(painter, selection());
}

/* inherits documentation from base class */
void QCPErrorBars::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
  applyDefaultAntialiasingHint(painter);
  painter->setPen(mPen);
  if (mErrorType == etValueError && mValueAxis && mValueAxis->orientation() == Qt::Vertical)
  {
    painter->drawLine(QLineF(rect.center().x(), rect.top()+2, rect.center().x(), rect.bottom()-1));
    painter->drawLine(QLineF(rect.center().x()-4, rect.top()+2, rect.center().x()+4, rect.top()+2));
    painter->drawLine(QLineF(rect.center().x()-4, rect.bottom()-1, rect.center().x()+4, rect.bottom()-1));
  } else
  {
    painter->drawLine(QLineF(rect.left()+2, rect.center().y(), rect.right()-2, rect.center().y()));
    painter->drawLine(QLineF(rect.left()+2, rect.center().y()-4, rect.left()+2, rect.center().y()+4));
    painter->drawLine(QLineF(rect.right()-2, rect.center().y()-4, rect.right()-2, rect.center().y()+4));
  }
}

/* inherits documentation from base class */
QCPRange QCPErrorBars::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
  if (!mDataPlottable)
  {
    foundRange = false;
    return {};
  }
  
  QCPRange range;
  bool haveLower = false;
  bool haveUpper = false;
  QCPErrorBarsDataContainer::const_iterator it;
  for (it = mDataContainer->constBegin(); it != mDataContainer->constEnd(); ++it)
  {
    if (mErrorType == etValueError)
    {
      // error bar doesn't extend in key dimension (except whisker but we ignore that here), so only use data point center
      const double current = mDataPlottable->interface1D()->dataMainKey(int(it-mDataContainer->constBegin()));
      if (qIsNaN(current)) continue;
      if (inSignDomain == QCP::sdBoth || (inSignDomain == QCP::sdNegative && current < 0) || (inSignDomain == QCP::sdPositive && current > 0))
      {
        if (current < range.lower || !haveLower)
        {
          range.lower = current;
          haveLower = true;
        }
        if (current > range.upper || !haveUpper)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
    } else // mErrorType == etKeyError
    {
      const double dataKey = mDataPlottable->interface1D()->dataMainKey(int(it-mDataContainer->constBegin()));
      if (qIsNaN(dataKey)) continue;
      // plus error:
      double current = dataKey + (qIsNaN(it->errorPlus) ? 0 : it->errorPlus);
      if (inSignDomain == QCP::sdBoth || (inSignDomain == QCP::sdNegative && current < 0) || (inSignDomain == QCP::sdPositive && current > 0))
      {
        if (current > range.upper || !haveUpper)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
      // minus error:
      current = dataKey - (qIsNaN(it->errorMinus) ? 0 : it->errorMinus);
      if (inSignDomain == QCP::sdBoth || (inSignDomain == QCP::sdNegative && current < 0) || (inSignDomain == QCP::sdPositive && current > 0))
      {
        if (current < range.lower || !haveLower)
        {
          range.lower = current;
          haveLower = true;
        }
      }
    }
  }
  
  if (haveUpper && !haveLower)
  {
    range.lower = range.upper;
    haveLower = true;
  } else if (haveLower && !haveUpper)
  {
    range.upper = range.lower;
    haveUpper = true;
  }
  
  foundRange = haveLower && haveUpper;
  return range;
}

/* inherits documentation from base class */
QCPRange QCPErrorBars::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain, const QCPRange &inKeyRange) const
{
  if (!mDataPlottable)
  {
    foundRange = false;
    return {};
  }
  
  QCPRange range;
  const bool restrictKeyRange = inKeyRange != QCPRange();
  bool haveLower = false;
  bool haveUpper = false;
  QCPErrorBarsDataContainer::const_iterator itBegin = mDataContainer->constBegin();
  QCPErrorBarsDataContainer::const_iterator itEnd = mDataContainer->constEnd();
  if (mDataPlottable->interface1D()->sortKeyIsMainKey() && restrictKeyRange)
  {
    itBegin = mDataContainer->constBegin()+findBegin(inKeyRange.lower, false);
    itEnd = mDataContainer->constBegin()+findEnd(inKeyRange.upper, false);
  }
  for (QCPErrorBarsDataContainer::const_iterator it = itBegin; it != itEnd; ++it)
  {
    if (restrictKeyRange)
    {
      const double dataKey = mDataPlottable->interface1D()->dataMainKey(int(it-mDataContainer->constBegin()));
      if (dataKey < inKeyRange.lower || dataKey > inKeyRange.upper)
        continue;
    }
    if (mErrorType == etValueError)
    {
      const double dataValue = mDataPlottable->interface1D()->dataMainValue(int(it-mDataContainer->constBegin()));
      if (qIsNaN(dataValue)) continue;
      // plus error:
      double current = dataValue + (qIsNaN(it->errorPlus) ? 0 : it->errorPlus);
      if (inSignDomain == QCP::sdBoth || (inSignDomain == QCP::sdNegative && current < 0) || (inSignDomain == QCP::sdPositive && current > 0))
      {
        if (current > range.upper || !haveUpper)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
      // minus error:
      current = dataValue - (qIsNaN(it->errorMinus) ? 0 : it->errorMinus);
      if (inSignDomain == QCP::sdBoth || (inSignDomain == QCP::sdNegative && current < 0) || (inSignDomain == QCP::sdPositive && current > 0))
      {
        if (current < range.lower || !haveLower)
        {
          range.lower = current;
          haveLower = true;
        }
      }
    } else // mErrorType == etKeyError
    {
      // error bar doesn't extend in value dimension (except whisker but we ignore that here), so only use data point center
      const double current = mDataPlottable->interface1D()->dataMainValue(int(it-mDataContainer->constBegin()));
      if (qIsNaN(current)) continue;
      if (inSignDomain == QCP::sdBoth || (inSignDomain == QCP::sdNegative && current < 0) || (inSignDomain == QCP::sdPositive && current > 0))
      {
        if (current < range.lower || !haveLower)
        {
          range.lower = current;
          haveLower = true;
        }
        if (current > range.upper || !haveUpper)
        {
          range.upper = current;
          haveUpper = true;
        }
      }
    }
  }
  
  if (haveUpper && !haveLower)
  {
    range.lower = range.upper;
    haveLower = true;
  } else if (haveLower && !haveUpper)
  {
    range.upper = range.lower;
    haveUpper = true;
  }
  
  foundRange = haveLower && haveUpper;
  return range;
}

/*! \internal

  Calculates the lines that make up the error bar belonging to the data point \a it.

  The resulting lines are added to \a backbones and \a whiskers. The vectors are not cleared, so
  calling this method with different \a it but the same \a backbones and \a whiskers allows to
  accumulate lines for multiple data points.

  This method assumes that \a it is a valid iterator within the bounds of this \ref QCPErrorBars
  instance and within the bounds of the associated data plottable.
*/
void QCPErrorBars::getErrorBarLines(QCPErrorBarsDataContainer::const_iterator it, QVector<QLineF> &backbones, QVector<QLineF> &whiskers) const
{
  if (!mDataPlottable) return;
  
  int index = int(it-mDataContainer->constBegin());
  QPointF centerPixel = mDataPlottable->interface1D()->dataPixelPosition(index);
  if (qIsNaN(centerPixel.x()) || qIsNaN(centerPixel.y()))
    return;
  QCPAxis *errorAxis = mErrorType == etValueError ? mValueAxis.data() : mKeyAxis.data();
  QCPAxis *orthoAxis = mErrorType == etValueError ? mKeyAxis.data() : mValueAxis.data();
  const double centerErrorAxisPixel = errorAxis->orientation() == Qt::Horizontal ? centerPixel.x() : centerPixel.y();
  const double centerOrthoAxisPixel = orthoAxis->orientation() == Qt::Horizontal ? centerPixel.x() : centerPixel.y();
  const double centerErrorAxisCoord = errorAxis->pixelToCoord(centerErrorAxisPixel); // depending on plottable, this might be different from just mDataPlottable->interface1D()->dataMainKey/Value
  const double symbolGap = mSymbolGap*0.5*errorAxis->pixelOrientation();
  // plus error:
  double errorStart, errorEnd;
  if (!qIsNaN(it->errorPlus))
  {
    errorStart = centerErrorAxisPixel+symbolGap;
    errorEnd = errorAxis->coordToPixel(centerErrorAxisCoord+it->errorPlus);
    if (errorAxis->orientation() == Qt::Vertical)
    {
      if ((errorStart > errorEnd) != errorAxis->rangeReversed())
        backbones.append(QLineF(centerOrthoAxisPixel, errorStart, centerOrthoAxisPixel, errorEnd));
      whiskers.append(QLineF(centerOrthoAxisPixel-mWhiskerWidth*0.5, errorEnd, centerOrthoAxisPixel+mWhiskerWidth*0.5, errorEnd));
    } else
    {
      if ((errorStart < errorEnd) != errorAxis->rangeReversed())
        backbones.append(QLineF(errorStart, centerOrthoAxisPixel, errorEnd, centerOrthoAxisPixel));
      whiskers.append(QLineF(errorEnd, centerOrthoAxisPixel-mWhiskerWidth*0.5, errorEnd, centerOrthoAxisPixel+mWhiskerWidth*0.5));
    }
  }
  // minus error:
  if (!qIsNaN(it->errorMinus))
  {
    errorStart = centerErrorAxisPixel-symbolGap;
    errorEnd = errorAxis->coordToPixel(centerErrorAxisCoord-it->errorMinus);
    if (errorAxis->orientation() == Qt::Vertical)
    {
      if ((errorStart < errorEnd) != errorAxis->rangeReversed())
        backbones.append(QLineF(centerOrthoAxisPixel, errorStart, centerOrthoAxisPixel, errorEnd));
      whiskers.append(QLineF(centerOrthoAxisPixel-mWhiskerWidth*0.5, errorEnd, centerOrthoAxisPixel+mWhiskerWidth*0.5, errorEnd));
    } else
    {
      if ((errorStart > errorEnd) != errorAxis->rangeReversed())
        backbones.append(QLineF(errorStart, centerOrthoAxisPixel, errorEnd, centerOrthoAxisPixel));
      whiskers.append(QLineF(errorEnd, centerOrthoAxisPixel-mWhiskerWidth*0.5, errorEnd, centerOrthoAxisPixel+mWhiskerWidth*0.5));
    }
  }
}

/*! \internal

  This method outputs the currently visible data range via \a begin and \a end. The returned range
  will also never exceed \a rangeRestriction.

  Since error bars with type \ref etKeyError may extend to arbitrarily positive and negative key
  coordinates relative to their data point key, this method checks all outer error bars whether
  they truly don't reach into the visible portion of the axis rect, by calling \ref
  errorBarVisible. On the other hand error bars with type \ref etValueError that are associated
  with data plottables whose sort key is equal to the main key (see \ref qcpdatacontainer-datatype
  "QCPDataContainer DataType") can be handled very efficiently by finding the visible range of
  error bars through binary search (\ref QCPPlottableInterface1D::findBegin and \ref
  QCPPlottableInterface1D::findEnd).

  If the plottable's sort key is not equal to the main key, this method returns the full data
  range, only restricted by \a rangeRestriction. Drawing optimization then has to be done on a
  point-by-point basis in the \ref draw method.
*/
void QCPErrorBars::getVisibleDataBounds(QCPErrorBarsDataContainer::const_iterator &begin, QCPErrorBarsDataContainer::const_iterator &end, const QCPDataRange &rangeRestriction) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis)
  {
    qDebug() << Q_FUNC_INFO << "invalid key or value axis";
    end = mDataContainer->constEnd();
    begin = end;
    return;
  }
  if (!mDataPlottable || rangeRestriction.isEmpty())
  {
    end = mDataContainer->constEnd();
    begin = end;
    return;
  }
  if (!mDataPlottable->interface1D()->sortKeyIsMainKey())
  {
    // if the sort key isn't the main key, it's not possible to find a contiguous range of visible
    // data points, so this method then only applies the range restriction and otherwise returns
    // the full data range. Visibility checks must be done on a per-datapoin-basis during drawing
    QCPDataRange dataRange(0, mDataContainer->size());
    dataRange = dataRange.bounded(rangeRestriction);
    begin = mDataContainer->constBegin()+dataRange.begin();
    end = mDataContainer->constBegin()+dataRange.end();
    return;
  }
  
  // get visible data range via interface from data plottable, and then restrict to available error data points:
  const int n = qMin(mDataContainer->size(), mDataPlottable->interface1D()->dataCount());
  int beginIndex = mDataPlottable->interface1D()->findBegin(keyAxis->range().lower);
  int endIndex = mDataPlottable->interface1D()->findEnd(keyAxis->range().upper);
  int i = beginIndex;
  while (i > 0 && i < n && i > rangeRestriction.begin())
  {
    if (errorBarVisible(i))
      beginIndex = i;
    --i;
  }
  i = endIndex;
  while (i >= 0 && i < n && i < rangeRestriction.end())
  {
    if (errorBarVisible(i))
      endIndex = i+1;
    ++i;
  }
  QCPDataRange dataRange(beginIndex, endIndex);
  dataRange = dataRange.bounded(rangeRestriction.bounded(QCPDataRange(0, mDataContainer->size())));
  begin = mDataContainer->constBegin()+dataRange.begin();
  end = mDataContainer->constBegin()+dataRange.end();
}

/*! \internal

  Calculates the minimum distance in pixels the error bars' representation has from the given \a
  pixelPoint. This is used to determine whether the error bar was clicked or not, e.g. in \ref
  selectTest. The closest data point to \a pixelPoint is returned in \a closestData.
*/
double QCPErrorBars::pointDistance(const QPointF &pixelPoint, QCPErrorBarsDataContainer::const_iterator &closestData) const
{
  closestData = mDataContainer->constEnd();
  if (!mDataPlottable || mDataContainer->isEmpty())
    return -1.0;
  if (!mKeyAxis || !mValueAxis)
  {
    qDebug() << Q_FUNC_INFO << "invalid key or value axis";
    return -1.0;
  }
  
  QCPErrorBarsDataContainer::const_iterator begin, end;
  getVisibleDataBounds(begin, end, QCPDataRange(0, dataCount()));
  
  // calculate minimum distances to error backbones (whiskers are ignored for speed) and find closestData iterator:
  double minDistSqr = (std::numeric_limits<double>::max)();
  QVector<QLineF> backbones, whiskers;
  for (QCPErrorBarsDataContainer::const_iterator it=begin; it!=end; ++it)
  {
    getErrorBarLines(it, backbones, whiskers);
    foreach (const QLineF &backbone, backbones)
    {
      const double currentDistSqr = QCPVector2D(pixelPoint).distanceSquaredToLine(backbone);
      if (currentDistSqr < minDistSqr)
      {
        minDistSqr = currentDistSqr;
        closestData = it;
      }
    }
  }
  return qSqrt(minDistSqr);
}

/*! \internal

  \note This method is identical to \ref QCPAbstractPlottable1D::getDataSegments but needs to be
  reproduced here since the \ref QCPErrorBars plottable, as a special case that doesn't have its
  own key/value data coordinates, doesn't derive from \ref QCPAbstractPlottable1D. See the
  documentation there for details.
*/
void QCPErrorBars::getDataSegments(QList<QCPDataRange> &selectedSegments, QList<QCPDataRange> &unselectedSegments) const
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

/*! \internal

  Returns whether the error bar at the specified \a index is visible within the current key axis
  range.

  This method assumes for performance reasons without checking that the key axis, the value axis,
  and the data plottable (\ref setDataPlottable) are not \c nullptr and that \a index is within
  valid bounds of this \ref QCPErrorBars instance and the bounds of the data plottable.
*/
bool QCPErrorBars::errorBarVisible(int index) const
{
  QPointF centerPixel = mDataPlottable->interface1D()->dataPixelPosition(index);
  const double centerKeyPixel = mKeyAxis->orientation() == Qt::Horizontal ? centerPixel.x() : centerPixel.y();
  if (qIsNaN(centerKeyPixel))
    return false;
  
  double keyMin, keyMax;
  if (mErrorType == etKeyError)
  {
    const double centerKey = mKeyAxis->pixelToCoord(centerKeyPixel);
    const double errorPlus = mDataContainer->at(index).errorPlus;
    const double errorMinus = mDataContainer->at(index).errorMinus;
    keyMax = centerKey+(qIsNaN(errorPlus) ? 0 : errorPlus);
    keyMin = centerKey-(qIsNaN(errorMinus) ? 0 : errorMinus);
  } else // mErrorType == etValueError
  {
    keyMax = mKeyAxis->pixelToCoord(centerKeyPixel+mWhiskerWidth*0.5*mKeyAxis->pixelOrientation());
    keyMin = mKeyAxis->pixelToCoord(centerKeyPixel-mWhiskerWidth*0.5*mKeyAxis->pixelOrientation());
  }
  return ((keyMax > mKeyAxis->range().lower) && (keyMin < mKeyAxis->range().upper));
}

/*! \internal

  Returns whether \a line intersects (or is contained in) \a pixelRect.

  \a line is assumed to be either perfectly horizontal or perfectly vertical, as is the case for
  error bar lines.
*/
bool QCPErrorBars::rectIntersectsLine(const QRectF &pixelRect, const QLineF &line) const
{
  if (pixelRect.left() > line.x1() && pixelRect.left() > line.x2())
    return false;
  else if (pixelRect.right() < line.x1() && pixelRect.right() < line.x2())
    return false;
  else if (pixelRect.top() > line.y1() && pixelRect.top() > line.y2())
    return false;
  else if (pixelRect.bottom() < line.y1() && pixelRect.bottom() < line.y2())
    return false;
  else
    return true;
}
