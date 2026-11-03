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

#include "layoutelement-axisrect.h"

#include "../painter.h"
#include "../core.h"
#include "../plottable.h"
#include "../plottables/plottable-graph.h"
#include "../item.h"
#include "../axis/axis.h"
#include "../axis/axisticker.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAxisRect
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPAxisRect
  \brief Holds multiple axes and arranges them in a rectangular shape.
  
  This class represents an axis rect, a rectangular area that is bounded on all sides with an
  arbitrary number of axes.
  
  Initially QCustomPlot has one axis rect, accessible via QCustomPlot::axisRect(). However, the
  layout system allows to have multiple axis rects, e.g. arranged in a grid layout
  (QCustomPlot::plotLayout).
  
  By default, QCPAxisRect comes with four axes, at bottom, top, left and right. They can be
  accessed via \ref axis by providing the respective axis type (\ref QCPAxis::AxisType) and index.
  If you need all axes in the axis rect, use \ref axes. The top and right axes are set to be
  invisible initially (QCPAxis::setVisible). To add more axes to a side, use \ref addAxis or \ref
  addAxes. To remove an axis, use \ref removeAxis.
  
  The axis rect layerable itself only draws a background pixmap or color, if specified (\ref
  setBackground). It is placed on the "background" layer initially (see \ref QCPLayer for an
  explanation of the QCustomPlot layer system). The axes that are held by the axis rect can be
  placed on other layers, independently of the axis rect.
  
  Every axis rect has a child layout of type \ref QCPLayoutInset. It is accessible via \ref
  insetLayout and can be used to have other layout elements (or even other layouts with multiple
  elements) hovering inside the axis rect.
  
  If an axis rect is clicked and dragged, it processes this by moving certain axis ranges. The
  behaviour can be controlled with \ref setRangeDrag and \ref setRangeDragAxes. If the mouse wheel
  is scrolled while the cursor is on the axis rect, certain axes are scaled. This is controllable
  via \ref setRangeZoom, \ref setRangeZoomAxes and \ref setRangeZoomFactor. These interactions are
  only enabled if \ref QCustomPlot::setInteractions contains \ref QCP::iRangeDrag and \ref
  QCP::iRangeZoom.
  
  \image html AxisRectSpacingOverview.png
  <center>Overview of the spacings and paddings that define the geometry of an axis. The dashed
  line on the far left indicates the viewport/widget border.</center>
*/

/* start documentation of inline functions */

/*! \fn QCPLayoutInset *QCPAxisRect::insetLayout() const
  
  Returns the inset layout of this axis rect. It can be used to place other layout elements (or
  even layouts with multiple other elements) inside/on top of an axis rect.
  
  \see QCPLayoutInset
*/

/*! \fn int QCPAxisRect::left() const
  
  Returns the pixel position of the left border of this axis rect. Margins are not taken into
  account here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPAxisRect::right() const
  
  Returns the pixel position of the right border of this axis rect. Margins are not taken into
  account here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPAxisRect::top() const
  
  Returns the pixel position of the top border of this axis rect. Margins are not taken into
  account here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPAxisRect::bottom() const
  
  Returns the pixel position of the bottom border of this axis rect. Margins are not taken into
  account here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPAxisRect::width() const
  
  Returns the pixel width of this axis rect. Margins are not taken into account here, so the
  returned value is with respect to the inner \ref rect.
*/

/*! \fn int QCPAxisRect::height() const
  
  Returns the pixel height of this axis rect. Margins are not taken into account here, so the
  returned value is with respect to the inner \ref rect.
*/

/*! \fn QSize QCPAxisRect::size() const
  
  Returns the pixel size of this axis rect. Margins are not taken into account here, so the
  returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPAxisRect::topLeft() const
  
  Returns the top left corner of this axis rect in pixels. Margins are not taken into account here,
  so the returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPAxisRect::topRight() const
  
  Returns the top right corner of this axis rect in pixels. Margins are not taken into account
  here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPAxisRect::bottomLeft() const
  
  Returns the bottom left corner of this axis rect in pixels. Margins are not taken into account
  here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPAxisRect::bottomRight() const
  
  Returns the bottom right corner of this axis rect in pixels. Margins are not taken into account
  here, so the returned value is with respect to the inner \ref rect.
*/

/*! \fn QPoint QCPAxisRect::center() const
  
  Returns the center of this axis rect in pixels. Margins are not taken into account here, so the
  returned value is with respect to the inner \ref rect.
*/

/* end documentation of inline functions */

