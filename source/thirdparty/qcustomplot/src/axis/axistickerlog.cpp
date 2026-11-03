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

#include "axistickerlog.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAxisTickerLog
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class QCPAxisTickerLog
  \brief Specialized axis ticker suited for logarithmic axes
  
  \image html axisticker-log.png
  
  This QCPAxisTicker subclass generates ticks with unequal tick intervals suited for logarithmic
  axis scales. The ticks are placed at powers of the specified log base (\ref setLogBase).
  
  Especially in the case of a log base equal to 10 (the default), it might be desirable to have
  tick labels in the form of powers of ten without mantissa display. To achieve this, set the
  number precision (\ref QCPAxis::setNumberPrecision) to zero and the number format (\ref
  QCPAxis::setNumberFormat) to scientific (exponential) display with beautifully typeset decimal
  powers, so a format string of <tt>"eb"</tt>. This will result in the following axis tick labels:
  
  \image html axisticker-log-powers.png

  The ticker can be created and assigned to an axis like this:
  \snippet documentation/doc-image-generator/mainwindow.cpp axistickerlog-creation
  
  Note that the nature of logarithmic ticks imply that there exists a smallest possible tick step,
  corresponding to one multiplication by the log base. If the user zooms in further than that, no
  new ticks would appear, leading to very sparse or even no axis ticks on the axis. To prevent this
  situation, this ticker falls back to regular tick generation if the axis range would be covered
  by too few logarithmically placed ticks.
*/

/*!
  Constructs the ticker and sets reasonable default values. Axis tickers are commonly created
  managed by a QSharedPointer, which then can be passed to QCPAxis::setTicker.
*/
QCPAxisTickerLog::QCPAxisTickerLog() :
  mLogBase(10.0),
  mSubTickCount(8), // generates 10 intervals
  mLogBaseLnInv(1.0/qLn(mLogBase))
{
}

/*!
  Sets the logarithm base used for tick coordinate generation. The ticks will be placed at integer
  powers of \a base.
*/
void QCPAxisTickerLog::setLogBase(double base)
{
  if (base > 0)
  {
    mLogBase = base;
    mLogBaseLnInv = 1.0/qLn(mLogBase);
  } else
    qDebug() << Q_FUNC_INFO << "log base has to be greater than zero:" << base;
}

/*!
  Sets the number of sub ticks in a tick interval. Within each interval, the sub ticks are spaced
  linearly to provide a better visual guide, so the sub tick density increases toward the higher
  tick.
  
  Note that \a subTicks is the number of sub ticks (not sub intervals) in one tick interval. So in
  the case of logarithm base 10 an intuitive sub tick spacing would be achieved with eight sub
  ticks (the default). This means e.g. between the ticks 10 and 100 there will be eight ticks,
  namely at 20, 30, 40, 50, 60, 70, 80 and 90.
*/
void QCPAxisTickerLog::setSubTickCount(int subTicks)
{
  if (subTicks >= 0)
    mSubTickCount = subTicks;
  else
    qDebug() << Q_FUNC_INFO << "sub tick count can't be negative:" << subTicks;
}

/*! \internal
  
  Returns the sub tick count specified in \ref setSubTickCount. For QCPAxisTickerLog, there is no
  automatic sub tick count calculation necessary.
  
  \seebaseclassmethod
*/
int QCPAxisTickerLog::getSubTickCount(double tickStep)
{
  Q_UNUSED(tickStep)
  return mSubTickCount;
}

/*! \internal
  
  Creates ticks with a spacing given by the logarithm base and an increasing integer power in the
  provided \a range. The step in which the power increases tick by tick is chosen in order to keep
  the total number of ticks as close as possible to the tick count (\ref setTickCount).

  The parameter \a tickStep is ignored for the normal logarithmic ticker generation. Only when
  zoomed in very far such that not enough logarithmically placed ticks would be visible, this
  function falls back to the regular QCPAxisTicker::createTickVector, which then uses \a tickStep.
  
  \seebaseclassmethod
*/
QVector<double> QCPAxisTickerLog::createTickVector(double tickStep, const QCPRange &range)
{
  QVector<double> result;
  if (range.lower > 0 && range.upper > 0) // positive range
  {
    const double baseTickCount = qLn(range.upper/range.lower)*mLogBaseLnInv;
    if (baseTickCount < 1.6) // if too few log ticks would be visible in axis range, fall back to regular tick vector generation
      return QCPAxisTicker::createTickVector(tickStep, range);
    const double exactPowerStep =  baseTickCount/double(mTickCount+1e-10);
    const double newLogBase = qPow(mLogBase, qMax(int(cleanMantissa(exactPowerStep)), 1));
    double currentTick = qPow(newLogBase, qFloor(qLn(range.lower)/qLn(newLogBase)));
    result.append(currentTick);
    while (currentTick < range.upper && currentTick > 0) // currentMag might be zero for ranges ~1e-300, just cancel in that case
    {
      currentTick *= newLogBase;
      result.append(currentTick);
    }
  } else if (range.lower < 0 && range.upper < 0) // negative range
  {
    const double baseTickCount = qLn(range.lower/range.upper)*mLogBaseLnInv;
    if (baseTickCount < 1.6) // if too few log ticks would be visible in axis range, fall back to regular tick vector generation
      return QCPAxisTicker::createTickVector(tickStep, range);
    const double exactPowerStep =  baseTickCount/double(mTickCount+1e-10);
    const double newLogBase = qPow(mLogBase, qMax(int(cleanMantissa(exactPowerStep)), 1));
    double currentTick = -qPow(newLogBase, qCeil(qLn(-range.lower)/qLn(newLogBase)));
    result.append(currentTick);
    while (currentTick < range.upper && currentTick < 0) // currentMag might be zero for ranges ~1e-300, just cancel in that case
    {
      currentTick /= newLogBase;
      result.append(currentTick);
    }
  } else // invalid range for logarithmic scale, because lower and upper have different sign
  {
    qDebug() << Q_FUNC_INFO << "Invalid range for logarithmic plot: " << range.lower << ".." << range.upper;
  }
  
  return result;
}
