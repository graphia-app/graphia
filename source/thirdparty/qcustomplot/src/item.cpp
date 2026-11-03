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

#include "item.h"

#include "painter.h"
#include "core.h"
#include "axis/axis.h"
#include "layoutelements/layoutelement-axisrect.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPItemAnchor
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPItemAnchor
  \brief An anchor of an item to which positions can be attached to.
  
  An item (QCPAbstractItem) may have one or more anchors. Unlike QCPItemPosition, an anchor doesn't
  control anything on its item, but provides a way to tie other items via their positions to the
  anchor.

  For example, a QCPItemRect is defined by its positions \a topLeft and \a bottomRight.
  Additionally it has various anchors like \a top, \a topRight or \a bottomLeft etc. So you can
  attach the \a start (which is a QCPItemPosition) of a QCPItemLine to one of the anchors by
  calling QCPItemPosition::setParentAnchor on \a start, passing the wanted anchor of the
  QCPItemRect. This way the start of the line will now always follow the respective anchor location
  on the rect item.
  
  Note that QCPItemPosition derives from QCPItemAnchor, so every position can also serve as an
  anchor to other positions.
  
  To learn how to provide anchors in your own item subclasses, see the subclassing section of the
  QCPAbstractItem documentation.
*/

/* start documentation of inline functions */

/*! \fn virtual QCPItemPosition *QCPItemAnchor::toQCPItemPosition()
  
  Returns \c nullptr if this instance is merely a QCPItemAnchor, and a valid pointer of type
  QCPItemPosition* if it actually is a QCPItemPosition (which is a subclass of QCPItemAnchor).
  
  This safe downcast functionality could also be achieved with a dynamic_cast. However, QCustomPlot avoids
  dynamic_cast to work with projects that don't have RTTI support enabled (e.g. -fno-rtti flag with
  gcc compiler).
*/

/* end documentation of inline functions */

/*!
  Creates a new QCPItemAnchor. You shouldn't create QCPItemAnchor instances directly, even if
  you want to make a new item subclass. Use \ref QCPAbstractItem::createAnchor instead, as
  explained in the subclassing section of the QCPAbstractItem documentation.
*/
QCPItemAnchor::QCPItemAnchor(QCustomPlot *parentPlot, QCPAbstractItem *parentItem, const QString &name, int anchorId) :
  mName(name),
  mParentPlot(parentPlot),
  mParentItem(parentItem),
  mAnchorId(anchorId)
{
}

QCPItemAnchor::~QCPItemAnchor()
{
  // unregister as parent at children:
  foreach (QCPItemPosition *child, mChildrenX.values())
  {
    if (child->parentAnchorX() == this)
      child->setParentAnchorX(nullptr); // this acts back on this anchor and child removes itself from mChildrenX
  }
  foreach (QCPItemPosition *child, mChildrenY.values())
  {
    if (child->parentAnchorY() == this)
      child->setParentAnchorY(nullptr); // this acts back on this anchor and child removes itself from mChildrenY
  }
}

/*!
  Returns the final absolute pixel position of the QCPItemAnchor on the QCustomPlot surface.
  
  The pixel information is internally retrieved via QCPAbstractItem::anchorPixelPosition of the
  parent item, QCPItemAnchor is just an intermediary.
*/
QPointF QCPItemAnchor::pixelPosition() const
{
  if (mParentItem)
  {
    if (mAnchorId > -1)
    {
      return mParentItem->anchorPixelPosition(mAnchorId);
    } else
    {
      qDebug() << Q_FUNC_INFO << "no valid anchor id set:" << mAnchorId;
      return {};
    }
  } else
  {
    qDebug() << Q_FUNC_INFO << "no parent item set";
    return {};
  }
}

/*! \internal

  Adds \a pos to the childX list of this anchor, which keeps track of which children use this
  anchor as parent anchor for the respective coordinate. This is necessary to notify the children
  prior to destruction of the anchor.
  
  Note that this function does not change the parent setting in \a pos.
*/
void QCPItemAnchor::addChildX(QCPItemPosition *pos)
{
  if (!mChildrenX.contains(pos))
    mChildrenX.insert(pos);
  else
    qDebug() << Q_FUNC_INFO << "provided pos is child already" << reinterpret_cast<quintptr>(pos);
}

/*! \internal

  Removes \a pos from the childX list of this anchor.
  
  Note that this function does not change the parent setting in \a pos.
*/
void QCPItemAnchor::removeChildX(QCPItemPosition *pos)
{
  if (!mChildrenX.remove(pos))
    qDebug() << Q_FUNC_INFO << "provided pos isn't child" << reinterpret_cast<quintptr>(pos);
}

/*! \internal

  Adds \a pos to the childY list of this anchor, which keeps track of which children use this
  anchor as parent anchor for the respective coordinate. This is necessary to notify the children
  prior to destruction of the anchor.
  
  Note that this function does not change the parent setting in \a pos.
*/
void QCPItemAnchor::addChildY(QCPItemPosition *pos)
{
  if (!mChildrenY.contains(pos))
    mChildrenY.insert(pos);
  else
    qDebug() << Q_FUNC_INFO << "provided pos is child already" << reinterpret_cast<quintptr>(pos);
}

