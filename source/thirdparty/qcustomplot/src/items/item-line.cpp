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

#include "item-line.h"

#include "../painter.h"
#include "../core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPItemLine
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPItemLine
  \brief A line from one point to another

  \image html QCPItemLine.png "Line example. Blue dotted circles are anchors, solid blue discs are positions."

  It has two positions, \a start and \a end, which define the end points of the line.
  
  With \ref setHead and \ref setTail you may set different line ending styles, e.g. to create an arrow.
*/

/*!
  Creates a line item and sets default values.
  
  The created item is automatically registered with \a parentPlot. This QCustomPlot instance takes
  ownership of the item, so do not delete it manually but use QCustomPlot::removeItem() instead.
*/
QCPItemLine::QCPItemLine(QCustomPlot *parentPlot) :
  QCPAbstractItem(parentPlot),
  start(createPosition(QLatin1String("start"))),
  end(createPosition(QLatin1String("end")))
{
  start->setCoords(0, 0);
  end->setCoords(1, 1);
  
  setPen(QPen(Qt::black));
  setSelectedPen(QPen(Qt::blue,2));
}

QCPItemLine::~QCPItemLine()
{
}

/*!
  Sets the pen that will be used to draw the line
  
  \see setSelectedPen
*/
void QCPItemLine::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  Sets the pen that will be used to draw the line when selected
  
  \see setPen, setSelected
*/
void QCPItemLine::setSelectedPen(const QPen &pen)
{
  mSelectedPen = pen;
}

/*!
  Sets the line ending style of the head. The head corresponds to the \a end position.
  
  Note that due to the overloaded QCPLineEnding constructor, you may directly specify
  a QCPLineEnding::EndingStyle here, e.g. \code setHead(QCPLineEnding::esSpikeArrow) \endcode
  
  \see setTail
*/
void QCPItemLine::setHead(const QCPLineEnding &head)
{
  mHead = head;
}

/*!
  Sets the line ending style of the tail. The tail corresponds to the \a start position.
  
  Note that due to the overloaded QCPLineEnding constructor, you may directly specify
  a QCPLineEnding::EndingStyle here, e.g. \code setTail(QCPLineEnding::esSpikeArrow) \endcode
  
  \see setHead
*/
void QCPItemLine::setTail(const QCPLineEnding &tail)
{
  mTail = tail;
}

/* inherits documentation from base class */
double QCPItemLine::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (onlySelectable && !mSelectable)
    return -1;
  
  return qSqrt(QCPVector2D(pos).distanceSquaredToLine(start->pixelPosition(), end->pixelPosition()));
}

/* inherits documentation from base class */
void QCPItemLine::draw(QCPPainter *painter)
{
  QCPVector2D startVec(start->pixelPosition());
  QCPVector2D endVec(end->pixelPosition());
  if (qFuzzyIsNull((startVec-endVec).lengthSquared()))
    return;
  // get visible segment of straight line inside clipRect:
  int clipPad = int(qMax(mHead.boundingDistance(), mTail.boundingDistance()));
  clipPad = qMax(clipPad, qCeil(mainPen().widthF()));
  QLineF line = getRectClippedLine(startVec, endVec, clipRect().adjusted(-clipPad, -clipPad, clipPad, clipPad));
  // paint visible segment, if existent:
  if (!line.isNull())
  {
    painter->setPen(mainPen());
    painter->drawLine(line);
    painter->setBrush(Qt::SolidPattern);
    if (mTail.style() != QCPLineEnding::esNone)
      mTail.draw(painter, startVec, startVec-endVec);
    if (mHead.style() != QCPLineEnding::esNone)
      mHead.draw(painter, endVec, endVec-startVec);
  }
}

/*! \internal

  Returns the section of the line defined by \a start and \a end, that is visible in the specified
  \a rect.
  
  This is a helper function for \ref draw.
*/
QLineF QCPItemLine::getRectClippedLine(const QCPVector2D &start, const QCPVector2D &end, const QRect &rect) const
{
  bool containsStart = rect.contains(qRound(start.x()), qRound(start.y()));
  bool containsEnd = rect.contains(qRound(end.x()), qRound(end.y()));
  if (containsStart && containsEnd)
    return {start.toPointF(), end.toPointF()};
  
  QCPVector2D base = start;
  QCPVector2D vec = end-start;
  double bx, by;
  double gamma, mu;
  QLineF result;
  QList<QCPVector2D> pointVectors;

  if (!qFuzzyIsNull(vec.y())) // line is not horizontal
  {
    // check top of rect:
    bx = rect.left();
    by = rect.top();
    mu = (by-base.y())/vec.y();
    if (mu >= 0 && mu <= 1)
    {
      gamma = base.x()-bx + mu*vec.x();
      if (gamma >= 0 && gamma <= rect.width())
        pointVectors.append(QCPVector2D(bx+gamma, by));
    }
    // check bottom of rect:
    bx = rect.left();
    by = rect.bottom();
    mu = (by-base.y())/vec.y();
    if (mu >= 0 && mu <= 1)
    {
      gamma = base.x()-bx + mu*vec.x();
      if (gamma >= 0 && gamma <= rect.width())
        pointVectors.append(QCPVector2D(bx+gamma, by));
    }
  }
  if (!qFuzzyIsNull(vec.x())) // line is not vertical
  {
    // check left of rect:
    bx = rect.left();
    by = rect.top();
    mu = (bx-base.x())/vec.x();
    if (mu >= 0 && mu <= 1)
    {
      gamma = base.y()-by + mu*vec.y();
      if (gamma >= 0 && gamma <= rect.height())
        pointVectors.append(QCPVector2D(bx, by+gamma));
    }
    // check right of rect:
    bx = rect.right();
    by = rect.top();
    mu = (bx-base.x())/vec.x();
    if (mu >= 0 && mu <= 1)
    {
      gamma = base.y()-by + mu*vec.y();
      if (gamma >= 0 && gamma <= rect.height())
        pointVectors.append(QCPVector2D(bx, by+gamma));
    }
  }
  
  if (containsStart)
    pointVectors.append(start);
  if (containsEnd)
    pointVectors.append(end);
  
  // evaluate points:
  if (pointVectors.size() == 2)
  {
    result.setPoints(pointVectors.at(0).toPointF(), pointVectors.at(1).toPointF());
  } else if (pointVectors.size() > 2)
  {
    // line probably goes through corner of rect, and we got two points there. single out the point pair with greatest distance:
    double distSqrMax = 0;
    QCPVector2D pv1, pv2;
    for (int i=0; i<pointVectors.size()-1; ++i)
    {
      for (int k=i+1; k<pointVectors.size(); ++k)
      {
        double distSqr = (pointVectors.at(i)-pointVectors.at(k)).lengthSquared();
        if (distSqr > distSqrMax)
        {
          pv1 = pointVectors.at(i);
          pv2 = pointVectors.at(k);
          distSqrMax = distSqr;
        }
      }
    }
    result.setPoints(pv1.toPointF(), pv2.toPointF());
  }
  return result;
}

/*! \internal

  Returns the pen that should be used for drawing lines. Returns mPen when the
  item is not selected and mSelectedPen when it is.
*/
QPen QCPItemLine::mainPen() const
{
  return mSelected ? mSelectedPen : mPen;
}
