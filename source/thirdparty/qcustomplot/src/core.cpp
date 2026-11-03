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

/*! \file */

#include "core.h"

#include "painter.h"
#include "layer.h"
#include "axis/axis.h"
#include "layoutelements/layoutelement-axisrect.h"
#include "layoutelements/layoutelement-legend.h"
#include "plottable.h"
#include "plottables/plottable-graph.h"
#include "item.h"
#include "selectionrect.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCustomPlot
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCustomPlot
  
  \brief The central class of the library. This is the QWidget which displays the plot and
  interacts with the user.
  
  For tutorials on how to use QCustomPlot, see the website\n
  https://www.qcustomplot.com/
*/

/* start of documentation of inline functions */

/*! \fn QCPSelectionRect *QCustomPlot::selectionRect() const
  
  Allows access to the currently used QCPSelectionRect instance (or subclass thereof), that is used
  to handle and draw selection rect interactions (see \ref setSelectionRectMode).
  
  \see setSelectionRect
*/

/*! \fn QCPLayoutGrid *QCustomPlot::plotLayout() const
  
  Returns the top level layout of this QCustomPlot instance. It is a \ref QCPLayoutGrid, initially containing just
  one cell with the main QCPAxisRect inside.
*/

/* end of documentation of inline functions */
/* start of documentation of signals */

/*! \fn void QCustomPlot::mouseDoubleClick(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse double click event.
*/

/*! \fn void QCustomPlot::mousePress(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse press event.
  
  It is emitted before QCustomPlot handles any other mechanism like range dragging. So a slot
  connected to this signal can still influence the behaviour e.g. with \ref QCPAxisRect::setRangeDrag or \ref
  QCPAxisRect::setRangeDragAxes.
*/

/*! \fn void QCustomPlot::mouseMove(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse move event.
  
  It is emitted before QCustomPlot handles any other mechanism like range dragging. So a slot
  connected to this signal can still influence the behaviour e.g. with \ref QCPAxisRect::setRangeDrag or \ref
  QCPAxisRect::setRangeDragAxes.
  
  \warning It is discouraged to change the drag-axes with \ref QCPAxisRect::setRangeDragAxes here,
  because the dragging starting point was saved the moment the mouse was pressed. Thus it only has
  a meaning for the range drag axes that were set at that moment. If you want to change the drag
  axes, consider doing this in the \ref mousePress signal instead.
*/

/*! \fn void QCustomPlot::mouseRelease(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse release event.
  
  It is emitted before QCustomPlot handles any other mechanisms like object selection. So a
  slot connected to this signal can still influence the behaviour e.g. with \ref setInteractions or
  \ref QCPAbstractPlottable::setSelectable.
*/

/*! \fn void QCustomPlot::mouseWheel(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse wheel event.
  
  It is emitted before QCustomPlot handles any other mechanisms like range zooming. So a slot
  connected to this signal can still influence the behaviour e.g. with \ref QCPAxisRect::setRangeZoom, \ref
  QCPAxisRect::setRangeZoomAxes or \ref QCPAxisRect::setRangeZoomFactor.
*/

/*! \fn void QCustomPlot::plottableClick(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event)

  This signal is emitted when a plottable is clicked.

  \a event is the mouse event that caused the click and \a plottable is the plottable that received
  the click. The parameter \a dataIndex indicates the data point that was closest to the click
  position.

  \see plottableDoubleClick
*/

/*! \fn void QCustomPlot::plottableDoubleClick(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event)

  This signal is emitted when a plottable is double clicked.

  \a event is the mouse event that caused the click and \a plottable is the plottable that received
  the click. The parameter \a dataIndex indicates the data point that was closest to the click
  position.

  \see plottableClick
*/

/*! \fn void QCustomPlot::itemClick(QCPAbstractItem *item, QMouseEvent *event)
  
  This signal is emitted when an item is clicked.

  \a event is the mouse event that caused the click and \a item is the item that received the
  click.
  
  \see itemDoubleClick
*/

/*! \fn void QCustomPlot::itemDoubleClick(QCPAbstractItem *item, QMouseEvent *event)
  
  This signal is emitted when an item is double clicked.
  
  \a event is the mouse event that caused the click and \a item is the item that received the
  click.
  
  \see itemClick
*/

/*! \fn void QCustomPlot::axisClick(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event)
  
  This signal is emitted when an axis is clicked.
  
  \a event is the mouse event that caused the click, \a axis is the axis that received the click and
  \a part indicates the part of the axis that was clicked.
  
  \see axisDoubleClick
*/

/*! \fn void QCustomPlot::axisDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event)

  This signal is emitted when an axis is double clicked.
  
  \a event is the mouse event that caused the click, \a axis is the axis that received the click and
  \a part indicates the part of the axis that was clicked.
  
  \see axisClick
*/

/*! \fn void QCustomPlot::legendClick(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)

  This signal is emitted when a legend (item) is clicked.
  
  \a event is the mouse event that caused the click, \a legend is the legend that received the
  click and \a item is the legend item that received the click. If only the legend and no item is
  clicked, \a item is \c nullptr. This happens for a click inside the legend padding or the space
  between two items.
  
  \see legendDoubleClick
*/

/*! \fn void QCustomPlot::legendDoubleClick(QCPLegend *legend,  QCPAbstractLegendItem *item, QMouseEvent *event)

  This signal is emitted when a legend (item) is double clicked.
  
  \a event is the mouse event that caused the click, \a legend is the legend that received the
  click and \a item is the legend item that received the click. If only the legend and no item is
  clicked, \a item is \c nullptr. This happens for a click inside the legend padding or the space
  between two items.
  
  \see legendClick
*/

/*! \fn void QCustomPlot::selectionChangedByUser()
  
  This signal is emitted after the user has changed the selection in the QCustomPlot, e.g. by
  clicking. It is not emitted when the selection state of an object has changed programmatically by
  a direct call to <tt>setSelected()</tt>/<tt>setSelection()</tt> on an object or by calling \ref
  deselectAll.
  
  In addition to this signal, selectable objects also provide individual signals, for example \ref
  QCPAxis::selectionChanged or \ref QCPAbstractPlottable::selectionChanged. Note that those signals
  are emitted even if the selection state is changed programmatically.
  
  See the documentation of \ref setInteractions for details about the selection mechanism.
  
  \see selectedPlottables, selectedGraphs, selectedItems, selectedAxes, selectedLegends
*/

/*! \fn void QCustomPlot::beforeReplot()
  
  This signal is emitted immediately before a replot takes place (caused by a call to the slot \ref
  replot).
  
  It is safe to mutually connect the replot slot with this signal on two QCustomPlots to make them
  replot synchronously, it won't cause an infinite recursion.
  
  \see replot, afterReplot, afterLayout
*/

/*! \fn void QCustomPlot::afterLayout()

  This signal is emitted immediately after the layout step has been completed, which occurs right
  before drawing the plot. This is typically during a call to \ref replot, and in such cases this
  signal is emitted in between the signals \ref beforeReplot and \ref afterReplot. Unlike those
  signals however, this signal is also emitted during off-screen painting, such as when calling
  \ref toPixmap or \ref savePdf.

  The layout step queries all layouts and layout elements in the plot for their proposed size and
  arranges the objects accordingly as preparation for the subsequent drawing step. Through this
  signal, you have the opportunity to update certain things in your plot that depend crucially on
  the exact dimensions/positioning of layout elements such as axes and axis rects.

  \warning However, changing any parameters of this QCustomPlot instance which would normally
  affect the layouting (e.g. axis range order of magnitudes, tick label sizes, etc.) will not issue
  a second run of the layout step. It will propagate directly to the draw step and may cause
  graphical inconsistencies such as overlapping objects, if sizes or positions have changed.

  \see updateLayout, beforeReplot, afterReplot
*/

/*! \fn void QCustomPlot::afterReplot()
  
  This signal is emitted immediately after a replot has taken place (caused by a call to the slot \ref
  replot).
  
  It is safe to mutually connect the replot slot with this signal on two QCustomPlots to make them
  replot synchronously, it won't cause an infinite recursion.
  
  \see replot, beforeReplot, afterLayout
*/

/* end of documentation of signals */
/* start of documentation of public members */

/*! \var QCPAxis *QCustomPlot::xAxis

  A pointer to the primary x Axis (bottom) of the main axis rect of the plot.
  
  QCustomPlot offers convenient pointers to the axes (\ref xAxis, \ref yAxis, \ref xAxis2, \ref
  yAxis2) and the \ref legend. They make it very easy working with plots that only have a single
  axis rect and at most one axis at each axis rect side. If you use \link thelayoutsystem the
  layout system\endlink to add multiple axis rects or multiple axes to one side, use the \ref
  QCPAxisRect::axis interface to access the new axes. If one of the four default axes or the
  default legend is removed due to manipulation of the layout system (e.g. by removing the main
  axis rect), the corresponding pointers become \c nullptr.
  
  If an axis convenience pointer is currently \c nullptr and a new axis rect or a corresponding
  axis is added in the place of the main axis rect, QCustomPlot resets the convenience pointers to
  the according new axes. Similarly the \ref legend convenience pointer will be reset if a legend
  is added after the main legend was removed before.
*/

/*! \var QCPAxis *QCustomPlot::yAxis

  A pointer to the primary y Axis (left) of the main axis rect of the plot.
  
  QCustomPlot offers convenient pointers to the axes (\ref xAxis, \ref yAxis, \ref xAxis2, \ref
  yAxis2) and the \ref legend. They make it very easy working with plots that only have a single
  axis rect and at most one axis at each axis rect side. If you use \link thelayoutsystem the
  layout system\endlink to add multiple axis rects or multiple axes to one side, use the \ref
  QCPAxisRect::axis interface to access the new axes. If one of the four default axes or the
  default legend is removed due to manipulation of the layout system (e.g. by removing the main
  axis rect), the corresponding pointers become \c nullptr.
  
  If an axis convenience pointer is currently \c nullptr and a new axis rect or a corresponding
  axis is added in the place of the main axis rect, QCustomPlot resets the convenience pointers to
  the according new axes. Similarly the \ref legend convenience pointer will be reset if a legend
  is added after the main legend was removed before.
*/

/*! \var QCPAxis *QCustomPlot::xAxis2

  A pointer to the secondary x Axis (top) of the main axis rect of the plot. Secondary axes are
  invisible by default. Use QCPAxis::setVisible to change this (or use \ref
  QCPAxisRect::setupFullAxesBox).
  
  QCustomPlot offers convenient pointers to the axes (\ref xAxis, \ref yAxis, \ref xAxis2, \ref
  yAxis2) and the \ref legend. They make it very easy working with plots that only have a single
  axis rect and at most one axis at each axis rect side. If you use \link thelayoutsystem the
  layout system\endlink to add multiple axis rects or multiple axes to one side, use the \ref
  QCPAxisRect::axis interface to access the new axes. If one of the four default axes or the
  default legend is removed due to manipulation of the layout system (e.g. by removing the main
  axis rect), the corresponding pointers become \c nullptr.
  
  If an axis convenience pointer is currently \c nullptr and a new axis rect or a corresponding
  axis is added in the place of the main axis rect, QCustomPlot resets the convenience pointers to
  the according new axes. Similarly the \ref legend convenience pointer will be reset if a legend
  is added after the main legend was removed before.
*/

/*! \var QCPAxis *QCustomPlot::yAxis2

  A pointer to the secondary y Axis (right) of the main axis rect of the plot. Secondary axes are
  invisible by default. Use QCPAxis::setVisible to change this (or use \ref
  QCPAxisRect::setupFullAxesBox).
  
  QCustomPlot offers convenient pointers to the axes (\ref xAxis, \ref yAxis, \ref xAxis2, \ref
  yAxis2) and the \ref legend. They make it very easy working with plots that only have a single
  axis rect and at most one axis at each axis rect side. If you use \link thelayoutsystem the
  layout system\endlink to add multiple axis rects or multiple axes to one side, use the \ref
  QCPAxisRect::axis interface to access the new axes. If one of the four default axes or the
  default legend is removed due to manipulation of the layout system (e.g. by removing the main
  axis rect), the corresponding pointers become \c nullptr.
  
  If an axis convenience pointer is currently \c nullptr and a new axis rect or a corresponding
  axis is added in the place of the main axis rect, QCustomPlot resets the convenience pointers to
  the according new axes. Similarly the \ref legend convenience pointer will be reset if a legend
  is added after the main legend was removed before.
*/

/*! \var QCPLegend *QCustomPlot::legend

  A pointer to the default legend of the main axis rect. The legend is invisible by default. Use
  QCPLegend::setVisible to change this.
  
  QCustomPlot offers convenient pointers to the axes (\ref xAxis, \ref yAxis, \ref xAxis2, \ref
  yAxis2) and the \ref legend. They make it very easy working with plots that only have a single
  axis rect and at most one axis at each axis rect side. If you use \link thelayoutsystem the
  layout system\endlink to add multiple legends to the plot, use the layout system interface to
  access the new legend. For example, legends can be placed inside an axis rect's \ref
  QCPAxisRect::insetLayout "inset layout", and must then also be accessed via the inset layout. If
  the default legend is removed due to manipulation of the layout system (e.g. by removing the main
  axis rect), the corresponding pointer becomes \c nullptr.
  
  If an axis convenience pointer is currently \c nullptr and a new axis rect or a corresponding
  axis is added in the place of the main axis rect, QCustomPlot resets the convenience pointers to
  the according new axes. Similarly the \ref legend convenience pointer will be reset if a legend
  is added after the main legend was removed before.
*/

/* end of documentation of public members */