/*! \internal

  Removes \a pos from the childY list of this anchor.
  
  Note that this function does not change the parent setting in \a pos.
*/
void QCPItemAnchor::removeChildY(QCPItemPosition *pos)
{
  if (!mChildrenY.remove(pos))
    qDebug() << Q_FUNC_INFO << "provided pos isn't child" << reinterpret_cast<quintptr>(pos);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPItemPosition
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPItemPosition
  \brief Manages the position of an item.
  
  Every item has at least one public QCPItemPosition member pointer which provides ways to position the
  item on the QCustomPlot surface. Some items have multiple positions, for example QCPItemRect has two:
  \a topLeft and \a bottomRight.

  QCPItemPosition has a type (\ref PositionType) that can be set with \ref setType. This type
  defines how coordinates passed to \ref setCoords are to be interpreted, e.g. as absolute pixel
  coordinates, as plot coordinates of certain axes (\ref QCPItemPosition::setAxes), as fractions of
  the axis rect (\ref QCPItemPosition::setAxisRect), etc. For more advanced plots it is also
  possible to assign different types per X/Y coordinate of the position (see \ref setTypeX, \ref
  setTypeY). This way an item could be positioned for example at a fixed pixel distance from the
  top in the Y direction, while following a plot coordinate in the X direction.

  A QCPItemPosition may have a parent QCPItemAnchor, see \ref setParentAnchor. This way you can tie
  multiple items together. If the QCPItemPosition has a parent, its coordinates (\ref setCoords)
  are considered to be absolute pixels in the reference frame of the parent anchor, where (0, 0)
  means directly ontop of the parent anchor. For example, You could attach the \a start position of
  a QCPItemLine to the \a bottom anchor of a QCPItemText to make the starting point of the line
  always be centered under the text label, no matter where the text is moved to. For more advanced
  plots, it is possible to assign different parent anchors per X/Y coordinate of the position, see
  \ref setParentAnchorX, \ref setParentAnchorY. This way an item could follow another item in the X
  direction but stay at a fixed position in the Y direction. Or even follow item A in X, and item B
  in Y.

  Note that every QCPItemPosition inherits from QCPItemAnchor and thus can itself be used as parent
  anchor for other positions.

  To set the apparent pixel position on the QCustomPlot surface directly, use \ref setPixelPosition. This
  works no matter what type this QCPItemPosition is or what parent-child situation it is in, as \ref
  setPixelPosition transforms the coordinates appropriately, to make the position appear at the specified
  pixel values.
*/

/* start documentation of inline functions */

/*! \fn QCPItemPosition::PositionType *QCPItemPosition::type() const
  
  Returns the current position type.
  
  If different types were set for X and Y (\ref setTypeX, \ref setTypeY), this method returns the
  type of the X coordinate. In that case rather use \a typeX() and \a typeY().
  
  \see setType
*/

/*! \fn QCPItemAnchor *QCPItemPosition::parentAnchor() const
  
  Returns the current parent anchor.
  
  If different parent anchors were set for X and Y (\ref setParentAnchorX, \ref setParentAnchorY),
  this method returns the parent anchor of the Y coordinate. In that case rather use \a
  parentAnchorX() and \a parentAnchorY().
  
  \see setParentAnchor
*/

/* end documentation of inline functions */

/*!
  Creates a new QCPItemPosition. You shouldn't create QCPItemPosition instances directly, even if
  you want to make a new item subclass. Use \ref QCPAbstractItem::createPosition instead, as
  explained in the subclassing section of the QCPAbstractItem documentation.
*/
QCPItemPosition::QCPItemPosition(QCustomPlot *parentPlot, QCPAbstractItem *parentItem, const QString &name) :
  QCPItemAnchor(parentPlot, parentItem, name),
  mPositionTypeX(ptAbsolute),
  mPositionTypeY(ptAbsolute),
  mKey(0),
  mValue(0),
  mParentAnchorX(nullptr),
  mParentAnchorY(nullptr)
{
}

QCPItemPosition::~QCPItemPosition()
{
  // unregister as parent at children:
  // Note: this is done in ~QCPItemAnchor again, but it's important QCPItemPosition does it itself, because only then
  //       the setParentAnchor(0) call the correct QCPItemPosition::pixelPosition function instead of QCPItemAnchor::pixelPosition
  foreach (QCPItemPosition *child, mChildrenX.values())
  {
    if (child->parentAnchorX() == this)
      child->setParentAnchorX(nullptr); // this acts back on this anchor and child removes itself from mChildrenX
  }
  foreach (QCPItemPosition *child, mChildrenY.values())
  {
    if (child->parentAnchorY() == this)
      child->setParentAnchorY(nullptr); // this acts back on this anchor and child removes itself from mChildrenY
  }
  // unregister as child in parent:
  if (mParentAnchorX)
    mParentAnchorX->removeChildX(this);
  if (mParentAnchorY)
    mParentAnchorY->removeChildY(this);
}

/* can't make this a header inline function, because QPointer breaks with forward declared types, see QTBUG-29588 */
QCPAxisRect *QCPItemPosition::axisRect() const
{
  return mAxisRect.data();
}

/*!
  Sets the type of the position. The type defines how the coordinates passed to \ref setCoords
  should be handled and how the QCPItemPosition should behave in the plot.
  
  The possible values for \a type can be separated in two main categories:

  \li The position is regarded as a point in plot coordinates. This corresponds to \ref ptPlotCoords
  and requires two axes that define the plot coordinate system. They can be specified with \ref setAxes.
  By default, the QCustomPlot's x- and yAxis are used.
  
  \li The position is fixed on the QCustomPlot surface, i.e. independent of axis ranges. This
  corresponds to all other types, i.e. \ref ptAbsolute, \ref ptViewportRatio and \ref
  ptAxisRectRatio. They differ only in the way the absolute position is described, see the
  documentation of \ref PositionType for details. For \ref ptAxisRectRatio, note that you can specify
  the axis rect with \ref setAxisRect. By default this is set to the main axis rect.
  
  Note that the position type \ref ptPlotCoords is only available (and sensible) when the position
  has no parent anchor (\ref setParentAnchor).
  
  If the type is changed, the apparent pixel position on the plot is preserved. This means
  the coordinates as retrieved with coords() and set with \ref setCoords may change in the process.
  
  This method sets the type for both X and Y directions. It is also possible to set different types
  for X and Y, see \ref setTypeX, \ref setTypeY.
*/
void QCPItemPosition::setType(QCPItemPosition::PositionType type)
{
  setTypeX(type);
  setTypeY(type);
}

/*!
  This method sets the position type of the X coordinate to \a type.
  
  For a detailed description of what a position type is, see the documentation of \ref setType.
  
  \see setType, setTypeY
*/
void QCPItemPosition::setTypeX(QCPItemPosition::PositionType type)
{
  if (mPositionTypeX != type)
  {
    // if switching from or to coordinate type that isn't valid (e.g. because axes or axis rect
    // were deleted), don't try to recover the pixelPosition() because it would output a qDebug warning.
    bool retainPixelPosition = true;
    if ((mPositionTypeX == ptPlotCoords || type == ptPlotCoords) && (!mKeyAxis || !mValueAxis))
      retainPixelPosition = false;
    if ((mPositionTypeX == ptAxisRectRatio || type == ptAxisRectRatio) && (!mAxisRect))
      retainPixelPosition = false;
    
    QPointF pixel;
    if (retainPixelPosition)
      pixel = pixelPosition();
    
    mPositionTypeX = type;
    
    if (retainPixelPosition)
      setPixelPosition(pixel);
  }
}

/*!
  This method sets the position type of the Y coordinate to \a type.
  
  For a detailed description of what a position type is, see the documentation of \ref setType.
  
  \see setType, setTypeX
*/
void QCPItemPosition::setTypeY(QCPItemPosition::PositionType type)
{
  if (mPositionTypeY != type)
  {
    // if switching from or to coordinate type that isn't valid (e.g. because axes or axis rect
    // were deleted), don't try to recover the pixelPosition() because it would output a qDebug warning.
    bool retainPixelPosition = true;
    if ((mPositionTypeY == ptPlotCoords || type == ptPlotCoords) && (!mKeyAxis || !mValueAxis))
      retainPixelPosition = false;
    if ((mPositionTypeY == ptAxisRectRatio || type == ptAxisRectRatio) && (!mAxisRect))
      retainPixelPosition = false;
    
    QPointF pixel;
    if (retainPixelPosition)
      pixel = pixelPosition();
    
    mPositionTypeY = type;
    
    if (retainPixelPosition)
      setPixelPosition(pixel);
  }
}

/*!
  Sets the parent of this QCPItemPosition to \a parentAnchor. This means the position will now
  follow any position changes of the anchor. The local coordinate system of positions with a parent
  anchor always is absolute pixels, with (0, 0) being exactly on top of the parent anchor. (Hence
  the type shouldn't be set to \ref ptPlotCoords for positions with parent anchors.)
  
  if \a keepPixelPosition is true, the current pixel position of the QCPItemPosition is preserved
  during reparenting. If it's set to false, the coordinates are set to (0, 0), i.e. the position
  will be exactly on top of the parent anchor.
  
  To remove this QCPItemPosition from any parent anchor, set \a parentAnchor to \c nullptr.
  
  If the QCPItemPosition previously had no parent and the type is \ref ptPlotCoords, the type is
  set to \ref ptAbsolute, to keep the position in a valid state.
  
  This method sets the parent anchor for both X and Y directions. It is also possible to set
  different parents for X and Y, see \ref setParentAnchorX, \ref setParentAnchorY.
*/
bool QCPItemPosition::setParentAnchor(QCPItemAnchor *parentAnchor, bool keepPixelPosition)
{
  bool successX = setParentAnchorX(parentAnchor, keepPixelPosition);
  bool successY = setParentAnchorY(parentAnchor, keepPixelPosition);
  return successX && successY;
}

/*!
  This method sets the parent anchor of the X coordinate to \a parentAnchor.
  
  For a detailed description of what a parent anchor is, see the documentation of \ref setParentAnchor.
  
  \see setParentAnchor, setParentAnchorY
*/
bool QCPItemPosition::setParentAnchorX(QCPItemAnchor *parentAnchor, bool keepPixelPosition)
{
  // make sure self is not assigned as parent:
  if (parentAnchor == this)
  {
    qDebug() << Q_FUNC_INFO << "can't set self as parent anchor" << reinterpret_cast<quintptr>(parentAnchor);
    return false;
  }
  // make sure no recursive parent-child-relationships are created:
  QCPItemAnchor *currentParent = parentAnchor;
  while (currentParent)
  {
    if (QCPItemPosition *currentParentPos = currentParent->toQCPItemPosition())
    {
      // is a QCPItemPosition, might have further parent, so keep iterating
      if (currentParentPos == this)
      {
        qDebug() << Q_FUNC_INFO << "can't create recursive parent-child-relationship" << reinterpret_cast<quintptr>(parentAnchor);
        return false;
      }
      currentParent = currentParentPos->parentAnchorX();
    } else
    {
      // is a QCPItemAnchor, can't have further parent. Now make sure the parent items aren't the
      // same, to prevent a position being child of an anchor which itself depends on the position,
      // because they're both on the same item:
      if (currentParent->mParentItem == mParentItem)
      {
        qDebug() << Q_FUNC_INFO << "can't set parent to be an anchor which itself depends on this position" << reinterpret_cast<quintptr>(parentAnchor);
        return false;
      }
      break;
    }
  }
  
  // if previously no parent set and PosType is still ptPlotCoords, set to ptAbsolute:
  if (!mParentAnchorX && mPositionTypeX == ptPlotCoords)
    setTypeX(ptAbsolute);
  
  // save pixel position:
  QPointF pixelP;
  if (keepPixelPosition)
    pixelP = pixelPosition();
  // unregister at current parent anchor:
  if (mParentAnchorX)
    mParentAnchorX->removeChildX(this);
  // register at new parent anchor:
  if (parentAnchor)
    parentAnchor->addChildX(this);
  mParentAnchorX = parentAnchor;
  // restore pixel position under new parent:
  if (keepPixelPosition)
    setPixelPosition(pixelP);
  else
    setCoords(0, coords().y());
  return true;
}

/*!
  This method sets the parent anchor of the Y coordinate to \a parentAnchor.
  
  For a detailed description of what a parent anchor is, see the documentation of \ref setParentAnchor.
  
  \see setParentAnchor, setParentAnchorX
*/
bool QCPItemPosition::setParentAnchorY(QCPItemAnchor *parentAnchor, bool keepPixelPosition)
{
  // make sure self is not assigned as parent:
  if (parentAnchor == this)
  {
    qDebug() << Q_FUNC_INFO << "can't set self as parent anchor" << reinterpret_cast<quintptr>(parentAnchor);
    return false;
  }
  // make sure no recursive parent-child-relationships are created:
  QCPItemAnchor *currentParent = parentAnchor;
  while (currentParent)
  {
    if (QCPItemPosition *currentParentPos = currentParent->toQCPItemPosition())
    {
      // is a QCPItemPosition, might have further parent, so keep iterating
      if (currentParentPos == this)
      {
        qDebug() << Q_FUNC_INFO << "can't create recursive parent-child-relationship" << reinterpret_cast<quintptr>(parentAnchor);
        return false;
      }
      currentParent = currentParentPos->parentAnchorY();
    } else
    {
      // is a QCPItemAnchor, can't have further parent. Now make sure the parent items aren't the
      // same, to prevent a position being child of an anchor which itself depends on the position,
      // because they're both on the same item:
      if (currentParent->mParentItem == mParentItem)
      {
        qDebug() << Q_FUNC_INFO << "can't set parent to be an anchor which itself depends on this position" << reinterpret_cast<quintptr>(parentAnchor);
        return false;
      }
      break;
    }
  }
  
  // if previously no parent set and PosType is still ptPlotCoords, set to ptAbsolute:
  if (!mParentAnchorY && mPositionTypeY == ptPlotCoords)
    setTypeY(ptAbsolute);
  
  // save pixel position:
  QPointF pixelP;
  if (keepPixelPosition)
    pixelP = pixelPosition();
  // unregister at current parent anchor:
  if (mParentAnchorY)
    mParentAnchorY->removeChildY(this);
  // register at new parent anchor:
  if (parentAnchor)
    parentAnchor->addChildY(this);
  mParentAnchorY = parentAnchor;
  // restore pixel position under new parent:
  if (keepPixelPosition)
    setPixelPosition(pixelP);
  else
    setCoords(coords().x(), 0);
  return true;
}

/*!
  Sets the coordinates of this QCPItemPosition. What the coordinates mean, is defined by the type
  (\ref setType, \ref setTypeX, \ref setTypeY).
  
  For example, if the type is \ref ptAbsolute, \a key and \a value mean the x and y pixel position
  on the QCustomPlot surface. In that case the origin (0, 0) is in the top left corner of the
  QCustomPlot viewport. If the type is \ref ptPlotCoords, \a key and \a value mean a point in the
  plot coordinate system defined by the axes set by \ref setAxes. By default those are the
  QCustomPlot's xAxis and yAxis. See the documentation of \ref setType for other available
  coordinate types and their meaning.
  
  If different types were configured for X and Y (\ref setTypeX, \ref setTypeY), \a key and \a
  value must also be provided in the different coordinate systems. Here, the X type refers to \a
  key, and the Y type refers to \a value.

  \see setPixelPosition
*/
void QCPItemPosition::setCoords(double key, double value)
{
  mKey = key;
  mValue = value;
}

/*! \overload

  Sets the coordinates as a QPointF \a pos where pos.x has the meaning of \a key and pos.y the
  meaning of \a value of the \ref setCoords(double key, double value) method.
*/
void QCPItemPosition::setCoords(const QPointF &pos)
{
  setCoords(pos.x(), pos.y());
}

/*!
  Returns the final absolute pixel position of the QCPItemPosition on the QCustomPlot surface. It
  includes all effects of type (\ref setType) and possible parent anchors (\ref setParentAnchor).

  \see setPixelPosition
*/
QPointF QCPItemPosition::pixelPosition() const
{
  QPointF result;
  
  // determine X:
  switch (mPositionTypeX)
  {
    case ptAbsolute:
    {
      result.rx() = mKey;
      if (mParentAnchorX)
        result.rx() += mParentAnchorX->pixelPosition().x();
      break;
    }
    case ptViewportRatio:
    {
      result.rx() = mKey*mParentPlot->viewport().width();
      if (mParentAnchorX)
        result.rx() += mParentAnchorX->pixelPosition().x();
      else
        result.rx() += mParentPlot->viewport().left();
      break;
    }
    case ptAxisRectRatio:
    {
      if (mAxisRect)
      {
        result.rx() = mKey*mAxisRect.data()->width();
        if (mParentAnchorX)
          result.rx() += mParentAnchorX->pixelPosition().x();
        else
          result.rx() += mAxisRect.data()->left();
      } else
        qDebug() << Q_FUNC_INFO << "Item position type x is ptAxisRectRatio, but no axis rect was defined";
      break;
    }
    case ptPlotCoords:
    {
      if (mKeyAxis && mKeyAxis.data()->orientation() == Qt::Horizontal)
        result.rx() = mKeyAxis.data()->coordToPixel(mKey);
      else if (mValueAxis && mValueAxis.data()->orientation() == Qt::Horizontal)
        result.rx() = mValueAxis.data()->coordToPixel(mValue);
      else
        qDebug() << Q_FUNC_INFO << "Item position type x is ptPlotCoords, but no axes were defined";
      break;
    }
  }
  
  // determine Y:
  switch (mPositionTypeY)
  {
    case ptAbsolute:
    {
      result.ry() = mValue;
      if (mParentAnchorY)
        result.ry() += mParentAnchorY->pixelPosition().y();
      break;
    }
    case ptViewportRatio:
    {
      result.ry() = mValue*mParentPlot->viewport().height();
      if (mParentAnchorY)
        result.ry() += mParentAnchorY->pixelPosition().y();
      else
        result.ry() += mParentPlot->viewport().top();
      break;
    }
    case ptAxisRectRatio:
    {
      if (mAxisRect)
      {
        result.ry() = mValue*mAxisRect.data()->height();
        if (mParentAnchorY)
          result.ry() += mParentAnchorY->pixelPosition().y();
        else
          result.ry() += mAxisRect.data()->top();
      } else
        qDebug() << Q_FUNC_INFO << "Item position type y is ptAxisRectRatio, but no axis rect was defined";
      break;
    }
    case ptPlotCoords:
    {
      if (mKeyAxis && mKeyAxis.data()->orientation() == Qt::Vertical)
        result.ry() = mKeyAxis.data()->coordToPixel(mKey);
      else if (mValueAxis && mValueAxis.data()->orientation() == Qt::Vertical)
        result.ry() = mValueAxis.data()->coordToPixel(mValue);
      else
        qDebug() << Q_FUNC_INFO << "Item position type y is ptPlotCoords, but no axes were defined";
      break;
    }
  }
  
  return result;
}

/*!
  When \ref setType is \ref ptPlotCoords, this function may be used to specify the axes the
  coordinates set with \ref setCoords relate to. By default they are set to the initial xAxis and
  yAxis of the QCustomPlot.
*/
void QCPItemPosition::setAxes(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
  mKeyAxis = keyAxis;
  mValueAxis = valueAxis;
}

/*!
  When \ref setType is \ref ptAxisRectRatio, this function may be used to specify the axis rect the
  coordinates set with \ref setCoords relate to. By default this is set to the main axis rect of
  the QCustomPlot.
*/
void QCPItemPosition::setAxisRect(QCPAxisRect *axisRect)
{
  mAxisRect = axisRect;
}

/*!
  Sets the apparent pixel position. This works no matter what type (\ref setType) this
  QCPItemPosition is or what parent-child situation it is in, as coordinates are transformed
  appropriately, to make the position finally appear at the specified pixel values.

  Only if the type is \ref ptAbsolute and no parent anchor is set, this function's effect is
  identical to that of \ref setCoords.

  \see pixelPosition, setCoords
*/
void QCPItemPosition::setPixelPosition(const QPointF &pixelPosition)
{
  double x = pixelPosition.x();
  double y = pixelPosition.y();
  
  switch (mPositionTypeX)
  {
    case ptAbsolute:
    {
      if (mParentAnchorX)
        x -= mParentAnchorX->pixelPosition().x();
      break;
    }
    case ptViewportRatio:
    {
      if (mParentAnchorX)
        x -= mParentAnchorX->pixelPosition().x();
      else
        x -= mParentPlot->viewport().left();
      x /= double(mParentPlot->viewport().width());
      break;
    }
    case ptAxisRectRatio:
    {
      if (mAxisRect)
      {
        if (mParentAnchorX)
          x -= mParentAnchorX->pixelPosition().x();
        else
          x -= mAxisRect.data()->left();
        x /= double(mAxisRect.data()->width());
      } else
        qDebug() << Q_FUNC_INFO << "Item position type x is ptAxisRectRatio, but no axis rect was defined";
      break;
    }
    case ptPlotCoords:
    {
      if (mKeyAxis && mKeyAxis.data()->orientation() == Qt::Horizontal)
        x = mKeyAxis.data()->pixelToCoord(x);
      else if (mValueAxis && mValueAxis.data()->orientation() == Qt::Horizontal)
        y = mValueAxis.data()->pixelToCoord(x);
      else
        qDebug() << Q_FUNC_INFO << "Item position type x is ptPlotCoords, but no axes were defined";
      break;
    }
  }
  
  switch (mPositionTypeY)
  {
    case ptAbsolute:
    {
      if (mParentAnchorY)
        y -= mParentAnchorY->pixelPosition().y();
      break;
    }
    case ptViewportRatio:
    {
      if (mParentAnchorY)
        y -= mParentAnchorY->pixelPosition().y();
      else
        y -= mParentPlot->viewport().top();
      y /= double(mParentPlot->viewport().height());
      break;
    }
    case ptAxisRectRatio:
    {
      if (mAxisRect)
      {
        if (mParentAnchorY)
          y -= mParentAnchorY->pixelPosition().y();
        else
          y -= mAxisRect.data()->top();
        y /= double(mAxisRect.data()->height());
      } else
        qDebug() << Q_FUNC_INFO << "Item position type y is ptAxisRectRatio, but no axis rect was defined";
      break;
    }
    case ptPlotCoords:
    {
      if (mKeyAxis && mKeyAxis.data()->orientation() == Qt::Vertical)
        x = mKeyAxis.data()->pixelToCoord(y);
      else if (mValueAxis && mValueAxis.data()->orientation() == Qt::Vertical)
        y = mValueAxis.data()->pixelToCoord(y);
      else
        qDebug() << Q_FUNC_INFO << "Item position type y is ptPlotCoords, but no axes were defined";
      break;
    }
  }
  
  setCoords(x, y);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAbstractItem
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPAbstractItem
  \brief The abstract base class for all items in a plot.
  
  In QCustomPlot, items are supplemental graphical elements that are neither plottables
  (QCPAbstractPlottable) nor axes (QCPAxis). While plottables are always tied to two axes and thus
  plot coordinates, items can also be placed in absolute coordinates independent of any axes. Each
  specific item has at least one QCPItemPosition member which controls the positioning. Some items
  are defined by more than one coordinate and thus have two or more QCPItemPosition members (For
  example, QCPItemRect has \a topLeft and \a bottomRight).
  
  This abstract base class defines a very basic interface like visibility and clipping. Since this
  class is abstract, it can't be instantiated. Use one of the subclasses or create a subclass
  yourself to create new items.
  
  The built-in items are:
  <table>
  <tr><td>QCPItemLine</td><td>A line defined by a start and an end point. May have different ending styles on each side (e.g. arrows).</td></tr>
  <tr><td>QCPItemStraightLine</td><td>A straight line defined by a start and a direction point. Unlike QCPItemLine, the straight line is infinitely long and has no endings.</td></tr>
  <tr><td>QCPItemCurve</td><td>A curve defined by start, end and two intermediate control points. May have different ending styles on each side (e.g. arrows).</td></tr>
  <tr><td>QCPItemRect</td><td>A rectangle</td></tr>
  <tr><td>QCPItemEllipse</td><td>An ellipse</td></tr>
  <tr><td>QCPItemPixmap</td><td>An arbitrary pixmap</td></tr>
  <tr><td>QCPItemText</td><td>A text label</td></tr>
  <tr><td>QCPItemBracket</td><td>A bracket which may be used to reference/highlight certain parts in the plot.</td></tr>
  <tr><td>QCPItemTracer</td><td>An item that can be attached to a QCPGraph and sticks to its data points, given a key coordinate.</td></tr>
  </table>
  
  \section items-clipping Clipping

  Items are by default clipped to the main axis rect (they are only visible inside the axis rect).
  To make an item visible outside that axis rect, disable clipping via \ref setClipToAxisRect
  "setClipToAxisRect(false)".

  On the other hand if you want the item to be clipped to a different axis rect, specify it via
  \ref setClipAxisRect. This clipAxisRect property of an item is only used for clipping behaviour, and
  in principle is independent of the coordinate axes the item might be tied to via its position
  members (\ref QCPItemPosition::setAxes). However, it is common that the axis rect for clipping
  also contains the axes used for the item positions.
  
  \section items-using Using items
  
  First you instantiate the item you want to use and add it to the plot:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpitemline-creation-1
  by default, the positions of the item are bound to the x- and y-Axis of the plot. So we can just
  set the plot coordinates where the line should start/end:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpitemline-creation-2
  If we don't want the line to be positioned in plot coordinates but a different coordinate system,
  e.g. absolute pixel positions on the QCustomPlot surface, we need to change the position type like this:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpitemline-creation-3
  Then we can set the coordinates, this time in pixels:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpitemline-creation-4
  and make the line visible on the entire QCustomPlot, by disabling clipping to the axis rect:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpitemline-creation-5
  
  For more advanced plots, it is even possible to set different types and parent anchors per X/Y
  coordinate of an item position, using for example \ref QCPItemPosition::setTypeX or \ref
  QCPItemPosition::setParentAnchorX. For details, see the documentation of \ref QCPItemPosition.
  
  \section items-subclassing Creating own items
  
  To create an own item, you implement a subclass of QCPAbstractItem. These are the pure
  virtual functions, you must implement:
  \li \ref selectTest
  \li \ref draw
  
  See the documentation of those functions for what they need to do.
  
  \subsection items-positioning Allowing the item to be positioned
  
  As mentioned, item positions are represented by QCPItemPosition members. Let's assume the new item shall
  have only one point as its position (as opposed to two like a rect or multiple like a polygon). You then add
  a public member of type QCPItemPosition like so:
  
  \code QCPItemPosition * const myPosition;\endcode
  
  the const makes sure the pointer itself can't be modified from the user of your new item (the QCPItemPosition
  instance it points to, can be modified, of course).
  The initialization of this pointer is made easy with the \ref createPosition function. Just assign
  the return value of this function to each QCPItemPosition in the constructor of your item. \ref createPosition
  takes a string which is the name of the position, typically this is identical to the variable name.
  For example, the constructor of QCPItemExample could look like this:
  
  \code
  QCPItemExample::QCPItemExample(QCustomPlot *parentPlot) :
    QCPAbstractItem(parentPlot),
    myPosition(createPosition("myPosition"))
  {
    // other constructor code
  }
  \endcode
  
  \subsection items-drawing The draw function
  
  To give your item a visual representation, reimplement the \ref draw function and use the passed
  QCPPainter to draw the item. You can retrieve the item position in pixel coordinates from the
  position member(s) via \ref QCPItemPosition::pixelPosition.

  To optimize performance you should calculate a bounding rect first (don't forget to take the pen
  width into account), check whether it intersects the \ref clipRect, and only draw the item at all
  if this is the case.
  
  \subsection items-selection The selectTest function
  
  Your implementation of the \ref selectTest function may use the helpers \ref
  QCPVector2D::distanceSquaredToLine and \ref rectDistance. With these, the implementation of the
  selection test becomes significantly simpler for most items. See the documentation of \ref
  selectTest for what the function parameters mean and what the function should return.
  
  \subsection anchors Providing anchors
  
  Providing anchors (QCPItemAnchor) starts off like adding a position. First you create a public
  member, e.g.
  
  \code QCPItemAnchor * const bottom;\endcode

  and create it in the constructor with the \ref createAnchor function, assigning it a name and an
  anchor id (an integer enumerating all anchors on the item, you may create an own enum for this).
  Since anchors can be placed anywhere, relative to the item's position(s), your item needs to
  provide the position of every anchor with the reimplementation of the \ref anchorPixelPosition(int
  anchorId) function.
  
  In essence the QCPItemAnchor is merely an intermediary that itself asks your item for the pixel
  position when anything attached to the anchor needs to know the coordinates.
*/

/* start of documentation of inline functions */

/*! \fn QList<QCPItemPosition*> QCPAbstractItem::positions() const
  
  Returns all positions of the item in a list.
  
  \see anchors, position
*/

/*! \fn QList<QCPItemAnchor*> QCPAbstractItem::anchors() const
  
  Returns all anchors of the item in a list. Note that since a position (QCPItemPosition) is always
  also an anchor, the list will also contain the positions of this item.
  
  \see positions, anchor
*/

/* end of documentation of inline functions */
/* start documentation of pure virtual functions */

/*! \fn void QCPAbstractItem::draw(QCPPainter *painter) = 0
  \internal
  
  Draws this item with the provided \a painter.
  
  The cliprect of the provided painter is set to the rect returned by \ref clipRect before this
  function is called. The clipRect depends on the clipping settings defined by \ref
  setClipToAxisRect and \ref setClipAxisRect.
*/

/* end documentation of pure virtual functions */
/* start documentation of signals */

/*! \fn void QCPAbstractItem::selectionChanged(bool selected)
  This signal is emitted when the selection state of this item has changed, either by user interaction
  or by a direct call to \ref setSelected.
*/

/* end documentation of signals */

/*!
  Base class constructor which initializes base class members.
*/
QCPAbstractItem::QCPAbstractItem(QCustomPlot *parentPlot) :
  QCPLayerable(parentPlot),
  mClipToAxisRect(false),
  mSelectable(true),
  mSelected(false)
{
  parentPlot->registerItem(this);
  
  QList<QCPAxisRect*> rects = parentPlot->axisRects();
  if (!rects.isEmpty())
  {
    setClipToAxisRect(true);
    setClipAxisRect(rects.first());
  }
}

QCPAbstractItem::~QCPAbstractItem()
{
  // don't delete mPositions because every position is also an anchor and thus in mAnchors
  qDeleteAll(mAnchors);
}

/* can't make this a header inline function, because QPointer breaks with forward declared types, see QTBUG-29588 */
QCPAxisRect *QCPAbstractItem::clipAxisRect() const
{
  return mClipAxisRect.data();
}

/*!
  Sets whether the item shall be clipped to an axis rect or whether it shall be visible on the
  entire QCustomPlot. The axis rect can be set with \ref setClipAxisRect.
  
  \see setClipAxisRect
*/
void QCPAbstractItem::setClipToAxisRect(bool clip)
{
  mClipToAxisRect = clip;
  if (mClipToAxisRect)
    setParentLayerable(mClipAxisRect.data());
}

/*!
  Sets the clip axis rect. It defines the rect that will be used to clip the item when \ref
  setClipToAxisRect is set to true.
  
  \see setClipToAxisRect
*/
void QCPAbstractItem::setClipAxisRect(QCPAxisRect *rect)
{
  mClipAxisRect = rect;
  if (mClipToAxisRect)
    setParentLayerable(mClipAxisRect.data());
}

/*!
  Sets whether the user can (de-)select this item by clicking on the QCustomPlot surface.
  (When \ref QCustomPlot::setInteractions contains QCustomPlot::iSelectItems.)
  
  However, even when \a selectable was set to false, it is possible to set the selection manually,
  by calling \ref setSelected.
  
  \see QCustomPlot::setInteractions, setSelected
*/
void QCPAbstractItem::setSelectable(bool selectable)
{
  if (mSelectable != selectable)
  {
    mSelectable = selectable;
    emit selectableChanged(mSelectable);
  }
}

/*!
  Sets whether this item is selected or not. When selected, it might use a different visual
  appearance (e.g. pen and brush), this depends on the specific item though.

  The entire selection mechanism for items is handled automatically when \ref
  QCustomPlot::setInteractions contains QCustomPlot::iSelectItems. You only need to call this
  function when you wish to change the selection state manually.
  
  This function can change the selection state even when \ref setSelectable was set to false.
  
  emits the \ref selectionChanged signal when \a selected is different from the previous selection state.
  
  \see setSelectable, selectTest
*/
void QCPAbstractItem::setSelected(bool selected)
{
  if (mSelected != selected)
  {
    mSelected = selected;
    emit selectionChanged(mSelected);
  }
}

/*!
  Returns the QCPItemPosition with the specified \a name. If this item doesn't have a position by
  that name, returns \c nullptr.
  
  This function provides an alternative way to access item positions. Normally, you access
  positions direcly by their member pointers (which typically have the same variable name as \a
  name).
  
  \see positions, anchor
*/
QCPItemPosition *QCPAbstractItem::position(const QString &name) const
{
  foreach (QCPItemPosition *position, mPositions)
  {
    if (position->name() == name)
      return position;
  }
  qDebug() << Q_FUNC_INFO << "position with name not found:" << name;
  return nullptr;
}

/*!
  Returns the QCPItemAnchor with the specified \a name. If this item doesn't have an anchor by
  that name, returns \c nullptr.
  
  This function provides an alternative way to access item anchors. Normally, you access
  anchors direcly by their member pointers (which typically have the same variable name as \a
  name).
  
  \see anchors, position
*/
QCPItemAnchor *QCPAbstractItem::anchor(const QString &name) const
{
  foreach (QCPItemAnchor *anchor, mAnchors)
  {
    if (anchor->name() == name)
      return anchor;
  }
  qDebug() << Q_FUNC_INFO << "anchor with name not found:" << name;
  return nullptr;
}

/*!
  Returns whether this item has an anchor with the specified \a name.
  
  Note that you can check for positions with this function, too. This is because every position is
  also an anchor (QCPItemPosition inherits from QCPItemAnchor).
  
  \see anchor, position
*/
bool QCPAbstractItem::hasAnchor(const QString &name) const
{
  foreach (QCPItemAnchor *anchor, mAnchors)
  {
    if (anchor->name() == name)
      return true;
  }
  return false;
}

/*! \internal
  
  Returns the rect the visual representation of this item is clipped to. This depends on the
  current setting of \ref setClipToAxisRect as well as the axis rect set with \ref setClipAxisRect.
  
  If the item is not clipped to an axis rect, QCustomPlot's viewport rect is returned.
  
  \see draw
*/
QRect QCPAbstractItem::clipRect() const
{
  if (mClipToAxisRect && mClipAxisRect)
    return mClipAxisRect.data()->rect();
  else
    return mParentPlot->viewport();
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing item lines.

  This is the antialiasing state the painter passed to the \ref draw method is in by default.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased
*/
void QCPAbstractItem::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeItems);
}

