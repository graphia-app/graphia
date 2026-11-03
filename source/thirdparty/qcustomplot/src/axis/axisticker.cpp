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

#include "axisticker.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAxisTicker
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class QCPAxisTicker
  \brief The base class tick generator used by QCPAxis to create tick positions and tick labels
  
  Each QCPAxis has an internal QCPAxisTicker (or a subclass) in order to generate tick positions
  and tick labels for the current axis range. The ticker of an axis can be set via \ref
  QCPAxis::setTicker. Since that method takes a <tt>QSharedPointer<QCPAxisTicker></tt>, multiple
  axes can share the same ticker instance.
  
  This base class generates normal tick coordinates and numeric labels for linear axes. It picks a
  reasonable tick step (the separation between ticks) which results in readable tick labels. The
  number of ticks that should be approximately generated can be set via \ref setTickCount.
  Depending on the current tick step strategy (\ref setTickStepStrategy), the algorithm either
  sacrifices readability to better match the specified tick count (\ref
  QCPAxisTicker::tssMeetTickCount) or relaxes the tick count in favor of better tick steps (\ref
  QCPAxisTicker::tssReadability), which is the default.
  
  The following more specialized axis ticker subclasses are available, see details in the
  respective class documentation:
  
  <center>
  <table>
  <tr><td style="text-align:right; padding: 0 1em">QCPAxisTickerFixed</td><td>\image html axisticker-fixed.png</td></tr>
  <tr><td style="text-align:right; padding: 0 1em">QCPAxisTickerLog</td><td>\image html axisticker-log.png</td></tr>
  <tr><td style="text-align:right; padding: 0 1em">QCPAxisTickerPi</td><td>\image html axisticker-pi.png</td></tr>
  <tr><td style="text-align:right; padding: 0 1em">QCPAxisTickerText</td><td>\image html axisticker-text.png</td></tr>
  <tr><td style="text-align:right; padding: 0 1em">QCPAxisTickerDateTime</td><td>\image html axisticker-datetime.png</td></tr>
  <tr><td style="text-align:right; padding: 0 1em">QCPAxisTickerTime</td><td>\image html axisticker-time.png
    \image html axisticker-time2.png</td></tr>
  </table>
  </center>
  
  \section axisticker-subclassing Creating own axis tickers
  
  Creating own axis tickers can be achieved very easily by sublassing QCPAxisTicker and
  reimplementing some or all of the available virtual methods.

  In the simplest case you might wish to just generate different tick steps than the other tickers,
  so you only reimplement the method \ref getTickStep. If you additionally want control over the
  string that will be shown as tick label, reimplement \ref getTickLabel.
  
  If you wish to have complete control, you can generate the tick vectors and tick label vectors
  yourself by reimplementing \ref createTickVector and \ref createLabelVector. The default
  implementations use the previously mentioned virtual methods \ref getTickStep and \ref
  getTickLabel, but your reimplementations don't necessarily need to do so. For example in the case
  of unequal tick steps, the method \ref getTickStep loses its usefulness and can be ignored.
  
  The sub tick count between major ticks can be controlled with \ref getSubTickCount. Full sub tick
  placement control is obtained by reimplementing \ref createSubTickVector.
  
  See the documentation of all these virtual methods in QCPAxisTicker for detailed information
  about the parameters and expected return values.
*/

/*!
  Constructs the ticker and sets reasonable default values. Axis tickers are commonly created
  managed by a QSharedPointer, which then can be passed to QCPAxis::setTicker.
*/
QCPAxisTicker::QCPAxisTicker() :
  mTickStepStrategy(tssReadability),
  mTickCount(5),
  mTickOrigin(0)
{
}

QCPAxisTicker::~QCPAxisTicker()
{
  
}

/*!
  Sets which strategy the axis ticker follows when choosing the size of the tick step. For the
  available strategies, see \ref TickStepStrategy.
*/
void QCPAxisTicker::setTickStepStrategy(QCPAxisTicker::TickStepStrategy strategy)
{
  mTickStepStrategy = strategy;
}

/*!
  Sets how many ticks this ticker shall aim to generate across the axis range. Note that \a count
  is not guaranteed to be matched exactly, as generating readable tick intervals may conflict with
  the requested number of ticks.

  Whether the readability has priority over meeting the requested \a count can be specified with
  \ref setTickStepStrategy.
*/
void QCPAxisTicker::setTickCount(int count)
{
  if (count > 0)
    mTickCount = count;
  else
    qDebug() << Q_FUNC_INFO << "tick count must be greater than zero:" << count;
}