/*!
  Creates a QCPAxisRect instance and sets default values. An axis is added for each of the four
  sides, the top and right axes are set invisible initially.
*/
QCPAxisRect::QCPAxisRect(QCustomPlot *parentPlot, bool setupDefaultAxes) :
  QCPLayoutElement(parentPlot),
  mBackgroundBrush(Qt::NoBrush),
  mBackgroundScaled(true),
  mBackgroundScaledMode(Qt::KeepAspectRatioByExpanding),
  mInsetLayout(new QCPLayoutInset),
  mRangeDrag(Qt::Horizontal|Qt::Vertical),
  mRangeZoom(Qt::Horizontal|Qt::Vertical),
  mRangeZoomFactorHorz(0.85),
  mRangeZoomFactorVert(0.85),
  mDragging(false)
{
  mInsetLayout->initializeParentPlot(mParentPlot);
  mInsetLayout->setParentLayerable(this);
  mInsetLayout->setParent(this);
  
  setMinimumSize(50, 50);
  setMinimumMargins(QMargins(15, 15, 15, 15));
  mAxes.insert(QCPAxis::atLeft, QList<QCPAxis*>());
  mAxes.insert(QCPAxis::atRight, QList<QCPAxis*>());
  mAxes.insert(QCPAxis::atTop, QList<QCPAxis*>());
  mAxes.insert(QCPAxis::atBottom, QList<QCPAxis*>());
  
  if (setupDefaultAxes)
  {
    QCPAxis *xAxis = addAxis(QCPAxis::atBottom);
    QCPAxis *yAxis = addAxis(QCPAxis::atLeft);
    QCPAxis *xAxis2 = addAxis(QCPAxis::atTop);
    QCPAxis *yAxis2 = addAxis(QCPAxis::atRight);
    setRangeDragAxes(xAxis, yAxis);
    setRangeZoomAxes(xAxis, yAxis);
    xAxis2->setVisible(false);
    yAxis2->setVisible(false);
    xAxis->grid()->setVisible(true);
    yAxis->grid()->setVisible(true);
    xAxis2->grid()->setVisible(false);
    yAxis2->grid()->setVisible(false);
    xAxis2->grid()->setZeroLinePen(Qt::NoPen);
    yAxis2->grid()->setZeroLinePen(Qt::NoPen);
    xAxis2->grid()->setVisible(false);
    yAxis2->grid()->setVisible(false);
  }
}

QCPAxisRect::~QCPAxisRect()
{
  delete mInsetLayout;
  mInsetLayout = nullptr;
  
  foreach (QCPAxis *axis, axes())
    removeAxis(axis);
}

/*!
  Returns the number of axes on the axis rect side specified with \a type.
  
  \see axis
*/
int QCPAxisRect::axisCount(QCPAxis::AxisType type) const
{
  return mAxes.value(type).size();
}

/*!
  Returns the axis with the given \a index on the axis rect side specified with \a type.
  
  \see axisCount, axes
*/
QCPAxis *QCPAxisRect::axis(QCPAxis::AxisType type, int index) const
{
  QList<QCPAxis*> ax(mAxes.value(type));
  if (index >= 0 && index < ax.size())
  {
    return ax.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "Axis index out of bounds:" << index;
    return nullptr;
  }
}

/*!
  Returns all axes on the axis rect sides specified with \a types.
  
  \a types may be a single \ref QCPAxis::AxisType or an <tt>or</tt>-combination, to get the axes of
  multiple sides.
  
  \see axis
*/
QList<QCPAxis*> QCPAxisRect::axes(QCPAxis::AxisTypes types) const
{
  QList<QCPAxis*> result;
  if (types.testFlag(QCPAxis::atLeft))
    result << mAxes.value(QCPAxis::atLeft);
  if (types.testFlag(QCPAxis::atRight))
    result << mAxes.value(QCPAxis::atRight);
  if (types.testFlag(QCPAxis::atTop))
    result << mAxes.value(QCPAxis::atTop);
  if (types.testFlag(QCPAxis::atBottom))
    result << mAxes.value(QCPAxis::atBottom);
  return result;
}

/*! \overload
  
  Returns all axes of this axis rect.
*/
QList<QCPAxis*> QCPAxisRect::axes() const
{
  QList<QCPAxis*> result;
  QHashIterator<QCPAxis::AxisType, QList<QCPAxis*> > it(mAxes);
  while (it.hasNext())
  {
    it.next();
    result << it.value();
  }
  return result;
}

/*!
  Adds a new axis to the axis rect side specified with \a type, and returns it. If \a axis is 0, a
  new QCPAxis instance is created internally. QCustomPlot owns the returned axis, so if you want to
  remove an axis, use \ref removeAxis instead of deleting it manually.

  You may inject QCPAxis instances (or subclasses of QCPAxis) by setting \a axis to an axis that was
  previously created outside QCustomPlot. It is important to note that QCustomPlot takes ownership
  of the axis, so you may not delete it afterwards. Further, the \a axis must have been created
  with this axis rect as parent and with the same axis type as specified in \a type. If this is not
  the case, a debug output is generated, the axis is not added, and the method returns \c nullptr.

  This method can not be used to move \a axis between axis rects. The same \a axis instance must
  not be added multiple times to the same or different axis rects.

  If an axis rect side already contains one or more axes, the lower and upper endings of the new
  axis (\ref QCPAxis::setLowerEnding, \ref QCPAxis::setUpperEnding) are set to \ref
  QCPLineEnding::esHalfBar.

  \see addAxes, setupFullAxesBox
*/
QCPAxis *QCPAxisRect::addAxis(QCPAxis::AxisType type, QCPAxis *axis)
{
  QCPAxis *newAxis = axis;
  if (!newAxis)
  {
    newAxis = new QCPAxis(this, type);
  } else // user provided existing axis instance, do some sanity checks
  {
    if (newAxis->axisType() != type)
    {
      qDebug() << Q_FUNC_INFO << "passed axis has different axis type than specified in type parameter";
      return nullptr;
    }
    if (newAxis->axisRect() != this)
    {
      qDebug() << Q_FUNC_INFO << "passed axis doesn't have this axis rect as parent axis rect";
      return nullptr;
    }
    if (axes().contains(newAxis))
    {
      qDebug() << Q_FUNC_INFO << "passed axis is already owned by this axis rect";
      return nullptr;
    }
  }
  if (!mAxes[type].isEmpty()) // multiple axes on one side, add half-bar axis ending to additional axes with offset
  {
    bool invert = (type == QCPAxis::atRight) || (type == QCPAxis::atBottom);
    newAxis->setLowerEnding(QCPLineEnding(QCPLineEnding::esHalfBar, 6, 10, !invert));
    newAxis->setUpperEnding(QCPLineEnding(QCPLineEnding::esHalfBar, 6, 10, invert));
  }
  mAxes[type].append(newAxis);
  
  // reset convenience axis pointers on parent QCustomPlot if they are unset:
  if (mParentPlot && mParentPlot->axisRectCount() > 0 && mParentPlot->axisRect(0) == this)
  {
    switch (type)
    {
      case QCPAxis::atBottom: { if (!mParentPlot->xAxis) mParentPlot->xAxis = newAxis; break; }
      case QCPAxis::atLeft: { if (!mParentPlot->yAxis) mParentPlot->yAxis = newAxis; break; }
      case QCPAxis::atTop: { if (!mParentPlot->xAxis2) mParentPlot->xAxis2 = newAxis; break; }
      case QCPAxis::atRight: { if (!mParentPlot->yAxis2) mParentPlot->yAxis2 = newAxis; break; }
    }
  }
  
  return newAxis;
}

