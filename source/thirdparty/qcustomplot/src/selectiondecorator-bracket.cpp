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

#include "selectiondecorator-bracket.h"

#include "plottable1d.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPSelectionDecoratorBracket
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPSelectionDecoratorBracket
  \brief A selection decorator which draws brackets around each selected data segment
  
  Additionally to the regular highlighting of selected segments via color, fill and scatter style,
  this \ref QCPSelectionDecorator subclass draws markers at the begin and end of each selected data
  segment of the plottable.
  
  The shape of the markers can be controlled with \ref setBracketStyle, \ref setBracketWidth and
  \ref setBracketHeight. The color/fill can be controlled with \ref setBracketPen and \ref
  setBracketBrush.
  
  To introduce custom bracket styles, it is only necessary to sublcass \ref
  QCPSelectionDecoratorBracket and reimplement \ref drawBracket. The rest will be managed by the
  base class.
*/

/*!
  Creates a new QCPSelectionDecoratorBracket instance with default values.
*/
QCPSelectionDecoratorBracket::QCPSelectionDecoratorBracket() :
  mBracketPen(QPen(Qt::black)),
  mBracketBrush(Qt::NoBrush),
  mBracketWidth(5),
  mBracketHeight(50),
  mBracketStyle(bsSquareBracket),
  mTangentToData(false),
  mTangentAverage(2)
{
  
}

QCPSelectionDecoratorBracket::~QCPSelectionDecoratorBracket()
{
}

/*!
  Sets the pen that will be used to draw the brackets at the beginning and end of each selected
  data segment.
*/
void QCPSelectionDecoratorBracket::setBracketPen(const QPen &pen)
{
  mBracketPen = pen;
}

/*!
  Sets the brush that will be used to draw the brackets at the beginning and end of each selected
  data segment.
*/
void QCPSelectionDecoratorBracket::setBracketBrush(const QBrush &brush)
{
  mBracketBrush = brush;
}

/*!
  Sets the width of the drawn bracket. The width dimension is always parallel to the key axis of
  the data, or the tangent direction of the current data slope, if \ref setTangentToData is
  enabled.
*/
void QCPSelectionDecoratorBracket::setBracketWidth(int width)
{
  mBracketWidth = width;
}

/*!
  Sets the height of the drawn bracket. The height dimension is always perpendicular to the key axis
  of the data, or the tangent direction of the current data slope, if \ref setTangentToData is
  enabled.
*/
void QCPSelectionDecoratorBracket::setBracketHeight(int height)
{
  mBracketHeight = height;
}

/*!
  Sets the shape that the bracket/marker will have.
  
  \see setBracketWidth, setBracketHeight
*/
void QCPSelectionDecoratorBracket::setBracketStyle(QCPSelectionDecoratorBracket::BracketStyle style)
{
  mBracketStyle = style;
}

/*!
  Sets whether the brackets will be rotated such that they align with the slope of the data at the
  position that they appear in.
  
  For noisy data, it might be more visually appealing to average the slope over multiple data
  points. This can be configured via \ref setTangentAverage.
*/
void QCPSelectionDecoratorBracket::setTangentToData(bool enabled)
{
  mTangentToData = enabled;
}

/*!
  Controls over how many data points the slope shall be averaged, when brackets shall be aligned
  with the data (if \ref setTangentToData is true).
  
  From the position of the bracket, \a pointCount points towards the selected data range will be
  taken into account. The smallest value of \a pointCount is 1, which is effectively equivalent to
  disabling \ref setTangentToData.
*/
void QCPSelectionDecoratorBracket::setTangentAverage(int pointCount)
{
  mTangentAverage = pointCount;
  if (mTangentAverage < 1)
    mTangentAverage = 1;
}

/*!
  Draws the bracket shape with \a painter. The parameter \a direction is either -1 or 1 and
  indicates whether the bracket shall point to the left or the right (i.e. is a closing or opening
  bracket, respectively).
  
  The passed \a painter already contains all transformations that are necessary to position and
  rotate the bracket appropriately. Painting operations can be performed as if drawing upright
  brackets on flat data with horizontal key axis, with (0, 0) being the center of the bracket.
  
  If you wish to sublcass \ref QCPSelectionDecoratorBracket in order to provide custom bracket
  shapes (see \ref QCPSelectionDecoratorBracket::bsUserStyle), this is the method you should
  reimplement.
*/
void QCPSelectionDecoratorBracket::drawBracket(QCPPainter *painter, int direction) const
{
  switch (mBracketStyle)
  {
    case bsSquareBracket:
    {
      painter->drawLine(QLineF(mBracketWidth*direction, -mBracketHeight*0.5, 0, -mBracketHeight*0.5));
      painter->drawLine(QLineF(mBracketWidth*direction, mBracketHeight*0.5, 0, mBracketHeight*0.5));
      painter->drawLine(QLineF(0, -mBracketHeight*0.5, 0, mBracketHeight*0.5));
      break;
    }
    case bsHalfEllipse:
    {
      painter->drawArc(QRectF(-mBracketWidth*0.5, -mBracketHeight*0.5, mBracketWidth, mBracketHeight), -90*16, -180*16*direction);
      break;
    }
    case bsEllipse:
    {
      painter->drawEllipse(QRectF(-mBracketWidth*0.5, -mBracketHeight*0.5, mBracketWidth, mBracketHeight));
      break;
    }
    case bsPlus:
    {
      painter->drawLine(QLineF(0, -mBracketHeight*0.5, 0, mBracketHeight*0.5));
      painter->drawLine(QLineF(-mBracketWidth*0.5, 0, mBracketWidth*0.5, 0));
      break;
    }
    default:
    {
      qDebug() << Q_FUNC_INFO << "unknown/custom bracket style can't be handeld by default implementation:" << static_cast<int>(mBracketStyle);
      break;
    }
  }
}

