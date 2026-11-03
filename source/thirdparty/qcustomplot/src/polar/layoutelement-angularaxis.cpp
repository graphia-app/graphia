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

#include "layoutelement-angularaxis.h"
#include "radialaxis.h"
#include "polargrid.h"
#include "polargraph.h"

#include "../painter.h"
#include "../axis/labelpainter.h"
#include "../axis/axistickerfixed.h"
#include "../core.h"
#include "../item.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPolarAxisAngular
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPolarAxisAngular
  \brief The main container for polar plots, representing the angular axis as a circle

  \warning In this QCustomPlot version, polar plots are a tech preview. Expect documentation and
  functionality to be incomplete, as well as changing public interfaces in the future.
*/

/* start documentation of inline functions */

/*! \fn QCPLayoutInset *QCPPolarAxisAngular::insetLayout() const
  
  Returns the inset layout of this axis rect. It can be used to place other layout elements (or
  even layouts with multiple other elements) inside/on top of an axis rect.
  
  \see QCPLayoutInset
*/

/*! \fn int QCPPolarAxisAngular::left() const
  
  Returns the pixel position of the left border of this axis rect. Margins are not taken into
  account here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPPolarAxisAngular::right() const
  
  Returns the pixel position of the right border of this axis rect. Margins are not taken into
  account here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPPolarAxisAngular::top() const
  
  Returns the pixel position of the top border of this axis rect. Margins are not taken into
  account here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPPolarAxisAngular::bottom() const
  
  Returns the pixel position of the bottom border of this axis rect. Margins are not taken into
  account here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPPolarAxisAngular::width() const
  
  Returns the pixel width of this axis rect. Margins are not taken into account here, so the
  returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPPolarAxisAngular::height() const
  
  Returns the pixel height of this axis rect. Margins are not taken into account here, so the
  returned value is with respect to the inner \ref rect.
*/

/*! \fn QSize QCPPolarAxisAngular::size() const
  
  Returns the pixel size of this axis rect. Margins are not taken into account here, so the
  returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPPolarAxisAngular::topLeft() const
  
  Returns the top left corner of this axis rect in pixels. Margins are not taken into account here,
  so the returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPPolarAxisAngular::topRight() const
  
  Returns the top right corner of this axis rect in pixels. Margins are not taken into account
  here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPPolarAxisAngular::bottomLeft() const
  
  Returns the bottom left corner of this axis rect in pixels. Margins are not taken into account
  here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPPolarAxisAngular::bottomRight() const
  
  Returns the bottom right corner of this axis rect in pixels. Margins are not taken into account
  here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPPolarAxisAngular::center() const
  
  Returns the center of this axis rect in pixels. Margins are not taken into account here, so the
  returned value is with respect to the inner \ref rect.
*/

/* end documentation of inline functions */

/*!
  Creates a QCPPolarAxis instance and sets default values. An axis is added for each of the four
  sides, the top and right axes are set invisible initially.
*/
QCPPolarAxisAngular::QCPPolarAxisAngular(QCustomPlot *parentPlot) :
  QCPLayoutElement(parentPlot),
  mBackgroundBrush(Qt::NoBrush),
  mBackgroundScaled(true),
  mBackgroundScaledMode(Qt::KeepAspectRatioByExpanding),
  mInsetLayout(new QCPLayoutInset),
  mRangeDrag(false),
  mRangeZoom(false),
  mRangeZoomFactor(0.85),
  // axis base:
  mAngle(-90),
  mAngleRad(mAngle/180.0*M_PI),
  mSelectableParts(spAxis | spTickLabels | spAxisLabel),
  mSelectedParts(spNone),
  mBasePen(QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap)),
  mSelectedBasePen(QPen(Qt::blue, 2)),
  // axis label:
  mLabelPadding(0),
  mLabel(),
  mLabelFont(mParentPlot->font()),
  mSelectedLabelFont(QFont(mLabelFont.family(), mLabelFont.pointSize(), QFont::Bold)),
  mLabelColor(Qt::black),
  mSelectedLabelColor(Qt::blue),
  // tick labels:
  //mTickLabelPadding(0), in label painter
  mTickLabels(true),
  //mTickLabelRotation(0), in label painter
  mTickLabelFont(mParentPlot->font()),
  mSelectedTickLabelFont(QFont(mTickLabelFont.family(), mTickLabelFont.pointSize(), QFont::Bold)),
  mTickLabelColor(Qt::black),
  mSelectedTickLabelColor(Qt::blue),
  mNumberPrecision(6),
  mNumberFormatChar('g'),
  mNumberBeautifulPowers(true),
  mNumberMultiplyCross(false),
  // ticks and subticks:
  mTicks(true),
  mSubTicks(true),
  mTickLengthIn(5),
  mTickLengthOut(0),
  mSubTickLengthIn(2),
  mSubTickLengthOut(0),
  mTickPen(QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap)),
  mSelectedTickPen(QPen(Qt::blue, 2)),
  mSubTickPen(QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap)),
  mSelectedSubTickPen(QPen(Qt::blue, 2)),
  // scale and range:
  mRange(0, 360),
  mRangeReversed(false),
  // internal members:
  mRadius(1), // non-zero initial value, will be overwritten in ::update() according to inner rect
  mGrid(new QCPPolarGrid(this)),
  mTicker(new QCPAxisTickerFixed),
  mDragging(false),
  mLabelPainter(parentPlot)
{
  // TODO:
  //mInsetLayout->initializeParentPlot(mParentPlot);
  //mInsetLayout->setParentLayerable(this);
  //mInsetLayout->setParent(this);
 
  if (QCPAxisTickerFixed *fixedTicker = mTicker.dynamicCast<QCPAxisTickerFixed>().data())
  {
    fixedTicker->setTickStep(30);
  }
  setAntialiased(true);
  setLayer(mParentPlot->currentLayer()); // it's actually on that layer already, but we want it in front of the grid, so we place it on there again
  
  setTickLabelPadding(5);
  setTickLabelRotation(0);
  setTickLabelMode(lmUpright);
  mLabelPainter.setAnchorReferenceType(QCPLabelPainterPrivate::artNormal);
  mLabelPainter.setAbbreviateDecimalPowers(false);
  mLabelPainter.setCacheSize(24); // so we can cache up to 15-degree intervals, polar angular axis uses a bit larger cache than normal axes
  
  setMinimumSize(50, 50);
  setMinimumMargins(QMargins(30, 30, 30, 30));
  
  addRadialAxis();
  mGrid->setRadialAxis(radialAxis());
}

QCPPolarAxisAngular::~QCPPolarAxisAngular()
{
  delete mGrid; // delete grid here instead of via parent ~QObject for better defined deletion order
  mGrid = 0;
  
  delete mInsetLayout;
  mInsetLayout = 0;
  
  QList<QCPPolarAxisRadial*> radialAxesList = radialAxes();
  for (int i=0; i<radialAxesList.size(); ++i)
    removeRadialAxis(radialAxesList.at(i));
}

