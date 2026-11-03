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

#include "lineending.h"

#include "painter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLineEnding
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPLineEnding
  \brief Handles the different ending decorations for line-like items
  
  \image html QCPLineEnding.png "The various ending styles currently supported"
  
  For every ending a line-like item has, an instance of this class exists. For example, QCPItemLine
  has two endings which can be set with QCPItemLine::setHead and QCPItemLine::setTail.
 
  The styles themselves are defined via the enum QCPLineEnding::EndingStyle. Most decorations can
  be modified regarding width and length, see \ref setWidth and \ref setLength. The direction of
  the ending decoration (e.g. direction an arrow is pointing) is controlled by the line-like item.
  For example, when both endings of a QCPItemLine are set to be arrows, they will point to opposite
  directions, e.g. "outward". This can be changed by \ref setInverted, which would make the
  respective arrow point inward.
  
  Note that due to the overloaded QCPLineEnding constructor, you may directly specify a
  QCPLineEnding::EndingStyle where actually a QCPLineEnding is expected, e.g.
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcplineending-sethead
*/

/*!
  Creates a QCPLineEnding instance with default values (style \ref esNone).
*/
QCPLineEnding::QCPLineEnding() :
  mStyle(esNone),
  mWidth(8),
  mLength(10),
  mInverted(false)
{
}

/*!
  Creates a QCPLineEnding instance with the specified values.
*/
QCPLineEnding::QCPLineEnding(QCPLineEnding::EndingStyle style, double width, double length, bool inverted) :
  mStyle(style),
  mWidth(width),
  mLength(length),
  mInverted(inverted)
{
}

/*!
  Sets the style of the ending decoration.
*/
void QCPLineEnding::setStyle(QCPLineEnding::EndingStyle style)
{
  mStyle = style;
}

/*!
  Sets the width of the ending decoration, if the style supports it. On arrows, for example, the
  width defines the size perpendicular to the arrow's pointing direction.
  
  \see setLength
*/
void QCPLineEnding::setWidth(double width)
{
  mWidth = width;
}

/*!
  Sets the length of the ending decoration, if the style supports it. On arrows, for example, the
  length defines the size in pointing direction.
  
  \see setWidth
*/
void QCPLineEnding::setLength(double length)
{
  mLength = length;
}

/*!
  Sets whether the ending decoration shall be inverted. For example, an arrow decoration will point
  inward when \a inverted is set to true.

  Note that also the \a width direction is inverted. For symmetrical ending styles like arrows or
  discs, this doesn't make a difference. However, asymmetric styles like \ref esHalfBar are
  affected by it, which can be used to control to which side the half bar points to.
*/
void QCPLineEnding::setInverted(bool inverted)
{
  mInverted = inverted;
}

/*! \internal
  
  Returns the maximum pixel radius the ending decoration might cover, starting from the position
  the decoration is drawn at (typically a line ending/\ref QCPItemPosition of an item).
  
  This is relevant for clipping. Only omit painting of the decoration when the position where the
  decoration is supposed to be drawn is farther away from the clipping rect than the returned
  distance.
*/
double QCPLineEnding::boundingDistance() const
{
  switch (mStyle)
  {
    case esNone:
      return 0;
      
    case esFlatArrow:
    case esSpikeArrow:
    case esLineArrow:
    case esSkewedBar:
      return qSqrt(mWidth*mWidth+mLength*mLength); // items that have width and length
      
    case esDisc:
    case esSquare:
    case esDiamond:
    case esBar:
    case esHalfBar:
      return mWidth*1.42; // items that only have a width -> width*sqrt(2)

  }
  return 0;
}

/*!
  Starting from the origin of this line ending (which is style specific), returns the length
  covered by the line ending symbol, in backward direction.
  
  For example, the \ref esSpikeArrow has a shorter real length than a \ref esFlatArrow, even if
  both have the same \ref setLength value, because the spike arrow has an inward curved back, which
  reduces the length along its center axis (the drawing origin for arrows is at the tip).
  
  This function is used for precise, style specific placement of line endings, for example in
  QCPAxes.
*/
double QCPLineEnding::realLength() const
{
  switch (mStyle)
  {
    case esNone:
    case esLineArrow:
    case esSkewedBar:
    case esBar:
    case esHalfBar:
      return 0;
      
    case esFlatArrow:
      return mLength;
      
    case esDisc:
    case esSquare:
    case esDiamond:
      return mWidth*0.5;
      
    case esSpikeArrow:
      return mLength*0.8;
  }
  return 0;
}

