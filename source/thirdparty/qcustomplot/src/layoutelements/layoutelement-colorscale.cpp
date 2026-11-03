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

#include "layoutelement-colorscale.h"

#include "../painter.h"
#include "../core.h"
#include "../plottable.h"
#include "../axis/axis.h"
#include "../plottables/plottable-colormap.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPColorScale
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPColorScale
  \brief A color scale for use with color coding data such as QCPColorMap
  
  This layout element can be placed on the plot to correlate a color gradient with data values. It
  is usually used in combination with one or multiple \ref QCPColorMap "QCPColorMaps".

  \image html QCPColorScale.png
  
  The color scale can be either horizontal or vertical, as shown in the image above. The
  orientation and the side where the numbers appear is controlled with \ref setType.
  
  Use \ref QCPColorMap::setColorScale to connect a color map with a color scale. Once they are
  connected, they share their gradient, data range and data scale type (\ref setGradient, \ref
  setDataRange, \ref setDataScaleType). Multiple color maps may be associated with a single color
  scale, to make them all synchronize these properties.
  
  To have finer control over the number display and axis behaviour, you can directly access the
  \ref axis. See the documentation of QCPAxis for details about configuring axes. For example, if
  you want to change the number of automatically generated ticks, call
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcolorscale-tickcount
  
  Placing a color scale next to the main axis rect works like with any other layout element:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcolorscale-creation
  In this case we have placed it to the right of the default axis rect, so it wasn't necessary to
  call \ref setType, since \ref QCPAxis::atRight is already the default. The text next to the color
  scale can be set with \ref setLabel.
  
  For optimum appearance (like in the image above), it may be desirable to line up the axis rect and
  the borders of the color scale. Use a \ref QCPMarginGroup to achieve this:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcolorscale-margingroup
  
  Color scales are initialized with a non-zero minimum top and bottom margin (\ref
  setMinimumMargins), because vertical color scales are most common and the minimum top/bottom
  margin makes sure it keeps some distance to the top/bottom widget border. So if you change to a
  horizontal color scale by setting \ref setType to \ref QCPAxis::atBottom or \ref QCPAxis::atTop, you
  might want to also change the minimum margins accordingly, e.g. <tt>setMinimumMargins(QMargins(6, 0, 6, 0))</tt>.
*/

/* start documentation of inline functions */

/*! \fn QCPAxis *QCPColorScale::axis() const
  
  Returns the internal \ref QCPAxis instance of this color scale. You can access it to alter the
  appearance and behaviour of the axis. \ref QCPColorScale duplicates some properties in its
  interface for convenience. Those are \ref setDataRange (\ref QCPAxis::setRange), \ref
  setDataScaleType (\ref QCPAxis::setScaleType), and the method \ref setLabel (\ref
  QCPAxis::setLabel). As they each are connected, it does not matter whether you use the method on
  the QCPColorScale or on its QCPAxis.
  
  If the type of the color scale is changed with \ref setType, the axis returned by this method
  will change, too, to either the left, right, bottom or top axis, depending on which type was set.
*/

/* end documentation of signals */
/* start documentation of signals */

/*! \fn void QCPColorScale::dataRangeChanged(const QCPRange &newRange);
  
  This signal is emitted when the data range changes.
  
  \see setDataRange
*/

/*! \fn void QCPColorScale::dataScaleTypeChanged(QCPAxis::ScaleType scaleType);
  
  This signal is emitted when the data scale type changes.
  
  \see setDataScaleType
*/

/*! \fn void QCPColorScale::gradientChanged(const QCPColorGradient &newGradient);
  
  This signal is emitted when the gradient changes.
  
  \see setGradient
*/

/* end documentation of signals */

/*!
  Constructs a new QCPColorScale.
*/
QCPColorScale::QCPColorScale(QCustomPlot *parentPlot) :
  QCPLayoutElement(parentPlot),
  mType(QCPAxis::atTop), // set to atTop such that setType(QCPAxis::atRight) below doesn't skip work because it thinks it's already atRight
  mDataScaleType(QCPAxis::stLinear),
  mGradient(QCPColorGradient::gpCold),
  mBarWidth(20),
  mAxisRect(new QCPColorScaleAxisRectPrivate(this))
{
  setMinimumMargins(QMargins(0, 6, 0, 6)); // for default right color scale types, keep some room at bottom and top (important if no margin group is used)
  setType(QCPAxis::atRight);
  setDataRange(QCPRange(0, 6));
}

