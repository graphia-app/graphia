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

#include "range.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPRange
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class QCPRange
  \brief Represents the range an axis is encompassing.
  
  contains a \a lower and \a upper double value and provides convenience input, output and
  modification functions.
  
  \see QCPAxis::setRange
*/

/* start of documentation of inline functions */

/*! \fn double QCPRange::size() const

  Returns the size of the range, i.e. \a upper-\a lower
*/

/*! \fn double QCPRange::center() const

  Returns the center of the range, i.e. (\a upper+\a lower)*0.5
*/

/*! \fn void QCPRange::normalize()

  Makes sure \a lower is numerically smaller than \a upper. If this is not the case, the values are
  swapped.
*/

/*! \fn bool QCPRange::contains(double value) const

  Returns true when \a value lies within or exactly on the borders of the range.
*/

/*! \fn QCPRange &QCPRange::operator+=(const double& value)

  Adds \a value to both boundaries of the range.
*/

/*! \fn QCPRange &QCPRange::operator-=(const double& value)

  Subtracts \a value from both boundaries of the range.
*/

/*! \fn QCPRange &QCPRange::operator*=(const double& value)

  Multiplies both boundaries of the range by \a value.
*/

/*! \fn QCPRange &QCPRange::operator/=(const double& value)

  Divides both boundaries of the range by \a value.
*/

/* end of documentation of inline functions */

/*!
  Minimum range size (\a upper - \a lower) the range changing functions will accept. Smaller
  intervals would cause errors due to the 11-bit exponent of double precision numbers,
  corresponding to a minimum magnitude of roughly 1e-308.

  \warning Do not use this constant to indicate "arbitrarily small" values in plotting logic (as
  values that will appear in the plot)! It is intended only as a bound to compare against, e.g. to
  prevent axis ranges from obtaining underflowing ranges.

  \see validRange, maxRange
*/
const double QCPRange::minRange = 1e-280;

/*!
  Maximum values (negative and positive) the range will accept in range-changing functions.
  Larger absolute values would cause errors due to the 11-bit exponent of double precision numbers,
  corresponding to a maximum magnitude of roughly 1e308.

  \warning Do not use this constant to indicate "arbitrarily large" values in plotting logic (as
  values that will appear in the plot)! It is intended only as a bound to compare against, e.g. to
  prevent axis ranges from obtaining overflowing ranges.

  \see validRange, minRange
*/
const double QCPRange::maxRange = 1e250;

/*!
  Constructs a range with \a lower and \a upper set to zero.
*/
QCPRange::QCPRange() :
  lower(0),
  upper(0)
{
}

/*! \overload

  Constructs a range with the specified \a lower and \a upper values.

  The resulting range will be normalized (see \ref normalize), so if \a lower is not numerically
  smaller than \a upper, they will be swapped.
*/
QCPRange::QCPRange(double lower, double upper) :
  lower(lower),
  upper(upper)
{
  normalize();
}

/*! \overload

  Expands this range such that \a otherRange is contained in the new range. It is assumed that both
  this range and \a otherRange are normalized (see \ref normalize).

  If this range contains NaN as lower or upper bound, it will be replaced by the respective bound
  of \a otherRange.

  If \a otherRange is already inside the current range, this function does nothing.

  \see expanded
*/
void QCPRange::expand(const QCPRange &otherRange)
{
  if (lower > otherRange.lower || qIsNaN(lower))
    lower = otherRange.lower;
  if (upper < otherRange.upper || qIsNaN(upper))
    upper = otherRange.upper;
}

/*! \overload

  Expands this range such that \a includeCoord is contained in the new range. It is assumed that
  this range is normalized (see \ref normalize).

  If this range contains NaN as lower or upper bound, the respective bound will be set to \a
  includeCoord.

  If \a includeCoord is already inside the current range, this function does nothing.

  \see expand
*/
void QCPRange::expand(double includeCoord)
{
  if (lower > includeCoord || qIsNaN(lower))
    lower = includeCoord;
  if (upper < includeCoord || qIsNaN(upper))
    upper = includeCoord;
}


/*! \overload

  Returns an expanded range that contains this and \a otherRange. It is assumed that both this
  range and \a otherRange are normalized (see \ref normalize).

  If this range contains NaN as lower or upper bound, the returned range's bound will be taken from
  \a otherRange.

  \see expand
*/
QCPRange QCPRange::expanded(const QCPRange &otherRange) const
{
  QCPRange result = *this;
  result.expand(otherRange);
  return result;
}

/*! \overload

  Returns an expanded range that includes the specified \a includeCoord. It is assumed that this
  range is normalized (see \ref normalize).

  If this range contains NaN as lower or upper bound, the returned range's bound will be set to \a
  includeCoord.

  \see expand
*/
QCPRange QCPRange::expanded(double includeCoord) const
{
  QCPRange result = *this;
  result.expand(includeCoord);
  return result;
}

