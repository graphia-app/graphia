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

#ifndef QCP_RANGE_H
#define QCP_RANGE_H

#include "../global.h"

class QCP_LIB_DECL QCPRange
{
public:
  double lower, upper;
  
  QCPRange();
  QCPRange(double lower, double upper);
  
  bool operator==(const QCPRange& other) const { return lower == other.lower && upper == other.upper; }
  bool operator!=(const QCPRange& other) const { return !(*this == other); }
  
  QCPRange &operator+=(const double& value) { lower+=value; upper+=value; return *this; }
  QCPRange &operator-=(const double& value) { lower-=value; upper-=value; return *this; }
  QCPRange &operator*=(const double& value) { lower*=value; upper*=value; return *this; }
  QCPRange &operator/=(const double& value) { lower/=value; upper/=value; return *this; }
  friend inline const QCPRange operator+(const QCPRange&, double);
  friend inline const QCPRange operator+(double, const QCPRange&);
  friend inline const QCPRange operator-(const QCPRange& range, double value);
  friend inline const QCPRange operator*(const QCPRange& range, double value);
  friend inline const QCPRange operator*(double value, const QCPRange& range);
  friend inline const QCPRange operator/(const QCPRange& range, double value);
  
  double size() const { return upper-lower; }
  double center() const { return (upper+lower)*0.5; }
  void normalize() { if (lower > upper) qSwap(lower, upper); }
  void expand(const QCPRange &otherRange);
  void expand(double includeCoord);
  QCPRange expanded(const QCPRange &otherRange) const;
  QCPRange expanded(double includeCoord) const;
  QCPRange bounded(double lowerBound, double upperBound) const;
  QCPRange sanitizedForLogScale() const;
  QCPRange sanitizedForLinScale() const;
  bool contains(double value) const { return value >= lower && value <= upper; }
  
  static bool validRange(double lower, double upper);
  static bool validRange(const QCPRange &range);
  static const double minRange;
  static const double maxRange;
  
};
Q_DECLARE_TYPEINFO(QCPRange, Q_MOVABLE_TYPE);

/*! \relates QCPRange

  Prints \a range in a human readable format to the qDebug output.
*/
inline QDebug operator<< (QDebug d, const QCPRange &range)
{
    d.nospace() << "QCPRange(" << range.lower << ", " << range.upper << ")";
    return d.space();
}

/*!
  Adds \a value to both boundaries of the range.
*/
inline const QCPRange operator+(const QCPRange& range, double value)
{
  QCPRange result(range);
  result += value;
  return result;
}

/*!
  Adds \a value to both boundaries of the range.
*/
inline const QCPRange operator+(double value, const QCPRange& range)
{
  QCPRange result(range);
  result += value;
  return result;
}

/*!
  Subtracts \a value from both boundaries of the range.
*/
inline const QCPRange operator-(const QCPRange& range, double value)
{
  QCPRange result(range);
  result -= value;
  return result;
}

/*!
  Multiplies both boundaries of the range by \a value.
*/
inline const QCPRange operator*(const QCPRange& range, double value)
{
  QCPRange result(range);
  result *= value;
  return result;
}

/*!
  Multiplies both boundaries of the range by \a value.
*/
inline const QCPRange operator*(double value, const QCPRange& range)
{
  QCPRange result(range);
  result *= value;
  return result;
}

/*!
  Divides both boundaries of the range by \a value.
*/
inline const QCPRange operator/(const QCPRange& range, double value)
{
  QCPRange result(range);
  result /= value;
  return result;
}

#endif // QCP_RANGE_H