QCPColorScale::~QCPColorScale()
{
  delete mAxisRect;
}

/* undocumented getter */
QString QCPColorScale::label() const
{
  if (!mColorAxis)
  {
    qDebug() << Q_FUNC_INFO << "internal color axis undefined";
    return QString();
  }
  
  return mColorAxis.data()->label();
}

/* undocumented getter */
bool QCPColorScale::rangeDrag() const
{
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return false;
  }
  
  return mAxisRect.data()->rangeDrag().testFlag(QCPAxis::orientation(mType)) &&
      mAxisRect.data()->rangeDragAxis(QCPAxis::orientation(mType)) &&
      mAxisRect.data()->rangeDragAxis(QCPAxis::orientation(mType))->orientation() == QCPAxis::orientation(mType);
}

/* undocumented getter */
bool QCPColorScale::rangeZoom() const
{
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return false;
  }
  
  return mAxisRect.data()->rangeZoom().testFlag(QCPAxis::orientation(mType)) &&
      mAxisRect.data()->rangeZoomAxis(QCPAxis::orientation(mType)) &&
      mAxisRect.data()->rangeZoomAxis(QCPAxis::orientation(mType))->orientation() == QCPAxis::orientation(mType);
}

/*!
  Sets at which side of the color scale the axis is placed, and thus also its orientation.
  
  Note that after setting \a type to a different value, the axis returned by \ref axis() will
  be a different one. The new axis will adopt the following properties from the previous axis: The
  range, scale type, label and ticker (the latter will be shared and not copied).
*/
void QCPColorScale::setType(QCPAxis::AxisType type)
{
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return;
  }
  if (mType != type)
  {
    mType = type;
    QCPRange rangeTransfer(0, 6);
    QString labelTransfer;
    QSharedPointer<QCPAxisTicker> tickerTransfer;
    // transfer/revert some settings on old axis if it exists:
    bool doTransfer = !mColorAxis.isNull();
    if (doTransfer)
    {
      rangeTransfer = mColorAxis.data()->range();
      labelTransfer = mColorAxis.data()->label();
      tickerTransfer = mColorAxis.data()->ticker();
      mColorAxis.data()->setLabel(QString());
      disconnect(mColorAxis.data(), SIGNAL(rangeChanged(QCPRange)), this, SLOT(setDataRange(QCPRange)));
      disconnect(mColorAxis.data(), SIGNAL(scaleTypeChanged(QCPAxis::ScaleType)), this, SLOT(setDataScaleType(QCPAxis::ScaleType)));
    }
    const QList<QCPAxis::AxisType> allAxisTypes = QList<QCPAxis::AxisType>() << QCPAxis::atLeft << QCPAxis::atRight << QCPAxis::atBottom << QCPAxis::atTop;
    foreach (QCPAxis::AxisType atype, allAxisTypes)
    {
      mAxisRect.data()->axis(atype)->setTicks(atype == mType);
      mAxisRect.data()->axis(atype)->setTickLabels(atype== mType);
    }
    // set new mColorAxis pointer:
    mColorAxis = mAxisRect.data()->axis(mType);
    // transfer settings to new axis:
    if (doTransfer)
    {
      mColorAxis.data()->setRange(rangeTransfer); // range transfer necessary if axis changes from vertical to horizontal or vice versa (axes with same orientation are synchronized via signals)
      mColorAxis.data()->setLabel(labelTransfer);
      mColorAxis.data()->setTicker(tickerTransfer);
    }
    connect(mColorAxis.data(), SIGNAL(rangeChanged(QCPRange)), this, SLOT(setDataRange(QCPRange)));
    connect(mColorAxis.data(), SIGNAL(scaleTypeChanged(QCPAxis::ScaleType)), this, SLOT(setDataScaleType(QCPAxis::ScaleType)));
    mAxisRect.data()->setRangeDragAxes(QList<QCPAxis*>() << mColorAxis.data());
  }
}

/*!
  Sets the range spanned by the color gradient and that is shown by the axis in the color scale.
  
  It is equivalent to calling QCPColorMap::setDataRange on any of the connected color maps. It is
  also equivalent to directly accessing the \ref axis and setting its range with \ref
  QCPAxis::setRange.
  
  \see setDataScaleType, setGradient, rescaleDataRange
*/
void QCPColorScale::setDataRange(const QCPRange &dataRange)
{
  if (mDataRange.lower != dataRange.lower || mDataRange.upper != dataRange.upper)
  {
    mDataRange = dataRange;
    if (mColorAxis)
      mColorAxis.data()->setRange(mDataRange);
    emit dataRangeChanged(mDataRange);
  }
}

