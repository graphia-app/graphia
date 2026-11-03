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

#include "axistickerdatetime.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAxisTickerDateTime
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class QCPAxisTickerDateTime
  \brief Specialized axis ticker for calendar dates and times as axis ticks
  
  \image html axisticker-datetime.png
  
  This QCPAxisTicker subclass generates ticks that correspond to real calendar dates and times. The
  plot axis coordinate is interpreted as Unix Time, so seconds since Epoch (January 1, 1970, 00:00
  UTC). This is also used for example by QDateTime in the <tt>toTime_t()/setTime_t()</tt> methods
  with a precision of one second. Since Qt 4.7, millisecond accuracy can be obtained from QDateTime
  by using <tt>QDateTime::fromMSecsSinceEpoch()/1000.0</tt>. The static methods \ref dateTimeToKey
  and \ref keyToDateTime conveniently perform this conversion achieving a precision of one
  millisecond on all Qt versions.
  
  The format of the date/time display in the tick labels is controlled with \ref setDateTimeFormat.
  If a different time spec or time zone shall be used for the tick label appearance, see \ref
  setDateTimeSpec or \ref setTimeZone, respectively.
  
  This ticker produces unequal tick spacing in order to provide intuitive date and time-of-day
  ticks. For example, if the axis range spans a few years such that there is one tick per year,
  ticks will be positioned on 1. January of every year. This is intuitive but, due to leap years,
  will result in slightly unequal tick intervals (visually unnoticeable). The same can be seen in
  the image above: even though the number of days varies month by month, this ticker generates
  ticks on the same day of each month.
  
  If you would like to change the date/time that is used as a (mathematical) starting date for the
  ticks, use the \ref setTickOrigin(const QDateTime &origin) method overload, which takes a
  QDateTime. If you pass 15. July, 9:45 to this method, the yearly ticks will end up on 15. July at
  9:45 of every year.
  
  The ticker can be created and assigned to an axis like this:
  \snippet documentation/doc-image-generator/mainwindow.cpp axistickerdatetime-creation
  
  \note If you rather wish to display relative times in terms of days, hours, minutes, seconds and
  milliseconds, and are not interested in the intricacies of real calendar dates with months and
  (leap) years, have a look at QCPAxisTickerTime instead.
*/

# if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
/*! \internal

  Qt 6.9 deprecated the Qt::TimeSpec overloads of QDateTime and QDate in favour of QTimeZone; this
  maps the former onto the latter. Qt::TimeZone is handled by the callers, which have the actual
  QTimeZone to hand, so it is treated as local time here.
*/
static QTimeZone qcpTimeZoneFromSpec(Qt::TimeSpec spec, int offsetSeconds = 0)
{
  switch (spec)
  {
    case Qt::UTC: return QTimeZone(QTimeZone::UTC);
    case Qt::OffsetFromUTC: return QTimeZone::fromSecondsAheadOfUtc(offsetSeconds);
    default: return QTimeZone(QTimeZone::LocalTime);
  }
}
# endif

/*!
  Constructs the ticker and sets reasonable default values. Axis tickers are commonly created
  managed by a QSharedPointer, which then can be passed to QCPAxis::setTicker.
*/
QCPAxisTickerDateTime::QCPAxisTickerDateTime() :
  mDateTimeFormat(QLatin1String("hh:mm:ss\ndd.MM.yy")),
  mDateTimeSpec(Qt::LocalTime),
  mDateStrategy(dsNone)
{
  setTickCount(4);
}

