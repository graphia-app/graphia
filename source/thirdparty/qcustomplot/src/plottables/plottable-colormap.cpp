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

#include "plottable-colormap.h"

#include "../painter.h"
#include "../core.h"
#include "../axis/axis.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPColorMapData
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPColorMapData
  \brief Holds the two-dimensional data of a QCPColorMap plottable.
  
  This class is a data storage for \ref QCPColorMap. It holds a two-dimensional array, which \ref
  QCPColorMap then displays as a 2D image in the plot, where the array values are represented by a
  color, depending on the value.
  
  The size of the array can be controlled via \ref setSize (or \ref setKeySize, \ref setValueSize).
  Which plot coordinates these cells correspond to can be configured with \ref setRange (or \ref
  setKeyRange, \ref setValueRange).
  
  The data cells can be accessed in two ways: They can be directly addressed by an integer index
  with \ref setCell. This is the fastest method. Alternatively, they can be addressed by their plot
  coordinate with \ref setData. plot coordinate to cell index transformations and vice versa are
  provided by the functions \ref coordToCell and \ref cellToCoord.
  
  A \ref QCPColorMapData also holds an on-demand two-dimensional array of alpha values which (if
  allocated) has the same size as the data map. It can be accessed via \ref setAlpha, \ref
  fillAlpha and \ref clearAlpha. The memory for the alpha map is only allocated if needed, i.e. on
  the first call of \ref setAlpha. \ref clearAlpha restores full opacity and frees the alpha map.
  
  This class also buffers the minimum and maximum values that are in the data set, to provide
  QCPColorMap::rescaleDataRange with the necessary information quickly. Setting a cell to a value
  that is greater than the current maximum increases this maximum to the new value. However,
  setting the cell that currently holds the maximum value to a smaller value doesn't decrease the
  maximum again, because finding the true new maximum would require going through the entire data
  array, which might be time consuming. The same holds for the data minimum. This functionality is
  given by \ref recalculateDataBounds, such that you can decide when it is sensible to find the
  true current minimum and maximum. The method QCPColorMap::rescaleDataRange offers a convenience
  parameter \a recalculateDataBounds which may be set to true to automatically call \ref
  recalculateDataBounds internally.
*/

/* start of documentation of inline functions */

/*! \fn bool QCPColorMapData::isEmpty() const
  
  Returns whether this instance carries no data. This is equivalent to having a size where at least
  one of the dimensions is 0 (see \ref setSize).
*/

/* end of documentation of inline functions */

/*!
  Constructs a new QCPColorMapData instance. The instance has \a keySize cells in the key direction
  and \a valueSize cells in the value direction. These cells will be displayed by the \ref QCPColorMap
  at the coordinates \a keyRange and \a valueRange.
  
  \see setSize, setKeySize, setValueSize, setRange, setKeyRange, setValueRange
*/
QCPColorMapData::QCPColorMapData(int keySize, int valueSize, const QCPRange &keyRange, const QCPRange &valueRange) :
  mKeySize(0),
  mValueSize(0),
  mKeyRange(keyRange),
  mValueRange(valueRange),
  mIsEmpty(true),
  mData(nullptr),
  mAlpha(nullptr),
  mDataModified(true)
{
  setSize(keySize, valueSize);
  fill(0);
}

QCPColorMapData::~QCPColorMapData()
{
  delete[] mData;
  delete[] mAlpha;
}

/*!
  Constructs a new QCPColorMapData instance copying the data and range of \a other.
*/
QCPColorMapData::QCPColorMapData(const QCPColorMapData &other) :
  mKeySize(0),
  mValueSize(0),
  mIsEmpty(true),
  mData(nullptr),
  mAlpha(nullptr),
  mDataModified(true)
{
  *this = other;
}

/*!
  Overwrites this color map data instance with the data stored in \a other. The alpha map state is
  transferred, too.
*/
QCPColorMapData &QCPColorMapData::operator=(const QCPColorMapData &other)
{
  if (&other != this)
  {
    const int keySize = other.keySize();
    const int valueSize = other.valueSize();
    if (!other.mAlpha && mAlpha)
      clearAlpha();
    setSize(keySize, valueSize);
    if (other.mAlpha && !mAlpha)
      createAlpha(false);
    setRange(other.keyRange(), other.valueRange());
    if (!isEmpty())
    {
      memcpy(mData, other.mData, sizeof(mData[0])*size_t(keySize*valueSize));
      if (mAlpha)
        memcpy(mAlpha, other.mAlpha, sizeof(mAlpha[0])*size_t(keySize*valueSize));
    }
    mDataBounds = other.mDataBounds;
    mDataModified = true;
  }
  return *this;
}

/* undocumented getter */
double QCPColorMapData::data(double key, double value)
{
  int keyCell = int( (key-mKeyRange.lower)/(mKeyRange.upper-mKeyRange.lower)*(mKeySize-1)+0.5 );
  int valueCell = int( (value-mValueRange.lower)/(mValueRange.upper-mValueRange.lower)*(mValueSize-1)+0.5 );
  if (keyCell >= 0 && keyCell < mKeySize && valueCell >= 0 && valueCell < mValueSize)
    return mData[valueCell*mKeySize + keyCell];
  else
    return 0;
}

/* undocumented getter */
double QCPColorMapData::cell(int keyIndex, int valueIndex)
{
  if (keyIndex >= 0 && keyIndex < mKeySize && valueIndex >= 0 && valueIndex < mValueSize)
    return mData[valueIndex*mKeySize + keyIndex];
  else
    return 0;
}

/*!
  Returns the alpha map value of the cell with the indices \a keyIndex and \a valueIndex.

  If this color map data doesn't have an alpha map (because \ref setAlpha was never called after
  creation or after a call to \ref clearAlpha), returns 255, which corresponds to full opacity.

  \see setAlpha
*/
unsigned char QCPColorMapData::alpha(int keyIndex, int valueIndex)
{
  if (mAlpha && keyIndex >= 0 && keyIndex < mKeySize && valueIndex >= 0 && valueIndex < mValueSize)
    return mAlpha[valueIndex*mKeySize + keyIndex];
  else
    return 255;
}

