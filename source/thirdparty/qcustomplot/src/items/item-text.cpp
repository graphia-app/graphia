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

#include "item-text.h"

#include "../painter.h"
#include "../core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPItemText
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPItemText
  \brief A text label

  \image html QCPItemText.png "Text example. Blue dotted circles are anchors, solid blue discs are positions."

  Its position is defined by the member \a position and the setting of \ref setPositionAlignment.
  The latter controls which part of the text rect shall be aligned with \a position.
  
  The text alignment itself (i.e. left, center, right) can be controlled with \ref
  setTextAlignment.
  
  The text may be rotated around the \a position point with \ref setRotation.
*/

/*!
  Creates a text item and sets default values.
  
  The created item is automatically registered with \a parentPlot. This QCustomPlot instance takes
  ownership of the item, so do not delete it manually but use QCustomPlot::removeItem() instead.
*/
QCPItemText::QCPItemText(QCustomPlot *parentPlot) :
  QCPAbstractItem(parentPlot),
  position(createPosition(QLatin1String("position"))),
  topLeft(createAnchor(QLatin1String("topLeft"), aiTopLeft)),
  top(createAnchor(QLatin1String("top"), aiTop)),
  topRight(createAnchor(QLatin1String("topRight"), aiTopRight)),
  right(createAnchor(QLatin1String("right"), aiRight)),
  bottomRight(createAnchor(QLatin1String("bottomRight"), aiBottomRight)),
  bottom(createAnchor(QLatin1String("bottom"), aiBottom)),
  bottomLeft(createAnchor(QLatin1String("bottomLeft"), aiBottomLeft)),
  left(createAnchor(QLatin1String("left"), aiLeft)),
  mText(QLatin1String("text")),
  mPositionAlignment(Qt::AlignCenter),
  mTextAlignment(Qt::AlignTop|Qt::AlignHCenter),
  mRotation(0)
{
  position->setCoords(0, 0);
  
  setPen(Qt::NoPen);
  setSelectedPen(Qt::NoPen);
  setBrush(Qt::NoBrush);
  setSelectedBrush(Qt::NoBrush);
  setColor(Qt::black);
  setSelectedColor(Qt::blue);
}

QCPItemText::~QCPItemText()
{
}

/*!
  Sets the color of the text.
*/
void QCPItemText::setColor(const QColor &color)
{
  mColor = color;
}

/*!
  Sets the color of the text that will be used when the item is selected.
*/
void QCPItemText::setSelectedColor(const QColor &color)
{
  mSelectedColor = color;
}

/*!
  Sets the pen that will be used do draw a rectangular border around the text. To disable the
  border, set \a pen to Qt::NoPen.
  
  \see setSelectedPen, setBrush, setPadding
*/
void QCPItemText::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  Sets the pen that will be used do draw a rectangular border around the text, when the item is
  selected. To disable the border, set \a pen to Qt::NoPen.
  
  \see setPen
*/
void QCPItemText::setSelectedPen(const QPen &pen)
{
  mSelectedPen = pen;
}

/*!
  Sets the brush that will be used do fill the background of the text. To disable the
  background, set \a brush to Qt::NoBrush.
  
  \see setSelectedBrush, setPen, setPadding
*/
void QCPItemText::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  Sets the brush that will be used do fill the background of the text, when the item is selected. To disable the
  background, set \a brush to Qt::NoBrush.
  
  \see setBrush
*/
void QCPItemText::setSelectedBrush(const QBrush &brush)
{
  mSelectedBrush = brush;
}

/*!
  Sets the font of the text.
  
  \see setSelectedFont, setColor
*/
void QCPItemText::setFont(const QFont &font)
{
  mFont = font;
}

/*!
  Sets the font of the text that will be used when the item is selected.
  
  \see setFont
*/
void QCPItemText::setSelectedFont(const QFont &font)
{
  mSelectedFont = font;
}

/*!
  Sets the text that will be displayed. Multi-line texts are supported by inserting a line break
  character, e.g. '\n'.
  
  \see setFont, setColor, setTextAlignment
*/
void QCPItemText::setText(const QString &text)
{
  mText = text;
}

