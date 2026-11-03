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

#include "plottable.h"

#include "painter.h"
#include "core.h"
#include "axis/axis.h"
#include "layoutelements/layoutelement-axisrect.h"
#include "layoutelements/layoutelement-legend.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPSelectionDecorator
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPSelectionDecorator
  \brief Controls how a plottable's data selection is drawn
  
  Each \ref QCPAbstractPlottable instance has one \ref QCPSelectionDecorator (accessible via \ref
  QCPAbstractPlottable::selectionDecorator) and uses it when drawing selected segments of its data.
  
  The selection decorator controls both pen (\ref setPen) and brush (\ref setBrush), as well as the
  scatter style (\ref setScatterStyle) if the plottable draws scatters. Since a \ref
  QCPScatterStyle is itself composed of different properties such as color shape and size, the
  decorator allows specifying exactly which of those properties shall be used for the selected data
  point, via \ref setUsedScatterProperties.
  
  A \ref QCPSelectionDecorator subclass instance can be passed to a plottable via \ref
  QCPAbstractPlottable::setSelectionDecorator, allowing greater customizability of the appearance
  of selected segments.
  
  Use \ref copyFrom to easily transfer the settings of one decorator to another one. This is
  especially useful since plottables take ownership of the passed selection decorator, and thus the
  same decorator instance can not be passed to multiple plottables.
  
  Selection decorators can also themselves perform drawing operations by reimplementing \ref
  drawDecoration, which is called by the plottable's draw method. The base class \ref
  QCPSelectionDecorator does not make use of this however. For example, \ref
  QCPSelectionDecoratorBracket draws brackets around selected data segments.
*/

/*!
  Creates a new QCPSelectionDecorator instance with default values
*/
QCPSelectionDecorator::QCPSelectionDecorator() :
  mPen(QColor(80, 80, 255), 2.5),
  mBrush(Qt::NoBrush),
  mUsedScatterProperties(QCPScatterStyle::spNone),
  mPlottable(nullptr)
{
}

QCPSelectionDecorator::~QCPSelectionDecorator()
{
}

/*!
  Sets the pen that will be used by the parent plottable to draw selected data segments.
*/
void QCPSelectionDecorator::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  Sets the brush that will be used by the parent plottable to draw selected data segments.
*/
void QCPSelectionDecorator::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  Sets the scatter style that will be used by the parent plottable to draw scatters in selected
  data segments.
  
  \a usedProperties specifies which parts of the passed \a scatterStyle will be used by the
  plottable. The used properties can also be changed via \ref setUsedScatterProperties.
*/
void QCPSelectionDecorator::setScatterStyle(const QCPScatterStyle &scatterStyle, QCPScatterStyle::ScatterProperties usedProperties)
{
  mScatterStyle = scatterStyle;
  setUsedScatterProperties(usedProperties);
}

/*!
  Use this method to define which properties of the scatter style (set via \ref setScatterStyle)
  will be used for selected data segments. All properties of the scatter style that are not
  specified in \a properties will remain as specified in the plottable's original scatter style.
  
  \see QCPScatterStyle::ScatterProperty
*/
void QCPSelectionDecorator::setUsedScatterProperties(const QCPScatterStyle::ScatterProperties &properties)
{
  mUsedScatterProperties = properties;
}

/*!
  Sets the pen of \a painter to the pen of this selection decorator.
  
  \see applyBrush, getFinalScatterStyle
*/
void QCPSelectionDecorator::applyPen(QCPPainter *painter) const
{
  painter->setPen(mPen);
}

/*!
  Sets the brush of \a painter to the brush of this selection decorator.
  
  \see applyPen, getFinalScatterStyle
*/
void QCPSelectionDecorator::applyBrush(QCPPainter *painter) const
{
  painter->setBrush(mBrush);
}

/*!
  Returns the scatter style that the parent plottable shall use for selected scatter points. The
  plottable's original (unselected) scatter style must be passed as \a unselectedStyle. Depending
  on the setting of \ref setUsedScatterProperties, the returned scatter style is a mixture of this
  selecion decorator's scatter style (\ref setScatterStyle), and \a unselectedStyle.
  
  \see applyPen, applyBrush, setScatterStyle
*/
QCPScatterStyle QCPSelectionDecorator::getFinalScatterStyle(const QCPScatterStyle &unselectedStyle) const
{
  QCPScatterStyle result(unselectedStyle);
  result.setFromOther(mScatterStyle, mUsedScatterProperties);
  
  // if style shall inherit pen from plottable (has no own pen defined), give it the selected
  // plottable pen explicitly, so it doesn't use the unselected plottable pen when used in the
  // plottable:
  if (!result.isPenDefined())
    result.setPen(mPen);
  
  return result;
}