/*!
  Constructs a QCustomPlot and sets reasonable default values.
*/
QCustomPlot::QCustomPlot(QWidget *parent) :
  QWidget(parent),
  xAxis(nullptr),
  yAxis(nullptr),
  xAxis2(nullptr),
  yAxis2(nullptr),
  legend(nullptr),
  mBufferDevicePixelRatio(1.0), // will be adapted to true value below
  mPlotLayout(nullptr),
  mAutoAddPlottableToLegend(true),
  mAntialiasedElements(QCP::aeNone),
  mNotAntialiasedElements(QCP::aeNone),
  mInteractions(QCP::iNone),
  mSelectionTolerance(8),
  mNoAntialiasingOnDrag(false),
  mBackgroundBrush(Qt::white, Qt::SolidPattern),
  mBackgroundScaled(true),
  mBackgroundScaledMode(Qt::KeepAspectRatioByExpanding),
  mCurrentLayer(nullptr),
  mPlottingHints(QCP::phCacheLabels|QCP::phImmediateRefresh),
  mMultiSelectModifier(Qt::ControlModifier),
  mSelectionRectMode(QCP::srmNone),
  mSelectionRect(nullptr),
  mOpenGl(false),
  mMouseHasMoved(false),
  mMouseEventLayerable(nullptr),
  mMouseSignalLayerable(nullptr),
  mReplotting(false),
  mReplotQueued(false),
  mReplotTime(0),
  mReplotTimeAverage(0),
  mOpenGlMultisamples(16),
  mOpenGlAntialiasedElementsBackup(QCP::aeNone),
  mOpenGlCacheLabelsBackup(true)
{
  setAttribute(Qt::WA_NoMousePropagation);
  setFocusPolicy(Qt::ClickFocus);
  setMouseTracking(true);
  QLocale currentLocale = locale();
  currentLocale.setNumberOptions(QLocale::OmitGroupSeparator);
  setLocale(currentLocale);
#ifdef QCP_DEVICEPIXELRATIO_SUPPORTED
#  ifdef QCP_DEVICEPIXELRATIO_FLOAT
  setBufferDevicePixelRatio(QWidget::devicePixelRatioF());
#  else
  setBufferDevicePixelRatio(QWidget::devicePixelRatio());
#  endif
#endif
  
  mOpenGlAntialiasedElementsBackup = mAntialiasedElements;
  mOpenGlCacheLabelsBackup = mPlottingHints.testFlag(QCP::phCacheLabels);
  // create initial layers:
  mLayers.append(new QCPLayer(this, QLatin1String("background")));
  mLayers.append(new QCPLayer(this, QLatin1String("grid")));
  mLayers.append(new QCPLayer(this, QLatin1String("main")));
  mLayers.append(new QCPLayer(this, QLatin1String("axes")));
  mLayers.append(new QCPLayer(this, QLatin1String("legend")));
  mLayers.append(new QCPLayer(this, QLatin1String("overlay")));
  updateLayerIndices();
  setCurrentLayer(QLatin1String("main"));
  layer(QLatin1String("overlay"))->setMode(QCPLayer::lmBuffered);
  
  // create initial layout, axis rect and legend:
  mPlotLayout = new QCPLayoutGrid;
  mPlotLayout->initializeParentPlot(this);
  mPlotLayout->setParent(this); // important because if parent is QWidget, QCPLayout::sizeConstraintsChanged will call QWidget::updateGeometry
  mPlotLayout->setLayer(QLatin1String("main"));
  QCPAxisRect *defaultAxisRect = new QCPAxisRect(this, true);
  mPlotLayout->addElement(0, 0, defaultAxisRect);
  xAxis = defaultAxisRect->axis(QCPAxis::atBottom);
  yAxis = defaultAxisRect->axis(QCPAxis::atLeft);
  xAxis2 = defaultAxisRect->axis(QCPAxis::atTop);
  yAxis2 = defaultAxisRect->axis(QCPAxis::atRight);
  legend = new QCPLegend;
  legend->setVisible(false);
  defaultAxisRect->insetLayout()->addElement(legend, Qt::AlignRight|Qt::AlignTop);
  defaultAxisRect->insetLayout()->setMargins(QMargins(12, 12, 12, 12));
  
  defaultAxisRect->setLayer(QLatin1String("background"));
  xAxis->setLayer(QLatin1String("axes"));
  yAxis->setLayer(QLatin1String("axes"));
  xAxis2->setLayer(QLatin1String("axes"));
  yAxis2->setLayer(QLatin1String("axes"));
  xAxis->grid()->setLayer(QLatin1String("grid"));
  yAxis->grid()->setLayer(QLatin1String("grid"));
  xAxis2->grid()->setLayer(QLatin1String("grid"));
  yAxis2->grid()->setLayer(QLatin1String("grid"));
  legend->setLayer(QLatin1String("legend"));
  
  // create selection rect instance:
  mSelectionRect = new QCPSelectionRect(this);
  mSelectionRect->setLayer(QLatin1String("overlay"));
  
  setViewport(rect()); // needs to be called after mPlotLayout has been created
  
  replot(rpQueuedReplot);
}

QCustomPlot::~QCustomPlot()
{
  clearPlottables();
  clearItems();

  if (mPlotLayout)
  {
    delete mPlotLayout;
    mPlotLayout = nullptr;
  }
  
  mCurrentLayer = nullptr;
  qDeleteAll(mLayers); // don't use removeLayer, because it would prevent the last layer to be removed
  mLayers.clear();
}

/*!
  Sets which elements are forcibly drawn antialiased as an \a or combination of QCP::AntialiasedElement.
  
  This overrides the antialiasing settings for whole element groups, normally controlled with the
  \a setAntialiasing function on the individual elements. If an element is neither specified in
  \ref setAntialiasedElements nor in \ref setNotAntialiasedElements, the antialiasing setting on
  each individual element instance is used.
  
  For example, if \a antialiasedElements contains \ref QCP::aePlottables, all plottables will be
  drawn antialiased, no matter what the specific QCPAbstractPlottable::setAntialiased value was set
  to.
  
  if an element in \a antialiasedElements is already set in \ref setNotAntialiasedElements, it is
  removed from there.
  
  \see setNotAntialiasedElements
*/
void QCustomPlot::setAntialiasedElements(const QCP::AntialiasedElements &antialiasedElements)
{
  mAntialiasedElements = antialiasedElements;
  
  // make sure elements aren't in mNotAntialiasedElements and mAntialiasedElements simultaneously:
  if ((mNotAntialiasedElements & mAntialiasedElements) != 0)
    mNotAntialiasedElements |= ~mAntialiasedElements;
}

/*!
  Sets whether the specified \a antialiasedElement is forcibly drawn antialiased.
  
  See \ref setAntialiasedElements for details.
  
  \see setNotAntialiasedElement
*/
void QCustomPlot::setAntialiasedElement(QCP::AntialiasedElement antialiasedElement, bool enabled)
{
  if (!enabled && mAntialiasedElements.testFlag(antialiasedElement))
    mAntialiasedElements &= ~antialiasedElement;
  else if (enabled && !mAntialiasedElements.testFlag(antialiasedElement))
    mAntialiasedElements |= antialiasedElement;
  
  // make sure elements aren't in mNotAntialiasedElements and mAntialiasedElements simultaneously:
  if ((mNotAntialiasedElements & mAntialiasedElements) != 0)
    mNotAntialiasedElements |= ~mAntialiasedElements;
}

/*!
  Sets which elements are forcibly drawn not antialiased as an \a or combination of
  QCP::AntialiasedElement.
  
  This overrides the antialiasing settings for whole element groups, normally controlled with the
  \a setAntialiasing function on the individual elements. If an element is neither specified in
  \ref setAntialiasedElements nor in \ref setNotAntialiasedElements, the antialiasing setting on
  each individual element instance is used.
  
  For example, if \a notAntialiasedElements contains \ref QCP::aePlottables, no plottables will be
  drawn antialiased, no matter what the specific QCPAbstractPlottable::setAntialiased value was set
  to.
  
  if an element in \a notAntialiasedElements is already set in \ref setAntialiasedElements, it is
  removed from there.
  
  \see setAntialiasedElements
*/
void QCustomPlot::setNotAntialiasedElements(const QCP::AntialiasedElements &notAntialiasedElements)
{
  mNotAntialiasedElements = notAntialiasedElements;
  
  // make sure elements aren't in mNotAntialiasedElements and mAntialiasedElements simultaneously:
  if ((mNotAntialiasedElements & mAntialiasedElements) != 0)
    mAntialiasedElements |= ~mNotAntialiasedElements;
}

/*!
  Sets whether the specified \a notAntialiasedElement is forcibly drawn not antialiased.
  
  See \ref setNotAntialiasedElements for details.
  
  \see setAntialiasedElement
*/
void QCustomPlot::setNotAntialiasedElement(QCP::AntialiasedElement notAntialiasedElement, bool enabled)
{
  if (!enabled && mNotAntialiasedElements.testFlag(notAntialiasedElement))
    mNotAntialiasedElements &= ~notAntialiasedElement;
  else if (enabled && !mNotAntialiasedElements.testFlag(notAntialiasedElement))
    mNotAntialiasedElements |= notAntialiasedElement;
  
  // make sure elements aren't in mNotAntialiasedElements and mAntialiasedElements simultaneously:
  if ((mNotAntialiasedElements & mAntialiasedElements) != 0)
    mAntialiasedElements |= ~mNotAntialiasedElements;
}

/*!
  If set to true, adding a plottable (e.g. a graph) to the QCustomPlot automatically also adds the
  plottable to the legend (QCustomPlot::legend).
  
  \see addGraph, QCPLegend::addItem
*/
void QCustomPlot::setAutoAddPlottableToLegend(bool on)
{
  mAutoAddPlottableToLegend = on;
}

/*!
  Sets the possible interactions of this QCustomPlot as an or-combination of \ref QCP::Interaction
  enums. There are the following types of interactions:
  
  <b>Axis range manipulation</b> is controlled via \ref QCP::iRangeDrag and \ref QCP::iRangeZoom. When the
  respective interaction is enabled, the user may drag axes ranges and zoom with the mouse wheel.
  For details how to control which axes the user may drag/zoom and in what orientations, see \ref
  QCPAxisRect::setRangeDrag, \ref QCPAxisRect::setRangeZoom, \ref QCPAxisRect::setRangeDragAxes,
  \ref QCPAxisRect::setRangeZoomAxes.
  
  <b>Plottable data selection</b> is controlled by \ref QCP::iSelectPlottables. If \ref
  QCP::iSelectPlottables is set, the user may select plottables (graphs, curves, bars,...) and
  their data by clicking on them or in their vicinity (\ref setSelectionTolerance). Whether the
  user can actually select a plottable and its data can further be restricted with the \ref
  QCPAbstractPlottable::setSelectable method on the specific plottable. For details, see the
  special page about the \ref dataselection "data selection mechanism". To retrieve a list of all
  currently selected plottables, call \ref selectedPlottables. If you're only interested in
  QCPGraphs, you may use the convenience function \ref selectedGraphs.
  
  <b>Item selection</b> is controlled by \ref QCP::iSelectItems. If \ref QCP::iSelectItems is set, the user
  may select items (QCPItemLine, QCPItemText,...) by clicking on them or in their vicinity. To find
  out whether a specific item is selected, call QCPAbstractItem::selected(). To retrieve a list of
  all currently selected items, call \ref selectedItems.
  
  <b>Axis selection</b> is controlled with \ref QCP::iSelectAxes. If \ref QCP::iSelectAxes is set, the user
  may select parts of the axes by clicking on them. What parts exactly (e.g. Axis base line, tick
  labels, axis label) are selectable can be controlled via \ref QCPAxis::setSelectableParts for
  each axis. To retrieve a list of all axes that currently contain selected parts, call \ref
  selectedAxes. Which parts of an axis are selected, can be retrieved with QCPAxis::selectedParts().
  
  <b>Legend selection</b> is controlled with \ref QCP::iSelectLegend. If this is set, the user may
  select the legend itself or individual items by clicking on them. What parts exactly are
  selectable can be controlled via \ref QCPLegend::setSelectableParts. To find out whether the
  legend or any of its child items are selected, check the value of QCPLegend::selectedParts. To
  find out which child items are selected, call \ref QCPLegend::selectedItems.
  
  <b>All other selectable elements</b> The selection of all other selectable objects (e.g.
  QCPTextElement, or your own layerable subclasses) is controlled with \ref QCP::iSelectOther. If set, the
  user may select those objects by clicking on them. To find out which are currently selected, you
  need to check their selected state explicitly.
  
  If the selection state has changed by user interaction, the \ref selectionChangedByUser signal is
  emitted. Each selectable object additionally emits an individual selectionChanged signal whenever
  their selection state has changed, i.e. not only by user interaction.
  
  To allow multiple objects to be selected by holding the selection modifier (\ref
  setMultiSelectModifier), set the flag \ref QCP::iMultiSelect.
  
  \note In addition to the selection mechanism presented here, QCustomPlot always emits
  corresponding signals, when an object is clicked or double clicked. see \ref plottableClick and
  \ref plottableDoubleClick for example.
  
  \see setInteraction, setSelectionTolerance
*/
void QCustomPlot::setInteractions(const QCP::Interactions &interactions)
{
  mInteractions = interactions;
}

/*!
  Sets the single \a interaction of this QCustomPlot to \a enabled.
  
  For details about the interaction system, see \ref setInteractions.
  
  \see setInteractions
*/
void QCustomPlot::setInteraction(const QCP::Interaction &interaction, bool enabled)
{
  if (!enabled && mInteractions.testFlag(interaction))
    mInteractions &= ~interaction;
  else if (enabled && !mInteractions.testFlag(interaction))
    mInteractions |= interaction;
}

/*!
  Sets the tolerance that is used to decide whether a click selects an object (e.g. a plottable) or
  not.
  
  If the user clicks in the vicinity of the line of e.g. a QCPGraph, it's only regarded as a
  potential selection when the minimum distance between the click position and the graph line is
  smaller than \a pixels. Objects that are defined by an area (e.g. QCPBars) only react to clicks
  directly inside the area and ignore this selection tolerance. In other words, it only has meaning
  for parts of objects that are too thin to exactly hit with a click and thus need such a
  tolerance.
  
  \see setInteractions, QCPLayerable::selectTest
*/
void QCustomPlot::setSelectionTolerance(int pixels)
{
  mSelectionTolerance = pixels;
}

/*!
  Sets whether antialiasing is disabled for this QCustomPlot while the user is dragging axes
  ranges. If many objects, especially plottables, are drawn antialiased, this greatly improves
  performance during dragging. Thus it creates a more responsive user experience. As soon as the
  user stops dragging, the last replot is done with normal antialiasing, to restore high image
  quality.
  
  \see setAntialiasedElements, setNotAntialiasedElements
*/
void QCustomPlot::setNoAntialiasingOnDrag(bool enabled)
{
  mNoAntialiasingOnDrag = enabled;
}

/*!
  Sets the plotting hints for this QCustomPlot instance as an \a or combination of QCP::PlottingHint.
  
  \see setPlottingHint
*/
void QCustomPlot::setPlottingHints(const QCP::PlottingHints &hints)
{
  mPlottingHints = hints;
}

/*!
  Sets the specified plotting \a hint to \a enabled.
  
  \see setPlottingHints
*/
void QCustomPlot::setPlottingHint(QCP::PlottingHint hint, bool enabled)
{
  QCP::PlottingHints newHints = mPlottingHints;
  if (!enabled)
    newHints &= ~hint;
  else
    newHints |= hint;
  
  if (newHints != mPlottingHints)
    setPlottingHints(newHints);
}

/*!
  Sets the keyboard modifier that will be recognized as multi-select-modifier.
  
  If \ref QCP::iMultiSelect is specified in \ref setInteractions, the user may select multiple
  objects (or data points) by clicking on them one after the other while holding down \a modifier.
  
  By default the multi-select-modifier is set to Qt::ControlModifier.
  
  \see setInteractions
*/
void QCustomPlot::setMultiSelectModifier(Qt::KeyboardModifier modifier)
{
  mMultiSelectModifier = modifier;
}

/*!
  Sets how QCustomPlot processes mouse click-and-drag interactions by the user.

  If \a mode is \ref QCP::srmNone, the mouse drag is forwarded to the underlying objects. For
  example, QCPAxisRect may process a mouse drag by dragging axis ranges, see \ref
  QCPAxisRect::setRangeDrag. If \a mode is not \ref QCP::srmNone, the current selection rect (\ref
  selectionRect) becomes activated and allows e.g. rect zooming and data point selection.
  
  If you wish to provide your user both with axis range dragging and data selection/range zooming,
  use this method to switch between the modes just before the interaction is processed, e.g. in
  reaction to the \ref mousePress or \ref mouseMove signals. For example you could check whether
  the user is holding a certain keyboard modifier, and then decide which \a mode shall be set.
  
  If a selection rect interaction is currently active, and \a mode is set to \ref QCP::srmNone, the
  interaction is canceled (\ref QCPSelectionRect::cancel). Switching between any of the other modes
  will keep the selection rect active. Upon completion of the interaction, the behaviour is as
  defined by the currently set \a mode, not the mode that was set when the interaction started.
  
  \see setInteractions, setSelectionRect, QCPSelectionRect
*/
void QCustomPlot::setSelectionRectMode(QCP::SelectionRectMode mode)
{
  if (mSelectionRect)
  {
    if (mode == QCP::srmNone)
      mSelectionRect->cancel(); // when switching to none, we immediately want to abort a potentially active selection rect
    
    // disconnect old connections:
    if (mSelectionRectMode == QCP::srmSelect)
      disconnect(mSelectionRect, SIGNAL(accepted(QRect,QMouseEvent*)), this, SLOT(processRectSelection(QRect,QMouseEvent*)));
    else if (mSelectionRectMode == QCP::srmZoom)
      disconnect(mSelectionRect, SIGNAL(accepted(QRect,QMouseEvent*)), this, SLOT(processRectZoom(QRect,QMouseEvent*)));
    
    // establish new ones:
    if (mode == QCP::srmSelect)
      connect(mSelectionRect, SIGNAL(accepted(QRect,QMouseEvent*)), this, SLOT(processRectSelection(QRect,QMouseEvent*)));
    else if (mode == QCP::srmZoom)
      connect(mSelectionRect, SIGNAL(accepted(QRect,QMouseEvent*)), this, SLOT(processRectZoom(QRect,QMouseEvent*)));
  }
  
  mSelectionRectMode = mode;
}