/*!
  Sets the format in which dates and times are displayed as tick labels. For details about the \a
  format string, see the documentation of QDateTime::toString().
  
  Typical expressions are
  <table>
    <tr><td>\c d</td><td>The day as a number without a leading zero (1 to 31)</td></tr>
    <tr><td>\c dd</td><td>The day as a number with a leading zero (01 to 31)</td></tr>
    <tr><td>\c ddd</td><td>The abbreviated localized day name (e.g. 'Mon' to 'Sun'). Uses the system locale to localize the name, i.e. QLocale::system().</td></tr>
    <tr><td>\c dddd</td><td>The long localized day name (e.g. 'Monday' to 'Sunday'). Uses the system locale to localize the name, i.e. QLocale::system().</td></tr>
    <tr><td>\c M</td><td>The month as a number without a leading zero (1 to 12)</td></tr>
    <tr><td>\c MM</td><td>The month as a number with a leading zero (01 to 12)</td></tr>
    <tr><td>\c MMM</td><td>The abbreviated localized month name (e.g. 'Jan' to 'Dec'). Uses the system locale to localize the name, i.e. QLocale::system().</td></tr>
    <tr><td>\c MMMM</td><td>The long localized month name (e.g. 'January' to 'December'). Uses the system locale to localize the name, i.e. QLocale::system().</td></tr>
    <tr><td>\c yy</td><td>The year as a two digit number (00 to 99)</td></tr>
    <tr><td>\c yyyy</td><td>The year as a four digit number. If the year is negative, a minus sign is prepended, making five characters.</td></tr>
    <tr><td>\c h</td><td>The hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)</td></tr>
    <tr><td>\c hh</td><td>The hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)</td></tr>
    <tr><td>\c H</td><td>The hour without a leading zero (0 to 23, even with AM/PM display)</td></tr>
    <tr><td>\c HH</td><td>The hour with a leading zero (00 to 23, even with AM/PM display)</td></tr>
    <tr><td>\c m</td><td>The minute without a leading zero (0 to 59)</td></tr>
    <tr><td>\c mm</td><td>The minute with a leading zero (00 to 59)</td></tr>
    <tr><td>\c s</td><td>The whole second, without any leading zero (0 to 59)</td></tr>
    <tr><td>\c ss</td><td>The whole second, with a leading zero where applicable (00 to 59)</td></tr>
    <tr><td>\c z</td><td>The fractional part of the second, to go after a decimal point, without trailing zeroes (0 to 999). Thus "s.z" reports the seconds to full available (millisecond) precision without trailing zeroes.</td></tr>
    <tr><td>\c zzz</td><td>The fractional part of the second, to millisecond precision, including trailing zeroes where applicable (000 to 999).</td></tr>
    <tr><td>\c AP or \c A</td><td>Use AM/PM display. A/AP will be replaced by an upper-case version of either QLocale::amText() or QLocale::pmText().</td></tr>
    <tr><td>\c ap or \c a</td><td>Use am/pm display. a/ap will be replaced by a lower-case version of either QLocale::amText() or QLocale::pmText().</td></tr>
    <tr><td>\c t</td><td>The timezone (for example "CEST")</td></tr>
  </table>
  
  Newlines can be inserted with \c "\n", literal strings (even when containing above expressions)
  by encapsulating them using single-quotes. A literal single quote can be generated by using two
  consecutive single quotes in the format.
  
  \see setDateTimeSpec, setTimeZone
*/
void QCPAxisTickerDateTime::setDateTimeFormat(const QString &format)
{
  mDateTimeFormat = format;
}

/*!
  Sets the time spec that is used for creating the tick labels from corresponding dates/times.

  The default value of QDateTime objects (and also QCPAxisTickerDateTime) is
  <tt>Qt::LocalTime</tt>. However, if the displayed tick labels shall be given in UTC, set \a spec
  to <tt>Qt::UTC</tt>.
  
  Tick labels corresponding to other time zones can be achieved with \ref setTimeZone (which sets
  \a spec to \c Qt::TimeZone internally). Note that if \a spec is afterwards set to not be \c
  Qt::TimeZone again, the \ref setTimeZone setting will be ignored accordingly.
  
  \see setDateTimeFormat, setTimeZone
*/
void QCPAxisTickerDateTime::setDateTimeSpec(Qt::TimeSpec spec)
{
  mDateTimeSpec = spec;
}

# if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
/*!
  Sets the time zone that is used for creating the tick labels from corresponding dates/times. The
  time spec (\ref setDateTimeSpec) is set to \c Qt::TimeZone.
  
  \see setDateTimeFormat, setTimeZone
*/
void QCPAxisTickerDateTime::setTimeZone(const QTimeZone &zone)
{
  mTimeZone = zone;
  mDateTimeSpec = Qt::TimeZone;
}
#endif

/*!
  Sets the tick origin (see \ref QCPAxisTicker::setTickOrigin) in seconds since Epoch (1. Jan 1970,
  00:00 UTC). For the date time ticker it might be more intuitive to use the overload which
  directly takes a QDateTime, see \ref setTickOrigin(const QDateTime &origin).
  
  This is useful to define the month/day/time recurring at greater tick interval steps. For
  example, If you pass 15. July, 9:45 to this method and the tick interval happens to be one tick
  per year, the ticks will end up on 15. July at 9:45 of every year.
*/
void QCPAxisTickerDateTime::setTickOrigin(double origin)
{
  QCPAxisTicker::setTickOrigin(origin);
}

/*!
  Sets the tick origin (see \ref QCPAxisTicker::setTickOrigin) as a QDateTime \a origin.
  
  This is useful to define the month/day/time recurring at greater tick interval steps. For
  example, If you pass 15. July, 9:45 to this method and the tick interval happens to be one tick
  per year, the ticks will end up on 15. July at 9:45 of every year.
*/
void QCPAxisTickerDateTime::setTickOrigin(const QDateTime &origin)
{
  setTickOrigin(dateTimeToKey(origin));
}