/*!
  Copies all properties (e.g. color, fill, scatter style) of the \a other selection decorator to
  this selection decorator.
*/
void QCPSelectionDecorator::copyFrom(const QCPSelectionDecorator *other)
{
  setPen(other->pen());
  setBrush(other->brush());
  setScatterStyle(other->scatterStyle(), other->usedScatterProperties());
}

/*!
  This method is called by all plottables' draw methods to allow custom selection decorations to be
  drawn. Use the passed \a painter to perform the drawing operations. \a selection carries the data
  selection for which the decoration shall be drawn.
  
  The default base class implementation of \ref QCPSelectionDecorator has no special decoration, so
  this method does nothing.
*/
void QCPSelectionDecorator::drawDecoration(QCPPainter *painter, QCPDataSelection selection)
{
  Q_UNUSED(painter)
  Q_UNUSED(selection)
}

/*! \internal
  
  This method is called as soon as a selection decorator is associated with a plottable, by a call
  to \ref QCPAbstractPlottable::setSelectionDecorator. This way the selection decorator can obtain a pointer to the plottable that uses it (e.g. to access
  data points via the \ref QCPAbstractPlottable::interface1D interface).
  
  If the selection decorator was already added to a different plottable before, this method aborts
  the registration and returns false.
*/
bool QCPSelectionDecorator::registerWithPlottable(QCPAbstractPlottable *plottable)
{
  if (!mPlottable)
  {
    mPlottable = plottable;
    return true;
  } else
  {
    qDebug() << Q_FUNC_INFO << "This selection decorator is already registered with plottable:" << reinterpret_cast<quintptr>(mPlottable);
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAbstractPlottable
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPAbstractPlottable
  \brief The abstract base class for all data representing objects in a plot.

  It defines a very basic interface like name, pen, brush, visibility etc. Since this class is
  abstract, it can't be instantiated. Use one of the subclasses or create a subclass yourself to
  create new ways of displaying data (see "Creating own plottables" below). Plottables that display
  one-dimensional data (i.e. data points have a single key dimension and one or multiple values at
  each key) are based off of the template subclass \ref QCPAbstractPlottable1D, see details
  there.
  
  All further specifics are in the subclasses, for example:
  \li A normal graph with possibly a line and/or scatter points \ref QCPGraph
  (typically created with \ref QCustomPlot::addGraph)
  \li A parametric curve: \ref QCPCurve
  \li A bar chart: \ref QCPBars
  \li A statistical box plot: \ref QCPStatisticalBox
  \li A color encoded two-dimensional map: \ref QCPColorMap
  \li An OHLC/Candlestick chart: \ref QCPFinancial
  
  \section plottables-subclassing Creating own plottables
  
  Subclassing directly from QCPAbstractPlottable is only recommended if you wish to display
  two-dimensional data like \ref QCPColorMap, i.e. two logical key dimensions and one (or more)
  data dimensions. If you want to display data with only one logical key dimension, you should
  rather derive from \ref QCPAbstractPlottable1D.
  
  If subclassing QCPAbstractPlottable directly, these are the pure virtual functions you must
  implement:
  \li \ref selectTest
  \li \ref draw
  \li \ref drawLegendIcon
  \li \ref getKeyRange
  \li \ref getValueRange
  
  See the documentation of those functions for what they need to do.
  
  For drawing your plot, you can use the \ref coordsToPixels functions to translate a point in plot
  coordinates to pixel coordinates. This function is quite convenient, because it takes the
  orientation of the key and value axes into account for you (x and y are swapped when the key axis
  is vertical and the value axis horizontal). If you are worried about performance (i.e. you need
  to translate many points in a loop like QCPGraph), you can directly use \ref
  QCPAxis::coordToPixel. However, you must then take care about the orientation of the axis
  yourself.
  
  Here are some important members you inherit from QCPAbstractPlottable:
  <table>
  <tr>
    <td>QCustomPlot *\b mParentPlot</td>
    <td>A pointer to the parent QCustomPlot instance. The parent plot is inferred from the axes that are passed in the constructor.</td>
  </tr><tr>
    <td>QString \b mName</td>
    <td>The name of the plottable.</td>
  </tr><tr>
    <td>QPen \b mPen</td>
    <td>The generic pen of the plottable. You should use this pen for the most prominent data representing lines in the plottable
        (e.g QCPGraph uses this pen for its graph lines and scatters)</td>
  </tr><tr>
    <td>QBrush \b mBrush</td>
    <td>The generic brush of the plottable. You should use this brush for the most prominent fillable structures in the plottable
        (e.g. QCPGraph uses this brush to control filling under the graph)</td>
  </tr><tr>
    <td>QPointer<\ref QCPAxis> \b mKeyAxis, \b mValueAxis</td>
    <td>The key and value axes this plottable is attached to. Call their QCPAxis::coordToPixel functions to translate coordinates
        to pixels in either the key or value dimension. Make sure to check whether the pointer is \c nullptr before using it. If one of
        the axes is null, don't draw the plottable.</td>
  </tr><tr>
    <td>\ref QCPSelectionDecorator \b mSelectionDecorator</td>
    <td>The currently set selection decorator which specifies how selected data of the plottable shall be drawn and decorated.
        When drawing your data, you must consult this decorator for the appropriate pen/brush before drawing unselected/selected data segments.
        Finally, you should call its \ref QCPSelectionDecorator::drawDecoration method at the end of your \ref draw implementation.</td>
  </tr><tr>
    <td>\ref QCP::SelectionType \b mSelectable</td>
    <td>In which composition, if at all, this plottable's data may be selected. Enforcing this setting on the data selection is done
        by QCPAbstractPlottable automatically.</td>
  </tr><tr>
    <td>\ref QCPDataSelection \b mSelection</td>
    <td>Holds the current selection state of the plottable's data, i.e. the selected data ranges (\ref QCPDataRange).</td>
  </tr>
  </table>
*/

/* start of documentation of inline functions */

/*! \fn QCPSelectionDecorator *QCPAbstractPlottable::selectionDecorator() const
  
  Provides access to the selection decorator of this plottable. The selection decorator controls
  how selected data ranges are drawn (e.g. their pen color and fill), see \ref
  QCPSelectionDecorator for details.
  
  If you wish to use an own \ref QCPSelectionDecorator subclass, pass an instance of it to \ref
  setSelectionDecorator.
*/

/*! \fn bool QCPAbstractPlottable::selected() const
  
  Returns true if there are any data points of the plottable currently selected. Use \ref selection
  to retrieve the current \ref QCPDataSelection.
*/

/*! \fn QCPDataSelection QCPAbstractPlottable::selection() const
  
  Returns a \ref QCPDataSelection encompassing all the data points that are currently selected on
  this plottable.
  
  \see selected, setSelection, setSelectable
*/

/*! \fn virtual QCPPlottableInterface1D *QCPAbstractPlottable::interface1D()
  
  If this plottable is a one-dimensional plottable, i.e. it implements the \ref
  QCPPlottableInterface1D, returns the \a this pointer with that type. Otherwise (e.g. in the case
  of a \ref QCPColorMap) returns zero.
  
  You can use this method to gain read access to data coordinates while holding a pointer to the
  abstract base class only.
*/

/* end of documentation of inline functions */
/* start of documentation of pure virtual functions */

/*! \fn void QCPAbstractPlottable::drawLegendIcon(QCPPainter *painter, const QRect &rect) const = 0
  \internal
  
  called by QCPLegend::draw (via QCPPlottableLegendItem::draw) to create a graphical representation
  of this plottable inside \a rect, next to the plottable name.
  
  The passed \a painter has its cliprect set to \a rect, so painting outside of \a rect won't
  appear outside the legend icon border.
*/

/*! \fn QCPRange QCPAbstractPlottable::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const = 0
  
  Returns the coordinate range that all data in this plottable span in the key axis dimension. For
  logarithmic plots, one can set \a inSignDomain to either \ref QCP::sdNegative or \ref
  QCP::sdPositive in order to restrict the returned range to that sign domain. E.g. when only
  negative range is wanted, set \a inSignDomain to \ref QCP::sdNegative and all positive points
  will be ignored for range calculation. For no restriction, just set \a inSignDomain to \ref
  QCP::sdBoth (default). \a foundRange is an output parameter that indicates whether a range could
  be found or not. If this is false, you shouldn't use the returned range (e.g. no points in data).

  Note that \a foundRange is not the same as \ref QCPRange::validRange, since the range returned by
  this function may have size zero (e.g. when there is only one data point). In this case \a
  foundRange would return true, but the returned range is not a valid range in terms of \ref
  QCPRange::validRange.
  
  \see rescaleAxes, getValueRange
*/

/*! \fn QCPRange QCPAbstractPlottable::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain, const QCPRange &inKeyRange) const = 0
  
  Returns the coordinate range that the data points in the specified key range (\a inKeyRange) span
  in the value axis dimension. For logarithmic plots, one can set \a inSignDomain to either \ref
  QCP::sdNegative or \ref QCP::sdPositive in order to restrict the returned range to that sign
  domain. E.g. when only negative range is wanted, set \a inSignDomain to \ref QCP::sdNegative and
  all positive points will be ignored for range calculation. For no restriction, just set \a
  inSignDomain to \ref QCP::sdBoth (default). \a foundRange is an output parameter that indicates
  whether a range could be found or not. If this is false, you shouldn't use the returned range
  (e.g. no points in data).
  
  If \a inKeyRange has both lower and upper bound set to zero (is equal to <tt>QCPRange()</tt>),
  all data points are considered, without any restriction on the keys.

  Note that \a foundRange is not the same as \ref QCPRange::validRange, since the range returned by
  this function may have size zero (e.g. when there is only one data point). In this case \a
  foundRange would return true, but the returned range is not a valid range in terms of \ref
  QCPRange::validRange.
  
  \see rescaleAxes, getKeyRange
*/

/* end of documentation of pure virtual functions */
/* start of documentation of signals */

/*! \fn void QCPAbstractPlottable::selectionChanged(bool selected)
  
  This signal is emitted when the selection state of this plottable has changed, either by user
  interaction or by a direct call to \ref setSelection. The parameter \a selected indicates whether
  there are any points selected or not.
  
  \see selectionChanged(const QCPDataSelection &selection)
*/

/*! \fn void QCPAbstractPlottable::selectionChanged(const QCPDataSelection &selection)
  
  This signal is emitted when the selection state of this plottable has changed, either by user
  interaction or by a direct call to \ref setSelection. The parameter \a selection holds the
  currently selected data ranges.
  
  \see selectionChanged(bool selected)
*/

/*! \fn void QCPAbstractPlottable::selectableChanged(QCP::SelectionType selectable);
  
  This signal is emitted when the selectability of this plottable has changed.
  
  \see setSelectable
*/

/* end of documentation of signals */

/*!
  Constructs an abstract plottable which uses \a keyAxis as its key axis ("x") and \a valueAxis as
  its value axis ("y"). \a keyAxis and \a valueAxis must reside in the same QCustomPlot instance
  and have perpendicular orientations. If either of these restrictions is violated, a corresponding
  message is printed to the debug output (qDebug), the construction is not aborted, though.
  
  Since QCPAbstractPlottable is an abstract class that defines the basic interface to plottables,
  it can't be directly instantiated.
  
  You probably want one of the subclasses like \ref QCPGraph or \ref QCPCurve instead.
*/
QCPAbstractPlottable::QCPAbstractPlottable(QCPAxis *keyAxis, QCPAxis *valueAxis) :
  QCPLayerable(keyAxis->parentPlot(), QString(), keyAxis->axisRect()),
  mName(),
  mAntialiasedFill(true),
  mAntialiasedScatters(true),
  mPen(Qt::black),
  mBrush(Qt::NoBrush),
  mKeyAxis(keyAxis),
  mValueAxis(valueAxis),
  mSelectable(QCP::stWhole),
  mSelectionDecorator(nullptr)
{
  if (keyAxis->parentPlot() != valueAxis->parentPlot())
    qDebug() << Q_FUNC_INFO << "Parent plot of keyAxis is not the same as that of valueAxis.";
  if (keyAxis->orientation() == valueAxis->orientation())
    qDebug() << Q_FUNC_INFO << "keyAxis and valueAxis must be orthogonal to each other.";
  
  mParentPlot->registerPlottable(this);
  setSelectionDecorator(new QCPSelectionDecorator);
}

QCPAbstractPlottable::~QCPAbstractPlottable()
{
  if (mSelectionDecorator)
  {
    delete mSelectionDecorator;
    mSelectionDecorator = nullptr;
  }
}

/*!
   The name is the textual representation of this plottable as it is displayed in the legend
   (\ref QCPLegend). It may contain any UTF-8 characters, including newlines.
*/
void QCPAbstractPlottable::setName(const QString &name)
{
  mName = name;
}

/*!
  Sets whether fills of this plottable are drawn antialiased or not.
  
  Note that this setting may be overridden by \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
*/
void QCPAbstractPlottable::setAntialiasedFill(bool enabled)
{
  mAntialiasedFill = enabled;
}

/*!
  Sets whether the scatter symbols of this plottable are drawn antialiased or not.
  
  Note that this setting may be overridden by \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
*/
void QCPAbstractPlottable::setAntialiasedScatters(bool enabled)
{
  mAntialiasedScatters = enabled;
}

/*!
  The pen is used to draw basic lines that make up the plottable representation in the
  plot.
  
  For example, the \ref QCPGraph subclass draws its graph lines with this pen.

  \see setBrush
*/
void QCPAbstractPlottable::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  The brush is used to draw basic fills of the plottable representation in the
  plot. The Fill can be a color, gradient or texture, see the usage of QBrush.
  
  For example, the \ref QCPGraph subclass draws the fill under the graph with this brush, when
  it's not set to Qt::NoBrush.

  \see setPen
*/
void QCPAbstractPlottable::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  The key axis of a plottable can be set to any axis of a QCustomPlot, as long as it is orthogonal
  to the plottable's value axis. This function performs no checks to make sure this is the case.
  The typical mathematical choice is to use the x-axis (QCustomPlot::xAxis) as key axis and the
  y-axis (QCustomPlot::yAxis) as value axis.
  
  Normally, the key and value axes are set in the constructor of the plottable (or \ref
  QCustomPlot::addGraph when working with QCPGraphs through the dedicated graph interface).

  \see setValueAxis
*/
void QCPAbstractPlottable::setKeyAxis(QCPAxis *axis)
{
  mKeyAxis = axis;
}

/*!
  The value axis of a plottable can be set to any axis of a QCustomPlot, as long as it is
  orthogonal to the plottable's key axis. This function performs no checks to make sure this is the
  case. The typical mathematical choice is to use the x-axis (QCustomPlot::xAxis) as key axis and
  the y-axis (QCustomPlot::yAxis) as value axis.

  Normally, the key and value axes are set in the constructor of the plottable (or \ref
  QCustomPlot::addGraph when working with QCPGraphs through the dedicated graph interface).
  
  \see setKeyAxis
*/
void QCPAbstractPlottable::setValueAxis(QCPAxis *axis)
{
  mValueAxis = axis;
}


/*!
  Sets which data ranges of this plottable are selected. Selected data ranges are drawn differently
  (e.g. color) in the plot. This can be controlled via the selection decorator (see \ref
  selectionDecorator).
  
  The entire selection mechanism for plottables is handled automatically when \ref
  QCustomPlot::setInteractions contains iSelectPlottables. You only need to call this function when
  you wish to change the selection state programmatically.
  
  Using \ref setSelectable you can further specify for each plottable whether and to which
  granularity it is selectable. If \a selection is not compatible with the current \ref
  QCP::SelectionType set via \ref setSelectable, the resulting selection will be adjusted
  accordingly (see \ref QCPDataSelection::enforceType).
  
  emits the \ref selectionChanged signal when \a selected is different from the previous selection state.
  
  \see setSelectable, selectTest
*/
void QCPAbstractPlottable::setSelection(QCPDataSelection selection)
{
  selection.enforceType(mSelectable);
  if (mSelection != selection)
  {
    mSelection = selection;
    emit selectionChanged(selected());
    emit selectionChanged(mSelection);
  }
}

/*!
  Use this method to set an own QCPSelectionDecorator (subclass) instance. This allows you to
  customize the visual representation of selected data ranges further than by using the default
  QCPSelectionDecorator.
  
  The plottable takes ownership of the \a decorator.
  
  The currently set decorator can be accessed via \ref selectionDecorator.
*/
void QCPAbstractPlottable::setSelectionDecorator(QCPSelectionDecorator *decorator)
{
  if (decorator)
  {
    if (decorator->registerWithPlottable(this))
    {
      delete mSelectionDecorator; // delete old decorator if necessary
      mSelectionDecorator = decorator;
    }
  } else if (mSelectionDecorator) // just clear decorator
  {
    delete mSelectionDecorator;
    mSelectionDecorator = nullptr;
  }
}

/*!
  Sets whether and to which granularity this plottable can be selected.

  A selection can happen by clicking on the QCustomPlot surface (When \ref
  QCustomPlot::setInteractions contains \ref QCP::iSelectPlottables), by dragging a selection rect
  (When \ref QCustomPlot::setSelectionRectMode is \ref QCP::srmSelect), or programmatically by
  calling \ref setSelection.
  
  \see setSelection, QCP::SelectionType
*/
void QCPAbstractPlottable::setSelectable(QCP::SelectionType selectable)
{
  if (mSelectable != selectable)
  {
    mSelectable = selectable;
    QCPDataSelection oldSelection = mSelection;
    mSelection.enforceType(mSelectable);
    emit selectableChanged(mSelectable);
    if (mSelection != oldSelection)
    {
      emit selectionChanged(selected());
      emit selectionChanged(mSelection);
    }
  }
}


/*!
  Convenience function for transforming a key/value pair to pixels on the QCustomPlot surface,
  taking the orientations of the axes associated with this plottable into account (e.g. whether key
  represents x or y).

  \a key and \a value are transformed to the coodinates in pixels and are written to \a x and \a y.

  \see pixelsToCoords, QCPAxis::coordToPixel
*/
void QCPAbstractPlottable::coordsToPixels(double key, double value, double &x, double &y) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  if (keyAxis->orientation() == Qt::Horizontal)
  {
    x = keyAxis->coordToPixel(key);
    y = valueAxis->coordToPixel(value);
  } else
  {
    y = keyAxis->coordToPixel(key);
    x = valueAxis->coordToPixel(value);
  }
}

/*! \overload

  Transforms the given \a key and \a value to pixel coordinates and returns them in a QPointF.
*/
const QPointF QCPAbstractPlottable::coordsToPixels(double key, double value) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return QPointF(); }
  
  if (keyAxis->orientation() == Qt::Horizontal)
    return QPointF(keyAxis->coordToPixel(key), valueAxis->coordToPixel(value));
  else
    return QPointF(valueAxis->coordToPixel(value), keyAxis->coordToPixel(key));
}

