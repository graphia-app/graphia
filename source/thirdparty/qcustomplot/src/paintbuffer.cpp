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

#include "paintbuffer.h"

#include "painter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAbstractPaintBuffer
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPAbstractPaintBuffer
  \brief The abstract base class for paint buffers, which define the rendering backend

  This abstract base class defines the basic interface that a paint buffer needs to provide in
  order to be usable by QCustomPlot.

  A paint buffer manages both a surface to draw onto, and the matching paint device. The size of
  the surface can be changed via \ref setSize. External classes (\ref QCustomPlot and \ref
  QCPLayer) request a painter via \ref startPainting and then perform the draw calls. Once the
  painting is complete, \ref donePainting is called, so the paint buffer implementation can do
  clean up if necessary. Before rendering a frame, each paint buffer is usually filled with a color
  using \ref clear (usually the color is \c Qt::transparent), to remove the contents of the
  previous frame.

  The simplest paint buffer implementation is \ref QCPPaintBufferPixmap which allows regular
  software rendering via the raster engine. Hardware accelerated rendering via pixel buffers and
  frame buffer objects is provided by \ref QCPPaintBufferGlPbuffer and \ref QCPPaintBufferGlFbo.
  They are used automatically if \ref QCustomPlot::setOpenGl is enabled.
*/

/* start documentation of pure virtual functions */

/*! \fn virtual QCPPainter *QCPAbstractPaintBuffer::startPainting() = 0

  Returns a \ref QCPPainter which is ready to draw to this buffer. The ownership and thus the
  responsibility to delete the painter after the painting operations are complete is given to the
  caller of this method.

  Once you are done using the painter, delete the painter and call \ref donePainting.

  While a painter generated with this method is active, you must not call \ref setSize, \ref
  setDevicePixelRatio or \ref clear.

  This method may return 0, if a painter couldn't be activated on the buffer. This usually
  indicates a problem with the respective painting backend.
*/

/*! \fn virtual void QCPAbstractPaintBuffer::draw(QCPPainter *painter) const = 0

  Draws the contents of this buffer with the provided \a painter. This is the method that is used
  to finally join all paint buffers and draw them onto the screen.
*/

/*! \fn virtual void QCPAbstractPaintBuffer::clear(const QColor &color) = 0

  Fills the entire buffer with the provided \a color. To have an empty transparent buffer, use the
  named color \c Qt::transparent.

  This method must not be called if there is currently a painter (acquired with \ref startPainting)
  active.
*/

/*! \fn virtual void QCPAbstractPaintBuffer::reallocateBuffer() = 0

  Reallocates the internal buffer with the currently configured size (\ref setSize) and device
  pixel ratio, if applicable (\ref setDevicePixelRatio). It is called as soon as any of those
  properties are changed on this paint buffer.

  \note Subclasses of \ref QCPAbstractPaintBuffer must call their reimplementation of this method
  in their constructor, to perform the first allocation (this can not be done by the base class
  because calling pure virtual methods in base class constructors is not possible).
*/

/* end documentation of pure virtual functions */
/* start documentation of inline functions */

/*! \fn virtual void QCPAbstractPaintBuffer::donePainting()

  If you have acquired a \ref QCPPainter to paint onto this paint buffer via \ref startPainting,
  call this method as soon as you are done with the painting operations and have deleted the
  painter.

  paint buffer subclasses may use this method to perform any type of cleanup that is necessary. The
  default implementation does nothing.
*/

/* end documentation of inline functions */

/*!
  Creates a paint buffer and initializes it with the provided \a size and \a devicePixelRatio.

  Subclasses must call their \ref reallocateBuffer implementation in their respective constructors.
*/
QCPAbstractPaintBuffer::QCPAbstractPaintBuffer(const QSize &size, double devicePixelRatio) :
  mSize(size),
  mDevicePixelRatio(devicePixelRatio),
  mInvalidated(true)
{
}

QCPAbstractPaintBuffer::~QCPAbstractPaintBuffer()
{
}

/*!
  Sets the paint buffer size.

  The buffer is reallocated (by calling \ref reallocateBuffer), so any painters that were obtained
  by \ref startPainting are invalidated and must not be used after calling this method.

  If \a size is already the current buffer size, this method does nothing.
*/
void QCPAbstractPaintBuffer::setSize(const QSize &size)
{
  if (mSize != size)
  {
    mSize = size;
    reallocateBuffer();
  }
}