/*! \internal
  
  Returns a sensible tick step with intervals appropriate for a date-time-display, such as weekly,
  monthly, bi-monthly, etc.
  
  Note that this tick step isn't used exactly when generating the tick vector in \ref
  createTickVector, but only as a guiding value requiring some correction for each individual tick
  interval. Otherwise this would lead to unintuitive date displays, e.g. jumping between first day
  in the month to the last day in the previous month from tick to tick, due to the non-uniform
  length of months. The same problem arises with leap years.
  
  \seebaseclassmethod
*/
double QCPAxisTickerDateTime::getTickStep(const QCPRange &range)
{
  double result = range.size()/double(mTickCount+1e-10); // mTickCount ticks on average, the small addition is to prevent jitter on exact integers
  
  mDateStrategy = dsNone; // leaving it at dsNone means tick coordinates will not be tuned in any special way in createTickVector
  if (result < 1) // ideal tick step is below 1 second -> use normal clean mantissa algorithm in units of seconds
  {
    result = cleanMantissa(result);
  } else if (result < 86400*30.4375*12) // below a year
  {
    result = pickClosest(result, QVector<double>()
                             << 1 << 2.5 << 5 << 10 << 15 << 30 << 60 << 2.5*60 << 5*60 << 10*60 << 15*60 << 30*60 << 60*60 // second, minute, hour range
                             << 3600*2 << 3600*3 << 3600*6 << 3600*12 << 3600*24 // hour to day range
                             << 86400*2 << 86400*5 << 86400*7 << 86400*14 << 86400*30.4375 << 86400*30.4375*2 << 86400*30.4375*3 << 86400*30.4375*6 << 86400*30.4375*12); // day, week, month range (avg. days per month includes leap years)
    if (result > 86400*30.4375-1) // month tick intervals or larger
      mDateStrategy = dsUniformDayInMonth;
    else if (result > 3600*24-1) // day tick intervals or larger
      mDateStrategy = dsUniformTimeInDay;
  } else // more than a year, go back to normal clean mantissa algorithm but in units of years
  {
    const double secondsPerYear = 86400*30.4375*12; // average including leap years
    result = cleanMantissa(result/secondsPerYear)*secondsPerYear;
    mDateStrategy = dsUniformDayInMonth;
  }
  return result;
}

/*! \internal
  
  Returns a sensible sub tick count with intervals appropriate for a date-time-display, such as weekly,
  monthly, bi-monthly, etc.
  
  \seebaseclassmethod
*/
int QCPAxisTickerDateTime::getSubTickCount(double tickStep)
{
  int result = QCPAxisTicker::getSubTickCount(tickStep);
  switch (qRound(tickStep)) // hand chosen subticks for specific minute/hour/day/week/month range (as specified in getTickStep)
  {
    case 5*60: result = 4; break;
    case 10*60: result = 1; break;
    case 15*60: result = 2; break;
    case 30*60: result = 1; break;
    case 60*60: result = 3; break;
    case 3600*2: result = 3; break;
    case 3600*3: result = 2; break;
    case 3600*6: result = 1; break;
    case 3600*12: result = 3; break;
    case 3600*24: result = 3; break;
    case 86400*2: result = 1; break;
    case 86400*5: result = 4; break;
    case 86400*7: result = 6; break;
    case 86400*14: result = 1; break;
    case int(86400*30.4375+0.5): result = 3; break;
    case int(86400*30.4375*2+0.5): result = 1; break;
    case int(86400*30.4375*3+0.5): result = 2; break;
    case int(86400*30.4375*6+0.5): result = 5; break;
    case int(86400*30.4375*12+0.5): result = 3; break;
  }
  return result;
}

/*! \internal
  
  Generates a date/time tick label for tick coordinate \a tick, based on the currently set format
  (\ref setDateTimeFormat), time spec (\ref setDateTimeSpec), and possibly time zone (\ref
  setTimeZone).
  
  \seebaseclassmethod
*/
QString QCPAxisTickerDateTime::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision)
{
  Q_UNUSED(precision)
  Q_UNUSED(formatChar)
# if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  if (mDateTimeSpec == Qt::TimeZone)
    return locale.toString(keyToDateTime(tick).toTimeZone(mTimeZone), mDateTimeFormat);
  else
    return locale.toString(keyToDateTime(tick).toTimeZone(qcpTimeZoneFromSpec(mDateTimeSpec)), mDateTimeFormat);
# elif QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
  if (mDateTimeSpec == Qt::TimeZone)
    return locale.toString(keyToDateTime(tick).toTimeZone(mTimeZone), mDateTimeFormat);
  else
    return locale.toString(keyToDateTime(tick).toTimeSpec(mDateTimeSpec), mDateTimeFormat);
# else
  return locale.toString(keyToDateTime(tick).toTimeSpec(mDateTimeSpec), mDateTimeFormat);
# endif
}

