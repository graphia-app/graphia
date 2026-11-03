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

#include "selection.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPDataRange
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPDataRange
  \brief Describes a data range given by begin and end index
  
  QCPDataRange holds two integers describing the begin (\ref setBegin) and end (\ref setEnd) index
  of a contiguous set of data points. The \a end index corresponds to the data point just after the
  last data point of the data range, like in standard iterators.

  Data Ranges are not bound to a certain plottable, thus they can be freely exchanged, created and
  modified. If a non-contiguous data set shall be described, the class \ref QCPDataSelection is
  used, which holds and manages multiple instances of \ref QCPDataRange. In most situations, \ref
  QCPDataSelection is thus used.
  
  Both \ref QCPDataRange and \ref QCPDataSelection offer convenience methods to work with them,
  e.g. \ref bounded, \ref expanded, \ref intersects, \ref intersection, \ref adjusted, \ref
  contains. Further, addition and subtraction operators (defined in \ref QCPDataSelection) can be
  used to join/subtract data ranges and data selections (or mixtures), to retrieve a corresponding
  \ref QCPDataSelection.
  
  %QCustomPlot's \ref dataselection "data selection mechanism" is based on \ref QCPDataSelection and
  QCPDataRange.
  
  \note Do not confuse \ref QCPDataRange with \ref QCPRange. A \ref QCPRange describes an interval
  in floating point plot coordinates, e.g. the current axis range.
*/

/* start documentation of inline functions */

/*! \fn int QCPDataRange::size() const
  
  Returns the number of data points described by this data range. This is equal to the end index
  minus the begin index.
  
  \see length
*/

/*! \fn int QCPDataRange::length() const
  
  Returns the number of data points described by this data range. Equivalent to \ref size.
*/

/*! \fn void QCPDataRange::setBegin(int begin)
  
  Sets the begin of this data range. The \a begin index points to the first data point that is part
  of the data range.
  
  No checks or corrections are made to ensure the resulting range is valid (\ref isValid).
  
  \see setEnd
*/

/*! \fn void QCPDataRange::setEnd(int end)
  
  Sets the end of this data range. The \a end index points to the data point just after the last
  data point that is part of the data range.
  
  No checks or corrections are made to ensure the resulting range is valid (\ref isValid).
  
  \see setBegin
*/

/*! \fn bool QCPDataRange::isValid() const
  
  Returns whether this range is valid. A valid range has a begin index greater or equal to 0, and
  an end index greater or equal to the begin index.
  
  \note Invalid ranges should be avoided and are never the result of any of QCustomPlot's methods
  (unless they are themselves fed with invalid ranges). Do not pass invalid ranges to QCustomPlot's
  methods. The invalid range is not inherently prevented in QCPDataRange, to allow temporary
  invalid begin/end values while manipulating the range. An invalid range is not necessarily empty
  (\ref isEmpty), since its \ref length can be negative and thus non-zero.
*/

/*! \fn bool QCPDataRange::isEmpty() const
  
  Returns whether this range is empty, i.e. whether its begin index equals its end index.
  
  \see size, length
*/

/*! \fn QCPDataRange QCPDataRange::adjusted(int changeBegin, int changeEnd) const
  
  Returns a data range where \a changeBegin and \a changeEnd were added to the begin and end
  indices, respectively.
*/

/* end documentation of inline functions */

/*!
  Creates an empty QCPDataRange, with begin and end set to 0.
*/
QCPDataRange::QCPDataRange() :
  mBegin(0),
  mEnd(0)
{
}

/*!
  Creates a QCPDataRange, initialized with the specified \a begin and \a end.
  
  No checks or corrections are made to ensure the resulting range is valid (\ref isValid).
*/
QCPDataRange::QCPDataRange(int begin, int end) :
  mBegin(begin),
  mEnd(end)
{
}

/*!
  Returns a data range that matches this data range, except that parts exceeding \a other are
  excluded.
  
  This method is very similar to \ref intersection, with one distinction: If this range and the \a
  other range share no intersection, the returned data range will be empty with begin and end set
  to the respective boundary side of \a other, at which this range is residing. (\ref intersection
  would just return a range with begin and end set to 0.)
*/
QCPDataRange QCPDataRange::bounded(const QCPDataRange &other) const
{
  QCPDataRange result(intersection(other));
  if (result.isEmpty()) // no intersection, preserve respective bounding side of otherRange as both begin and end of return value
  {
    if (mEnd <= other.mBegin)
      result = QCPDataRange(other.mBegin, other.mBegin);
    else
      result = QCPDataRange(other.mEnd, other.mEnd);
  }
  return result;
}

/*!
  Returns a data range that contains both this data range as well as \a other.
*/
QCPDataRange QCPDataRange::expanded(const QCPDataRange &other) const
{
  return {qMin(mBegin, other.mBegin), qMax(mEnd, other.mEnd)};
}