/*!
  Sets the scale type of the color scale, i.e. whether values are associated with colors linearly
  or logarithmically.
  
  It is equivalent to calling QCPColorMap::setDataScaleType on any of the connected color maps. It is
  also equivalent to directly accessing the \ref axis and setting its scale type with \ref
  QCPAxis::setScaleType.
  
  Note that this method controls the coordinate transformation. For logarithmic scales, you will
  likely also want to use a logarithmic tick spacing and labeling, which can be achieved by setting
  the color scale's \ref axis ticker to an instance of \ref QCPAxisTickerLog :
  
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpaxisticker-log-colorscale
  
  See the documentation of \ref QCPAxisTickerLog about the details of logarithmic axis tick
  creation.
  
  \see setDataRange, setGradient
*/
void QCPColorScale::setDataScaleType(QCPAxis::ScaleType scaleType)
{
  if (mDataScaleType != scaleType)
  {
    mDataScaleType = scaleType;
    if (mColorAxis)
      mColorAxis.data()->setScaleType(mDataScaleType);
    if (mDataScaleType == QCPAxis::stLogarithmic)
      setDataRange(mDataRange.sanitizedForLogScale());
    emit dataScaleTypeChanged(mDataScaleType);
  }
}

/*!
  Sets the color gradient that will be used to represent data values.
  
  It is equivalent to calling QCPColorMap::setGradient on any of the connected color maps.
  
  \see setDataRange, setDataScaleType
*/
void QCPColorScale::setGradient(const QCPColorGradient &gradient)
{
  if (mGradient != gradient)
  {
    mGradient = gradient;
    if (mAxisRect)
      mAxisRect.data()->mGradientImageInvalidated = true;
    emit gradientChanged(mGradient);
  }
}

/*!
  Sets the axis label of the color scale. This is equivalent to calling \ref QCPAxis::setLabel on
  the internal \ref axis.
*/
void QCPColorScale::setLabel(const QString &str)
{
  if (!mColorAxis)
  {
    qDebug() << Q_FUNC_INFO << "internal color axis undefined";
    return;
  }
  
  mColorAxis.data()->setLabel(str);
}

/*!
  Sets the width (or height, for horizontal color scales) the bar where the gradient is displayed
  will have.
*/
void QCPColorScale::setBarWidth(int width)
{
  mBarWidth = width;
}

/*!
  Sets whether the user can drag the data range (\ref setDataRange).
  
  Note that \ref QCP::iRangeDrag must be in the QCustomPlot's interactions (\ref
  QCustomPlot::setInteractions) to allow range dragging.
*/
void QCPColorScale::setRangeDrag(bool enabled)
{
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return;
  }
  
  if (enabled)
  {
    mAxisRect.data()->setRangeDrag(QCPAxis::orientation(mType));
  } else
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
    mAxisRect.data()->setRangeDrag(nullptr);
#else
    mAxisRect.data()->setRangeDrag({});
#endif
  }
}

/*!
  Sets whether the user can zoom the data range (\ref setDataRange) by scrolling the mouse wheel.
  
  Note that \ref QCP::iRangeZoom must be in the QCustomPlot's interactions (\ref
  QCustomPlot::setInteractions) to allow range dragging.
*/
void QCPColorScale::setRangeZoom(bool enabled)
{
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return;
  }
  
  if (enabled)
  {
    mAxisRect.data()->setRangeZoom(QCPAxis::orientation(mType));
  } else
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
    mAxisRect.data()->setRangeDrag(nullptr);
#else
    mAxisRect.data()->setRangeZoom({});
#endif
  }
}

/*!
  Returns a list of all the color maps associated with this color scale.
*/
QList<QCPColorMap*> QCPColorScale::colorMaps() const
{
  QList<QCPColorMap*> result;
  for (int i=0; i<mParentPlot->plottableCount(); ++i)
  {
    if (QCPColorMap *cm = qobject_cast<QCPColorMap*>(mParentPlot->plottable(i)))
      if (cm->colorScale() == this)
        result.append(cm);
  }
  return result;
}