/*! \internal
  
  Uses the passed \a tickStep as a guiding value and applies corrections in order to obtain
  non-uniform tick intervals but intuitive tick labels, e.g. falling on the same day of each month.
  
  \seebaseclassmethod
*/
QVector<double> QCPAxisTickerDateTime::createTickVector(double tickStep, const QCPRange &range)
{
  QVector<double> result = QCPAxisTicker::createTickVector(tickStep, range);
  if (!result.isEmpty())
  {
    if (mDateStrategy == dsUniformTimeInDay)
    {
      QDateTime uniformDateTime = keyToDateTime(mTickOrigin); // the time of this datetime will be set for all other ticks, if possible
      QDateTime tickDateTime;
      for (int i=0; i<result.size(); ++i)
      {
        tickDateTime = keyToDateTime(result.at(i));
        tickDateTime.setTime(uniformDateTime.time());
        result[i] = dateTimeToKey(tickDateTime);
      }
    } else if (mDateStrategy == dsUniformDayInMonth)
    {
      QDateTime uniformDateTime = keyToDateTime(mTickOrigin); // this day (in month) and time will be set for all other ticks, if possible
      QDateTime tickDateTime;
      for (int i=0; i<result.size(); ++i)
      {
        tickDateTime = keyToDateTime(result.at(i));
        tickDateTime.setTime(uniformDateTime.time());
        int thisUniformDay = uniformDateTime.date().day() <= tickDateTime.date().daysInMonth() ? uniformDateTime.date().day() : tickDateTime.date().daysInMonth(); // don't exceed month (e.g. try to set day 31 in February)
        if (thisUniformDay-tickDateTime.date().day() < -15) // with leap years involved, date month may jump backwards or forwards, and needs to be corrected before setting day
          tickDateTime = tickDateTime.addMonths(1);
        else if (thisUniformDay-tickDateTime.date().day() > 15) // with leap years involved, date month may jump backwards or forwards, and needs to be corrected before setting day
          tickDateTime = tickDateTime.addMonths(-1);
        tickDateTime.setDate(QDate(tickDateTime.date().year(), tickDateTime.date().month(), thisUniformDay));
        result[i] = dateTimeToKey(tickDateTime);
      }
    }
  }
  return result;
}

/*!
  A convenience method which turns \a key (in seconds since Epoch 1. Jan 1970, 00:00 UTC) into a
  QDateTime object. This can be used to turn axis coordinates to actual QDateTimes.
  
  The accuracy achieved by this method is one millisecond, irrespective of the used Qt version (it
  works around the lack of a QDateTime::fromMSecsSinceEpoch in Qt 4.6)
  
  \see dateTimeToKey
*/
QDateTime QCPAxisTickerDateTime::keyToDateTime(double key)
{
# if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  return QDateTime::fromTime_t(key).addMSecs((key-(qint64)key)*1000);
# else
  return QDateTime::fromMSecsSinceEpoch(qint64(key*1000.0));
# endif
}

/*! \overload
  
  A convenience method which turns a QDateTime object into a double value that corresponds to
  seconds since Epoch (1. Jan 1970, 00:00 UTC). This is the format used as axis coordinates by
  QCPAxisTickerDateTime.
  
  The accuracy achieved by this method is one millisecond, irrespective of the used Qt version (it
  works around the lack of a QDateTime::toMSecsSinceEpoch in Qt 4.6)
  
  \see keyToDateTime
*/
double QCPAxisTickerDateTime::dateTimeToKey(const QDateTime &dateTime)
{
# if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  return dateTime.toTime_t()+dateTime.time().msec()/1000.0;
# else
  return dateTime.toMSecsSinceEpoch()/1000.0;
# endif
}

/*! \overload
  
  A convenience method which turns a QDate object into a double value that corresponds to seconds
  since Epoch (1. Jan 1970, 00:00 UTC). This is the format used
  as axis coordinates by QCPAxisTickerDateTime.
  
  The returned value will be the start of the passed day of \a date, interpreted in the given \a
  timeSpec.
  
  \see keyToDateTime
*/
double QCPAxisTickerDateTime::dateTimeToKey(const QDate &date, Qt::TimeSpec timeSpec)
{
# if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  return QDateTime(date, QTime(0, 0), timeSpec).toTime_t();
# elif QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  return QDateTime(date, QTime(0, 0), timeSpec).toMSecsSinceEpoch()/1000.0;
# elif QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
  return date.startOfDay(timeSpec).toMSecsSinceEpoch()/1000.0;
# else
  return date.startOfDay(qcpTimeZoneFromSpec(timeSpec)).toMSecsSinceEpoch()/1000.0;
# endif
}