/*!
  Sets the \ref QCPSelectionRect instance that QCustomPlot will use if \a mode is not \ref
  QCP::srmNone and the user performs a click-and-drag interaction. QCustomPlot takes ownership of
  the passed \a selectionRect. It can be accessed later via \ref selectionRect.
  
  This method is useful if you wish to replace the default QCPSelectionRect instance with an
  instance of a QCPSelectionRect subclass, to introduce custom behaviour of the selection rect.
  
  \see setSelectionRectMode
*/
void QCustomPlot::setSelectionRect(QCPSelectionRect *selectionRect)
{
  delete mSelectionRect;
  
  mSelectionRect = selectionRect;
  
  if (mSelectionRect)
  {
    // establish connections with new selection rect:
    if (mSelectionRectMode == QCP::srmSelect)
      connect(mSelectionRect, SIGNAL(accepted(QRect,QMouseEvent*)), this, SLOT(processRectSelection(QRect,QMouseEvent*)));
    else if (mSelectionRectMode == QCP::srmZoom)
      connect(mSelectionRect, SIGNAL(accepted(QRect,QMouseEvent*)), this, SLOT(processRectZoom(QRect,QMouseEvent*)));
  }
}

/*!
  \warning This is still an experimental feature and its performance depends on the system that it
  runs on. Having multiple QCustomPlot widgets in one application with enabled OpenGL rendering
  might cause context conflicts on some systems.
  
  This method allows to enable OpenGL plot rendering, for increased plotting performance of
  graphically demanding plots (thick lines, translucent fills, etc.).

  If \a enabled is set to true, QCustomPlot will try to initialize OpenGL and, if successful,
  continue plotting with hardware acceleration. The parameter \a multisampling controls how many
  samples will be used per pixel, it essentially controls the antialiasing quality. If \a
  multisampling is set too high for the current graphics hardware, the maximum allowed value will
  be used.

  You can test whether switching to OpenGL rendering was successful by checking whether the
  according getter \a QCustomPlot::openGl() returns true. If the OpenGL initialization fails,
  rendering continues with the regular software rasterizer, and an according qDebug output is
  generated.

  If switching to OpenGL was successful, this method disables label caching (\ref setPlottingHint
  "setPlottingHint(QCP::phCacheLabels, false)") and turns on QCustomPlot's antialiasing override
  for all elements (\ref setAntialiasedElements "setAntialiasedElements(QCP::aeAll)"), leading to a
  higher quality output. The antialiasing override allows for pixel-grid aligned drawing in the
  OpenGL paint device. As stated before, in OpenGL rendering the actual antialiasing of the plot is
  controlled with \a multisampling. If \a enabled is set to false, the antialiasing/label caching
  settings are restored to what they were before OpenGL was enabled, if they weren't altered in the
  meantime.

  \note OpenGL support is only enabled if QCustomPlot is compiled with the macro \c QCUSTOMPLOT_USE_OPENGL
  defined. This define must be set before including the QCustomPlot header both during compilation
  of the QCustomPlot library as well as when compiling your application. It is best to just include
  the line <tt>DEFINES += QCUSTOMPLOT_USE_OPENGL</tt> in the respective qmake project files.
  \note If you are using a Qt version before 5.0, you must also add the module "opengl" to your \c
  QT variable in the qmake project files. For Qt versions 5.0 and higher, QCustomPlot switches to a
  newer OpenGL interface which is already in the "gui" module.
*/
void QCustomPlot::setOpenGl(bool enabled, int multisampling, QSurface *surface)
{
  if (surface)
    multisampling = surface->format().samples();
  mOpenGlMultisamples = qMax(0, multisampling);
#ifdef QCUSTOMPLOT_USE_OPENGL
  mOpenGl = enabled;
  if (mOpenGl)
  {
    if (setupOpenGl(surface))
    {
      // backup antialiasing override and labelcaching setting so we can restore upon disabling OpenGL
      mOpenGlAntialiasedElementsBackup = mAntialiasedElements;
      mOpenGlCacheLabelsBackup = mPlottingHints.testFlag(QCP::phCacheLabels);
      // set antialiasing override to antialias all (aligns gl pixel grid properly), and disable label caching (would use software rasterizer for pixmap caches):
      setAntialiasedElements(QCP::aeAll);
      setPlottingHint(QCP::phCacheLabels, false);
    } else
    {
      qDebug() << Q_FUNC_INFO << "Failed to enable OpenGL, continuing plotting without hardware acceleration.";
      mOpenGl = false;
    }
  } else
  {
    // restore antialiasing override and labelcaching to what it was before enabling OpenGL, if nobody changed it in the meantime:
    if (mAntialiasedElements == QCP::aeAll)
      setAntialiasedElements(mOpenGlAntialiasedElementsBackup);
    if (!mPlottingHints.testFlag(QCP::phCacheLabels))
      setPlottingHint(QCP::phCacheLabels, mOpenGlCacheLabelsBackup);
    freeOpenGl();
  }
  // recreate all paint buffers:
  mPaintBuffers.clear();
  setupPaintBuffers();
#else
  Q_UNUSED(enabled)
  qDebug() << Q_FUNC_INFO << "QCustomPlot can't use OpenGL because QCUSTOMPLOT_USE_OPENGL was not defined during compilation (add 'DEFINES += QCUSTOMPLOT_USE_OPENGL' to your qmake .pro file)";
#endif
}

/*!
  Sets the viewport of this QCustomPlot. Usually users of QCustomPlot don't need to change the
  viewport manually.

  The viewport is the area in which the plot is drawn. All mechanisms, e.g. margin calculation take
  the viewport to be the outer border of the plot. The viewport normally is the rect() of the
  QCustomPlot widget, i.e. a rect with top left (0, 0) and size of the QCustomPlot widget.

  Don't confuse the viewport with the axis rect (QCustomPlot::axisRect). An axis rect is typically
  an area enclosed by four axes, where the graphs/plottables are drawn in. The viewport is larger
  and contains also the axes themselves, their tick numbers, their labels, or even additional axis
  rects, color scales and other layout elements.

  This function is used to allow arbitrary size exports with \ref toPixmap, \ref savePng, \ref
  savePdf, etc. by temporarily changing the viewport size.
*/
void QCustomPlot::setViewport(const QRect &rect)
{
  mViewport = rect;
  if (mPlotLayout)
    mPlotLayout->setOuterRect(mViewport);
}

/*!
  Sets the device pixel ratio used by the paint buffers of this QCustomPlot instance.

  Normally, this doesn't need to be set manually, because it is initialized with the regular \a
  QWidget::devicePixelRatio which is configured by Qt to fit the display device (e.g. 1 for normal
  displays, 2 for High-DPI displays).

  Device pixel ratios are supported by Qt only for Qt versions since 5.4. If this method is called
  when QCustomPlot is being used with older Qt versions, outputs an according qDebug message and
  leaves the internal buffer device pixel ratio at 1.0.
*/
void QCustomPlot::setBufferDevicePixelRatio(double ratio)
{
  if (!qFuzzyCompare(ratio, mBufferDevicePixelRatio))
  {
#ifdef QCP_DEVICEPIXELRATIO_SUPPORTED
    mBufferDevicePixelRatio = ratio;
    foreach (QSharedPointer<QCPAbstractPaintBuffer> buffer, mPaintBuffers)
      buffer->setDevicePixelRatio(mBufferDevicePixelRatio);
    // Note: axis label cache has devicePixelRatio as part of cache hash, so no need to manually clear cache here
#else
    qDebug() << Q_FUNC_INFO << "Device pixel ratios not supported for Qt versions before 5.4";
    mBufferDevicePixelRatio = 1.0;
#endif
  }
}

/*!
  Sets \a pm as the viewport background pixmap (see \ref setViewport). The pixmap is always drawn
  below all other objects in the plot.

  For cases where the provided pixmap doesn't have the same size as the viewport, scaling can be
  enabled with \ref setBackgroundScaled and the scaling mode (whether and how the aspect ratio is
  preserved) can be set with \ref setBackgroundScaledMode. To set all these options in one call,
  consider using the overloaded version of this function.
  
  If a background brush was set with \ref setBackground(const QBrush &brush), the viewport will
  first be filled with that brush, before drawing the background pixmap. This can be useful for
  background pixmaps with translucent areas.

  \see setBackgroundScaled, setBackgroundScaledMode
*/
void QCustomPlot::setBackground(const QPixmap &pm)
{
  mBackgroundPixmap = pm;
  mScaledBackgroundPixmap = QPixmap();
}

/*!
  Sets the background brush of the viewport (see \ref setViewport).

  Before drawing everything else, the background is filled with \a brush. If a background pixmap
  was set with \ref setBackground(const QPixmap &pm), this brush will be used to fill the viewport
  before the background pixmap is drawn. This can be useful for background pixmaps with translucent
  areas.
  
  Set \a brush to Qt::NoBrush or Qt::Transparent to leave background transparent. This can be
  useful for exporting to image formats which support transparency, e.g. \ref savePng.

  \see setBackgroundScaled, setBackgroundScaledMode
*/
void QCustomPlot::setBackground(const QBrush &brush)
{
  mBackgroundBrush = brush;
}

/*! \overload
  
  Allows setting the background pixmap of the viewport, whether it shall be scaled and how it
  shall be scaled in one call.

  \see setBackground(const QPixmap &pm), setBackgroundScaled, setBackgroundScaledMode
*/
void QCustomPlot::setBackground(const QPixmap &pm, bool scaled, Qt::AspectRatioMode mode)
{
  mBackgroundPixmap = pm;
  mScaledBackgroundPixmap = QPixmap();
  mBackgroundScaled = scaled;
  mBackgroundScaledMode = mode;
}

/*!
  Sets whether the viewport background pixmap shall be scaled to fit the viewport. If \a scaled is
  set to true, control whether and how the aspect ratio of the original pixmap is preserved with
  \ref setBackgroundScaledMode.
  
  Note that the scaled version of the original pixmap is buffered, so there is no performance
  penalty on replots. (Except when the viewport dimensions are changed continuously.)
  
  \see setBackground, setBackgroundScaledMode
*/
void QCustomPlot::setBackgroundScaled(bool scaled)
{
  mBackgroundScaled = scaled;
}

/*!
  If scaling of the viewport background pixmap is enabled (\ref setBackgroundScaled), use this
  function to define whether and how the aspect ratio of the original pixmap is preserved.
  
  \see setBackground, setBackgroundScaled
*/
void QCustomPlot::setBackgroundScaledMode(Qt::AspectRatioMode mode)
{
  mBackgroundScaledMode = mode;
}

/*!
  Returns the plottable with \a index. If the index is invalid, returns \c nullptr.
  
  There is an overloaded version of this function with no parameter which returns the last added
  plottable, see QCustomPlot::plottable()
  
  \see plottableCount
*/
QCPAbstractPlottable *QCustomPlot::plottable(int index)
{
  if (index >= 0 && index < mPlottables.size())
  {
    return mPlottables.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << index;
    return nullptr;
  }
}

/*! \overload
  
  Returns the last plottable that was added to the plot. If there are no plottables in the plot,
  returns \c nullptr.
  
  \see plottableCount
*/
QCPAbstractPlottable *QCustomPlot::plottable()
{
  if (!mPlottables.isEmpty())
  {
    return mPlottables.last();
  } else
    return nullptr;
}

/*!
  Removes the specified plottable from the plot and deletes it. If necessary, the corresponding
  legend item is also removed from the default legend (QCustomPlot::legend).
  
  Returns true on success.
  
  \see clearPlottables
*/
bool QCustomPlot::removePlottable(QCPAbstractPlottable *plottable)
{
  if (!mPlottables.contains(plottable))
  {
    qDebug() << Q_FUNC_INFO << "plottable not in list:" << reinterpret_cast<quintptr>(plottable);
    return false;
  }
  
  // remove plottable from legend:
  plottable->removeFromLegend();
  // special handling for QCPGraphs to maintain the simple graph interface:
  if (QCPGraph *graph = qobject_cast<QCPGraph*>(plottable))
    mGraphs.removeOne(graph);
  // remove plottable:
  delete plottable;
  mPlottables.removeOne(plottable);
  return true;
}

/*! \overload
  
  Removes and deletes the plottable by its \a index.
*/
bool QCustomPlot::removePlottable(int index)
{
  if (index >= 0 && index < mPlottables.size())
    return removePlottable(mPlottables[index]);
  else
  {
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << index;
    return false;
  }
}

/*!
  Removes all plottables from the plot and deletes them. Corresponding legend items are also
  removed from the default legend (QCustomPlot::legend).
  
  Returns the number of plottables removed.
  
  \see removePlottable
*/
int QCustomPlot::clearPlottables()
{
  int c = mPlottables.size();
  for (int i=c-1; i >= 0; --i)
    removePlottable(mPlottables[i]);
  return c;
}

/*!
  Returns the number of currently existing plottables in the plot
  
  \see plottable
*/
int QCustomPlot::plottableCount() const
{
  return mPlottables.size();
}

/*!
  Returns a list of the selected plottables. If no plottables are currently selected, the list is empty.
  
  There is a convenience function if you're only interested in selected graphs, see \ref selectedGraphs.
  
  \see setInteractions, QCPAbstractPlottable::setSelectable, QCPAbstractPlottable::setSelection
*/
QList<QCPAbstractPlottable*> QCustomPlot::selectedPlottables() const
{
  QList<QCPAbstractPlottable*> result;
  foreach (QCPAbstractPlottable *plottable, mPlottables)
  {
    if (plottable->selected())
      result.append(plottable);
  }
  return result;
}

/*!
  Returns any plottable at the pixel position \a pos. Since it can capture all plottables, the
  return type is the abstract base class of all plottables, QCPAbstractPlottable.
  
  For details, and if you wish to specify a certain plottable type (e.g. QCPGraph), see the
  template method plottableAt<PlottableType>()
  
  \see plottableAt<PlottableType>(), itemAt, layoutElementAt
*/
QCPAbstractPlottable *QCustomPlot::plottableAt(const QPointF &pos, bool onlySelectable, int *dataIndex) const
{
  return plottableAt<QCPAbstractPlottable>(pos, onlySelectable, dataIndex);
}

/*!
  Returns whether this QCustomPlot instance contains the \a plottable.
*/
bool QCustomPlot::hasPlottable(QCPAbstractPlottable *plottable) const
{
  return mPlottables.contains(plottable);
}

/*!
  Returns the graph with \a index. If the index is invalid, returns \c nullptr.
  
  There is an overloaded version of this function with no parameter which returns the last created
  graph, see QCustomPlot::graph()
  
  \see graphCount, addGraph
*/
QCPGraph *QCustomPlot::graph(int index) const
{
  if (index >= 0 && index < mGraphs.size())
  {
    return mGraphs.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << index;
    return nullptr;
  }
}

/*! \overload
  
  Returns the last graph, that was created with \ref addGraph. If there are no graphs in the plot,
  returns \c nullptr.
  
  \see graphCount, addGraph
*/
QCPGraph *QCustomPlot::graph() const
{
  if (!mGraphs.isEmpty())
  {
    return mGraphs.last();
  } else
    return nullptr;
}