/*!
  Convenience function for transforming a x/y pixel pair on the QCustomPlot surface to plot coordinates,
  taking the orientations of the axes associated with this plottable into account (e.g. whether key
  represents x or y).

  \a x and \a y are transformed to the plot coodinates and are written to \a key and \a value.

  \see coordsToPixels, QCPAxis::coordToPixel
*/
void QCPAbstractPlottable::pixelsToCoords(double x, double y, double &key, double &value) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  if (keyAxis->orientation() == Qt::Horizontal)
  {
    key = keyAxis->pixelToCoord(x);
    value = valueAxis->pixelToCoord(y);
  } else
  {
    key = keyAxis->pixelToCoord(y);
    value = valueAxis->pixelToCoord(x);
  }
}

/*! \overload

  Returns the pixel input \a pixelPos as plot coordinates \a key and \a value.
*/
void QCPAbstractPlottable::pixelsToCoords(const QPointF &pixelPos, double &key, double &value) const
{
  pixelsToCoords(pixelPos.x(), pixelPos.y(), key, value);
}

/*!
  Rescales the key and value axes associated with this plottable to contain all displayed data, so
  the whole plottable is visible. If the scaling of an axis is logarithmic, rescaleAxes will make
  sure not to rescale to an illegal range i.e. a range containing different signs and/or zero.
  Instead it will stay in the current sign domain and ignore all parts of the plottable that lie
  outside of that domain.
  
  \a onlyEnlarge makes sure the ranges are only expanded, never reduced. So it's possible to show
  multiple plottables in their entirety by multiple calls to rescaleAxes where the first call has
  \a onlyEnlarge set to false (the default), and all subsequent set to true.
  
  \see rescaleKeyAxis, rescaleValueAxis, QCustomPlot::rescaleAxes, QCPAxis::rescale
*/
void QCPAbstractPlottable::rescaleAxes(bool onlyEnlarge) const
{
  rescaleKeyAxis(onlyEnlarge);
  rescaleValueAxis(onlyEnlarge);
}