/*!
  Resizes the data array to have \a keySize cells in the key dimension and \a valueSize cells in
  the value dimension.

  The current data is discarded and the map cells are set to 0, unless the map had already the
  requested size.
  
  Setting at least one of \a keySize or \a valueSize to zero frees the internal data array and \ref
  isEmpty returns true.

  \see setRange, setKeySize, setValueSize
*/
void QCPColorMapData::setSize(int keySize, int valueSize)
{
  if (keySize != mKeySize || valueSize != mValueSize)
  {
    mKeySize = keySize;
    mValueSize = valueSize;
    delete[] mData;
    mIsEmpty = mKeySize == 0 || mValueSize == 0;
    if (!mIsEmpty)
    {
#ifdef __EXCEPTIONS
      try { // 2D arrays get memory intensive fast. So if the allocation fails, at least output debug message
#endif
      mData = new double[size_t(mKeySize*mValueSize)];
#ifdef __EXCEPTIONS
      } catch (...) { mData = nullptr; }
#endif
      if (mData)
        fill(0);
      else
        qDebug() << Q_FUNC_INFO << "out of memory for data dimensions "<< mKeySize << "*" << mValueSize;
    } else
      mData = nullptr;
    
    if (mAlpha) // if we had an alpha map, recreate it with new size
      createAlpha();
    
    mDataModified = true;
  }
}

/*!
  Resizes the data array to have \a keySize cells in the key dimension.

  The current data is discarded and the map cells are set to 0, unless the map had already the
  requested size.
  
  Setting \a keySize to zero frees the internal data array and \ref isEmpty returns true.

  \see setKeyRange, setSize, setValueSize
*/
void QCPColorMapData::setKeySize(int keySize)
{
  setSize(keySize, mValueSize);
}

/*!
  Resizes the data array to have \a valueSize cells in the value dimension.

  The current data is discarded and the map cells are set to 0, unless the map had already the
  requested size.
  
  Setting \a valueSize to zero frees the internal data array and \ref isEmpty returns true.

  \see setValueRange, setSize, setKeySize
*/
void QCPColorMapData::setValueSize(int valueSize)
{
  setSize(mKeySize, valueSize);
}

/*!
  Sets the coordinate ranges the data shall be distributed over. This defines the rectangular area
  covered by the color map in plot coordinates.
  
  The outer cells will be centered on the range boundaries given to this function. For example, if
  the key size (\ref setKeySize) is 3 and \a keyRange is set to <tt>QCPRange(2, 3)</tt> there will
  be cells centered on the key coordinates 2, 2.5 and 3.
 
  \see setSize
*/
void QCPColorMapData::setRange(const QCPRange &keyRange, const QCPRange &valueRange)
{
  setKeyRange(keyRange);
  setValueRange(valueRange);
}

/*!
  Sets the coordinate range the data shall be distributed over in the key dimension. Together with
  the value range, This defines the rectangular area covered by the color map in plot coordinates.
  
  The outer cells will be centered on the range boundaries given to this function. For example, if
  the key size (\ref setKeySize) is 3 and \a keyRange is set to <tt>QCPRange(2, 3)</tt> there will
  be cells centered on the key coordinates 2, 2.5 and 3.
 
  \see setRange, setValueRange, setSize
*/
void QCPColorMapData::setKeyRange(const QCPRange &keyRange)
{
  mKeyRange = keyRange;
}

/*!
  Sets the coordinate range the data shall be distributed over in the value dimension. Together with
  the key range, This defines the rectangular area covered by the color map in plot coordinates.
  
  The outer cells will be centered on the range boundaries given to this function. For example, if
  the value size (\ref setValueSize) is 3 and \a valueRange is set to <tt>QCPRange(2, 3)</tt> there
  will be cells centered on the value coordinates 2, 2.5 and 3.
 
  \see setRange, setKeyRange, setSize
*/
void QCPColorMapData::setValueRange(const QCPRange &valueRange)
{
  mValueRange = valueRange;
}

/*!
  Sets the data of the cell, which lies at the plot coordinates given by \a key and \a value, to \a
  z.
  
  \note The QCPColorMap always displays the data at equal key/value intervals, even if the key or
  value axis is set to a logarithmic scaling. If you want to use QCPColorMap with logarithmic axes,
  you shouldn't use the \ref QCPColorMapData::setData method as it uses a linear transformation to
  determine the cell index. Rather directly access the cell index with \ref
  QCPColorMapData::setCell.
 
  \see setCell, setRange
*/
void QCPColorMapData::setData(double key, double value, double z)
{
  int keyCell = int( (key-mKeyRange.lower)/(mKeyRange.upper-mKeyRange.lower)*(mKeySize-1)+0.5 );
  int valueCell = int( (value-mValueRange.lower)/(mValueRange.upper-mValueRange.lower)*(mValueSize-1)+0.5 );
  if (keyCell >= 0 && keyCell < mKeySize && valueCell >= 0 && valueCell < mValueSize)
  {
    mData[valueCell*mKeySize + keyCell] = z;
    if (z < mDataBounds.lower)
      mDataBounds.lower = z;
    if (z > mDataBounds.upper)
      mDataBounds.upper = z;
     mDataModified = true;
  }
}

/*!
  Sets the data of the cell with indices \a keyIndex and \a valueIndex to \a z. The indices
  enumerate the cells starting from zero, up to the map's size-1 in the respective dimension (see
  \ref setSize).
  
  In the standard plot configuration (horizontal key axis and vertical value axis, both not
  range-reversed), the cell with indices (0, 0) is in the bottom left corner and the cell with
  indices (keySize-1, valueSize-1) is in the top right corner of the color map.
  
  \see setData, setSize
*/
void QCPColorMapData::setCell(int keyIndex, int valueIndex, double z)
{
  if (keyIndex >= 0 && keyIndex < mKeySize && valueIndex >= 0 && valueIndex < mValueSize)
  {
    mData[valueIndex*mKeySize + keyIndex] = z;
    if (z < mDataBounds.lower)
      mDataBounds.lower = z;
    if (z > mDataBounds.upper)
      mDataBounds.upper = z;
     mDataModified = true;
  } else
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << keyIndex << valueIndex;
}