/*!
  Adds a new axis with \ref addAxis to each axis rect side specified in \a types. This may be an
  <tt>or</tt>-combination of QCPAxis::AxisType, so axes can be added to multiple sides at once.
  
  Returns a list of the added axes.
  
  \see addAxis, setupFullAxesBox
*/
QList<QCPAxis*> QCPAxisRect::addAxes(QCPAxis::AxisTypes types)
{
  QList<QCPAxis*> result;
  if (types.testFlag(QCPAxis::atLeft))
    result << addAxis(QCPAxis::atLeft);
  if (types.testFlag(QCPAxis::atRight))
    result << addAxis(QCPAxis::atRight);
  if (types.testFlag(QCPAxis::atTop))
    result << addAxis(QCPAxis::atTop);
  if (types.testFlag(QCPAxis::atBottom))
    result << addAxis(QCPAxis::atBottom);
  return result;
}

/*!
  Removes the specified \a axis from the axis rect and deletes it.
  
  Returns true on success, i.e. if \a axis was a valid axis in this axis rect.
  
  \see addAxis
*/
bool QCPAxisRect::removeAxis(QCPAxis *axis)
{
  // don't access axis->axisType() to provide safety when axis is an invalid pointer, rather go through all axis containers:
  QHashIterator<QCPAxis::AxisType, QList<QCPAxis*> > it(mAxes);
  while (it.hasNext())
  {
    it.next();
    if (it.value().contains(axis))
    {
      if (it.value().first() == axis && it.value().size() > 1) // if removing first axis, transfer axis offset to the new first axis (which at this point is the second axis, if it exists)
        it.value()[1]->setOffset(axis->offset());
      mAxes[it.key()].removeOne(axis);
      if (qobject_cast<QCustomPlot*>(parentPlot())) // make sure this isn't called from QObject dtor when QCustomPlot is already destructed (happens when the axis rect is not in any layout and thus QObject-child of QCustomPlot)
        parentPlot()->axisRemoved(axis);
      delete axis;
      return true;
    }
  }
  qDebug() << Q_FUNC_INFO << "Axis isn't in axis rect:" << reinterpret_cast<quintptr>(axis);
  return false;
}

/*!
  Zooms in (or out) to the passed rectangular region \a pixelRect, given in pixel coordinates.

  All axes of this axis rect will have their range zoomed accordingly. If you only wish to zoom
  specific axes, use the overloaded version of this method.
  
  \see QCustomPlot::setSelectionRectMode
*/
void QCPAxisRect::zoom(const QRectF &pixelRect)
{
  zoom(pixelRect, axes());
}

/*! \overload
  
  Zooms in (or out) to the passed rectangular region \a pixelRect, given in pixel coordinates.
  
  Only the axes passed in \a affectedAxes will have their ranges zoomed accordingly.
  
  \see QCustomPlot::setSelectionRectMode
*/
void QCPAxisRect::zoom(const QRectF &pixelRect, const QList<QCPAxis*> &affectedAxes)
{
  foreach (QCPAxis *axis, affectedAxes)
  {
    if (!axis)
    {
      qDebug() << Q_FUNC_INFO << "a passed axis was zero";
      continue;
    }
    QCPRange pixelRange;
    if (axis->orientation() == Qt::Horizontal)
      pixelRange = QCPRange(pixelRect.left(), pixelRect.right());
    else
      pixelRange = QCPRange(pixelRect.top(), pixelRect.bottom());
    axis->setRange(axis->pixelToCoord(pixelRange.lower), axis->pixelToCoord(pixelRange.upper));
  }
}