/*! \internal

  A convenience function which returns the selectTest value for a specified \a rect and a specified
  click position \a pos. \a filledRect defines whether a click inside the rect should also be
  considered a hit or whether only the rect border is sensitive to hits.
  
  This function may be used to help with the implementation of the \ref selectTest function for
  specific items.
  
  For example, if your item consists of four rects, call this function four times, once for each
  rect, in your \ref selectTest reimplementation. Finally, return the minimum (non -1) of all four
  returned values.
*/
double QCPAbstractItem::rectDistance(const QRectF &rect, const QPointF &pos, bool filledRect) const
{
  double result = -1;

  // distance to border:
  const QList<QLineF> lines = QList<QLineF>() << QLineF(rect.topLeft(), rect.topRight()) << QLineF(rect.bottomLeft(), rect.bottomRight())
                                              << QLineF(rect.topLeft(), rect.bottomLeft()) << QLineF(rect.topRight(), rect.bottomRight());
  const QCPVector2D posVec(pos);
  double minDistSqr = (std::numeric_limits<double>::max)();
  foreach (const QLineF &line, lines)
  {
    double distSqr = posVec.distanceSquaredToLine(line.p1(), line.p2());
    if (distSqr < minDistSqr)
      minDistSqr = distSqr;
  }
  result = qSqrt(minDistSqr);
  
  // filled rect, allow click inside to count as hit:
  if (filledRect && result > mParentPlot->selectionTolerance()*0.99)
  {
    if (rect.contains(pos))
      result = mParentPlot->selectionTolerance()*0.99;
  }
  return result;
}

