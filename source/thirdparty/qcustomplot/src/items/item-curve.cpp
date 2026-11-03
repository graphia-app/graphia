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

#include "item-curve.h"

#include "../painter.h"
#include "../core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPItemCurve
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPItemCurve
  \brief A curved line from one point to another

  \image html QCPItemCurve.png "Curve example. Blue dotted circles are anchors, solid blue discs are positions."

  It has four positions, \a start and \a end, which define the end points of the line, and two
  control points which define the direction the line exits from the start and the direction from
  which it approaches the end: \a startDir and \a endDir.
  
  With \ref setHead and \ref setTail you may set different line ending styles, e.g. to create an
  arrow.
  
  Often it is desirable for the control points to stay at fixed relative positions to the start/end
  point. This can be achieved by setting the parent anchor e.g. of \a startDir simply to \a start,
  and then specify the desired pixel offset with QCPItemPosition::setCoords on \a startDir.
*/

/*!
  Creates a curve item and sets default values.
  
  The created item is automatically registered with \a parentPlot. This QCustomPlot instance takes
  ownership of the item, so do not delete it manually but use QCustomPlot::removeItem() instead.
*/
QCPItemCurve::QCPItemCurve(QCustomPlot *parentPlot) :
  QCPAbstractItem(parentPlot),
  start(createPosition(QLatin1String("start"))),
  startDir(createPosition(QLatin1String("startDir"))),
  endDir(createPosition(QLatin1String("endDir"))),
  end(createPosition(QLatin1String("end")))
{
  start->setCoords(0, 0);
  startDir->setCoords(0.5, 0);
  endDir->setCoords(0, 0.5);
  end->setCoords(1, 1);
  
  setPen(QPen(Qt::black));
  setSelectedPen(QPen(Qt::blue,2));
}

QCPItemCurve::~QCPItemCurve()
{
}

/*!
  Sets the pen that will be used to draw the line
  
  \see setSelectedPen
*/
void QCPItemCurve::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  Sets the pen that will be used to draw the line when selected
  
  \see setPen, setSelected
*/
void QCPItemCurve::setSelectedPen(const QPen &pen)
{
  mSelectedPen = pen;
}

/*!
  Sets the line ending style of the head. The head corresponds to the \a end position.
  
  Note that due to the overloaded QCPLineEnding constructor, you may directly specify
  a QCPLineEnding::EndingStyle here, e.g. \code setHead(QCPLineEnding::esSpikeArrow) \endcode
  
  \see setTail
*/
void QCPItemCurve::setHead(const QCPLineEnding &head)
{
  mHead = head;
}

/*!
  Sets the line ending style of the tail. The tail corresponds to the \a start position.
  
  Note that due to the overloaded QCPLineEnding constructor, you may directly specify
  a QCPLineEnding::EndingStyle here, e.g. \code setTail(QCPLineEnding::esSpikeArrow) \endcode
  
  \see setHead
*/
void QCPItemCurve::setTail(const QCPLineEnding &tail)
{
  mTail = tail;
}

/* inherits documentation from base class */
double QCPItemCurve::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (onlySelectable && !mSelectable)
    return -1;
  
  QPointF startVec(start->pixelPosition());
  QPointF startDirVec(startDir->pixelPosition());
  QPointF endDirVec(endDir->pixelPosition());
  QPointF endVec(end->pixelPosition());

  QPainterPath cubicPath(startVec);
  cubicPath.cubicTo(startDirVec, endDirVec, endVec);
  
  QList<QPolygonF> polygons = cubicPath.toSubpathPolygons();
  if (polygons.isEmpty())
    return -1;
  const QPolygonF polygon = polygons.first();
  QCPVector2D p(pos);
  double minDistSqr = (std::numeric_limits<double>::max)();
  for (int i=1; i<polygon.size(); ++i)
  {
    double distSqr = p.distanceSquaredToLine(polygon.at(i-1), polygon.at(i));
    if (distSqr < minDistSqr)
      minDistSqr = distSqr;
  }
  return qSqrt(minDistSqr);
}

/* inherits documentation from base class */
void QCPItemCurve::draw(QCPPainter *painter)
{
  QCPVector2D startVec(start->pixelPosition());
  QCPVector2D startDirVec(startDir->pixelPosition());
  QCPVector2D endDirVec(endDir->pixelPosition());
  QCPVector2D endVec(end->pixelPosition());
  if ((endVec-startVec).length() > 1e10) // too large curves cause crash
    return;

  QPainterPath cubicPath(startVec.toPointF());
  cubicPath.cubicTo(startDirVec.toPointF(), endDirVec.toPointF(), endVec.toPointF());

  // paint visible segment, if existent:
  const int clipEnlarge = qCeil(mainPen().widthF());
  QRect clip = clipRect().adjusted(-clipEnlarge, -clipEnlarge, clipEnlarge, clipEnlarge);
  QRect cubicRect = cubicPath.controlPointRect().toRect();
  if (cubicRect.isEmpty()) // may happen when start and end exactly on same x or y position
    cubicRect.adjust(0, 0, 1, 1);
  if (clip.intersects(cubicRect))
  {
    painter->setPen(mainPen());
    painter->drawPath(cubicPath);
    painter->setBrush(Qt::SolidPattern);
    if (mTail.style() != QCPLineEnding::esNone)
      mTail.draw(painter, startVec, M_PI-cubicPath.angleAtPercent(0)/180.0*M_PI);
    if (mHead.style() != QCPLineEnding::esNone)
      mHead.draw(painter, endVec, -cubicPath.angleAtPercent(1)/180.0*M_PI);
  }
}

/*! \internal

  Returns the pen that should be used for drawing lines. Returns mPen when the
  item is not selected and mSelectedPen when it is.
*/
QPen QCPItemCurve::mainPen() const
{
  return mSelected ? mSelectedPen : mPen;
}