/*!
  Convenience function to create an axis on each side that doesn't have any axes yet and set their
  visibility to true. Further, the top/right axes are assigned the following properties of the
  bottom/left axes:

  \li range (\ref QCPAxis::setRange)
  \li range reversed (\ref QCPAxis::setRangeReversed)
  \li scale type (\ref QCPAxis::setScaleType)
  \li tick visibility (\ref QCPAxis::setTicks)
  \li number format (\ref QCPAxis::setNumberFormat)
  \li number precision (\ref QCPAxis::setNumberPrecision)
  \li tick count of ticker (\ref QCPAxisTicker::setTickCount)
  \li tick origin of ticker (\ref QCPAxisTicker::setTickOrigin)

  Tick label visibility (\ref QCPAxis::setTickLabels) of the right and top axes are set to false.

  If \a connectRanges is true, the \ref QCPAxis::rangeChanged "rangeChanged" signals of the bottom
  and left axes are connected to the \ref QCPAxis::setRange slots of the top and right axes.
*/
void QCPAxisRect::setupFullAxesBox(bool connectRanges)
{
  QCPAxis *xAxis, *yAxis, *xAxis2, *yAxis2;
  if (axisCount(QCPAxis::atBottom) == 0)
    xAxis = addAxis(QCPAxis::atBottom);
  else
    xAxis = axis(QCPAxis::atBottom);
  
  if (axisCount(QCPAxis::atLeft) == 0)
    yAxis = addAxis(QCPAxis::atLeft);
  else
    yAxis = axis(QCPAxis::atLeft);
  
  if (axisCount(QCPAxis::atTop) == 0)
    xAxis2 = addAxis(QCPAxis::atTop);
  else
    xAxis2 = axis(QCPAxis::atTop);
  
  if (axisCount(QCPAxis::atRight) == 0)
    yAxis2 = addAxis(QCPAxis::atRight);
  else
    yAxis2 = axis(QCPAxis::atRight);
  
  xAxis->setVisible(true);
  yAxis->setVisible(true);
  xAxis2->setVisible(true);
  yAxis2->setVisible(true);
  xAxis2->setTickLabels(false);
  yAxis2->setTickLabels(false);
  
  xAxis2->setRange(xAxis->range());
  xAxis2->setRangeReversed(xAxis->rangeReversed());
  xAxis2->setScaleType(xAxis->scaleType());
  xAxis2->setTicks(xAxis->ticks());
  xAxis2->setNumberFormat(xAxis->numberFormat());
  xAxis2->setNumberPrecision(xAxis->numberPrecision());
  xAxis2->ticker()->setTickCount(xAxis->ticker()->tickCount());
  xAxis2->ticker()->setTickOrigin(xAxis->ticker()->tickOrigin());
  
  yAxis2->setRange(yAxis->range());
  yAxis2->setRangeReversed(yAxis->rangeReversed());
  yAxis2->setScaleType(yAxis->scaleType());
  yAxis2->setTicks(yAxis->ticks());
  yAxis2->setNumberFormat(yAxis->numberFormat());
  yAxis2->setNumberPrecision(yAxis->numberPrecision());
  yAxis2->ticker()->setTickCount(yAxis->ticker()->tickCount());
  yAxis2->ticker()->setTickOrigin(yAxis->ticker()->tickOrigin());
  
  if (connectRanges)
  {
    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
    connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));
  }
}

/*!
  Returns a list of all the plottables that are associated with this axis rect.
  
  A plottable is considered associated with an axis rect if its key or value axis (or both) is in
  this axis rect.
  
  \see graphs, items
*/
QList<QCPAbstractPlottable*> QCPAxisRect::plottables() const
{
  // Note: don't append all QCPAxis::plottables() into a list, because we might get duplicate entries
  QList<QCPAbstractPlottable*> result;
  foreach (QCPAbstractPlottable *plottable, mParentPlot->mPlottables)
  {
    if (plottable->keyAxis()->axisRect() == this || plottable->valueAxis()->axisRect() == this)
      result.append(plottable);
  }
  return result;
}

/*!
  Returns a list of all the graphs that are associated with this axis rect.
  
  A graph is considered associated with an axis rect if its key or value axis (or both) is in
  this axis rect.
  
  \see plottables, items
*/
QList<QCPGraph*> QCPAxisRect::graphs() const
{
  // Note: don't append all QCPAxis::graphs() into a list, because we might get duplicate entries
  QList<QCPGraph*> result;
  foreach (QCPGraph *graph, mParentPlot->mGraphs)
  {
    if (graph->keyAxis()->axisRect() == this || graph->valueAxis()->axisRect() == this)
      result.append(graph);
  }
  return result;
}

/*!
  Returns a list of all the items that are associated with this axis rect.
  
  An item is considered associated with an axis rect if any of its positions has key or value axis
  set to an axis that is in this axis rect, or if any of its positions has \ref
  QCPItemPosition::setAxisRect set to the axis rect, or if the clip axis rect (\ref
  QCPAbstractItem::setClipAxisRect) is set to this axis rect.
  
  \see plottables, graphs
*/
QList<QCPAbstractItem *> QCPAxisRect::items() const
{
  // Note: don't just append all QCPAxis::items() into a list, because we might get duplicate entries
  //       and miss those items that have this axis rect as clipAxisRect.
  QList<QCPAbstractItem*> result;
  foreach (QCPAbstractItem *item, mParentPlot->mItems)
  {
    if (item->clipAxisRect() == this)
    {
      result.append(item);
      continue;
    }
    foreach (QCPItemPosition *position, item->positions())
    {
      if (position->axisRect() == this ||
          position->keyAxis()->axisRect() == this ||
          position->valueAxis()->axisRect() == this)
      {
        result.append(item);
        break;
      }
    }
  }
  return result;
}

/*!
  This method is called automatically upon replot and doesn't need to be called by users of
  QCPAxisRect.
  
  Calls the base class implementation to update the margins (see \ref QCPLayoutElement::update),
  and finally passes the \ref rect to the inset layout (\ref insetLayout) and calls its
  QCPInsetLayout::update function.
  
  \seebaseclassmethod
*/
void QCPAxisRect::update(UpdatePhase phase)
{
  QCPLayoutElement::update(phase);
  
  switch (phase)
  {
    case upPreparation:
    {
      foreach (QCPAxis *axis, axes())
        axis->setupTickVectors();
      break;
    }
    case upLayout:
    {
      mInsetLayout->setOuterRect(rect());
      break;
    }
    default: break;
  }
  
  // pass update call on to inset layout (doesn't happen automatically, because QCPAxisRect doesn't derive from QCPLayout):
  mInsetLayout->update(phase);
}