/*!
  Rescales the key axis of the plottable so the whole plottable is visible.
  
  See \ref rescaleAxes for detailed behaviour.
*/
void QCPAbstractPlottable::rescaleKeyAxis(bool onlyEnlarge) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  if (!keyAxis) { qDebug() << Q_FUNC_INFO << "invalid key axis"; return; }
  
  QCP::SignDomain signDomain = QCP::sdBoth;
  if (keyAxis->scaleType() == QCPAxis::stLogarithmic)
    signDomain = (keyAxis->range().upper < 0 ? QCP::sdNegative : QCP::sdPositive);
  
  bool foundRange;
  QCPRange newRange = getKeyRange(foundRange, signDomain);
  if (foundRange)
  {
    if (onlyEnlarge)
      newRange.expand(keyAxis->range());
    if (!QCPRange::validRange(newRange)) // likely due to range being zero (plottable has only constant data in this axis dimension), shift current range to at least center the plottable
    {
      double center = (newRange.lower+newRange.upper)*0.5; // upper and lower should be equal anyway, but just to make sure, incase validRange returned false for other reason
      if (keyAxis->scaleType() == QCPAxis::stLinear)
      {
        newRange.lower = center-keyAxis->range().size()/2.0;
        newRange.upper = center+keyAxis->range().size()/2.0;
      } else // scaleType() == stLogarithmic
      {
        newRange.lower = center/qSqrt(keyAxis->range().upper/keyAxis->range().lower);
        newRange.upper = center*qSqrt(keyAxis->range().upper/keyAxis->range().lower);
      }
    }
    keyAxis->setRange(newRange);
  }
}