/*!
  Sets which point of the text rect shall be aligned with \a position.
  
  Examples:
  \li If \a alignment is <tt>Qt::AlignHCenter | Qt::AlignTop</tt>, the text will be positioned such
  that the top of the text rect will be horizontally centered on \a position.
  \li If \a alignment is <tt>Qt::AlignLeft | Qt::AlignBottom</tt>, \a position will indicate the
  bottom left corner of the text rect.
  
  If you want to control the alignment of (multi-lined) text within the text rect, use \ref
  setTextAlignment.
*/
void QCPItemText::setPositionAlignment(Qt::Alignment alignment)
{
  mPositionAlignment = alignment;
}

/*!
  Controls how (multi-lined) text is aligned inside the text rect (typically Qt::AlignLeft, Qt::AlignCenter or Qt::AlignRight).
*/
void QCPItemText::setTextAlignment(Qt::Alignment alignment)
{
  mTextAlignment = alignment;
}

/*!
  Sets the angle in degrees by which the text (and the text rectangle, if visible) will be rotated
  around \a position.
*/
void QCPItemText::setRotation(double degrees)
{
  mRotation = degrees;
}

/*!
  Sets the distance between the border of the text rectangle and the text. The appearance (and
  visibility) of the text rectangle can be controlled with \ref setPen and \ref setBrush.
*/
void QCPItemText::setPadding(const QMargins &padding)
{
  mPadding = padding;
}

/* inherits documentation from base class */
double QCPItemText::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (onlySelectable && !mSelectable)
    return -1;
  
  // The rect may be rotated, so we transform the actual clicked pos to the rotated
  // coordinate system, so we can use the normal rectDistance function for non-rotated rects:
  QPointF positionPixels(position->pixelPosition());
  QTransform inputTransform;
  inputTransform.translate(positionPixels.x(), positionPixels.y());
  inputTransform.rotate(-mRotation);
  inputTransform.translate(-positionPixels.x(), -positionPixels.y());
  QPointF rotatedPos = inputTransform.map(pos);
  QFontMetrics fontMetrics(mFont);
  QRect textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip|mTextAlignment, mText);
  QRect textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(), mPadding.right(), mPadding.bottom());
  QPointF textPos = getTextDrawPoint(positionPixels, textBoxRect, mPositionAlignment);
  textBoxRect.moveTopLeft(textPos.toPoint());

  return rectDistance(textBoxRect, rotatedPos, true);
}

/* inherits documentation from base class */
void QCPItemText::draw(QCPPainter *painter)
{
  QPointF pos(position->pixelPosition());
  QTransform transform = painter->transform();
  transform.translate(pos.x(), pos.y());
  if (!qFuzzyIsNull(mRotation))
    transform.rotate(mRotation);
  painter->setFont(mainFont());
  QRect textRect = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip|mTextAlignment, mText);
  QRect textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(), mPadding.right(), mPadding.bottom());
  QPointF textPos = getTextDrawPoint(QPointF(0, 0), textBoxRect, mPositionAlignment); // 0, 0 because the transform does the translation
  textRect.moveTopLeft(textPos.toPoint()+QPoint(mPadding.left(), mPadding.top()));
  textBoxRect.moveTopLeft(textPos.toPoint());
  int clipPad = qCeil(mainPen().widthF());
  QRect boundingRect = textBoxRect.adjusted(-clipPad, -clipPad, clipPad, clipPad);
  if (transform.mapRect(boundingRect).intersects(painter->transform().mapRect(clipRect())))
  {
    painter->setTransform(transform);
    if ((mainBrush().style() != Qt::NoBrush && mainBrush().color().alpha() != 0) ||
        (mainPen().style() != Qt::NoPen && mainPen().color().alpha() != 0))
    {
      painter->setPen(mainPen());
      painter->setBrush(mainBrush());
      painter->drawRect(textBoxRect);
    }
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(mainColor()));
    painter->drawText(textRect, Qt::TextDontClip|mTextAlignment, mText);
  }
}