/* inherits documentation from base class */
QList<QCPLayoutElement*> QCPAxisRect::elements(bool recursive) const
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

/* inherits documentation from base class */
void QCPAxisRect::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  painter->setAntialiasing(false);
}

/* inherits documentation from base class */
void QCPAxisRect::draw(QCPPainter *painter)
{
  drawBackground(painter);
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
void QCPAxisRect::setBackground(const QPixmap &pm)
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
void QCPAxisRect::setBackground(const QBrush &brush)
{
  mBackgroundBrush = brush;
}

/*! \overload
  
  Allows setting the background pixmap of the axis rect, whether it shall be scaled and how it
  shall be scaled in one call.

  \see setBackground(const QPixmap &pm), setBackgroundScaled, setBackgroundScaledMode
*/
void QCPAxisRect::setBackground(const QPixmap &pm, bool scaled, Qt::AspectRatioMode mode)
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
void QCPAxisRect::setBackgroundScaled(bool scaled)
{
  mBackgroundScaled = scaled;
}

/*!
  If scaling of the axis background pixmap is enabled (\ref setBackgroundScaled), use this function to
  define whether and how the aspect ratio of the original pixmap passed to \ref setBackground is preserved.
  \see setBackground, setBackgroundScaled
*/
void QCPAxisRect::setBackgroundScaledMode(Qt::AspectRatioMode mode)
{
  mBackgroundScaledMode = mode;
}

/*!
  Returns the range drag axis of the \a orientation provided. If multiple axes were set, returns
  the first one (use \ref rangeDragAxes to retrieve a list with all set axes).

  \see setRangeDragAxes
*/
QCPAxis *QCPAxisRect::rangeDragAxis(Qt::Orientation orientation)
{
  if (orientation == Qt::Horizontal)
    return mRangeDragHorzAxis.isEmpty() ? nullptr : mRangeDragHorzAxis.first().data();
  else
    return mRangeDragVertAxis.isEmpty() ? nullptr : mRangeDragVertAxis.first().data();
}

/*!
  Returns the range zoom axis of the \a orientation provided. If multiple axes were set, returns
  the first one (use \ref rangeZoomAxes to retrieve a list with all set axes).

  \see setRangeZoomAxes
*/
QCPAxis *QCPAxisRect::rangeZoomAxis(Qt::Orientation orientation)
{
  if (orientation == Qt::Horizontal)
    return mRangeZoomHorzAxis.isEmpty() ? nullptr : mRangeZoomHorzAxis.first().data();
  else
    return mRangeZoomVertAxis.isEmpty() ? nullptr : mRangeZoomVertAxis.first().data();
}

/*!
  Returns all range drag axes of the \a orientation provided.

  \see rangeZoomAxis, setRangeZoomAxes
*/
QList<QCPAxis*> QCPAxisRect::rangeDragAxes(Qt::Orientation orientation)
{
  QList<QCPAxis*> result;
  if (orientation == Qt::Horizontal)
  {
    foreach (QPointer<QCPAxis> axis, mRangeDragHorzAxis)
    {
      if (!axis.isNull())
        result.append(axis.data());
    }
  } else
  {
    foreach (QPointer<QCPAxis> axis, mRangeDragVertAxis)
    {
      if (!axis.isNull())
        result.append(axis.data());
    }
  }
  return result;
}

/*!
  Returns all range zoom axes of the \a orientation provided.

  \see rangeDragAxis, setRangeDragAxes
*/
QList<QCPAxis*> QCPAxisRect::rangeZoomAxes(Qt::Orientation orientation)
{
  QList<QCPAxis*> result;
  if (orientation == Qt::Horizontal)
  {
    foreach (QPointer<QCPAxis> axis, mRangeZoomHorzAxis)
    {
      if (!axis.isNull())
        result.append(axis.data());
    }
  } else
  {
    foreach (QPointer<QCPAxis> axis, mRangeZoomVertAxis)
    {
      if (!axis.isNull())
        result.append(axis.data());
    }
  }
  return result;
}

/*!
  Returns the range zoom factor of the \a orientation provided.
  
  \see setRangeZoomFactor
*/
double QCPAxisRect::rangeZoomFactor(Qt::Orientation orientation)
{
  return (orientation == Qt::Horizontal ? mRangeZoomFactorHorz : mRangeZoomFactorVert);
}

/*!
  Sets which axis orientation may be range dragged by the user with mouse interaction.
  What orientation corresponds to which specific axis can be set with
  \ref setRangeDragAxes(QCPAxis *horizontal, QCPAxis *vertical). By
  default, the horizontal axis is the bottom axis (xAxis) and the vertical axis
  is the left axis (yAxis).
  
  To disable range dragging entirely, pass \c nullptr as \a orientations or remove \ref
  QCP::iRangeDrag from \ref QCustomPlot::setInteractions. To enable range dragging for both
  directions, pass <tt>Qt::Horizontal | Qt::Vertical</tt> as \a orientations.
  
  In addition to setting \a orientations to a non-zero value, make sure \ref QCustomPlot::setInteractions
  contains \ref QCP::iRangeDrag to enable the range dragging interaction.
  
  \see setRangeZoom, setRangeDragAxes, QCustomPlot::setNoAntialiasingOnDrag
*/
void QCPAxisRect::setRangeDrag(Qt::Orientations orientations)
{
  mRangeDrag = orientations;
}

/*!
  Sets which axis orientation may be zoomed by the user with the mouse wheel. What orientation
  corresponds to which specific axis can be set with \ref setRangeZoomAxes(QCPAxis *horizontal,
  QCPAxis *vertical). By default, the horizontal axis is the bottom axis (xAxis) and the vertical
  axis is the left axis (yAxis).

  To disable range zooming entirely, pass \c nullptr as \a orientations or remove \ref
  QCP::iRangeZoom from \ref QCustomPlot::setInteractions. To enable range zooming for both
  directions, pass <tt>Qt::Horizontal | Qt::Vertical</tt> as \a orientations.
  
  In addition to setting \a orientations to a non-zero value, make sure \ref QCustomPlot::setInteractions
  contains \ref QCP::iRangeZoom to enable the range zooming interaction.
  
  \see setRangeZoomFactor, setRangeZoomAxes, setRangeDrag
*/
void QCPAxisRect::setRangeZoom(Qt::Orientations orientations)
{
  mRangeZoom = orientations;
}

/*! \overload
  
  Sets the axes whose range will be dragged when \ref setRangeDrag enables mouse range dragging on
  the QCustomPlot widget. Pass \c nullptr if no axis shall be dragged in the respective
  orientation.

  Use the overload taking a list of axes, if multiple axes (more than one per orientation) shall
  react to dragging interactions.

  \see setRangeZoomAxes
*/
void QCPAxisRect::setRangeDragAxes(QCPAxis *horizontal, QCPAxis *vertical)
{
  QList<QCPAxis*> horz, vert;
  if (horizontal)
    horz.append(horizontal);
  if (vertical)
    vert.append(vertical);
  setRangeDragAxes(horz, vert);
}

/*! \overload

  This method allows to set up multiple axes to react to horizontal and vertical dragging. The drag
  orientation that the respective axis will react to is deduced from its orientation (\ref
  QCPAxis::orientation).

  In the unusual case that you wish to e.g. drag a vertically oriented axis with a horizontal drag
  motion, use the overload taking two separate lists for horizontal and vertical dragging.
*/
void QCPAxisRect::setRangeDragAxes(QList<QCPAxis*> axes)
{
  QList<QCPAxis*> horz, vert;
  foreach (QCPAxis *ax, axes)
  {
    if (ax->orientation() == Qt::Horizontal)
      horz.append(ax);
    else
      vert.append(ax);
  }
  setRangeDragAxes(horz, vert);
}

/*! \overload

  This method allows to set multiple axes up to react to horizontal and vertical dragging, and
  define specifically which axis reacts to which drag orientation (irrespective of the axis
  orientation).
*/
void QCPAxisRect::setRangeDragAxes(QList<QCPAxis*> horizontal, QList<QCPAxis*> vertical)
{
  mRangeDragHorzAxis.clear();
  foreach (QCPAxis *ax, horizontal)
  {
    QPointer<QCPAxis> axPointer(ax);
    if (!axPointer.isNull())
      mRangeDragHorzAxis.append(axPointer);
    else
      qDebug() << Q_FUNC_INFO << "invalid axis passed in horizontal list:" << reinterpret_cast<quintptr>(ax);
  }
  mRangeDragVertAxis.clear();
  foreach (QCPAxis *ax, vertical)
  {
    QPointer<QCPAxis> axPointer(ax);
    if (!axPointer.isNull())
      mRangeDragVertAxis.append(axPointer);
    else
      qDebug() << Q_FUNC_INFO << "invalid axis passed in vertical list:" << reinterpret_cast<quintptr>(ax);
  }
}

/*!
  Sets the axes whose range will be zoomed when \ref setRangeZoom enables mouse wheel zooming on
  the QCustomPlot widget. Pass \c nullptr if no axis shall be zoomed in the respective orientation.

  The two axes can be zoomed with different strengths, when different factors are passed to \ref
  setRangeZoomFactor(double horizontalFactor, double verticalFactor).

  Use the overload taking a list of axes, if multiple axes (more than one per orientation) shall
  react to zooming interactions.

  \see setRangeDragAxes
*/
void QCPAxisRect::setRangeZoomAxes(QCPAxis *horizontal, QCPAxis *vertical)
{
  QList<QCPAxis*> horz, vert;
  if (horizontal)
    horz.append(horizontal);
  if (vertical)
    vert.append(vertical);
  setRangeZoomAxes(horz, vert);
}

/*! \overload

  This method allows to set up multiple axes to react to horizontal and vertical range zooming. The
  zoom orientation that the respective axis will react to is deduced from its orientation (\ref
  QCPAxis::orientation).

  In the unusual case that you wish to e.g. zoom a vertically oriented axis with a horizontal zoom
  interaction, use the overload taking two separate lists for horizontal and vertical zooming.
*/
void QCPAxisRect::setRangeZoomAxes(QList<QCPAxis*> axes)
{
  QList<QCPAxis*> horz, vert;
  foreach (QCPAxis *ax, axes)
  {
    if (ax->orientation() == Qt::Horizontal)
      horz.append(ax);
    else
      vert.append(ax);
  }
  setRangeZoomAxes(horz, vert);
}

/*! \overload

  This method allows to set multiple axes up to react to horizontal and vertical zooming, and
  define specifically which axis reacts to which zoom orientation (irrespective of the axis
  orientation).
*/
void QCPAxisRect::setRangeZoomAxes(QList<QCPAxis*> horizontal, QList<QCPAxis*> vertical)
{
  mRangeZoomHorzAxis.clear();
  foreach (QCPAxis *ax, horizontal)
  {
    QPointer<QCPAxis> axPointer(ax);
    if (!axPointer.isNull())
      mRangeZoomHorzAxis.append(axPointer);
    else
      qDebug() << Q_FUNC_INFO << "invalid axis passed in horizontal list:" << reinterpret_cast<quintptr>(ax);
  }
  mRangeZoomVertAxis.clear();
  foreach (QCPAxis *ax, vertical)
  {
    QPointer<QCPAxis> axPointer(ax);
    if (!axPointer.isNull())
      mRangeZoomVertAxis.append(axPointer);
    else
      qDebug() << Q_FUNC_INFO << "invalid axis passed in vertical list:" << reinterpret_cast<quintptr>(ax);
  }
}

/*!
  Sets how strong one rotation step of the mouse wheel zooms, when range zoom was activated with
  \ref setRangeZoom. The two parameters \a horizontalFactor and \a verticalFactor provide a way to
  let the horizontal axis zoom at different rates than the vertical axis. Which axis is horizontal
  and which is vertical, can be set with \ref setRangeZoomAxes.

  When the zoom factor is greater than one, scrolling the mouse wheel backwards (towards the user)
  will zoom in (make the currently visible range smaller). For zoom factors smaller than one, the
  same scrolling direction will zoom out.
*/
void QCPAxisRect::setRangeZoomFactor(double horizontalFactor, double verticalFactor)
{
  mRangeZoomFactorHorz = horizontalFactor;
  mRangeZoomFactorVert = verticalFactor;
}

/*! \overload
  
  Sets both the horizontal and vertical zoom \a factor.
*/
void QCPAxisRect::setRangeZoomFactor(double factor)
{
  mRangeZoomFactorHorz = factor;
  mRangeZoomFactorVert = factor;
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
void QCPAxisRect::drawBackground(QCPPainter *painter)
{
  // draw background fill:
  if (mBackgroundBrush != Qt::NoBrush)
    painter->fillRect(mRect, mBackgroundBrush);
  
  // draw background pixmap (on top of fill, if brush specified):
  if (!mBackgroundPixmap.isNull())
  {
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
  }
}

/*! \internal
  
  This function makes sure multiple axes on the side specified with \a type don't collide, but are
  distributed according to their respective space requirement (QCPAxis::calculateMargin).
  
  It does this by setting an appropriate offset (\ref QCPAxis::setOffset) on all axes except the
  one with index zero.
  
  This function is called by \ref calculateAutoMargin.
*/
void QCPAxisRect::updateAxesOffset(QCPAxis::AxisType type)
{
  const QList<QCPAxis*> axesList = mAxes.value(type);
  if (axesList.isEmpty())
    return;
  
  bool isFirstVisible = !axesList.first()->visible(); // if the first axis is visible, the second axis (which is where the loop starts) isn't the first visible axis, so initialize with false
  for (int i=1; i<axesList.size(); ++i)
  {
    int offset = axesList.at(i-1)->offset() + axesList.at(i-1)->calculateMargin();
    if (axesList.at(i)->visible()) // only add inner tick length to offset if this axis is visible and it's not the first visible one (might happen if true first axis is invisible)
    {
      if (!isFirstVisible)
        offset += axesList.at(i)->tickLengthIn();
      isFirstVisible = false;
    }
    axesList.at(i)->setOffset(offset);
  }
}

/* inherits documentation from base class */
int QCPAxisRect::calculateAutoMargin(QCP::MarginSide side)
{
  if (!mAutoMargins.testFlag(side))
    qDebug() << Q_FUNC_INFO << "Called with side that isn't specified as auto margin";
  
  updateAxesOffset(QCPAxis::marginSideToAxisType(side));
  
  // note: only need to look at the last (outer most) axis to determine the total margin, due to updateAxisOffset call
  const QList<QCPAxis*> axesList = mAxes.value(QCPAxis::marginSideToAxisType(side));
  if (!axesList.isEmpty())
    return axesList.last()->offset() + axesList.last()->calculateMargin();
  else
    return 0;
}

/*! \internal
  
  Reacts to a change in layout to potentially set the convenience axis pointers \ref
  QCustomPlot::xAxis, \ref QCustomPlot::yAxis, etc. of the parent QCustomPlot to the respective
  axes of this axis rect. This is only done if the respective convenience pointer is currently zero
  and if there is no QCPAxisRect at position (0, 0) of the plot layout.
  
  This automation makes it simpler to replace the main axis rect with a newly created one, without
  the need to manually reset the convenience pointers.
*/
void QCPAxisRect::layoutChanged()
{
  if (mParentPlot && mParentPlot->axisRectCount() > 0 && mParentPlot->axisRect(0) == this)
  {
    if (axisCount(QCPAxis::atBottom) > 0 && !mParentPlot->xAxis)
      mParentPlot->xAxis = axis(QCPAxis::atBottom);
    if (axisCount(QCPAxis::atLeft) > 0 && !mParentPlot->yAxis)
      mParentPlot->yAxis = axis(QCPAxis::atLeft);
    if (axisCount(QCPAxis::atTop) > 0 && !mParentPlot->xAxis2)
      mParentPlot->xAxis2 = axis(QCPAxis::atTop);
    if (axisCount(QCPAxis::atRight) > 0 && !mParentPlot->yAxis2)
      mParentPlot->yAxis2 = axis(QCPAxis::atRight);
  }
}

/*! \internal
  
  Event handler for when a mouse button is pressed on the axis rect. If the left mouse button is
  pressed, the range dragging interaction is initialized (the actual range manipulation happens in
  the \ref mouseMoveEvent).

  The mDragging flag is set to true and some anchor points are set that are needed to determine the
  distance the mouse was dragged in the mouse move/release events later.
  
  \see mouseMoveEvent, mouseReleaseEvent
*/
void QCPAxisRect::mousePressEvent(QMouseEvent *event, const QVariant &details)
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
      mDragStartHorzRange.clear();
      foreach (QPointer<QCPAxis> axis, mRangeDragHorzAxis)
        mDragStartHorzRange.append(axis.isNull() ? QCPRange() : axis->range());
      mDragStartVertRange.clear();
      foreach (QPointer<QCPAxis> axis, mRangeDragVertAxis)
        mDragStartVertRange.append(axis.isNull() ? QCPRange() : axis->range());
    }
  }
}