/*!
  Sets the alpha of the color map cell given by \a keyIndex and \a valueIndex to \a alpha. A value
  of 0 for \a alpha results in a fully transparent cell, and a value of 255 results in a fully
  opaque cell.

  If an alpha map doesn't exist yet for this color map data, it will be created here. If you wish
  to restore full opacity and free any allocated memory of the alpha map, call \ref clearAlpha.

  Note that the cell-wise alpha which can be configured here is independent of any alpha configured
  in the color map's gradient (\ref QCPColorGradient). If a cell is affected both by the cell-wise
  and gradient alpha, the alpha values will be blended accordingly during rendering of the color
  map.

  \see fillAlpha, clearAlpha
*/
void QCPColorMapData::setAlpha(int keyIndex, int valueIndex, unsigned char alpha)
{
  if (keyIndex >= 0 && keyIndex < mKeySize && valueIndex >= 0 && valueIndex < mValueSize)
  {
    if (mAlpha || createAlpha())
    {
      mAlpha[valueIndex*mKeySize + keyIndex] = alpha;
      mDataModified = true;
    }
  } else
    qDebug() << Q_FUNC_INFO << "index out of bounds:" << keyIndex << valueIndex;
}

/*!
  Goes through the data and updates the buffered minimum and maximum data values.
  
  Calling this method is only advised if you are about to call \ref QCPColorMap::rescaleDataRange
  and can not guarantee that the cells holding the maximum or minimum data haven't been overwritten
  with a smaller or larger value respectively, since the buffered maximum/minimum values have been
  updated the last time. Why this is the case is explained in the class description (\ref
  QCPColorMapData).
  
  Note that the method \ref QCPColorMap::rescaleDataRange provides a parameter \a
  recalculateDataBounds for convenience. Setting this to true will call this method for you, before
  doing the rescale.
*/
void QCPColorMapData::recalculateDataBounds()
{
  if (mKeySize > 0 && mValueSize > 0)
  {
    double minHeight = std::numeric_limits<double>::max();
    double maxHeight = -std::numeric_limits<double>::max();
    const int dataCount = mValueSize*mKeySize;
    for (int i=0; i<dataCount; ++i)
    {
      if (mData[i] > maxHeight)
        maxHeight = mData[i];
      if (mData[i] < minHeight)
        minHeight = mData[i];
    }
    mDataBounds.lower = minHeight;
    mDataBounds.upper = maxHeight;
  }
}

/*!
  Frees the internal data memory.
  
  This is equivalent to calling \ref setSize "setSize(0, 0)".
*/
void QCPColorMapData::clear()
{
  setSize(0, 0);
}

/*!
  Frees the internal alpha map. The color map will have full opacity again.
*/
void QCPColorMapData::clearAlpha()
{
  if (mAlpha)
  {
    delete[] mAlpha;
    mAlpha = nullptr;
    mDataModified = true;
  }
}

/*!
  Sets all cells to the value \a z.
*/
void QCPColorMapData::fill(double z)
{
  const int dataCount = mValueSize*mKeySize;
  for(size_t i = 0; i < dataCount; i++)
    mData[i] = z;
  mDataBounds = QCPRange(z, z);
  mDataModified = true;
}

/*!
  Sets the opacity of all color map cells to \a alpha. A value of 0 for \a alpha results in a fully
  transparent color map, and a value of 255 results in a fully opaque color map.

  If you wish to restore opacity to 100% and free any used memory for the alpha map, rather use
  \ref clearAlpha.

  \see setAlpha
*/
void QCPColorMapData::fillAlpha(unsigned char alpha)
{
  if (mAlpha || createAlpha(false))
  {
    const int dataCount = mValueSize*mKeySize;
    memset(mAlpha, alpha, dataCount*sizeof(*mAlpha));
    mDataModified = true;
  }
}

/*!
  Transforms plot coordinates given by \a key and \a value to cell indices of this QCPColorMapData
  instance. The resulting cell indices are returned via the output parameters \a keyIndex and \a
  valueIndex.
  
  The retrieved key/value cell indices can then be used for example with \ref setCell.
  
  If you are only interested in a key or value index, you may pass \c nullptr as \a valueIndex or
  \a keyIndex.
  
  \note The QCPColorMap always displays the data at equal key/value intervals, even if the key or
  value axis is set to a logarithmic scaling. If you want to use QCPColorMap with logarithmic axes,
  you shouldn't use the \ref QCPColorMapData::coordToCell method as it uses a linear transformation to
  determine the cell index.
  
  \see cellToCoord, QCPAxis::coordToPixel
*/
void QCPColorMapData::coordToCell(double key, double value, int *keyIndex, int *valueIndex) const
{
  if (keyIndex)
    *keyIndex = int( (key-mKeyRange.lower)/(mKeyRange.upper-mKeyRange.lower)*(mKeySize-1)+0.5 );
  if (valueIndex)
    *valueIndex = int( (value-mValueRange.lower)/(mValueRange.upper-mValueRange.lower)*(mValueSize-1)+0.5 );
}

/*!
  Transforms cell indices given by \a keyIndex and \a valueIndex to cell indices of this QCPColorMapData
  instance. The resulting coordinates are returned via the output parameters \a key and \a
  value.
  
  If you are only interested in a key or value coordinate, you may pass \c nullptr as \a key or \a
  value.
  
  \note The QCPColorMap always displays the data at equal key/value intervals, even if the key or
  value axis is set to a logarithmic scaling. If you want to use QCPColorMap with logarithmic axes,
  you shouldn't use the \ref QCPColorMapData::cellToCoord method as it uses a linear transformation to
  determine the cell index.
  
  \see coordToCell, QCPAxis::pixelToCoord
*/
void QCPColorMapData::cellToCoord(int keyIndex, int valueIndex, double *key, double *value) const
{
  if (key)
    *key = keyIndex/double(mKeySize-1)*(mKeyRange.upper-mKeyRange.lower)+mKeyRange.lower;
  if (value)
    *value = valueIndex/double(mValueSize-1)*(mValueRange.upper-mValueRange.lower)+mValueRange.lower;
}