QCPPolarAxisAngular::LabelMode QCPPolarAxisAngular::tickLabelMode() const
{
  switch (mLabelPainter.anchorMode())
  {
    case QCPLabelPainterPrivate::amSkewedUpright: return lmUpright;
    case QCPLabelPainterPrivate::amSkewedRotated: return lmRotated;
    default: qDebug() << Q_FUNC_INFO << "invalid mode for polar axis"; break;
  }
  return lmUpright;
}

/* No documentation as it is a property getter */
QString QCPPolarAxisAngular::numberFormat() const
{
  QString result;
  result.append(mNumberFormatChar);
  if (mNumberBeautifulPowers)
  {
    result.append(QLatin1Char('b'));
    if (mLabelPainter.multiplicationSymbol() == QCPLabelPainterPrivate::SymbolCross)
      result.append(QLatin1Char('c'));
  }
  return result;
}

/*!
  Returns the number of axes on the axis rect side specified with \a type.
  
  \see axis
*/
int QCPPolarAxisAngular::radialAxisCount() const
{
  return mRadialAxes.size();
}

/*!
  Returns the axis with the given \a index on the axis rect side specified with \a type.
  
  \see axisCount, axes
*/
QCPPolarAxisRadial *QCPPolarAxisAngular::radialAxis(int index) const
{
  if (index >= 0 && index < mRadialAxes.size())
  {
    return mRadialAxes.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "Axis index out of bounds:" << index;
    return 0;
  }
}

/*!
  Returns all axes on the axis rect sides specified with \a types.
  
  \a types may be a single \ref QCPAxis::AxisType or an <tt>or</tt>-combination, to get the axes of
  multiple sides.
  
  \see axis
*/
QList<QCPPolarAxisRadial*> QCPPolarAxisAngular::radialAxes() const
{
  return mRadialAxes;
}


/*!
  Adds a new axis to the axis rect side specified with \a type, and returns it. If \a axis is 0, a
  new QCPAxis instance is created internally. QCustomPlot owns the returned axis, so if you want to
  remove an axis, use \ref removeAxis instead of deleting it manually.

  You may inject QCPAxis instances (or subclasses of QCPAxis) by setting \a axis to an axis that was
  previously created outside QCustomPlot. It is important to note that QCustomPlot takes ownership
  of the axis, so you may not delete it afterwards. Further, the \a axis must have been created
  with this axis rect as parent and with the same axis type as specified in \a type. If this is not
  the case, a debug output is generated, the axis is not added, and the method returns 0.

  This method can not be used to move \a axis between axis rects. The same \a axis instance must
  not be added multiple times to the same or different axis rects.

  If an axis rect side already contains one or more axes, the lower and upper endings of the new
  axis (\ref QCPAxis::setLowerEnding, \ref QCPAxis::setUpperEnding) are set to \ref
  QCPLineEnding::esHalfBar.

  \see addAxes, setupFullAxesBox
*/
QCPPolarAxisRadial *QCPPolarAxisAngular::addRadialAxis(QCPPolarAxisRadial *axis)
{
  QCPPolarAxisRadial *newAxis = axis;
  if (!newAxis)
  {
    newAxis = new QCPPolarAxisRadial(this);
  } else // user provided existing axis instance, do some sanity checks
  {
    if (newAxis->angularAxis() != this)
    {
      qDebug() << Q_FUNC_INFO << "passed radial axis doesn't have this angular axis as parent angular axis";
      return 0;
    }
    if (radialAxes().contains(newAxis))
    {
      qDebug() << Q_FUNC_INFO << "passed axis is already owned by this angular axis";
      return 0;
    }
  }
  mRadialAxes.append(newAxis);
  return newAxis;
}

/*!
  Removes the specified \a axis from the axis rect and deletes it.
  
  Returns true on success, i.e. if \a axis was a valid axis in this axis rect.
  
  \see addAxis
*/
bool QCPPolarAxisAngular::removeRadialAxis(QCPPolarAxisRadial *radialAxis)
{
  if (mRadialAxes.contains(radialAxis))
  {
    mRadialAxes.removeOne(radialAxis);
    delete radialAxis;
    return true;
  } else
  {
    qDebug() << Q_FUNC_INFO << "Radial axis isn't associated with this angular axis:" << reinterpret_cast<quintptr>(radialAxis);
    return false;
  }
}

QRegion QCPPolarAxisAngular::exactClipRegion() const
{
  return QRegion(mCenter.x()-mRadius, mCenter.y()-mRadius, qRound(2*mRadius), qRound(2*mRadius), QRegion::Ellipse);
}

/*!
  If the scale type (\ref setScaleType) is \ref stLinear, \a diff is added to the lower and upper
  bounds of the range. The range is simply moved by \a diff.
  
  If the scale type is \ref stLogarithmic, the range bounds are multiplied by \a diff. This
  corresponds to an apparent "linear" move in logarithmic scaling by a distance of log(diff).
*/
void QCPPolarAxisAngular::moveRange(double diff)
{
  QCPRange oldRange = mRange;
  mRange.lower += diff;
  mRange.upper += diff;
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Scales the range of this axis by \a factor around the center of the current axis range. For
  example, if \a factor is 2.0, then the axis range will double its size, and the point at the axis
  range center won't have changed its position in the QCustomPlot widget (i.e. coordinates around
  the center will have moved symmetrically closer).

  If you wish to scale around a different coordinate than the current axis range center, use the
  overload \ref scaleRange(double factor, double center).
*/
void QCPPolarAxisAngular::scaleRange(double factor)
{
  scaleRange(factor, range().center());
}