/*!
  Creates a new graph inside the plot. If \a keyAxis and \a valueAxis are left unspecified (0), the
  bottom (xAxis) is used as key and the left (yAxis) is used as value axis. If specified, \a
  keyAxis and \a valueAxis must reside in this QCustomPlot.
  
  \a keyAxis will be used as key axis (typically "x") and \a valueAxis as value axis (typically
  "y") for the graph.
  
  Returns a pointer to the newly created graph, or \c nullptr if adding the graph failed.
  
  \see graph, graphCount, removeGraph, clearGraphs
*/
QCPGraph *QCustomPlot::addGraph(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
  if (!keyAxis) keyAxis = xAxis;
  if (!valueAxis) valueAxis = yAxis;
  if (!keyAxis || !valueAxis)
  {
    qDebug() << Q_FUNC_INFO << "can't use default QCustomPlot xAxis or yAxis, because at least one is invalid (has been deleted)";
    return nullptr;
  }
  if (keyAxis->parentPlot() != this || valueAxis->parentPlot() != this)
  {
    qDebug() << Q_FUNC_INFO << "passed keyAxis or valueAxis doesn't have this QCustomPlot as parent";
    return nullptr;
  }
  
  QCPGraph *newGraph = new QCPGraph(keyAxis, valueAxis);
  newGraph->setName(QLatin1String("Graph ")+QString::number(mGraphs.size()));
  return newGraph;
}

/*!
  Removes the specified \a graph from the plot and deletes it. If necessary, the corresponding
  legend item is also removed from the default legend (QCustomPlot::legend). If any other graphs in
  the plot have a channel fill set towards the removed graph, the channel fill property of those
  graphs is reset to \c nullptr (no channel fill).
  
  Returns true on success.
  
  \see clearGraphs
*/
bool QCustomPlot::removeGraph(QCPGraph *graph)
{
  return removePlottable(graph);
}

/*! \overload
  
  Removes and deletes the graph by its \a index.
*/
bool QCustomPlot::removeGraph(int index)
{
  if (index >= 0 && index < mGraphs.size())
    return removeGraph(mGraphs[index]);
  else
    return false;
}

/*!
  Removes all graphs from the plot and deletes them. Corresponding legend items are also removed
  from the default legend (QCustomPlot::legend).

  Returns the number of graphs removed.
  
  \see removeGraph
*/
int QCustomPlot::clearGraphs()
{
  int c = mGraphs.size();
  for (int i=c-1; i >= 0; --i)
    removeGraph(mGraphs[i]);
  return c;
}

/*!
  Returns the number of currently existing graphs in the plot
  
  \see graph, addGraph
*/
int QCustomPlot::graphCount() const
{
  return mGraphs.size();
}

/*!
  Returns a list of the selected graphs. If no graphs are currently selected, the list is empty.
  
  If you are not only interested in selected graphs but other plottables like QCPCurve, QCPBars,
  etc., use \ref selectedPlottables.
  
  \see setInteractions, selectedPlottables, QCPAbstractPlottable::setSelectable, QCPAbstractPlottable::setSelection
*/
QList<QCPGraph*> QCustomPlot::selectedGraphs() const
{
  QList<QCPGraph*> result;
  foreach (QCPGraph *graph, mGraphs)
  {
    if (graph->selected())
      result.append(graph);
  }
  return result;
}

/*!
  Returns the item with \a index. If the index is invalid, returns \c nullptr.
  
  There is an overloaded version of this function with no parameter which returns the last added
  item, see QCustomPlot::item()
  
  \see itemCount
*/
QCPAbstractItem *QCustomPlot::item(int index) const
{
  if (index >= 0 && index < mItems.size())
  {
    return mItems.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << index;
    return nullptr;
  }
}

/*! \overload
  
  Returns the last item that was added to this plot. If there are no items in the plot,
  returns \c nullptr.
  
  \see itemCount
*/
QCPAbstractItem *QCustomPlot::item() const
{
  if (!mItems.isEmpty())
  {
    return mItems.last();
  } else
    return nullptr;
}

/*!
  Removes the specified item from the plot and deletes it.
  
  Returns true on success.
  
  \see clearItems
*/
bool QCustomPlot::removeItem(QCPAbstractItem *item)
{
  if (mItems.contains(item))
  {
    delete item;
    mItems.removeOne(item);
    return true;
  } else
  {
    qDebug() << Q_FUNC_INFO << "item not in list:" << reinterpret_cast<quintptr>(item);
    return false;
  }
}

/*! \overload
  
  Removes and deletes the item by its \a index.
*/
bool QCustomPlot::removeItem(int index)
{
  if (index >= 0 && index < mItems.size())
    return removeItem(mItems[index]);
  else
  {
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << index;
    return false;
  }
}

/*!
  Removes all items from the plot and deletes them.
  
  Returns the number of items removed.
  
  \see removeItem
*/
int QCustomPlot::clearItems()
{
  int c = mItems.size();
  for (int i=c-1; i >= 0; --i)
    removeItem(mItems[i]);
  return c;
}

/*!
  Returns the number of currently existing items in the plot
  
  \see item
*/
int QCustomPlot::itemCount() const
{
  return mItems.size();
}

/*!
  Returns a list of the selected items. If no items are currently selected, the list is empty.
  
  \see setInteractions, QCPAbstractItem::setSelectable, QCPAbstractItem::setSelected
*/
QList<QCPAbstractItem*> QCustomPlot::selectedItems() const
{
  QList<QCPAbstractItem*> result;
  foreach (QCPAbstractItem *item, mItems)
  {
    if (item->selected())
      result.append(item);
  }
  return result;
}

/*!
  Returns the item at the pixel position \a pos. Since it can capture all items, the
  return type is the abstract base class of all items, QCPAbstractItem.
  
  For details, and if you wish to specify a certain item type (e.g. QCPItemLine), see the
  template method itemAt<ItemType>()
  
  \see itemAt<ItemType>(), plottableAt, layoutElementAt
*/
QCPAbstractItem *QCustomPlot::itemAt(const QPointF &pos, bool onlySelectable) const
{
  return itemAt<QCPAbstractItem>(pos, onlySelectable);
}

/*!
  Returns whether this QCustomPlot contains the \a item.
  
  \see item
*/
bool QCustomPlot::hasItem(QCPAbstractItem *item) const
{
  return mItems.contains(item);
}

/*!
  Returns the layer with the specified \a name. If there is no layer with the specified name, \c
  nullptr is returned.
  
  Layer names are case-sensitive.
  
  \see addLayer, moveLayer, removeLayer
*/
QCPLayer *QCustomPlot::layer(const QString &name) const
{
  foreach (QCPLayer *layer, mLayers)
  {
    if (layer->name() == name)
      return layer;
  }
  return nullptr;
}

/*! \overload
  
  Returns the layer by \a index. If the index is invalid, \c nullptr is returned.
  
  \see addLayer, moveLayer, removeLayer
*/
QCPLayer *QCustomPlot::layer(int index) const
{
  if (index >= 0 && index < mLayers.size())
  {
    return mLayers.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << index;
    return nullptr;
  }
}

/*!
  Returns the layer that is set as current layer (see \ref setCurrentLayer).
*/
QCPLayer *QCustomPlot::currentLayer() const
{
  return mCurrentLayer;
}

/*!
  Sets the layer with the specified \a name to be the current layer. All layerables (\ref
  QCPLayerable), e.g. plottables and items, are created on the current layer.
  
  Returns true on success, i.e. if there is a layer with the specified \a name in the QCustomPlot.
  
  Layer names are case-sensitive.
  
  \see addLayer, moveLayer, removeLayer, QCPLayerable::setLayer
*/
bool QCustomPlot::setCurrentLayer(const QString &name)
{
  if (QCPLayer *newCurrentLayer = layer(name))
  {
    return setCurrentLayer(newCurrentLayer);
  } else
  {
    qDebug() << Q_FUNC_INFO << "layer with name doesn't exist:" << name;
    return false;
  }
}

/*! \overload
  
  Sets the provided \a layer to be the current layer.
  
  Returns true on success, i.e. when \a layer is a valid layer in the QCustomPlot.
  
  \see addLayer, moveLayer, removeLayer
*/
bool QCustomPlot::setCurrentLayer(QCPLayer *layer)
{
  if (!mLayers.contains(layer))
  {
    qDebug() << Q_FUNC_INFO << "layer not a layer of this QCustomPlot:" << reinterpret_cast<quintptr>(layer);
    return false;
  }
  
  mCurrentLayer = layer;
  return true;
}

/*!
  Returns the number of currently existing layers in the plot
  
  \see layer, addLayer
*/
int QCustomPlot::layerCount() const
{
  return mLayers.size();
}

/*!
  Adds a new layer to this QCustomPlot instance. The new layer will have the name \a name, which
  must be unique. Depending on \a insertMode, it is positioned either below or above \a otherLayer.
  
  Returns true on success, i.e. if there is no other layer named \a name and \a otherLayer is a
  valid layer inside this QCustomPlot.
  
  If \a otherLayer is 0, the highest layer in the QCustomPlot will be used.
  
  For an explanation of what layers are in QCustomPlot, see the documentation of \ref QCPLayer.
  
  \see layer, moveLayer, removeLayer
*/
bool QCustomPlot::addLayer(const QString &name, QCPLayer *otherLayer, QCustomPlot::LayerInsertMode insertMode)
{
  if (!otherLayer)
    otherLayer = mLayers.last();
  if (!mLayers.contains(otherLayer))
  {
    qDebug() << Q_FUNC_INFO << "otherLayer not a layer of this QCustomPlot:" << reinterpret_cast<quintptr>(otherLayer);
    return false;
  }
  if (layer(name))
  {
    qDebug() << Q_FUNC_INFO << "A layer exists already with the name" << name;
    return false;
  }
    
  QCPLayer *newLayer = new QCPLayer(this, name);
  mLayers.insert(otherLayer->index() + (insertMode==limAbove ? 1:0), newLayer);
  updateLayerIndices();
  setupPaintBuffers(); // associates new layer with the appropriate paint buffer
  return true;
}

/*!
  Removes the specified \a layer and returns true on success.
  
  All layerables (e.g. plottables and items) on the removed layer will be moved to the layer below
  \a layer. If \a layer is the bottom layer, the layerables are moved to the layer above. In both
  cases, the total rendering order of all layerables in the QCustomPlot is preserved.
  
  If \a layer is the current layer (\ref setCurrentLayer), the layer below (or above, if bottom
  layer) becomes the new current layer.
  
  It is not possible to remove the last layer of the plot.
  
  \see layer, addLayer, moveLayer
*/
bool QCustomPlot::removeLayer(QCPLayer *layer)
{
  if (!mLayers.contains(layer))
  {
    qDebug() << Q_FUNC_INFO << "layer not a layer of this QCustomPlot:" << reinterpret_cast<quintptr>(layer);
    return false;
  }
  if (mLayers.size() < 2)
  {
    qDebug() << Q_FUNC_INFO << "can't remove last layer";
    return false;
  }
  
  // append all children of this layer to layer below (if this is lowest layer, prepend to layer above)
  int removedIndex = layer->index();
  bool isFirstLayer = removedIndex==0;
  QCPLayer *targetLayer = isFirstLayer ? mLayers.at(removedIndex+1) : mLayers.at(removedIndex-1);
  QList<QCPLayerable*> children = layer->children();
  if (isFirstLayer) // prepend in reverse order (such that relative order stays the same)
    std::reverse(children.begin(), children.end());
  foreach (QCPLayerable *child, children)
    child->moveToLayer(targetLayer, isFirstLayer); // prepend if isFirstLayer, otherwise append
  
  // if removed layer is current layer, change current layer to layer below/above:
  if (layer == mCurrentLayer)
    setCurrentLayer(targetLayer);
  
  // invalidate the paint buffer that was responsible for this layer:
  if (QSharedPointer<QCPAbstractPaintBuffer> pb = layer->mPaintBuffer.toStrongRef())
    pb->setInvalidated();
  
  // remove layer:
  delete layer;
  mLayers.removeOne(layer);
  updateLayerIndices();
  return true;
}

/*!
  Moves the specified \a layer either above or below \a otherLayer. Whether it's placed above or
  below is controlled with \a insertMode.
  
  Returns true on success, i.e. when both \a layer and \a otherLayer are valid layers in the
  QCustomPlot.
  
  \see layer, addLayer, moveLayer
*/
bool QCustomPlot::moveLayer(QCPLayer *layer, QCPLayer *otherLayer, QCustomPlot::LayerInsertMode insertMode)
{
  if (!mLayers.contains(layer))
  {
    qDebug() << Q_FUNC_INFO << "layer not a layer of this QCustomPlot:" << reinterpret_cast<quintptr>(layer);
    return false;
  }
  if (!mLayers.contains(otherLayer))
  {
    qDebug() << Q_FUNC_INFO << "otherLayer not a layer of this QCustomPlot:" << reinterpret_cast<quintptr>(otherLayer);
    return false;
  }
  
  if (layer->index() > otherLayer->index())
    mLayers.move(layer->index(), otherLayer->index() + (insertMode==limAbove ? 1:0));
  else if (layer->index() < otherLayer->index())
    mLayers.move(layer->index(), otherLayer->index() + (insertMode==limAbove ? 0:-1));
  
  // invalidate the paint buffers that are responsible for the layers:
  if (QSharedPointer<QCPAbstractPaintBuffer> pb = layer->mPaintBuffer.toStrongRef())
    pb->setInvalidated();
  if (QSharedPointer<QCPAbstractPaintBuffer> pb = otherLayer->mPaintBuffer.toStrongRef())
    pb->setInvalidated();
  
  updateLayerIndices();
  return true;
}

/*!
  Returns the number of axis rects in the plot.
  
  All axis rects can be accessed via QCustomPlot::axisRect().
  
  Initially, only one axis rect exists in the plot.
  
  \see axisRect, axisRects
*/
int QCustomPlot::axisRectCount() const
{
  return axisRects().size();
}

/*!
  Returns the axis rect with \a index.
  
  Initially, only one axis rect (with index 0) exists in the plot. If multiple axis rects were
  added, all of them may be accessed with this function in a linear fashion (even when they are
  nested in a layout hierarchy or inside other axis rects via QCPAxisRect::insetLayout).
  
  The order of the axis rects is given by the fill order of the \ref QCPLayout that is holding
  them. For example, if the axis rects are in the top level grid layout (accessible via \ref
  QCustomPlot::plotLayout), they are ordered from left to right, top to bottom, if the layout's
  default \ref QCPLayoutGrid::setFillOrder "setFillOrder" of \ref QCPLayoutGrid::foColumnsFirst
  "foColumnsFirst" wasn't changed.
  
  If you want to access axis rects by their row and column index, use the layout interface. For
  example, use \ref QCPLayoutGrid::element of the top level grid layout, and \c qobject_cast the
  returned layout element to \ref QCPAxisRect. (See also \ref thelayoutsystem.)
  
  \see axisRectCount, axisRects, QCPLayoutGrid::setFillOrder
*/
QCPAxisRect *QCustomPlot::axisRect(int index) const
{
  const QList<QCPAxisRect*> rectList = axisRects();
  if (index >= 0 && index < rectList.size())
  {
    return rectList.at(index);
  } else
  {
    qDebug() << Q_FUNC_INFO << "invalid axis rect index" << index;
    return nullptr;
  }
}

/*!
  Returns all axis rects in the plot.
  
  The order of the axis rects is given by the fill order of the \ref QCPLayout that is holding
  them. For example, if the axis rects are in the top level grid layout (accessible via \ref
  QCustomPlot::plotLayout), they are ordered from left to right, top to bottom, if the layout's
  default \ref QCPLayoutGrid::setFillOrder "setFillOrder" of \ref QCPLayoutGrid::foColumnsFirst
  "foColumnsFirst" wasn't changed.
  
  \see axisRectCount, axisRect, QCPLayoutGrid::setFillOrder
*/
QList<QCPAxisRect*> QCustomPlot::axisRects() const
{
  QList<QCPAxisRect*> result;
  QStack<QCPLayoutElement*> elementStack;
  if (mPlotLayout)
    elementStack.push(mPlotLayout);
  
  while (!elementStack.isEmpty())
  {
    foreach (QCPLayoutElement *element, elementStack.pop()->elements(false))
    {
      if (element)
      {
        elementStack.push(element);
        if (QCPAxisRect *ar = qobject_cast<QCPAxisRect*>(element))
          result.append(ar);
      }
    }
  }
  
  return result;
}

