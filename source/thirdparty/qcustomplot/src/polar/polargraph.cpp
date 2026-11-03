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

#include "polargraph.h"

#include "../painter.h"
#include "../core.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPolarLegendItem
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPolarLegendItem
  \brief A legend item for polar plots

  \warning In this QCustomPlot version, polar plots are a tech preview. Expect documentation and
  functionality to be incomplete, as well as changing public interfaces in the future.
*/
QCPPolarLegendItem::QCPPolarLegendItem(QCPLegend *parent, QCPPolarGraph *graph) :
  QCPAbstractLegendItem(parent),
  mPolarGraph(graph)
{
  setAntialiased(false);
}

void QCPPolarLegendItem::draw(QCPPainter *painter)
{
  if (!mPolarGraph) return;
  painter->setFont(getFont());
  painter->setPen(QPen(getTextColor()));
  QSizeF iconSize = mParentLegend->iconSize();
  QRectF textRect = painter->fontMetrics().boundingRect(0, 0, 0, iconSize.height(), Qt::TextDontClip, mPolarGraph->name());
  QRectF iconRect(mRect.topLeft(), iconSize);
  int textHeight = qMax(textRect.height(), iconSize.height());  // if text has smaller height than icon, center text vertically in icon height, else align tops
  painter->drawText(mRect.x()+iconSize.width()+mParentLegend->iconTextPadding(), mRect.y(), textRect.width(), textHeight, Qt::TextDontClip, mPolarGraph->name());
  // draw icon:
  painter->save();
  painter->setClipRect(iconRect, Qt::IntersectClip);
  mPolarGraph->drawLegendIcon(painter, iconRect);
  painter->restore();
  // draw icon border:
  if (getIconBorderPen().style() != Qt::NoPen)
  {
    painter->setPen(getIconBorderPen());
    painter->setBrush(Qt::NoBrush);
    int halfPen = qCeil(painter->pen().widthF()*0.5)+1;
    painter->setClipRect(mOuterRect.adjusted(-halfPen, -halfPen, halfPen, halfPen)); // extend default clip rect so thicker pens (especially during selection) are not clipped
    painter->drawRect(iconRect);
  }
}

QSize QCPPolarLegendItem::minimumOuterSizeHint() const
{
  if (!mPolarGraph) return QSize();
  QSize result(0, 0);
  QRect textRect;
  QFontMetrics fontMetrics(getFont());
  QSize iconSize = mParentLegend->iconSize();
  textRect = fontMetrics.boundingRect(0, 0, 0, iconSize.height(), Qt::TextDontClip, mPolarGraph->name());
  result.setWidth(iconSize.width() + mParentLegend->iconTextPadding() + textRect.width());
  result.setHeight(qMax(textRect.height(), iconSize.height()));
  result.rwidth() += mMargins.left()+mMargins.right();
  result.rheight() += mMargins.top()+mMargins.bottom();
  return result;
}

QPen QCPPolarLegendItem::getIconBorderPen() const
{
  return mSelected ? mParentLegend->selectedIconBorderPen() : mParentLegend->iconBorderPen();
}

QColor QCPPolarLegendItem::getTextColor() const
{
  return mSelected ? mSelectedTextColor : mTextColor;
}