/*! \internal
  
  Event handler for when the mouse is moved on the axis rect. If range dragging was activated in a
  preceding \ref mousePressEvent, the range is moved accordingly.
  
  \see mousePressEvent, mouseReleaseEvent
*/
void QCPAxisRect::mouseMoveEvent(QMouseEvent *event, const QPointF &startPos)
{
  Q_UNUSED(startPos)
  // Mouse range dragging interaction:
  if (mDragging && mParentPlot->interactions().testFlag(QCP::iRangeDrag))
  {
    
    if (mRangeDrag.testFlag(Qt::Horizontal))
    {
      for (int i=0; i<mRangeDragHorzAxis.size(); ++i)
      {
        QCPAxis *ax = mRangeDragHorzAxis.at(i).data();
        if (!ax)
          continue;
        if (i >= mDragStartHorzRange.size())
          break;
        if (ax->mScaleType == QCPAxis::stLinear)
        {
          double diff = ax->pixelToCoord(startPos.x()) - ax->pixelToCoord(event->pos().x());
          ax->setRange(mDragStartHorzRange.at(i).lower+diff, mDragStartHorzRange.at(i).upper+diff);
        } else if (ax->mScaleType == QCPAxis::stLogarithmic)
        {
          double diff = ax->pixelToCoord(startPos.x()) / ax->pixelToCoord(event->pos().x());
          ax->setRange(mDragStartHorzRange.at(i).lower*diff, mDragStartHorzRange.at(i).upper*diff);
        }
      }
    }
    
    if (mRangeDrag.testFlag(Qt::Vertical))
    {
      for (int i=0; i<mRangeDragVertAxis.size(); ++i)
      {
        QCPAxis *ax = mRangeDragVertAxis.at(i).data();
        if (!ax)
          continue;
        if (i >= mDragStartVertRange.size())
          break;
        if (ax->mScaleType == QCPAxis::stLinear)
        {
          double diff = ax->pixelToCoord(startPos.y()) - ax->pixelToCoord(event->pos().y());
          ax->setRange(mDragStartVertRange.at(i).lower+diff, mDragStartVertRange.at(i).upper+diff);
        } else if (ax->mScaleType == QCPAxis::stLogarithmic)
        {
          double diff = ax->pixelToCoord(startPos.y()) / ax->pixelToCoord(event->pos().y());
          ax->setRange(mDragStartVertRange.at(i).lower*diff, mDragStartVertRange.at(i).upper*diff);
        }
      }
    }
    
    if (mRangeDrag != 0) // if either vertical or horizontal drag was enabled, do a replot
    {
      if (mParentPlot->noAntialiasingOnDrag())
        mParentPlot->setNotAntialiasedElements(QCP::aeAll);
      mParentPlot->replot(QCustomPlot::rpQueuedReplot);
    }
    
  }
}