/*!
  Returns the layout element at pixel position \a pos. If there is no element at that position,
  returns \c nullptr.
  
  Only visible elements are used. If \ref QCPLayoutElement::setVisible on the element itself or on
  any of its parent elements is set to false, it will not be considered.
  
  \see itemAt, plottableAt
*/
QCPLayoutElement *QCustomPlot::layoutElementAt(const QPointF &pos) const
{
  QCPLayoutElement *currentElement = mPlotLayout;
  bool searchSubElements = true;
  while (searchSubElements && currentElement)
  {
    searchSubElements = false;
    foreach (QCPLayoutElement *subElement, currentElement->elements(false))
    {
      if (subElement && subElement->realVisibility() && subElement->selectTest(pos, false) >= 0)
      {
        currentElement = subElement;
        searchSubElements = true;
        break;
      }
    }
  }
  return currentElement;
}

/*!
  Returns the layout element of type \ref QCPAxisRect at pixel position \a pos. This method ignores
  other layout elements even if they are visually in front of the axis rect (e.g. a \ref
  QCPLegend). If there is no axis rect at that position, returns \c nullptr.

  Only visible axis rects are used. If \ref QCPLayoutElement::setVisible on the axis rect itself or
  on any of its parent elements is set to false, it will not be considered.

  \see layoutElementAt
*/
QCPAxisRect *QCustomPlot::axisRectAt(const QPointF &pos) const
{
  QCPAxisRect *result = nullptr;
  QCPLayoutElement *currentElement = mPlotLayout;
  bool searchSubElements = true;
  while (searchSubElements && currentElement)
  {
    searchSubElements = false;
    foreach (QCPLayoutElement *subElement, currentElement->elements(false))
    {
      if (subElement && subElement->realVisibility() && subElement->selectTest(pos, false) >= 0)
      {
        currentElement = subElement;
        searchSubElements = true;
        if (QCPAxisRect *ar = qobject_cast<QCPAxisRect*>(currentElement))
          result = ar;
        break;
      }
    }
  }
  return result;
}

/*!
  Returns the axes that currently have selected parts, i.e. whose selection state is not \ref
  QCPAxis::spNone.
  
  \see selectedPlottables, selectedLegends, setInteractions, QCPAxis::setSelectedParts,
  QCPAxis::setSelectableParts
*/
QList<QCPAxis*> QCustomPlot::selectedAxes() const
{
  QList<QCPAxis*> result, allAxes;
  foreach (QCPAxisRect *rect, axisRects())
    allAxes << rect->axes();
  
  foreach (QCPAxis *axis, allAxes)
  {
    if (axis->selectedParts() != QCPAxis::spNone)
      result.append(axis);
  }
  
  return result;
}

/*!
  Returns the legends that currently have selected parts, i.e. whose selection state is not \ref
  QCPLegend::spNone.
  
  \see selectedPlottables, selectedAxes, setInteractions, QCPLegend::setSelectedParts,
  QCPLegend::setSelectableParts, QCPLegend::selectedItems
*/
QList<QCPLegend*> QCustomPlot::selectedLegends() const
{
  QList<QCPLegend*> result;
  
  QStack<QCPLayoutElement*> elementStack;
  if (mPlotLayout)
    elementStack.push(mPlotLayout);
  
  while (!elementStack.isEmpty())
  {
    foreach (QCPLayoutElement *subElement, elementStack.pop()->elements(false))
    {
      if (subElement)
      {
        elementStack.push(subElement);
        if (QCPLegend *leg = qobject_cast<QCPLegend*>(subElement))
        {
          if (leg->selectedParts() != QCPLegend::spNone)
            result.append(leg);
        }
      }
    }
  }
  
  return result;
}

/*!
  Deselects all layerables (plottables, items, axes, legends,...) of the QCustomPlot.
  
  Since calling this function is not a user interaction, this does not emit the \ref
  selectionChangedByUser signal. The individual selectionChanged signals are emitted though, if the
  objects were previously selected.
  
  \see setInteractions, selectedPlottables, selectedItems, selectedAxes, selectedLegends
*/
void QCustomPlot::deselectAll()
{
  foreach (QCPLayer *layer, mLayers)
  {
    foreach (QCPLayerable *layerable, layer->children())
      layerable->deselectEvent(nullptr);
  }
}

/*!
  Causes a complete replot into the internal paint buffer(s). Finally, the widget surface is
  refreshed with the new buffer contents. This is the method that must be called to make changes to
  the plot, e.g. on the axis ranges or data points of graphs, visible.

  The parameter \a refreshPriority can be used to fine-tune the timing of the replot. For example
  if your application calls \ref replot very quickly in succession (e.g. multiple independent
  functions change some aspects of the plot and each wants to make sure the change gets replotted),
  it is advisable to set \a refreshPriority to \ref QCustomPlot::rpQueuedReplot. This way, the
  actual replotting is deferred to the next event loop iteration. Multiple successive calls of \ref
  replot with this priority will only cause a single replot, avoiding redundant replots and
  improving performance.

  Under a few circumstances, QCustomPlot causes a replot by itself. Those are resize events of the
  QCustomPlot widget and user interactions (object selection and range dragging/zooming).

  Before the replot happens, the signal \ref beforeReplot is emitted. After the replot, \ref
  afterReplot is emitted. It is safe to mutually connect the replot slot with any of those two
  signals on two QCustomPlots to make them replot synchronously, it won't cause an infinite
  recursion.

  If a layer is in mode \ref QCPLayer::lmBuffered (\ref QCPLayer::setMode), it is also possible to
  replot only that specific layer via \ref QCPLayer::replot. See the documentation there for
  details.
  
  \see replotTime
*/
void QCustomPlot::replot(QCustomPlot::RefreshPriority refreshPriority)
{
  if (refreshPriority == QCustomPlot::rpQueuedReplot)
  {
    if (!mReplotQueued)
    {
      mReplotQueued = true;
      QTimer::singleShot(0, this, SLOT(replot()));
    }
    return;
  }
  
  if (mReplotting) // incase signals loop back to replot slot
    return;
  mReplotting = true;
  mReplotQueued = false;
  emit beforeReplot();
  
# if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
  QTime replotTimer;
  replotTimer.start();
# else
  QElapsedTimer replotTimer;
  replotTimer.start();
# endif
  
  updateLayout();
  // draw all layered objects (grid, axes, plottables, items, legend,...) into their buffers:
  setupPaintBuffers();
  foreach (QCPLayer *layer, mLayers)
    layer->drawToPaintBuffer();
  foreach (QSharedPointer<QCPAbstractPaintBuffer> buffer, mPaintBuffers)
    buffer->setInvalidated(false);
  
  if ((refreshPriority == rpRefreshHint && mPlottingHints.testFlag(QCP::phImmediateRefresh)) || refreshPriority==rpImmediateRefresh)
    repaint();
  else
    update();
  
# if QT_VERSION < QT_VERSION_CHECK(4, 8, 0)
  mReplotTime = replotTimer.elapsed();
# else
  mReplotTime = replotTimer.nsecsElapsed()*1e-6;
# endif
  if (!qFuzzyIsNull(mReplotTimeAverage))
    mReplotTimeAverage = mReplotTimeAverage*0.9 + mReplotTime*0.1; // exponential moving average with a time constant of 10 last replots
  else
    mReplotTimeAverage = mReplotTime; // no previous replots to average with, so initialize with replot time
  
  emit afterReplot();
  mReplotting = false;
}

/*!
  Returns the time in milliseconds that the last replot took. If \a average is set to true, an
  exponential moving average over the last couple of replots is returned.
  
  \see replot
*/
double QCustomPlot::replotTime(bool average) const
{
  return average ? mReplotTimeAverage : mReplotTime;
}

/*!
  Rescales the axes such that all plottables (like graphs) in the plot are fully visible.
  
  if \a onlyVisiblePlottables is set to true, only the plottables that have their visibility set to true
  (QCPLayerable::setVisible), will be used to rescale the axes.
  
  \see QCPAbstractPlottable::rescaleAxes, QCPAxis::rescale
*/
void QCustomPlot::rescaleAxes(bool onlyVisiblePlottables)
{
  QList<QCPAxis*> allAxes;
  foreach (QCPAxisRect *rect, axisRects())
    allAxes << rect->axes();
  
  foreach (QCPAxis *axis, allAxes)
    axis->rescale(onlyVisiblePlottables);
}

/*!
  Saves a PDF with the vectorized plot to the file \a fileName. The axis ratio as well as the scale
  of texts and lines will be derived from the specified \a width and \a height. This means, the
  output will look like the normal on-screen output of a QCustomPlot widget with the corresponding
  pixel width and height. If either \a width or \a height is zero, the exported image will have the
  same dimensions as the QCustomPlot widget currently has.

  Setting \a exportPen to \ref QCP::epNoCosmetic allows to disable the use of cosmetic pens when
  drawing to the PDF file. Cosmetic pens are pens with numerical width 0, which are always drawn as
  a one pixel wide line, no matter what zoom factor is set in the PDF-Viewer. For more information
  about cosmetic pens, see the QPainter and QPen documentation.

  The objects of the plot will appear in the current selection state. If you don't want any
  selected objects to be painted in their selected look, deselect everything with \ref deselectAll
  before calling this function.

  Returns true on success.

  \warning
  \li If you plan on editing the exported PDF file with a vector graphics editor like Inkscape, it
  is advised to set \a exportPen to \ref QCP::epNoCosmetic to avoid losing those cosmetic lines
  (which might be quite many, because cosmetic pens are the default for e.g. axes and tick marks).
  \li If calling this function inside the constructor of the parent of the QCustomPlot widget
  (i.e. the MainWindow constructor, if QCustomPlot is inside the MainWindow), always provide
  explicit non-zero widths and heights. If you leave \a width or \a height as 0 (default), this
  function uses the current width and height of the QCustomPlot widget. However, in Qt, these
  aren't defined yet inside the constructor, so you would get an image that has strange
  widths/heights.

  \a pdfCreator and \a pdfTitle may be used to set the according metadata fields in the resulting
  PDF file.

  \note On Android systems, this method does nothing and issues an according qDebug warning
  message. This is also the case if for other reasons the define flag \c QT_NO_PRINTER is set.

  \see savePng, saveBmp, saveJpg, saveRastered
*/
bool QCustomPlot::savePdf(const QString &fileName, int width, int height, QCP::ExportPen exportPen, const QString &pdfCreator, const QString &pdfTitle)
{
  bool success = false;
#ifdef QT_NO_PRINTER
  Q_UNUSED(fileName)
  Q_UNUSED(exportPen)
  Q_UNUSED(width)
  Q_UNUSED(height)
  Q_UNUSED(pdfCreator)
  Q_UNUSED(pdfTitle)
  qDebug() << Q_FUNC_INFO << "Qt was built without printer support (QT_NO_PRINTER). PDF not created.";
#else
  int newWidth, newHeight;
  if (width == 0 || height == 0)
  {
    newWidth = this->width();
    newHeight = this->height();
  } else
  {
    newWidth = width;
    newHeight = height;
  }
  
  QPrinter printer(QPrinter::ScreenResolution);
  printer.setOutputFileName(fileName);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setColorMode(QPrinter::Color);
  printer.printEngine()->setProperty(QPrintEngine::PPK_Creator, pdfCreator);
  printer.printEngine()->setProperty(QPrintEngine::PPK_DocumentName, pdfTitle);
  QRect oldViewport = viewport();
  setViewport(QRect(0, 0, newWidth, newHeight));
#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
  printer.setFullPage(true);
  printer.setPaperSize(viewport().size(), QPrinter::DevicePixel);
#else
  QPageLayout pageLayout;
  pageLayout.setMode(QPageLayout::FullPageMode);
  pageLayout.setOrientation(QPageLayout::Portrait);
  pageLayout.setMargins(QMarginsF(0, 0, 0, 0));
  pageLayout.setPageSize(QPageSize(viewport().size(), QPageSize::Point, QString(), QPageSize::ExactMatch));
  printer.setPageLayout(pageLayout);
#endif
  QCPPainter printpainter;
  if (printpainter.begin(&printer))
  {
    printpainter.setMode(QCPPainter::pmVectorized);
    printpainter.setMode(QCPPainter::pmNoCaching);
    printpainter.setMode(QCPPainter::pmNonCosmetic, exportPen==QCP::epNoCosmetic);
    printpainter.setWindow(mViewport);
    if (mBackgroundBrush.style() != Qt::NoBrush &&
        mBackgroundBrush.color() != Qt::white &&
        mBackgroundBrush.color() != Qt::transparent &&
        mBackgroundBrush.color().alpha() > 0) // draw pdf background color if not white/transparent
      printpainter.fillRect(viewport(), mBackgroundBrush);
    draw(&printpainter);
    printpainter.end();
    success = true;
  }
  setViewport(oldViewport);
#endif // QT_NO_PRINTER
  return success;
}

/*!
  Saves a PNG image file to \a fileName on disc. The output plot will have the dimensions \a width
  and \a height in pixels, multiplied by \a scale. If either \a width or \a height is zero, the
  current width and height of the QCustomPlot widget is used instead. Line widths and texts etc.
  are not scaled up when larger widths/heights are used. If you want that effect, use the \a scale
  parameter.

  For example, if you set both \a width and \a height to 100 and \a scale to 2, you will end up with an
  image file of size 200*200 in which all graphical elements are scaled up by factor 2 (line widths,
  texts, etc.). This scaling is not done by stretching a 100*100 image, the result will have full
  200*200 pixel resolution.

  If you use a high scaling factor, it is recommended to enable antialiasing for all elements by
  temporarily setting \ref QCustomPlot::setAntialiasedElements to \ref QCP::aeAll as this allows
  QCustomPlot to place objects with sub-pixel accuracy.

  image compression can be controlled with the \a quality parameter which must be between 0 and 100
  or -1 to use the default setting.

  The \a resolution will be written to the image file header and has no direct consequence for the
  quality or the pixel size. However, if opening the image with a tool which respects the metadata,
  it will be able to scale the image to match either a given size in real units of length (inch,
  centimeters, etc.), or the target display DPI. You can specify in which units \a resolution is
  given, by setting \a resolutionUnit. The \a resolution is converted to the format's expected
  resolution unit internally.

  Returns true on success. If this function fails, most likely the PNG format isn't supported by
  the system, see Qt docs about QImageWriter::supportedImageFormats().

  The objects of the plot will appear in the current selection state. If you don't want any selected
  objects to be painted in their selected look, deselect everything with \ref deselectAll before calling
  this function.

  If you want the PNG to have a transparent background, call \ref setBackground(const QBrush &brush)
  with no brush (Qt::NoBrush) or a transparent color (Qt::transparent), before saving.

  \warning If calling this function inside the constructor of the parent of the QCustomPlot widget
  (i.e. the MainWindow constructor, if QCustomPlot is inside the MainWindow), always provide
  explicit non-zero widths and heights. If you leave \a width or \a height as 0 (default), this
  function uses the current width and height of the QCustomPlot widget. However, in Qt, these
  aren't defined yet inside the constructor, so you would get an image that has strange
  widths/heights.

  \see savePdf, saveBmp, saveJpg, saveRastered
*/
bool QCustomPlot::savePng(const QString &fileName, int width, int height, double scale, int quality, int resolution, QCP::ResolutionUnit resolutionUnit)
{
  return saveRastered(fileName, width, height, scale, "PNG", quality, resolution, resolutionUnit);
}

