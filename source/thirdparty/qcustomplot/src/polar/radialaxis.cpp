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

#include "radialaxis.h"
#include "layoutelement-angularaxis.h"

#include "../painter.h"
#include "../core.h"
#include "../item.h"



////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPolarAxisRadial
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPolarAxisRadial
  \brief The radial axis inside a radial plot
  
  \warning In this QCustomPlot version, polar plots are a tech preview. Expect documentation and
  functionality to be incomplete, as well as changing public interfaces in the future.
  
  Each axis holds an instance of QCPAxisTicker which is used to generate the tick coordinates and
  tick labels. You can access the currently installed \ref ticker or set a new one (possibly one of
  the specialized subclasses, or your own subclass) via \ref setTicker. For details, see the
  documentation of QCPAxisTicker.
*/

/* start of documentation of inline functions */

/*! \fn QSharedPointer<QCPAxisTicker> QCPPolarAxisRadial::ticker() const

  Returns a modifiable shared pointer to the currently installed axis ticker. The axis ticker is
  responsible for generating the tick positions and tick labels of this axis. You can access the
  \ref QCPAxisTicker with this method and modify basic properties such as the approximate tick count
  (\ref QCPAxisTicker::setTickCount).

  You can gain more control over the axis ticks by setting a different \ref QCPAxisTicker subclass, see
  the documentation there. A new axis ticker can be set with \ref setTicker.

  Since the ticker is stored in the axis as a shared pointer, multiple axes may share the same axis
  ticker simply by passing the same shared pointer to multiple axes.

  \see setTicker
*/

/* end of documentation of inline functions */
/* start of documentation of signals */

/*! \fn void QCPPolarAxisRadial::rangeChanged(const QCPRange &newRange)

  This signal is emitted when the range of this axis has changed. You can connect it to the \ref
  setRange slot of another axis to communicate the new range to the other axis, in order for it to
  be synchronized.
  
  You may also manipulate/correct the range with \ref setRange in a slot connected to this signal.
  This is useful if for example a maximum range span shall not be exceeded, or if the lower/upper
  range shouldn't go beyond certain values (see \ref QCPRange::bounded). For example, the following
  slot would limit the x axis to ranges between 0 and 10:
  \code
  customPlot->xAxis->setRange(newRange.bounded(0, 10))
  \endcode
*/

/*! \fn void QCPPolarAxisRadial::rangeChanged(const QCPRange &newRange, const QCPRange &oldRange)
  \overload
  
  Additionally to the new range, this signal also provides the previous range held by the axis as
  \a oldRange.
*/

/*! \fn void QCPPolarAxisRadial::scaleTypeChanged(QCPPolarAxisRadial::ScaleType scaleType);
  
  This signal is emitted when the scale type changes, by calls to \ref setScaleType
*/

/*! \fn void QCPPolarAxisRadial::selectionChanged(QCPPolarAxisRadial::SelectableParts selection)
  
  This signal is emitted when the selection state of this axis has changed, either by user interaction
  or by a direct call to \ref setSelectedParts.
*/

/*! \fn void QCPPolarAxisRadial::selectableChanged(const QCPPolarAxisRadial::SelectableParts &parts);
  
  This signal is emitted when the selectability changes, by calls to \ref setSelectableParts
*/

/* end of documentation of signals */

/*!
  Constructs an Axis instance of Type \a type for the axis rect \a parent.
  
  Usually it isn't necessary to instantiate axes directly, because you can let QCustomPlot create
  them for you with \ref QCPAxisRect::addAxis. If you want to use own QCPAxis-subclasses however,
  create them manually and then inject them also via \ref QCPAxisRect::addAxis.
*/
QCPPolarAxisRadial::QCPPolarAxisRadial(QCPPolarAxisAngular *parent) :
  QCPLayerable(parent->parentPlot(), QString(), parent),
  mRangeDrag(true),
  mRangeZoom(true),
  mRangeZoomFactor(0.85),
  // axis base:
  mAngularAxis(parent),
  mAngle(45),
  mAngleReference(arAngularAxis),
  mSelectableParts(spAxis | spTickLabels | spAxisLabel),
  mSelectedParts(spNone),
  mBasePen(QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap)),
  mSelectedBasePen(QPen(Qt::blue, 2)),
  // axis label:
  mLabelPadding(0),
  mLabel(),
  mLabelFont(mParentPlot->font()),
  mSelectedLabelFont(QFont(mLabelFont.family(), mLabelFont.pointSize(), QFont::Bold)),
  mLabelColor(Qt::black),
  mSelectedLabelColor(Qt::blue),
  // tick labels:
  // mTickLabelPadding(0), in label painter
  mTickLabels(true),
  // mTickLabelRotation(0), in label painter
  mTickLabelFont(mParentPlot->font()),
  mSelectedTickLabelFont(QFont(mTickLabelFont.family(), mTickLabelFont.pointSize(), QFont::Bold)),
  mTickLabelColor(Qt::black),
  mSelectedTickLabelColor(Qt::blue),
  mNumberPrecision(6),
  mNumberFormatChar('g'),
  mNumberBeautifulPowers(true),
  mNumberMultiplyCross(false),
  // ticks and subticks:
  mTicks(true),
  mSubTicks(true),
  mTickLengthIn(5),
  mTickLengthOut(0),
  mSubTickLengthIn(2),
  mSubTickLengthOut(0),
  mTickPen(QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap)),
  mSelectedTickPen(QPen(Qt::blue, 2)),
  mSubTickPen(QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap)),
  mSelectedSubTickPen(QPen(Qt::blue, 2)),
  // scale and range:
  mRange(0, 5),
  mRangeReversed(false),
  mScaleType(stLinear),
  // internal members:
  mRadius(1), // non-zero initial value, will be overwritten in ::update() according to inner rect
  mTicker(new QCPAxisTicker),
  mLabelPainter(mParentPlot)
{
  setParent(parent);
  setAntialiased(true);
  
  setTickLabelPadding(5);
  setTickLabelRotation(0);
  setTickLabelMode(lmUpright);
  mLabelPainter.setAnchorReferenceType(QCPLabelPainterPrivate::artTangent);
  mLabelPainter.setAbbreviateDecimalPowers(false);
}

