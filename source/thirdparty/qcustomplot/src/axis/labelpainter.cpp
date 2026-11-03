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

#include "labelpainter.h"

#include "../painter.h"
#include "../core.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLabelPainterPrivate
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPLabelPainterPrivate

  \internal
  \brief (Private)
  
  This is a private class and not part of the public QCustomPlot interface.
  
*/

const QChar QCPLabelPainterPrivate::SymbolDot(183);
const QChar QCPLabelPainterPrivate::SymbolCross(215);

/*!
  Constructs a QCPLabelPainterPrivate instance. Make sure to not create a new
  instance on every redraw, to utilize the caching mechanisms.
  
  the \a parentPlot does not take ownership of the label painter. Make sure
  to delete it appropriately.
*/
QCPLabelPainterPrivate::QCPLabelPainterPrivate(QCustomPlot *parentPlot) :
  mAnchorMode(amRectangular),
  mAnchorSide(asLeft),
  mAnchorReferenceType(artNormal),
  mColor(Qt::black),
  mPadding(0),
  mRotation(0),
  mSubstituteExponent(true),
  mMultiplicationSymbol(QChar(215)),
  mAbbreviateDecimalPowers(false),
  mParentPlot(parentPlot),
  mLabelCache(16)
{
  analyzeFontMetrics();
}

QCPLabelPainterPrivate::~QCPLabelPainterPrivate()
{
}

void QCPLabelPainterPrivate::setAnchorSide(AnchorSide side)
{
  mAnchorSide = side;
}

void QCPLabelPainterPrivate::setAnchorMode(AnchorMode mode)
{
  mAnchorMode = mode;
}

void QCPLabelPainterPrivate::setAnchorReference(const QPointF &pixelPoint)
{
  mAnchorReference = pixelPoint;
}

void QCPLabelPainterPrivate::setAnchorReferenceType(AnchorReferenceType type)
{
  mAnchorReferenceType = type;
}

void QCPLabelPainterPrivate::setFont(const QFont &font)
{
  if (mFont != font)
  {
    mFont = font;
    analyzeFontMetrics();
  }
}

void QCPLabelPainterPrivate::setColor(const QColor &color)
{
  mColor = color;
}

void QCPLabelPainterPrivate::setPadding(int padding)
{
  mPadding = padding;
}

void QCPLabelPainterPrivate::setRotation(double rotation)
{
  mRotation = qBound(-90.0, rotation, 90.0);
}

void QCPLabelPainterPrivate::setSubstituteExponent(bool enabled)
{
  mSubstituteExponent = enabled;
}

void QCPLabelPainterPrivate::setMultiplicationSymbol(QChar symbol)
{
  mMultiplicationSymbol = symbol;
}

void QCPLabelPainterPrivate::setAbbreviateDecimalPowers(bool enabled)
{
  mAbbreviateDecimalPowers = enabled;
}

void QCPLabelPainterPrivate::setCacheSize(int labelCount)
{
  mLabelCache.setMaxCost(labelCount);
}

int QCPLabelPainterPrivate::cacheSize() const
{
  return mLabelCache.maxCost();
}

void QCPLabelPainterPrivate::drawTickLabel(QCPPainter *painter, const QPointF &tickPos, const QString &text)
{
  double realRotation = mRotation;
  
  AnchorSide realSide = mAnchorSide;
  // for circular axes, the anchor side is determined depending on the quadrant of tickPos with respect to mCircularReference
  if (mAnchorMode == amSkewedUpright)
  {
    realSide = skewedAnchorSide(tickPos, 0.2, 0.3); 
  } else if (mAnchorMode == amSkewedRotated) // in this mode every label is individually rotated to match circle tangent
  {
    realSide = skewedAnchorSide(tickPos, 0, 0);
    realRotation += QCPVector2D(tickPos-mAnchorReference).angle()/M_PI*180.0;
    if (realRotation > 90) realRotation -= 180;
    else if (realRotation < -90) realRotation += 180;
  }
  
  realSide = rotationCorrectedSide(realSide, realRotation); // rotation angles may change the true anchor side of the label
  drawLabelMaybeCached(painter, mFont, mColor, getAnchorPos(tickPos), realSide, realRotation, text);
}