/* inherits documentation from base class */
QPointF QCPItemText::anchorPixelPosition(int anchorId) const
{
  // get actual rect points (pretty much copied from draw function):
  QPointF pos(position->pixelPosition());
  QTransform transform;
  transform.translate(pos.x(), pos.y());
  if (!qFuzzyIsNull(mRotation))
    transform.rotate(mRotation);
  QFontMetrics fontMetrics(mainFont());
  QRect textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip|mTextAlignment, mText);
  QRectF textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(), mPadding.right(), mPadding.bottom());
  QPointF textPos = getTextDrawPoint(QPointF(0, 0), textBoxRect, mPositionAlignment); // 0, 0 because the transform does the translation
  textBoxRect.moveTopLeft(textPos.toPoint());
  QPolygonF rectPoly = transform.map(QPolygonF(textBoxRect));
  
  switch (anchorId)
  {
    case aiTopLeft:     return rectPoly.at(0);
    case aiTop:         return (rectPoly.at(0)+rectPoly.at(1))*0.5;
    case aiTopRight:    return rectPoly.at(1);
    case aiRight:       return (rectPoly.at(1)+rectPoly.at(2))*0.5;
    case aiBottomRight: return rectPoly.at(2);
    case aiBottom:      return (rectPoly.at(2)+rectPoly.at(3))*0.5;
    case aiBottomLeft:  return rectPoly.at(3);
    case aiLeft:        return (rectPoly.at(3)+rectPoly.at(0))*0.5;
  }
  
  qDebug() << Q_FUNC_INFO << "invalid anchorId" << anchorId;
  return {};
}

/*! \internal
  
  Returns the point that must be given to the QPainter::drawText function (which expects the top
  left point of the text rect), according to the position \a pos, the text bounding box \a rect and
  the requested \a positionAlignment.
  
  For example, if \a positionAlignment is <tt>Qt::AlignLeft | Qt::AlignBottom</tt> the returned point
  will be shifted upward by the height of \a rect, starting from \a pos. So if the text is finally
  drawn at that point, the lower left corner of the resulting text rect is at \a pos.
*/
QPointF QCPItemText::getTextDrawPoint(const QPointF &pos, const QRectF &rect, Qt::Alignment positionAlignment) const
{
  if (positionAlignment == 0 || positionAlignment == (Qt::AlignLeft|Qt::AlignTop))
    return pos;
  
  QPointF result = pos; // start at top left
  if (positionAlignment.testFlag(Qt::AlignHCenter))
    result.rx() -= rect.width()/2.0;
  else if (positionAlignment.testFlag(Qt::AlignRight))
    result.rx() -= rect.width();
  if (positionAlignment.testFlag(Qt::AlignVCenter))
    result.ry() -= rect.height()/2.0;
  else if (positionAlignment.testFlag(Qt::AlignBottom))
    result.ry() -= rect.height();
  return result;
}

/*! \internal

  Returns the font that should be used for drawing text. Returns mFont when the item is not selected
  and mSelectedFont when it is.
*/
QFont QCPItemText::mainFont() const
{
  return mSelected ? mSelectedFont : mFont;
}

/*! \internal

  Returns the color that should be used for drawing text. Returns mColor when the item is not
  selected and mSelectedColor when it is.
*/
QColor QCPItemText::mainColor() const
{
  return mSelected ? mSelectedColor : mColor;
}

/*! \internal

  Returns the pen that should be used for drawing lines. Returns mPen when the item is not selected
  and mSelectedPen when it is.
*/
QPen QCPItemText::mainPen() const
{
  return mSelected ? mSelectedPen : mPen;
}

/*! \internal

  Returns the brush that should be used for drawing fills of the item. Returns mBrush when the item
  is not selected and mSelectedBrush when it is.
*/
QBrush QCPItemText::mainBrush() const
{
  return mSelected ? mSelectedBrush : mBrush;
}
