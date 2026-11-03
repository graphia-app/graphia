
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

#include "scatterstyle.h"

#include "painter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPScatterStyle
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPScatterStyle
  \brief Represents the visual appearance of scatter points
  
  This class holds information about shape, color and size of scatter points. In plottables like
  QCPGraph it is used to store how scatter points shall be drawn. For example, \ref
  QCPGraph::setScatterStyle takes a QCPScatterStyle instance.
  
  A scatter style consists of a shape (\ref setShape), a line color (\ref setPen) and possibly a
  fill (\ref setBrush), if the shape provides a fillable area. Further, the size of the shape can
  be controlled with \ref setSize.

  \section QCPScatterStyle-defining Specifying a scatter style
  
  You can set all these configurations either by calling the respective functions on an instance:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpscatterstyle-creation-1
  
  Or you can use one of the various constructors that take different parameter combinations, making
  it easy to specify a scatter style in a single call, like so:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpscatterstyle-creation-2
  
  \section QCPScatterStyle-undefinedpen Leaving the color/pen up to the plottable
  
  There are two constructors which leave the pen undefined: \ref QCPScatterStyle() and \ref
  QCPScatterStyle(ScatterShape shape, double size). If those constructors are used, a call to \ref
  isPenDefined will return false. It leads to scatter points that inherit the pen from the
  plottable that uses the scatter style. Thus, if such a scatter style is passed to QCPGraph, the line
  color of the graph (\ref QCPGraph::setPen) will be used by the scatter points. This makes
  it very convenient to set up typical scatter settings:
  
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpscatterstyle-shortcreation

  Notice that it wasn't even necessary to explicitly call a QCPScatterStyle constructor. This works
  because QCPScatterStyle provides a constructor that can transform a \ref ScatterShape directly
  into a QCPScatterStyle instance (that's the \ref QCPScatterStyle(ScatterShape shape, double size)
  constructor with a default for \a size). In those cases, C++ allows directly supplying a \ref
  ScatterShape, where actually a QCPScatterStyle is expected.
  
  \section QCPScatterStyle-custompath-and-pixmap Custom shapes and pixmaps
  
  QCPScatterStyle supports drawing custom shapes and arbitrary pixmaps as scatter points.

  For custom shapes, you can provide a QPainterPath with the desired shape to the \ref
  setCustomPath function or call the constructor that takes a painter path. The scatter shape will
  automatically be set to \ref ssCustom.
  
  For pixmaps, you call \ref setPixmap with the desired QPixmap. Alternatively you can use the
  constructor that takes a QPixmap. The scatter shape will automatically be set to \ref ssPixmap.
  Note that \ref setSize does not influence the appearance of the pixmap.
*/

/* start documentation of inline functions */

/*! \fn bool QCPScatterStyle::isNone() const
  
  Returns whether the scatter shape is \ref ssNone.
  
  \see setShape
*/

/*! \fn bool QCPScatterStyle::isPenDefined() const
  
  Returns whether a pen has been defined for this scatter style.
  
  The pen is undefined if a constructor is called that does not carry \a pen as parameter. Those
  are \ref QCPScatterStyle() and \ref QCPScatterStyle(ScatterShape shape, double size). If the pen
  is undefined, the pen of the respective plottable will be used for drawing scatters.
  
  If a pen was defined for this scatter style instance, and you now wish to undefine the pen, call
  \ref undefinePen.
  
  \see setPen
*/

/* end documentation of inline functions */

/*!
  Creates a new QCPScatterStyle instance with size set to 6. No shape, pen or brush is defined.
  
  Since the pen is undefined (\ref isPenDefined returns false), the scatter color will be inherited
  from the plottable that uses this scatter style.
*/
QCPScatterStyle::QCPScatterStyle() :
  mSize(6),
  mShape(ssNone),
  mPen(Qt::NoPen),
  mBrush(Qt::NoBrush),
  mPenDefined(false)
{
}

/*!
  Creates a new QCPScatterStyle instance with shape set to \a shape and size to \a size. No pen or
  brush is defined.
  
  Since the pen is undefined (\ref isPenDefined returns false), the scatter color will be inherited
  from the plottable that uses this scatter style.
*/
QCPScatterStyle::QCPScatterStyle(ScatterShape shape, double size) :
  mSize(size),
  mShape(shape),
  mPen(Qt::NoPen),
  mBrush(Qt::NoBrush),
  mPenDefined(false)
{
}