/*!
  Saves a JPEG image file to \a fileName on disc. The output plot will have the dimensions \a width
  and \a height in pixels, multiplied by \a scale. If either \a width or \a height is zero, the
  current width and height of the QCustomPlot widget is used instead. Line widths and texts etc.
  are not scaled up when larger widths/heights are used. If you want that effect, use the \a scale
  parameter.

  For example, if you set both \a width and \a height to 100 and \a scale to 2, you will end up with an
  image file of size 200*200 in which all graphical elements are scaled up by factor 2 (line widths,
  texts, etc.). This scaling is not done by stretching a 100*100 image, the result will have full
  200*200 pixel resolution.

  If you use a high scaling factor, it is recommended to enable antialiasing for all elements by
  temporarily setting \ref QCustomPlot::setAntialiasedElements to \ref QCP::aeAll as this allows
  QCustomPlot to place objects with sub-pixel accuracy.

  image compression can be controlled with the \a quality parameter which must be between 0 and 100
  or -1 to use the default setting.

  The \a resolution will be written to the image file header and has no direct consequence for the
  quality or the pixel size. However, if opening the image with a tool which respects the metadata,
  it will be able to scale the image to match either a given size in real units of length (inch,
  centimeters, etc.), or the target display DPI. You can specify in which units \a resolution is
  given, by setting \a resolutionUnit. The \a resolution is converted to the format's expected
  resolution unit internally.

  Returns true on success. If this function fails, most likely the JPEG format isn't supported by
  the system, see Qt docs about QImageWriter::supportedImageFormats().

  The objects of the plot will appear in the current selection state. If you don't want any selected
  objects to be painted in their selected look, deselect everything with \ref deselectAll before calling
  this function.

  \warning If calling this function inside the constructor of the parent of the QCustomPlot widget
  (i.e. the MainWindow constructor, if QCustomPlot is inside the MainWindow), always provide
  explicit non-zero widths and heights. If you leave \a width or \a height as 0 (default), this
  function uses the current width and height of the QCustomPlot widget. However, in Qt, these
  aren't defined yet inside the constructor, so you would get an image that has strange
  widths/heights.

  \see savePdf, savePng, saveBmp, saveRastered
*/
bool QCustomPlot::saveJpg(const QString &fileName, int width, int height, double scale, int quality, int resolution, QCP::ResolutionUnit resolutionUnit)
{
  return saveRastered(fileName, width, height, scale, "JPG", quality, resolution, resolutionUnit);
}

/*!
  Saves a BMP image file to \a fileName on disc. The output plot will have the dimensions \a width
  and \a height in pixels, multiplied by \a scale. If either \a width or \a height is zero, the
  current width and height of the QCustomPlot widget is used instead. Line widths and texts etc.
  are not scaled up when larger widths/heights are used. If you want that effect, use the \a scale
  parameter.

  For example, if you set both \a width and \a height to 100 and \a scale to 2, you will end up with an
  image file of size 200*200 in which all graphical elements are scaled up by factor 2 (line widths,
  texts, etc.). This scaling is not done by stretching a 100*100 image, the result will have full
  200*200 pixel resolution.

  If you use a high scaling factor, it is recommended to enable antialiasing for all elements by
  temporarily setting \ref QCustomPlot::setAntialiasedElements to \ref QCP::aeAll as this allows
  QCustomPlot to place objects with sub-pixel accuracy.

  The \a resolution will be written to the image file header and has no direct consequence for the
  quality or the pixel size. However, if opening the image with a tool which respects the metadata,
  it will be able to scale the image to match either a given size in real units of length (inch,
  centimeters, etc.), or the target display DPI. You can specify in which units \a resolution is
  given, by setting \a resolutionUnit. The \a resolution is converted to the format's expected
  resolution unit internally.

  Returns true on success. If this function fails, most likely the BMP format isn't supported by
  the system, see Qt docs about QImageWriter::supportedImageFormats().

  The objects of the plot will appear in the current selection state. If you don't want any selected
  objects to be painted in their selected look, deselect everything with \ref deselectAll before calling
  this function.

  \warning If calling this function inside the constructor of the parent of the QCustomPlot widget
  (i.e. the MainWindow constructor, if QCustomPlot is inside the MainWindow), always provide
  explicit non-zero widths and heights. If you leave \a width or \a height as 0 (default), this
  function uses the current width and height of the QCustomPlot widget. However, in Qt, these
  aren't defined yet inside the constructor, so you would get an image that has strange
  widths/heights.

  \see savePdf, savePng, saveJpg, saveRastered
*/
bool QCustomPlot::saveBmp(const QString &fileName, int width, int height, double scale, int resolution, QCP::ResolutionUnit resolutionUnit)
{
  return saveRastered(fileName, width, height, scale, "BMP", -1, resolution, resolutionUnit);
}

/*! \internal
  
  Returns a minimum size hint that corresponds to the minimum size of the top level layout
  (\ref plotLayout). To prevent QCustomPlot from being collapsed to size/width zero, set a minimum
  size (setMinimumSize) either on the whole QCustomPlot or on any layout elements inside the plot.
  This is especially important, when placed in a QLayout where other components try to take in as
  much space as possible (e.g. QMdiArea).
*/
QSize QCustomPlot::minimumSizeHint() const
{
  return mPlotLayout->minimumOuterSizeHint();
}

/*! \internal
  
  Returns a size hint that is the same as \ref minimumSizeHint.
  
*/
QSize QCustomPlot::sizeHint() const
{
  return mPlotLayout->minimumOuterSizeHint();
}

/*! \internal
  
  Event handler for when the QCustomPlot widget needs repainting. This does not cause a \ref replot, but
  draws the internal buffer on the widget surface.
*/
void QCustomPlot::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event)
  
  // detect if the device pixel ratio has changed (e.g. moving window between different DPI screens), and adapt buffers if necessary:
#ifdef QCP_DEVICEPIXELRATIO_SUPPORTED
#  ifdef QCP_DEVICEPIXELRATIO_FLOAT
  double newDpr = devicePixelRatioF();
#  else
  double newDpr = devicePixelRatio();
#  endif
  if (!qFuzzyCompare(mBufferDevicePixelRatio, newDpr))
  {
    setBufferDevicePixelRatio(newDpr);
    replot(QCustomPlot::rpQueuedRefresh);
    return;
  }
#endif
  
  QCPPainter painter(this);
  if (painter.isActive())
  {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  painter.setRenderHint(QPainter::Antialiasing); // to make Antialiasing look good if using the OpenGL graphicssystem
#endif
    if (mBackgroundBrush.style() != Qt::NoBrush)
      painter.fillRect(mViewport, mBackgroundBrush);
    drawBackground(&painter);
    foreach (QSharedPointer<QCPAbstractPaintBuffer> buffer, mPaintBuffers)
      buffer->draw(&painter);
  }
}

/*! \internal
  
  Event handler for a resize of the QCustomPlot widget. The viewport (which becomes the outer rect
  of mPlotLayout) is resized appropriately. Finally a \ref replot is performed.
*/
void QCustomPlot::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event)
  // resize and repaint the buffer:
  setViewport(rect());
  replot(rpQueuedRefresh); // queued refresh is important here, to prevent painting issues in some contexts (e.g. MDI subwindow)
}

/*! \internal
  
 Event handler for when a double click occurs. Emits the \ref mouseDoubleClick signal, then
 determines the layerable under the cursor and forwards the event to it. Finally, emits the
 specialized signals when certain objecs are clicked (e.g. \ref plottableDoubleClick, \ref
 axisDoubleClick, etc.).
 
 \see mousePressEvent, mouseReleaseEvent
*/
void QCustomPlot::mouseDoubleClickEvent(QMouseEvent *event)
{
  emit mouseDoubleClick(event);
  mMouseHasMoved = false;
  mMousePressPos = event->pos();
  
  // determine layerable under the cursor (this event is called instead of the second press event in a double-click):
  QList<QVariant> details;
  QList<QCPLayerable*> candidates = layerableListAt(mMousePressPos, false, &details);
  for (int i=0; i<candidates.size(); ++i)
  {
    event->accept(); // default impl of QCPLayerable's mouse events ignore the event, in that case propagate to next candidate in list
    candidates.at(i)->mouseDoubleClickEvent(event, details.at(i));
    if (event->isAccepted())
    {
      mMouseEventLayerable = candidates.at(i);
      mMouseEventLayerableDetails = details.at(i);
      break;
    }
  }
  
  // emit specialized object double click signals:
  if (!candidates.isEmpty())
  {
    if (QCPAbstractPlottable *ap = qobject_cast<QCPAbstractPlottable*>(candidates.first()))
    {
      int dataIndex = 0;
      if (!details.first().value<QCPDataSelection>().isEmpty())
        dataIndex = details.first().value<QCPDataSelection>().dataRange().begin();
      emit plottableDoubleClick(ap, dataIndex, event);
    } else if (QCPAxis *ax = qobject_cast<QCPAxis*>(candidates.first()))
      emit axisDoubleClick(ax, details.first().value<QCPAxis::SelectablePart>(), event);
    else if (QCPAbstractItem *ai = qobject_cast<QCPAbstractItem*>(candidates.first()))
      emit itemDoubleClick(ai, event);
    else if (QCPLegend *lg = qobject_cast<QCPLegend*>(candidates.first()))
      emit legendDoubleClick(lg, nullptr, event);
    else if (QCPAbstractLegendItem *li = qobject_cast<QCPAbstractLegendItem*>(candidates.first()))
      emit legendDoubleClick(li->parentLegend(), li, event);
  }
  
  event->accept(); // in case QCPLayerable reimplementation manipulates event accepted state. In QWidget event system, QCustomPlot wants to accept the event.
}

/*! \internal
  
  Event handler for when a mouse button is pressed. Emits the mousePress signal.

  If the current \ref setSelectionRectMode is not \ref QCP::srmNone, passes the event to the
  selection rect. Otherwise determines the layerable under the cursor and forwards the event to it.
  
  \see mouseMoveEvent, mouseReleaseEvent
*/
void QCustomPlot::mousePressEvent(QMouseEvent *event)
{
  emit mousePress(event);
  // save some state to tell in releaseEvent whether it was a click:
  mMouseHasMoved = false;
  mMousePressPos = event->pos();
  
  if (mSelectionRect && mSelectionRectMode != QCP::srmNone)
  {
    if (mSelectionRectMode != QCP::srmZoom || qobject_cast<QCPAxisRect*>(axisRectAt(mMousePressPos))) // in zoom mode only activate selection rect if on an axis rect
      mSelectionRect->startSelection(event);
  } else
  {
    // no selection rect interaction, prepare for click signal emission and forward event to layerable under the cursor:
    QList<QVariant> details;
    QList<QCPLayerable*> candidates = layerableListAt(mMousePressPos, false, &details);
    if (!candidates.isEmpty())
    {
      mMouseSignalLayerable = candidates.first(); // candidate for signal emission is always topmost hit layerable (signal emitted in release event)
      mMouseSignalLayerableDetails = details.first();
    }
    // forward event to topmost candidate which accepts the event:
    for (int i=0; i<candidates.size(); ++i)
    {
      event->accept(); // default impl of QCPLayerable's mouse events call ignore() on the event, in that case propagate to next candidate in list
      candidates.at(i)->mousePressEvent(event, details.at(i));
      if (event->isAccepted())
      {
        mMouseEventLayerable = candidates.at(i);
        mMouseEventLayerableDetails = details.at(i);
        break;
      }
    }
  }
  
  event->accept(); // in case QCPLayerable reimplementation manipulates event accepted state. In QWidget event system, QCustomPlot wants to accept the event.
}

/*! \internal
  
  Event handler for when the cursor is moved. Emits the \ref mouseMove signal.

  If the selection rect (\ref setSelectionRect) is currently active, the event is forwarded to it
  in order to update the rect geometry.
  
  Otherwise, if a layout element has mouse capture focus (a mousePressEvent happened on top of the
  layout element before), the mouseMoveEvent is forwarded to that element.
  
  \see mousePressEvent, mouseReleaseEvent
*/
void QCustomPlot::mouseMoveEvent(QMouseEvent *event)
{
  emit mouseMove(event);
  
  if (!mMouseHasMoved && (mMousePressPos-event->pos()).manhattanLength() > 3)
    mMouseHasMoved = true; // moved too far from mouse press position, don't handle as click on mouse release
  
  if (mSelectionRect && mSelectionRect->isActive())
    mSelectionRect->moveSelection(event);
  else if (mMouseEventLayerable) // call event of affected layerable:
    mMouseEventLayerable->mouseMoveEvent(event, mMousePressPos);
  
  event->accept(); // in case QCPLayerable reimplementation manipulates event accepted state. In QWidget event system, QCustomPlot wants to accept the event.
}

/*! \internal

  Event handler for when a mouse button is released. Emits the \ref mouseRelease signal.

  If the mouse was moved less than a certain threshold in any direction since the \ref
  mousePressEvent, it is considered a click which causes the selection mechanism (if activated via
  \ref setInteractions) to possibly change selection states accordingly. Further, specialized mouse
  click signals are emitted (e.g. \ref plottableClick, \ref axisClick, etc.)

  If a layerable is the mouse capturer (a \ref mousePressEvent happened on top of the layerable
  before), the \ref mouseReleaseEvent is forwarded to that element.

  \see mousePressEvent, mouseMoveEvent
*/
void QCustomPlot::mouseReleaseEvent(QMouseEvent *event)
{
  emit mouseRelease(event);
  
  if (!mMouseHasMoved) // mouse hasn't moved (much) between press and release, so handle as click
  {
    if (mSelectionRect && mSelectionRect->isActive()) // a simple click shouldn't successfully finish a selection rect, so cancel it here
      mSelectionRect->cancel();
    if (event->button() == Qt::LeftButton)
      processPointSelection(event);
    
    // emit specialized click signals of QCustomPlot instance:
    if (QCPAbstractPlottable *ap = qobject_cast<QCPAbstractPlottable*>(mMouseSignalLayerable))
    {
      int dataIndex = 0;
      if (!mMouseSignalLayerableDetails.value<QCPDataSelection>().isEmpty())
        dataIndex = mMouseSignalLayerableDetails.value<QCPDataSelection>().dataRange().begin();
      emit plottableClick(ap, dataIndex, event);
    } else if (QCPAxis *ax = qobject_cast<QCPAxis*>(mMouseSignalLayerable))
      emit axisClick(ax, mMouseSignalLayerableDetails.value<QCPAxis::SelectablePart>(), event);
    else if (QCPAbstractItem *ai = qobject_cast<QCPAbstractItem*>(mMouseSignalLayerable))
      emit itemClick(ai, event);
    else if (QCPLegend *lg = qobject_cast<QCPLegend*>(mMouseSignalLayerable))
      emit legendClick(lg, nullptr, event);
    else if (QCPAbstractLegendItem *li = qobject_cast<QCPAbstractLegendItem*>(mMouseSignalLayerable))
      emit legendClick(li->parentLegend(), li, event);
    mMouseSignalLayerable = nullptr;
  }
  
  if (mSelectionRect && mSelectionRect->isActive()) // Note: if a click was detected above, the selection rect is canceled there
  {
    // finish selection rect, the appropriate action will be taken via signal-slot connection:
    mSelectionRect->endSelection(event);
  } else
  {
    // call event of affected layerable:
    if (mMouseEventLayerable)
    {
      mMouseEventLayerable->mouseReleaseEvent(event, mMousePressPos);
      mMouseEventLayerable = nullptr;
    }
  }
  
  if (noAntialiasingOnDrag())
    replot(rpQueuedReplot);
  
  event->accept(); // in case QCPLayerable reimplementation manipulates event accepted state. In QWidget event system, QCustomPlot wants to accept the event.
}