/*!
  Rescales the value axis of the plottable so the whole plottable is visible. If \a inKeyRange is
  set to true, only the data points which are in the currently visible key axis range are
  considered.

  Returns true if the axis was actually scaled. This might not be the case if this plottable has an
  invalid range, e.g. because it has no data points.

  See \ref rescaleAxes for detailed behaviour.
*/
void QCPAbstractPlottable::rescaleValueAxis(bool onlyEnlarge, bool inKeyRange) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  QCP::SignDomain signDomain = QCP::sdBoth;
  if (valueAxis->scaleType() == QCPAxis::stLogarithmic)
    signDomain = (valueAxis->range().upper < 0 ? QCP::sdNegative : QCP::sdPositive);
  
  bool foundRange;
  QCPRange newRange = getValueRange(foundRange, signDomain, inKeyRange ? keyAxis->range() : QCPRange());
  if (foundRange)
  {
    if (onlyEnlarge)
      newRange.expand(valueAxis->range());
    if (!QCPRange::validRange(newRange)) // likely due to range being zero (plottable has only constant data in this axis dimension), shift current range to at least center the plottable
    {
      double center = (newRange.lower+newRange.upper)*0.5; // upper and lower should be equal anyway, but just to make sure, incase validRange returned false for other reason
      if (valueAxis->scaleType() == QCPAxis::stLinear)
      {
        newRange.lower = center-valueAxis->range().size()/2.0;
        newRange.upper = center+valueAxis->range().size()/2.0;
      } else // scaleType() == stLogarithmic
      {
        newRange.lower = center/qSqrt(valueAxis->range().upper/valueAxis->range().lower);
        newRange.upper = center*qSqrt(valueAxis->range().upper/valueAxis->range().lower);
      }
    }
    valueAxis->setRange(newRange);
  }
}