QCPPolarAxisRadial::~QCPPolarAxisRadial()
{
}

QCPPolarAxisRadial::LabelMode QCPPolarAxisRadial::tickLabelMode() const
{
  switch (mLabelPainter.anchorMode())
  {
    case QCPLabelPainterPrivate::amSkewedUpright: return lmUpright;
    case QCPLabelPainterPrivate::amSkewedRotated: return lmRotated;
    default: qDebug() << Q_FUNC_INFO << "invalid mode for polar axis"; break;
  }
  return lmUpright;
}

/* No documentation as it is a property getter */
QString QCPPolarAxisRadial::numberFormat() const
{
  QString result;
  result.append(mNumberFormatChar);
  if (mNumberBeautifulPowers)
  {
    result.append(QLatin1Char('b'));
    if (mNumberMultiplyCross)
      result.append(QLatin1Char('c'));
  }
  return result;
}

/* No documentation as it is a property getter */
int QCPPolarAxisRadial::tickLengthIn() const
{
  return mTickLengthIn;
}

/* No documentation as it is a property getter */
int QCPPolarAxisRadial::tickLengthOut() const
{
  return mTickLengthOut;
}

/* No documentation as it is a property getter */
int QCPPolarAxisRadial::subTickLengthIn() const
{
  return mSubTickLengthIn;
}

/* No documentation as it is a property getter */
int QCPPolarAxisRadial::subTickLengthOut() const
{
  return mSubTickLengthOut;
}

/* No documentation as it is a property getter */
int QCPPolarAxisRadial::labelPadding() const
{
  return mLabelPadding;
}

void QCPPolarAxisRadial::setRangeDrag(bool enabled)
{
  mRangeDrag = enabled;
}

void QCPPolarAxisRadial::setRangeZoom(bool enabled)
{
  mRangeZoom = enabled;
}

void QCPPolarAxisRadial::setRangeZoomFactor(double factor)
{
  mRangeZoomFactor = factor;
}

/*!
  Sets whether the axis uses a linear scale or a logarithmic scale.
  
  Note that this method controls the coordinate transformation. For logarithmic scales, you will
  likely also want to use a logarithmic tick spacing and labeling, which can be achieved by setting
  the axis ticker to an instance of \ref QCPAxisTickerLog :
  
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpaxisticker-log-creation
  
  See the documentation of \ref QCPAxisTickerLog about the details of logarithmic axis tick
  creation.
  
  \ref setNumberPrecision
*/
void QCPPolarAxisRadial::setScaleType(QCPPolarAxisRadial::ScaleType type)
{
  if (mScaleType != type)
  {
    mScaleType = type;
    if (mScaleType == stLogarithmic)
      setRange(mRange.sanitizedForLogScale());
    //mCachedMarginValid = false;
    emit scaleTypeChanged(mScaleType);
  }
}

