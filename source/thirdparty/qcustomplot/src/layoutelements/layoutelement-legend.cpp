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

#include "layoutelement-legend.h"

#include "../painter.h"
#include "../core.h"
#include "../plottable.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAbstractLegendItem
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPAbstractLegendItem
  \brief The abstract base class for all entries in a QCPLegend.
  
  It defines a very basic interface for entries in a QCPLegend. For representing plottables in the
  legend, the subclass \ref QCPPlottableLegendItem is more suitable.
  
  Only derive directly from this class when you need absolute freedom (e.g. a custom legend entry
  that's not even associated with a plottable).

  You must implement the following pure virtual functions:
  \li \ref draw (from QCPLayerable)
  
  You inherit the following members you may use:
  <table>
    <tr>
      <td>QCPLegend *\b mParentLegend</td>
      <td>A pointer to the parent QCPLegend.</td>
    </tr><tr>
      <td>QFont \b mFont</td>
      <td>The generic font of the item. You should use this font for all or at least the most prominent text of the item.</td>
    </tr>
  </table>
*/

/* start of documentation of signals */

/*! \fn void QCPAbstractLegendItem::selectionChanged(bool selected)
  
  This signal is emitted when the selection state of this legend item has changed, either by user
  interaction or by a direct call to \ref setSelected.
*/

/* end of documentation of signals */

/*!
  Constructs a QCPAbstractLegendItem and associates it with the QCPLegend \a parent. This does not
  cause the item to be added to \a parent, so \ref QCPLegend::addItem must be called separately.
*/
QCPAbstractLegendItem::QCPAbstractLegendItem(QCPLegend *parent) :
  QCPLayoutElement(parent->parentPlot()),
  mParentLegend(parent),
  mFont(parent->font()),
  mTextColor(parent->textColor()),
  mSelectedFont(parent->selectedFont()),
  mSelectedTextColor(parent->selectedTextColor()),
  mSelectable(true),
  mSelected(false)
{
  setLayer(QLatin1String("legend"));
  setMargins(QMargins(0, 0, 0, 0));
}

/*!
  Sets the default font of this specific legend item to \a font.
  
  \see setTextColor, QCPLegend::setFont
*/
void QCPAbstractLegendItem::setFont(const QFont &font)
{
  mFont = font;
}

/*!
  Sets the default text color of this specific legend item to \a color.
  
  \see setFont, QCPLegend::setTextColor
*/
void QCPAbstractLegendItem::setTextColor(const QColor &color)
{
  mTextColor = color;
}

/*!
  When this legend item is selected, \a font is used to draw generic text, instead of the normal
  font set with \ref setFont.
  
  \see setFont, QCPLegend::setSelectedFont
*/
void QCPAbstractLegendItem::setSelectedFont(const QFont &font)
{
  mSelectedFont = font;
}

/*!
  When this legend item is selected, \a color is used to draw generic text, instead of the normal
  color set with \ref setTextColor.
  
  \see setTextColor, QCPLegend::setSelectedTextColor
*/
void QCPAbstractLegendItem::setSelectedTextColor(const QColor &color)
{
  mSelectedTextColor = color;
}

/*!
  Sets whether this specific legend item is selectable.
  
  \see setSelectedParts, QCustomPlot::setInteractions
*/
void QCPAbstractLegendItem::setSelectable(bool selectable)
{
  if (mSelectable != selectable)
  {
    mSelectable = selectable;
    emit selectableChanged(mSelectable);
  }
}

/*!
  Sets whether this specific legend item is selected.
  
  It is possible to set the selection state of this item by calling this function directly, even if
  setSelectable is set to false.
  
  \see setSelectableParts, QCustomPlot::setInteractions
*/
void QCPAbstractLegendItem::setSelected(bool selected)
{
  if (mSelected != selected)
  {
    mSelected = selected;
    emit selectionChanged(mSelected);
  }
}

/* inherits documentation from base class */
double QCPAbstractLegendItem::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (!mParentPlot) return -1;
  if (onlySelectable && (!mSelectable || !mParentLegend->selectableParts().testFlag(QCPLegend::spItems)))
    return -1;
  
  if (mRect.contains(pos.toPoint()))
    return mParentPlot->selectionTolerance()*0.99;
  else
    return -1;
}

