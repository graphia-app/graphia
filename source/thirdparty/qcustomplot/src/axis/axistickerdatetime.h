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

#ifndef QCP_AXISTICKERDATETIME_H
#define QCP_AXISTICKERDATETIME_H

#include "axisticker.h"

class QCP_LIB_DECL QCPAxisTickerDateTime : public QCPAxisTicker
{
public:
  QCPAxisTickerDateTime();
  
  // getters:
  QString dateTimeFormat() const { return mDateTimeFormat; }
  Qt::TimeSpec dateTimeSpec() const { return mDateTimeSpec; }
# if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
  QTimeZone timeZone() const { return mTimeZone; }
#endif
  
  // setters:
  void setDateTimeFormat(const QString &format);
  void setDateTimeSpec(Qt::TimeSpec spec);
# if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
  void setTimeZone(const QTimeZone &zone);
# endif
  void setTickOrigin(double origin); // hides base class method but calls baseclass implementation ("using" throws off IDEs and doxygen)
  void setTickOrigin(const QDateTime &origin);
  
  // static methods:
  static QDateTime keyToDateTime(double key);
  static double dateTimeToKey(const QDateTime &dateTime);
  static double dateTimeToKey(const QDate &date, Qt::TimeSpec timeSpec=Qt::LocalTime);
  
protected:
  // property members:
  QString mDateTimeFormat;
  Qt::TimeSpec mDateTimeSpec;
# if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
  QTimeZone mTimeZone;
# endif
  // non-property members:
  enum DateStrategy {dsNone, dsUniformTimeInDay, dsUniformDayInMonth} mDateStrategy;
  
  // reimplemented virtual methods:
  virtual double getTickStep(const QCPRange &range) Q_DECL_OVERRIDE;
  virtual int getSubTickCount(double tickStep) Q_DECL_OVERRIDE;
  virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) Q_DECL_OVERRIDE;
  virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) Q_DECL_OVERRIDE;
};

#endif // QCP_AXISTICKERDATETIME_H
