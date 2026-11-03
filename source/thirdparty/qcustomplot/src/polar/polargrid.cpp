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

#include "polargrid.h"
#include "layoutelement-angularaxis.h"

#include "../core.h"
#include "../painter.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPolarGrid
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPolarGrid
  \brief The grid in both angular and radial dimensions for polar plots

  \warning In this QCustomPlot version, polar plots are a tech preview. Expect documentation and
  functionality to be incomplete, as well as changing public interfaces in the future.
*/

/*!
  Creates a QCPPolarGrid instance and sets default values.
  
  You shouldn't instantiate grids on their own, since every axis brings its own grid.
*/
QCPPolarGrid::QCPPolarGrid(QCPPolarAxisAngular *parentAxis) :
  QCPLayerable(parentAxis->parentPlot(), QString(), parentAxis),
  mType(gtNone),
  mSubGridType(gtNone),
  mAntialiasedSubGrid(true),
  mAntialiasedZeroLine(true),
  mParentAxis(parentAxis)
{
  // warning: this is called in QCPPolarAxisAngular constructor, so parentAxis members should not be accessed/called
  setParent(parentAxis);
  setType(gtAll);
  setSubGridType(gtNone);
  
  setAngularPen(QPen(QColor(200,200,200), 0, Qt::DotLine));
  setAngularSubGridPen(QPen(QColor(220,220,220), 0, Qt::DotLine));
  
  setRadialPen(QPen(QColor(200,200,200), 0, Qt::DotLine));
  setRadialSubGridPen(QPen(QColor(220,220,220), 0, Qt::DotLine));
  setRadialZeroLinePen(QPen(QColor(200,200,200), 0, Qt::SolidLine));
  
  setAntialiased(true);
}

void QCPPolarGrid::setRadialAxis(QCPPolarAxisRadial *axis)
{
  mRadialAxis = axis;
}

void QCPPolarGrid::setType(GridTypes type)
{
  mType = type;
}

void QCPPolarGrid::setSubGridType(GridTypes type)
{
  mSubGridType = type;
}

/*!
  Sets whether sub grid lines are drawn antialiased.
*/
void QCPPolarGrid::setAntialiasedSubGrid(bool enabled)
{
  mAntialiasedSubGrid = enabled;
}

/*!
  Sets whether zero lines are drawn antialiased.
*/
void QCPPolarGrid::setAntialiasedZeroLine(bool enabled)
{
  mAntialiasedZeroLine = enabled;
}

/*!
  Sets the pen with which (major) grid lines are drawn.
*/
void QCPPolarGrid::setAngularPen(const QPen &pen)
{
  mAngularPen = pen;
}

/*!
  Sets the pen with which sub grid lines are drawn.
*/
void QCPPolarGrid::setAngularSubGridPen(const QPen &pen)
{
  mAngularSubGridPen = pen;
}

void QCPPolarGrid::setRadialPen(const QPen &pen)
{
  mRadialPen = pen;
}

void QCPPolarGrid::setRadialSubGridPen(const QPen &pen)
{
  mRadialSubGridPen = pen;
}

void QCPPolarGrid::setRadialZeroLinePen(const QPen &pen)
{
  mRadialZeroLinePen = pen;
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing the major grid lines.

  This is the antialiasing state the painter passed to the \ref draw method is in by default.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased
*/
void QCPPolarGrid::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeGrid);
}

/*! \internal
  
  Draws grid lines and sub grid lines at the positions of (sub) ticks of the parent axis, spanning
  over the complete axis rect. Also draws the zero line, if appropriate (\ref setZeroLinePen).
*/
void QCPPolarGrid::draw(QCPPainter *painter)
{
  if (!mParentAxis) { qDebug() << Q_FUNC_INFO << "invalid parent axis"; return; }
  
  const QPointF center = mParentAxis->mCenter;
  const double radius = mParentAxis->mRadius;
  
  painter->setBrush(Qt::NoBrush);
  // draw main angular grid:
  if (mType.testFlag(gtAngular))
    drawAngularGrid(painter, center, radius, mParentAxis->mTickVectorCosSin, mAngularPen);
  // draw main radial grid:
  if (mType.testFlag(gtRadial) && mRadialAxis)
    drawRadialGrid(painter, center, mRadialAxis->tickVector(), mRadialPen, mRadialZeroLinePen);
  
  applyAntialiasingHint(painter, mAntialiasedSubGrid, QCP::aeGrid);
  // draw sub angular grid:
  if (mSubGridType.testFlag(gtAngular))
    drawAngularGrid(painter, center, radius, mParentAxis->mSubTickVectorCosSin, mAngularSubGridPen);
  // draw sub radial grid:
  if (mSubGridType.testFlag(gtRadial) && mRadialAxis)
    drawRadialGrid(painter, center, mRadialAxis->subTickVector(), mRadialSubGridPen);
}

void QCPPolarGrid::drawRadialGrid(QCPPainter *painter, const QPointF &center, const QVector<double> &coords, const QPen &pen, const QPen &zeroPen)
{
  if (!mRadialAxis) return;
  if (coords.isEmpty()) return;
  const bool drawZeroLine = zeroPen != Qt::NoPen;
  const double zeroLineEpsilon = qAbs(coords.last()-coords.first())*1e-6;
  
  painter->setPen(pen);
  for (int i=0; i<coords.size(); ++i)
  {
    const double r = mRadialAxis->coordToRadius(coords.at(i));
    if (drawZeroLine && qAbs(coords.at(i)) < zeroLineEpsilon)
    {
      applyAntialiasingHint(painter, mAntialiasedZeroLine, QCP::aeZeroLine);
      painter->setPen(zeroPen);
      painter->drawEllipse(center, r, r);
      painter->setPen(pen);
      applyDefaultAntialiasingHint(painter);
    } else
    {
      painter->drawEllipse(center, r, r);
    }
  }
}

void QCPPolarGrid::drawAngularGrid(QCPPainter *painter, const QPointF &center, double radius, const QVector<QPointF> &ticksCosSin, const QPen &pen)
{
  if (ticksCosSin.isEmpty()) return;
  
  painter->setPen(pen);
  for (int i=0; i<ticksCosSin.size(); ++i)
    painter->drawLine(center, center+ticksCosSin.at(i)*radius);
}