/*!
  Changes the data range such that all color maps associated with this color scale are fully mapped
  to the gradient in the data dimension.
  
  \see setDataRange
*/
void QCPColorScale::rescaleDataRange(bool onlyVisibleMaps)
{
  QList<QCPColorMap*> maps = colorMaps();
  QCPRange newRange;
  bool haveRange = false;
  QCP::SignDomain sign = QCP::sdBoth;
  if (mDataScaleType == QCPAxis::stLogarithmic)
    sign = (mDataRange.upper < 0 ? QCP::sdNegative : QCP::sdPositive);
  foreach (QCPColorMap *map, maps)
  {
    if (!map->realVisibility() && onlyVisibleMaps)
      continue;
    QCPRange mapRange;
    if (map->colorScale() == this)
    {
      bool currentFoundRange = true;
      mapRange = map->data()->dataBounds();
      if (sign == QCP::sdPositive)
      {
        if (mapRange.lower <= 0 && mapRange.upper > 0)
          mapRange.lower = mapRange.upper*1e-3;
        else if (mapRange.lower <= 0 && mapRange.upper <= 0)
          currentFoundRange = false;
      } else if (sign == QCP::sdNegative)
      {
        if (mapRange.upper >= 0 && mapRange.lower < 0)
          mapRange.upper = mapRange.lower*1e-3;
        else if (mapRange.upper >= 0 && mapRange.lower >= 0)
          currentFoundRange = false;
      }
      if (currentFoundRange)
      {
        if (!haveRange)
          newRange = mapRange;
        else
          newRange.expand(mapRange);
        haveRange = true;
      }
    }
  }
  if (haveRange)
  {
    if (!QCPRange::validRange(newRange)) // likely due to range being zero (plottable has only constant data in this dimension), shift current range to at least center the data
    {
      double center = (newRange.lower+newRange.upper)*0.5; // upper and lower should be equal anyway, but just to make sure, incase validRange returned false for other reason
      if (mDataScaleType == QCPAxis::stLinear)
      {
        newRange.lower = center-mDataRange.size()/2.0;
        newRange.upper = center+mDataRange.size()/2.0;
      } else // mScaleType == stLogarithmic
      {
        newRange.lower = center/qSqrt(mDataRange.upper/mDataRange.lower);
        newRange.upper = center*qSqrt(mDataRange.upper/mDataRange.lower);
      }
    }
    setDataRange(newRange);
  }
}

/* inherits documentation from base class */
void QCPColorScale::update(UpdatePhase phase)
{
  QCPLayoutElement::update(phase);
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return;
  }
  
  mAxisRect.data()->update(phase);
  
  switch (phase)
  {
    case upMargins:
    {
      if (mType == QCPAxis::atBottom || mType == QCPAxis::atTop)
      {
        setMaximumSize(QWIDGETSIZE_MAX, mBarWidth+mAxisRect.data()->margins().top()+mAxisRect.data()->margins().bottom());
        setMinimumSize(0,               mBarWidth+mAxisRect.data()->margins().top()+mAxisRect.data()->margins().bottom());
      } else
      {
        setMaximumSize(mBarWidth+mAxisRect.data()->margins().left()+mAxisRect.data()->margins().right(), QWIDGETSIZE_MAX);
        setMinimumSize(mBarWidth+mAxisRect.data()->margins().left()+mAxisRect.data()->margins().right(), 0);
      }
      break;
    }
    case upLayout:
    {
      mAxisRect.data()->setOuterRect(rect());
      break;
    }
    default: break;
  }
}

/* inherits documentation from base class */
void QCPColorScale::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  painter->setAntialiasing(false);
}

/* inherits documentation from base class */
void QCPColorScale::mousePressEvent(QMouseEvent *event, const QVariant &details)
{
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return;
  }
  mAxisRect.data()->mousePressEvent(event, details);
}

/* inherits documentation from base class */
void QCPColorScale::mouseMoveEvent(QMouseEvent *event, const QPointF &startPos)
{
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return;
  }
  mAxisRect.data()->mouseMoveEvent(event, startPos);
}

/* inherits documentation from base class */
void QCPColorScale::mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos)
{
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return;
  }
  mAxisRect.data()->mouseReleaseEvent(event, startPos);
}