/* inherits documentation from base class */
void QCPAbstractLegendItem::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeLegendItems);
}

/* inherits documentation from base class */
QRect QCPAbstractLegendItem::clipRect() const
{
  return mOuterRect;
}

/* inherits documentation from base class */
void QCPAbstractLegendItem::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  Q_UNUSED(details)
  if (mSelectable && mParentLegend->selectableParts().testFlag(QCPLegend::spItems))
  {
    bool selBefore = mSelected;
    setSelected(additive ? !mSelected : true);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}

/* inherits documentation from base class */
void QCPAbstractLegendItem::deselectEvent(bool *selectionStateChanged)
{
  if (mSelectable && mParentLegend->selectableParts().testFlag(QCPLegend::spItems))
  {
    bool selBefore = mSelected;
    setSelected(false);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPlottableLegendItem
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPlottableLegendItem
  \brief A legend item representing a plottable with an icon and the plottable name.
  
  This is the standard legend item for plottables. It displays an icon of the plottable next to the
  plottable name. The icon is drawn by the respective plottable itself (\ref
  QCPAbstractPlottable::drawLegendIcon), and tries to give an intuitive symbol for the plottable.
  For example, the QCPGraph draws a centered horizontal line and/or a single scatter point in the
  middle.
  
  Legend items of this type are always associated with one plottable (retrievable via the
  plottable() function and settable with the constructor). You may change the font of the plottable
  name with \ref setFont. Icon padding and border pen is taken from the parent QCPLegend, see \ref
  QCPLegend::setIconBorderPen and \ref QCPLegend::setIconTextPadding.

  The function \ref QCPAbstractPlottable::addToLegend/\ref QCPAbstractPlottable::removeFromLegend
  creates/removes legend items of this type.
  
  Since QCPLegend is based on QCPLayoutGrid, a legend item itself is just a subclass of
  QCPLayoutElement. While it could be added to a legend (or any other layout) via the normal layout
  interface, QCPLegend has specialized functions for handling legend items conveniently, see the
  documentation of \ref QCPLegend.
*/

/*!
  Creates a new legend item associated with \a plottable.
  
  Once it's created, it can be added to the legend via \ref QCPLegend::addItem.
  
  A more convenient way of adding/removing a plottable to/from the legend is via the functions \ref
  QCPAbstractPlottable::addToLegend and \ref QCPAbstractPlottable::removeFromLegend.
*/
QCPPlottableLegendItem::QCPPlottableLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable) :
  QCPAbstractLegendItem(parent),
  mPlottable(plottable)
{
  setAntialiased(false);
}

/*! \internal
  
  Returns the pen that shall be used to draw the icon border, taking into account the selection
  state of this item.
*/
QPen QCPPlottableLegendItem::getIconBorderPen() const
{
  return mSelected ? mParentLegend->selectedIconBorderPen() : mParentLegend->iconBorderPen();
}

/*! \internal
  
  Returns the text color that shall be used to draw text, taking into account the selection state
  of this item.
*/
QColor QCPPlottableLegendItem::getTextColor() const
{
  return mSelected ? mSelectedTextColor : mTextColor;
}

/*! \internal
  
  Returns the font that shall be used to draw text, taking into account the selection state of this
  item.
*/
QFont QCPPlottableLegendItem::getFont() const
{
  return mSelected ? mSelectedFont : mFont;
}

/*! \internal
  
  Draws the item with \a painter. The size and position of the drawn legend item is defined by the
  parent layout (typically a \ref QCPLegend) and the \ref minimumOuterSizeHint and \ref
  maximumOuterSizeHint of this legend item.
*/
void QCPPlottableLegendItem::draw(QCPPainter *painter)
{
  if (!mPlottable) return;
  painter->setFont(getFont());
  painter->setPen(QPen(getTextColor()));
  QSize iconSize = mParentLegend->iconSize();
  QRect textRect = painter->fontMetrics().boundingRect(0, 0, 0, iconSize.height(), Qt::TextDontClip, mPlottable->name());
  QRect iconRect(mRect.topLeft(), iconSize);
  int textHeight = qMax(textRect.height(), iconSize.height());  // if text has smaller height than icon, center text vertically in icon height, else align tops
  painter->drawText(mRect.x()+iconSize.width()+mParentLegend->iconTextPadding(), mRect.y(), textRect.width(), textHeight, Qt::TextDontClip, mPlottable->name());
  // draw icon:
  painter->save();
  painter->setClipRect(iconRect, Qt::IntersectClip);
  mPlottable->drawLegendIcon(painter, iconRect);
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

/*! \internal
  
  Calculates and returns the size of this item. This includes the icon, the text and the padding in
  between.
  
  \seebaseclassmethod
*/
QSize QCPPlottableLegendItem::minimumOuterSizeHint() const
{
  if (!mPlottable) return {};
  QSize result(0, 0);
  QRect textRect;
  QFontMetrics fontMetrics(getFont());
  QSize iconSize = mParentLegend->iconSize();
  textRect = fontMetrics.boundingRect(0, 0, 0, iconSize.height(), Qt::TextDontClip, mPlottable->name());
  result.setWidth(iconSize.width() + mParentLegend->iconTextPadding() + textRect.width());
  result.setHeight(qMax(textRect.height(), iconSize.height()));
  result.rwidth() += mMargins.left()+mMargins.right();
  result.rheight() += mMargins.top()+mMargins.bottom();
  return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLegend
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPLegend
  \brief Manages a legend inside a QCustomPlot.

  A legend is a small box somewhere in the plot which lists plottables with their name and icon.

  A legend is populated with legend items by calling \ref QCPAbstractPlottable::addToLegend on the
  plottable, for which a legend item shall be created. In the case of the main legend (\ref
  QCustomPlot::legend), simply adding plottables to the plot while \ref
  QCustomPlot::setAutoAddPlottableToLegend is set to true (the default) creates corresponding
  legend items. The legend item associated with a certain plottable can be removed with \ref
  QCPAbstractPlottable::removeFromLegend. However, QCPLegend also offers an interface to add and
  manipulate legend items directly: \ref item, \ref itemWithPlottable, \ref itemCount, \ref
  addItem, \ref removeItem, etc.

  Since \ref QCPLegend derives from \ref QCPLayoutGrid, it can be placed in any position a \ref
  QCPLayoutElement may be positioned. The legend items are themselves \ref QCPLayoutElement
  "QCPLayoutElements" which are placed in the grid layout of the legend. \ref QCPLegend only adds
  an interface specialized for handling child elements of type \ref QCPAbstractLegendItem, as
  mentioned above. In principle, any other layout elements may also be added to a legend via the
  normal \ref QCPLayoutGrid interface. See the special page about \link thelayoutsystem The Layout
  System\endlink for examples on how to add other elements to the legend and move it outside the axis
  rect.

  Use the methods \ref setFillOrder and \ref setWrap inherited from \ref QCPLayoutGrid to control
  in which order (column first or row first) the legend is filled up when calling \ref addItem, and
  at which column or row wrapping occurs. The default fill order for legends is \ref foRowsFirst.

  By default, every QCustomPlot has one legend (\ref QCustomPlot::legend) which is placed in the
  inset layout of the main axis rect (\ref QCPAxisRect::insetLayout). To move the legend to another
  position inside the axis rect, use the methods of the \ref QCPLayoutInset. To move the legend
  outside of the axis rect, place it anywhere else with the \ref QCPLayout/\ref QCPLayoutElement
  interface.
*/

/* start of documentation of signals */

/*! \fn void QCPLegend::selectionChanged(QCPLegend::SelectableParts selection);

  This signal is emitted when the selection state of this legend has changed.
  
  \see setSelectedParts, setSelectableParts
*/

/* end of documentation of signals */

/*!
  Constructs a new QCPLegend instance with default values.
  
  Note that by default, QCustomPlot already contains a legend ready to be used as \ref
  QCustomPlot::legend
*/
QCPLegend::QCPLegend() :
  mIconTextPadding{}
{
  setFillOrder(QCPLayoutGrid::foRowsFirst);
  setWrap(0);
  
  setRowSpacing(3);
  setColumnSpacing(8);
  setMargins(QMargins(7, 5, 7, 4));
  setAntialiased(false);
  setIconSize(32, 18);
  
  setIconTextPadding(7);
  
  setSelectableParts(spLegendBox | spItems);
  setSelectedParts(spNone);
  
  setBorderPen(QPen(Qt::black, 0));
  setSelectedBorderPen(QPen(Qt::blue, 2));
  setIconBorderPen(Qt::NoPen);
  setSelectedIconBorderPen(QPen(Qt::blue, 2));
  setBrush(Qt::white);
  setSelectedBrush(Qt::white);
  setTextColor(Qt::black);
  setSelectedTextColor(Qt::blue);
}

QCPLegend::~QCPLegend()
{
  clearItems();
  if (qobject_cast<QCustomPlot*>(mParentPlot)) // make sure this isn't called from QObject dtor when QCustomPlot is already destructed (happens when the legend is not in any layout and thus QObject-child of QCustomPlot)
    mParentPlot->legendRemoved(this);
}

/* no doc for getter, see setSelectedParts */
QCPLegend::SelectableParts QCPLegend::selectedParts() const
{
  // check whether any legend elements selected, if yes, add spItems to return value
  bool hasSelectedItems = false;
  for (int i=0; i<itemCount(); ++i)
  {
    if (item(i) && item(i)->selected())
    {
      hasSelectedItems = true;
      break;
    }
  }
  if (hasSelectedItems)
    return mSelectedParts | spItems;
  else
    return mSelectedParts & ~spItems;
}

/*!
  Sets the pen, the border of the entire legend is drawn with.
*/
void QCPLegend::setBorderPen(const QPen &pen)
{
  mBorderPen = pen;
}

/*!
  Sets the brush of the legend background.
*/
void QCPLegend::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  Sets the default font of legend text. Legend items that draw text (e.g. the name of a graph) will
  use this font by default. However, a different font can be specified on a per-item-basis by
  accessing the specific legend item.
  
  This function will also set \a font on all already existing legend items.
  
  \see QCPAbstractLegendItem::setFont
*/
void QCPLegend::setFont(const QFont &font)
{
  mFont = font;
  for (int i=0; i<itemCount(); ++i)
  {
    if (item(i))
      item(i)->setFont(mFont);
  }
}

/*!
  Sets the default color of legend text. Legend items that draw text (e.g. the name of a graph)
  will use this color by default. However, a different colors can be specified on a per-item-basis
  by accessing the specific legend item.
  
  This function will also set \a color on all already existing legend items.
  
  \see QCPAbstractLegendItem::setTextColor
*/
void QCPLegend::setTextColor(const QColor &color)
{
  mTextColor = color;
  for (int i=0; i<itemCount(); ++i)
  {
    if (item(i))
      item(i)->setTextColor(color);
  }
}

/*!
  Sets the size of legend icons. Legend items that draw an icon (e.g. a visual
  representation of the graph) will use this size by default.
*/
void QCPLegend::setIconSize(const QSize &size)
{
  mIconSize = size;
}

/*! \overload
*/
void QCPLegend::setIconSize(int width, int height)
{
  mIconSize.setWidth(width);
  mIconSize.setHeight(height);
}

/*!
  Sets the horizontal space in pixels between the legend icon and the text next to it.
  Legend items that draw an icon (e.g. a visual representation of the graph) and text (e.g. the
  name of the graph) will use this space by default.
*/
void QCPLegend::setIconTextPadding(int padding)
{
  mIconTextPadding = padding;
}

/*!
  Sets the pen used to draw a border around each legend icon. Legend items that draw an
  icon (e.g. a visual representation of the graph) will use this pen by default.
  
  If no border is wanted, set this to \a Qt::NoPen.
*/
void QCPLegend::setIconBorderPen(const QPen &pen)
{
  mIconBorderPen = pen;
}

/*!
  Sets whether the user can (de-)select the parts in \a selectable by clicking on the QCustomPlot surface.
  (When \ref QCustomPlot::setInteractions contains \ref QCP::iSelectLegend.)
  
  However, even when \a selectable is set to a value not allowing the selection of a specific part,
  it is still possible to set the selection of this part manually, by calling \ref setSelectedParts
  directly.
  
  \see SelectablePart, setSelectedParts
*/
void QCPLegend::setSelectableParts(const SelectableParts &selectable)
{
  if (mSelectableParts != selectable)
  {
    mSelectableParts = selectable;
    emit selectableChanged(mSelectableParts);
  }
}

/*!
  Sets the selected state of the respective legend parts described by \ref SelectablePart. When a part
  is selected, it uses a different pen/font and brush. If some legend items are selected and \a selected
  doesn't contain \ref spItems, those items become deselected.
  
  The entire selection mechanism is handled automatically when \ref QCustomPlot::setInteractions
  contains iSelectLegend. You only need to call this function when you wish to change the selection
  state manually.
  
  This function can change the selection state of a part even when \ref setSelectableParts was set to a
  value that actually excludes the part.
  
  emits the \ref selectionChanged signal when \a selected is different from the previous selection state.
  
  Note that it doesn't make sense to set the selected state \ref spItems here when it wasn't set
  before, because there's no way to specify which exact items to newly select. Do this by calling
  \ref QCPAbstractLegendItem::setSelected directly on the legend item you wish to select.
  
  \see SelectablePart, setSelectableParts, selectTest, setSelectedBorderPen, setSelectedIconBorderPen, setSelectedBrush,
  setSelectedFont
*/
void QCPLegend::setSelectedParts(const SelectableParts &selected)
{
  SelectableParts newSelected = selected;
  mSelectedParts = this->selectedParts(); // update mSelectedParts in case item selection changed

  if (mSelectedParts != newSelected)
  {
    if (!mSelectedParts.testFlag(spItems) && newSelected.testFlag(spItems)) // attempt to set spItems flag (can't do that)
    {
      qDebug() << Q_FUNC_INFO << "spItems flag can not be set, it can only be unset with this function";
      newSelected &= ~spItems;
    }
    if (mSelectedParts.testFlag(spItems) && !newSelected.testFlag(spItems)) // spItems flag was unset, so clear item selection
    {
      for (int i=0; i<itemCount(); ++i)
      {
        if (item(i))
          item(i)->setSelected(false);
      }
    }
    mSelectedParts = newSelected;
    emit selectionChanged(mSelectedParts);
  }
}

/*!
  When the legend box is selected, this pen is used to draw the border instead of the normal pen
  set via \ref setBorderPen.

  \see setSelectedParts, setSelectableParts, setSelectedBrush
*/
void QCPLegend::setSelectedBorderPen(const QPen &pen)
{
  mSelectedBorderPen = pen;
}

/*!
  Sets the pen legend items will use to draw their icon borders, when they are selected.

  \see setSelectedParts, setSelectableParts, setSelectedFont
*/
void QCPLegend::setSelectedIconBorderPen(const QPen &pen)
{
  mSelectedIconBorderPen = pen;
}

/*!
  When the legend box is selected, this brush is used to draw the legend background instead of the normal brush
  set via \ref setBrush.

  \see setSelectedParts, setSelectableParts, setSelectedBorderPen
*/
void QCPLegend::setSelectedBrush(const QBrush &brush)
{
  mSelectedBrush = brush;
}

/*!
  Sets the default font that is used by legend items when they are selected.
  
  This function will also set \a font on all already existing legend items.

  \see setFont, QCPAbstractLegendItem::setSelectedFont
*/
void QCPLegend::setSelectedFont(const QFont &font)
{
  mSelectedFont = font;
  for (int i=0; i<itemCount(); ++i)
  {
    if (item(i))
      item(i)->setSelectedFont(font);
  }
}

/*!
  Sets the default text color that is used by legend items when they are selected.
  
  This function will also set \a color on all already existing legend items.

  \see setTextColor, QCPAbstractLegendItem::setSelectedTextColor
*/
void QCPLegend::setSelectedTextColor(const QColor &color)
{
  mSelectedTextColor = color;
  for (int i=0; i<itemCount(); ++i)
  {
    if (item(i))
      item(i)->setSelectedTextColor(color);
  }
}

/*!
  Returns the item with index \a i. If non-legend items were added to the legend, and the element
  at the specified cell index is not a QCPAbstractLegendItem, returns \c nullptr.

  Note that the linear index depends on the current fill order (\ref setFillOrder).

  \see itemCount, addItem, itemWithPlottable
*/
QCPAbstractLegendItem *QCPLegend::item(int index) const
{
  return qobject_cast<QCPAbstractLegendItem*>(elementAt(index));
}

/*!
  Returns the QCPPlottableLegendItem which is associated with \a plottable (e.g. a \ref QCPGraph*).
  If such an item isn't in the legend, returns \c nullptr.
  
  \see hasItemWithPlottable
*/
QCPPlottableLegendItem *QCPLegend::itemWithPlottable(const QCPAbstractPlottable *plottable) const
{
  for (int i=0; i<itemCount(); ++i)
  {
    if (QCPPlottableLegendItem *pli = qobject_cast<QCPPlottableLegendItem*>(item(i)))
    {
      if (pli->plottable() == plottable)
        return pli;
    }
  }
  return nullptr;
}

/*!
  Returns the number of items currently in the legend. It is identical to the base class
  QCPLayoutGrid::elementCount(), and unlike the other "item" interface methods of QCPLegend,
  doesn't only address elements which can be cast to QCPAbstractLegendItem.

  Note that if empty cells are in the legend (e.g. by calling methods of the \ref QCPLayoutGrid
  base class which allows creating empty cells), they are included in the returned count.

  \see item
*/
int QCPLegend::itemCount() const
{
  return elementCount();
}

/*!
  Returns whether the legend contains \a item.
  
  \see hasItemWithPlottable
*/
bool QCPLegend::hasItem(QCPAbstractLegendItem *item) const
{
  for (int i=0; i<itemCount(); ++i)
  {
    if (item == this->item(i))
        return true;
  }
  return false;
}

/*!
  Returns whether the legend contains a QCPPlottableLegendItem which is associated with \a plottable (e.g. a \ref QCPGraph*).
  If such an item isn't in the legend, returns false.
  
  \see itemWithPlottable
*/
bool QCPLegend::hasItemWithPlottable(const QCPAbstractPlottable *plottable) const
{
  return itemWithPlottable(plottable);
}

/*!
  Adds \a item to the legend, if it's not present already. The element is arranged according to the
  current fill order (\ref setFillOrder) and wrapping (\ref setWrap).

  Returns true on sucess, i.e. if the item wasn't in the list already and has been successfuly added.

  The legend takes ownership of the item.

  \see removeItem, item, hasItem
*/
bool QCPLegend::addItem(QCPAbstractLegendItem *item)
{
  return addElement(item);
}

/*! \overload

  Removes the item with the specified \a index from the legend and deletes it.

  After successful removal, the legend is reordered according to the current fill order (\ref
  setFillOrder) and wrapping (\ref setWrap), so no empty cell remains where the removed \a item
  was. If you don't want this, rather use the raw element interface of \ref QCPLayoutGrid.

  Returns true, if successful. Unlike \ref QCPLayoutGrid::removeAt, this method only removes
  elements derived from \ref QCPAbstractLegendItem.

  \see itemCount, clearItems
*/
bool QCPLegend::removeItem(int index)
{
  if (QCPAbstractLegendItem *ali = item(index))
  {
    bool success = remove(ali);
    if (success)
      setFillOrder(fillOrder(), true); // gets rid of empty cell by reordering
    return success;
  } else
    return false;
}

/*! \overload

  Removes \a item from the legend and deletes it.

  After successful removal, the legend is reordered according to the current fill order (\ref
  setFillOrder) and wrapping (\ref setWrap), so no empty cell remains where the removed \a item
  was. If you don't want this, rather use the raw element interface of \ref QCPLayoutGrid.

  Returns true, if successful.

  \see clearItems
*/
bool QCPLegend::removeItem(QCPAbstractLegendItem *item)
{
  bool success = remove(item);
  if (success)
    setFillOrder(fillOrder(), true); // gets rid of empty cell by reordering
  return success;
}

/*!
  Removes all items from the legend.
*/
void QCPLegend::clearItems()
{
  for (int i=elementCount()-1; i>=0; --i)
  {
    if (item(i))
      removeAt(i); // don't use removeItem() because it would unnecessarily reorder the whole legend for each item
  }
  setFillOrder(fillOrder(), true); // get rid of empty cells by reordering once after all items are removed
}

/*!
  Returns the legend items that are currently selected. If no items are selected,
  the list is empty.
  
  \see QCPAbstractLegendItem::setSelected, setSelectable
*/
QList<QCPAbstractLegendItem *> QCPLegend::selectedItems() const
{
  QList<QCPAbstractLegendItem*> result;
  for (int i=0; i<itemCount(); ++i)
  {
    if (QCPAbstractLegendItem *ali = item(i))
    {
      if (ali->selected())
        result.append(ali);
    }
  }
  return result;
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing main legend elements.

  This is the antialiasing state the painter passed to the \ref draw method is in by default.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \seebaseclassmethod
  
  \see setAntialiased
*/
void QCPLegend::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeLegend);
}

/*! \internal
  
  Returns the pen used to paint the border of the legend, taking into account the selection state
  of the legend box.
*/
QPen QCPLegend::getBorderPen() const
{
  return mSelectedParts.testFlag(spLegendBox) ? mSelectedBorderPen : mBorderPen;
}

/*! \internal
  
  Returns the brush used to paint the background of the legend, taking into account the selection
  state of the legend box.
*/
QBrush QCPLegend::getBrush() const
{
  return mSelectedParts.testFlag(spLegendBox) ? mSelectedBrush : mBrush;
}

/*! \internal
  
  Draws the legend box with the provided \a painter. The individual legend items are layerables
  themselves, thus are drawn independently.
*/
void QCPLegend::draw(QCPPainter *painter)
{
  // draw background rect:
  painter->setBrush(getBrush());
  painter->setPen(getBorderPen());
  painter->drawRect(mOuterRect);
}

/* inherits documentation from base class */
double QCPLegend::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if (!mParentPlot) return -1;
  if (onlySelectable && !mSelectableParts.testFlag(spLegendBox))
    return -1;
  
  if (mOuterRect.contains(pos.toPoint()))
  {
    if (details) details->setValue(spLegendBox);
    return mParentPlot->selectionTolerance()*0.99;
  }
  return -1;
}

/* inherits documentation from base class */
void QCPLegend::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  mSelectedParts = selectedParts(); // in case item selection has changed
  if (details.value<SelectablePart>() == spLegendBox && mSelectableParts.testFlag(spLegendBox))
  {
    SelectableParts selBefore = mSelectedParts;
    setSelectedParts(additive ? mSelectedParts^spLegendBox : mSelectedParts|spLegendBox); // no need to unset spItems in !additive case, because they will be deselected by QCustomPlot (they're normal QCPLayerables with own deselectEvent)
    if (selectionStateChanged)
      *selectionStateChanged = mSelectedParts != selBefore;
  }
}

/* inherits documentation from base class */
void QCPLegend::deselectEvent(bool *selectionStateChanged)
{
  mSelectedParts = selectedParts(); // in case item selection has changed
  if (mSelectableParts.testFlag(spLegendBox))
  {
    SelectableParts selBefore = mSelectedParts;
    setSelectedParts(selectedParts() & ~spLegendBox);
    if (selectionStateChanged)
      *selectionStateChanged = mSelectedParts != selBefore;
  }
}

/* inherits documentation from base class */
QCP::Interaction QCPLegend::selectionCategory() const
{
  return QCP::iSelectLegend;
}

/* inherits documentation from base class */
QCP::Interaction QCPAbstractLegendItem::selectionCategory() const
{
  return QCP::iSelectLegend;
}

/* inherits documentation from base class */
void QCPLegend::parentPlotInitialized(QCustomPlot *parentPlot)
{
  if (parentPlot && !parentPlot->legend)
    parentPlot->legend = this;
}