QFont QCPPolarLegendItem::getFont() const
{
  return mSelected ? mSelectedFont : mFont;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPolarGraph
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPolarGraph
  \brief A radial graph used to display data in polar plots

  \warning In this QCustomPlot version, polar plots are a tech preview. Expect documentation and
  functionality to be incomplete, as well as changing public interfaces in the future.
*/

/* start of documentation of inline functions */

// TODO

/* end of documentation of inline functions */

/*!
  Constructs a graph which uses \a keyAxis as its angular and \a valueAxis as its radial axis. \a
  keyAxis and \a valueAxis must reside in the same QCustomPlot, and the radial axis must be
  associated with the angular axis. If either of these restrictions is violated, a corresponding
  message is printed to the debug output (qDebug), the construction is not aborted, though.

  The created QCPPolarGraph is automatically registered with the QCustomPlot instance inferred from
  \a keyAxis. This QCustomPlot instance takes ownership of the QCPPolarGraph, so do not delete it
  manually but use QCPPolarAxisAngular::removeGraph() instead.

  To directly create a QCPPolarGraph inside a plot, you shoud use the QCPPolarAxisAngular::addGraph
  method.
*/
QCPPolarGraph::QCPPolarGraph(QCPPolarAxisAngular *keyAxis, QCPPolarAxisRadial *valueAxis) :
  QCPLayerable(keyAxis->parentPlot(), QString(), keyAxis),
  mDataContainer(new QCPGraphDataContainer),
  mName(),
  mAntialiasedFill(true),
  mAntialiasedScatters(true),
  mPen(Qt::black),
  mBrush(Qt::NoBrush),
  mPeriodic(true),
  mKeyAxis(keyAxis),
  mValueAxis(valueAxis),
  mSelectable(QCP::stWhole)
  //mSelectionDecorator(0) // TODO
{
  if (keyAxis->parentPlot() != valueAxis->parentPlot())
    qDebug() << Q_FUNC_INFO << "Parent plot of keyAxis is not the same as that of valueAxis.";
  
  mKeyAxis->registerPolarGraph(this);
  
  //setSelectionDecorator(new QCPSelectionDecorator); // TODO
  
  setPen(QPen(Qt::blue, 0));
  setBrush(Qt::NoBrush);
  setLineStyle(lsLine);
}

QCPPolarGraph::~QCPPolarGraph()
{
  /* TODO
  if (mSelectionDecorator)
  {
    delete mSelectionDecorator;
    mSelectionDecorator = 0;
  }
  */
}

/*!
   The name is the textual representation of this plottable as it is displayed in the legend
   (\ref QCPLegend). It may contain any UTF-8 characters, including newlines.
*/
void QCPPolarGraph::setName(const QString &name)
{
  mName = name;
}

/*!
  Sets whether fills of this plottable are drawn antialiased or not.
  
  Note that this setting may be overridden by \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
*/
void QCPPolarGraph::setAntialiasedFill(bool enabled)
{
  mAntialiasedFill = enabled;
}

/*!
  Sets whether the scatter symbols of this plottable are drawn antialiased or not.
  
  Note that this setting may be overridden by \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
*/
void QCPPolarGraph::setAntialiasedScatters(bool enabled)
{
  mAntialiasedScatters = enabled;
}

/*!
  The pen is used to draw basic lines that make up the plottable representation in the
  plot.
  
  For example, the \ref QCPGraph subclass draws its graph lines with this pen.

  \see setBrush
*/
void QCPPolarGraph::setPen(const QPen &pen)
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
void QCPPolarGraph::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

void QCPPolarGraph::setPeriodic(bool enabled)
{
  mPeriodic = enabled;
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
void QCPPolarGraph::setKeyAxis(QCPPolarAxisAngular *axis)
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
void QCPPolarGraph::setValueAxis(QCPPolarAxisRadial *axis)
{
  mValueAxis = axis;
}

/*!
  Sets whether and to which granularity this plottable can be selected.

  A selection can happen by clicking on the QCustomPlot surface (When \ref
  QCustomPlot::setInteractions contains \ref QCP::iSelectPlottables), by dragging a selection rect
  (When \ref QCustomPlot::setSelectionRectMode is \ref QCP::srmSelect), or programmatically by
  calling \ref setSelection.
  
  \see setSelection, QCP::SelectionType
*/
void QCPPolarGraph::setSelectable(QCP::SelectionType selectable)
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
void QCPPolarGraph::setSelection(QCPDataSelection selection)
{
  selection.enforceType(mSelectable);
  if (mSelection != selection)
  {
    mSelection = selection;
    emit selectionChanged(selected());
    emit selectionChanged(mSelection);
  }
}

/*! \overload
  
  Replaces the current data container with the provided \a data container.
  
  Since a QSharedPointer is used, multiple QCPPolarGraphs may share the same data container safely.
  Modifying the data in the container will then affect all graphs that share the container. Sharing
  can be achieved by simply exchanging the data containers wrapped in shared pointers:
  \snippet documentation/doc-code-snippets/mainwindow.cpp QCPPolarGraph-datasharing-1
  
  If you do not wish to share containers, but create a copy from an existing container, rather use
  the \ref QCPDataContainer<DataType>::set method on the graph's data container directly:
  \snippet documentation/doc-code-snippets/mainwindow.cpp QCPPolarGraph-datasharing-2
  
  \see addData
*/
void QCPPolarGraph::setData(QSharedPointer<QCPGraphDataContainer> data)
{
  mDataContainer = data;
}

/*! \overload
  
  Replaces the current data with the provided points in \a keys and \a values. The provided
  vectors should have equal length. Else, the number of added points will be the size of the
  smallest vector.
  
  If you can guarantee that the passed data points are sorted by \a keys in ascending order, you
  can set \a alreadySorted to true, to improve performance by saving a sorting run.
  
  \see addData
*/
void QCPPolarGraph::setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  mDataContainer->clear();
  addData(keys, values, alreadySorted);
}

/*!
  Sets how the single data points are connected in the plot. For scatter-only plots, set \a ls to
  \ref lsNone and \ref setScatterStyle to the desired scatter style.
  
  \see setScatterStyle
*/
void QCPPolarGraph::setLineStyle(LineStyle ls)
{
  mLineStyle = ls;
}

/*!
  Sets the visual appearance of single data points in the plot. If set to \ref QCPScatterStyle::ssNone, no scatter points
  are drawn (e.g. for line-only-plots with appropriate line style).
  
  \see QCPScatterStyle, setLineStyle
*/
void QCPPolarGraph::setScatterStyle(const QCPScatterStyle &style)
{
  mScatterStyle = style;
}

void QCPPolarGraph::addData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
  if (keys.size() != values.size())
    qDebug() << Q_FUNC_INFO << "keys and values have different sizes:" << keys.size() << values.size();
  const int n = qMin(keys.size(), values.size());
  QVector<QCPGraphData> tempData(n);
  QVector<QCPGraphData>::iterator it = tempData.begin();
  const QVector<QCPGraphData>::iterator itEnd = tempData.end();
  int i = 0;
  while (it != itEnd)
  {
    it->key = keys[i];
    it->value = values[i];
    ++it;
    ++i;
  }
  mDataContainer->add(tempData, alreadySorted); // don't modify tempData beyond this to prevent copy on write
}