/*! \overload

  Scales the range of this axis by \a factor around the coordinate \a center. For example, if \a
  factor is 2.0, \a center is 1.0, then the axis range will double its size, and the point at
  coordinate 1.0 won't have changed its position in the QCustomPlot widget (i.e. coordinates
  around 1.0 will have moved symmetrically closer to 1.0).

  \see scaleRange(double factor)
*/
void QCPPolarAxisAngular::scaleRange(double factor, double center)
{
  QCPRange oldRange = mRange;
  QCPRange newRange;
  newRange.lower = (mRange.lower-center)*factor + center;
  newRange.upper = (mRange.upper-center)*factor + center;
  if (QCPRange::validRange(newRange))
    mRange = newRange.sanitizedForLinScale();
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Changes the axis range such that all plottables associated with this axis are fully visible in
  that dimension.
  
  \see QCPAbstractPlottable::rescaleAxes, QCustomPlot::rescaleAxes
*/
void QCPPolarAxisAngular::rescale(bool onlyVisiblePlottables)
{
  QCPRange newRange;
  bool haveRange = false;
  for (int i=0; i<mGraphs.size(); ++i)
  {
    if (!mGraphs.at(i)->realVisibility() && onlyVisiblePlottables)
      continue;
    QCPRange range;
    bool currentFoundRange;
    if (mGraphs.at(i)->keyAxis() == this)
      range = mGraphs.at(i)->getKeyRange(currentFoundRange, QCP::sdBoth);
    else
      range = mGraphs.at(i)->getValueRange(currentFoundRange, QCP::sdBoth);
    if (currentFoundRange)
    {
      if (!haveRange)
        newRange = range;
      else
        newRange.expand(range);
      haveRange = true;
    }
  }
  if (haveRange)
  {
    if (!QCPRange::validRange(newRange)) // likely due to range being zero (plottable has only constant data in this axis dimension), shift current range to at least center the plottable
    {
      double center = (newRange.lower+newRange.upper)*0.5; // upper and lower should be equal anyway, but just to make sure, incase validRange returned false for other reason
      newRange.lower = center-mRange.size()/2.0;
      newRange.upper = center+mRange.size()/2.0;
    }
    setRange(newRange);
  }
}

/*!
  Transforms \a value, in pixel coordinates of the QCustomPlot widget, to axis coordinates.
*/
void QCPPolarAxisAngular::pixelToCoord(QPointF pixelPos, double &angleCoord, double &radiusCoord) const
{
  if (!mRadialAxes.isEmpty())
    mRadialAxes.first()->pixelToCoord(pixelPos, angleCoord, radiusCoord);
  else
    qDebug() << Q_FUNC_INFO << "no radial axis configured";
}

/*!
  Transforms \a value, in coordinates of the axis, to pixel coordinates of the QCustomPlot widget.
*/
QPointF QCPPolarAxisAngular::coordToPixel(double angleCoord, double radiusCoord) const
{
  if (!mRadialAxes.isEmpty())
  {
    return mRadialAxes.first()->coordToPixel(angleCoord, radiusCoord);
  } else
  {
    qDebug() << Q_FUNC_INFO << "no radial axis configured";
    return QPointF();
  }
}

/*!
  Returns the part of the axis that is hit by \a pos (in pixels). The return value of this function
  is independent of the user-selectable parts defined with \ref setSelectableParts. Further, this
  function does not change the current selection state of the axis.
  
  If the axis is not visible (\ref setVisible), this function always returns \ref spNone.
  
  \see setSelectedParts, setSelectableParts, QCustomPlot::setInteractions
*/
QCPPolarAxisAngular::SelectablePart QCPPolarAxisAngular::getPartAt(const QPointF &pos) const
{
  Q_UNUSED(pos) // TODO remove later
  
  if (!mVisible)
    return spNone;
  
  /*
    TODO:
  if (mAxisPainter->axisSelectionBox().contains(pos.toPoint()))
    return spAxis;
  else if (mAxisPainter->tickLabelsSelectionBox().contains(pos.toPoint()))
    return spTickLabels;
  else if (mAxisPainter->labelSelectionBox().contains(pos.toPoint()))
    return spAxisLabel;
  else */
    return spNone;
}

/* inherits documentation from base class */
double QCPPolarAxisAngular::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  /*
  if (!mParentPlot) return -1;
  SelectablePart part = getPartAt(pos);
  if ((onlySelectable && !mSelectableParts.testFlag(part)) || part == spNone)
    return -1;
  
  if (details)
    details->setValue(part);
  return mParentPlot->selectionTolerance()*0.99;
  */
  
  Q_UNUSED(details)
  
  if (onlySelectable)
    return -1;
  
  if (QRectF(mOuterRect).contains(pos))
  {
    if (mParentPlot)
      return mParentPlot->selectionTolerance()*0.99;
    else
    {
      qDebug() << Q_FUNC_INFO << "parent plot not defined";
      return -1;
    }
  } else
    return -1;
}

/*!
  This method is called automatically upon replot and doesn't need to be called by users of
  QCPPolarAxisAngular.
  
  Calls the base class implementation to update the margins (see \ref QCPLayoutElement::update),
  and finally passes the \ref rect to the inset layout (\ref insetLayout) and calls its
  QCPInsetLayout::update function.
  
  \seebaseclassmethod
*/
void QCPPolarAxisAngular::update(UpdatePhase phase)
{
  QCPLayoutElement::update(phase);
  
  switch (phase)
  {
    case upPreparation:
    {
      setupTickVectors();
      for (int i=0; i<mRadialAxes.size(); ++i)
        mRadialAxes.at(i)->setupTickVectors();
      break;
    }
    case upLayout:
    {
      mCenter = mRect.center();
      mRadius = 0.5*qMin(qAbs(mRect.width()), qAbs(mRect.height()));
      if (mRadius < 1) mRadius = 1; // prevent cases where radius might become 0 which causes trouble
      for (int i=0; i<mRadialAxes.size(); ++i)
        mRadialAxes.at(i)->updateGeometry(mCenter, mRadius);
      
      mInsetLayout->setOuterRect(rect());
      break;
    }
    default: break;
  }
  
  // pass update call on to inset layout (doesn't happen automatically, because QCPPolarAxis doesn't derive from QCPLayout):
  mInsetLayout->update(phase);
}

/* inherits documentation from base class */
QList<QCPLayoutElement*> QCPPolarAxisAngular::elements(bool recursive) const
{
  QList<QCPLayoutElement*> result;
  if (mInsetLayout)
  {
    result << mInsetLayout;
    if (recursive)
      result << mInsetLayout->elements(recursive);
  }
  return result;
}

bool QCPPolarAxisAngular::removeGraph(QCPPolarGraph *graph)
{
  if (!mGraphs.contains(graph))
  {
    qDebug() << Q_FUNC_INFO << "graph not in list:" << reinterpret_cast<quintptr>(graph);
    return false;
  }
  
  // remove plottable from legend:
  graph->removeFromLegend();
  // remove plottable:
  delete graph;
  mGraphs.removeOne(graph);
  return true;
}

/* inherits documentation from base class */
void QCPPolarAxisAngular::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeAxes);
}

/* inherits documentation from base class */
void QCPPolarAxisAngular::draw(QCPPainter *painter)
{
  drawBackground(painter, mCenter, mRadius);
  
  // draw baseline circle:
  painter->setPen(getBasePen());
  painter->drawEllipse(mCenter, mRadius, mRadius);
  
  // draw subticks:
  if (!mSubTickVector.isEmpty())
  {
    painter->setPen(getSubTickPen());
    for (int i=0; i<mSubTickVector.size(); ++i)
    {
      painter->drawLine(mCenter+mSubTickVectorCosSin.at(i)*(mRadius-mSubTickLengthIn),
                        mCenter+mSubTickVectorCosSin.at(i)*(mRadius+mSubTickLengthOut));
    }
  }
  
  // draw ticks and labels:
  if (!mTickVector.isEmpty())
  {
    mLabelPainter.setAnchorReference(mCenter);
    mLabelPainter.setFont(getTickLabelFont());
    mLabelPainter.setColor(getTickLabelColor());
    const QPen ticksPen = getTickPen();
    painter->setPen(ticksPen);
    for (int i=0; i<mTickVector.size(); ++i)
    {
      const QPointF outerTick = mCenter+mTickVectorCosSin.at(i)*(mRadius+mTickLengthOut);
      painter->drawLine(mCenter+mTickVectorCosSin.at(i)*(mRadius-mTickLengthIn), outerTick);
      // draw tick labels:
      if (!mTickVectorLabels.isEmpty())
      {
        if (i < mTickVectorLabels.count()-1 || (mTickVectorCosSin.at(i)-mTickVectorCosSin.first()).manhattanLength() > 5/180.0*M_PI) // skip last label if it's closer than approx 5 degrees to first
          mLabelPainter.drawTickLabel(painter, outerTick, mTickVectorLabels.at(i));
      }
    }
  }
}

