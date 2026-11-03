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

#include "layoutelement-textelement.h"

#include "../painter.h"
#include "../core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPTextElement
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPTextElement
  \brief A layout element displaying a text

  The text may be specified with \ref setText, the formatting can be controlled with \ref setFont,
  \ref setTextColor, and \ref setTextFlags.

  A text element can be added as follows:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcptextelement-creation
*/

/* start documentation of signals */

/*! \fn void QCPTextElement::selectionChanged(bool selected)
  
  This signal is emitted when the selection state has changed to \a selected, either by user
  interaction or by a direct call to \ref setSelected.
  
  \see setSelected, setSelectable
*/

/*! \fn void QCPTextElement::clicked(QMouseEvent *event)

  This signal is emitted when the text element is clicked.

  \see doubleClicked, selectTest
*/

/*! \fn void QCPTextElement::doubleClicked(QMouseEvent *event)

  This signal is emitted when the text element is double clicked.

  \see clicked, selectTest
*/

/* end documentation of signals */

/*! \overload
  
  Creates a new QCPTextElement instance and sets default values. The initial text is empty (\ref
  setText).
*/
QCPTextElement::QCPTextElement(QCustomPlot *parentPlot) :
  QCPLayoutElement(parentPlot),
  mText(),
  mTextFlags(Qt::AlignCenter),
  mFont(QFont(QLatin1String("sans serif"), 12)), // will be taken from parentPlot if available, see below
  mTextColor(Qt::black),
  mSelectedFont(QFont(QLatin1String("sans serif"), 12)), // will be taken from parentPlot if available, see below
  mSelectedTextColor(Qt::blue),
  mSelectable(false),
  mSelected(false)
{
  if (parentPlot)
  {
    mFont = parentPlot->font();
    mSelectedFont = parentPlot->font();
  }
  setMargins(QMargins(2, 2, 2, 2));
}

/*! \overload
  
  Creates a new QCPTextElement instance and sets default values.

  The initial text is set to \a text.
*/
QCPTextElement::QCPTextElement(QCustomPlot *parentPlot, const QString &text) :
  QCPLayoutElement(parentPlot),
  mText(text),
  mTextFlags(Qt::AlignCenter),
  mFont(QFont(QLatin1String("sans serif"), 12)), // will be taken from parentPlot if available, see below
  mTextColor(Qt::black),
  mSelectedFont(QFont(QLatin1String("sans serif"), 12)), // will be taken from parentPlot if available, see below
  mSelectedTextColor(Qt::blue),
  mSelectable(false),
  mSelected(false)
{
  if (parentPlot)
  {
    mFont = parentPlot->font();
    mSelectedFont = parentPlot->font();
  }
  setMargins(QMargins(2, 2, 2, 2));
}

/*! \overload
  
  Creates a new QCPTextElement instance and sets default values.

  The initial text is set to \a text with \a pointSize.
*/
QCPTextElement::QCPTextElement(QCustomPlot *parentPlot, const QString &text, double pointSize) :
  QCPLayoutElement(parentPlot),
  mText(text),
  mTextFlags(Qt::AlignCenter),
  mFont(QFont(QLatin1String("sans serif"), int(pointSize))), // will be taken from parentPlot if available, see below
  mTextColor(Qt::black),
  mSelectedFont(QFont(QLatin1String("sans serif"), int(pointSize))), // will be taken from parentPlot if available, see below
  mSelectedTextColor(Qt::blue),
  mSelectable(false),
  mSelected(false)
{
  mFont.setPointSizeF(pointSize); // set here again as floating point, because constructor above only takes integer
  if (parentPlot)
  {
    mFont = parentPlot->font();
    mFont.setPointSizeF(pointSize);
    mSelectedFont = parentPlot->font();
    mSelectedFont.setPointSizeF(pointSize);
  }
  setMargins(QMargins(2, 2, 2, 2));
}

/*! \overload
  
  Creates a new QCPTextElement instance and sets default values.

  The initial text is set to \a text with \a pointSize and the specified \a fontFamily.
*/
QCPTextElement::QCPTextElement(QCustomPlot *parentPlot, const QString &text, const QString &fontFamily, double pointSize) :
  QCPLayoutElement(parentPlot),
  mText(text),
  mTextFlags(Qt::AlignCenter),
  mFont(QFont(fontFamily, int(pointSize))),
  mTextColor(Qt::black),
  mSelectedFont(QFont(fontFamily, int(pointSize))),
  mSelectedTextColor(Qt::blue),
  mSelectable(false),
  mSelected(false)
{
  mFont.setPointSizeF(pointSize); // set here again as floating point, because constructor above only takes integer
  setMargins(QMargins(2, 2, 2, 2));
}

/*! \overload
  
  Creates a new QCPTextElement instance and sets default values.

  The initial text is set to \a text with the specified \a font.
*/
QCPTextElement::QCPTextElement(QCustomPlot *parentPlot, const QString &text, const QFont &font) :
  QCPLayoutElement(parentPlot),
  mText(text),
  mTextFlags(Qt::AlignCenter),
  mFont(font),
  mTextColor(Qt::black),
  mSelectedFont(font),
  mSelectedTextColor(Qt::blue),
  mSelectable(false),
  mSelected(false)
{
  setMargins(QMargins(2, 2, 2, 2));
}

/*!
  Sets the text that will be displayed to \a text. Multiple lines can be created by insertion of "\n".
  
  \see setFont, setTextColor, setTextFlags
*/
void QCPTextElement::setText(const QString &text)
{
  mText = text;
}

