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

#ifndef QCP_AXISTICKERLOG_H
#define QCP_AXISTICKERLOG_H

#include "axisticker.h"

class QCP_LIB_DECL QCPAxisTickerLog : public QCPAxisTicker
{
public:
  QCPAxisTickerLog();
  
  // getters:
  double logBase() const { return mLogBase; }
  int subTickCount() const { return mSubTickCount; }
  
  // setters:
  void setLogBase(double base);
  void setSubTickCount(int subTicks);
  
protected:
  // property members:
  double mLogBase;
  int mSubTickCount;
  
  // non-property members:
  double mLogBaseLnInv;
  
  // reimplemented virtual methods:
  virtual int getSubTickCount(double tickStep) Q_DECL_OVERRIDE;
  virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) Q_DECL_OVERRIDE;
};

#endif // QCP_AXISTICKERLOG_H