/* inherits documentation from base class */
void QCPColorScale::wheelEvent(QWheelEvent *event)
{
  if (!mAxisRect)
  {
    qDebug() << Q_FUNC_INFO << "internal axis rect was deleted";
    return;
  }
  mAxisRect.data()->wheelEvent(event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPColorScaleAxisRectPrivate
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPColorScaleAxisRectPrivate

  \internal
  \brief An axis rect subclass for use in a QCPColorScale
  
  This is a private class and not part of the public QCustomPlot interface.
  
  It provides the axis rect functionality for the QCPColorScale class.
*/


/*!
  Creates a new instance, as a child of \a parentColorScale.
*/
QCPColorScaleAxisRectPrivate::QCPColorScaleAxisRectPrivate(QCPColorScale *parentColorScale) :
  QCPAxisRect(parentColorScale->parentPlot(), true),
  mParentColorScale(parentColorScale),
  mGradientImageInvalidated(true)
{
  setParentLayerable(parentColorScale);
  setMinimumMargins(QMargins(0, 0, 0, 0));
  const QList<QCPAxis::AxisType> allAxisTypes = QList<QCPAxis::AxisType>() << QCPAxis::atBottom << QCPAxis::atTop << QCPAxis::atLeft << QCPAxis::atRight;
  foreach (QCPAxis::AxisType type, allAxisTypes)
  {
    axis(type)->setVisible(true);
    axis(type)->grid()->setVisible(false);
    axis(type)->setPadding(0);
    connect(axis(type), SIGNAL(selectionChanged(QCPAxis::SelectableParts)), this, SLOT(axisSelectionChanged(QCPAxis::SelectableParts)));
    connect(axis(type), SIGNAL(selectableChanged(QCPAxis::SelectableParts)), this, SLOT(axisSelectableChanged(QCPAxis::SelectableParts)));
  }

  connect(axis(QCPAxis::atLeft), SIGNAL(rangeChanged(QCPRange)), axis(QCPAxis::atRight), SLOT(setRange(QCPRange)));
  connect(axis(QCPAxis::atRight), SIGNAL(rangeChanged(QCPRange)), axis(QCPAxis::atLeft), SLOT(setRange(QCPRange)));
  connect(axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), axis(QCPAxis::atTop), SLOT(setRange(QCPRange)));
  connect(axis(QCPAxis::atTop), SIGNAL(rangeChanged(QCPRange)), axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
  connect(axis(QCPAxis::atLeft), SIGNAL(scaleTypeChanged(QCPAxis::ScaleType)), axis(QCPAxis::atRight), SLOT(setScaleType(QCPAxis::ScaleType)));
  connect(axis(QCPAxis::atRight), SIGNAL(scaleTypeChanged(QCPAxis::ScaleType)), axis(QCPAxis::atLeft), SLOT(setScaleType(QCPAxis::ScaleType)));
  connect(axis(QCPAxis::atBottom), SIGNAL(scaleTypeChanged(QCPAxis::ScaleType)), axis(QCPAxis::atTop), SLOT(setScaleType(QCPAxis::ScaleType)));
  connect(axis(QCPAxis::atTop), SIGNAL(scaleTypeChanged(QCPAxis::ScaleType)), axis(QCPAxis::atBottom), SLOT(setScaleType(QCPAxis::ScaleType)));
  
  // make layer transfers of color scale transfer to axis rect and axes
  // the axes must be set after axis rect, such that they appear above color gradient drawn by axis rect:
  connect(parentColorScale, SIGNAL(layerChanged(QCPLayer*)), this, SLOT(setLayer(QCPLayer*)));
  foreach (QCPAxis::AxisType type, allAxisTypes)
    connect(parentColorScale, SIGNAL(layerChanged(QCPLayer*)), axis(type), SLOT(setLayer(QCPLayer*)));
}

/*! \internal
  
  Updates the color gradient image if necessary, by calling \ref updateGradientImage, then draws
  it. Then the axes are drawn by calling the \ref QCPAxisRect::draw base class implementation.
  
  \seebaseclassmethod
*/
void QCPColorScaleAxisRectPrivate::draw(QCPPainter *painter)
{
  if (mGradientImageInvalidated)
    updateGradientImage();
  
  bool mirrorHorz = false;
  bool mirrorVert = false;
  if (mParentColorScale->mColorAxis)
  {
    mirrorHorz = mParentColorScale->mColorAxis.data()->rangeReversed() && (mParentColorScale->type() == QCPAxis::atBottom || mParentColorScale->type() == QCPAxis::atTop);
    mirrorVert = mParentColorScale->mColorAxis.data()->rangeReversed() && (mParentColorScale->type() == QCPAxis::atLeft || mParentColorScale->type() == QCPAxis::atRight);
  }
  
  painter->drawImage(rect().adjusted(0, -1, 0, -1), mGradientImage.mirrored(mirrorHorz, mirrorVert));
  QCPAxisRect::draw(painter);
}

/*! \internal

  Uses the current gradient of the parent \ref QCPColorScale (specified in the constructor) to
  generate a gradient image. This gradient image will be used in the \ref draw method.
*/
void QCPColorScaleAxisRectPrivate::updateGradientImage()
{
  if (rect().isEmpty())
    return;
  
  const QImage::Format format = QImage::Format_ARGB32_Premultiplied;
  int n = mParentColorScale->mGradient.levelCount();
  int w, h;
  QVector<double> data(n);
  for (int i=0; i<n; ++i)
    data[i] = i;
  if (mParentColorScale->mType == QCPAxis::atBottom || mParentColorScale->mType == QCPAxis::atTop)
  {
    w = n;
    h = rect().height();
    mGradientImage = QImage(w, h, format);
    QVector<QRgb*> pixels;
    for (int y=0; y<h; ++y)
      pixels.append(reinterpret_cast<QRgb*>(mGradientImage.scanLine(y)));
    mParentColorScale->mGradient.colorize(data.constData(), QCPRange(0, n-1), pixels.first(), n);
    for (int y=1; y<h; ++y)
      memcpy(pixels.at(y), pixels.first(), size_t(n)*sizeof(QRgb));
  } else
  {
    w = rect().width();
    h = n;
    mGradientImage = QImage(w, h, format);
    for (int y=0; y<h; ++y)
    {
      QRgb *pixels = reinterpret_cast<QRgb*>(mGradientImage.scanLine(y));
      const QRgb lineColor = mParentColorScale->mGradient.color(data[h-1-y], QCPRange(0, n-1));
      for (int x=0; x<w; ++x)
        pixels[x] = lineColor;
    }
  }
  mGradientImageInvalidated = false;
}

/*! \internal

  This slot is connected to the selectionChanged signals of the four axes in the constructor. It
  synchronizes the selection state of the axes.
*/
void QCPColorScaleAxisRectPrivate::axisSelectionChanged(QCPAxis::SelectableParts selectedParts)
{
  // axis bases of four axes shall always (de-)selected synchronously:
  const QList<QCPAxis::AxisType> allAxisTypes = QList<QCPAxis::AxisType>() << QCPAxis::atBottom << QCPAxis::atTop << QCPAxis::atLeft << QCPAxis::atRight;
  foreach (QCPAxis::AxisType type, allAxisTypes)
  {
    if (QCPAxis *senderAxis = qobject_cast<QCPAxis*>(sender()))
      if (senderAxis->axisType() == type)
        continue;
    
    if (axis(type)->selectableParts().testFlag(QCPAxis::spAxis))
    {
      if (selectedParts.testFlag(QCPAxis::spAxis))
        axis(type)->setSelectedParts(axis(type)->selectedParts() | QCPAxis::spAxis);
      else
        axis(type)->setSelectedParts(axis(type)->selectedParts() & ~QCPAxis::spAxis);
    }
  }
}

/*! \internal

  This slot is connected to the selectableChanged signals of the four axes in the constructor. It
  synchronizes the selectability of the axes.
*/
void QCPColorScaleAxisRectPrivate::axisSelectableChanged(QCPAxis::SelectableParts selectableParts)
{
  // synchronize axis base selectability:
  const QList<QCPAxis::AxisType> allAxisTypes = QList<QCPAxis::AxisType>() << QCPAxis::atBottom << QCPAxis::atTop << QCPAxis::atLeft << QCPAxis::atRight;
  foreach (QCPAxis::AxisType type, allAxisTypes)
  {
    if (QCPAxis *senderAxis = qobject_cast<QCPAxis*>(sender()))
      if (senderAxis->axisType() == type)
        continue;
    
    if (axis(type)->selectableParts().testFlag(QCPAxis::spAxis))
    {
      if (selectableParts.testFlag(QCPAxis::spAxis))
        axis(type)->setSelectableParts(axis(type)->selectableParts() | QCPAxis::spAxis);
      else
        axis(type)->setSelectableParts(axis(type)->selectableParts() & ~QCPAxis::spAxis);
    }
  }
}