/*! \internal

  Allocates the internal alpha map with the current data map key/value size and, if \a
  initializeOpaque is true, initializes all values to 255. If \a initializeOpaque is false, the
  values are not initialized at all. In this case, the alpha map should be initialized manually,
  e.g. with \ref fillAlpha.

  If an alpha map exists already, it is deleted first. If this color map is empty (has either key
  or value size zero, see \ref isEmpty), the alpha map is cleared.

  The return value indicates the existence of the alpha map after the call. So this method returns
  true if the data map isn't empty and an alpha map was successfully allocated.
*/
bool QCPColorMapData::createAlpha(bool initializeOpaque)
{
  clearAlpha();
  if (isEmpty())
    return false;
  
#ifdef __EXCEPTIONS
  try { // 2D arrays get memory intensive fast. So if the allocation fails, at least output debug message
#endif
    mAlpha = new unsigned char[size_t(mKeySize*mValueSize)];
#ifdef __EXCEPTIONS
  } catch (...) { mAlpha = nullptr; }
#endif
  if (mAlpha)
  {
    if (initializeOpaque)
      fillAlpha(255);
    return true;
  } else
  {
    qDebug() << Q_FUNC_INFO << "out of memory for data dimensions "<< mKeySize << "*" << mValueSize;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPColorMap
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPColorMap
  \brief A plottable representing a two-dimensional color map in a plot.

  \image html QCPColorMap.png
  
  The data is stored in the class \ref QCPColorMapData, which can be accessed via the data()
  method.
  
  A color map has three dimensions to represent a data point: The \a key dimension, the \a value
  dimension and the \a data dimension. As with other plottables such as graphs, \a key and \a value
  correspond to two orthogonal axes on the QCustomPlot surface that you specify in the QCPColorMap
  constructor. The \a data dimension however is encoded as the color of the point at (\a key, \a
  value).

  Set the number of points (or \a cells) in the key/value dimension via \ref
  QCPColorMapData::setSize. The plot coordinate range over which these points will be displayed is
  specified via \ref QCPColorMapData::setRange. The first cell will be centered on the lower range
  boundary and the last cell will be centered on the upper range boundary. The data can be set by
  either accessing the cells directly with QCPColorMapData::setCell or by addressing the cells via
  their plot coordinates with \ref QCPColorMapData::setData. If possible, you should prefer
  setCell, since it doesn't need to do any coordinate transformation and thus performs a bit
  better.
  
  The cell with index (0, 0) is at the bottom left, if the color map uses normal (i.e. not reversed)
  key and value axes.
  
  To show the user which colors correspond to which \a data values, a \ref QCPColorScale is
  typically placed to the right of the axis rect. See the documentation there for details on how to
  add and use a color scale.
  
  \section qcpcolormap-appearance Changing the appearance
  
  Most important to the appearance is the color gradient, which can be specified via \ref
  setGradient. See the documentation of \ref QCPColorGradient for details on configuring a color
  gradient.
  
  The \a data range that is mapped to the colors of the gradient can be specified with \ref
  setDataRange. To make the data range encompass the whole data set minimum to maximum, call \ref
  rescaleDataRange. If your data may contain NaN values, use \ref QCPColorGradient::setNanHandling
  to define how they are displayed.
  
  \section qcpcolormap-transparency Transparency
  
  Transparency in color maps can be achieved by two mechanisms. On one hand, you can specify alpha
  values for color stops of the \ref QCPColorGradient, via the regular QColor interface. This will
  cause the color map data which gets mapped to colors around those color stops to appear with the
  accordingly interpolated transparency.
  
  On the other hand you can also directly apply an alpha value to each cell independent of its
  data, by using the alpha map feature of \ref QCPColorMapData. The relevant methods are \ref
  QCPColorMapData::setAlpha, QCPColorMapData::fillAlpha and \ref QCPColorMapData::clearAlpha().
  
  The two transparencies will be joined together in the plot and otherwise not interfere with each
  other. They are mixed in a multiplicative matter, so an alpha of e.g. 50% (128/255) in both modes
  simultaneously, will result in a total transparency of 25% (64/255).
  
  \section qcpcolormap-usage Usage
  
  Like all data representing objects in QCustomPlot, the QCPColorMap is a plottable
  (QCPAbstractPlottable). So the plottable-interface of QCustomPlot applies
  (QCustomPlot::plottable, QCustomPlot::removePlottable, etc.)
  
  Usually, you first create an instance:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcolormap-creation-1
  which registers it with the QCustomPlot instance of the passed axes. Note that this QCustomPlot instance takes
  ownership of the plottable, so do not delete it manually but use QCustomPlot::removePlottable() instead.
  The newly created plottable can be modified, e.g.:
  \snippet documentation/doc-code-snippets/mainwindow.cpp qcpcolormap-creation-2
  
  \note The QCPColorMap always displays the data at equal key/value intervals, even if the key or
  value axis is set to a logarithmic scaling. If you want to use QCPColorMap with logarithmic axes,
  you shouldn't use the \ref QCPColorMapData::setData method as it uses a linear transformation to
  determine the cell index. Rather directly access the cell index with \ref
  QCPColorMapData::setCell.
*/

/* start documentation of inline functions */

/*! \fn QCPColorMapData *QCPColorMap::data() const
  
  Returns a pointer to the internal data storage of type \ref QCPColorMapData. Access this to
  modify data points (cells) and the color map key/value range.
  
  \see setData
*/

/* end documentation of inline functions */

/* start documentation of signals */

/*! \fn void QCPColorMap::dataRangeChanged(const QCPRange &newRange);
  
  This signal is emitted when the data range changes.
  
  \see setDataRange
*/

/*! \fn void QCPColorMap::dataScaleTypeChanged(QCPAxis::ScaleType scaleType);
  
  This signal is emitted when the data scale type changes.
  
  \see setDataScaleType
*/

/*! \fn void QCPColorMap::gradientChanged(const QCPColorGradient &newGradient);
  
  This signal is emitted when the gradient changes.
  
  \see setGradient
*/

/* end documentation of signals */

/*!
  Constructs a color map with the specified \a keyAxis and \a valueAxis.
  
  The created QCPColorMap is automatically registered with the QCustomPlot instance inferred from
  \a keyAxis. This QCustomPlot instance takes ownership of the QCPColorMap, so do not delete it
  manually but use QCustomPlot::removePlottable() instead.
*/
QCPColorMap::QCPColorMap(QCPAxis *keyAxis, QCPAxis *valueAxis) :
  QCPAbstractPlottable(keyAxis, valueAxis),
  mDataScaleType(QCPAxis::stLinear),
  mMapData(new QCPColorMapData(10, 10, QCPRange(0, 5), QCPRange(0, 5))),
  mGradient(QCPColorGradient::gpCold),
  mInterpolate(true),
  mTightBoundary(false),
  mMapImageInvalidated(true)
{
}

QCPColorMap::~QCPColorMap()
{
  delete mMapData;
}

/*!
  Replaces the current \ref data with the provided \a data.
  
  If \a copy is set to true, the \a data object will only be copied. if false, the color map
  takes ownership of the passed data and replaces the internal data pointer with it. This is
  significantly faster than copying for large datasets.
*/
void QCPColorMap::setData(QCPColorMapData *data, bool copy)
{
  if (mMapData == data)
  {
    qDebug() << Q_FUNC_INFO << "The data pointer is already in (and owned by) this plottable" << reinterpret_cast<quintptr>(data);
    return;
  }
  if (copy)
  {
    *mMapData = *data;
  } else
  {
    delete mMapData;
    mMapData = data;
  }
  mMapImageInvalidated = true;
}

/*!
  Sets the data range of this color map to \a dataRange. The data range defines which data values
  are mapped to the color gradient.
  
  To make the data range span the full range of the data set, use \ref rescaleDataRange.
  
  \see QCPColorScale::setDataRange
*/
void QCPColorMap::setDataRange(const QCPRange &dataRange)
{
  if (!QCPRange::validRange(dataRange)) return;
  if (mDataRange.lower != dataRange.lower || mDataRange.upper != dataRange.upper)
  {
    if (mDataScaleType == QCPAxis::stLogarithmic)
      mDataRange = dataRange.sanitizedForLogScale();
    else
      mDataRange = dataRange.sanitizedForLinScale();
    mMapImageInvalidated = true;
    emit dataRangeChanged(mDataRange);
  }
}

/*!
  Sets whether the data is correlated with the color gradient linearly or logarithmically.
  
  \see QCPColorScale::setDataScaleType
*/
void QCPColorMap::setDataScaleType(QCPAxis::ScaleType scaleType)
{
  if (mDataScaleType != scaleType)
  {
    mDataScaleType = scaleType;
    mMapImageInvalidated = true;
    emit dataScaleTypeChanged(mDataScaleType);
    if (mDataScaleType == QCPAxis::stLogarithmic)
      setDataRange(mDataRange.sanitizedForLogScale());
  }
}

/*!
  Sets the color gradient that is used to represent the data. For more details on how to create an
  own gradient or use one of the preset gradients, see \ref QCPColorGradient.
  
  The colors defined by the gradient will be used to represent data values in the currently set
  data range, see \ref setDataRange. Data points that are outside this data range will either be
  colored uniformly with the respective gradient boundary color, or the gradient will repeat,
  depending on \ref QCPColorGradient::setPeriodic.
  
  \see QCPColorScale::setGradient
*/
void QCPColorMap::setGradient(const QCPColorGradient &gradient)
{
  if (mGradient != gradient)
  {
    mGradient = gradient;
    mMapImageInvalidated = true;
    emit gradientChanged(mGradient);
  }
}

/*!
  Sets whether the color map image shall use bicubic interpolation when displaying the color map
  shrinked or expanded, and not at a 1:1 pixel-to-data scale.
  
  \image html QCPColorMap-interpolate.png "A 10*10 color map, with interpolation and without interpolation enabled"
*/
void QCPColorMap::setInterpolate(bool enabled)
{
  mInterpolate = enabled;
  mMapImageInvalidated = true; // because oversampling factors might need to change
}

/*!
  Sets whether the outer most data rows and columns are clipped to the specified key and value
  range (see \ref QCPColorMapData::setKeyRange, \ref QCPColorMapData::setValueRange).
  
  if \a enabled is set to false, the data points at the border of the color map are drawn with the
  same width and height as all other data points. Since the data points are represented by
  rectangles of one color centered on the data coordinate, this means that the shown color map
  extends by half a data point over the specified key/value range in each direction.
  
  \image html QCPColorMap-tightboundary.png "A color map, with tight boundary enabled and disabled"
*/
void QCPColorMap::setTightBoundary(bool enabled)
{
  mTightBoundary = enabled;
}

/*!
  Associates the color scale \a colorScale with this color map.
  
  This means that both the color scale and the color map synchronize their gradient, data range and
  data scale type (\ref setGradient, \ref setDataRange, \ref setDataScaleType). Multiple color maps
  can be associated with one single color scale. This causes the color maps to also synchronize
  those properties, via the mutual color scale.
  
  This function causes the color map to adopt the current color gradient, data range and data scale
  type of \a colorScale. After this call, you may change these properties at either the color map
  or the color scale, and the setting will be applied to both.
  
  Pass \c nullptr as \a colorScale to disconnect the color scale from this color map again.
*/
void QCPColorMap::setColorScale(QCPColorScale *colorScale)
{
  if (mColorScale) // unconnect signals from old color scale
  {
    disconnect(this, SIGNAL(dataRangeChanged(QCPRange)), mColorScale.data(), SLOT(setDataRange(QCPRange)));
    disconnect(this, SIGNAL(dataScaleTypeChanged(QCPAxis::ScaleType)), mColorScale.data(), SLOT(setDataScaleType(QCPAxis::ScaleType)));
    disconnect(this, SIGNAL(gradientChanged(QCPColorGradient)), mColorScale.data(), SLOT(setGradient(QCPColorGradient)));
    disconnect(mColorScale.data(), SIGNAL(dataRangeChanged(QCPRange)), this, SLOT(setDataRange(QCPRange)));
    disconnect(mColorScale.data(), SIGNAL(gradientChanged(QCPColorGradient)), this, SLOT(setGradient(QCPColorGradient)));
    disconnect(mColorScale.data(), SIGNAL(dataScaleTypeChanged(QCPAxis::ScaleType)), this, SLOT(setDataScaleType(QCPAxis::ScaleType)));
  }
  mColorScale = colorScale;
  if (mColorScale) // connect signals to new color scale
  {
    setGradient(mColorScale.data()->gradient());
    setDataRange(mColorScale.data()->dataRange());
    setDataScaleType(mColorScale.data()->dataScaleType());
    connect(this, SIGNAL(dataRangeChanged(QCPRange)), mColorScale.data(), SLOT(setDataRange(QCPRange)));
    connect(this, SIGNAL(dataScaleTypeChanged(QCPAxis::ScaleType)), mColorScale.data(), SLOT(setDataScaleType(QCPAxis::ScaleType)));
    connect(this, SIGNAL(gradientChanged(QCPColorGradient)), mColorScale.data(), SLOT(setGradient(QCPColorGradient)));
    connect(mColorScale.data(), SIGNAL(dataRangeChanged(QCPRange)), this, SLOT(setDataRange(QCPRange)));
    connect(mColorScale.data(), SIGNAL(gradientChanged(QCPColorGradient)), this, SLOT(setGradient(QCPColorGradient)));
    connect(mColorScale.data(), SIGNAL(dataScaleTypeChanged(QCPAxis::ScaleType)), this, SLOT(setDataScaleType(QCPAxis::ScaleType)));
  }
}

/*!
  Sets the data range (\ref setDataRange) to span the minimum and maximum values that occur in the
  current data set. This corresponds to the \ref rescaleKeyAxis or \ref rescaleValueAxis methods,
  only for the third data dimension of the color map.
  
  The minimum and maximum values of the data set are buffered in the internal QCPColorMapData
  instance (\ref data). As data is updated via its \ref QCPColorMapData::setCell or \ref
  QCPColorMapData::setData, the buffered minimum and maximum values are updated, too. For
  performance reasons, however, they are only updated in an expanding fashion. So the buffered
  maximum can only increase and the buffered minimum can only decrease. In consequence, changes to
  the data that actually lower the maximum of the data set (by overwriting the cell holding the
  current maximum with a smaller value), aren't recognized and the buffered maximum overestimates
  the true maximum of the data set. The same happens for the buffered minimum. To recalculate the
  true minimum and maximum by explicitly looking at each cell, the method
  QCPColorMapData::recalculateDataBounds can be used. For convenience, setting the parameter \a
  recalculateDataBounds calls this method before setting the data range to the buffered minimum and
  maximum.
  
  \see setDataRange
*/
void QCPColorMap::rescaleDataRange(bool recalculateDataBounds)
{
  if (recalculateDataBounds)
    mMapData->recalculateDataBounds();
  setDataRange(mMapData->dataBounds());
}

/*!
  Takes the current appearance of the color map and updates the legend icon, which is used to
  represent this color map in the legend (see \ref QCPLegend).
  
  The \a transformMode specifies whether the rescaling is done by a faster, low quality image
  scaling algorithm (Qt::FastTransformation) or by a slower, higher quality algorithm
  (Qt::SmoothTransformation).
  
  The current color map appearance is scaled down to \a thumbSize. Ideally, this should be equal to
  the size of the legend icon (see \ref QCPLegend::setIconSize). If it isn't exactly the configured
  legend icon size, the thumb will be rescaled during drawing of the legend item.
  
  \see setDataRange
*/
void QCPColorMap::updateLegendIcon(Qt::TransformationMode transformMode, const QSize &thumbSize)
{
  if (mMapImage.isNull() && !data()->isEmpty())
    updateMapImage(); // try to update map image if it's null (happens if no draw has happened yet)
  
  if (!mMapImage.isNull()) // might still be null, e.g. if data is empty, so check here again
  {
    bool mirrorX = (keyAxis()->orientation() == Qt::Horizontal ? keyAxis() : valueAxis())->rangeReversed();
    bool mirrorY = (valueAxis()->orientation() == Qt::Vertical ? valueAxis() : keyAxis())->rangeReversed();
    mLegendIcon = QPixmap::fromImage(mMapImage.mirrored(mirrorX, mirrorY)).scaled(thumbSize, Qt::KeepAspectRatio, transformMode);
  }
}

/* inherits documentation from base class */
double QCPColorMap::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if ((onlySelectable && mSelectable == QCP::stNone) || mMapData->isEmpty())
    return -1;
  if (!mKeyAxis || !mValueAxis)
    return -1;
  
  if (mKeyAxis.data()->axisRect()->rect().contains(pos.toPoint()) || mParentPlot->interactions().testFlag(QCP::iSelectPlottablesBeyondAxisRect))
  {
    double posKey, posValue;
    pixelsToCoords(pos, posKey, posValue);
    if (mMapData->keyRange().contains(posKey) && mMapData->valueRange().contains(posValue))
    {
      if (details)
        details->setValue(QCPDataSelection(QCPDataRange(0, 1))); // temporary solution, to facilitate whole-plottable selection. Replace in future version with segmented 2D selection.
      return mParentPlot->selectionTolerance()*0.99;
    }
  }
  return -1;
}

/* inherits documentation from base class */
QCPRange QCPColorMap::getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain) const
{
  foundRange = true;
  QCPRange result = mMapData->keyRange();
  result.normalize();
  if (inSignDomain == QCP::sdPositive)
  {
    if (result.lower <= 0 && result.upper > 0)
      result.lower = result.upper*1e-3;
    else if (result.lower <= 0 && result.upper <= 0)
      foundRange = false;
  } else if (inSignDomain == QCP::sdNegative)
  {
    if (result.upper >= 0 && result.lower < 0)
      result.upper = result.lower*1e-3;
    else if (result.upper >= 0 && result.lower >= 0)
      foundRange = false;
  }
  return result;
}