/*! \overload

  Adds this plottable to the specified \a legend.

  Creates a QCPPlottableLegendItem which is inserted into the legend. Returns true on success, i.e.
  when the legend exists and a legend item associated with this plottable isn't already in the
  legend.

  If the plottable needs a more specialized representation in the legend, you can create a
  corresponding subclass of \ref QCPPlottableLegendItem and add it to the legend manually instead
  of calling this method.

  \see removeFromLegend, QCPLegend::addItem
*/
bool QCPAbstractPlottable::addToLegend(QCPLegend *legend)
{
  if (!legend)
  {
    qDebug() << Q_FUNC_INFO << "passed legend is null";
    return false;
  }
  if (legend->parentPlot() != mParentPlot)
  {
    qDebug() << Q_FUNC_INFO << "passed legend isn't in the same QCustomPlot as this plottable";
    return false;
  }
  
  if (!legend->hasItemWithPlottable(this))
  {
    legend->addItem(new QCPPlottableLegendItem(legend, this));
    return true;
  } else
    return false;
}

/*! \overload

  Adds this plottable to the legend of the parent QCustomPlot (\ref QCustomPlot::legend).

  \see removeFromLegend
*/
bool QCPAbstractPlottable::addToLegend()
{
  if (!mParentPlot || !mParentPlot->legend)
    return false;
  else
    return addToLegend(mParentPlot->legend);
}