/* inherits documentation from base class */
void QCPAxisRect::mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos)
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
  
  Note, that event->angleDelta() is usually +/-120 for single rotation steps. However, if the mouse
  wheel is turned rapidly, many steps may bunch up to one event, so the delta may then be multiples
  of 120. This is taken into account here, by calculating \a wheelSteps and using it as exponent of
  the range zoom factor. This takes care of the wheel direction automatically, by inverting the
  factor, when the wheel step is negative (f^-1 = 1/f).
*/
void QCPAxisRect::wheelEvent(QWheelEvent *event)
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
  
  // Mouse range zooming interaction:
  if (mParentPlot->interactions().testFlag(QCP::iRangeZoom))
  {
    if (mRangeZoom != 0)
    {
      double factor;
      double wheelSteps = delta/120.0; // a single step delta is +/-120 usually
      if (mRangeZoom.testFlag(Qt::Horizontal))
      {
        factor = qPow(mRangeZoomFactorHorz, wheelSteps);
        foreach (QPointer<QCPAxis> axis, mRangeZoomHorzAxis)
        {
          if (!axis.isNull())
            axis->scaleRange(factor, axis->pixelToCoord(pos.x()));
        }
      }
      if (mRangeZoom.testFlag(Qt::Vertical))
      {
        factor = qPow(mRangeZoomFactorVert, wheelSteps);
        foreach (QPointer<QCPAxis> axis, mRangeZoomVertAxis)
        {
          if (!axis.isNull())
            axis->scaleRange(factor, axis->pixelToCoord(pos.y()));
        }
      }
      mParentPlot->replot();
    }
  }
}