/* inherits documentation from base class */
QCPRange QCPColorMap::getValueRange(bool &foundRange, QCP::SignDomain inSignDomain, const QCPRange &inKeyRange) const
{
  if (inKeyRange != QCPRange())
  {
    if (mMapData->keyRange().upper < inKeyRange.lower || mMapData->keyRange().lower > inKeyRange.upper)
    {
      foundRange = false;
      return {};
    }
  }
  
  foundRange = true;
  QCPRange result = mMapData->valueRange();
  result.normalize();
  if (inSignDomain == QCP::sdPositive)
  {
    if (result.lower <= 0 && result.upper > 0)
      result.lower = result.upper*1e-3;
    else if (result.lower <= 0 && result.upper <= 0)
      foundRange = false;
  } else if (inSignDomain == QCP::sdNegative)
  {
    if (result.upper >= 0 && result.lower < 0)
      result.upper = result.lower*1e-3;
    else if (result.upper >= 0 && result.lower >= 0)
      foundRange = false;
  }
  return result;
}

/*! \internal
  
  Updates the internal map image buffer by going through the internal \ref QCPColorMapData and
  turning the data values into color pixels with \ref QCPColorGradient::colorize.
  
  This method is called by \ref QCPColorMap::draw if either the data has been modified or the map image
  has been invalidated for a different reason (e.g. a change of the data range with \ref
  setDataRange).
  
  If the map cell count is low, the image created will be oversampled in order to avoid a
  QPainter::drawImage bug which makes inner pixel boundaries jitter when stretch-drawing images
  without smooth transform enabled. Accordingly, oversampling isn't performed if \ref
  setInterpolate is true.
*/
void QCPColorMap::updateMapImage()
{
  QCPAxis *keyAxis = mKeyAxis.data();
  if (!keyAxis) return;
  if (mMapData->isEmpty()) return;
  
  const QImage::Format format = QImage::Format_ARGB32_Premultiplied;
  const int keySize = mMapData->keySize();
  const int valueSize = mMapData->valueSize();
  int keyOversamplingFactor = mInterpolate ? 1 : int(1.0+100.0/double(keySize)); // make mMapImage have at least size 100, factor becomes 1 if size > 200 or interpolation is on
  int valueOversamplingFactor = mInterpolate ? 1 : int(1.0+100.0/double(valueSize)); // make mMapImage have at least size 100, factor becomes 1 if size > 200 or interpolation is on
  
  // resize mMapImage to correct dimensions including possible oversampling factors, according to key/value axes orientation:
  if (keyAxis->orientation() == Qt::Horizontal && (mMapImage.width() != keySize*keyOversamplingFactor || mMapImage.height() != valueSize*valueOversamplingFactor))
    mMapImage = QImage(QSize(keySize*keyOversamplingFactor, valueSize*valueOversamplingFactor), format);
  else if (keyAxis->orientation() == Qt::Vertical && (mMapImage.width() != valueSize*valueOversamplingFactor || mMapImage.height() != keySize*keyOversamplingFactor))
    mMapImage = QImage(QSize(valueSize*valueOversamplingFactor, keySize*keyOversamplingFactor), format);
  
  if (mMapImage.isNull())
  {
    qDebug() << Q_FUNC_INFO << "Couldn't create map image (possibly too large for memory)";
    mMapImage = QImage(QSize(10, 10), format);
    mMapImage.fill(Qt::black);
  } else
  {
    QImage *localMapImage = &mMapImage; // this is the image on which the colorization operates. Either the final mMapImage, or if we need oversampling, mUndersampledMapImage
    if (keyOversamplingFactor > 1 || valueOversamplingFactor > 1)
    {
      // resize undersampled map image to actual key/value cell sizes:
      if (keyAxis->orientation() == Qt::Horizontal && (mUndersampledMapImage.width() != keySize || mUndersampledMapImage.height() != valueSize))
        mUndersampledMapImage = QImage(QSize(keySize, valueSize), format);
      else if (keyAxis->orientation() == Qt::Vertical && (mUndersampledMapImage.width() != valueSize || mUndersampledMapImage.height() != keySize))
        mUndersampledMapImage = QImage(QSize(valueSize, keySize), format);
      localMapImage = &mUndersampledMapImage; // make the colorization run on the undersampled image
    } else if (!mUndersampledMapImage.isNull())
      mUndersampledMapImage = QImage(); // don't need oversampling mechanism anymore (map size has changed) but mUndersampledMapImage still has nonzero size, free it
    
    const double *rawData = mMapData->mData;
    const unsigned char *rawAlpha = mMapData->mAlpha;
    if (keyAxis->orientation() == Qt::Horizontal)
    {
      const int lineCount = valueSize;
      const int rowCount = keySize;
      for (int line=0; line<lineCount; ++line)
      {
        QRgb* pixels = reinterpret_cast<QRgb*>(localMapImage->scanLine(lineCount-1-line)); // invert scanline index because QImage counts scanlines from top, but our vertical index counts from bottom (mathematical coordinate system)
        if (rawAlpha)
          mGradient.colorize(rawData+line*rowCount, rawAlpha+line*rowCount, mDataRange, pixels, rowCount, 1, mDataScaleType==QCPAxis::stLogarithmic);
        else
          mGradient.colorize(rawData+line*rowCount, mDataRange, pixels, rowCount, 1, mDataScaleType==QCPAxis::stLogarithmic);
      }
    } else // keyAxis->orientation() == Qt::Vertical
    {
      const int lineCount = keySize;
      const int rowCount = valueSize;
      for (int line=0; line<lineCount; ++line)
      {
        QRgb* pixels = reinterpret_cast<QRgb*>(localMapImage->scanLine(lineCount-1-line)); // invert scanline index because QImage counts scanlines from top, but our vertical index counts from bottom (mathematical coordinate system)
        if (rawAlpha)
          mGradient.colorize(rawData+line, rawAlpha+line, mDataRange, pixels, rowCount, lineCount, mDataScaleType==QCPAxis::stLogarithmic);
        else
          mGradient.colorize(rawData+line, mDataRange, pixels, rowCount, lineCount, mDataScaleType==QCPAxis::stLogarithmic);
      }
    }
    
    if (keyOversamplingFactor > 1 || valueOversamplingFactor > 1)
    {
      if (keyAxis->orientation() == Qt::Horizontal)
        mMapImage = mUndersampledMapImage.scaled(keySize*keyOversamplingFactor, valueSize*valueOversamplingFactor, Qt::IgnoreAspectRatio, Qt::FastTransformation);
      else
        mMapImage = mUndersampledMapImage.scaled(valueSize*valueOversamplingFactor, keySize*keyOversamplingFactor, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
  }
  mMapData->mDataModified = false;
  mMapImageInvalidated = false;
}

/* inherits documentation from base class */
void QCPColorMap::draw(QCPPainter *painter)
{
  if (mMapData->isEmpty()) return;
  if (!mKeyAxis || !mValueAxis) return;
  applyDefaultAntialiasingHint(painter);
  
  if (mMapData->mDataModified || mMapImageInvalidated)
    updateMapImage();
  
  // use buffer if painting vectorized (PDF):
  const bool useBuffer = painter->modes().testFlag(QCPPainter::pmVectorized);
  QCPPainter *localPainter = painter; // will be redirected to paint on mapBuffer if painting vectorized
  QRectF mapBufferTarget; // the rect in absolute widget coordinates where the visible map portion/buffer will end up in
  QPixmap mapBuffer;
  if (useBuffer)
  {
    const double mapBufferPixelRatio = 3; // factor by which DPI is increased in embedded bitmaps
    mapBufferTarget = painter->clipRegion().boundingRect();
    mapBuffer = QPixmap((mapBufferTarget.size()*mapBufferPixelRatio).toSize());
    mapBuffer.fill(Qt::transparent);
    localPainter = new QCPPainter(&mapBuffer);
    localPainter->scale(mapBufferPixelRatio, mapBufferPixelRatio);
    localPainter->translate(-mapBufferTarget.topLeft());
  }
  
  QRectF imageRect = QRectF(coordsToPixels(mMapData->keyRange().lower, mMapData->valueRange().lower),
                            coordsToPixels(mMapData->keyRange().upper, mMapData->valueRange().upper)).normalized();
  // extend imageRect to contain outer halves/quarters of bordering/cornering pixels (cells are centered on map range boundary):
  double halfCellWidth = 0; // in pixels
  double halfCellHeight = 0; // in pixels
  if (keyAxis()->orientation() == Qt::Horizontal)
  {
    if (mMapData->keySize() > 1)
      halfCellWidth = 0.5*imageRect.width()/double(mMapData->keySize()-1);
    if (mMapData->valueSize() > 1)
      halfCellHeight = 0.5*imageRect.height()/double(mMapData->valueSize()-1);
  } else // keyAxis orientation is Qt::Vertical
  {
    if (mMapData->keySize() > 1)
      halfCellHeight = 0.5*imageRect.height()/double(mMapData->keySize()-1);
    if (mMapData->valueSize() > 1)
      halfCellWidth = 0.5*imageRect.width()/double(mMapData->valueSize()-1);
  }
  imageRect.adjust(-halfCellWidth, -halfCellHeight, halfCellWidth, halfCellHeight);
  const bool mirrorX = (keyAxis()->orientation() == Qt::Horizontal ? keyAxis() : valueAxis())->rangeReversed();
  const bool mirrorY = (valueAxis()->orientation() == Qt::Vertical ? valueAxis() : keyAxis())->rangeReversed();
  const bool smoothBackup = localPainter->renderHints().testFlag(QPainter::SmoothPixmapTransform);
  localPainter->setRenderHint(QPainter::SmoothPixmapTransform, mInterpolate);
  QRegion clipBackup;
  if (mTightBoundary)
  {
    clipBackup = localPainter->clipRegion();
    QRectF tightClipRect = QRectF(coordsToPixels(mMapData->keyRange().lower, mMapData->valueRange().lower),
                                  coordsToPixels(mMapData->keyRange().upper, mMapData->valueRange().upper)).normalized();
    localPainter->setClipRect(tightClipRect, Qt::IntersectClip);
  }
  localPainter->drawImage(imageRect, mMapImage.mirrored(mirrorX, mirrorY));
  if (mTightBoundary)
    localPainter->setClipRegion(clipBackup);
  localPainter->setRenderHint(QPainter::SmoothPixmapTransform, smoothBackup);
  
  if (useBuffer) // localPainter painted to mapBuffer, so now draw buffer with original painter
  {
    delete localPainter;
    painter->drawPixmap(mapBufferTarget.toRect(), mapBuffer);
  }
}

/* inherits documentation from base class */
void QCPColorMap::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{
  applyDefaultAntialiasingHint(painter);
  // draw map thumbnail:
  if (!mLegendIcon.isNull())
  {
    QPixmap scaledIcon = mLegendIcon.scaled(rect.size().toSize(), Qt::KeepAspectRatio, Qt::FastTransformation);
    QRectF iconRect = QRectF(0, 0, scaledIcon.width(), scaledIcon.height());
    iconRect.moveCenter(rect.center());
    painter->drawPixmap(iconRect.topLeft(), scaledIcon);
  }
  /*
  // draw frame:
  painter->setBrush(Qt::NoBrush);
  painter->setPen(Qt::black);
  painter->drawRect(rect.adjusted(1, 1, 0, 0));
  */
}