/*! \internal
  
  Returns the size ("margin" in QCPAxisRect context, so measured perpendicular to the axis backbone
  direction) needed to fit the axis.
*/
/* TODO: needed?
int QCPLabelPainterPrivate::size() const
{
  int result = 0;
  // get length of tick marks pointing outwards:
  if (!tickPositions.isEmpty())
    result += qMax(0, qMax(tickLengthOut, subTickLengthOut));
  
  // calculate size of tick labels:
  if (tickLabelSide == QCPAxis::lsOutside)
  {
    QSize tickLabelsSize(0, 0);
    if (!tickLabels.isEmpty())
    {
      for (int i=0; i<tickLabels.size(); ++i)
        getMaxTickLabelSize(tickLabelFont, tickLabels.at(i), &tickLabelsSize);
      result += QCPAxis::orientation(type) == Qt::Horizontal ? tickLabelsSize.height() : tickLabelsSize.width();
    result += tickLabelPadding;
    }
  }
  
  // calculate size of axis label (only height needed, because left/right labels are rotated by 90 degrees):
  if (!label.isEmpty())
  {
    QFontMetrics fontMetrics(labelFont);
    QRect bounds;
    bounds = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignHCenter | Qt::AlignVCenter, label);
    result += bounds.height() + labelPadding;
  }

  return result;
}
*/

/*! \internal
  
  Clears the internal label cache. Upon the next \ref draw, all labels will be created new. This
  method is called automatically if any parameters have changed that invalidate the cached labels,
  such as font, color, etc. Usually you won't need to call this method manually.
*/
void QCPLabelPainterPrivate::clearCache()
{
  mLabelCache.clear();
}

/*! \internal
  
  Returns a hash that allows uniquely identifying whether the label parameters have changed such
  that the cached labels must be refreshed (\ref clearCache). It is used in \ref draw. If the
  return value of this method hasn't changed since the last redraw, the respective label parameters
  haven't changed and cached labels may be used.
*/
QByteArray QCPLabelPainterPrivate::generateLabelParameterHash() const
{
  QByteArray result;
  result.append(QByteArray::number(mParentPlot->bufferDevicePixelRatio()));
  result.append(QByteArray::number(mRotation));
  //result.append(QByteArray::number(int(tickLabelSide))); TODO: check whether this is really a cache-invalidating property
  result.append(QByteArray::number(int(mSubstituteExponent)));
  result.append(QString(mMultiplicationSymbol).toUtf8());
  result.append(mColor.name().toLatin1()+QByteArray::number(mColor.alpha(), 16));
  result.append(mFont.toString().toLatin1());
  return result;
}

