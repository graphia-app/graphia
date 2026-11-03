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

#include "item-rect.h"

#include "../painter.h"
#include "../core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPItemRect
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPItemRect
  \brief A rectangle

  \image html QCPItemRect.png "Rectangle example. Blue dotted circles are anchors, solid blue discs are positions."

  It has two positions, \a topLeft and \a bottomRight, which define the rectangle.
*/

/*!
  Creates a rectangle item and sets default values.
  
  The created item is automatically registered with \a parentPlot. This QCustomPlot instance takes
  ownership of the item, so do not delete it manually but use QCustomPlot::removeItem() instead.
*/
QCPItemRect::QCPItemRect(QCustomPlot *parentPlot) :
  QCPAbstractItem(parentPlot),
  topLeft(createPosition(QLatin1String("topLeft"))),
  bottomRight(createPosition(QLatin1String("bottomRight"))),
  top(createAnchor(QLatin1String("top"), aiTop)),
  topRight(createAnchor(QLatin1String("topRight"), aiTopRight)),
  right(createAnchor(QLatin1String("right"), aiRight)),
  bottom(createAnchor(QLatin1String("bottom"), aiBottom)),
  bottomLeft(createAnchor(QLatin1String("bottomLeft"), aiBottomLeft)),
  left(createAnchor(QLatin1String("left"), aiLeft))
{
  topLeft->setCoords(0, 1);
  bottomRight->setCoords(1, 0);
  
  setPen(QPen(Qt::black));
  setSelectedPen(QPen(Qt::blue,2));
  setBrush(Qt::NoBrush);
  setSelectedBrush(Qt::NoBrush);
}

QCPItemRect::~QCPItemRect()
{
}

/*!
  Sets the pen that will be used to draw the line of the rectangle
  
  \see setSelectedPen, setBrush
*/
void QCPItemRect::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  Sets the pen that will be used to draw the line of the rectangle when selected
  
  \see setPen, setSelected
*/
void QCPItemRect::setSelectedPen(const QPen &pen)
{
  mSelectedPen = pen;
}

/*!
  Sets the brush that will be used to fill the rectangle. To disable filling, set \a brush to
  Qt::NoBrush.
  
  \see setSelectedBrush, setPen
*/
void QCPItemRect::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  Sets the brush that will be used to fill the rectangle when selected. To disable filling, set \a
  brush to Qt::NoBrush.
  
  \see setBrush
*/
void QCPItemRect::setSelectedBrush(const QBrush &brush)
{
  mSelectedBrush = brush;
}

/* inherits documentation from base class */
double QCPItemRect::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (onlySelectable && !mSelectable)
    return -1;
  
  QRectF rect = QRectF(topLeft->pixelPosition(), bottomRight->pixelPosition()).normalized();
  bool filledRect = mBrush.style() != Qt::NoBrush && mBrush.color().alpha() != 0;
  return rectDistance(rect, pos, filledRect);
}

/* inherits documentation from base class */
void QCPItemRect::draw(QCPPainter *painter)
{
  QPointF p1 = topLeft->pixelPosition();
  QPointF p2 = bottomRight->pixelPosition();
  if (p1.toPoint() == p2.toPoint())
    return;
  QRectF rect = QRectF(p1, p2).normalized();
  double clipPad = mainPen().widthF();
  QRectF boundingRect = rect.adjusted(-clipPad, -clipPad, clipPad, clipPad);
  if (boundingRect.intersects(clipRect())) // only draw if bounding rect of rect item is visible in cliprect
  {
    painter->setPen(mainPen());
    painter->setBrush(mainBrush());
    painter->drawRect(rect);
  }
}

/* inherits documentation from base class */
QPointF QCPItemRect::anchorPixelPosition(int anchorId) const
{
  QRectF rect = QRectF(topLeft->pixelPosition(), bottomRight->pixelPosition());
  switch (anchorId)
  {
    case aiTop:         return (rect.topLeft()+rect.topRight())*0.5;
    case aiTopRight:    return rect.topRight();
    case aiRight:       return (rect.topRight()+rect.bottomRight())*0.5;
    case aiBottom:      return (rect.bottomLeft()+rect.bottomRight())*0.5;
    case aiBottomLeft:  return rect.bottomLeft();
    case aiLeft:        return (rect.topLeft()+rect.bottomLeft())*0.5;
  }
  
  qDebug() << Q_FUNC_INFO << "invalid anchorId" << anchorId;
  return {};
}

/*! \internal

  Returns the pen that should be used for drawing lines. Returns mPen when the item is not selected
  and mSelectedPen when it is.
*/
QPen QCPItemRect::mainPen() const
{
  return mSelected ? mSelectedPen : mPen;
}

/*! \internal

  Returns the brush that should be used for drawing fills of the item. Returns mBrush when the item
  is not selected and mSelectedBrush when it is.
*/
QBrush QCPItemRect::mainBrush() const
{
  return mSelected ? mSelectedBrush : mBrush;
}