void QCPPolarGraph::addData(double key, double value)
{
  mDataContainer->add(QCPGraphData(key, value));
}

/*!
  Use this method to set an own QCPSelectionDecorator (subclass) instance. This allows you to
  customize the visual representation of selected data ranges further than by using the default
  QCPSelectionDecorator.
  
  The plottable takes ownership of the \a decorator.
  
  The currently set decorator can be accessed via \ref selectionDecorator.
*/
/*
void QCPPolarGraph::setSelectionDecorator(QCPSelectionDecorator *decorator)
{
  if (decorator)
  {
    if (decorator->registerWithPlottable(this))
    {
      if (mSelectionDecorator) // delete old decorator if necessary
        delete mSelectionDecorator;
      mSelectionDecorator = decorator;
    }
  } else if (mSelectionDecorator) // just clear decorator
  {
    delete mSelectionDecorator;
    mSelectionDecorator = 0;
  }
}
*/

void QCPPolarGraph::coordsToPixels(double key, double value, double &x, double &y) const
{
  if (mValueAxis)
  {
    const QPointF point = mValueAxis->coordToPixel(key, value);
    x = point.x();
    y = point.y();
  } else
  {
    qDebug() << Q_FUNC_INFO << "invalid key or value axis";
  }
}

const QPointF QCPPolarGraph::coordsToPixels(double key, double value) const
{
  if (mValueAxis)
  {
    return mValueAxis->coordToPixel(key, value);
  } else
  {
    qDebug() << Q_FUNC_INFO << "invalid key or value axis";
    return QPointF();
  }
}

void QCPPolarGraph::pixelsToCoords(double x, double y, double &key, double &value) const
{
  if (mValueAxis)
  {
    mValueAxis->pixelToCoord(QPointF(x, y), key, value);
  } else
  {
    qDebug() << Q_FUNC_INFO << "invalid key or value axis";
  }
}

void QCPPolarGraph::pixelsToCoords(const QPointF &pixelPos, double &key, double &value) const
{
  if (mValueAxis)
  {
    mValueAxis->pixelToCoord(pixelPos, key, value);
  } else
  {
    qDebug() << Q_FUNC_INFO << "invalid key or value axis";
  }
}

void QCPPolarGraph::rescaleAxes(bool onlyEnlarge) const
{
  rescaleKeyAxis(onlyEnlarge);
  rescaleValueAxis(onlyEnlarge);
}

void QCPPolarGraph::rescaleKeyAxis(bool onlyEnlarge) const
{
  QCPPolarAxisAngular *keyAxis = mKeyAxis.data();
  if (!keyAxis) { qDebug() << Q_FUNC_INFO << "invalid key axis"; return; }
  
  bool foundRange;
  QCPRange newRange = getKeyRange(foundRange, QCP::sdBoth);
  if (foundRange)
  {
    if (onlyEnlarge)
      newRange.expand(keyAxis->range());
    if (!QCPRange::validRange(newRange)) // likely due to range being zero (plottable has only constant data in this axis dimension), shift current range to at least center the plottable
    {
      double center = (newRange.lower+newRange.upper)*0.5; // upper and lower should be equal anyway, but just to make sure, incase validRange returned false for other reason
      newRange.lower = center-keyAxis->range().size()/2.0;
      newRange.upper = center+keyAxis->range().size()/2.0;
    }
    keyAxis->setRange(newRange);
  }
}

void QCPPolarGraph::rescaleValueAxis(bool onlyEnlarge, bool inKeyRange) const
{
  QCPPolarAxisAngular *keyAxis = mKeyAxis.data();
  QCPPolarAxisRadial *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  QCP::SignDomain signDomain = QCP::sdBoth;
  if (valueAxis->scaleType() == QCPPolarAxisRadial::stLogarithmic)
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
      if (valueAxis->scaleType() == QCPPolarAxisRadial::stLinear)
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

bool QCPPolarGraph::addToLegend(QCPLegend *legend)
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
  
  //if (!legend->hasItemWithPlottable(this)) // TODO
  //{
    legend->addItem(new QCPPolarLegendItem(legend, this));
    return true;
  //} else
  //  return false;
}