/*!
  Sets the mathematical coordinate (or "offset") of the zeroth tick. This tick coordinate is just a
  concept and doesn't need to be inside the currently visible axis range.
  
  By default \a origin is zero, which for example yields ticks {-5, 0, 5, 10, 15,...} when the tick
  step is five. If \a origin is now set to 1 instead, the correspondingly generated ticks would be
  {-4, 1, 6, 11, 16,...}.
*/
void QCPAxisTicker::setTickOrigin(double origin)
{
  mTickOrigin = origin;
}

/*!
  This is the method called by QCPAxis in order to actually generate tick coordinates (\a ticks),
  tick label strings (\a tickLabels) and sub tick coordinates (\a subTicks).
  
  The ticks are generated for the specified \a range. The generated labels typically follow the
  specified \a locale, \a formatChar and number \a precision, however this might be different (or
  even irrelevant) for certain QCPAxisTicker subclasses.
  
  The output parameter \a ticks is filled with the generated tick positions in axis coordinates.
  The output parameters \a subTicks and \a tickLabels are optional (set them to \c nullptr if not
  needed) and are respectively filled with sub tick coordinates, and tick label strings belonging
  to \a ticks by index.
*/
void QCPAxisTicker::generate(const QCPRange &range, const QLocale &locale, QChar formatChar, int precision, QVector<double> &ticks, QVector<double> *subTicks, QVector<QString> *tickLabels)
{
  // generate (major) ticks:
  double tickStep = getTickStep(range);
  ticks = createTickVector(tickStep, range);
  trimTicks(range, ticks, true); // trim ticks to visible range plus one outer tick on each side (incase a subclass createTickVector creates more)
  
  // generate sub ticks between major ticks:
  if (subTicks)
  {
    if (!ticks.isEmpty())
    {
      *subTicks = createSubTickVector(getSubTickCount(tickStep), ticks);
      trimTicks(range, *subTicks, false);
    } else
      *subTicks = QVector<double>();
  }
  
  // finally trim also outliers (no further clipping happens in axis drawing):
  trimTicks(range, ticks, false);
  // generate labels for visible ticks if requested:
  if (tickLabels)
    *tickLabels = createLabelVector(ticks, locale, formatChar, precision);
}

/*! \internal
  
  Takes the entire currently visible axis range and returns a sensible tick step in
  order to provide readable tick labels as well as a reasonable number of tick counts (see \ref
  setTickCount, \ref setTickStepStrategy).
  
  If a QCPAxisTicker subclass only wants a different tick step behaviour than the default
  implementation, it should reimplement this method. See \ref cleanMantissa for a possible helper
  function.
*/
double QCPAxisTicker::getTickStep(const QCPRange &range)
{
  double exactStep = range.size()/double(mTickCount+1e-10); // mTickCount ticks on average, the small addition is to prevent jitter on exact integers
  return cleanMantissa(exactStep);
}

