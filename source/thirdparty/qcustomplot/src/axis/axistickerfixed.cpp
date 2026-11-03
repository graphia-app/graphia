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

#include "axistickerfixed.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAxisTickerFixed
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class QCPAxisTickerFixed
  \brief Specialized axis ticker with a fixed tick step
  
  \image html axisticker-fixed.png
  
  This QCPAxisTicker subclass generates ticks with a fixed tick step set with \ref setTickStep. It
  is also possible to allow integer multiples and integer powers of the specified tick step with
  \ref setScaleStrategy.
  
  A typical application of this ticker is to make an axis only display integers, by setting the
  tick step of the ticker to 1.0 and the scale strategy to \ref ssMultiples.
  
  Another case is when a certain number has a special meaning and axis ticks should only appear at
  multiples of that value. In this case you might also want to consider \ref QCPAxisTickerPi
  because despite the name it is not limited to only pi symbols/values.
  
  The ticker can be created and assigned to an axis like this:
  \snippet documentation/doc-image-generator/mainwindow.cpp axistickerfixed-creation
*/

/*!
  Constructs the ticker and sets reasonable default values. Axis tickers are commonly created
  managed by a QSharedPointer, which then can be passed to QCPAxis::setTicker.
*/
QCPAxisTickerFixed::QCPAxisTickerFixed() :
  mTickStep(1.0),
  mScaleStrategy(ssNone)
{
}

/*!
  Sets the fixed tick interval to \a step.
  
  The axis ticker will only use this tick step when generating axis ticks. This might cause a very
  high tick density and overlapping labels if the axis range is zoomed out. Using \ref
  setScaleStrategy it is possible to relax the fixed step and also allow multiples or powers of \a
  step. This will enable the ticker to reduce the number of ticks to a reasonable amount (see \ref
  setTickCount).
*/
void QCPAxisTickerFixed::setTickStep(double step)
{
  if (step > 0)
    mTickStep = step;
  else
    qDebug() << Q_FUNC_INFO << "tick step must be greater than zero:" << step;
}

/*!
  Sets whether the specified tick step (\ref setTickStep) is absolutely fixed or whether
  modifications may be applied to it before calculating the finally used tick step, such as
  permitting multiples or powers. See \ref ScaleStrategy for details.
  
  The default strategy is \ref ssNone, which means the tick step is absolutely fixed.
*/
void QCPAxisTickerFixed::setScaleStrategy(QCPAxisTickerFixed::ScaleStrategy strategy)
{
  mScaleStrategy = strategy;
}

/*! \internal
  
  Determines the actually used tick step from the specified tick step and scale strategy (\ref
  setTickStep, \ref setScaleStrategy).
  
  This method either returns the specified tick step exactly, or, if the scale strategy is not \ref
  ssNone, a modification of it to allow varying the number of ticks in the current axis range.
  
  \seebaseclassmethod
*/
double QCPAxisTickerFixed::getTickStep(const QCPRange &range)
{
  switch (mScaleStrategy)
  {
    case ssNone:
    {
      return mTickStep;
    }
    case ssMultiples:
    {
      double exactStep = range.size()/double(mTickCount+1e-10); // mTickCount ticks on average, the small addition is to prevent jitter on exact integers
      if (exactStep < mTickStep)
        return mTickStep;
      else
        return qint64(cleanMantissa(exactStep/mTickStep)+0.5)*mTickStep;
    }
    case ssPowers:
    {
      double exactStep = range.size()/double(mTickCount+1e-10); // mTickCount ticks on average, the small addition is to prevent jitter on exact integers
      return qPow(mTickStep, int(qLn(exactStep)/qLn(mTickStep)+0.5));
    }
  }
  return mTickStep;
}