/*! \internal

  Returns the pixel position of the anchor with Id \a anchorId. This function must be reimplemented in
  item subclasses if they want to provide anchors (QCPItemAnchor).
  
  For example, if the item has two anchors with id 0 and 1, this function takes one of these anchor
  ids and returns the respective pixel points of the specified anchor.
  
  \see createAnchor
*/
QPointF QCPAbstractItem::anchorPixelPosition(int anchorId) const
{
  qDebug() << Q_FUNC_INFO << "called on item which shouldn't have any anchors (this method not reimplemented). anchorId" << anchorId;
  return {};
}

/*! \internal

  Creates a QCPItemPosition, registers it with this item and returns a pointer to it. The specified
  \a name must be a unique string that is usually identical to the variable name of the position
  member (This is needed to provide the name-based \ref position access to positions).
  
  Don't delete positions created by this function manually, as the item will take care of it.
  
  Use this function in the constructor (initialization list) of the specific item subclass to
  create each position member. Don't create QCPItemPositions with \b new yourself, because they
  won't be registered with the item properly.
  
  \see createAnchor
*/
QCPItemPosition *QCPAbstractItem::createPosition(const QString &name)
{
  if (hasAnchor(name))
    qDebug() << Q_FUNC_INFO << "anchor/position with name exists already:" << name;
  QCPItemPosition *newPosition = new QCPItemPosition(mParentPlot, this, name);
  mPositions.append(newPosition);
  mAnchors.append(newPosition); // every position is also an anchor
  newPosition->setAxes(mParentPlot->xAxis, mParentPlot->yAxis);
  newPosition->setType(QCPItemPosition::ptPlotCoords);
  if (mParentPlot->axisRect())
    newPosition->setAxisRect(mParentPlot->axisRect());
  newPosition->setCoords(0, 0);
  return newPosition;
}