/*! \internal
  
  Draws a single tick label with the provided \a painter, utilizing the internal label cache to
  significantly speed up drawing of labels that were drawn in previous calls. The tick label is
  always bound to an axis, the distance to the axis is controllable via \a distanceToAxis in
  pixels. The pixel position in the axis direction is passed in the \a position parameter. Hence
  for the bottom axis, \a position would indicate the horizontal pixel position (not coordinate),
  at which the label should be drawn.
  
  In order to later draw the axis label in a place that doesn't overlap with the tick labels, the
  largest tick label size is needed. This is acquired by passing a \a tickLabelsSize to the \ref
  drawTickLabel calls during the process of drawing all tick labels of one axis. In every call, \a
  tickLabelsSize is expanded, if the drawn label exceeds the value \a tickLabelsSize currently
  holds.
  
  The label is drawn with the font and pen that are currently set on the \a painter. To draw
  superscripted powers, the font is temporarily made smaller by a fixed factor (see \ref
  getTickLabelData).
*/
void QCPLabelPainterPrivate::drawLabelMaybeCached(QCPPainter *painter, const QFont &font, const QColor &color, const QPointF &pos, AnchorSide side, double rotation, const QString &text)
{
  // warning: if you change anything here, also adapt getMaxTickLabelSize() accordingly!
  if (text.isEmpty()) return;
  QSize finalSize;

  if (mParentPlot->plottingHints().testFlag(QCP::phCacheLabels) && !painter->modes().testFlag(QCPPainter::pmNoCaching)) // label caching enabled
  {
    QByteArray key = cacheKey(text, color, rotation, side);
    CachedLabel *cachedLabel = mLabelCache.take(QString::fromUtf8(key)); // attempt to take label from cache (don't use object() because we want ownership/prevent deletion during our operations, we re-insert it afterwards)
    if (!cachedLabel)  // no cached label existed, create it
    {
      LabelData labelData = getTickLabelData(font, color, rotation, side, text);
      cachedLabel = createCachedLabel(labelData);
    }
    // if label would be partly clipped by widget border on sides, don't draw it (only for outside tick labels):
    bool labelClippedByBorder = false;
    /*
    if (tickLabelSide == QCPAxis::lsOutside)
    {
      if (QCPAxis::orientation(type) == Qt::Horizontal)
        labelClippedByBorder = labelAnchor.x()+cachedLabel->offset.x()+cachedLabel->pixmap.width()/mParentPlot->bufferDevicePixelRatio() > viewportRect.right() || labelAnchor.x()+cachedLabel->offset.x() < viewportRect.left();
      else
        labelClippedByBorder = labelAnchor.y()+cachedLabel->offset.y()+cachedLabel->pixmap.height()/mParentPlot->bufferDevicePixelRatio() > viewportRect.bottom() || labelAnchor.y()+cachedLabel->offset.y() < viewportRect.top();
    }
    */
    if (!labelClippedByBorder)
    {
      painter->drawPixmap(pos+cachedLabel->offset, cachedLabel->pixmap);
      finalSize = cachedLabel->pixmap.size()/mParentPlot->bufferDevicePixelRatio(); // TODO: collect this in a member rect list?
    }
    mLabelCache.insert(QString::fromUtf8(key), cachedLabel);
  } else // label caching disabled, draw text directly on surface:
  {
    LabelData labelData = getTickLabelData(font, color, rotation, side, text);
    // if label would be partly clipped by widget border on sides, don't draw it (only for outside tick labels):
     bool labelClippedByBorder = false;
     /*
    if (tickLabelSide == QCPAxis::lsOutside)
    {
      if (QCPAxis::orientation(type) == Qt::Horizontal)
        labelClippedByBorder = finalPosition.x()+(labelData.rotatedTotalBounds.width()+labelData.rotatedTotalBounds.left()) > viewportRect.right() || finalPosition.x()+labelData.rotatedTotalBounds.left() < viewportRect.left();
      else
        labelClippedByBorder = finalPosition.y()+(labelData.rotatedTotalBounds.height()+labelData.rotatedTotalBounds.top()) > viewportRect.bottom() || finalPosition.y()+labelData.rotatedTotalBounds.top() < viewportRect.top();
    }
    */
    if (!labelClippedByBorder)
    {
      drawText(painter, pos, labelData);
      finalSize = labelData.rotatedTotalBounds.size();
    }
  }
  /*
  // expand passed tickLabelsSize if current tick label is larger:
  if (finalSize.width() > tickLabelsSize->width())
    tickLabelsSize->setWidth(finalSize.width());
  if (finalSize.height() > tickLabelsSize->height())
    tickLabelsSize->setHeight(finalSize.height());
  */
}