/*!
  Sets options for text alignment and wrapping behaviour. \a flags is a bitwise OR-combination of
  \c Qt::AlignmentFlag and \c Qt::TextFlag enums.
  
  Possible enums are:
  - Qt::AlignLeft
  - Qt::AlignRight
  - Qt::AlignHCenter
  - Qt::AlignJustify
  - Qt::AlignTop
  - Qt::AlignBottom
  - Qt::AlignVCenter
  - Qt::AlignCenter
  - Qt::TextDontClip
  - Qt::TextSingleLine
  - Qt::TextExpandTabs
  - Qt::TextShowMnemonic
  - Qt::TextWordWrap
  - Qt::TextIncludeTrailingSpaces
*/
void QCPTextElement::setTextFlags(int flags)
{
  mTextFlags = flags;
}

/*!
  Sets the \a font of the text.
  
  \see setTextColor, setSelectedFont
*/
void QCPTextElement::setFont(const QFont &font)
{
  mFont = font;
}

/*!
  Sets the \a color of the text.
  
  \see setFont, setSelectedTextColor
*/
void QCPTextElement::setTextColor(const QColor &color)
{
  mTextColor = color;
}

/*!
  Sets the \a font of the text that will be used if the text element is selected (\ref setSelected).
  
  \see setFont
*/
void QCPTextElement::setSelectedFont(const QFont &font)
{
  mSelectedFont = font;
}

/*!
  Sets the \a color of the text that will be used if the text element is selected (\ref setSelected).
  
  \see setTextColor
*/
void QCPTextElement::setSelectedTextColor(const QColor &color)
{
  mSelectedTextColor = color;
}

/*!
  Sets whether the user may select this text element.

  Note that even when \a selectable is set to <tt>false</tt>, the selection state may be changed
  programmatically via \ref setSelected.
*/
void QCPTextElement::setSelectable(bool selectable)
{
  if (mSelectable != selectable)
  {
    mSelectable = selectable;
    emit selectableChanged(mSelectable);
  }
}

/*!
  Sets the selection state of this text element to \a selected. If the selection has changed, \ref
  selectionChanged is emitted.
  
  Note that this function can change the selection state independently of the current \ref
  setSelectable state.
*/
void QCPTextElement::setSelected(bool selected)
{
  if (mSelected != selected)
  {
    mSelected = selected;
    emit selectionChanged(mSelected);
  }
}

/* inherits documentation from base class */
void QCPTextElement::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeOther);
}

/* inherits documentation from base class */
void QCPTextElement::draw(QCPPainter *painter)
{
  painter->setFont(mainFont());
  painter->setPen(QPen(mainTextColor()));
  painter->drawText(mRect, mTextFlags, mText, &mTextBoundingRect);
}

/* inherits documentation from base class */
QSize QCPTextElement::minimumOuterSizeHint() const
{
  QFontMetrics metrics(mFont);
  QSize result(metrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip, mText).size());
  result.rwidth() += mMargins.left()+mMargins.right();
  result.rheight() += mMargins.top()+mMargins.bottom();
  return result;
}

/* inherits documentation from base class */
QSize QCPTextElement::maximumOuterSizeHint() const
{
  QFontMetrics metrics(mFont);
  QSize result(metrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip, mText).size());
  result.setWidth(QWIDGETSIZE_MAX);
  result.rheight() += mMargins.top()+mMargins.bottom();
  return result;
}

/* inherits documentation from base class */
void QCPTextElement::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  Q_UNUSED(details)
  if (mSelectable)
  {
    bool selBefore = mSelected;
    setSelected(additive ? !mSelected : true);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}

/* inherits documentation from base class */
void QCPTextElement::deselectEvent(bool *selectionStateChanged)
{
  if (mSelectable)
  {
    bool selBefore = mSelected;
    setSelected(false);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}

/*!
  Returns 0.99*selectionTolerance (see \ref QCustomPlot::setSelectionTolerance) when \a pos is
  within the bounding box of the text element's text. Note that this bounding box is updated in the
  draw call.

  If \a pos is outside the text's bounding box or if \a onlySelectable is true and this text
  element is not selectable (\ref setSelectable), returns -1.

  \seebaseclassmethod
*/
double QCPTextElement::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (onlySelectable && !mSelectable)
    return -1;
  
  if (mTextBoundingRect.contains(pos.toPoint()))
    return mParentPlot->selectionTolerance()*0.99;
  else
    return -1;
}

/*!
  Accepts the mouse event in order to emit the according click signal in the \ref
  mouseReleaseEvent.

  \seebaseclassmethod
*/
void QCPTextElement::mousePressEvent(QMouseEvent *event, const QVariant &details)
{
  Q_UNUSED(details)
  event->accept();
}

/*!
  Emits the \ref clicked signal if the cursor hasn't moved by more than a few pixels since the \ref
  mousePressEvent.

  \seebaseclassmethod
*/
void QCPTextElement::mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos)
{
  if ((QPointF(event->pos())-startPos).manhattanLength() <= 3)
    emit clicked(event);
}

/*!
  Emits the \ref doubleClicked signal.

  \seebaseclassmethod
*/
void QCPTextElement::mouseDoubleClickEvent(QMouseEvent *event, const QVariant &details)
{
  Q_UNUSED(details)
  emit doubleClicked(event);
}

/*! \internal
  
  Returns the main font to be used. This is mSelectedFont if \ref setSelected is set to
  <tt>true</tt>, else mFont is returned.
*/
QFont QCPTextElement::mainFont() const
{
  return mSelected ? mSelectedFont : mFont;
}

/*! \internal
  
  Returns the main color to be used. This is mSelectedTextColor if \ref setSelected is set to
  <tt>true</tt>, else mTextColor is returned.
*/
QColor QCPTextElement::mainTextColor() const
{
  return mSelected ? mSelectedTextColor : mTextColor;
}

