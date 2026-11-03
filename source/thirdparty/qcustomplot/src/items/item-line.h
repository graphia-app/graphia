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

#ifndef QCP_ITEM_LINE_H
#define QCP_ITEM_LINE_H

#include "../global.h"
#include "../item.h"
#include "../lineending.h"

class QCPPainter;
class QCustomPlot;

class QCP_LIB_DECL QCPItemLine : public QCPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(QCPLineEnding head READ head WRITE setHead)
  Q_PROPERTY(QCPLineEnding tail READ tail WRITE setTail)
  /// \endcond
public:
  explicit QCPItemLine(QCustomPlot *parentPlot);
  virtual ~QCPItemLine() Q_DECL_OVERRIDE;
  
  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  QCPLineEnding head() const { return mHead; }
  QCPLineEnding tail() const { return mTail; }
  
  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setHead(const QCPLineEnding &head);
  void setTail(const QCPLineEnding &tail);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  
  QCPItemPosition * const start;
  QCPItemPosition * const end;
  
protected:
  // property members:
  QPen mPen, mSelectedPen;
  QCPLineEnding mHead, mTail;
  
  // reimplemented virtual methods:
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  QLineF getRectClippedLine(const QCPVector2D &start, const QCPVector2D &end, const QRect &rect) const;
  QPen mainPen() const;
};

#endif // QCP_ITEM_LINE_H