/*!
  Creates a new QCPScatterStyle instance with shape set to \a shape, the pen color set to \a color,
  and size to \a size. No brush is defined, i.e. the scatter point will not be filled.
*/
QCPScatterStyle::QCPScatterStyle(ScatterShape shape, const QColor &color, double size) :
  mSize(size),
  mShape(shape),
  mPen(QPen(color)),
  mBrush(Qt::NoBrush),
  mPenDefined(true)
{
}

/*!
  Creates a new QCPScatterStyle instance with shape set to \a shape, the pen color set to \a color,
  the brush color to \a fill (with a solid pattern), and size to \a size.
*/
QCPScatterStyle::QCPScatterStyle(ScatterShape shape, const QColor &color, const QColor &fill, double size) :
  mSize(size),
  mShape(shape),
  mPen(QPen(color)),
  mBrush(QBrush(fill)),
  mPenDefined(true)
{
}

/*!
  Creates a new QCPScatterStyle instance with shape set to \a shape, the pen set to \a pen, the
  brush to \a brush, and size to \a size.
  
  \warning In some cases it might be tempting to directly use a pen style like <tt>Qt::NoPen</tt> as \a pen
  and a color like <tt>Qt::blue</tt> as \a brush. Notice however, that the corresponding call\n
  <tt>QCPScatterStyle(QCPScatterShape::ssCircle, Qt::NoPen, Qt::blue, 5)</tt>\n
  doesn't necessarily lead C++ to use this constructor in some cases, but might mistake
  <tt>Qt::NoPen</tt> for a QColor and use the
  \ref QCPScatterStyle(ScatterShape shape, const QColor &color, const QColor &fill, double size)
  constructor instead (which will lead to an unexpected look of the scatter points). To prevent
  this, be more explicit with the parameter types. For example, use <tt>QBrush(Qt::blue)</tt>
  instead of just <tt>Qt::blue</tt>, to clearly point out to the compiler that this constructor is
  wanted.
*/
QCPScatterStyle::QCPScatterStyle(ScatterShape shape, const QPen &pen, const QBrush &brush, double size) :
  mSize(size),
  mShape(shape),
  mPen(pen),
  mBrush(brush),
  mPenDefined(pen.style() != Qt::NoPen)
{
}

/*!
  Creates a new QCPScatterStyle instance which will show the specified \a pixmap. The scatter shape
  is set to \ref ssPixmap.
*/
QCPScatterStyle::QCPScatterStyle(const QPixmap &pixmap) :
  mSize(5),
  mShape(ssPixmap),
  mPen(Qt::NoPen),
  mBrush(Qt::NoBrush),
  mPixmap(pixmap),
  mPenDefined(false)
{
}

/*!
  Creates a new QCPScatterStyle instance with a custom shape that is defined via \a customPath. The
  scatter shape is set to \ref ssCustom.
  
  The custom shape line will be drawn with \a pen and filled with \a brush. The size has a slightly
  different meaning than for built-in scatter points: The custom path will be drawn scaled by a
  factor of \a size/6.0. Since the default \a size is 6, the custom path will appear in its
  original size by default. To for example double the size of the path, set \a size to 12.
*/
QCPScatterStyle::QCPScatterStyle(const QPainterPath &customPath, const QPen &pen, const QBrush &brush, double size) :
  mSize(size),
  mShape(ssCustom),
  mPen(pen),
  mBrush(brush),
  mCustomPath(customPath),
  mPenDefined(pen.style() != Qt::NoPen)
{
}

/*!
  Copies the specified \a properties from the \a other scatter style to this scatter style.
*/
void QCPScatterStyle::setFromOther(const QCPScatterStyle &other, ScatterProperties properties)
{
  if (properties.testFlag(spPen))
  {
    setPen(other.pen());
    if (!other.isPenDefined())
      undefinePen();
  }
  if (properties.testFlag(spBrush))
    setBrush(other.brush());
  if (properties.testFlag(spSize))
    setSize(other.size());
  if (properties.testFlag(spShape))
  {
    setShape(other.shape());
    if (other.shape() == ssPixmap)
      setPixmap(other.pixmap());
    else if (other.shape() == ssCustom)
      setCustomPath(other.customPath());
  }
}