QPointF QCPLabelPainterPrivate::getAnchorPos(const QPointF &tickPos)
{
  switch (mAnchorMode)
  {
    case amRectangular:
    {
      switch (mAnchorSide)
      {
        case asLeft:   return tickPos+QPointF(mPadding, 0);
        case asRight:  return tickPos+QPointF(-mPadding, 0);
        case asTop:    return tickPos+QPointF(0, mPadding);
        case asBottom: return tickPos+QPointF(0, -mPadding);
        case asTopLeft:     return tickPos+QPointF(mPadding*M_SQRT1_2, mPadding*M_SQRT1_2);
        case asTopRight:    return tickPos+QPointF(-mPadding*M_SQRT1_2, mPadding*M_SQRT1_2);
        case asBottomRight: return tickPos+QPointF(-mPadding*M_SQRT1_2, -mPadding*M_SQRT1_2);
        case asBottomLeft:  return tickPos+QPointF(mPadding*M_SQRT1_2, -mPadding*M_SQRT1_2);
        default: qDebug() << Q_FUNC_INFO << "invalid mode for anchor side: " << mAnchorSide; break;
      }
      break;
    }
    case amSkewedUpright:
      // fall through
    case amSkewedRotated:
    {
      QCPVector2D anchorNormal(tickPos-mAnchorReference);
      if (mAnchorReferenceType == artTangent)
        anchorNormal = anchorNormal.perpendicular();
      anchorNormal.normalize();
      return tickPos+(anchorNormal*mPadding).toPointF();
    }
    default: qDebug() << Q_FUNC_INFO << "invalid mode for anchor mode: " << mAnchorMode; break;
  }
  return tickPos;
}

/*! \internal
  
  This is a \ref placeTickLabel helper function.
  
  Draws the tick label specified in \a labelData with \a painter at the pixel positions \a x and \a
  y. This function is used by \ref placeTickLabel to create new tick labels for the cache, or to
  directly draw the labels on the QCustomPlot surface when label caching is disabled, i.e. when
  QCP::phCacheLabels plotting hint is not set.
*/
void QCPLabelPainterPrivate::drawText(QCPPainter *painter, const QPointF &pos, const LabelData &labelData) const
{
  // backup painter settings that we're about to change:
  QTransform oldTransform = painter->transform();
  QFont oldFont = painter->font();
  QPen oldPen = painter->pen();
  
  // transform painter to position/rotation:
  painter->translate(pos);
  painter->setTransform(labelData.transform, true);
  
  // draw text:
  painter->setFont(labelData.baseFont);
  painter->setPen(QPen(labelData.color));
  if (!labelData.expPart.isEmpty()) // use superscripted exponent typesetting
  {
    painter->drawText(0, 0, 0, 0, Qt::TextDontClip, labelData.basePart);
    if (!labelData.suffixPart.isEmpty())
      painter->drawText(labelData.baseBounds.width()+1+labelData.expBounds.width(), 0, 0, 0, Qt::TextDontClip, labelData.suffixPart);
    painter->setFont(labelData.expFont);
    painter->drawText(labelData.baseBounds.width()+1, 0, labelData.expBounds.width(), labelData.expBounds.height(), Qt::TextDontClip,  labelData.expPart);
  } else
  {
    painter->drawText(0, 0, labelData.totalBounds.width(), labelData.totalBounds.height(), Qt::TextDontClip | Qt::AlignHCenter, labelData.basePart);
  }
  
  /* Debug code to draw label bounding boxes, baseline, and capheight
  painter->save();
  painter->setPen(QPen(QColor(0, 0, 0, 150)));
  painter->drawRect(labelData.totalBounds);
  const int baseline = labelData.totalBounds.height()-mLetterDescent;
  painter->setPen(QPen(QColor(255, 0, 0, 150)));
  painter->drawLine(QLineF(0, baseline, labelData.totalBounds.width(), baseline));
  painter->setPen(QPen(QColor(0, 0, 255, 150)));
  painter->drawLine(QLineF(0, baseline-mLetterCapHeight, labelData.totalBounds.width(), baseline-mLetterCapHeight));
  painter->restore();
  */
  
  // reset painter settings to what it was before:
  painter->setTransform(oldTransform);
  painter->setFont(oldFont);
  painter->setPen(oldPen);
}