/*! \internal

  Creates a QCPItemAnchor, registers it with this item and returns a pointer to it. The specified
  \a name must be a unique string that is usually identical to the variable name of the anchor
  member (This is needed to provide the name based \ref anchor access to anchors).
  
  The \a anchorId must be a number identifying the created anchor. It is recommended to create an
  enum (e.g. "AnchorIndex") for this on each item that uses anchors. This id is used by the anchor
  to identify itself when it calls QCPAbstractItem::anchorPixelPosition. That function then returns
  the correct pixel coordinates for the passed anchor id.
  
  Don't delete anchors created by this function manually, as the item will take care of it.
  
  Use this function in the constructor (initialization list) of the specific item subclass to
  create each anchor member. Don't create QCPItemAnchors with \b new yourself, because then they
  won't be registered with the item properly.
  
  \see createPosition
*/
QCPItemAnchor *QCPAbstractItem::createAnchor(const QString &name, int anchorId)
{
  if (hasAnchor(name))
    qDebug() << Q_FUNC_INFO << "anchor/position with name exists already:" << name;
  QCPItemAnchor *newAnchor = new QCPItemAnchor(mParentPlot, this, name, anchorId);
  mAnchors.append(newAnchor);
  return newAnchor;
}

/* inherits documentation from base class */
void QCPAbstractItem::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  Q_UNUSED(details)
  if (mSelectable)
  {
    bool selBefore = mSelected;
    setSelected(additive ? !mSelected : true);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}

/* inherits documentation from base class */
void QCPAbstractItem::deselectEvent(bool *selectionStateChanged)
{
  if (mSelectable)
  {
    bool selBefore = mSelected;
    setSelected(false);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}

/* inherits documentation from base class */
QCP::Interaction QCPAbstractItem::selectionCategory() const
{
  return QCP::iSelectItems;
}