/*! \internal
  
  Takes the \a tickStep, i.e. the distance between two consecutive ticks, and returns
  an appropriate number of sub ticks for that specific tick step.
  
  Note that a returned sub tick count of e.g. 4 will split each tick interval into 5 sections.
*/
int QCPAxisTicker::getSubTickCount(double tickStep)
{
  int result = 1; // default to 1, if no proper value can be found
  
  // separate integer and fractional part of mantissa:
  double epsilon = 0.01;
  double intPartf;
  int intPart;
  double fracPart = modf(getMantissa(tickStep), &intPartf);
  intPart = int(intPartf);
  
  // handle cases with (almost) integer mantissa:
  if (fracPart < epsilon || 1.0-fracPart < epsilon)
  {
    if (1.0-fracPart < epsilon)
      ++intPart;
    switch (intPart)
    {
      case 1: result = 4; break; // 1.0 -> 0.2 substep
      case 2: result = 3; break; // 2.0 -> 0.5 substep
      case 3: result = 2; break; // 3.0 -> 1.0 substep
      case 4: result = 3; break; // 4.0 -> 1.0 substep
      case 5: result = 4; break; // 5.0 -> 1.0 substep
      case 6: result = 2; break; // 6.0 -> 2.0 substep
      case 7: result = 6; break; // 7.0 -> 1.0 substep
      case 8: result = 3; break; // 8.0 -> 2.0 substep
      case 9: result = 2; break; // 9.0 -> 3.0 substep
    }
  } else
  {
    // handle cases with significantly fractional mantissa:
    if (qAbs(fracPart-0.5) < epsilon) // *.5 mantissa
    {
      switch (intPart)
      {
        case 1: result = 2; break; // 1.5 -> 0.5 substep
        case 2: result = 4; break; // 2.5 -> 0.5 substep
        case 3: result = 4; break; // 3.5 -> 0.7 substep
        case 4: result = 2; break; // 4.5 -> 1.5 substep
        case 5: result = 4; break; // 5.5 -> 1.1 substep (won't occur with default getTickStep from here on)
        case 6: result = 4; break; // 6.5 -> 1.3 substep
        case 7: result = 2; break; // 7.5 -> 2.5 substep
        case 8: result = 4; break; // 8.5 -> 1.7 substep
        case 9: result = 4; break; // 9.5 -> 1.9 substep
      }
    }
    // if mantissa fraction isn't 0.0 or 0.5, don't bother finding good sub tick marks, leave default
  }
  
  return result;
}

/*! \internal
  
  This method returns the tick label string as it should be printed under the \a tick coordinate.
  If a textual number is returned, it should respect the provided \a locale, \a formatChar and \a
  precision.
  
  If the returned value contains exponentials of the form "2e5" and beautifully typeset powers is
  enabled in the QCPAxis number format (\ref QCPAxis::setNumberFormat), the exponential part will
  be formatted accordingly using multiplication symbol and superscript during rendering of the
  label automatically.
*/
QString QCPAxisTicker::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision)
{
  return locale.toString(tick, formatChar.toLatin1(), precision);
}

/*! \internal
  
  Returns a vector containing all coordinates of sub ticks that should be drawn. It generates \a
  subTickCount sub ticks between each tick pair given in \a ticks.
  
  If a QCPAxisTicker subclass needs maximal control over the generated sub ticks, it should
  reimplement this method. Depending on the purpose of the subclass it doesn't necessarily need to
  base its result on \a subTickCount or \a ticks.
*/
QVector<double> QCPAxisTicker::createSubTickVector(int subTickCount, const QVector<double> &ticks)
{
  QVector<double> result;
  if (subTickCount <= 0 || ticks.size() < 2)
    return result;
  
  result.reserve((ticks.size()-1)*subTickCount);
  for (int i=1; i<ticks.size(); ++i)
  {
    double subTickStep = (ticks.at(i)-ticks.at(i-1))/double(subTickCount+1);
    for (int k=1; k<=subTickCount; ++k)
      result.append(ticks.at(i-1) + k*subTickStep);
  }
  return result;
}

/*! \internal
  
  Returns a vector containing all coordinates of ticks that should be drawn. The default
  implementation generates ticks with a spacing of \a tickStep (mathematically starting at the tick
  step origin, see \ref setTickOrigin) distributed over the passed \a range.
  
  In order for the axis ticker to generate proper sub ticks, it is necessary that the first and
  last tick coordinates returned by this method are just below/above the provided \a range.
  Otherwise the outer intervals won't contain any sub ticks.
  
  If a QCPAxisTicker subclass needs maximal control over the generated ticks, it should reimplement
  this method. Depending on the purpose of the subclass it doesn't necessarily need to base its
  result on \a tickStep, e.g. when the ticks are spaced unequally like in the case of
  QCPAxisTickerLog.
*/
QVector<double> QCPAxisTicker::createTickVector(double tickStep, const QCPRange &range)
{
  QVector<double> result;
  // Generate tick positions according to tickStep:
  qint64 firstStep = qint64(floor((range.lower-mTickOrigin)/tickStep)); // do not use qFloor here, or we'll lose 64 bit precision
  qint64 lastStep = qint64(ceil((range.upper-mTickOrigin)/tickStep)); // do not use qCeil here, or we'll lose 64 bit precision
  int tickcount = int(lastStep-firstStep+1);
  if (tickcount < 0) tickcount = 0;
  result.resize(tickcount);
  for (int i=0; i<tickcount; ++i)
    result[i] = mTickOrigin + (firstStep+i)*tickStep;
  return result;
}