/*! \internal

  Event handler for mouse wheel events. First, the \ref mouseWheel signal is emitted. Then
  determines the affected layerable and forwards the event to it.
*/
void QCustomPlot::wheelEvent(QWheelEvent *event)
{
  emit mouseWheel(event);
  
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  const QPointF pos = event->pos();
#else
  const QPointF pos = event->position();
#endif
  
  // forward event to layerable under cursor:
  foreach (QCPLayerable *candidate, layerableListAt(pos, false))
  {
    event->accept(); // default impl of QCPLayerable's mouse events ignore the event, in that case propagate to next candidate in list
    candidate->wheelEvent(event);
    if (event->isAccepted())
      break;
  }
  event->accept(); // in case QCPLayerable reimplementation manipulates event accepted state. In QWidget event system, QCustomPlot wants to accept the event.
}

/*! \internal
  
  This function draws the entire plot, including background pixmap, with the specified \a painter.
  It does not make use of the paint buffers like \ref replot, so this is the function typically
  used by saving/exporting methods such as \ref savePdf or \ref toPainter.

  Note that it does not fill the background with the background brush (as the user may specify with
  \ref setBackground(const QBrush &brush)), this is up to the respective functions calling this
  method.
*/
void QCustomPlot::draw(QCPPainter *painter)
{
  updateLayout();
  
  // draw viewport background pixmap:
  drawBackground(painter);

  // draw all layered objects (grid, axes, plottables, items, legend,...):
  foreach (QCPLayer *layer, mLayers)
    layer->draw(painter);
  
  /* Debug code to draw all layout element rects
  foreach (QCPLayoutElement *el, findChildren<QCPLayoutElement*>())
  {
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QColor(0, 0, 0, 100), 0, Qt::DashLine));
    painter->drawRect(el->rect());
    painter->setPen(QPen(QColor(255, 0, 0, 100), 0, Qt::DashLine));
    painter->drawRect(el->outerRect());
  }
  */
}

/*! \internal

  Performs the layout update steps defined by \ref QCPLayoutElement::UpdatePhase, by calling \ref
  QCPLayoutElement::update on the main plot layout.

  Here, the layout elements calculate their positions and margins, and prepare for the following
  draw call.
*/
void QCustomPlot::updateLayout()
{
  // run through layout phases:
  mPlotLayout->update(QCPLayoutElement::upPreparation);
  mPlotLayout->update(QCPLayoutElement::upMargins);
  mPlotLayout->update(QCPLayoutElement::upLayout);

  emit afterLayout();
}

/*! \internal
  
  Draws the viewport background pixmap of the plot.
  
  If a pixmap was provided via \ref setBackground, this function buffers the scaled version
  depending on \ref setBackgroundScaled and \ref setBackgroundScaledMode and then draws it inside
  the viewport with the provided \a painter. The scaled version is buffered in
  mScaledBackgroundPixmap to prevent expensive rescaling at every redraw. It is only updated, when
  the axis rect has changed in a way that requires a rescale of the background pixmap (this is
  dependent on the \ref setBackgroundScaledMode), or when a differend axis background pixmap was
  set.
  
  Note that this function does not draw a fill with the background brush
  (\ref setBackground(const QBrush &brush)) beneath the pixmap.
  
  \see setBackground, setBackgroundScaled, setBackgroundScaledMode
*/
void QCustomPlot::drawBackground(QCPPainter *painter)
{
  // Note: background color is handled in individual replot/save functions

  // draw background pixmap (on top of fill, if brush specified):
  if (!mBackgroundPixmap.isNull())
  {
    if (mBackgroundScaled)
    {
      // check whether mScaledBackground needs to be updated:
      QSize scaledSize(mBackgroundPixmap.size());
      scaledSize.scale(mViewport.size(), mBackgroundScaledMode);
      if (mScaledBackgroundPixmap.size() != scaledSize)
        mScaledBackgroundPixmap = mBackgroundPixmap.scaled(mViewport.size(), mBackgroundScaledMode, Qt::SmoothTransformation);
      painter->drawPixmap(mViewport.topLeft(), mScaledBackgroundPixmap, QRect(0, 0, mViewport.width(), mViewport.height()) & mScaledBackgroundPixmap.rect());
    } else
    {
      painter->drawPixmap(mViewport.topLeft(), mBackgroundPixmap, QRect(0, 0, mViewport.width(), mViewport.height()));
    }
  }
}

/*! \internal

  Goes through the layers and makes sure this QCustomPlot instance holds the correct number of
  paint buffers and that they have the correct configuration (size, pixel ratio, etc.).
  Allocations, reallocations and deletions of paint buffers are performed as necessary. It also
  associates the paint buffers with the layers, so they draw themselves into the right buffer when
  \ref QCPLayer::drawToPaintBuffer is called. This means it associates adjacent \ref
  QCPLayer::lmLogical layers to a mutual paint buffer and creates dedicated paint buffers for
  layers in \ref QCPLayer::lmBuffered mode.

  This method uses \ref createPaintBuffer to create new paint buffers.

  After this method, the paint buffers are empty (filled with \c Qt::transparent) and invalidated
  (so an attempt to replot only a single buffered layer causes a full replot).

  This method is called in every \ref replot call, prior to actually drawing the layers (into their
  associated paint buffer). If the paint buffers don't need changing/reallocating, this method
  basically leaves them alone and thus finishes very fast.
*/
void QCustomPlot::setupPaintBuffers()
{
  int bufferIndex = 0;
  if (mPaintBuffers.isEmpty())
    mPaintBuffers.append(QSharedPointer<QCPAbstractPaintBuffer>(createPaintBuffer()));
  
  for (int layerIndex = 0; layerIndex < mLayers.size(); ++layerIndex)
  {
    QCPLayer *layer = mLayers.at(layerIndex);
    if (layer->mode() == QCPLayer::lmLogical)
    {
      layer->mPaintBuffer = mPaintBuffers.at(bufferIndex).toWeakRef();
    } else if (layer->mode() == QCPLayer::lmBuffered)
    {
      ++bufferIndex;
      if (bufferIndex >= mPaintBuffers.size())
        mPaintBuffers.append(QSharedPointer<QCPAbstractPaintBuffer>(createPaintBuffer()));
      layer->mPaintBuffer = mPaintBuffers.at(bufferIndex).toWeakRef();
      if (layerIndex < mLayers.size()-1 && mLayers.at(layerIndex+1)->mode() == QCPLayer::lmLogical) // not last layer, and next one is logical, so prepare another buffer for next layerables
      {
        ++bufferIndex;
        if (bufferIndex >= mPaintBuffers.size())
          mPaintBuffers.append(QSharedPointer<QCPAbstractPaintBuffer>(createPaintBuffer()));
      }
    }
  }
  // remove unneeded buffers:
  while (mPaintBuffers.size()-1 > bufferIndex)
    mPaintBuffers.removeLast();
  // resize buffers to viewport size and clear contents:
  foreach (QSharedPointer<QCPAbstractPaintBuffer> buffer, mPaintBuffers)
  {
    buffer->setSize(viewport().size()); // won't do anything if already correct size
    buffer->clear(Qt::transparent);
    buffer->setInvalidated();
  }
}

/*! \internal

  This method is used by \ref setupPaintBuffers when it needs to create new paint buffers.

  Depending on the current setting of \ref setOpenGl, and the current Qt version, different
  backends (subclasses of \ref QCPAbstractPaintBuffer) are created, initialized with the proper
  size and device pixel ratio, and returned.
*/
QCPAbstractPaintBuffer *QCustomPlot::createPaintBuffer()
{
  if (mOpenGl)
  {
#if defined(QCP_OPENGL_FBO)
    return new QCPPaintBufferGlFbo(viewport().size(), mBufferDevicePixelRatio, mGlContext, mGlPaintDevice);
#elif defined(QCP_OPENGL_PBUFFER)
    return new QCPPaintBufferGlPbuffer(viewport().size(), mBufferDevicePixelRatio, mOpenGlMultisamples);
#else
    qDebug() << Q_FUNC_INFO << "OpenGL enabled even though no support for it compiled in, this shouldn't have happened. Falling back to pixmap paint buffer.";
    return new QCPPaintBufferPixmap(viewport().size(), mBufferDevicePixelRatio);
#endif
  } else
    return new QCPPaintBufferPixmap(viewport().size(), mBufferDevicePixelRatio);
}

/*!
  This method returns whether any of the paint buffers held by this QCustomPlot instance are
  invalidated.

  If any buffer is invalidated, a partial replot (\ref QCPLayer::replot) is not allowed and always
  causes a full replot (\ref QCustomPlot::replot) of all layers. This is the case when for example
  the layer order has changed, new layers were added or removed, layer modes were changed (\ref
  QCPLayer::setMode), or layerables were added or removed.

  \see QCPAbstractPaintBuffer::setInvalidated
*/
bool QCustomPlot::hasInvalidatedPaintBuffers()
{
  foreach (QSharedPointer<QCPAbstractPaintBuffer> buffer, mPaintBuffers)
  {
    if (buffer->invalidated())
      return true;
  }
  return false;
}

/*! \internal

  When \ref setOpenGl is set to true, this method is used to initialize OpenGL (create a context,
  surface, paint device).

  Returns true on success.

  If this method is successful, all paint buffers should be deleted and then reallocated by calling
  \ref setupPaintBuffers, so the OpenGL-based paint buffer subclasses (\ref
  QCPPaintBufferGlPbuffer, \ref QCPPaintBufferGlFbo) are used for subsequent replots.

  \see freeOpenGl
*/
bool QCustomPlot::setupOpenGl(QSurface *surface)
{
#ifdef QCP_OPENGL_FBO
  freeOpenGl();
  if (!surface)
  {
    QSurfaceFormat proposedSurfaceFormat;
    proposedSurfaceFormat.setSamples(mOpenGlMultisamples);
#ifdef QCP_OPENGL_OFFSCREENSURFACE
    QOffscreenSurface *newSurface = new QOffscreenSurface;
#else
    QWindow *newSurface = new QWindow;
    newSurface->setSurfaceType(QSurface::OpenGLSurface);
#endif
    newSurface->setFormat(proposedSurfaceFormat);
    newSurface->create();
    surface = newSurface;
  }
  mGlSurface = QSharedPointer<QSurface>(surface);
  mGlContext = QSharedPointer<QOpenGLContext>(new QOpenGLContext);
  mGlContext->setFormat(mGlSurface->format());
  if (!mGlContext->create())
  {
    qDebug() << Q_FUNC_INFO << "Failed to create OpenGL context";
    mGlContext.clear();
    mGlSurface.clear();
    return false;
  }
  if (!mGlContext->makeCurrent(mGlSurface.data())) // context needs to be current to create paint device
  {
    qDebug() << Q_FUNC_INFO << "Failed to make opengl context current";
    mGlContext.clear();
    mGlSurface.clear();
    return false;
  }
  if (!QOpenGLFramebufferObject::hasOpenGLFramebufferObjects())
  {
    qDebug() << Q_FUNC_INFO << "OpenGL of this system doesn't support frame buffer objects";
    mGlContext.clear();
    mGlSurface.clear();
    return false;
  }
  mGlPaintDevice = QSharedPointer<QOpenGLPaintDevice>(new QOpenGLPaintDevice);
  return true;
#elif defined(QCP_OPENGL_PBUFFER)
  return QGLFormat::hasOpenGL();
#else
  return false;
#endif
}

/*! \internal

  When \ref setOpenGl is set to false, this method is used to deinitialize OpenGL (releases the
  context and frees resources).

  After OpenGL is disabled, all paint buffers should be deleted and then reallocated by calling
  \ref setupPaintBuffers, so the standard software rendering paint buffer subclass (\ref
  QCPPaintBufferPixmap) is used for subsequent replots.

  \see setupOpenGl
*/
void QCustomPlot::freeOpenGl()
{
#ifdef QCP_OPENGL_FBO
  mGlPaintDevice.clear();
  mGlContext.clear();
  mGlSurface.clear();
#endif
}

/*! \internal
  
  This method is used by \ref QCPAxisRect::removeAxis to report removed axes to the QCustomPlot
  so it may clear its QCustomPlot::xAxis, yAxis, xAxis2 and yAxis2 members accordingly.
*/
void QCustomPlot::axisRemoved(QCPAxis *axis)
{
  if (xAxis == axis)
    xAxis = nullptr;
  if (xAxis2 == axis)
    xAxis2 = nullptr;
  if (yAxis == axis)
    yAxis = nullptr;
  if (yAxis2 == axis)
    yAxis2 = nullptr;
  
  // Note: No need to take care of range drag axes and range zoom axes, because they are stored in smart pointers
}

/*! \internal
  
  This method is used by the QCPLegend destructor to report legend removal to the QCustomPlot so
  it may clear its QCustomPlot::legend member accordingly.
*/
void QCustomPlot::legendRemoved(QCPLegend *legend)
{
  if (this->legend == legend)
    this->legend = nullptr;
}

/*! \internal
  
  This slot is connected to the selection rect's \ref QCPSelectionRect::accepted signal when \ref
  setSelectionRectMode is set to \ref QCP::srmSelect.

  First, it determines which axis rect was the origin of the selection rect judging by the starting
  point of the selection. Then it goes through the plottables (\ref QCPAbstractPlottable1D to be
  precise) associated with that axis rect and finds the data points that are in \a rect. It does
  this by querying their \ref QCPAbstractPlottable1D::selectTestRect method.
  
  Then, the actual selection is done by calling the plottables' \ref
  QCPAbstractPlottable::selectEvent, placing the found selected data points in the \a details
  parameter as <tt>QVariant(\ref QCPDataSelection)</tt>. All plottables that weren't touched by \a
  rect receive a \ref QCPAbstractPlottable::deselectEvent.
  
  \see processRectZoom
*/
void QCustomPlot::processRectSelection(QRect rect, QMouseEvent *event)
{
  typedef QPair<QCPAbstractPlottable*, QCPDataSelection> SelectionCandidate;
  typedef QMultiMap<int, SelectionCandidate> SelectionCandidates; // map key is number of selected data points, so we have selections sorted by size
  
  bool selectionStateChanged = false;
  
  if (mInteractions.testFlag(QCP::iSelectPlottables))
  {
    SelectionCandidates potentialSelections;
    QRectF rectF(rect.normalized());
    if (QCPAxisRect *affectedAxisRect = axisRectAt(rectF.topLeft()))
    {
      // determine plottables that were hit by the rect and thus are candidates for selection:
      foreach (QCPAbstractPlottable *plottable, affectedAxisRect->plottables())
      {
        if (QCPPlottableInterface1D *plottableInterface = plottable->interface1D())
        {
          QCPDataSelection dataSel = plottableInterface->selectTestRect(rectF, true);
          if (!dataSel.isEmpty())
            potentialSelections.insert(dataSel.dataPointCount(), SelectionCandidate(plottable, dataSel));
        }
      }
      
      if (!mInteractions.testFlag(QCP::iMultiSelect))
      {
        // only leave plottable with most selected points in map, since we will only select a single plottable:
        if (!potentialSelections.isEmpty())
        {
          SelectionCandidates::iterator it = potentialSelections.begin();
          while (it != std::prev(potentialSelections.end())) // erase all except last element
            it = potentialSelections.erase(it);
        }
      }
      
      bool additive = event->modifiers().testFlag(mMultiSelectModifier);
      // deselect all other layerables if not additive selection:
      if (!additive)
      {
        // emit deselection except to those plottables who will be selected afterwards:
        foreach (QCPLayer *layer, mLayers)
        {
          foreach (QCPLayerable *layerable, layer->children())
          {
            if ((potentialSelections.isEmpty() || potentialSelections.constBegin()->first != layerable) && mInteractions.testFlag(layerable->selectionCategory()))
            {
              bool selChanged = false;
              layerable->deselectEvent(&selChanged);
              selectionStateChanged |= selChanged;
            }
          }
        }
      }
      
      // go through selections in reverse (largest selection first) and emit select events:
      SelectionCandidates::const_iterator it = potentialSelections.constEnd();
      while (it != potentialSelections.constBegin())
      {
        --it;
        if (mInteractions.testFlag(it.value().first->selectionCategory()))
        {
          bool selChanged = false;
          it.value().first->selectEvent(event, additive, QVariant::fromValue(it.value().second), &selChanged);
          selectionStateChanged |= selChanged;
        }
      }
    }
  }
  
  if (selectionStateChanged)
  {
    emit selectionChangedByUser();
    replot(rpQueuedReplot);
  } else if (mSelectionRect)
    mSelectionRect->layer()->replot();
}

