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

#include "axistickertext.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAxisTickerText
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class QCPAxisTickerText
  \brief Specialized axis ticker which allows arbitrary labels at specified coordinates
  
  \image html axisticker-text.png
  
  This QCPAxisTicker subclass generates ticks which can be directly specified by the user as
  coordinates and associated strings. They can be passed as a whole with \ref setTicks or one at a
  time with \ref addTick. Alternatively you can directly access the internal storage via \ref ticks
  and modify the tick/label data there.
  
  This is useful for cases where the axis represents categories rather than numerical values.
  
  If you are updating the ticks of this ticker regularly and in a dynamic fasion (e.g. dependent on
  the axis range), it is a sign that you should probably create an own ticker by subclassing
  QCPAxisTicker, instead of using this one.
  
  The ticker can be created and assigned to an axis like this:
  \snippet documentation/doc-image-generator/mainwindow.cpp axistickertext-creation
*/

/* start of documentation of inline functions */

/*! \fn QMap<double, QString> &QCPAxisTickerText::ticks()
  
  Returns a non-const reference to the internal map which stores the tick coordinates and their
  labels.

  You can access the map directly in order to add, remove or manipulate ticks, as an alternative to
  using the methods provided by QCPAxisTickerText, such as \ref setTicks and \ref addTick.
*/

/* end of documentation of inline functions */

/*!
  Constructs the ticker and sets reasonable default values. Axis tickers are commonly created
  managed by a QSharedPointer, which then can be passed to QCPAxis::setTicker.
*/
QCPAxisTickerText::QCPAxisTickerText() :
  mSubTickCount(0)
{
}

/*! \overload
  
  Sets the ticks that shall appear on the axis. The map key of \a ticks corresponds to the axis
  coordinate, and the map value is the string that will appear as tick label.
  
  An alternative to manipulate ticks is to directly access the internal storage with the \ref ticks
  getter.
  
  \see addTicks, addTick, clear
*/
void QCPAxisTickerText::setTicks(const QMap<double, QString> &ticks)
{
  mTicks = ticks;
}

/*! \overload
  
  Sets the ticks that shall appear on the axis. The entries of \a positions correspond to the axis
  coordinates, and the entries of \a labels are the respective strings that will appear as tick
  labels.
  
  \see addTicks, addTick, clear
*/
void QCPAxisTickerText::setTicks(const QVector<double> &positions, const QVector<QString> &labels)
{
  clear();
  addTicks(positions, labels);
}

/*!
  Sets the number of sub ticks that shall appear between ticks. For QCPAxisTickerText, there is no
  automatic sub tick count calculation. So if sub ticks are needed, they must be configured with this
  method.
*/
void QCPAxisTickerText::setSubTickCount(int subTicks)
{
  if (subTicks >= 0)
    mSubTickCount = subTicks;
  else
    qDebug() << Q_FUNC_INFO << "sub tick count can't be negative:" << subTicks;
}

/*!
  Clears all ticks.
  
  An alternative to manipulate ticks is to directly access the internal storage with the \ref ticks
  getter.
  
  \see setTicks, addTicks, addTick
*/
void QCPAxisTickerText::clear()
{
  mTicks.clear();
}

/*!
  Adds a single tick to the axis at the given axis coordinate \a position, with the provided tick \a
  label.
  
  \see addTicks, setTicks, clear
*/
void QCPAxisTickerText::addTick(double position, const QString &label)
{
  mTicks.insert(position, label);
}

/*! \overload
  
  Adds the provided \a ticks to the ones already existing. The map key of \a ticks corresponds to
  the axis coordinate, and the map value is the string that will appear as tick label.
  
  An alternative to manipulate ticks is to directly access the internal storage with the \ref ticks
  getter.
  
  \see addTick, setTicks, clear
*/
void QCPAxisTickerText::addTicks(const QMap<double, QString> &ticks)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  mTicks.unite(ticks);
#else
  mTicks.insert(ticks);
#endif
}

/*! \overload
  
  Adds the provided ticks to the ones already existing. The entries of \a positions correspond to
  the axis coordinates, and the entries of \a labels are the respective strings that will appear as
  tick labels.
  
  An alternative to manipulate ticks is to directly access the internal storage with the \ref ticks
  getter.
  
  \see addTick, setTicks, clear
*/
void QCPAxisTickerText::addTicks(const QVector<double> &positions, const QVector<QString> &labels)
{
  if (positions.size() != labels.size())
    qDebug() << Q_FUNC_INFO << "passed unequal length vectors for positions and labels:" << positions.size() << labels.size();
  int n = qMin(positions.size(), labels.size());
  for (int i=0; i<n; ++i)
    mTicks.insert(positions.at(i), labels.at(i));
}

/*!
  Since the tick coordinates are provided externally, this method implementation does nothing.
  
  \seebaseclassmethod
*/
double QCPAxisTickerText::getTickStep(const QCPRange &range)
{
  // text axis ticker has manual tick positions, so doesn't need this method
  Q_UNUSED(range)
  return 1.0;
}

/*!
  Returns the sub tick count that was configured with \ref setSubTickCount.
  
  \seebaseclassmethod
*/
int QCPAxisTickerText::getSubTickCount(double tickStep)
{
  Q_UNUSED(tickStep)
  return mSubTickCount;
}

/*!
  Returns the tick label which corresponds to the key \a tick in the internal tick storage. Since
  the labels are provided externally, \a locale, \a formatChar, and \a precision are ignored.
  
  \seebaseclassmethod
*/
QString QCPAxisTickerText::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision)
{
  Q_UNUSED(locale)
  Q_UNUSED(formatChar)
  Q_UNUSED(precision)
  return mTicks.value(tick);
}

/*!
  Returns the externally provided tick coordinates which are in the specified \a range. If
  available, one tick above and below the range is provided in addition, to allow possible sub tick
  calculation. The parameter \a tickStep is ignored.
  
  \seebaseclassmethod
*/
QVector<double> QCPAxisTickerText::createTickVector(double tickStep, const QCPRange &range)
{
  Q_UNUSED(tickStep)
  QVector<double> result;
  if (mTicks.isEmpty())
    return result;
  
  QMap<double, QString>::const_iterator start = mTicks.lowerBound(range.lower);
  QMap<double, QString>::const_iterator end = mTicks.upperBound(range.upper);
  // this method should try to give one tick outside of range so proper subticks can be generated:
  if (start != mTicks.constBegin()) --start;
  if (end != mTicks.constEnd()) ++end;
  for (QMap<double, QString>::const_iterator it = start; it != end; ++it)
    result.append(it.key());
  
  return result;
}