/*!
  Sets the range of the axis.
  
  This slot may be connected with the \ref rangeChanged signal of another axis so this axis
  is always synchronized with the other axis range, when it changes.
  
  To invert the direction of an axis, use \ref setRangeReversed.
*/
void QCPPolarAxisRadial::setRange(const QCPRange &range)
{
  if (range.lower == mRange.lower && range.upper == mRange.upper)
    return;
  
  if (!QCPRange::validRange(range)) return;
  QCPRange oldRange = mRange;
  if (mScaleType == stLogarithmic)
  {
    mRange = range.sanitizedForLogScale();
  } else
  {
    mRange = range.sanitizedForLinScale();
  }
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Sets whether the user can (de-)select the parts in \a selectable by clicking on the QCustomPlot surface.
  (When \ref QCustomPlot::setInteractions contains iSelectAxes.)
  
  However, even when \a selectable is set to a value not allowing the selection of a specific part,
  it is still possible to set the selection of this part manually, by calling \ref setSelectedParts
  directly.
  
  \see SelectablePart, setSelectedParts
*/
void QCPPolarAxisRadial::setSelectableParts(const SelectableParts &selectable)
{
  if (mSelectableParts != selectable)
  {
    mSelectableParts = selectable;
    emit selectableChanged(mSelectableParts);
  }
}

/*!
  Sets the selected state of the respective axis parts described by \ref SelectablePart. When a part
  is selected, it uses a different pen/font.
  
  The entire selection mechanism for axes is handled automatically when \ref
  QCustomPlot::setInteractions contains iSelectAxes. You only need to call this function when you
  wish to change the selection state manually.
  
  This function can change the selection state of a part, independent of the \ref setSelectableParts setting.
  
  emits the \ref selectionChanged signal when \a selected is different from the previous selection state.
  
  \see SelectablePart, setSelectableParts, selectTest, setSelectedBasePen, setSelectedTickPen, setSelectedSubTickPen,
  setSelectedTickLabelFont, setSelectedLabelFont, setSelectedTickLabelColor, setSelectedLabelColor
*/
void QCPPolarAxisRadial::setSelectedParts(const SelectableParts &selected)
{
  if (mSelectedParts != selected)
  {
    mSelectedParts = selected;
    emit selectionChanged(mSelectedParts);
  }
}

/*!
  \overload
  
  Sets the lower and upper bound of the axis range.
  
  To invert the direction of an axis, use \ref setRangeReversed.
  
  There is also a slot to set a range, see \ref setRange(const QCPRange &range).
*/
void QCPPolarAxisRadial::setRange(double lower, double upper)
{
  if (lower == mRange.lower && upper == mRange.upper)
    return;
  
  if (!QCPRange::validRange(lower, upper)) return;
  QCPRange oldRange = mRange;
  mRange.lower = lower;
  mRange.upper = upper;
  if (mScaleType == stLogarithmic)
  {
    mRange = mRange.sanitizedForLogScale();
  } else
  {
    mRange = mRange.sanitizedForLinScale();
  }
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  \overload
  
  Sets the range of the axis.
  
  The \a position coordinate indicates together with the \a alignment parameter, where the new
  range will be positioned. \a size defines the size of the new axis range. \a alignment may be
  Qt::AlignLeft, Qt::AlignRight or Qt::AlignCenter. This will cause the left border, right border,
  or center of the range to be aligned with \a position. Any other values of \a alignment will
  default to Qt::AlignCenter.
*/
void QCPPolarAxisRadial::setRange(double position, double size, Qt::AlignmentFlag alignment)
{
  if (alignment == Qt::AlignLeft)
    setRange(position, position+size);
  else if (alignment == Qt::AlignRight)
    setRange(position-size, position);
  else // alignment == Qt::AlignCenter
    setRange(position-size/2.0, position+size/2.0);
}

/*!
  Sets the lower bound of the axis range. The upper bound is not changed.
  \see setRange
*/
void QCPPolarAxisRadial::setRangeLower(double lower)
{
  if (mRange.lower == lower)
    return;
  
  QCPRange oldRange = mRange;
  mRange.lower = lower;
  if (mScaleType == stLogarithmic)
  {
    mRange = mRange.sanitizedForLogScale();
  } else
  {
    mRange = mRange.sanitizedForLinScale();
  }
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Sets the upper bound of the axis range. The lower bound is not changed.
  \see setRange
*/
void QCPPolarAxisRadial::setRangeUpper(double upper)
{
  if (mRange.upper == upper)
    return;
  
  QCPRange oldRange = mRange;
  mRange.upper = upper;
  if (mScaleType == stLogarithmic)
  {
    mRange = mRange.sanitizedForLogScale();
  } else
  {
    mRange = mRange.sanitizedForLinScale();
  }
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Sets whether the axis range (direction) is displayed reversed. Normally, the values on horizontal
  axes increase left to right, on vertical axes bottom to top. When \a reversed is set to true, the
  direction of increasing values is inverted.

  Note that the range and data interface stays the same for reversed axes, e.g. the \a lower part
  of the \ref setRange interface will still reference the mathematically smaller number than the \a
  upper part.
*/
void QCPPolarAxisRadial::setRangeReversed(bool reversed)
{
  mRangeReversed = reversed;
}

void QCPPolarAxisRadial::setAngle(double degrees)
{
  mAngle = degrees;
}

void QCPPolarAxisRadial::setAngleReference(AngleReference reference)
{
  mAngleReference = reference;
}

/*!
  The axis ticker is responsible for generating the tick positions and tick labels. See the
  documentation of QCPAxisTicker for details on how to work with axis tickers.
  
  You can change the tick positioning/labeling behaviour of this axis by setting a different
  QCPAxisTicker subclass using this method. If you only wish to modify the currently installed axis
  ticker, access it via \ref ticker.
  
  Since the ticker is stored in the axis as a shared pointer, multiple axes may share the same axis
  ticker simply by passing the same shared pointer to multiple axes.
  
  \see ticker
*/
void QCPPolarAxisRadial::setTicker(QSharedPointer<QCPAxisTicker> ticker)
{
  if (ticker)
    mTicker = ticker;
  else
    qDebug() << Q_FUNC_INFO << "can not set 0 as axis ticker";
  // no need to invalidate margin cache here because produced tick labels are checked for changes in setupTickVector
}

/*!
  Sets whether tick marks are displayed.

  Note that setting \a show to false does not imply that tick labels are invisible, too. To achieve
  that, see \ref setTickLabels.
  
  \see setSubTicks
*/
void QCPPolarAxisRadial::setTicks(bool show)
{
  if (mTicks != show)
  {
    mTicks = show;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets whether tick labels are displayed. Tick labels are the numbers drawn next to tick marks.
*/
void QCPPolarAxisRadial::setTickLabels(bool show)
{
  if (mTickLabels != show)
  {
    mTickLabels = show;
    //mCachedMarginValid = false;
    if (!mTickLabels)
      mTickVectorLabels.clear();
  }
}

/*!
  Sets the distance between the axis base line (including any outward ticks) and the tick labels.
  \see setLabelPadding, setPadding
*/
void QCPPolarAxisRadial::setTickLabelPadding(int padding)
{
  mLabelPainter.setPadding(padding);
}

/*!
  Sets the font of the tick labels.
  
  \see setTickLabels, setTickLabelColor
*/
void QCPPolarAxisRadial::setTickLabelFont(const QFont &font)
{
  if (font != mTickLabelFont)
  {
    mTickLabelFont = font;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the color of the tick labels.
  
  \see setTickLabels, setTickLabelFont
*/
void QCPPolarAxisRadial::setTickLabelColor(const QColor &color)
{
  mTickLabelColor = color;
}

/*!
  Sets the rotation of the tick labels. If \a degrees is zero, the labels are drawn normally. Else,
  the tick labels are drawn rotated by \a degrees clockwise. The specified angle is bound to values
  from -90 to 90 degrees.
  
  If \a degrees is exactly -90, 0 or 90, the tick labels are centered on the tick coordinate. For
  other angles, the label is drawn with an offset such that it seems to point toward or away from
  the tick mark.
*/
void QCPPolarAxisRadial::setTickLabelRotation(double degrees)
{
  mLabelPainter.setRotation(degrees);
}

void QCPPolarAxisRadial::setTickLabelMode(LabelMode mode)
{
  switch (mode)
  {
    case lmUpright: mLabelPainter.setAnchorMode(QCPLabelPainterPrivate::amSkewedUpright); break;
    case lmRotated: mLabelPainter.setAnchorMode(QCPLabelPainterPrivate::amSkewedRotated); break;
  }
}

/*!
  Sets the number format for the numbers in tick labels. This \a formatCode is an extended version
  of the format code used e.g. by QString::number() and QLocale::toString(). For reference about
  that, see the "Argument Formats" section in the detailed description of the QString class.
  
  \a formatCode is a string of one, two or three characters. The first character is identical to
  the normal format code used by Qt. In short, this means: 'e'/'E' scientific format, 'f' fixed
  format, 'g'/'G' scientific or fixed, whichever is shorter.
  
  The second and third characters are optional and specific to QCustomPlot:\n
  If the first char was 'e' or 'g', numbers are/might be displayed in the scientific format, e.g.
  "5.5e9", which is ugly in a plot. So when the second char of \a formatCode is set to 'b' (for
  "beautiful"), those exponential numbers are formatted in a more natural way, i.e. "5.5
  [multiplication sign] 10 [superscript] 9". By default, the multiplication sign is a centered dot.
  If instead a cross should be shown (as is usual in the USA), the third char of \a formatCode can
  be set to 'c'. The inserted multiplication signs are the UTF-8 characters 215 (0xD7) for the
  cross and 183 (0xB7) for the dot.
  
  Examples for \a formatCode:
  \li \c g normal format code behaviour. If number is small, fixed format is used, if number is large,
  normal scientific format is used
  \li \c gb If number is small, fixed format is used, if number is large, scientific format is used with
  beautifully typeset decimal powers and a dot as multiplication sign
  \li \c ebc All numbers are in scientific format with beautifully typeset decimal power and a cross as
  multiplication sign
  \li \c fb illegal format code, since fixed format doesn't support (or need) beautifully typeset decimal
  powers. Format code will be reduced to 'f'.
  \li \c hello illegal format code, since first char is not 'e', 'E', 'f', 'g' or 'G'. Current format
  code will not be changed.
*/
void QCPPolarAxisRadial::setNumberFormat(const QString &formatCode)
{
  if (formatCode.isEmpty())
  {
    qDebug() << Q_FUNC_INFO << "Passed formatCode is empty";
    return;
  }
  //mCachedMarginValid = false;
  
  // interpret first char as number format char:
  QString allowedFormatChars(QLatin1String("eEfgG"));
  if (allowedFormatChars.contains(formatCode.at(0)))
  {
    mNumberFormatChar = QLatin1Char(formatCode.at(0).toLatin1());
  } else
  {
    qDebug() << Q_FUNC_INFO << "Invalid number format code (first char not in 'eEfgG'):" << formatCode;
    return;
  }
  
  if (formatCode.length() < 2)
  {
    mNumberBeautifulPowers = false;
    mNumberMultiplyCross = false;
  } else
  {
    // interpret second char as indicator for beautiful decimal powers:
    if (formatCode.at(1) == QLatin1Char('b') && (mNumberFormatChar == QLatin1Char('e') || mNumberFormatChar == QLatin1Char('g')))
      mNumberBeautifulPowers = true;
    else
      qDebug() << Q_FUNC_INFO << "Invalid number format code (second char not 'b' or first char neither 'e' nor 'g'):" << formatCode;
    
    if (formatCode.length() < 3)
    {
      mNumberMultiplyCross = false;
    } else
    {
      // interpret third char as indicator for dot or cross multiplication symbol:
      if (formatCode.at(2) == QLatin1Char('c'))
        mNumberMultiplyCross = true;
      else if (formatCode.at(2) == QLatin1Char('d'))
        mNumberMultiplyCross = false;
      else
        qDebug() << Q_FUNC_INFO << "Invalid number format code (third char neither 'c' nor 'd'):" << formatCode;
    }
  }
  mLabelPainter.setSubstituteExponent(mNumberBeautifulPowers);
  mLabelPainter.setMultiplicationSymbol(mNumberMultiplyCross ? QCPLabelPainterPrivate::SymbolCross : QCPLabelPainterPrivate::SymbolDot);
}

/*!
  Sets the precision of the tick label numbers. See QLocale::toString(double i, char f, int prec)
  for details. The effect of precisions are most notably for number Formats starting with 'e', see
  \ref setNumberFormat
*/
void QCPPolarAxisRadial::setNumberPrecision(int precision)
{
  if (mNumberPrecision != precision)
  {
    mNumberPrecision = precision;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the length of the ticks in pixels. \a inside is the length the ticks will reach inside the
  plot and \a outside is the length they will reach outside the plot. If \a outside is greater than
  zero, the tick labels and axis label will increase their distance to the axis accordingly, so
  they won't collide with the ticks.
  
  \see setSubTickLength, setTickLengthIn, setTickLengthOut
*/
void QCPPolarAxisRadial::setTickLength(int inside, int outside)
{
  setTickLengthIn(inside);
  setTickLengthOut(outside);
}

/*!
  Sets the length of the inward ticks in pixels. \a inside is the length the ticks will reach
  inside the plot.
  
  \see setTickLengthOut, setTickLength, setSubTickLength
*/
void QCPPolarAxisRadial::setTickLengthIn(int inside)
{
  if (mTickLengthIn != inside)
  {
    mTickLengthIn = inside;
  }
}

/*!
  Sets the length of the outward ticks in pixels. \a outside is the length the ticks will reach
  outside the plot. If \a outside is greater than zero, the tick labels and axis label will
  increase their distance to the axis accordingly, so they won't collide with the ticks.
  
  \see setTickLengthIn, setTickLength, setSubTickLength
*/
void QCPPolarAxisRadial::setTickLengthOut(int outside)
{
  if (mTickLengthOut != outside)
  {
    mTickLengthOut = outside;
    //mCachedMarginValid = false; // only outside tick length can change margin
  }
}

/*!
  Sets whether sub tick marks are displayed.
  
  Sub ticks are only potentially visible if (major) ticks are also visible (see \ref setTicks)
  
  \see setTicks
*/
void QCPPolarAxisRadial::setSubTicks(bool show)
{
  if (mSubTicks != show)
  {
    mSubTicks = show;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the length of the subticks in pixels. \a inside is the length the subticks will reach inside
  the plot and \a outside is the length they will reach outside the plot. If \a outside is greater
  than zero, the tick labels and axis label will increase their distance to the axis accordingly,
  so they won't collide with the ticks.
  
  \see setTickLength, setSubTickLengthIn, setSubTickLengthOut
*/
void QCPPolarAxisRadial::setSubTickLength(int inside, int outside)
{
  setSubTickLengthIn(inside);
  setSubTickLengthOut(outside);
}

/*!
  Sets the length of the inward subticks in pixels. \a inside is the length the subticks will reach inside
  the plot.
  
  \see setSubTickLengthOut, setSubTickLength, setTickLength
*/
void QCPPolarAxisRadial::setSubTickLengthIn(int inside)
{
  if (mSubTickLengthIn != inside)
  {
    mSubTickLengthIn = inside;
  }
}

/*!
  Sets the length of the outward subticks in pixels. \a outside is the length the subticks will reach
  outside the plot. If \a outside is greater than zero, the tick labels will increase their
  distance to the axis accordingly, so they won't collide with the ticks.
  
  \see setSubTickLengthIn, setSubTickLength, setTickLength
*/
void QCPPolarAxisRadial::setSubTickLengthOut(int outside)
{
  if (mSubTickLengthOut != outside)
  {
    mSubTickLengthOut = outside;
    //mCachedMarginValid = false; // only outside tick length can change margin
  }
}

/*!
  Sets the pen, the axis base line is drawn with.
  
  \see setTickPen, setSubTickPen
*/
void QCPPolarAxisRadial::setBasePen(const QPen &pen)
{
  mBasePen = pen;
}

/*!
  Sets the pen, tick marks will be drawn with.
  
  \see setTickLength, setBasePen
*/
void QCPPolarAxisRadial::setTickPen(const QPen &pen)
{
  mTickPen = pen;
}

/*!
  Sets the pen, subtick marks will be drawn with.
  
  \see setSubTickCount, setSubTickLength, setBasePen
*/
void QCPPolarAxisRadial::setSubTickPen(const QPen &pen)
{
  mSubTickPen = pen;
}

/*!
  Sets the font of the axis label.
  
  \see setLabelColor
*/
void QCPPolarAxisRadial::setLabelFont(const QFont &font)
{
  if (mLabelFont != font)
  {
    mLabelFont = font;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the color of the axis label.
  
  \see setLabelFont
*/
void QCPPolarAxisRadial::setLabelColor(const QColor &color)
{
  mLabelColor = color;
}

/*!
  Sets the text of the axis label that will be shown below/above or next to the axis, depending on
  its orientation. To disable axis labels, pass an empty string as \a str.
*/
void QCPPolarAxisRadial::setLabel(const QString &str)
{
  if (mLabel != str)
  {
    mLabel = str;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the distance between the tick labels and the axis label.
  
  \see setTickLabelPadding, setPadding
*/
void QCPPolarAxisRadial::setLabelPadding(int padding)
{
  if (mLabelPadding != padding)
  {
    mLabelPadding = padding;
    //mCachedMarginValid = false;
  }
}

/*!
  Sets the font that is used for tick labels when they are selected.
  
  \see setTickLabelFont, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisRadial::setSelectedTickLabelFont(const QFont &font)
{
  if (font != mSelectedTickLabelFont)
  {
    mSelectedTickLabelFont = font;
    // don't set mCachedMarginValid to false here because margin calculation is always done with non-selected fonts
  }
}

/*!
  Sets the font that is used for the axis label when it is selected.
  
  \see setLabelFont, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisRadial::setSelectedLabelFont(const QFont &font)
{
  mSelectedLabelFont = font;
  // don't set mCachedMarginValid to false here because margin calculation is always done with non-selected fonts
}

/*!
  Sets the color that is used for tick labels when they are selected.
  
  \see setTickLabelColor, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisRadial::setSelectedTickLabelColor(const QColor &color)
{
  if (color != mSelectedTickLabelColor)
  {
    mSelectedTickLabelColor = color;
  }
}

/*!
  Sets the color that is used for the axis label when it is selected.
  
  \see setLabelColor, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisRadial::setSelectedLabelColor(const QColor &color)
{
  mSelectedLabelColor = color;
}

/*!
  Sets the pen that is used to draw the axis base line when selected.
  
  \see setBasePen, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisRadial::setSelectedBasePen(const QPen &pen)
{
  mSelectedBasePen = pen;
}

/*!
  Sets the pen that is used to draw the (major) ticks when selected.
  
  \see setTickPen, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisRadial::setSelectedTickPen(const QPen &pen)
{
  mSelectedTickPen = pen;
}

/*!
  Sets the pen that is used to draw the subticks when selected.
  
  \see setSubTickPen, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPPolarAxisRadial::setSelectedSubTickPen(const QPen &pen)
{
  mSelectedSubTickPen = pen;
}

/*!
  If the scale type (\ref setScaleType) is \ref stLinear, \a diff is added to the lower and upper
  bounds of the range. The range is simply moved by \a diff.
  
  If the scale type is \ref stLogarithmic, the range bounds are multiplied by \a diff. This
  corresponds to an apparent "linear" move in logarithmic scaling by a distance of log(diff).
*/
void QCPPolarAxisRadial::moveRange(double diff)
{
  QCPRange oldRange = mRange;
  if (mScaleType == stLinear)
  {
    mRange.lower += diff;
    mRange.upper += diff;
  } else // mScaleType == stLogarithmic
  {
    mRange.lower *= diff;
    mRange.upper *= diff;
  }
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Scales the range of this axis by \a factor around the center of the current axis range. For
  example, if \a factor is 2.0, then the axis range will double its size, and the point at the axis
  range center won't have changed its position in the QCustomPlot widget (i.e. coordinates around
  the center will have moved symmetrically closer).

  If you wish to scale around a different coordinate than the current axis range center, use the
  overload \ref scaleRange(double factor, double center).
*/
void QCPPolarAxisRadial::scaleRange(double factor)
{
  scaleRange(factor, range().center());
}

/*! \overload

  Scales the range of this axis by \a factor around the coordinate \a center. For example, if \a
  factor is 2.0, \a center is 1.0, then the axis range will double its size, and the point at
  coordinate 1.0 won't have changed its position in the QCustomPlot widget (i.e. coordinates
  around 1.0 will have moved symmetrically closer to 1.0).

  \see scaleRange(double factor)
*/
void QCPPolarAxisRadial::scaleRange(double factor, double center)
{
  QCPRange oldRange = mRange;
  if (mScaleType == stLinear)
  {
    QCPRange newRange;
    newRange.lower = (mRange.lower-center)*factor + center;
    newRange.upper = (mRange.upper-center)*factor + center;
    if (QCPRange::validRange(newRange))
      mRange = newRange.sanitizedForLinScale();
  } else // mScaleType == stLogarithmic
  {
    if ((mRange.upper < 0 && center < 0) || (mRange.upper > 0 && center > 0)) // make sure center has same sign as range
    {
      QCPRange newRange;
      newRange.lower = qPow(mRange.lower/center, factor)*center;
      newRange.upper = qPow(mRange.upper/center, factor)*center;
      if (QCPRange::validRange(newRange))
        mRange = newRange.sanitizedForLogScale();
    } else
      qDebug() << Q_FUNC_INFO << "Center of scaling operation doesn't lie in same logarithmic sign domain as range:" << center;
  }
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Changes the axis range such that all plottables associated with this axis are fully visible in
  that dimension.
  
  \see QCPAbstractPlottable::rescaleAxes, QCustomPlot::rescaleAxes
*/
void QCPPolarAxisRadial::rescale(bool onlyVisiblePlottables)
{
  Q_UNUSED(onlyVisiblePlottables)
  /* TODO
  QList<QCPAbstractPlottable*> p = plottables();
  QCPRange newRange;
  bool haveRange = false;
  for (int i=0; i<p.size(); ++i)
  {
    if (!p.at(i)->realVisibility() && onlyVisiblePlottables)
      continue;
    QCPRange plottableRange;
    bool currentFoundRange;
    QCP::SignDomain signDomain = QCP::sdBoth;
    if (mScaleType == stLogarithmic)
      signDomain = (mRange.upper < 0 ? QCP::sdNegative : QCP::sdPositive);
    if (p.at(i)->keyAxis() == this)
      plottableRange = p.at(i)->getKeyRange(currentFoundRange, signDomain);
    else
      plottableRange = p.at(i)->getValueRange(currentFoundRange, signDomain);
    if (currentFoundRange)
    {
      if (!haveRange)
        newRange = plottableRange;
      else
        newRange.expand(plottableRange);
      haveRange = true;
    }
  }
  if (haveRange)
  {
    if (!QCPRange::validRange(newRange)) // likely due to range being zero (plottable has only constant data in this axis dimension), shift current range to at least center the plottable
    {
      double center = (newRange.lower+newRange.upper)*0.5; // upper and lower should be equal anyway, but just to make sure, incase validRange returned false for other reason
      if (mScaleType == stLinear)
      {
        newRange.lower = center-mRange.size()/2.0;
        newRange.upper = center+mRange.size()/2.0;
      } else // mScaleType == stLogarithmic
      {
        newRange.lower = center/qSqrt(mRange.upper/mRange.lower);
        newRange.upper = center*qSqrt(mRange.upper/mRange.lower);
      }
    }
    setRange(newRange);
  }
  */
}

/*!
  Transforms \a value, in pixel coordinates of the QCustomPlot widget, to axis coordinates.
*/
void QCPPolarAxisRadial::pixelToCoord(QPointF pixelPos, double &angleCoord, double &radiusCoord) const
{
  QCPVector2D posVector(pixelPos-mCenter);
  radiusCoord = radiusToCoord(posVector.length());
  angleCoord = mAngularAxis->angleRadToCoord(posVector.angle());
}

/*!
  Transforms \a value, in coordinates of the axis, to pixel coordinates of the QCustomPlot widget.
*/
QPointF QCPPolarAxisRadial::coordToPixel(double angleCoord, double radiusCoord) const
{
  const double radiusPixel = coordToRadius(radiusCoord);
  const double angleRad = mAngularAxis->coordToAngleRad(angleCoord);
  return QPointF(mCenter.x()+qCos(angleRad)*radiusPixel, mCenter.y()+qSin(angleRad)*radiusPixel);
}

double QCPPolarAxisRadial::coordToRadius(double coord) const
{
  if (mScaleType == stLinear)
  {
    if (!mRangeReversed)
      return (coord-mRange.lower)/mRange.size()*mRadius;
    else
      return (mRange.upper-coord)/mRange.size()*mRadius;
  } else // mScaleType == stLogarithmic
  {
    if (coord >= 0.0 && mRange.upper < 0.0) // invalid value for logarithmic scale, just return outside visible range
      return !mRangeReversed ? mRadius+200 : mRadius-200;
    else if (coord <= 0.0 && mRange.upper >= 0.0) // invalid value for logarithmic scale, just return outside visible range
      return !mRangeReversed ? mRadius-200 :mRadius+200;
    else
    {
      if (!mRangeReversed)
        return qLn(coord/mRange.lower)/qLn(mRange.upper/mRange.lower)*mRadius;
      else
        return qLn(mRange.upper/coord)/qLn(mRange.upper/mRange.lower)*mRadius;
    }
  }
}

double QCPPolarAxisRadial::radiusToCoord(double radius) const
{
  if (mScaleType == stLinear)
  {
    if (!mRangeReversed)
      return (radius)/mRadius*mRange.size()+mRange.lower;
    else
      return -(radius)/mRadius*mRange.size()+mRange.upper;
  } else // mScaleType == stLogarithmic
  {
    if (!mRangeReversed)
      return qPow(mRange.upper/mRange.lower, (radius)/mRadius)*mRange.lower;
    else
      return qPow(mRange.upper/mRange.lower, (-radius)/mRadius)*mRange.upper;
  }
}


/*!
  Returns the part of the axis that is hit by \a pos (in pixels). The return value of this function
  is independent of the user-selectable parts defined with \ref setSelectableParts. Further, this
  function does not change the current selection state of the axis.
  
  If the axis is not visible (\ref setVisible), this function always returns \ref spNone.
  
  \see setSelectedParts, setSelectableParts, QCustomPlot::setInteractions
*/
QCPPolarAxisRadial::SelectablePart QCPPolarAxisRadial::getPartAt(const QPointF &pos) const
{
  Q_UNUSED(pos) // TODO remove later
  if (!mVisible)
    return spNone;
  
  /*
    TODO:
  if (mAxisPainter->axisSelectionBox().contains(pos.toPoint()))
    return spAxis;
  else if (mAxisPainter->tickLabelsSelectionBox().contains(pos.toPoint()))
    return spTickLabels;
  else if (mAxisPainter->labelSelectionBox().contains(pos.toPoint()))
    return spAxisLabel;
  else */
    return spNone;
}

/* inherits documentation from base class */
double QCPPolarAxisRadial::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if (!mParentPlot) return -1;
  SelectablePart part = getPartAt(pos);
  if ((onlySelectable && !mSelectableParts.testFlag(part)) || part == spNone)
    return -1;
  
  if (details)
    details->setValue(part);
  return mParentPlot->selectionTolerance()*0.99;
}

/* inherits documentation from base class */
void QCPPolarAxisRadial::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  SelectablePart part = details.value<SelectablePart>();
  if (mSelectableParts.testFlag(part))
  {
    SelectableParts selBefore = mSelectedParts;
    setSelectedParts(additive ? mSelectedParts^part : part);
    if (selectionStateChanged)
      *selectionStateChanged = mSelectedParts != selBefore;
  }
}

/* inherits documentation from base class */
void QCPPolarAxisRadial::deselectEvent(bool *selectionStateChanged)
{
  SelectableParts selBefore = mSelectedParts;
  setSelectedParts(mSelectedParts & ~mSelectableParts);
  if (selectionStateChanged)
    *selectionStateChanged = mSelectedParts != selBefore;
}

/*! \internal
  
  This mouse event reimplementation provides the functionality to let the user drag individual axes
  exclusively, by startig the drag on top of the axis.

  For the axis to accept this event and perform the single axis drag, the parent \ref QCPAxisRect
  must be configured accordingly, i.e. it must allow range dragging in the orientation of this axis
  (\ref QCPAxisRect::setRangeDrag) and this axis must be a draggable axis (\ref
  QCPAxisRect::setRangeDragAxes)
  
  \seebaseclassmethod
  
  \note The dragging of possibly multiple axes at once by starting the drag anywhere in the axis
  rect is handled by the axis rect's mouse event, e.g. \ref QCPAxisRect::mousePressEvent.
*/
void QCPPolarAxisRadial::mousePressEvent(QMouseEvent *event, const QVariant &details)
{
  Q_UNUSED(details)
  if (!mParentPlot->interactions().testFlag(QCP::iRangeDrag))
  {
    event->ignore();
    return;
  }
  
  if (event->buttons() & Qt::LeftButton)
  {
    mDragging = true;
    // initialize antialiasing backup in case we start dragging:
    if (mParentPlot->noAntialiasingOnDrag())
    {
      mAADragBackup = mParentPlot->antialiasedElements();
      mNotAADragBackup = mParentPlot->notAntialiasedElements();
    }
    // Mouse range dragging interaction:
    if (mParentPlot->interactions().testFlag(QCP::iRangeDrag))
      mDragStartRange = mRange;
  }
}

/*! \internal
  
  This mouse event reimplementation provides the functionality to let the user drag individual axes
  exclusively, by startig the drag on top of the axis.
  
  \seebaseclassmethod
  
  \note The dragging of possibly multiple axes at once by starting the drag anywhere in the axis
  rect is handled by the axis rect's mouse event, e.g. \ref QCPAxisRect::mousePressEvent.
  
  \see QCPAxis::mousePressEvent
*/
void QCPPolarAxisRadial::mouseMoveEvent(QMouseEvent *event, const QPointF &startPos)
{
  Q_UNUSED(event) // TODO remove later
  Q_UNUSED(startPos) // TODO remove later
  if (mDragging)
  {
    /* TODO
    const double startPixel = orientation() == Qt::Horizontal ? startPos.x() : startPos.y();
    const double currentPixel = orientation() == Qt::Horizontal ? event->pos().x() : event->pos().y();
    if (mScaleType == QCPPolarAxisRadial::stLinear)
    {
      const double diff = pixelToCoord(startPixel) - pixelToCoord(currentPixel);
      setRange(mDragStartRange.lower+diff, mDragStartRange.upper+diff);
    } else if (mScaleType == QCPPolarAxisRadial::stLogarithmic)
    {
      const double diff = pixelToCoord(startPixel) / pixelToCoord(currentPixel);
      setRange(mDragStartRange.lower*diff, mDragStartRange.upper*diff);
    }
    */
    
    if (mParentPlot->noAntialiasingOnDrag())
      mParentPlot->setNotAntialiasedElements(QCP::aeAll);
    mParentPlot->replot(QCustomPlot::rpQueuedReplot);
  }
}

/*! \internal
  
  This mouse event reimplementation provides the functionality to let the user drag individual axes
  exclusively, by startig the drag on top of the axis.
  
  \seebaseclassmethod
  
  \note The dragging of possibly multiple axes at once by starting the drag anywhere in the axis
  rect is handled by the axis rect's mouse event, e.g. \ref QCPAxisRect::mousePressEvent.
  
  \see QCPAxis::mousePressEvent
*/
void QCPPolarAxisRadial::mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos)
{
  Q_UNUSED(event)
  Q_UNUSED(startPos)
  mDragging = false;
  if (mParentPlot->noAntialiasingOnDrag())
  {
    mParentPlot->setAntialiasedElements(mAADragBackup);
    mParentPlot->setNotAntialiasedElements(mNotAADragBackup);
  }
}

/*! \internal
  
  This mouse event reimplementation provides the functionality to let the user zoom individual axes
  exclusively, by performing the wheel event on top of the axis.

  For the axis to accept this event and perform the single axis zoom, the parent \ref QCPAxisRect
  must be configured accordingly, i.e. it must allow range zooming in the orientation of this axis
  (\ref QCPAxisRect::setRangeZoom) and this axis must be a zoomable axis (\ref
  QCPAxisRect::setRangeZoomAxes)
  
  \seebaseclassmethod
  
  \note The zooming of possibly multiple axes at once by performing the wheel event anywhere in the
  axis rect is handled by the axis rect's mouse event, e.g. \ref QCPAxisRect::wheelEvent.
*/
void QCPPolarAxisRadial::wheelEvent(QWheelEvent *event)
{
  // Mouse range zooming interaction:
  if (!mParentPlot->interactions().testFlag(QCP::iRangeZoom))
  {
    event->ignore();
    return;
  }
  
  // TODO:
  //const double wheelSteps = event->delta()/120.0; // a single step delta is +/-120 usually
  //const double factor = qPow(mRangeZoomFactor, wheelSteps);
  //scaleRange(factor, pixelToCoord(orientation() == Qt::Horizontal ? event->pos().x() : event->pos().y()));
  mParentPlot->replot();
}

void QCPPolarAxisRadial::updateGeometry(const QPointF &center, double radius)
{
  mCenter = center;
  mRadius = radius;
  if (mRadius < 1) mRadius = 1;
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing axis lines.

  This is the antialiasing state the painter passed to the \ref draw method is in by default.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \seebaseclassmethod
  
  \see setAntialiased
*/
void QCPPolarAxisRadial::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeAxes);
}

/*! \internal
  
  Draws the axis with the specified \a painter, using the internal QCPAxisPainterPrivate instance.

  \seebaseclassmethod
*/
void QCPPolarAxisRadial::draw(QCPPainter *painter)
{
  const double axisAngleRad = (mAngle+(mAngleReference==arAngularAxis ? mAngularAxis->angle() : 0))/180.0*M_PI;
  const QPointF axisVector(qCos(axisAngleRad), qSin(axisAngleRad)); // semantically should be QCPVector2D, but we save time in loops when we keep it as QPointF
  const QPointF tickNormal = QCPVector2D(axisVector).perpendicular().toPointF(); // semantically should be QCPVector2D, but we save time in loops when we keep it as QPointF
  
  // draw baseline:
  painter->setPen(getBasePen());
  painter->drawLine(QLineF(mCenter, mCenter+axisVector*(mRadius-0.5)));
  
  // draw subticks:
  if (!mSubTickVector.isEmpty())
  {
    painter->setPen(getSubTickPen());
    for (int i=0; i<mSubTickVector.size(); ++i)
    {
      const QPointF tickPosition = mCenter+axisVector*coordToRadius(mSubTickVector.at(i));
      painter->drawLine(QLineF(tickPosition-tickNormal*mSubTickLengthIn, tickPosition+tickNormal*mSubTickLengthOut));
    }
  }
  
  // draw ticks and labels:
  if (!mTickVector.isEmpty())
  {
    mLabelPainter.setAnchorReference(mCenter-axisVector); // subtract (normalized) axisVector, just to prevent degenerate tangents for tick label at exact lower axis range
    mLabelPainter.setFont(getTickLabelFont());
    mLabelPainter.setColor(getTickLabelColor());
    const QPen ticksPen = getTickPen();
    painter->setPen(ticksPen);
    for (int i=0; i<mTickVector.size(); ++i)
    {
      const double r = coordToRadius(mTickVector.at(i));
      const QPointF tickPosition = mCenter+axisVector*r;
      painter->drawLine(QLineF(tickPosition-tickNormal*mTickLengthIn, tickPosition+tickNormal*mTickLengthOut));
      // possibly draw tick labels:
      if (!mTickVectorLabels.isEmpty())
      {
        if ((!mRangeReversed && (i < mTickVectorLabels.count()-1 || mRadius-r > 10)) ||
            (mRangeReversed && (i > 0 || mRadius-r > 10))) // skip last label if it's closer than 10 pixels to angular axis
          mLabelPainter.drawTickLabel(painter, tickPosition+tickNormal*mSubTickLengthOut, mTickVectorLabels.at(i));
      }
    }
  }
}

/*! \internal
  
  Prepares the internal tick vector, sub tick vector and tick label vector. This is done by calling
  QCPAxisTicker::generate on the currently installed ticker.
  
  If a change in the label text/count is detected, the cached axis margin is invalidated to make
  sure the next margin calculation recalculates the label sizes and returns an up-to-date value.
*/
void QCPPolarAxisRadial::setupTickVectors()
{
  if (!mParentPlot) return;
  if ((!mTicks && !mTickLabels) || mRange.size() <= 0) return;
  
  mTicker->generate(mRange, mParentPlot->locale(), mNumberFormatChar, mNumberPrecision, mTickVector, mSubTicks ? &mSubTickVector : 0, mTickLabels ? &mTickVectorLabels : 0);
}

/*! \internal
  
  Returns the pen that is used to draw the axis base line. Depending on the selection state, this
  is either mSelectedBasePen or mBasePen.
*/
QPen QCPPolarAxisRadial::getBasePen() const
{
  return mSelectedParts.testFlag(spAxis) ? mSelectedBasePen : mBasePen;
}

/*! \internal
  
  Returns the pen that is used to draw the (major) ticks. Depending on the selection state, this
  is either mSelectedTickPen or mTickPen.
*/
QPen QCPPolarAxisRadial::getTickPen() const
{
  return mSelectedParts.testFlag(spAxis) ? mSelectedTickPen : mTickPen;
}

/*! \internal
  
  Returns the pen that is used to draw the subticks. Depending on the selection state, this
  is either mSelectedSubTickPen or mSubTickPen.
*/
QPen QCPPolarAxisRadial::getSubTickPen() const
{
  return mSelectedParts.testFlag(spAxis) ? mSelectedSubTickPen : mSubTickPen;
}

/*! \internal
  
  Returns the font that is used to draw the tick labels. Depending on the selection state, this
  is either mSelectedTickLabelFont or mTickLabelFont.
*/
QFont QCPPolarAxisRadial::getTickLabelFont() const
{
  return mSelectedParts.testFlag(spTickLabels) ? mSelectedTickLabelFont : mTickLabelFont;
}

/*! \internal
  
  Returns the font that is used to draw the axis label. Depending on the selection state, this
  is either mSelectedLabelFont or mLabelFont.
*/
QFont QCPPolarAxisRadial::getLabelFont() const
{
  return mSelectedParts.testFlag(spAxisLabel) ? mSelectedLabelFont : mLabelFont;
}

/*! \internal
  
  Returns the color that is used to draw the tick labels. Depending on the selection state, this
  is either mSelectedTickLabelColor or mTickLabelColor.
*/
QColor QCPPolarAxisRadial::getTickLabelColor() const
{
  return mSelectedParts.testFlag(spTickLabels) ? mSelectedTickLabelColor : mTickLabelColor;
}

/*! \internal
  
  Returns the color that is used to draw the axis label. Depending on the selection state, this
  is either mSelectedLabelColor or mLabelColor.
*/
QColor QCPPolarAxisRadial::getLabelColor() const
{
  return mSelectedParts.testFlag(spAxisLabel) ? mSelectedLabelColor : mLabelColor;
}


/* inherits documentation from base class */
QCP::Interaction QCPPolarAxisRadial::selectionCategory() const
{
  return QCP::iSelectAxes;
}