/*! \overload

  Removes the plottable from the specifed \a legend. This means the \ref QCPPlottableLegendItem
  that is associated with this plottable is removed.

  Returns true on success, i.e. if the legend exists and a legend item associated with this
  plottable was found and removed.

  \see addToLegend, QCPLegend::removeItem
*/
bool QCPAbstractPlottable::removeFromLegend(QCPLegend *legend) const
{
  if (!legend)
  {
    qDebug() << Q_FUNC_INFO << "passed legend is null";
    return false;
  }
  
  if (QCPPlottableLegendItem *lip = legend->itemWithPlottable(this))
    return legend->removeItem(lip);
  else
    return false;
}

/*! \overload

  Removes the plottable from the legend of the parent QCustomPlot.

  \see addToLegend
*/
bool QCPAbstractPlottable::removeFromLegend() const
{
  if (!mParentPlot || !mParentPlot->legend)
    return false;
  else
    return removeFromLegend(mParentPlot->legend);
}

/* inherits documentation from base class */
QRect QCPAbstractPlottable::clipRect() const
{
  if (mKeyAxis && mValueAxis)
    return mKeyAxis.data()->axisRect()->rect() & mValueAxis.data()->axisRect()->rect();
  else
    return {};
}

/* inherits documentation from base class */
QCP::Interaction QCPAbstractPlottable::selectionCategory() const
{
  return QCP::iSelectPlottables;
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing plottable lines.

  This is the antialiasing state the painter passed to the \ref draw method is in by default.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \seebaseclassmethod
  
  \see setAntialiased, applyFillAntialiasingHint, applyScattersAntialiasingHint
*/
void QCPAbstractPlottable::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aePlottables);
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing plottable fills.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased, applyDefaultAntialiasingHint, applyScattersAntialiasingHint
*/
void QCPAbstractPlottable::applyFillAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiasedFill, QCP::aeFills);
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing plottable scatter points.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased, applyFillAntialiasingHint, applyDefaultAntialiasingHint
*/
void QCPAbstractPlottable::applyScattersAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiasedScatters, QCP::aeScatters);
}