/* inherits documentation from base class */
QCP::Interaction QCPPolarAxisAngular::selectionCategory() const
{
  return QCP::iSelectAxes;
}


/*!
  Sets \a pm as the axis background pixmap. The axis background pixmap will be drawn inside the
  axis rect. Since axis rects place themselves on the "background" layer by default, the axis rect
  backgrounds are usually drawn below everything else.

  For cases where the provided pixmap doesn't have the same size as the axis rect, scaling can be
  enabled with \ref setBackgroundScaled and the scaling mode (i.e. whether and how the aspect ratio
  is preserved) can be set with \ref setBackgroundScaledMode. To set all these options in one call,
  consider using the overloaded version of this function.

  Below the pixmap, the axis rect may be optionally filled with a brush, if specified with \ref
  setBackground(const QBrush &brush).
  
  \see setBackgroundScaled, setBackgroundScaledMode, setBackground(const QBrush &brush)
*/
void QCPPolarAxisAngular::setBackground(const QPixmap &pm)
{
  mBackgroundPixmap = pm;
  mScaledBackgroundPixmap = QPixmap();
}

/*! \overload
  
  Sets \a brush as the background brush. The axis rect background will be filled with this brush.
  Since axis rects place themselves on the "background" layer by default, the axis rect backgrounds
  are usually drawn below everything else.

  The brush will be drawn before (under) any background pixmap, which may be specified with \ref
  setBackground(const QPixmap &pm).

  To disable drawing of a background brush, set \a brush to Qt::NoBrush.
  
  \see setBackground(const QPixmap &pm)
*/
void QCPPolarAxisAngular::setBackground(const QBrush &brush)
{
  mBackgroundBrush = brush;
}

/*! \overload
  
  Allows setting the background pixmap of the axis rect, whether it shall be scaled and how it
  shall be scaled in one call.

  \see setBackground(const QPixmap &pm), setBackgroundScaled, setBackgroundScaledMode
*/
void QCPPolarAxisAngular::setBackground(const QPixmap &pm, bool scaled, Qt::AspectRatioMode mode)
{
  mBackgroundPixmap = pm;
  mScaledBackgroundPixmap = QPixmap();
  mBackgroundScaled = scaled;
  mBackgroundScaledMode = mode;
}

/*!
  Sets whether the axis background pixmap shall be scaled to fit the axis rect or not. If \a scaled
  is set to true, you may control whether and how the aspect ratio of the original pixmap is
  preserved with \ref setBackgroundScaledMode.
  
  Note that the scaled version of the original pixmap is buffered, so there is no performance
  penalty on replots. (Except when the axis rect dimensions are changed continuously.)
  
  \see setBackground, setBackgroundScaledMode
*/
void QCPPolarAxisAngular::setBackgroundScaled(bool scaled)
{
  mBackgroundScaled = scaled;
}

/*!
  If scaling of the axis background pixmap is enabled (\ref setBackgroundScaled), use this function to
  define whether and how the aspect ratio of the original pixmap passed to \ref setBackground is preserved.
  \see setBackground, setBackgroundScaled
*/
void QCPPolarAxisAngular::setBackgroundScaledMode(Qt::AspectRatioMode mode)
{
  mBackgroundScaledMode = mode;
}

void QCPPolarAxisAngular::setRangeDrag(bool enabled)
{
  mRangeDrag = enabled;
}

void QCPPolarAxisAngular::setRangeZoom(bool enabled)
{
  mRangeZoom = enabled;
}

void QCPPolarAxisAngular::setRangeZoomFactor(double factor)
{
  mRangeZoomFactor = factor;
}