/*!
  Sets the invalidated flag to \a invalidated.

  This mechanism is used internally in conjunction with isolated replotting of \ref QCPLayer
  instances (in \ref QCPLayer::lmBuffered mode). If \ref QCPLayer::replot is called on a buffered
  layer, i.e. an isolated repaint of only that layer (and its dedicated paint buffer) is requested,
  QCustomPlot will decide depending on the invalidated flags of other paint buffers whether it also
  replots them, instead of only the layer on which the replot was called.

  The invalidated flag is set to true when \ref QCPLayer association has changed, i.e. if layers
  were added or removed from this buffer, or if they were reordered. It is set to false as soon as
  all associated \ref QCPLayer instances are drawn onto the buffer.

  Under normal circumstances, it is not necessary to manually call this method.
*/
void QCPAbstractPaintBuffer::setInvalidated(bool invalidated)
{
  mInvalidated = invalidated;
}

/*!
  Sets the device pixel ratio to \a ratio. This is useful to render on high-DPI output devices.
  The ratio is automatically set to the device pixel ratio used by the parent QCustomPlot instance.

  The buffer is reallocated (by calling \ref reallocateBuffer), so any painters that were obtained
  by \ref startPainting are invalidated and must not be used after calling this method.

  \note This method is only available for Qt versions 5.4 and higher.
*/
void QCPAbstractPaintBuffer::setDevicePixelRatio(double ratio)
{
  if (!qFuzzyCompare(ratio, mDevicePixelRatio))
  {
#ifdef QCP_DEVICEPIXELRATIO_SUPPORTED
    mDevicePixelRatio = ratio;
    reallocateBuffer();
#else
    qDebug() << Q_FUNC_INFO << "Device pixel ratios not supported for Qt versions before 5.4";
    mDevicePixelRatio = 1.0;
#endif
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPaintBufferPixmap
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPaintBufferPixmap
  \brief A paint buffer based on QPixmap, using software raster rendering

  This paint buffer is the default and fall-back paint buffer which uses software rendering and
  QPixmap as internal buffer. It is used if \ref QCustomPlot::setOpenGl is false.
*/

/*!
  Creates a pixmap paint buffer instancen with the specified \a size and \a devicePixelRatio, if
  applicable.
*/
QCPPaintBufferPixmap::QCPPaintBufferPixmap(const QSize &size, double devicePixelRatio) :
  QCPAbstractPaintBuffer(size, devicePixelRatio)
{
  QCPPaintBufferPixmap::reallocateBuffer();
}

QCPPaintBufferPixmap::~QCPPaintBufferPixmap()
{
}

/* inherits documentation from base class */
QCPPainter *QCPPaintBufferPixmap::startPainting()
{
  QCPPainter *result = new QCPPainter(&mBuffer);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  result->setRenderHint(QPainter::Antialiasing);
#endif
  return result;
}

/* inherits documentation from base class */
void QCPPaintBufferPixmap::draw(QCPPainter *painter) const
{
  if (painter && painter->isActive())
    painter->drawPixmap(0, 0, mBuffer);
  else
    qDebug() << Q_FUNC_INFO << "invalid or inactive painter passed";
}

/* inherits documentation from base class */
void QCPPaintBufferPixmap::clear(const QColor &color)
{
  mBuffer.fill(color);
}

/* inherits documentation from base class */
void QCPPaintBufferPixmap::reallocateBuffer()
{
  setInvalidated();
  if (!qFuzzyCompare(1.0, mDevicePixelRatio))
  {
#ifdef QCP_DEVICEPIXELRATIO_SUPPORTED
    mBuffer = QPixmap(mSize*mDevicePixelRatio);
    mBuffer.setDevicePixelRatio(mDevicePixelRatio);
#else
    qDebug() << Q_FUNC_INFO << "Device pixel ratios not supported for Qt versions before 5.4";
    mDevicePixelRatio = 1.0;
    mBuffer = QPixmap(mSize);
#endif
  } else
  {
    mBuffer = QPixmap(mSize);
  }
}


#ifdef QCP_OPENGL_PBUFFER
////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPaintBufferGlPbuffer
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPaintBufferGlPbuffer
  \brief A paint buffer based on OpenGL pixel buffers, using hardware accelerated rendering

  This paint buffer is one of the OpenGL paint buffers which facilitate hardware accelerated plot
  rendering. It is based on OpenGL pixel buffers (pbuffer) and is used in Qt versions before 5.0.
  (See \ref QCPPaintBufferGlFbo used in newer Qt versions.)

  The OpenGL paint buffers are used if \ref QCustomPlot::setOpenGl is set to true, and if they are
  supported by the system.
*/

/*!
  Creates a \ref QCPPaintBufferGlPbuffer instance with the specified \a size and \a
  devicePixelRatio, if applicable.

  The parameter \a multisamples defines how many samples are used per pixel. Higher values thus
  result in higher quality antialiasing. If the specified \a multisamples value exceeds the
  capability of the graphics hardware, the highest supported multisampling is used.
*/
QCPPaintBufferGlPbuffer::QCPPaintBufferGlPbuffer(const QSize &size, double devicePixelRatio, int multisamples) :
  QCPAbstractPaintBuffer(size, devicePixelRatio),
  mGlPBuffer(0),
  mMultisamples(qMax(0, multisamples))
{
  QCPPaintBufferGlPbuffer::reallocateBuffer();
}

QCPPaintBufferGlPbuffer::~QCPPaintBufferGlPbuffer()
{
  if (mGlPBuffer)
    delete mGlPBuffer;
}

/* inherits documentation from base class */
QCPPainter *QCPPaintBufferGlPbuffer::startPainting()
{
  if (!mGlPBuffer->isValid())
  {
    qDebug() << Q_FUNC_INFO << "OpenGL frame buffer object doesn't exist, reallocateBuffer was not called?";
    return 0;
  }
  
  QCPPainter *result = new QCPPainter(mGlPBuffer);
  result->setRenderHint(QPainter::Antialiasing);
  return result;
}

/* inherits documentation from base class */
void QCPPaintBufferGlPbuffer::draw(QCPPainter *painter) const
{
  if (!painter || !painter->isActive())
  {
    qDebug() << Q_FUNC_INFO << "invalid or inactive painter passed";
    return;
  }
  if (!mGlPBuffer->isValid())
  {
    qDebug() << Q_FUNC_INFO << "OpenGL pbuffer isn't valid, reallocateBuffer was not called?";
    return;
  }
  painter->drawImage(0, 0, mGlPBuffer->toImage());
}

/* inherits documentation from base class */
void QCPPaintBufferGlPbuffer::clear(const QColor &color)
{
  if (mGlPBuffer->isValid())
  {
    mGlPBuffer->makeCurrent();
    glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mGlPBuffer->doneCurrent();
  } else
    qDebug() << Q_FUNC_INFO << "OpenGL pbuffer invalid or context not current";
}

/* inherits documentation from base class */
void QCPPaintBufferGlPbuffer::reallocateBuffer()
{
  if (mGlPBuffer)
    delete mGlPBuffer;
  
  QGLFormat format;
  format.setAlpha(true);
  format.setSamples(mMultisamples);
  mGlPBuffer = new QGLPixelBuffer(mSize, format);
}
#endif // QCP_OPENGL_PBUFFER


#ifdef QCP_OPENGL_FBO
////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPaintBufferGlFbo
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPaintBufferGlFbo
  \brief A paint buffer based on OpenGL frame buffers objects, using hardware accelerated rendering

  This paint buffer is one of the OpenGL paint buffers which facilitate hardware accelerated plot
  rendering. It is based on OpenGL frame buffer objects (fbo) and is used in Qt versions 5.0 and
  higher. (See \ref QCPPaintBufferGlPbuffer used in older Qt versions.)

  The OpenGL paint buffers are used if \ref QCustomPlot::setOpenGl is set to true, and if they are
  supported by the system.
*/

/*!
  Creates a \ref QCPPaintBufferGlFbo instance with the specified \a size and \a devicePixelRatio,
  if applicable.

  All frame buffer objects shall share one OpenGL context and paint device, which need to be set up
  externally and passed via \a glContext and \a glPaintDevice. The set-up is done in \ref
  QCustomPlot::setupOpenGl and the context and paint device are managed by the parent QCustomPlot
  instance.
*/
QCPPaintBufferGlFbo::QCPPaintBufferGlFbo(const QSize &size, double devicePixelRatio, QWeakPointer<QOpenGLContext> glContext, QWeakPointer<QOpenGLPaintDevice> glPaintDevice) :
  QCPAbstractPaintBuffer(size, devicePixelRatio),
  mGlContext(glContext),
  mGlPaintDevice(glPaintDevice),
  mGlFrameBuffer(0)
{
  QCPPaintBufferGlFbo::reallocateBuffer();
}

QCPPaintBufferGlFbo::~QCPPaintBufferGlFbo()
{
  if (mGlFrameBuffer)
    delete mGlFrameBuffer;
}

/* inherits documentation from base class */
QCPPainter *QCPPaintBufferGlFbo::startPainting()
{
  QSharedPointer<QOpenGLPaintDevice> paintDevice = mGlPaintDevice.toStrongRef();
  QSharedPointer<QOpenGLContext> context = mGlContext.toStrongRef();
  if (!paintDevice)
  {
    qDebug() << Q_FUNC_INFO << "OpenGL paint device doesn't exist";
    return 0;
  }
  if (!context)
  {
    qDebug() << Q_FUNC_INFO << "OpenGL context doesn't exist";
    return 0;
  }
  if (!mGlFrameBuffer)
  {
    qDebug() << Q_FUNC_INFO << "OpenGL frame buffer object doesn't exist, reallocateBuffer was not called?";
    return 0;
  }
  
  if (QOpenGLContext::currentContext() != context.data())
    context->makeCurrent(context->surface());
  mGlFrameBuffer->bind();
  QCPPainter *result = new QCPPainter(paintDevice.data());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  result->setRenderHint(QPainter::Antialiasing);
#endif
  return result;
}

/* inherits documentation from base class */
void QCPPaintBufferGlFbo::donePainting()
{
  if (mGlFrameBuffer && mGlFrameBuffer->isBound())
    mGlFrameBuffer->release();
  else
    qDebug() << Q_FUNC_INFO << "Either OpenGL frame buffer not valid or was not bound";
}

/* inherits documentation from base class */
void QCPPaintBufferGlFbo::draw(QCPPainter *painter) const
{
  if (!painter || !painter->isActive())
  {
    qDebug() << Q_FUNC_INFO << "invalid or inactive painter passed";
    return;
  }
  if (!mGlFrameBuffer)
  {
    qDebug() << Q_FUNC_INFO << "OpenGL frame buffer object doesn't exist, reallocateBuffer was not called?";
    return;
  }
  painter->drawImage(0, 0, mGlFrameBuffer->toImage());
}

/* inherits documentation from base class */
void QCPPaintBufferGlFbo::clear(const QColor &color)
{
  QSharedPointer<QOpenGLContext> context = mGlContext.toStrongRef();
  if (!context)
  {
    qDebug() << Q_FUNC_INFO << "OpenGL context doesn't exist";
    return;
  }
  if (!mGlFrameBuffer)
  {
    qDebug() << Q_FUNC_INFO << "OpenGL frame buffer object doesn't exist, reallocateBuffer was not called?";
    return;
  }
  
  if (QOpenGLContext::currentContext() != context.data())
    context->makeCurrent(context->surface());
  mGlFrameBuffer->bind();
  mGlContext.toStrongRef()->functions()->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
  mGlContext.toStrongRef()->functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mGlFrameBuffer->release();
}

/* inherits documentation from base class */
void QCPPaintBufferGlFbo::reallocateBuffer()
{
  // release and delete possibly existing framebuffer:
  if (mGlFrameBuffer)
  {
    if (mGlFrameBuffer->isBound())
      mGlFrameBuffer->release();
    delete mGlFrameBuffer;
    mGlFrameBuffer = 0;
  }
  
  QSharedPointer<QOpenGLPaintDevice> paintDevice = mGlPaintDevice.toStrongRef();
  QSharedPointer<QOpenGLContext> context = mGlContext.toStrongRef();
  if (!paintDevice)
  {
    qDebug() << Q_FUNC_INFO << "OpenGL paint device doesn't exist";
    return;
  }
  if (!context)
  {
    qDebug() << Q_FUNC_INFO << "OpenGL context doesn't exist";
    return;
  }
  
  // create new fbo with appropriate size:
  context->makeCurrent(context->surface());
  QOpenGLFramebufferObjectFormat frameBufferFormat;
  frameBufferFormat.setSamples(context->format().samples());
  frameBufferFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
  mGlFrameBuffer = new QOpenGLFramebufferObject(mSize*mDevicePixelRatio, frameBufferFormat);
  if (paintDevice->size() != mSize*mDevicePixelRatio)
    paintDevice->setSize(mSize*mDevicePixelRatio);
#ifdef QCP_DEVICEPIXELRATIO_SUPPORTED
  paintDevice->setDevicePixelRatio(mDevicePixelRatio);
#endif
}
#endif // QCP_OPENGL_FBO