/*!
  Returns the data range which is contained in both this data range and \a other.
  
  This method is very similar to \ref bounded, with one distinction: If this range and the \a other
  range share no intersection, the returned data range will be empty with begin and end set to 0.
  (\ref bounded would return a range with begin and end set to one of the boundaries of \a other,
  depending on which side this range is on.)
  
  \see QCPDataSelection::intersection
*/
QCPDataRange QCPDataRange::intersection(const QCPDataRange &other) const
{
  QCPDataRange result(qMax(mBegin, other.mBegin), qMin(mEnd, other.mEnd));
  if (result.isValid())
    return result;
  else
    return {};
}

/*!
  Returns whether this data range and \a other share common data points.
  
  \see intersection, contains
*/
bool QCPDataRange::intersects(const QCPDataRange &other) const
{
   return !( (mBegin > other.mBegin && mBegin >= other.mEnd) ||
             (mEnd <= other.mBegin && mEnd < other.mEnd) );
}

/*!
  Returns whether all data points of \a other are also contained inside this data range.
  
  \see intersects
*/
bool QCPDataRange::contains(const QCPDataRange &other) const
{
  return mBegin <= other.mBegin && mEnd >= other.mEnd;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPDataSelection
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPDataSelection
  \brief Describes a data set by holding multiple QCPDataRange instances
  
  QCPDataSelection manages multiple instances of QCPDataRange in order to represent any (possibly
  disjoint) set of data selection.
  
  The data selection can be modified with addition and subtraction operators which take
  QCPDataSelection and QCPDataRange instances, as well as methods such as \ref addDataRange and
  \ref clear. Read access is provided by \ref dataRange, \ref dataRanges, \ref dataRangeCount, etc.
  
  The method \ref simplify is used to join directly adjacent or even overlapping QCPDataRange
  instances. QCPDataSelection automatically simplifies when using the addition/subtraction
  operators. The only case when \ref simplify is left to the user, is when calling \ref
  addDataRange, with the parameter \a simplify explicitly set to false. This is useful if many data
  ranges will be added to the selection successively and the overhead for simplifying after each
  iteration shall be avoided. In this case, you should make sure to call \ref simplify after
  completing the operation.
  
  Use \ref enforceType to bring the data selection into a state complying with the constraints for
  selections defined in \ref QCP::SelectionType.
  
  %QCustomPlot's \ref dataselection "data selection mechanism" is based on QCPDataSelection and
  QCPDataRange.
  
  \section qcpdataselection-iterating Iterating over a data selection
  
  As an example, the following code snippet calculates the average value of a graph's data
  \ref QCPAbstractPlottable::selection "selection":
  
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpdataselection-iterating-1
  
*/

/* start documentation of inline functions */

/*! \fn int QCPDataSelection::dataRangeCount() const
  
  Returns the number of ranges that make up the data selection. The ranges can be accessed by \ref
  dataRange via their index.
  
  \see dataRange, dataPointCount
*/

/*! \fn QList<QCPDataRange> QCPDataSelection::dataRanges() const
  
  Returns all data ranges that make up the data selection. If the data selection is simplified (the
  usual state of the selection, see \ref simplify), the ranges are sorted by ascending data point
  index.
  
  \see dataRange
*/

/*! \fn bool QCPDataSelection::isEmpty() const
  
  Returns true if there are no data ranges, and thus no data points, in this QCPDataSelection
  instance.
  
  \see dataRangeCount
*/

/* end documentation of inline functions */

/*!
  Creates an empty QCPDataSelection.
*/
QCPDataSelection::QCPDataSelection()
{
}

/*!
  Creates a QCPDataSelection containing the provided \a range.
*/
QCPDataSelection::QCPDataSelection(const QCPDataRange &range)
{
  mDataRanges.append(range);
}

/*!
  Returns true if this selection is identical (contains the same data ranges with the same begin
  and end indices) to \a other.

  Note that both data selections must be in simplified state (the usual state of the selection, see
  \ref simplify) for this operator to return correct results.
*/
bool QCPDataSelection::operator==(const QCPDataSelection &other) const
{
  if (mDataRanges.size() != other.mDataRanges.size())
    return false;
  for (int i=0; i<mDataRanges.size(); ++i)
  {
    if (mDataRanges.at(i) != other.mDataRanges.at(i))
      return false;
  }
  return true;
}

/*!
  Adds the data selection of \a other to this data selection, and then simplifies this data
  selection (see \ref simplify).
*/
QCPDataSelection &QCPDataSelection::operator+=(const QCPDataSelection &other)
{
  mDataRanges << other.mDataRanges;
  simplify();
  return *this;
}

/*!
  Adds the data range \a other to this data selection, and then simplifies this data selection (see
  \ref simplify).
*/
QCPDataSelection &QCPDataSelection::operator+=(const QCPDataRange &other)
{
  addDataRange(other);
  return *this;
}

/*!
  Removes all data point indices that are described by \a other from this data selection.
*/
QCPDataSelection &QCPDataSelection::operator-=(const QCPDataSelection &other)
{
  for (int i=0; i<other.dataRangeCount(); ++i)
    *this -= other.dataRange(i);
  
  return *this;
}

/*!
  Removes all data point indices that are described by \a other from this data selection.
*/
QCPDataSelection &QCPDataSelection::operator-=(const QCPDataRange &other)
{
  if (other.isEmpty() || isEmpty())
    return *this;
  
  simplify();
  int i=0;
  while (i < mDataRanges.size())
  {
    const int thisBegin = mDataRanges.at(i).begin();
    const int thisEnd = mDataRanges.at(i).end();
    if (thisBegin >= other.end())
      break; // since data ranges are sorted after the simplify() call, no ranges which contain other will come after this
    
    if (thisEnd > other.begin()) // ranges which don't fulfill this are entirely before other and can be ignored
    {
      if (thisBegin >= other.begin()) // range leading segment is encompassed
      {
        if (thisEnd <= other.end()) // range fully encompassed, remove completely
        {
          mDataRanges.removeAt(i);
          continue;
        } else // only leading segment is encompassed, trim accordingly
          mDataRanges[i].setBegin(other.end());
      } else // leading segment is not encompassed
      {
        if (thisEnd <= other.end()) // only trailing segment is encompassed, trim accordingly
        {
          mDataRanges[i].setEnd(other.begin());
        } else // other lies inside this range, so split range
        {
          mDataRanges[i].setEnd(other.begin());
          mDataRanges.insert(i+1, QCPDataRange(other.end(), thisEnd));
          break; // since data ranges are sorted (and don't overlap) after simplify() call, we're done here
        }
      }
    }
    ++i;
  }
  
  return *this;
}

/*!
  Returns the total number of data points contained in all data ranges that make up this data
  selection.
*/
int QCPDataSelection::dataPointCount() const
{
  int result = 0;
  foreach (QCPDataRange dataRange, mDataRanges)
    result += dataRange.length();
  return result;
}

/*!
  Returns the data range with the specified \a index.
  
  If the data selection is simplified (the usual state of the selection, see \ref simplify), the
  ranges are sorted by ascending data point index.
  
  \see dataRangeCount
*/
QCPDataRange QCPDataSelection::dataRange(int index) const
{
  if (index >= 0 && index < mDataRanges.size())
  {
    return mDataRanges.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "index out of range:" << index;
    return {};
  }
}

/*!
  Returns a \ref QCPDataRange which spans the entire data selection, including possible
  intermediate segments which are not part of the original data selection.
*/
QCPDataRange QCPDataSelection::span() const
{
  if (isEmpty())
    return {};
  else
    return {mDataRanges.first().begin(), mDataRanges.last().end()};
}

/*!
  Adds the given \a dataRange to this data selection. This is equivalent to the += operator but
  allows disabling immediate simplification by setting \a simplify to false. This can improve
  performance if adding a very large amount of data ranges successively. In this case, make sure to
  call \ref simplify manually, after the operation.
*/
void QCPDataSelection::addDataRange(const QCPDataRange &dataRange, bool simplify)
{
  mDataRanges.append(dataRange);
  if (simplify)
    this->simplify();
}

/*!
  Removes all data ranges. The data selection then contains no data points.
  
  \ref isEmpty
*/
void QCPDataSelection::clear()
{
  mDataRanges.clear();
}

/*!
  Sorts all data ranges by range begin index in ascending order, and then joins directly adjacent
  or overlapping ranges. This can reduce the number of individual data ranges in the selection, and
  prevents possible double-counting when iterating over the data points held by the data ranges.

  This method is automatically called when using the addition/subtraction operators. The only case
  when \ref simplify is left to the user, is when calling \ref addDataRange, with the parameter \a
  simplify explicitly set to false.
*/
void QCPDataSelection::simplify()
{
  // remove any empty ranges:
  for (int i=mDataRanges.size()-1; i>=0; --i)
  {
    if (mDataRanges.at(i).isEmpty())
      mDataRanges.removeAt(i);
  }
  if (mDataRanges.isEmpty())
    return;
  
  // sort ranges by starting value, ascending:
  std::sort(mDataRanges.begin(), mDataRanges.end(), lessThanDataRangeBegin);
  
  // join overlapping/contiguous ranges:
  int i = 1;
  while (i < mDataRanges.size())
  {
    if (mDataRanges.at(i-1).end() >= mDataRanges.at(i).begin()) // range i overlaps/joins with i-1, so expand range i-1 appropriately and remove range i from list
    {
      mDataRanges[i-1].setEnd(qMax(mDataRanges.at(i-1).end(), mDataRanges.at(i).end()));
      mDataRanges.removeAt(i);
    } else
      ++i;
  }
}

/*!
  Makes sure this data selection conforms to the specified \a type selection type. Before the type
  is enforced, \ref simplify is called.
  
  Depending on \a type, enforcing means adding new data points that were previously not part of the
  selection, or removing data points from the selection. If the current selection already conforms
  to \a type, the data selection is not changed.
  
  \see QCP::SelectionType
*/
void QCPDataSelection::enforceType(QCP::SelectionType type)
{
  simplify();
  switch (type)
  {
    case QCP::stNone:
    {
      mDataRanges.clear();
      break;
    }
    case QCP::stWhole:
    {
      // whole selection isn't defined by data range, so don't change anything (is handled in plottable methods)
      break;
    }
    case QCP::stSingleData:
    {
      // reduce all data ranges to the single first data point:
      if (!mDataRanges.isEmpty())
      {
        if (mDataRanges.size() > 1)
          mDataRanges = QList<QCPDataRange>() << mDataRanges.first();
        if (mDataRanges.first().length() > 1)
          mDataRanges.first().setEnd(mDataRanges.first().begin()+1);
      }
      break;
    }
    case QCP::stDataRange:
    {
      if (!isEmpty())
        mDataRanges = QList<QCPDataRange>() << span();
      break;
    }
    case QCP::stMultipleDataRanges:
    {
      // this is the selection type that allows all concievable combinations of ranges, so do nothing
      break;
    }
  }
}

/*!
  Returns true if the data selection \a other is contained entirely in this data selection, i.e.
  all data point indices that are in \a other are also in this data selection.
  
  \see QCPDataRange::contains
*/
bool QCPDataSelection::contains(const QCPDataSelection &other) const
{
  if (other.isEmpty()) return false;
  
  int otherIndex = 0;
  int thisIndex = 0;
  while (thisIndex < mDataRanges.size() && otherIndex < other.mDataRanges.size())
  {
    if (mDataRanges.at(thisIndex).contains(other.mDataRanges.at(otherIndex)))
      ++otherIndex;
    else
      ++thisIndex;
  }
  return thisIndex < mDataRanges.size(); // if thisIndex ran all the way to the end to find a containing range for the current otherIndex, other is not contained in this
}

/*!
  Returns a data selection containing the points which are both in this data selection and in the
  data range \a other.

  A common use case is to limit an unknown data selection to the valid range of a data container,
  using \ref QCPDataContainer::dataRange as \a other. One can then safely iterate over the returned
  data selection without exceeding the data container's bounds.
*/
QCPDataSelection QCPDataSelection::intersection(const QCPDataRange &other) const
{
  QCPDataSelection result;
  foreach (QCPDataRange dataRange, mDataRanges)
    result.addDataRange(dataRange.intersection(other), false);
  result.simplify();
  return result;
}

/*!
  Returns a data selection containing the points which are both in this data selection and in the
  data selection \a other.
*/
QCPDataSelection QCPDataSelection::intersection(const QCPDataSelection &other) const
{
  QCPDataSelection result;
  for (int i=0; i<other.dataRangeCount(); ++i)
    result += intersection(other.dataRange(i));
  result.simplify();
  return result;
}

/*!
  Returns a data selection which is the exact inverse of this data selection, with \a outerRange
  defining the base range on which to invert. If \a outerRange is smaller than the \ref span of
  this data selection, it is expanded accordingly.

  For example, this method can be used to retrieve all unselected segments by setting \a outerRange
  to the full data range of the plottable, and calling this method on a data selection holding the
  selected segments.
*/
QCPDataSelection QCPDataSelection::inverse(const QCPDataRange &outerRange) const
{
  if (isEmpty())
    return QCPDataSelection(outerRange);
  QCPDataRange fullRange = outerRange.expanded(span());
  
  QCPDataSelection result;
  // first unselected segment:
  if (mDataRanges.first().begin() != fullRange.begin())
    result.addDataRange(QCPDataRange(fullRange.begin(), mDataRanges.first().begin()), false);
  // intermediate unselected segments:
  for (int i=1; i<mDataRanges.size(); ++i)
    result.addDataRange(QCPDataRange(mDataRanges.at(i-1).end(), mDataRanges.at(i).begin()), false);
  // last unselected segment:
  if (mDataRanges.last().end() != fullRange.end())
    result.addDataRange(QCPDataRange(mDataRanges.last().end(), fullRange.end()), false);
  result.simplify();
  return result;
}