bool QCPPolarGraph::addToLegend()
{
  if (!mParentPlot || !mParentPlot->legend)
    return false;
  else
    return addToLegend(mParentPlot->legend);
}

bool QCPPolarGraph::removeFromLegend(QCPLegend *legend) const
{
  if (!legend)
  {
    qDebug() << Q_FUNC_INFO << "passed legend is null";
    return false;
  }
  
  
  QCPPolarLegendItem *removableItem = 0;
  for (int i=0; i<legend->itemCount(); ++i) // TODO: reduce this to code in QCPAbstractPlottable::removeFromLegend once unified
  {
    if (QCPPolarLegendItem *pli = qobject_cast<QCPPolarLegendItem*>(legend->item(i)))
    {
      if (pli->polarGraph() == this)
      {
        removableItem = pli;
        break;
      }
    }
  }
  
  if (removableItem)
    return legend->removeItem(removableItem);
  else
    return false;
}

bool QCPPolarGraph::removeFromLegend() const
{
  if (!mParentPlot || !mParentPlot->legend)
    return false;
  else
    return removeFromLegend(mParentPlot->legend);
}

double QCPPolarGraph::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if ((onlySelectable && mSelectable == QCP::stNone) || mDataContainer->isEmpty())
    return -1;
  if (!mKeyAxis || !mValueAxis)
    return -1;
  
  if (mKeyAxis->rect().contains(pos.toPoint()))
  {
    QCPGraphDataContainer::const_iterator closestDataPoint = mDataContainer->constEnd();
    double result = pointDistance(pos, closestDataPoint);
    if (details)
    {
      int pointIndex = closestDataPoint-mDataContainer->constBegin();
      details->setValue(QCPDataSelection(QCPDataRange(pointIndex, pointIndex+1)));
    }
    return result;
  } else
    return -1;
}

/* inherits documentation from base class */
QCPRange QCPPolarGraph::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
  return mDataContainer->keyRange(foundRange, inSignDomain);
}

/* inherits documentation from base class */
QCPRange QCPPolarGraph::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain, const QCPRange &inKeyRange) const
{
  return mDataContainer->valueRange(foundRange, inSignDomain, inKeyRange);
}

/* inherits documentation from base class */
QRect QCPPolarGraph::clipRect() const
{
  if (mKeyAxis)
    return mKeyAxis.data()->rect();
  else
    return QRect();
}

void QCPPolarGraph::draw(QCPPainter *painter)
{
  if (!mKeyAxis || !mValueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  if (mKeyAxis.data()->range().size() <= 0 || mDataContainer->isEmpty()) return;
  if (mLineStyle == lsNone && mScatterStyle.isNone()) return;
  
  painter->setClipRegion(mKeyAxis->exactClipRegion());
  
  QVector<QPointF> lines, scatters; // line and (if necessary) scatter pixel coordinates will be stored here while iterating over segments
  
  // loop over and draw segments of unselected/selected data:
  QList<QCPDataRange> selectedSegments, unselectedSegments, allSegments;
  getDataSegments(selectedSegments, unselectedSegments);
  allSegments << unselectedSegments << selectedSegments;
  for (int i=0; i<allSegments.size(); ++i)
  {
    bool isSelectedSegment = i >= unselectedSegments.size();
    // get line pixel points appropriate to line style:
    QCPDataRange lineDataRange = isSelectedSegment ? allSegments.at(i) : allSegments.at(i).adjusted(-1, 1); // unselected segments extend lines to bordering selected data point (safe to exceed total data bounds in first/last segment, getLines takes care)
    getLines(&lines, lineDataRange);
    
    // check data validity if flag set:
#ifdef QCUSTOMPLOT_CHECK_DATA
    QCPGraphDataContainer::const_iterator it;
    for (it = mDataContainer->constBegin(); it != mDataContainer->constEnd(); ++it)
    {
      if (QCP::isInvalidData(it->key, it->value))
        qDebug() << Q_FUNC_INFO << "Data point at" << it->key << "invalid." << "Plottable name:" << name();
    }
#endif
    
    // draw fill of graph:
    //if (isSelectedSegment && mSelectionDecorator)
    //  mSelectionDecorator->applyBrush(painter);
    //else
      painter->setBrush(mBrush);
    painter->setPen(Qt::NoPen);
    drawFill(painter, &lines);
    
    
    // draw line:
    if (mLineStyle != lsNone)
    {
      //if (isSelectedSegment && mSelectionDecorator)
      //  mSelectionDecorator->applyPen(painter);
      //else
        painter->setPen(mPen);
      painter->setBrush(Qt::NoBrush);
      drawLinePlot(painter, lines);
    }
    
    // draw scatters:
    
    QCPScatterStyle finalScatterStyle = mScatterStyle;
    //if (isSelectedSegment && mSelectionDecorator)
    //  finalScatterStyle = mSelectionDecorator->getFinalScatterStyle(mScatterStyle);
    if (!finalScatterStyle.isNone())
    {
      getScatters(&scatters, allSegments.at(i));
      drawScatterPlot(painter, scatters, finalScatterStyle);
    }
  }
  
  // draw other selection decoration that isn't just line/scatter pens and brushes:
  //if (mSelectionDecorator)
  //  mSelectionDecorator->drawDecoration(painter, selection());
}

QCP::Interaction QCPPolarGraph::selectionCategory() const
{
  return QCP::iSelectPlottables;
}

void QCPPolarGraph::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aePlottables);
}

