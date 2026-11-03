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

#ifndef QCP_SELECTIONRECT_H
#define QCP_SELECTIONRECT_H

#include "global.h"
#include "layer.h"
#include "axis/range.h"

class QCPAxis;

class QCP_LIB_DECL QCPSelectionRect : public QCPLayerable
{
  Q_OBJECT
public:
  explicit QCPSelectionRect(QCustomPlot *parentPlot);
  virtual ~QCPSelectionRect() Q_DECL_OVERRIDE;
  
  // getters:
  QRect rect() const { return mRect; }
  QCPRange range(const QCPAxis *axis) const;
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  bool isActive() const { return mActive; }
  
  // setters:
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  
  // non-property methods:
  Q_SLOT void cancel();
  
signals:
  void started(QMouseEvent *event);
  void changed(const QRect &rect, QMouseEvent *event);
  void canceled(const QRect &rect, QInputEvent *event);
  void accepted(const QRect &rect, QMouseEvent *event);
  
protected:
  // property members:
  QRect mRect;
  QPen mPen;
  QBrush mBrush;
  // non-property members:
  bool mActive;
  
  // introduced virtual methods:
  virtual void startSelection(QMouseEvent *event);
  virtual void moveSelection(QMouseEvent *event);
  virtual void endSelection(QMouseEvent *event);
  virtual void keyPressEvent(QKeyEvent *event);
  
  // reimplemented virtual methods
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  
  friend class QCustomPlot;
};

#endif // QCP_SELECTIONRECT_H