/*! \internal
  
  Returns a vector containing all tick label strings corresponding to the tick coordinates provided
  in \a ticks. The default implementation calls \ref getTickLabel to generate the respective
  strings.
  
  It is possible but uncommon for QCPAxisTicker subclasses to reimplement this method, as
  reimplementing \ref getTickLabel often achieves the intended result easier.
*/
QVector<QString> QCPAxisTicker::createLabelVector(const QVector<double> &ticks, const QLocale &locale, QChar formatChar, int precision)
{
  QVector<QString> result;
  result.reserve(ticks.size());
  foreach (double tickCoord, ticks)
    result.append(getTickLabel(tickCoord, locale, formatChar, precision));
  return result;
}

/*! \internal
  
  Removes tick coordinates from \a ticks which lie outside the specified \a range. If \a
  keepOneOutlier is true, it preserves one tick just outside the range on both sides, if present.
  
  The passed \a ticks must be sorted in ascending order.
*/
void QCPAxisTicker::trimTicks(const QCPRange &range, QVector<double> &ticks, bool keepOneOutlier) const
{
  bool lowFound = false;
  bool highFound = false;
  int lowIndex = 0;
  int highIndex = -1;
  
  for (int i=0; i < ticks.size(); ++i)
  {
    if (ticks.at(i) >= range.lower)
    {
      lowFound = true;
      lowIndex = i;
      break;
    }
  }
  for (int i=ticks.size()-1; i >= 0; --i)
  {
    if (ticks.at(i) <= range.upper)
    {
      highFound = true;
      highIndex = i;
      break;
    }
  }
  
  if (highFound && lowFound)
  {
    int trimFront = qMax(0, lowIndex-(keepOneOutlier ? 1 : 0));
    int trimBack = qMax(0, ticks.size()-(keepOneOutlier ? 2 : 1)-highIndex);
    if (trimFront > 0 || trimBack > 0)
      ticks = ticks.mid(trimFront, ticks.size()-trimFront-trimBack);
  } else // all ticks are either all below or all above the range
    ticks.clear();
}

/*! \internal
  
  Returns the coordinate contained in \a candidates which is closest to the provided \a target.
  
  This method assumes \a candidates is not empty and sorted in ascending order.
*/
double QCPAxisTicker::pickClosest(double target, const QVector<double> &candidates) const
{
  if (candidates.size() == 1)
    return candidates.first();
  QVector<double>::const_iterator it = std::lower_bound(candidates.constBegin(), candidates.constEnd(), target);
  if (it == candidates.constEnd())
    return *(it-1);
  else if (it == candidates.constBegin())
    return *it;
  else
    return target-*(it-1) < *it-target ? *(it-1) : *it;
}

/*! \internal
  
  Returns the decimal mantissa of \a input. Optionally, if \a magnitude is not set to zero, it also
  returns the magnitude of \a input as a power of 10.
  
  For example, an input of 142.6 will return a mantissa of 1.426 and a magnitude of 100.
*/
double QCPAxisTicker::getMantissa(double input, double *magnitude) const
{
  const double mag = std::pow(10.0, std::floor(std::log10(input)));
  if (magnitude) *magnitude = mag;
  return input/mag;
}

/*! \internal
  
  Returns a number that is close to \a input but has a clean, easier human readable mantissa. How
  strongly the mantissa is altered, and thus how strong the result deviates from the original \a
  input, depends on the current tick step strategy (see \ref setTickStepStrategy).
*/
double QCPAxisTicker::cleanMantissa(double input) const
{
  double magnitude;
  const double mantissa = getMantissa(input, &magnitude);
  switch (mTickStepStrategy)
  {
    case tssReadability:
    {
      return pickClosest(mantissa, QVector<double>() << 1.0 << 2.0 << 2.5 << 5.0 << 10.0)*magnitude;
    }
    case tssMeetTickCount:
    {
      // this gives effectively a mantissa of 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 6.0, 8.0, 10.0
      if (mantissa <= 5.0)
        return int(mantissa*2)/2.0*magnitude; // round digit after decimal point to 0.5
      else
        return int(mantissa/2.0)*2.0*magnitude; // round to first digit in multiples of 2
    }
  }
  return input;
}