/*! \internal
  
  This slot is connected to the selection rect's \ref QCPSelectionRect::accepted signal when \ref
  setSelectionRectMode is set to \ref QCP::srmZoom.

  It determines which axis rect was the origin of the selection rect judging by the starting point
  of the selection, and then zooms the axes defined via \ref QCPAxisRect::setRangeZoomAxes to the
  provided \a rect (see \ref QCPAxisRect::zoom).
  
  \see processRectSelection
*/
void QCustomPlot::processRectZoom(QRect rect, QMouseEvent *event)
{
  Q_UNUSED(event)
  if (QCPAxisRect *axisRect = axisRectAt(rect.topLeft()))
  {
    QList<QCPAxis*> affectedAxes = QList<QCPAxis*>() << axisRect->rangeZoomAxes(Qt::Horizontal) << axisRect->rangeZoomAxes(Qt::Vertical);
    affectedAxes.removeAll(static_cast<QCPAxis*>(nullptr));
    axisRect->zoom(QRectF(rect), affectedAxes);
  }
  replot(rpQueuedReplot); // always replot to make selection rect disappear
}

/*! \internal

  This method is called when a simple left mouse click was detected on the QCustomPlot surface.

  It first determines the layerable that was hit by the click, and then calls its \ref
  QCPLayerable::selectEvent. All other layerables receive a QCPLayerable::deselectEvent (unless the
  multi-select modifier was pressed, see \ref setMultiSelectModifier).

  In this method the hit layerable is determined a second time using \ref layerableAt (after the
  one in \ref mousePressEvent), because we want \a onlySelectable set to true this time. This
  implies that the mouse event grabber (mMouseEventLayerable) may be a different one from the
  clicked layerable determined here. For example, if a non-selectable layerable is in front of a
  selectable layerable at the click position, the front layerable will receive mouse events but the
  selectable one in the back will receive the \ref QCPLayerable::selectEvent.

  \see processRectSelection, QCPLayerable::selectTest
*/
void QCustomPlot::processPointSelection(QMouseEvent *event)
{
  QVariant details;
  QCPLayerable *clickedLayerable = layerableAt(event->pos(), true, &details);
  bool selectionStateChanged = false;
  bool additive = mInteractions.testFlag(QCP::iMultiSelect) && event->modifiers().testFlag(mMultiSelectModifier);
  // deselect all other layerables if not additive selection:
  if (!additive)
  {
    foreach (QCPLayer *layer, mLayers)
    {
      foreach (QCPLayerable *layerable, layer->children())
      {
        if (layerable != clickedLayerable && mInteractions.testFlag(layerable->selectionCategory()))
        {
          bool selChanged = false;
          layerable->deselectEvent(&selChanged);
          selectionStateChanged |= selChanged;
        }
      }
    }
  }
  if (clickedLayerable && mInteractions.testFlag(clickedLayerable->selectionCategory()))
  {
    // a layerable was actually clicked, call its selectEvent:
    bool selChanged = false;
    clickedLayerable->selectEvent(event, additive, details, &selChanged);
    selectionStateChanged |= selChanged;
  }
  if (selectionStateChanged)
  {
    emit selectionChangedByUser();
    replot(rpQueuedReplot);
  }
}

/*! \internal
  
  Registers the specified plottable with this QCustomPlot and, if \ref setAutoAddPlottableToLegend
  is enabled, adds it to the legend (QCustomPlot::legend). QCustomPlot takes ownership of the
  plottable.
  
  Returns true on success, i.e. when \a plottable isn't already in this plot and the parent plot of
  \a plottable is this QCustomPlot.
  
  This method is called automatically in the QCPAbstractPlottable base class constructor.
*/
bool QCustomPlot::registerPlottable(QCPAbstractPlottable *plottable)
{
  if (mPlottables.contains(plottable))
  {
    qDebug() << Q_FUNC_INFO << "plottable already added to this QCustomPlot:" << reinterpret_cast<quintptr>(plottable);
    return false;
  }
  if (plottable->parentPlot() != this)
  {
    qDebug() << Q_FUNC_INFO << "plottable not created with this QCustomPlot as parent:" << reinterpret_cast<quintptr>(plottable);
    return false;
  }
  
  mPlottables.append(plottable);
  // possibly add plottable to legend:
  if (mAutoAddPlottableToLegend)
    plottable->addToLegend();
  if (!plottable->layer()) // usually the layer is already set in the constructor of the plottable (via QCPLayerable constructor)
    plottable->setLayer(currentLayer());
  return true;
}

/*! \internal
  
  In order to maintain the simplified graph interface of QCustomPlot, this method is called by the
  QCPGraph constructor to register itself with this QCustomPlot's internal graph list. Returns true
  on success, i.e. if \a graph is valid and wasn't already registered with this QCustomPlot.
  
  This graph specific registration happens in addition to the call to \ref registerPlottable by the
  QCPAbstractPlottable base class.
*/
bool QCustomPlot::registerGraph(QCPGraph *graph)
{
  if (!graph)
  {
    qDebug() << Q_FUNC_INFO << "passed graph is zero";
    return false;
  }
  if (mGraphs.contains(graph))
  {
    qDebug() << Q_FUNC_INFO << "graph already registered with this QCustomPlot";
    return false;
  }
  
  mGraphs.append(graph);
  return true;
}


/*! \internal

  Registers the specified item with this QCustomPlot. QCustomPlot takes ownership of the item.
  
  Returns true on success, i.e. when \a item wasn't already in the plot and the parent plot of \a
  item is this QCustomPlot.
  
  This method is called automatically in the QCPAbstractItem base class constructor.
*/
bool QCustomPlot::registerItem(QCPAbstractItem *item)
{
  if (mItems.contains(item))
  {
    qDebug() << Q_FUNC_INFO << "item already added to this QCustomPlot:" << reinterpret_cast<quintptr>(item);
    return false;
  }
  if (item->parentPlot() != this)
  {
    qDebug() << Q_FUNC_INFO << "item not created with this QCustomPlot as parent:" << reinterpret_cast<quintptr>(item);
    return false;
  }
  
  mItems.append(item);
  if (!item->layer()) // usually the layer is already set in the constructor of the item (via QCPLayerable constructor)
    item->setLayer(currentLayer());
  return true;
}

/*! \internal
  
  Assigns all layers their index (QCPLayer::mIndex) in the mLayers list. This method is thus called
  after every operation that changes the layer indices, like layer removal, layer creation, layer
  moving.
*/
void QCustomPlot::updateLayerIndices() const
{
  for (int i=0; i<mLayers.size(); ++i)
    mLayers.at(i)->mIndex = i;
}

/*! \internal

  Returns the top-most layerable at pixel position \a pos. If \a onlySelectable is set to true,
  only those layerables that are selectable will be considered. (Layerable subclasses communicate
  their selectability via the QCPLayerable::selectTest method, by returning -1.)

  \a selectionDetails is an output parameter that contains selection specifics of the affected
  layerable. This is useful if the respective layerable shall be given a subsequent
  QCPLayerable::selectEvent (like in \ref mouseReleaseEvent). \a selectionDetails usually contains
  information about which part of the layerable was hit, in multi-part layerables (e.g.
  QCPAxis::SelectablePart). If the layerable is a plottable, \a selectionDetails contains a \ref
  QCPDataSelection instance with the single data point which is closest to \a pos.
  
  \see layerableListAt, layoutElementAt, axisRectAt
*/
QCPLayerable *QCustomPlot::layerableAt(const QPointF &pos, bool onlySelectable, QVariant *selectionDetails) const
{
  QList<QVariant> details;
  QList<QCPLayerable*> candidates = layerableListAt(pos, onlySelectable, selectionDetails ? &details : nullptr);
  if (selectionDetails && !details.isEmpty())
    *selectionDetails = details.first();
  if (!candidates.isEmpty())
    return candidates.first();
  else
    return nullptr;
}

/*! \internal

  Returns the layerables at pixel position \a pos. If \a onlySelectable is set to true, only those
  layerables that are selectable will be considered. (Layerable subclasses communicate their
  selectability via the QCPLayerable::selectTest method, by returning -1.)

  The returned list is sorted by the layerable/drawing order such that the layerable that appears
  on top in the plot is at index 0 of the returned list. If you only need to know the top
  layerable, rather use \ref layerableAt.

  \a selectionDetails is an output parameter that contains selection specifics of the affected
  layerable. This is useful if the respective layerable shall be given a subsequent
  QCPLayerable::selectEvent (like in \ref mouseReleaseEvent). \a selectionDetails usually contains
  information about which part of the layerable was hit, in multi-part layerables (e.g.
  QCPAxis::SelectablePart). If the layerable is a plottable, \a selectionDetails contains a \ref
  QCPDataSelection instance with the single data point which is closest to \a pos.
  
  \see layerableAt, layoutElementAt, axisRectAt
*/
QList<QCPLayerable*> QCustomPlot::layerableListAt(const QPointF &pos, bool onlySelectable, QList<QVariant> *selectionDetails) const
{
  QList<QCPLayerable*> result;
  for (int layerIndex=mLayers.size()-1; layerIndex>=0; --layerIndex)
  {
    const QList<QCPLayerable*> layerables = mLayers.at(layerIndex)->children();
    for (int i=layerables.size()-1; i>=0; --i)
    {
      if (!layerables.at(i)->realVisibility())
        continue;
      QVariant details;
      double dist = layerables.at(i)->selectTest(pos, onlySelectable, selectionDetails ? &details : nullptr);
      if (dist >= 0 && dist < selectionTolerance())
      {
        result.append(layerables.at(i));
        if (selectionDetails)
          selectionDetails->append(details);
      }
    }
  }
  return result;
}

/*!
  Saves the plot to a rastered image file \a fileName in the image format \a format. The plot is
  sized to \a width and \a height in pixels and scaled with \a scale. (width 100 and scale 2.0 lead
  to a full resolution file with width 200.) If the \a format supports compression, \a quality may
  be between 0 and 100 to control it.

  Returns true on success. If this function fails, most likely the given \a format isn't supported
  by the system, see Qt docs about QImageWriter::supportedImageFormats().

  The \a resolution will be written to the image file header (if the file format supports this) and
  has no direct consequence for the quality or the pixel size. However, if opening the image with a
  tool which respects the metadata, it will be able to scale the image to match either a given size
  in real units of length (inch, centimeters, etc.), or the target display DPI. You can specify in
  which units \a resolution is given, by setting \a resolutionUnit. The \a resolution is converted
  to the format's expected resolution unit internally.

  \see saveBmp, saveJpg, savePng, savePdf
*/
bool QCustomPlot::saveRastered(const QString &fileName, int width, int height, double scale, const char *format, int quality, int resolution, QCP::ResolutionUnit resolutionUnit)
{
  QImage buffer = toPixmap(width, height, scale).toImage();
  
  int dotsPerMeter = 0;
  switch (resolutionUnit)
  {
    case QCP::ruDotsPerMeter: dotsPerMeter = resolution; break;
    case QCP::ruDotsPerCentimeter: dotsPerMeter = resolution*100; break;
    case QCP::ruDotsPerInch: dotsPerMeter = int(resolution/0.0254); break;
  }
  buffer.setDotsPerMeterX(dotsPerMeter); // this is saved together with some image formats, e.g. PNG, and is relevant when opening image in other tools
  buffer.setDotsPerMeterY(dotsPerMeter); // this is saved together with some image formats, e.g. PNG, and is relevant when opening image in other tools
  if (!buffer.isNull())
    return buffer.save(fileName, format, quality);
  else
    return false;
}

/*!
  Renders the plot to a pixmap and returns it.
  
  The plot is sized to \a width and \a height in pixels and scaled with \a scale. (width 100 and
  scale 2.0 lead to a full resolution pixmap with width 200.)
  
  \see toPainter, saveRastered, saveBmp, savePng, saveJpg, savePdf
*/
QPixmap QCustomPlot::toPixmap(int width, int height, double scale)
{
  // this method is somewhat similar to toPainter. Change something here, and a change in toPainter might be necessary, too.
  int newWidth, newHeight;
  if (width == 0 || height == 0)
  {
    newWidth = this->width();
    newHeight = this->height();
  } else
  {
    newWidth = width;
    newHeight = height;
  }
  int scaledWidth = qRound(scale*newWidth);
  int scaledHeight = qRound(scale*newHeight);

  QPixmap result(scaledWidth, scaledHeight);
  result.fill(mBackgroundBrush.style() == Qt::SolidPattern ? mBackgroundBrush.color() : Qt::transparent); // if using non-solid pattern, make transparent now and draw brush pattern later
  QCPPainter painter;
  painter.begin(&result);
  if (painter.isActive())
  {
    QRect oldViewport = viewport();
    setViewport(QRect(0, 0, newWidth, newHeight));
    painter.setMode(QCPPainter::pmNoCaching);
    if (!qFuzzyCompare(scale, 1.0))
    {
      if (scale > 1.0) // for scale < 1 we always want cosmetic pens where possible, because else lines might disappear for very small scales
        painter.setMode(QCPPainter::pmNonCosmetic);
      painter.scale(scale, scale);
    }
    if (mBackgroundBrush.style() != Qt::SolidPattern && mBackgroundBrush.style() != Qt::NoBrush) // solid fills were done a few lines above with QPixmap::fill
      painter.fillRect(mViewport, mBackgroundBrush);
    draw(&painter);
    setViewport(oldViewport);
    painter.end();
  } else // might happen if pixmap has width or height zero
  {
    qDebug() << Q_FUNC_INFO << "Couldn't activate painter on pixmap";
    return QPixmap();
  }
  return result;
}

/*!
  Renders the plot using the passed \a painter.
  
  The plot is sized to \a width and \a height in pixels. If the \a painter's scale is not 1.0, the resulting plot will
  appear scaled accordingly.
  
  \note If you are restricted to using a QPainter (instead of QCPPainter), create a temporary QPicture and open a QCPPainter
  on it. Then call \ref toPainter with this QCPPainter. After ending the paint operation on the picture, draw it with
  the QPainter. This will reproduce the painter actions the QCPPainter took, with a QPainter.
  
  \see toPixmap
*/
void QCustomPlot::toPainter(QCPPainter *painter, int width, int height)
{
  // this method is somewhat similar to toPixmap. Change something here, and a change in toPixmap might be necessary, too.
  int newWidth, newHeight;
  if (width == 0 || height == 0)
  {
    newWidth = this->width();
    newHeight = this->height();
  } else
  {
    newWidth = width;
    newHeight = height;
  }

  if (painter->isActive())
  {
    QRect oldViewport = viewport();
    setViewport(QRect(0, 0, newWidth, newHeight));
    painter->setMode(QCPPainter::pmNoCaching);
    if (mBackgroundBrush.style() != Qt::NoBrush) // unlike in toPixmap, we can't do QPixmap::fill for Qt::SolidPattern brush style, so we also draw solid fills with fillRect here
      painter->fillRect(mViewport, mBackgroundBrush);
    draw(painter);
    setViewport(oldViewport);
  } else
    qDebug() << Q_FUNC_INFO << "Passed painter is not active";
}