/*!
  Sets the size (pixel diameter) of the drawn scatter points to \a size.
  
  \see setShape
*/
void QCPScatterStyle::setSize(double size)
{
  mSize = size;
}

/*!
  Sets the shape to \a shape.
  
  Note that the calls \ref setPixmap and \ref setCustomPath automatically set the shape to \ref
  ssPixmap and \ref ssCustom, respectively.
  
  \see setSize
*/
void QCPScatterStyle::setShape(QCPScatterStyle::ScatterShape shape)
{
  mShape = shape;
}

/*!
  Sets the pen that will be used to draw scatter points to \a pen.
  
  If the pen was previously undefined (see \ref isPenDefined), the pen is considered defined after
  a call to this function, even if \a pen is <tt>Qt::NoPen</tt>. If you have defined a pen
  previously by calling this function and now wish to undefine the pen, call \ref undefinePen.
  
  \see setBrush
*/
void QCPScatterStyle::setPen(const QPen &pen)
{
  mPenDefined = true;
  mPen = pen;
}

/*!
  Sets the brush that will be used to fill scatter points to \a brush. Note that not all scatter
  shapes have fillable areas. For example, \ref ssPlus does not while \ref ssCircle does.
  
  \see setPen
*/
void QCPScatterStyle::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  Sets the pixmap that will be drawn as scatter point to \a pixmap.
  
  Note that \ref setSize does not influence the appearance of the pixmap.
  
  The scatter shape is automatically set to \ref ssPixmap.
*/
void QCPScatterStyle::setPixmap(const QPixmap &pixmap)
{
  setShape(ssPixmap);
  mPixmap = pixmap;
}

/*!
  Sets the custom shape that will be drawn as scatter point to \a customPath.
  
  The scatter shape is automatically set to \ref ssCustom.
*/
void QCPScatterStyle::setCustomPath(const QPainterPath &customPath)
{
  setShape(ssCustom);
  mCustomPath = customPath;
}

/*!
  Sets this scatter style to have an undefined pen (see \ref isPenDefined for what an undefined pen
  implies).

  A call to \ref setPen will define a pen.
*/
void QCPScatterStyle::undefinePen()
{
  mPenDefined = false;
}

/*!
  Applies the pen and the brush of this scatter style to \a painter. If this scatter style has an
  undefined pen (\ref isPenDefined), sets the pen of \a painter to \a defaultPen instead.
  
  This function is used by plottables (or any class that wants to draw scatters) just before a
  number of scatters with this style shall be drawn with the \a painter.
  
  \see drawShape
*/
void QCPScatterStyle::applyTo(QCPPainter *painter, const QPen &defaultPen) const
{
  painter->setPen(mPenDefined ? mPen : defaultPen);
  painter->setBrush(mBrush);
}

/*!
  Draws the scatter shape with \a painter at position \a pos.
  
  This function does not modify the pen or the brush on the painter, as \ref applyTo is meant to be
  called before scatter points are drawn with \ref drawShape.
  
  \see applyTo
*/
void QCPScatterStyle::drawShape(QCPPainter *painter, const QPointF &pos) const
{
  drawShape(painter, pos.x(), pos.y());
}