/*! \internal
  
  This is a \ref placeTickLabel helper function.
  
  Transforms the passed \a text and \a font to a tickLabelData structure that can then be further
  processed by \ref getTickLabelDrawOffset and \ref drawTickLabel. It splits the text into base and
  exponent if necessary (member substituteExponent) and calculates appropriate bounding boxes.
*/
QCPLabelPainterPrivate::LabelData QCPLabelPainterPrivate::getTickLabelData(const QFont &font, const QColor &color, double rotation, AnchorSide side, const QString &text) const
{
  LabelData result;
  result.rotation = rotation;
  result.side = side;
  result.color = color;
  
  // determine whether beautiful decimal powers should be used
  bool useBeautifulPowers = false;
  int ePos = -1; // first index of exponent part, text before that will be basePart, text until eLast will be expPart
  int eLast = -1; // last index of exponent part, rest of text after this will be suffixPart
  if (mSubstituteExponent)
  {
    ePos = text.indexOf(QLatin1Char('e'));
    if (ePos > 0 && text.at(ePos-1).isDigit())
    {
      eLast = ePos;
      while (eLast+1 < text.size() && (text.at(eLast+1) == QLatin1Char('+') || text.at(eLast+1) == QLatin1Char('-') || text.at(eLast+1).isDigit()))
        ++eLast;
      if (eLast > ePos) // only if also to right of 'e' is a digit/+/- interpret it as beautifiable power
        useBeautifulPowers = true;
    }
  }
  
  // calculate text bounding rects and do string preparation for beautiful decimal powers:
  result.baseFont = font;
  if (result.baseFont.pointSizeF() > 0) // might return -1 if specified with setPixelSize, in that case we can't do correction in next line
    result.baseFont.setPointSizeF(result.baseFont.pointSizeF()+0.05); // QFontMetrics.boundingRect has a bug for exact point sizes that make the results oscillate due to internal rounding
  
  QFontMetrics baseFontMetrics(result.baseFont);
  if (useBeautifulPowers)
  {
    // split text into parts of number/symbol that will be drawn normally and part that will be drawn as exponent:
    result.basePart = text.left(ePos);
    result.suffixPart = text.mid(eLast+1); // also drawn normally but after exponent
    // in log scaling, we want to turn "1*10^n" into "10^n", else add multiplication sign and decimal base:
    if (mAbbreviateDecimalPowers && result.basePart == QLatin1String("1"))
      result.basePart = QLatin1String("10");
    else
      result.basePart += QString(mMultiplicationSymbol) + QLatin1String("10");
    result.expPart = text.mid(ePos+1, eLast-ePos);
    // clip "+" and leading zeros off expPart:
    while (result.expPart.length() > 2 && result.expPart.at(1) == QLatin1Char('0')) // length > 2 so we leave one zero when numberFormatChar is 'e'
      result.expPart.remove(1, 1);
    if (!result.expPart.isEmpty() && result.expPart.at(0) == QLatin1Char('+'))
      result.expPart.remove(0, 1);
    // prepare smaller font for exponent:
    result.expFont = font;
    if (result.expFont.pointSize() > 0)
      result.expFont.setPointSize(result.expFont.pointSize()*0.75);
    else
      result.expFont.setPixelSize(result.expFont.pixelSize()*0.75);
    // calculate bounding rects of base part(s), exponent part and total one:
    result.baseBounds = baseFontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip, result.basePart);
    result.expBounds = QFontMetrics(result.expFont).boundingRect(0, 0, 0, 0, Qt::TextDontClip, result.expPart);
    if (!result.suffixPart.isEmpty())
      result.suffixBounds = QFontMetrics(result.baseFont).boundingRect(0, 0, 0, 0, Qt::TextDontClip, result.suffixPart);
    result.totalBounds = result.baseBounds.adjusted(0, 0, result.expBounds.width()+result.suffixBounds.width()+2, 0); // +2 consists of the 1 pixel spacing between base and exponent (see drawTickLabel) and an extra pixel to include AA
  } else // useBeautifulPowers == false
  {
    result.basePart = text;
    result.totalBounds = baseFontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignHCenter, result.basePart);
  }
  result.totalBounds.moveTopLeft(QPoint(0, 0));
  applyAnchorTransform(result);
  result.rotatedTotalBounds = result.transform.mapRect(result.totalBounds);
  
  return result;
}