/* inherits documentation from base class */
void QCPPolarGraph::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
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
void QCPPolarGraph::deselectEvent(bool *selectionStateChanged)
{
  if (mSelectable != QCP::stNone)
  {
    QCPDataSelection selectionBefore = mSelection;
    setSelection(QCPDataSelection());
    if (selectionStateChanged)
      *selectionStateChanged = mSelection != selectionBefore;
  }
}

/*!  \internal
  
  Draws lines between the points in \a lines, given in pixel coordinates.
  
  \see drawScatterPlot, drawImpulsePlot, QCPAbstractPlottable1D::drawPolyline
*/
void QCPPolarGraph::drawLinePlot(QCPPainter *painter, const QVector<QPointF> &lines) const
{
  if (painter->pen().style() != Qt::NoPen && painter->pen().color().alpha() != 0)
  {
    applyDefaultAntialiasingHint(painter);
    drawPolyline(painter, lines);
  }
}

/*! \internal
  
  Draws the fill of the graph using the specified \a painter, with the currently set brush.
  
  Depending on whether a normal fill or a channel fill (\ref setChannelFillGraph) is needed, \ref
  getFillPolygon or \ref getChannelFillPolygon are used to find the according fill polygons.
  
  In order to handle NaN Data points correctly (the fill needs to be split into disjoint areas),
  this method first determines a list of non-NaN segments with \ref getNonNanSegments, on which to
  operate. In the channel fill case, \ref getOverlappingSegments is used to consolidate the non-NaN
  segments of the two involved graphs, before passing the overlapping pairs to \ref
  getChannelFillPolygon.
  
  Pass the points of this graph's line as \a lines, in pixel coordinates.

  \see drawLinePlot, drawImpulsePlot, drawScatterPlot
*/
void QCPPolarGraph::drawFill(QCPPainter *painter, QVector<QPointF> *lines) const
{
  applyFillAntialiasingHint(painter);
  if (painter->brush().style() != Qt::NoBrush && painter->brush().color().alpha() != 0)
    painter->drawPolygon(QPolygonF(*lines));
}

/*! \internal

  Draws scatter symbols at every point passed in \a scatters, given in pixel coordinates. The
  scatters will be drawn with \a painter and have the appearance as specified in \a style.

  \see drawLinePlot, drawImpulsePlot
*/
void QCPPolarGraph::drawScatterPlot(QCPPainter *painter, const QVector<QPointF> &scatters, const QCPScatterStyle &style) const
{
  applyScattersAntialiasingHint(painter);
  style.applyTo(painter, mPen);
  for (int i=0; i<scatters.size(); ++i)
    style.drawShape(painter, scatters.at(i).x(), scatters.at(i).y());
}

void QCPPolarGraph::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
  // draw fill:
  if (mBrush.style() != Qt::NoBrush)
  {
    applyFillAntialiasingHint(painter);
    painter->fillRect(QRectF(rect.left(), rect.top()+rect.height()/2.0, rect.width(), rect.height()/3.0), mBrush);
  }
  // draw line vertically centered:
  if (mLineStyle != lsNone)
  {
    applyDefaultAntialiasingHint(painter);
    painter->setPen(mPen);
    painter->drawLine(QLineF(rect.left(), rect.top()+rect.height()/2.0, rect.right()+5, rect.top()+rect.height()/2.0)); // +5 on x2 else last segment is missing from dashed/dotted pens
  }
  // draw scatter symbol:
  if (!mScatterStyle.isNone())
  {
    applyScattersAntialiasingHint(painter);
    // scale scatter pixmap if it's too large to fit in legend icon rect:
    if (mScatterStyle.shape() == QCPScatterStyle::ssPixmap && (mScatterStyle.pixmap().size().width() > rect.width() || mScatterStyle.pixmap().size().height() > rect.height()))
    {
      QCPScatterStyle scaledStyle(mScatterStyle);
      scaledStyle.setPixmap(scaledStyle.pixmap().scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
      scaledStyle.applyTo(painter, mPen);
      scaledStyle.drawShape(painter, QRectF(rect).center());
    } else
    {
      mScatterStyle.applyTo(painter, mPen);
      mScatterStyle.drawShape(painter, QRectF(rect).center());
    }
  }
}

