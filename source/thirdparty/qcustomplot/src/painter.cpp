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

#include "painter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPainter
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPainter
  \brief QPainter subclass used internally
  
  This QPainter subclass is used to provide some extended functionality e.g. for tweaking position
  consistency between antialiased and non-antialiased painting. Further it provides workarounds
  for QPainter quirks.
  
  \warning This class intentionally hides non-virtual functions of QPainter, e.g. setPen, save and
  restore. So while it is possible to pass a QCPPainter instance to a function that expects a
  QPainter pointer, some of the workarounds and tweaks will be unavailable to the function (because
  it will call the base class implementations of the functions actually hidden by QCPPainter).
*/

/*!
  Creates a new QCPPainter instance and sets default values
*/
QCPPainter::QCPPainter() :
  mModes(pmDefault),
  mIsAntialiasing(false)
{
  // don't setRenderHint(QPainter::NonCosmeticDefautPen) here, because painter isn't active yet and
  // a call to begin() will follow
}

/*!
  Creates a new QCPPainter instance on the specified paint \a device and sets default values. Just
  like the analogous QPainter constructor, begins painting on \a device immediately.
  
  Like \ref begin, this method sets QPainter::NonCosmeticDefaultPen in Qt versions before Qt5.
*/
QCPPainter::QCPPainter(QPaintDevice *device) :
  QPainter(device),
  mModes(pmDefault),
  mIsAntialiasing(false)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0) // before Qt5, default pens used to be cosmetic if NonCosmeticDefaultPen flag isn't set. So we set it to get consistency across Qt versions.
  if (isActive())
    setRenderHint(QPainter::NonCosmeticDefaultPen);
#endif
}

/*!
  Sets the pen of the painter and applies certain fixes to it, depending on the mode of this
  QCPPainter.
  
  \note this function hides the non-virtual base class implementation.
*/
void QCPPainter::setPen(const QPen &pen)
{
  QPainter::setPen(pen);
  if (mModes.testFlag(pmNonCosmetic))
    makeNonCosmetic();
}

/*! \overload
  
  Sets the pen (by color) of the painter and applies certain fixes to it, depending on the mode of
  this QCPPainter.
  
  \note this function hides the non-virtual base class implementation.
*/
void QCPPainter::setPen(const QColor &color)
{
  QPainter::setPen(color);
  if (mModes.testFlag(pmNonCosmetic))
    makeNonCosmetic();
}

/*! \overload
  
  Sets the pen (by style) of the painter and applies certain fixes to it, depending on the mode of
  this QCPPainter.
  
  \note this function hides the non-virtual base class implementation.
*/
void QCPPainter::setPen(Qt::PenStyle penStyle)
{
  QPainter::setPen(penStyle);
  if (mModes.testFlag(pmNonCosmetic))
    makeNonCosmetic();
}

/*! \overload
  
  Works around a Qt bug introduced with Qt 4.8 which makes drawing QLineF unpredictable when
  antialiasing is disabled. Thus when antialiasing is disabled, it rounds the \a line to
  integer coordinates and then passes it to the original drawLine.
  
  \note this function hides the non-virtual base class implementation.
*/
void QCPPainter::drawLine(const QLineF &line)
{
  if (mIsAntialiasing || mModes.testFlag(pmVectorized))
    QPainter::drawLine(line);
  else
    QPainter::drawLine(line.toLine());
}

/*!
  Sets whether painting uses antialiasing or not. Use this method instead of using setRenderHint
  with QPainter::Antialiasing directly, as it allows QCPPainter to regain pixel exactness between
  antialiased and non-antialiased painting (Since Qt < 5.0 uses slightly different coordinate systems for
  AA/Non-AA painting).
*/
void QCPPainter::setAntialiasing(bool enabled)
{
  setRenderHint(QPainter::Antialiasing, enabled);
  if (mIsAntialiasing != enabled)
  {
    mIsAntialiasing = enabled;
    if (!mModes.testFlag(pmVectorized)) // antialiasing half-pixel shift only needed for rasterized outputs
    {
      if (mIsAntialiasing)
        translate(0.5, 0.5);
      else
        translate(-0.5, -0.5);
    }
  }
}

/*!
  Sets the mode of the painter. This controls whether the painter shall adjust its
  fixes/workarounds optimized for certain output devices.
*/
void QCPPainter::setModes(QCPPainter::PainterModes modes)
{
  mModes = modes;
}

/*!
  Sets the QPainter::NonCosmeticDefaultPen in Qt versions before Qt5 after beginning painting on \a
  device. This is necessary to get cosmetic pen consistency across Qt versions, because since Qt5,
  all pens are non-cosmetic by default, and in Qt4 this render hint must be set to get that
  behaviour.
  
  The Constructor \ref QCPPainter(QPaintDevice *device) which directly starts painting also sets
  the render hint as appropriate.
  
  \note this function hides the non-virtual base class implementation.
*/
bool QCPPainter::begin(QPaintDevice *device)
{
  bool result = QPainter::begin(device);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0) // before Qt5, default pens used to be cosmetic if NonCosmeticDefaultPen flag isn't set. So we set it to get consistency across Qt versions.
  if (result)
    setRenderHint(QPainter::NonCosmeticDefaultPen);
#endif
  return result;
}

/*! \overload
  
  Sets the mode of the painter. This controls whether the painter shall adjust its
  fixes/workarounds optimized for certain output devices.
*/
void QCPPainter::setMode(QCPPainter::PainterMode mode, bool enabled)
{
  if (!enabled && mModes.testFlag(mode))
    mModes &= ~mode;
  else if (enabled && !mModes.testFlag(mode))
    mModes |= mode;
}

/*!
  Saves the painter (see QPainter::save). Since QCPPainter adds some new internal state to
  QPainter, the save/restore functions are reimplemented to also save/restore those members.
  
  \note this function hides the non-virtual base class implementation.
  
  \see restore
*/
void QCPPainter::save()
{
  mAntialiasingStack.push(mIsAntialiasing);
  QPainter::save();
}

/*!
  Restores the painter (see QPainter::restore). Since QCPPainter adds some new internal state to
  QPainter, the save/restore functions are reimplemented to also save/restore those members.
  
  \note this function hides the non-virtual base class implementation.
  
  \see save
*/
void QCPPainter::restore()
{
  if (!mAntialiasingStack.isEmpty())
    mIsAntialiasing = mAntialiasingStack.pop();
  else
    qDebug() << Q_FUNC_INFO << "Unbalanced save/restore";
  QPainter::restore();
}

/*!
  Changes the pen width to 1 if it currently is 0. This function is called in the \ref setPen
  overrides when the \ref pmNonCosmetic mode is set.
*/
void QCPPainter::makeNonCosmetic()
{
  if (qFuzzyIsNull(pen().widthF()))
  {
    QPen p = pen();
    p.setWidth(1);
    QPainter::setPen(p);
  }
}

