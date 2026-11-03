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

#ifndef QCP_AXISTICKERTEXT_H
#define QCP_AXISTICKERTEXT_H

#include "axisticker.h"

class QCP_LIB_DECL QCPAxisTickerText : public QCPAxisTicker
{
public:
  QCPAxisTickerText();
  
  // getters:
  QMap<double, QString> &ticks() { return mTicks; }
  int subTickCount() const { return mSubTickCount; }
  
  // setters:
  void setTicks(const QMap<double, QString> &ticks);
  void setTicks(const QVector<double> &positions, const QVector<QString> &labels);
  void setSubTickCount(int subTicks);
  
  // non-virtual methods:
  void clear();
  void addTick(double position, const QString &label);
  void addTicks(const QMap<double, QString> &ticks);
  void addTicks(const QVector<double> &positions, const QVector<QString> &labels);
  
protected:
  // property members:
  QMap<double, QString> mTicks;
  int mSubTickCount;
  
  // reimplemented virtual methods:
  virtual double getTickStep(const QCPRange &range) Q_DECL_OVERRIDE;
  virtual int getSubTickCount(double tickStep) Q_DECL_OVERRIDE;
  virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) Q_DECL_OVERRIDE;
  virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) Q_DECL_OVERRIDE;
};

#endif // QCP_AXISTICKERTEXT_H