void QCPPolarGraph::applyFillAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiasedFill, QCP::aeFills);
}

void QCPPolarGraph::applyScattersAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiasedScatters, QCP::aeScatters);
}

double QCPPolarGraph::pointDistance(const QPointF &pixelPoint, QCPGraphDataContainer::const_iterator &closestData) const
{
  closestData = mDataContainer->constEnd();
  if (mDataContainer->isEmpty())
    return -1.0;
  if (mLineStyle == lsNone && mScatterStyle.isNone())
    return -1.0;
  
  // calculate minimum distances to graph data points and find closestData iterator:
  double minDistSqr = (std::numeric_limits<double>::max)();
  // determine which key range comes into question, taking selection tolerance around pos into account:
  double posKeyMin, posKeyMax, dummy;
  pixelsToCoords(pixelPoint-QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMin, dummy);
  pixelsToCoords(pixelPoint+QPointF(mParentPlot->selectionTolerance(), mParentPlot->selectionTolerance()), posKeyMax, dummy);
  if (posKeyMin > posKeyMax)
    qSwap(posKeyMin, posKeyMax);
  // iterate over found data points and then choose the one with the shortest distance to pos:
  QCPGraphDataContainer::const_iterator begin = mDataContainer->findBegin(posKeyMin, true);
  QCPGraphDataContainer::const_iterator end = mDataContainer->findEnd(posKeyMax, true);
  for (QCPGraphDataContainer::const_iterator it=begin; it!=end; ++it)
  {
    const double currentDistSqr = QCPVector2D(coordsToPixels(it->key, it->value)-pixelPoint).lengthSquared();
    if (currentDistSqr < minDistSqr)
    {
      minDistSqr = currentDistSqr;
      closestData = it;
    }
  }
    
  // calculate distance to graph line if there is one (if so, will probably be smaller than distance to closest data point):
  if (mLineStyle != lsNone)
  {
    // line displayed, calculate distance to line segments:
    QVector<QPointF> lineData;
    getLines(&lineData, QCPDataRange(0, dataCount()));
    QCPVector2D p(pixelPoint);
    for (int i=0; i<lineData.size()-1; ++i)
    {
      const double currentDistSqr = p.distanceSquaredToLine(lineData.at(i), lineData.at(i+1));
      if (currentDistSqr < minDistSqr)
        minDistSqr = currentDistSqr;
    }
  }
  
  return qSqrt(minDistSqr);
}

int QCPPolarGraph::dataCount() const
{
  return mDataContainer->size();
}

void QCPPolarGraph::getDataSegments(QList<QCPDataRange> &selectedSegments, QList<QCPDataRange> &unselectedSegments) const
{
  selectedSegments.clear();
  unselectedSegments.clear();
  if (mSelectable == QCP::stWhole) // stWhole selection type draws the entire plottable with selected style if mSelection isn't empty
  {
    if (selected())
      selectedSegments << QCPDataRange(0, dataCount());
    else
      unselectedSegments << QCPDataRange(0, dataCount());
  } else
  {
    QCPDataSelection sel(selection());
    sel.simplify();
    selectedSegments = sel.dataRanges();
    unselectedSegments = sel.inverse(QCPDataRange(0, dataCount())).dataRanges();
  }
}

void QCPPolarGraph::drawPolyline(QCPPainter *painter, const QVector<QPointF> &lineData) const
{
  // if drawing solid line and not in PDF, use much faster line drawing instead of polyline:
  if (mParentPlot->plottingHints().testFlag(QCP::phFastPolylines) &&
      painter->pen().style() == Qt::SolidLine &&
      !painter->modes().testFlag(QCPPainter::pmVectorized) &&
      !painter->modes().testFlag(QCPPainter::pmNoCaching))
  {
    int i = 0;
    bool lastIsNan = false;
    const int lineDataSize = lineData.size();
    while (i < lineDataSize && (qIsNaN(lineData.at(i).y()) || qIsNaN(lineData.at(i).x()))) // make sure first point is not NaN
      ++i;
    ++i; // because drawing works in 1 point retrospect
    while (i < lineDataSize)
    {
      if (!qIsNaN(lineData.at(i).y()) && !qIsNaN(lineData.at(i).x())) // NaNs create a gap in the line
      {
        if (!lastIsNan)
          painter->drawLine(lineData.at(i-1), lineData.at(i));
        else
          lastIsNan = false;
      } else
        lastIsNan = true;
      ++i;
    }
  } else
  {
    int segmentStart = 0;
    int i = 0;
    const int lineDataSize = lineData.size();
    while (i < lineDataSize)
    {
      if (qIsNaN(lineData.at(i).y()) || qIsNaN(lineData.at(i).x()) || qIsInf(lineData.at(i).y())) // NaNs create a gap in the line. Also filter Infs which make drawPolyline block
      {
        painter->drawPolyline(lineData.constData()+segmentStart, i-segmentStart); // i, because we don't want to include the current NaN point
        segmentStart = i+1;
      }
      ++i;
    }
    // draw last segment:
    painter->drawPolyline(lineData.constData()+segmentStart, lineDataSize-segmentStart);
  }
}