/* inherits documentation from base class */
void QCPAbstractPlottable::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  
  if (mSelectable != QCP::stNone)
  {
    QCPDataSelection newSelection = details.value<QCPDataSelection>();
    QCPDataSelection selectionBefore = mSelection;
    if (additive)
    {
      if (mSelectable == QCP::stWhole) // in whole selection mode, we toggle to no selection even if currently unselected point was hit
      {
        if (selected())
          setSelection(QCPDataSelection());
        else
          setSelection(newSelection);
      } else // in all other selection modes we toggle selections of homogeneously selected/unselected segments
      {
        if (mSelection.contains(newSelection)) // if entire newSelection is already selected, toggle selection
          setSelection(mSelection-newSelection);
        else
          setSelection(mSelection+newSelection);
      }
    } else
      setSelection(newSelection);
    if (selectionStateChanged)
      *selectionStateChanged = mSelection != selectionBefore;
  }
}

/* inherits documentation from base class */
void QCPAbstractPlottable::deselectEvent(bool *selectionStateChanged)
{
  if (mSelectable != QCP::stNone)
  {
    QCPDataSelection selectionBefore = mSelection;
    setSelection(QCPDataSelection());
    if (selectionStateChanged)
      *selectionStateChanged = mSelection != selectionBefore;
  }
}