/*! \overload
  Draws the scatter shape with \a painter at position \a x and \a y.
*/
void QCPScatterStyle::drawShape(QCPPainter *painter, double x, double y) const
{
  double w = mSize/2.0;
  switch (mShape)
  {
    case ssNone: break;
    case ssDot:
    {
      painter->drawLine(QPointF(x, y), QPointF(x+0.0001, y));
      break;
    }
    case ssCross:
    {
      painter->drawLine(QLineF(x-w, y-w, x+w, y+w));
      painter->drawLine(QLineF(x-w, y+w, x+w, y-w));
      break;
    }
    case ssPlus:
    {
      painter->drawLine(QLineF(x-w,   y, x+w,   y));
      painter->drawLine(QLineF(  x, y+w,   x, y-w));
      break;
    }
    case ssCircle:
    {
      painter->drawEllipse(QPointF(x , y), w, w);
      break;
    }
    case ssDisc:
    {
      QBrush b = painter->brush();
      painter->setBrush(painter->pen().color());
      painter->drawEllipse(QPointF(x , y), w, w);
      painter->setBrush(b);
      break;
    }
    case ssSquare:
    {
      painter->drawRect(QRectF(x-w, y-w, mSize, mSize));
      break;
    }
    case ssDiamond:
    {
      QPointF lineArray[4] = {QPointF(x-w,   y),
                              QPointF(  x, y-w),
                              QPointF(x+w,   y),
                              QPointF(  x, y+w)};
      painter->drawPolygon(lineArray, 4);
      break;
    }
    case ssStar:
    {
      painter->drawLine(QLineF(x-w,   y, x+w,   y));
      painter->drawLine(QLineF(  x, y+w,   x, y-w));
      painter->drawLine(QLineF(x-w*0.707, y-w*0.707, x+w*0.707, y+w*0.707));
      painter->drawLine(QLineF(x-w*0.707, y+w*0.707, x+w*0.707, y-w*0.707));
      break;
    }
    case ssTriangle:
    {
      QPointF lineArray[3] = {QPointF(x-w, y+0.755*w),
                              QPointF(x+w, y+0.755*w),
                              QPointF(  x, y-0.977*w)};
      painter->drawPolygon(lineArray, 3);
      break;
    }
    case ssTriangleInverted:
    {
      QPointF lineArray[3] = {QPointF(x-w, y-0.755*w),
                              QPointF(x+w, y-0.755*w),
                              QPointF(  x, y+0.977*w)};
      painter->drawPolygon(lineArray, 3);
      break;
    }
    case ssCrossSquare:
    {
      painter->drawRect(QRectF(x-w, y-w, mSize, mSize));
      painter->drawLine(QLineF(x-w, y-w, x+w*0.95, y+w*0.95));
      painter->drawLine(QLineF(x-w, y+w*0.95, x+w*0.95, y-w));
      break;
    }
    case ssPlusSquare:
    {
      painter->drawRect(QRectF(x-w, y-w, mSize, mSize));
      painter->drawLine(QLineF(x-w,   y, x+w*0.95,   y));
      painter->drawLine(QLineF(  x, y+w,        x, y-w));
      break;
    }
    case ssCrossCircle:
    {
      painter->drawEllipse(QPointF(x, y), w, w);
      painter->drawLine(QLineF(x-w*0.707, y-w*0.707, x+w*0.670, y+w*0.670));
      painter->drawLine(QLineF(x-w*0.707, y+w*0.670, x+w*0.670, y-w*0.707));
      break;
    }
    case ssPlusCircle:
    {
      painter->drawEllipse(QPointF(x, y), w, w);
      painter->drawLine(QLineF(x-w,   y, x+w,   y));
      painter->drawLine(QLineF(  x, y+w,   x, y-w));
      break;
    }
    case ssPeace:
    {
      painter->drawEllipse(QPointF(x, y), w, w);
      painter->drawLine(QLineF(x, y-w,         x,       y+w));
      painter->drawLine(QLineF(x,   y, x-w*0.707, y+w*0.707));
      painter->drawLine(QLineF(x,   y, x+w*0.707, y+w*0.707));
      break;
    }
    case ssPixmap:
    {
      const double widthHalf = mPixmap.width()*0.5;
      const double heightHalf = mPixmap.height()*0.5;
#if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
      const QRectF clipRect = painter->clipRegion().boundingRect().adjusted(-widthHalf, -heightHalf, widthHalf, heightHalf);
#else
      const QRectF clipRect = painter->clipBoundingRect().adjusted(-widthHalf, -heightHalf, widthHalf, heightHalf);
#endif
      if (clipRect.contains(x, y))
        painter->drawPixmap(qRound(x-widthHalf), qRound(y-heightHalf), mPixmap);
      break;
    }
    case ssCustom:
    {
      QTransform oldTransform = painter->transform();
      painter->translate(x, y);
      painter->scale(mSize/6.0, mSize/6.0);
      painter->drawPath(mCustomPath);
      painter->setTransform(oldTransform);
      break;
    }
  }
}