/*!
  Returns this range, possibly modified to not exceed the bounds provided as \a lowerBound and \a
  upperBound. If possible, the size of the current range is preserved in the process.
  
  If the range shall only be bounded at the lower side, you can set \a upperBound to \ref
  QCPRange::maxRange. If it shall only be bounded at the upper side, set \a lowerBound to -\ref
  QCPRange::maxRange.
*/
QCPRange QCPRange::bounded(double lowerBound, double upperBound) const
{
  if (lowerBound > upperBound)
    qSwap(lowerBound, upperBound);
  
  QCPRange result(lower, upper);
  if (result.lower < lowerBound)
  {
    result.lower = lowerBound;
    result.upper = lowerBound + size();
    if (result.upper > upperBound || qFuzzyCompare(size(), upperBound-lowerBound))
      result.upper = upperBound;
  } else if (result.upper > upperBound)
  {
    result.upper = upperBound;
    result.lower = upperBound - size();
    if (result.lower < lowerBound || qFuzzyCompare(size(), upperBound-lowerBound))
      result.lower = lowerBound;
  }
  
  return result;
}

/*!
  Returns a sanitized version of the range. Sanitized means for logarithmic scales, that
  the range won't span the positive and negative sign domain, i.e. contain zero. Further
  \a lower will always be numerically smaller (or equal) to \a upper.
  
  If the original range does span positive and negative sign domains or contains zero,
  the returned range will try to approximate the original range as good as possible.
  If the positive interval of the original range is wider than the negative interval, the
  returned range will only contain the positive interval, with lower bound set to \a rangeFac or
  \a rangeFac *\a upper, whichever is closer to zero. Same procedure is used if the negative interval
  is wider than the positive interval, this time by changing the \a upper bound.
*/
QCPRange QCPRange::sanitizedForLogScale() const
{
  double rangeFac = 1e-3;
  QCPRange sanitizedRange(lower, upper);
  sanitizedRange.normalize();
  // can't have range spanning negative and positive values in log plot, so change range to fix it
  //if (qFuzzyCompare(sanitizedRange.lower+1, 1) && !qFuzzyCompare(sanitizedRange.upper+1, 1))
  if (sanitizedRange.lower == 0.0 && sanitizedRange.upper != 0.0)
  {
    // case lower is 0
    if (rangeFac < sanitizedRange.upper*rangeFac)
      sanitizedRange.lower = rangeFac;
    else
      sanitizedRange.lower = sanitizedRange.upper*rangeFac;
  } //else if (!qFuzzyCompare(lower+1, 1) && qFuzzyCompare(upper+1, 1))
  else if (sanitizedRange.lower != 0.0 && sanitizedRange.upper == 0.0)
  {
    // case upper is 0
    if (-rangeFac > sanitizedRange.lower*rangeFac)
      sanitizedRange.upper = -rangeFac;
    else
      sanitizedRange.upper = sanitizedRange.lower*rangeFac;
  } else if (sanitizedRange.lower < 0 && sanitizedRange.upper > 0)
  {
    // find out whether negative or positive interval is wider to decide which sign domain will be chosen
    if (-sanitizedRange.lower > sanitizedRange.upper)
    {
      // negative is wider, do same as in case upper is 0
      if (-rangeFac > sanitizedRange.lower*rangeFac)
        sanitizedRange.upper = -rangeFac;
      else
        sanitizedRange.upper = sanitizedRange.lower*rangeFac;
    } else
    {
      // positive is wider, do same as in case lower is 0
      if (rangeFac < sanitizedRange.upper*rangeFac)
        sanitizedRange.lower = rangeFac;
      else
        sanitizedRange.lower = sanitizedRange.upper*rangeFac;
    }
  }
  // due to normalization, case lower>0 && upper<0 should never occur, because that implies upper<lower
  return sanitizedRange;
}

/*!
  Returns a sanitized version of the range. Sanitized means for linear scales, that
  \a lower will always be numerically smaller (or equal) to \a upper.
*/
QCPRange QCPRange::sanitizedForLinScale() const
{
  QCPRange sanitizedRange(lower, upper);
  sanitizedRange.normalize();
  return sanitizedRange;
}

/*!
  Checks, whether the specified range is within valid bounds, which are defined
  as QCPRange::maxRange and QCPRange::minRange.
  A valid range means:
  \li range bounds within -maxRange and maxRange
  \li range size above minRange
  \li range size below maxRange
*/
bool QCPRange::validRange(double lower, double upper)
{
  return (lower > -maxRange &&
          upper < maxRange &&
          qAbs(lower-upper) > minRange &&
          qAbs(lower-upper) < maxRange &&
          !(lower > 0 && qIsInf(upper/lower)) &&
          !(upper < 0 && qIsInf(lower/upper)));
}

/*!
  \overload
  Checks, whether the specified range is within valid bounds, which are defined
  as QCPRange::maxRange and QCPRange::minRange.
  A valid range means:
  \li range bounds within -maxRange and maxRange
  \li range size above minRange
  \li range size below maxRange
*/
bool QCPRange::validRange(const QCPRange &range)
{
  return (range.lower > -maxRange &&
          range.upper < maxRange &&
          qAbs(range.lower-range.upper) > minRange &&
          qAbs(range.lower-range.upper) < maxRange &&
          !(range.lower > 0 && qIsInf(range.upper/range.lower)) &&
          !(range.upper < 0 && qIsInf(range.lower/range.upper)));
}