/*!
  Draws the bracket decoration on the data points at the begin and end of each selected data
  segment given in \a seletion.
  
  It uses the method \ref drawBracket to actually draw the shapes.
  
  \seebaseclassmethod
*/
void QCPSelectionDecoratorBracket::drawDecoration(QCPPainter *painter, QCPDataSelection selection)
{
  if (!mPlottable || selection.isEmpty()) return;
  
  if (QCPPlottableInterface1D *interface1d = mPlottable->interface1D())
  {
    foreach (const QCPDataRange &dataRange, selection.dataRanges())
    {
      // determine position and (if tangent mode is enabled) angle of brackets:
      int openBracketDir = (mPlottable->keyAxis() && !mPlottable->keyAxis()->rangeReversed()) ? 1 : -1;
      int closeBracketDir = -openBracketDir;
      QPointF openBracketPos = getPixelCoordinates(interface1d, dataRange.begin());
      QPointF closeBracketPos = getPixelCoordinates(interface1d, dataRange.end()-1);
      double openBracketAngle = 0;
      double closeBracketAngle = 0;
      if (mTangentToData)
      {
        openBracketAngle = getTangentAngle(interface1d, dataRange.begin(), openBracketDir);
        closeBracketAngle = getTangentAngle(interface1d, dataRange.end()-1, closeBracketDir);
      }
      // draw opening bracket:
      QTransform oldTransform = painter->transform();
      painter->setPen(mBracketPen);
      painter->setBrush(mBracketBrush);
      painter->translate(openBracketPos);
      painter->rotate(openBracketAngle/M_PI*180.0);
      drawBracket(painter, openBracketDir);
      painter->setTransform(oldTransform);
      // draw closing bracket:
      painter->setPen(mBracketPen);
      painter->setBrush(mBracketBrush);
      painter->translate(closeBracketPos);
      painter->rotate(closeBracketAngle/M_PI*180.0);
      drawBracket(painter, closeBracketDir);
      painter->setTransform(oldTransform);
    }
  }
}

/*! \internal
  
  If \ref setTangentToData is enabled, brackets need to be rotated according to the data slope.
  This method returns the angle in radians by which a bracket at the given \a dataIndex must be
  rotated.
  
  The parameter \a direction must be set to either -1 or 1, representing whether it is an opening
  or closing bracket. Since for slope calculation multiple data points are required, this defines
  the direction in which the algorithm walks, starting at \a dataIndex, to average those data
  points. (see \ref setTangentToData and \ref setTangentAverage)
  
  \a interface1d is the interface to the plottable's data which is used to query data coordinates.
*/
double QCPSelectionDecoratorBracket::getTangentAngle(const QCPPlottableInterface1D *interface1d, int dataIndex, int direction) const
{
  if (!interface1d || dataIndex < 0 || dataIndex >= interface1d->dataCount())
    return 0;
  direction = direction < 0 ? -1 : 1; // enforce direction is either -1 or 1
  
  // how many steps we can actually go from index in the given direction without exceeding data bounds:
  int averageCount;
  if (direction < 0)
    averageCount = qMin(mTangentAverage, dataIndex);
  else
    averageCount = qMin(mTangentAverage, interface1d->dataCount()-1-dataIndex);
  qDebug() << averageCount;
  // calculate point average of averageCount points:
  QVector<QPointF> points(averageCount);
  QPointF pointsAverage;
  int currentIndex = dataIndex;
  for (int i=0; i<averageCount; ++i)
  {
    points[i] = getPixelCoordinates(interface1d, currentIndex);
    pointsAverage += points[i];
    currentIndex += direction;
  }
  pointsAverage /= double(averageCount);
  
  // calculate slope of linear regression through points:
  double numSum = 0;
  double denomSum = 0;
  for (int i=0; i<averageCount; ++i)
  {
    const double dx = points.at(i).x()-pointsAverage.x();
    const double dy = points.at(i).y()-pointsAverage.y();
    numSum += dx*dy;
    denomSum += dx*dx;
  }
  if (!qFuzzyIsNull(denomSum) && !qFuzzyIsNull(numSum))
  {
    return qAtan2(numSum, denomSum);
  } else // undetermined angle, probably mTangentAverage == 1, so using only one data point
    return 0;
}

/*! \internal
  
  Returns the pixel coordinates of the data point at \a dataIndex, using \a interface1d to access
  the data points.
*/
QPointF QCPSelectionDecoratorBracket::getPixelCoordinates(const QCPPlottableInterface1D *interface1d, int dataIndex) const
{
  QCPAxis *keyAxis = mPlottable->keyAxis();
  QCPAxis *valueAxis = mPlottable->valueAxis();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return {0, 0}; }
  
  if (keyAxis->orientation() == Qt::Horizontal)
    return {keyAxis->coordToPixel(interface1d->dataMainKey(dataIndex)), valueAxis->coordToPixel(interface1d->dataMainValue(dataIndex))};
  else
    return {valueAxis->coordToPixel(interface1d->dataMainValue(dataIndex)), keyAxis->coordToPixel(interface1d->dataMainKey(dataIndex))};
}