void QCPLabelPainterPrivate::applyAnchorTransform(LabelData &labelData) const
{
  if (!qFuzzyIsNull(labelData.rotation))
    labelData.transform.rotate(labelData.rotation); // rotates effectively clockwise (due to flipped y axis of painter vs widget coordinate system)
  
  // from now on we translate in rotated label-local coordinate system.
  // shift origin of coordinate system to appropriate point on label:
  labelData.transform.translate(0, -labelData.totalBounds.height()+mLetterDescent+mLetterCapHeight); // shifts origin to true top of capital (or number) characters
  
  if (labelData.side == asLeft || labelData.side == asRight) // anchor is centered vertically
    labelData.transform.translate(0, -mLetterCapHeight/2.0);
  else if (labelData.side == asTop || labelData.side == asBottom) // anchor is centered horizontally
    labelData.transform.translate(-labelData.totalBounds.width()/2.0, 0);
  
  if (labelData.side == asTopRight || labelData.side == asRight || labelData.side == asBottomRight) // anchor is at right
    labelData.transform.translate(-labelData.totalBounds.width(), 0);
  if (labelData.side == asBottomLeft || labelData.side == asBottom || labelData.side == asBottomRight) // anchor is at bottom (no elseif!)
    labelData.transform.translate(0, -mLetterCapHeight);
}

/*! \internal
  
  Simulates the steps done by \ref placeTickLabel by calculating bounding boxes of the text label
  to be drawn, depending on number format etc. Since only the largest tick label is wanted for the
  margin calculation, the passed \a tickLabelsSize is only expanded, if it's currently set to a
  smaller width/height.
*/
/*
void QCPLabelPainterPrivate::getMaxTickLabelSize(const QFont &font, const QString &text,  QSize *tickLabelsSize) const
{
  // note: this function must return the same tick label sizes as the placeTickLabel function.
  QSize finalSize;
  if (mParentPlot->plottingHints().testFlag(QCP::phCacheLabels) && mLabelCache.contains(text)) // label caching enabled and have cached label
  {
    const CachedLabel *cachedLabel = mLabelCache.object(text);
    finalSize = cachedLabel->pixmap.size()/mParentPlot->bufferDevicePixelRatio();
  } else // label caching disabled or no label with this text cached:
  {
    // TODO: LabelData labelData = getTickLabelData(font, text);
    // TODO: finalSize = labelData.rotatedTotalBounds.size();
  }
  
  // expand passed tickLabelsSize if current tick label is larger:
  if (finalSize.width() > tickLabelsSize->width())
    tickLabelsSize->setWidth(finalSize.width());
  if (finalSize.height() > tickLabelsSize->height())
    tickLabelsSize->setHeight(finalSize.height());
}
*/

QCPLabelPainterPrivate::CachedLabel *QCPLabelPainterPrivate::createCachedLabel(const LabelData &labelData) const
{
  CachedLabel *result = new CachedLabel;
  
  // allocate pixmap with the correct size and pixel ratio:
  if (!qFuzzyCompare(1.0, mParentPlot->bufferDevicePixelRatio()))
  {
    result->pixmap = QPixmap(labelData.rotatedTotalBounds.size()*mParentPlot->bufferDevicePixelRatio());
#ifdef QCP_DEVICEPIXELRATIO_SUPPORTED
#  ifdef QCP_DEVICEPIXELRATIO_FLOAT
    result->pixmap.setDevicePixelRatio(mParentPlot->bufferDevicePixelRatio());
#  else
    result->pixmap.setDevicePixelRatio(mParentPlot->bufferDevicePixelRatio());
#  endif
#endif
  } else
    result->pixmap = QPixmap(labelData.rotatedTotalBounds.size());
  result->pixmap.fill(Qt::transparent);
  
  // draw the label into the pixmap
  // offset is between label anchor and topleft of cache pixmap, so pixmap can be drawn at pos+offset to make the label anchor appear at pos.
  // We use rotatedTotalBounds.topLeft() because rotatedTotalBounds is in a coordinate system where the label anchor is at (0, 0)
  result->offset = labelData.rotatedTotalBounds.topLeft();
  QCPPainter cachePainter(&result->pixmap);
  drawText(&cachePainter, -result->offset, labelData);
  return result;
}

