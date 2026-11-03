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

#include "selectionrect.h"

#include "axis/axis.h"
#include "painter.h"
#include "core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPSelectionRect
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPSelectionRect
  \brief Provides rect/rubber-band data selection and range zoom interaction
  
  QCPSelectionRect is used by QCustomPlot when the \ref QCustomPlot::setSelectionRectMode is not
  \ref QCP::srmNone. When the user drags the mouse across the plot, the current selection rect
  instance (\ref QCustomPlot::setSelectionRect) is forwarded these events and makes sure an
  according rect shape is drawn. At the begin, during, and after completion of the interaction, it
  emits the corresponding signals \ref started, \ref changed, \ref canceled, and \ref accepted.
  
  The QCustomPlot instance connects own slots to the current selection rect instance, in order to
  react to an accepted selection rect interaction accordingly.
  
  \ref isActive can be used to check whether the selection rect is currently active. An ongoing
  selection interaction can be cancelled programmatically via calling \ref cancel at any time.
  
  The appearance of the selection rect can be controlled via \ref setPen and \ref setBrush.

  If you wish to provide custom behaviour, e.g. a different visual representation of the selection
  rect (\ref QCPSelectionRect::draw), you can subclass QCPSelectionRect and pass an instance of
  your subclass to \ref QCustomPlot::setSelectionRect.
*/

/* start of documentation of inline functions */

/*! \fn bool QCPSelectionRect::isActive() const
   
  Returns true if there is currently a selection going on, i.e. the user has started dragging a
  selection rect, but hasn't released the mouse button yet.
    
  \see cancel
*/

/* end of documentation of inline functions */
/* start documentation of signals */

/*! \fn void QCPSelectionRect::started(QMouseEvent *event);
   
  This signal is emitted when a selection rect interaction was initiated, i.e. the user just
  started dragging the selection rect with the mouse.
*/

/*! \fn void QCPSelectionRect::changed(const QRect &rect, QMouseEvent *event);
  
  This signal is emitted while the selection rect interaction is ongoing and the \a rect has
  changed its size due to the user moving the mouse.
  
  Note that \a rect may have a negative width or height, if the selection is being dragged to the
  upper or left side of the selection rect origin.
*/

/*! \fn void QCPSelectionRect::canceled(const QRect &rect, QInputEvent *event);
  
  This signal is emitted when the selection interaction was cancelled. Note that \a event is \c
  nullptr if the selection interaction was cancelled programmatically, by a call to \ref cancel.
  
  The user may cancel the selection interaction by pressing the escape key. In this case, \a event
  holds the respective input event.
  
  Note that \a rect may have a negative width or height, if the selection is being dragged to the
  upper or left side of the selection rect origin.
*/

/*! \fn void QCPSelectionRect::accepted(const QRect &rect, QMouseEvent *event);
  
  This signal is emitted when the selection interaction was completed by the user releasing the
  mouse button.
    
  Note that \a rect may have a negative width or height, if the selection is being dragged to the
  upper or left side of the selection rect origin.
*/

/* end documentation of signals */

/*!
  Creates a new QCPSelectionRect instance. To make QCustomPlot use the selection rect instance,
  pass it to \ref QCustomPlot::setSelectionRect. \a parentPlot should be set to the same
  QCustomPlot widget.
*/
QCPSelectionRect::QCPSelectionRect(QCustomPlot *parentPlot) :
  QCPLayerable(parentPlot),
  mPen(QBrush(Qt::gray), 0, Qt::DashLine),
  mBrush(Qt::NoBrush),
  mActive(false)
{
}

QCPSelectionRect::~QCPSelectionRect()
{
  cancel();
}

/*!
  A convenience function which returns the coordinate range of the provided \a axis, that this
  selection rect currently encompasses.
*/
QCPRange QCPSelectionRect::range(const QCPAxis *axis) const
{
  if (axis)
  {
    if (axis->orientation() == Qt::Horizontal)
      return {axis->pixelToCoord(mRect.left()), axis->pixelToCoord(mRect.left()+mRect.width())};
    else
      return {axis->pixelToCoord(mRect.top()+mRect.height()), axis->pixelToCoord(mRect.top())};
  } else
  {
    qDebug() << Q_FUNC_INFO << "called with axis zero";
    return {};
  }
}

/*!
  Sets the pen that will be used to draw the selection rect outline.
  
  \see setBrush
*/
void QCPSelectionRect::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  Sets the brush that will be used to fill the selection rect. By default the selection rect is not
  filled, i.e. \a brush is <tt>Qt::NoBrush</tt>.
  
  \see setPen
*/
void QCPSelectionRect::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  If there is currently a selection interaction going on (\ref isActive), the interaction is
  canceled. The selection rect will emit the \ref canceled signal.
*/
void QCPSelectionRect::cancel()
{
  if (mActive)
  {
    mActive = false;
    emit canceled(mRect, nullptr);
  }
}

/*! \internal
  
  This method is called by QCustomPlot to indicate that a selection rect interaction was initiated.
  The default implementation sets the selection rect to active, initializes the selection rect
  geometry and emits the \ref started signal.
*/
void QCPSelectionRect::startSelection(QMouseEvent *event)
{
  mActive = true;
  mRect = QRect(event->pos(), event->pos());
  emit started(event);
}

/*! \internal
  
  This method is called by QCustomPlot to indicate that an ongoing selection rect interaction needs
  to update its geometry. The default implementation updates the rect and emits the \ref changed
  signal.
*/
void QCPSelectionRect::moveSelection(QMouseEvent *event)
{
  mRect.setBottomRight(event->pos());
  emit changed(mRect, event);
  layer()->replot();
}

/*! \internal
  
  This method is called by QCustomPlot to indicate that an ongoing selection rect interaction has
  finished by the user releasing the mouse button. The default implementation deactivates the
  selection rect and emits the \ref accepted signal.
*/
void QCPSelectionRect::endSelection(QMouseEvent *event)
{
  mRect.setBottomRight(event->pos());
  mActive = false;
  emit accepted(mRect, event);
}

/*! \internal
  
  This method is called by QCustomPlot when a key has been pressed by the user while the selection
  rect interaction is active. The default implementation allows to \ref cancel the interaction by
  hitting the escape key.
*/
void QCPSelectionRect::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape && mActive)
  {
    mActive = false;
    emit canceled(mRect, event);
  }
}

/* inherits documentation from base class */
void QCPSelectionRect::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeOther);
}

/*! \internal
  
  If the selection rect is active (\ref isActive), draws the selection rect defined by \a mRect.
  
  \seebaseclassmethod
*/
void QCPSelectionRect::draw(QCPPainter *painter)
{
  if (mActive)
  {
    painter->setPen(mPen);
    painter->setBrush(mBrush);
    painter->drawRect(mRect);
  }
}