/*!
  Sets the range of the axis.
  
  This slot may be connected with the \ref rangeChanged signal of another axis so this axis
  is always synchronized with the other axis range, when it changes.
  
  To invert the direction of an axis, use \ref setRangeReversed.
*/
void QCPPolarAxisAngular::setRange(const QCPRange &range)
{
  if (range.lower == mRange.lower && range.upper == mRange.upper)
    return;
  
  if (!QCPRange::validRange(range)) return;
  QCPRange oldRange = mRange;
  mRange = range.sanitizedForLinScale();
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Sets whether the user can (de-)select the parts in \a selectable by clicking on the QCustomPlot surface.
  (When \ref QCustomPlot::setInteractions contains iSelectAxes.)
  
  However, even when \a selectable is set to a value not allowing the selection of a specific part,
  it is still possible to set the selection of this part manually, by calling \ref setSelectedParts
  directly.
  
  \see SelectablePart, setSelectedParts
*/
void QCPPolarAxisAngular::setSelectableParts(const SelectableParts &selectable)
{
  if (mSelectableParts != selectable)
  {
    mSelectableParts = selectable;
    emit selectableChanged(mSelectableParts);
  }
}

/*!
  Sets the selected state of the respective axis parts described by \ref SelectablePart. When a part
  is selected, it uses a different pen/font.
  
  The entire selection mechanism for axes is handled automatically when \ref
  QCustomPlot::setInteractions contains iSelectAxes. You only need to call this function when you
  wish to change the selection state manually.
  
  This function can change the selection state of a part, independent of the \ref setSelectableParts setting.
  
  emits the \ref selectionChanged signal when \a selected is different from the previous selection state.
  
  \see SelectablePart, setSelectableParts, selectTest, setSelectedBasePen, setSelectedTickPen, setSelectedSubTickPen,
  setSelectedTickLabelFont, setSelectedLabelFont, setSelectedTickLabelColor, setSelectedLabelColor
*/
void QCPPolarAxisAngular::setSelectedParts(const SelectableParts &selected)
{
  if (mSelectedParts != selected)
  {
    mSelectedParts = selected;
    emit selectionChanged(mSelectedParts);
  }
}

/*!
  \overload
  
  Sets the lower and upper bound of the axis range.
  
  To invert the direction of an axis, use \ref setRangeReversed.
  
  There is also a slot to set a range, see \ref setRange(const QCPRange &range).
*/
void QCPPolarAxisAngular::setRange(double lower, double upper)
{
  if (lower == mRange.lower && upper == mRange.upper)
    return;
  
  if (!QCPRange::validRange(lower, upper)) return;
  QCPRange oldRange = mRange;
  mRange.lower = lower;
  mRange.upper = upper;
  mRange = mRange.sanitizedForLinScale();
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  \overload
  
  Sets the range of the axis.
  
  The \a position coordinate indicates together with the \a alignment parameter, where the new
  range will be positioned. \a size defines the size of the new axis range. \a alignment may be
  Qt::AlignLeft, Qt::AlignRight or Qt::AlignCenter. This will cause the left border, right border,
  or center of the range to be aligned with \a position. Any other values of \a alignment will
  default to Qt::AlignCenter.
*/
void QCPPolarAxisAngular::setRange(double position, double size, Qt::AlignmentFlag alignment)
{
  if (alignment == Qt::AlignLeft)
    setRange(position, position+size);
  else if (alignment == Qt::AlignRight)
    setRange(position-size, position);
  else // alignment == Qt::AlignCenter
    setRange(position-size/2.0, position+size/2.0);
}

/*!
  Sets the lower bound of the axis range. The upper bound is not changed.
  \see setRange
*/
void QCPPolarAxisAngular::setRangeLower(double lower)
{
  if (mRange.lower == lower)
    return;
  
  QCPRange oldRange = mRange;
  mRange.lower = lower;
  mRange = mRange.sanitizedForLinScale();
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Sets the upper bound of the axis range. The lower bound is not changed.
  \see setRange
*/
void QCPPolarAxisAngular::setRangeUpper(double upper)
{
  if (mRange.upper == upper)
    return;
  
  QCPRange oldRange = mRange;
  mRange.upper = upper;
  mRange = mRange.sanitizedForLinScale();
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Sets whether the axis range (direction) is displayed reversed. Normally, the values on horizontal
  axes increase left to right, on vertical axes bottom to top. When \a reversed is set to true, the
  direction of increasing values is inverted.

  Note that the range and data interface stays the same for reversed axes, e.g. the \a lower part
  of the \ref setRange interface will still reference the mathematically smaller number than the \a
  upper part.
*/
void QCPPolarAxisAngular::setRangeReversed(bool reversed)
{
  mRangeReversed = reversed;
}

void QCPPolarAxisAngular::setAngle(double degrees)
{
  mAngle = degrees;
  mAngleRad = mAngle/180.0*M_PI;
}

/*!
  The axis ticker is responsible for generating the tick positions and tick labels. See the
  documentation of QCPAxisTicker for details on how to work with axis tickers.
  
  You can change the tick positioning/labeling behaviour of this axis by setting a different
  QCPAxisTicker subclass using this method. If you only wish to modify the currently installed axis
  ticker, access it via \ref ticker.
  
  Since the ticker is stored in the axis as a shared pointer, multiple axes may share the same axis
  ticker simply by passing the same shared pointer to multiple axes.
  
  \see ticker
*/
void QCPPolarAxisAngular::setTicker(QSharedPointer<QCPAxisTicker> ticker)
{
  if (ticker)
    mTicker = ticker;
  else
    qDebug() << Q_FUNC_INFO << "can not set 0 as axis ticker";
  // no need to invalidate margin cache here because produced tick labels are checked for changes in setupTickVector
}

/*!
  Sets whether tick marks are displayed.

  Note that setting \a show to false does not imply that tick labels are invisible, too. To achieve
  that, see \ref setTickLabels.
  
  \see setSubTicks
*/
void QCPPolarAxisAngular::setTicks(bool show)
{
  if (mTicks != show)
  {
    mTicks = show;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets whether tick labels are displayed. Tick labels are the numbers drawn next to tick marks.
*/
void QCPPolarAxisAngular::setTickLabels(bool show)
{
  if (mTickLabels != show)
  {
    mTickLabels = show;
    //mCachedMarginValid = false;
    if (!mTickLabels)
      mTickVectorLabels.clear();
  }
}

/*!
  Sets the distance between the axis base line (including any outward ticks) and the tick labels.
  \see setLabelPadding, setPadding
*/
void QCPPolarAxisAngular::setTickLabelPadding(int padding)
{
  mLabelPainter.setPadding(padding);
}

/*!
  Sets the font of the tick labels.
  
  \see setTickLabels, setTickLabelColor
*/
void QCPPolarAxisAngular::setTickLabelFont(const QFont &font)
{
  mTickLabelFont = font;
}

/*!
  Sets the color of the tick labels.
  
  \see setTickLabels, setTickLabelFont
*/
void QCPPolarAxisAngular::setTickLabelColor(const QColor &color)
{
  mTickLabelColor = color;
}

/*!
  Sets the rotation of the tick labels. If \a degrees is zero, the labels are drawn normally. Else,
  the tick labels are drawn rotated by \a degrees clockwise. The specified angle is bound to values
  from -90 to 90 degrees.
  
  If \a degrees is exactly -90, 0 or 90, the tick labels are centered on the tick coordinate. For
  other angles, the label is drawn with an offset such that it seems to point toward or away from
  the tick mark.
*/
void QCPPolarAxisAngular::setTickLabelRotation(double degrees)
{
  mLabelPainter.setRotation(degrees);
}

void QCPPolarAxisAngular::setTickLabelMode(LabelMode mode)
{
  switch (mode)
  {
    case lmUpright: mLabelPainter.setAnchorMode(QCPLabelPainterPrivate::amSkewedUpright); break;
    case lmRotated: mLabelPainter.setAnchorMode(QCPLabelPainterPrivate::amSkewedRotated); break;
  }
}

/*!
  Sets the number format for the numbers in tick labels. This \a formatCode is an extended version
  of the format code used e.g. by QString::number() and QLocale::toString(). For reference about
  that, see the "Argument Formats" section in the detailed description of the QString class.
  
  \a formatCode is a string of one, two or three characters. The first character is identical to
  the normal format code used by Qt. In short, this means: 'e'/'E' scientific format, 'f' fixed
  format, 'g'/'G' scientific or fixed, whichever is shorter.
  
  The second and third characters are optional and specific to QCustomPlot:\n If the first char was
  'e' or 'g', numbers are/might be displayed in the scientific format, e.g. "5.5e9", which might be
  visually unappealing in a plot. So when the second char of \a formatCode is set to 'b' (for
  "beautiful"), those exponential numbers are formatted in a more natural way, i.e. "5.5
  [multiplication sign] 10 [superscript] 9". By default, the multiplication sign is a centered dot.
  If instead a cross should be shown (as is usual in the USA), the third char of \a formatCode can
  be set to 'c'. The inserted multiplication signs are the UTF-8 characters 215 (0xD7) for the
  cross and 183 (0xB7) for the dot.
  
  Examples for \a formatCode:
  \li \c g normal format code behaviour. If number is small, fixed format is used, if number is large,
  normal scientific format is used
  \li \c gb If number is small, fixed format is used, if number is large, scientific format is used with
  beautifully typeset decimal powers and a dot as multiplication sign
  \li \c ebc All numbers are in scientific format with beautifully typeset decimal power and a cross as
  multiplication sign
  \li \c fb illegal format code, since fixed format doesn't support (or need) beautifully typeset decimal
  powers. Format code will be reduced to 'f'.
  \li \c hello illegal format code, since first char is not 'e', 'E', 'f', 'g' or 'G'. Current format
  code will not be changed.
*/
void QCPPolarAxisAngular::setNumberFormat(const QString &formatCode)
{
  if (formatCode.isEmpty())
  {
    qDebug() << Q_FUNC_INFO << "Passed formatCode is empty";
    return;
  }
  //mCachedMarginValid = false;
  
  // interpret first char as number format char:
  QString allowedFormatChars(QLatin1String("eEfgG"));
  if (allowedFormatChars.contains(formatCode.at(0)))
  {
    mNumberFormatChar = QLatin1Char(formatCode.at(0).toLatin1());
  } else
  {
    qDebug() << Q_FUNC_INFO << "Invalid number format code (first char not in 'eEfgG'):" << formatCode;
    return;
  }
  
  if (formatCode.length() < 2)
  {
    mNumberBeautifulPowers = false;
    mNumberMultiplyCross = false;
  } else
  {
    // interpret second char as indicator for beautiful decimal powers:
    if (formatCode.at(1) == QLatin1Char('b') && (mNumberFormatChar == QLatin1Char('e') || mNumberFormatChar == QLatin1Char('g')))
      mNumberBeautifulPowers = true;
    else
      qDebug() << Q_FUNC_INFO << "Invalid number format code (second char not 'b' or first char neither 'e' nor 'g'):" << formatCode;
    
    if (formatCode.length() < 3)
    {
      mNumberMultiplyCross = false;
    } else
    {
      // interpret third char as indicator for dot or cross multiplication symbol:
      if (formatCode.at(2) == QLatin1Char('c'))
        mNumberMultiplyCross = true;
      else if (formatCode.at(2) == QLatin1Char('d'))
        mNumberMultiplyCross = false;
      else
        qDebug() << Q_FUNC_INFO << "Invalid number format code (third char neither 'c' nor 'd'):" << formatCode;
    }
  }
  mLabelPainter.setSubstituteExponent(mNumberBeautifulPowers);
  mLabelPainter.setMultiplicationSymbol(mNumberMultiplyCross ? QCPLabelPainterPrivate::SymbolCross : QCPLabelPainterPrivate::SymbolDot);
}

/*!
  Sets the precision of the tick label numbers. See QLocale::toString(double i, char f, int prec)
  for details. The effect of precisions are most notably for number Formats starting with 'e', see
  \ref setNumberFormat
*/
void QCPPolarAxisAngular::setNumberPrecision(int precision)
{
  if (mNumberPrecision != precision)
  {
    mNumberPrecision = precision;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the length of the ticks in pixels. \a inside is the length the ticks will reach inside the
  plot and \a outside is the length they will reach outside the plot. If \a outside is greater than
  zero, the tick labels and axis label will increase their distance to the axis accordingly, so
  they won't collide with the ticks.
  
  \see setSubTickLength, setTickLengthIn, setTickLengthOut
*/
void QCPPolarAxisAngular::setTickLength(int inside, int outside)
{
  setTickLengthIn(inside);
  setTickLengthOut(outside);
}

/*!
  Sets the length of the inward ticks in pixels. \a inside is the length the ticks will reach
  inside the plot.
  
  \see setTickLengthOut, setTickLength, setSubTickLength
*/
void QCPPolarAxisAngular::setTickLengthIn(int inside)
{
  if (mTickLengthIn != inside)
  {
    mTickLengthIn = inside;
  }
}

/*!
  Sets the length of the outward ticks in pixels. \a outside is the length the ticks will reach
  outside the plot. If \a outside is greater than zero, the tick labels and axis label will
  increase their distance to the axis accordingly, so they won't collide with the ticks.
  
  \see setTickLengthIn, setTickLength, setSubTickLength
*/
void QCPPolarAxisAngular::setTickLengthOut(int outside)
{
  if (mTickLengthOut != outside)
  {
    mTickLengthOut = outside;
    //mCachedMarginValid = false; // only outside tick length can change margin
  }
}

/*!
  Sets whether sub tick marks are displayed.
  
  Sub ticks are only potentially visible if (major) ticks are also visible (see \ref setTicks)
  
  \see setTicks
*/
void QCPPolarAxisAngular::setSubTicks(bool show)
{
  if (mSubTicks != show)
  {
    mSubTicks = show;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the length of the subticks in pixels. \a inside is the length the subticks will reach inside
  the plot and \a outside is the length they will reach outside the plot. If \a outside is greater
  than zero, the tick labels and axis label will increase their distance to the axis accordingly,
  so they won't collide with the ticks.
  
  \see setTickLength, setSubTickLengthIn, setSubTickLengthOut
*/
void QCPPolarAxisAngular::setSubTickLength(int inside, int outside)
{
  setSubTickLengthIn(inside);
  setSubTickLengthOut(outside);
}

/*!
  Sets the length of the inward subticks in pixels. \a inside is the length the subticks will reach inside
  the plot.
  
  \see setSubTickLengthOut, setSubTickLength, setTickLength
*/
void QCPPolarAxisAngular::setSubTickLengthIn(int inside)
{
  if (mSubTickLengthIn != inside)
  {
    mSubTickLengthIn = inside;
  }
}

/*!
  Sets the length of the outward subticks in pixels. \a outside is the length the subticks will reach
  outside the plot. If \a outside is greater than zero, the tick labels will increase their
  distance to the axis accordingly, so they won't collide with the ticks.
  
  \see setSubTickLengthIn, setSubTickLength, setTickLength
*/
void QCPPolarAxisAngular::setSubTickLengthOut(int outside)
{
  if (mSubTickLengthOut != outside)
  {
    mSubTickLengthOut = outside;
    //mCachedMarginValid = false; // only outside tick length can change margin
  }
}

/*!
  Sets the pen, the axis base line is drawn with.
  
  \see setTickPen, setSubTickPen
*/
void QCPPolarAxisAngular::setBasePen(const QPen &pen)
{
  mBasePen = pen;
}

/*!
  Sets the pen, tick marks will be drawn with.
  
  \see setTickLength, setBasePen
*/
void QCPPolarAxisAngular::setTickPen(const QPen &pen)
{
  mTickPen = pen;
}

/*!
  Sets the pen, subtick marks will be drawn with.
  
  \see setSubTickCount, setSubTickLength, setBasePen
*/
void QCPPolarAxisAngular::setSubTickPen(const QPen &pen)
{
  mSubTickPen = pen;
}

/*!
  Sets the font of the axis label.
  
  \see setLabelColor
*/
void QCPPolarAxisAngular::setLabelFont(const QFont &font)
{
  if (mLabelFont != font)
  {
    mLabelFont = font;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the color of the axis label.
  
  \see setLabelFont
*/
void QCPPolarAxisAngular::setLabelColor(const QColor &color)
{
  mLabelColor = color;
}

/*!
  Sets the text of the axis label that will be shown below/above or next to the axis, depending on
  its orientation. To disable axis labels, pass an empty string as \a str.
*/
void QCPPolarAxisAngular::setLabel(const QString &str)
{
  if (mLabel != str)
  {
    mLabel = str;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the distance between the tick labels and the axis label.
  
  \see setTickLabelPadding, setPadding
*/
void QCPPolarAxisAngular::setLabelPadding(int padding)
{
  if (mLabelPadding != padding)
  {
    mLabelPadding = padding;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the font that is used for tick labels when they are selected.
  
  \see setTickLabelFont, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisAngular::setSelectedTickLabelFont(const QFont &font)
{
  if (font != mSelectedTickLabelFont)
  {
    mSelectedTickLabelFont = font;
    // don't set mCachedMarginValid to false here because margin calculation is always done with non-selected fonts
  }
}

/*!
  Sets the font that is used for the axis label when it is selected.
  
  \see setLabelFont, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisAngular::setSelectedLabelFont(const QFont &font)
{
  mSelectedLabelFont = font;
  // don't set mCachedMarginValid to false here because margin calculation is always done with non-selected fonts
}

/*!
  Sets the color that is used for tick labels when they are selected.
  
  \see setTickLabelColor, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisAngular::setSelectedTickLabelColor(const QColor &color)
{
  if (color != mSelectedTickLabelColor)
  {
    mSelectedTickLabelColor = color;
  }
}

/*!
  Sets the color that is used for the axis label when it is selected.
  
  \see setLabelColor, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisAngular::setSelectedLabelColor(const QColor &color)
{
  mSelectedLabelColor = color;
}

/*!
  Sets the pen that is used to draw the axis base line when selected.
  
  \see setBasePen, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisAngular::setSelectedBasePen(const QPen &pen)
{
  mSelectedBasePen = pen;
}

/*!
  Sets the pen that is used to draw the (major) ticks when selected.
  
  \see setTickPen, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisAngular::setSelectedTickPen(const QPen &pen)
{
  mSelectedTickPen = pen;
}

/*!
  Sets the pen that is used to draw the subticks when selected.
  
  \see setSubTickPen, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisAngular::setSelectedSubTickPen(const QPen &pen)
{
  mSelectedSubTickPen = pen;
}

/*! \internal
  
  Draws the background of this axis rect. It may consist of a background fill (a QBrush) and a
  pixmap.
  
  If a brush was given via \ref setBackground(const QBrush &brush), this function first draws an
  according filling inside the axis rect with the provided \a painter.
  
  Then, if a pixmap was provided via \ref setBackground, this function buffers the scaled version
  depending on \ref setBackgroundScaled and \ref setBackgroundScaledMode and then draws it inside
  the axis rect with the provided \a painter. The scaled version is buffered in
  mScaledBackgroundPixmap to prevent expensive rescaling at every redraw. It is only updated, when
  the axis rect has changed in a way that requires a rescale of the background pixmap (this is
  dependent on the \ref setBackgroundScaledMode), or when a differend axis background pixmap was
  set.
  
  \see setBackground, setBackgroundScaled, setBackgroundScaledMode
*/
void QCPPolarAxisAngular::drawBackground(QCPPainter *painter, const QPointF &center, double radius)
{
  // draw background fill (don't use circular clip, looks bad):
  if (mBackgroundBrush != Qt::NoBrush)
  {
    QPainterPath ellipsePath;
    ellipsePath.addEllipse(center, radius, radius);
    painter->fillPath(ellipsePath, mBackgroundBrush);
  }
  
  // draw background pixmap (on top of fill, if brush specified):
  if (!mBackgroundPixmap.isNull())
  {
    QRegion clipCircle(center.x()-radius, center.y()-radius, qRound(2*radius), qRound(2*radius), QRegion::Ellipse);
    QRegion originalClip = painter->clipRegion();
    painter->setClipRegion(clipCircle);
    if (mBackgroundScaled)
    {
      // check whether mScaledBackground needs to be updated:
      QSize scaledSize(mBackgroundPixmap.size());
      scaledSize.scale(mRect.size(), mBackgroundScaledMode);
      if (mScaledBackgroundPixmap.size() != scaledSize)
        mScaledBackgroundPixmap = mBackgroundPixmap.scaled(mRect.size(), mBackgroundScaledMode, Qt::SmoothTransformation);
      painter->drawPixmap(mRect.topLeft()+QPoint(0, -1), mScaledBackgroundPixmap, QRect(0, 0, mRect.width(), mRect.height()) & mScaledBackgroundPixmap.rect());
    } else
    {
      painter->drawPixmap(mRect.topLeft()+QPoint(0, -1), mBackgroundPixmap, QRect(0, 0, mRect.width(), mRect.height()));
    }
    painter->setClipRegion(originalClip);
  }
}

/*! \internal
  
  Prepares the internal tick vector, sub tick vector and tick label vector. This is done by calling
  QCPAxisTicker::generate on the currently installed ticker.
  
  If a change in the label text/count is detected, the cached axis margin is invalidated to make
  sure the next margin calculation recalculates the label sizes and returns an up-to-date value.
*/
void QCPPolarAxisAngular::setupTickVectors()
{
  if (!mParentPlot) return;
  if ((!mTicks && !mTickLabels && !mGrid->visible()) || mRange.size() <= 0) return;
  
  mSubTickVector.clear(); // since we might not pass it to mTicker->generate(), and we don't want old data in there
  mTicker->generate(mRange, mParentPlot->locale(), mNumberFormatChar, mNumberPrecision, mTickVector, mSubTicks ? &mSubTickVector : 0, mTickLabels ? &mTickVectorLabels : 0);
  
  // fill cos/sin buffers which will be used by draw() and QCPPolarGrid::draw(), so we don't have to calculate it twice:
  mTickVectorCosSin.resize(mTickVector.size());
  for (int i=0; i<mTickVector.size(); ++i)
  {
    const double theta = coordToAngleRad(mTickVector.at(i));
    mTickVectorCosSin[i] = QPointF(qCos(theta), qSin(theta));
  }
  mSubTickVectorCosSin.resize(mSubTickVector.size());
  for (int i=0; i<mSubTickVector.size(); ++i)
  {
    const double theta = coordToAngleRad(mSubTickVector.at(i));
    mSubTickVectorCosSin[i] = QPointF(qCos(theta), qSin(theta));
  }
}

/*! \internal
  
  Returns the pen that is used to draw the axis base line. Depending on the selection state, this
  is either mSelectedBasePen or mBasePen.
*/
QPen QCPPolarAxisAngular::getBasePen() const
{
  return mSelectedParts.testFlag(spAxis) ? mSelectedBasePen : mBasePen;
}

/*! \internal
  
  Returns the pen that is used to draw the (major) ticks. Depending on the selection state, this
  is either mSelectedTickPen or mTickPen.
*/
QPen QCPPolarAxisAngular::getTickPen() const
{
  return mSelectedParts.testFlag(spAxis) ? mSelectedTickPen : mTickPen;
}

/*! \internal
  
  Returns the pen that is used to draw the subticks. Depending on the selection state, this
  is either mSelectedSubTickPen or mSubTickPen.
*/
QPen QCPPolarAxisAngular::getSubTickPen() const
{
  return mSelectedParts.testFlag(spAxis) ? mSelectedSubTickPen : mSubTickPen;
}

/*! \internal
  
  Returns the font that is used to draw the tick labels. Depending on the selection state, this
  is either mSelectedTickLabelFont or mTickLabelFont.
*/
QFont QCPPolarAxisAngular::getTickLabelFont() const
{
  return mSelectedParts.testFlag(spTickLabels) ? mSelectedTickLabelFont : mTickLabelFont;
}

/*! \internal
  
  Returns the font that is used to draw the axis label. Depending on the selection state, this
  is either mSelectedLabelFont or mLabelFont.
*/
QFont QCPPolarAxisAngular::getLabelFont() const
{
  return mSelectedParts.testFlag(spAxisLabel) ? mSelectedLabelFont : mLabelFont;
}

/*! \internal
  
  Returns the color that is used to draw the tick labels. Depending on the selection state, this
  is either mSelectedTickLabelColor or mTickLabelColor.
*/
QColor QCPPolarAxisAngular::getTickLabelColor() const
{
  return mSelectedParts.testFlag(spTickLabels) ? mSelectedTickLabelColor : mTickLabelColor;
}

/*! \internal
  
  Returns the color that is used to draw the axis label. Depending on the selection state, this
  is either mSelectedLabelColor or mLabelColor.
*/
QColor QCPPolarAxisAngular::getLabelColor() const
{
  return mSelectedParts.testFlag(spAxisLabel) ? mSelectedLabelColor : mLabelColor;
}

/*! \internal
  
  Event handler for when a mouse button is pressed on the axis rect. If the left mouse button is
  pressed, the range dragging interaction is initialized (the actual range manipulation happens in
  the \ref mouseMoveEvent).

  The mDragging flag is set to true and some anchor points are set that are needed to determine the
  distance the mouse was dragged in the mouse move/release events later.
  
  \see mouseMoveEvent, mouseReleaseEvent
*/
void QCPPolarAxisAngular::mousePressEvent(QMouseEvent *event, const QVariant &details)
{
  Q_UNUSED(details)
  if (event->buttons() & Qt::LeftButton)
  {
    mDragging = true;
    // initialize antialiasing backup in case we start dragging:
    if (mParentPlot->noAntialiasingOnDrag())
    {
      mAADragBackup = mParentPlot->antialiasedElements();
      mNotAADragBackup = mParentPlot->notAntialiasedElements();
    }
    // Mouse range dragging interaction:
    if (mParentPlot->interactions().testFlag(QCP::iRangeDrag))
    {
      mDragAngularStart = range();
      mDragRadialStart.clear();
      for (int i=0; i<mRadialAxes.size(); ++i)
        mDragRadialStart.append(mRadialAxes.at(i)->range());
    }
  }
}

/*! \internal
  
  Event handler for when the mouse is moved on the axis rect. If range dragging was activated in a
  preceding \ref mousePressEvent, the range is moved accordingly.
  
  \see mousePressEvent, mouseReleaseEvent
*/
void QCPPolarAxisAngular::mouseMoveEvent(QMouseEvent *event, const QPointF &startPos)
{
  Q_UNUSED(startPos)
  bool doReplot = false;
  // Mouse range dragging interaction:
  if (mDragging && mParentPlot->interactions().testFlag(QCP::iRangeDrag))
  {
    if (mRangeDrag)
    {
      doReplot = true;
      double angleCoordStart, radiusCoordStart;
      double angleCoord, radiusCoord;
      pixelToCoord(startPos, angleCoordStart, radiusCoordStart);
      pixelToCoord(event->pos(), angleCoord, radiusCoord);
      double diff = angleCoordStart - angleCoord;
      setRange(mDragAngularStart.lower+diff, mDragAngularStart.upper+diff);
    }
    
    for (int i=0; i<mRadialAxes.size(); ++i)
    {
      QCPPolarAxisRadial *ax = mRadialAxes.at(i);
      if (!ax->rangeDrag())
        continue;
      doReplot = true;
      double angleCoordStart, radiusCoordStart;
      double angleCoord, radiusCoord;
      ax->pixelToCoord(startPos, angleCoordStart, radiusCoordStart);
      ax->pixelToCoord(event->pos(), angleCoord, radiusCoord);
      if (ax->scaleType() == QCPPolarAxisRadial::stLinear)
      {
        double diff = radiusCoordStart - radiusCoord;
        ax->setRange(mDragRadialStart.at(i).lower+diff, mDragRadialStart.at(i).upper+diff);
      } else if (ax->scaleType() == QCPPolarAxisRadial::stLogarithmic)
      {
        if (radiusCoord != 0)
        {
          double diff = radiusCoordStart/radiusCoord;
          ax->setRange(mDragRadialStart.at(i).lower*diff, mDragRadialStart.at(i).upper*diff);
        }
      }
    }
    
    if (doReplot) // if either vertical or horizontal drag was enabled, do a replot
    {
      if (mParentPlot->noAntialiasingOnDrag())
        mParentPlot->setNotAntialiasedElements(QCP::aeAll);
      mParentPlot->replot(QCustomPlot::rpQueuedReplot);
    }
  }
}

/* inherits documentation from base class */
void QCPPolarAxisAngular::mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos)
{
  Q_UNUSED(event)
  Q_UNUSED(startPos)
  mDragging = false;
  if (mParentPlot->noAntialiasingOnDrag())
  {
    mParentPlot->setAntialiasedElements(mAADragBackup);
    mParentPlot->setNotAntialiasedElements(mNotAADragBackup);
  }
}

/*! \internal
  
  Event handler for mouse wheel events. If rangeZoom is Qt::Horizontal, Qt::Vertical or both, the
  ranges of the axes defined as rangeZoomHorzAxis and rangeZoomVertAxis are scaled. The center of
  the scaling operation is the current cursor position inside the axis rect. The scaling factor is
  dependent on the mouse wheel delta (which direction the wheel was rotated) to provide a natural
  zooming feel. The Strength of the zoom can be controlled via \ref setRangeZoomFactor.
  
  Note, that event->delta() is usually +/-120 for single rotation steps. However, if the mouse
  wheel is turned rapidly, many steps may bunch up to one event, so the event->delta() may then be
  multiples of 120. This is taken into account here, by calculating \a wheelSteps and using it as
  exponent of the range zoom factor. This takes care of the wheel direction automatically, by
  inverting the factor, when the wheel step is negative (f^-1 = 1/f).
*/
void QCPPolarAxisAngular::wheelEvent(QWheelEvent *event)
{
  bool doReplot = false;
  // Mouse range zooming interaction:
  if (mParentPlot->interactions().testFlag(QCP::iRangeZoom))
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    const double delta = event->delta();
#else
    const double delta = event->angleDelta().y();
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    const QPointF pos = event->pos();
#else
    const QPointF pos = event->position();
#endif
    const double wheelSteps = delta/120.0; // a single step delta is +/-120 usually
    if (mRangeZoom)
    {
      double angleCoord, radiusCoord;
      pixelToCoord(pos, angleCoord, radiusCoord);
      scaleRange(qPow(mRangeZoomFactor, wheelSteps), angleCoord);
    }

    for (int i=0; i<mRadialAxes.size(); ++i)
    {
      QCPPolarAxisRadial *ax = mRadialAxes.at(i);
      if (!ax->rangeZoom())
        continue;
      doReplot = true;
      double angleCoord, radiusCoord;
      ax->pixelToCoord(pos, angleCoord, radiusCoord);
      ax->scaleRange(qPow(ax->rangeZoomFactor(), wheelSteps), radiusCoord);
    }
  }
  if (doReplot)
    mParentPlot->replot();
}

bool QCPPolarAxisAngular::registerPolarGraph(QCPPolarGraph *graph)
{
  if (mGraphs.contains(graph))
  {
    qDebug() << Q_FUNC_INFO << "plottable already added:" << reinterpret_cast<quintptr>(graph);
    return false;
  }
  if (graph->keyAxis() != this)
  {
    qDebug() << Q_FUNC_INFO << "plottable not created with this as axis:" << reinterpret_cast<quintptr>(graph);
    return false;
  }
  
  mGraphs.append(graph);
  // possibly add plottable to legend:
  if (mParentPlot->autoAddPlottableToLegend())
    graph->addToLegend();
  if (!graph->layer()) // usually the layer is already set in the constructor of the plottable (via QCPLayerable constructor)
    graph->setLayer(mParentPlot->currentLayer());
  return true;
}