void QCPPolarGraph::getVisibleDataBounds(QCPGraphDataContainer::const_iterator &begin, QCPGraphDataContainer::const_iterator &end, const QCPDataRange &rangeRestriction) const
{
  if (rangeRestriction.isEmpty())
  {
    end = mDataContainer->constEnd();
    begin = end;
  } else
  {
    QCPPolarAxisAngular *keyAxis = mKeyAxis.data();
    QCPPolarAxisRadial *valueAxis = mValueAxis.data();
    if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
    // get visible data range:
    if (mPeriodic)
    {
      begin = mDataContainer->constBegin();
      end = mDataContainer->constEnd();
    } else
    {
      begin = mDataContainer->findBegin(keyAxis->range().lower);
      end = mDataContainer->findEnd(keyAxis->range().upper);
    }
    // limit lower/upperEnd to rangeRestriction:
    mDataContainer->limitIteratorsToDataRange(begin, end, rangeRestriction); // this also ensures rangeRestriction outside data bounds doesn't break anything
  }
}

/*! \internal

  This method retrieves an optimized set of data points via \ref getOptimizedLineData, an branches
  out to the line style specific functions such as \ref dataToLines, \ref dataToStepLeftLines, etc.
  according to the line style of the graph.

  \a lines will be filled with points in pixel coordinates, that can be drawn with the according
  draw functions like \ref drawLinePlot and \ref drawImpulsePlot. The points returned in \a lines
  aren't necessarily the original data points. For example, step line styles require additional
  points to form the steps when drawn. If the line style of the graph is \ref lsNone, the \a
  lines vector will be empty.

  \a dataRange specifies the beginning and ending data indices that will be taken into account for
  conversion. In this function, the specified range may exceed the total data bounds without harm:
  a correspondingly trimmed data range will be used. This takes the burden off the user of this
  function to check for valid indices in \a dataRange, e.g. when extending ranges coming from \ref
  getDataSegments.

  \see getScatters
*/
void QCPPolarGraph::getLines(QVector<QPointF> *lines, const QCPDataRange &dataRange) const
{
  if (!lines) return;
  QCPGraphDataContainer::const_iterator begin, end;
  getVisibleDataBounds(begin, end, dataRange);
  if (begin == end)
  {
    lines->clear();
    return;
  }
  
  QVector<QCPGraphData> lineData;
  if (mLineStyle != lsNone)
    getOptimizedLineData(&lineData, begin, end);

  switch (mLineStyle)
  {
    case lsNone: lines->clear(); break;
    case lsLine: *lines = dataToLines(lineData); break;
  }
}

void QCPPolarGraph::getScatters(QVector<QPointF> *scatters, const QCPDataRange &dataRange) const
{
  QCPPolarAxisAngular *keyAxis = mKeyAxis.data();
  QCPPolarAxisRadial *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  if (!scatters) return;
  QCPGraphDataContainer::const_iterator begin, end;
  getVisibleDataBounds(begin, end, dataRange);
  if (begin == end)
  {
    scatters->clear();
    return;
  }
  
  QVector<QCPGraphData> data;
  getOptimizedScatterData(&data, begin, end);
  
  scatters->resize(data.size());
  for (int i=0; i<data.size(); ++i)
  {
    if (!qIsNaN(data.at(i).value))
      (*scatters)[i] = valueAxis->coordToPixel(data.at(i).key, data.at(i).value);
  }
}

