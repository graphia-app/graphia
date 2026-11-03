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

#ifndef QCP_SELECTIONDECORATOR_BRACKET_H
#define QCP_SELECTIONDECORATOR_BRACKET_H

#include "plottable.h"
#include "vector2d.h"

class QCP_LIB_DECL QCPSelectionDecoratorBracket : public QCPSelectionDecorator
{
  Q_GADGET
public:
  
  /*!
    Defines which shape is drawn at the boundaries of selected data ranges.
    
    Some of the bracket styles further allow specifying a height and/or width, see \ref
    setBracketHeight and \ref setBracketWidth.
  */
  enum BracketStyle { bsSquareBracket ///< A square bracket is drawn.
                      ,bsHalfEllipse   ///< A half ellipse is drawn. The size of the ellipse is given by the bracket width/height properties.
                      ,bsEllipse       ///< An ellipse is drawn. The size of the ellipse is given by the bracket width/height properties.
                      ,bsPlus         ///< A plus is drawn.
                      ,bsUserStyle    ///< Start custom bracket styles at this index when subclassing and reimplementing \ref drawBracket.
  };
  Q_ENUM(BracketStyle)
  
  QCPSelectionDecoratorBracket();
  virtual ~QCPSelectionDecoratorBracket() Q_DECL_OVERRIDE;
  
  // getters:
  QPen bracketPen() const { return mBracketPen; }
  QBrush bracketBrush() const { return mBracketBrush; }
  int bracketWidth() const { return mBracketWidth; }
  int bracketHeight() const { return mBracketHeight; }
  BracketStyle bracketStyle() const { return mBracketStyle; }
  bool tangentToData() const { return mTangentToData; }
  int tangentAverage() const { return mTangentAverage; }
  
  // setters:
  void setBracketPen(const QPen &pen);
  void setBracketBrush(const QBrush &brush);
  void setBracketWidth(int width);
  void setBracketHeight(int height);
  void setBracketStyle(BracketStyle style);
  void setTangentToData(bool enabled);
  void setTangentAverage(int pointCount);
  
  // introduced virtual methods:
  virtual void drawBracket(QCPPainter *painter, int direction) const;
  
  // virtual methods:
  virtual void drawDecoration(QCPPainter *painter, QCPDataSelection selection) Q_DECL_OVERRIDE;
  
protected:
  // property members:
  QPen mBracketPen;
  QBrush mBracketBrush;
  int mBracketWidth;
  int mBracketHeight;
  BracketStyle mBracketStyle;
  bool mTangentToData;
  int mTangentAverage;
  
  // non-virtual methods:
  double getTangentAngle(const QCPPlottableInterface1D *interface1d, int dataIndex, int direction) const;
  QPointF getPixelCoordinates(const QCPPlottableInterface1D *interface1d, int dataIndex) const;
  
};
Q_DECLARE_METATYPE(QCPSelectionDecoratorBracket::BracketStyle)

#endif // QCP_SELECTIONDECORATOR_BRACKET_H