QByteArray QCPLabelPainterPrivate::cacheKey(const QString &text, const QColor &color, double rotation, AnchorSide side) const
{
  return text.toUtf8()+
      QByteArray::number(color.red()+256*color.green()+65536*color.blue(), 36)+
      QByteArray::number(color.alpha()+256*int(side), 36)+
      QByteArray::number(int(rotation*100), 36);
}

QCPLabelPainterPrivate::AnchorSide QCPLabelPainterPrivate::skewedAnchorSide(const QPointF &tickPos, double sideExpandHorz, double sideExpandVert) const
{
  QCPVector2D anchorNormal = QCPVector2D(tickPos-mAnchorReference);
  if (mAnchorReferenceType == artTangent)
    anchorNormal = anchorNormal.perpendicular();
  const double radius = anchorNormal.length();
  const double sideHorz = sideExpandHorz*radius;
  const double sideVert = sideExpandVert*radius;
  if (anchorNormal.x() > sideHorz)
  {
    if (anchorNormal.y() > sideVert) return asTopLeft;
    else if (anchorNormal.y() < -sideVert) return asBottomLeft;
    else return asLeft;
  } else if (anchorNormal.x() < -sideHorz)
  {
    if (anchorNormal.y() > sideVert) return asTopRight;
    else if (anchorNormal.y() < -sideVert) return asBottomRight;
    else return asRight;
  } else
  {
    if (anchorNormal.y() > 0) return asTop;
    else return asBottom;
  }
  return asBottom; // should never be reached
}

QCPLabelPainterPrivate::AnchorSide QCPLabelPainterPrivate::rotationCorrectedSide(AnchorSide side, double rotation) const
{
  AnchorSide result = side;
  const bool rotateClockwise = rotation > 0;
  if (!qFuzzyIsNull(rotation))
  {
    if (!qFuzzyCompare(qAbs(rotation), 90)) // avoid graphical collision with anchor tangent (e.g. axis line) when rotating, so change anchor side appropriately:
    {
      if (side == asTop) result = rotateClockwise ? asLeft : asRight;
      else if (side == asBottom) result = rotateClockwise ? asRight : asLeft;
      else if (side == asTopLeft) result = rotateClockwise ? asLeft : asTop;
      else if (side == asTopRight) result = rotateClockwise ? asTop : asRight;
      else if (side == asBottomLeft) result = rotateClockwise ? asBottom : asLeft;
      else if (side == asBottomRight) result = rotateClockwise ? asRight : asBottom;
    } else // for full rotation by +/-90 degrees, other sides are more appropriate for centering on anchor:
    {
      if (side == asLeft) result = rotateClockwise ? asBottom : asTop;
      else if (side == asRight) result = rotateClockwise ? asTop : asBottom;
      else if (side == asTop) result = rotateClockwise ? asLeft : asRight;
      else if (side == asBottom) result = rotateClockwise ? asRight : asLeft;
      else if (side == asTopLeft) result = rotateClockwise ? asBottomLeft : asTopRight;
      else if (side == asTopRight) result = rotateClockwise ? asTopLeft : asBottomRight;
      else if (side == asBottomLeft) result = rotateClockwise ? asBottomRight : asTopLeft;
      else if (side == asBottomRight) result = rotateClockwise ? asTopRight : asBottomLeft;
    }
  }
  return result;
}

void QCPLabelPainterPrivate::analyzeFontMetrics()
{
  const QFontMetrics fm(mFont);
  mLetterCapHeight = fm.tightBoundingRect(QLatin1String("8")).height(); // this method is slow, that's why we query it only upon font change
  mLetterDescent = fm.descent();
}