void QCPPolarGraph::getOptimizedLineData(QVector<QCPGraphData> *lineData, const QCPGraphDataContainer::const_iterator &begin, const QCPGraphDataContainer::const_iterator &end) const
{
  lineData->clear();
  
  // TODO: fix for log axes and thick line style
  
  const QCPRange range = mValueAxis->range();
  bool reversed = mValueAxis->rangeReversed();
  const double clipMargin = range.size()*0.05; // extra distance from visible circle, so optimized outside lines can cover more angle before having to place a dummy point to prevent tangents
  const double upperClipValue = range.upper + (reversed ? 0 : range.size()*0.05+clipMargin); // clip slightly outside of actual range to avoid line thicknesses to peek into visible circle
  const double lowerClipValue = range.lower - (reversed ? range.size()*0.05+clipMargin : 0); // clip slightly outside of actual range to avoid line thicknesses to peek into visible circle
  const double maxKeySkip = qAsin(qSqrt(clipMargin*(clipMargin+2*range.size()))/(range.size()+clipMargin))/M_PI*mKeyAxis->range().size(); // the maximum angle between two points on outer circle (r=clipValue+clipMargin) before connecting line becomes tangent to inner circle (r=clipValue)
  double skipBegin = 0;
  bool belowRange = false;
  bool aboveRange = false;
  QCPGraphDataContainer::const_iterator it = begin;
  while (it != end)
  {
    if (it->value < lowerClipValue)
    {
      if (aboveRange) // jumped directly from above to below visible range, draw previous point so entry angle is correct
      {
        aboveRange = false;
        if (!reversed) // TODO: with inner radius, we'll need else case here with projected border point
          lineData->append(*(it-1));
      }
      if (!belowRange)
      {
        skipBegin = it->key;
        lineData->append(QCPGraphData(it->key, lowerClipValue));
        belowRange = true;
      }
      if (it->key-skipBegin > maxKeySkip) // add dummy point if we're exceeding the maximum skippable angle (to prevent unintentional intersections with visible circle)
      {
        skipBegin += maxKeySkip;
        lineData->append(QCPGraphData(skipBegin, lowerClipValue));
      }
    } else if (it->value > upperClipValue)
    {
      if (belowRange) // jumped directly from below to above visible range, draw previous point so entry angle is correct (if lower means outer, so if reversed axis)
      {
        belowRange = false;
        if (reversed)
          lineData->append(*(it-1));
      }
      if (!aboveRange)
      {
        skipBegin = it->key;
        lineData->append(QCPGraphData(it->key, upperClipValue));
        aboveRange = true;
      }
      if (it->key-skipBegin > maxKeySkip) // add dummy point if we're exceeding the maximum skippable angle (to prevent unintentional intersections with visible circle)
      {
        skipBegin += maxKeySkip;
        lineData->append(QCPGraphData(skipBegin, upperClipValue));
      }
    } else // value within bounds where we don't optimize away points
    {
      if (aboveRange)
      {
        aboveRange = false;
        if (!reversed)
          lineData->append(*(it-1)); // just entered from above, draw previous point so entry angle is correct (if above means outer, so if not reversed axis)
      }
      if (belowRange)
      {
        belowRange = false;
        if (reversed)
          lineData->append(*(it-1)); // just entered from below, draw previous point so entry angle is correct (if below means outer, so if reversed axis)
      }
      lineData->append(*it); // inside visible circle, add point normally
    }
    ++it;
  }
  // to make fill not erratic, add last point normally if it was outside visible circle:
  if (aboveRange)
  {
    aboveRange = false;
    if (!reversed)
      lineData->append(*(it-1)); // just entered from above, draw previous point so entry angle is correct (if above means outer, so if not reversed axis)
  }
  if (belowRange)
  {
    belowRange = false;
    if (reversed)
      lineData->append(*(it-1)); // just entered from below, draw previous point so entry angle is correct (if below means outer, so if reversed axis)
  }
}

void QCPPolarGraph::getOptimizedScatterData(QVector<QCPGraphData> *scatterData, QCPGraphDataContainer::const_iterator begin, QCPGraphDataContainer::const_iterator end) const
{
  scatterData->clear();
  
  const QCPRange range = mValueAxis->range();
  bool reversed = mValueAxis->rangeReversed();
  const double clipMargin = range.size()*0.05;
  const double upperClipValue = range.upper + (reversed ? 0 : clipMargin); // clip slightly outside of actual range to avoid scatter size to peek into visible circle
  const double lowerClipValue = range.lower - (reversed ? clipMargin : 0); // clip slightly outside of actual range to avoid scatter size to peek into visible circle
  QCPGraphDataContainer::const_iterator it = begin;
  while (it != end)
  {
    if (it->value > lowerClipValue && it->value < upperClipValue)
      scatterData->append(*it);
    ++it;
  }
}

/*! \internal

  Takes raw data points in plot coordinates as \a data, and returns a vector containing pixel
  coordinate points which are suitable for drawing the line style \ref lsLine.
  
  The source of \a data is usually \ref getOptimizedLineData, and this method is called in \a
  getLines if the line style is set accordingly.

  \see dataToStepLeftLines, dataToStepRightLines, dataToStepCenterLines, dataToImpulseLines, getLines, drawLinePlot
*/
QVector<QPointF> QCPPolarGraph::dataToLines(const QVector<QCPGraphData> &data) const
{
  QVector<QPointF> result;
  QCPPolarAxisAngular *keyAxis = mKeyAxis.data();
  QCPPolarAxisRadial *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return result; }

  // transform data points to pixels:
  result.resize(data.size());
  for (int i=0; i<data.size(); ++i)
    result[i] = mValueAxis->coordToPixel(data.at(i).key, data.at(i).value);
  return result;
}