/*! \internal
  
  Draws the line ending with the specified \a painter at the position \a pos. The direction of the
  line ending is controlled with \a dir.
*/
void QCPLineEnding::draw(QCPPainter *painter, const QCPVector2D &pos, const QCPVector2D &dir) const
{
  if (mStyle == esNone)
    return;
  
  QCPVector2D lengthVec = dir.normalized() * mLength*(mInverted ? -1 : 1);
  if (lengthVec.isNull())
    lengthVec = QCPVector2D(1, 0);
  QCPVector2D widthVec = dir.normalized().perpendicular() * mWidth*0.5*(mInverted ? -1 : 1);
  
  QPen penBackup = painter->pen();
  QBrush brushBackup = painter->brush();
  QPen miterPen = penBackup;
  miterPen.setJoinStyle(Qt::MiterJoin); // to make arrow heads spikey
  QBrush brush(painter->pen().color(), Qt::SolidPattern);
  switch (mStyle)
  {
    case esNone: break;
    case esFlatArrow:
    {
      QPointF points[3] = {pos.toPointF(),
                           (pos-lengthVec+widthVec).toPointF(),
                           (pos-lengthVec-widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->setBrush(brush);
      painter->drawConvexPolygon(points, 3);
      painter->setBrush(brushBackup);
      painter->setPen(penBackup);
      break;
    }
    case esSpikeArrow:
    {
      QPointF points[4] = {pos.toPointF(),
                           (pos-lengthVec+widthVec).toPointF(),
                           (pos-lengthVec*0.8).toPointF(),
                           (pos-lengthVec-widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->setBrush(brush);
      painter->drawConvexPolygon(points, 4);
      painter->setBrush(brushBackup);
      painter->setPen(penBackup);
      break;
    }
    case esLineArrow:
    {
      QPointF points[3] = {(pos-lengthVec+widthVec).toPointF(),
                           pos.toPointF(),
                           (pos-lengthVec-widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->drawPolyline(points, 3);
      painter->setPen(penBackup);
      break;
    }
    case esDisc:
    {
      painter->setBrush(brush);
      painter->drawEllipse(pos.toPointF(),  mWidth*0.5, mWidth*0.5);
      painter->setBrush(brushBackup);
      break;
    }
    case esSquare:
    {
      QCPVector2D widthVecPerp = widthVec.perpendicular();
      QPointF points[4] = {(pos-widthVecPerp+widthVec).toPointF(),
                           (pos-widthVecPerp-widthVec).toPointF(),
                           (pos+widthVecPerp-widthVec).toPointF(),
                           (pos+widthVecPerp+widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->setBrush(brush);
      painter->drawConvexPolygon(points, 4);
      painter->setBrush(brushBackup);
      painter->setPen(penBackup);
      break;
    }
    case esDiamond:
    {
      QCPVector2D widthVecPerp = widthVec.perpendicular();
      QPointF points[4] = {(pos-widthVecPerp).toPointF(),
                           (pos-widthVec).toPointF(),
                           (pos+widthVecPerp).toPointF(),
                           (pos+widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->setBrush(brush);
      painter->drawConvexPolygon(points, 4);
      painter->setBrush(brushBackup);
      painter->setPen(penBackup);
      break;
    }
    case esBar:
    {
      painter->drawLine((pos+widthVec).toPointF(), (pos-widthVec).toPointF());
      break;
    }
    case esHalfBar:
    {
      painter->drawLine((pos+widthVec).toPointF(), pos.toPointF());
      break;
    }
    case esSkewedBar:
    {
      QCPVector2D shift;
      if (!qFuzzyIsNull(painter->pen().widthF()) || painter->modes().testFlag(QCPPainter::pmNonCosmetic))
        shift = dir.normalized()*qMax(qreal(1.0), painter->pen().widthF())*qreal(0.5);
      // if drawing with thick (non-cosmetic) pen, shift bar a little in line direction to prevent line from sticking through bar slightly
      painter->drawLine((pos+widthVec+lengthVec*0.2*(mInverted?-1:1)+shift).toPointF(),
                        (pos-widthVec-lengthVec*0.2*(mInverted?-1:1)+shift).toPointF());
      break;
    }
  }
}

/*! \internal
  \overload
  
  Draws the line ending. The direction is controlled with the \a angle parameter in radians.
*/
void QCPLineEnding::draw(QCPPainter *painter, const QCPVector2D &pos, double angle) const
{
  draw(painter, pos, QCPVector2D(qCos(angle), qSin(angle)));
}
