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

#ifndef QCP_LAYER_H
#define QCP_LAYER_H

#include "global.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  Q_MOC_INCLUDE("core.h") // Qt6 needs this if using forward declared types in Q_PROPERTY
#endif
#include "paintbuffer.h"

class QCPPainter;
class QCustomPlot;
class QCPLayerable;
class QCPLayoutElement;
class QCPLayout;

class QCP_LIB_DECL QCPLayer : public QObject
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QCustomPlot* parentPlot READ parentPlot)
  Q_PROPERTY(QString name READ name)
  Q_PROPERTY(int index READ index)
  Q_PROPERTY(QList<QCPLayerable*> children READ children)
  Q_PROPERTY(bool visible READ visible WRITE setVisible)
  Q_PROPERTY(LayerMode mode READ mode WRITE setMode)
  /// \endcond
public:
  
  /*!
    Defines the different rendering modes of a layer. Depending on the mode, certain layers can be
    replotted individually, without the need to replot (possibly complex) layerables on other
    layers.

    \see setMode
  */
  enum LayerMode { lmLogical   ///< Layer is used only for rendering order, and shares paint buffer with all other adjacent logical layers.
                   ,lmBuffered ///< Layer has its own paint buffer and may be replotted individually (see \ref replot).
                 };
  Q_ENUM(LayerMode)
  
  QCPLayer(QCustomPlot* parentPlot, const QString &layerName);
  virtual ~QCPLayer();
  
  // getters:
  QCustomPlot *parentPlot() const { return mParentPlot; }
  QString name() const { return mName; }
  int index() const { return mIndex; }
  QList<QCPLayerable*> children() const { return mChildren; }
  bool visible() const { return mVisible; }
  LayerMode mode() const { return mMode; }
  
  // setters:
  void setVisible(bool visible);
  void setMode(LayerMode mode);
  
  // non-virtual methods:
  void replot();
  
protected:
  // property members:
  QCustomPlot *mParentPlot;
  QString mName;
  int mIndex;
  QList<QCPLayerable*> mChildren;
  bool mVisible;
  LayerMode mMode;
  
  // non-property members:
  QWeakPointer<QCPAbstractPaintBuffer> mPaintBuffer;
  
  // non-virtual methods:
  void draw(QCPPainter *painter);
  void drawToPaintBuffer();
  void addChild(QCPLayerable *layerable, bool prepend);
  void removeChild(QCPLayerable *layerable);
  
private:
  Q_DISABLE_COPY(QCPLayer)
  
  friend class QCustomPlot;
  friend class QCPLayerable;
};
Q_DECLARE_METATYPE(QCPLayer::LayerMode)

class QCP_LIB_DECL QCPLayerable : public QObject
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(bool visible READ visible WRITE setVisible)
  Q_PROPERTY(QCustomPlot* parentPlot READ parentPlot)
  Q_PROPERTY(QCPLayerable* parentLayerable READ parentLayerable)
  Q_PROPERTY(QCPLayer* layer READ layer WRITE setLayer NOTIFY layerChanged)
  Q_PROPERTY(bool antialiased READ antialiased WRITE setAntialiased)
  /// \endcond
public:
  QCPLayerable(QCustomPlot *plot, QString targetLayer=QString(), QCPLayerable *parentLayerable=nullptr);
  virtual ~QCPLayerable();
  
  // getters:
  bool visible() const { return mVisible; }
  QCustomPlot *parentPlot() const { return mParentPlot; }
  QCPLayerable *parentLayerable() const { return mParentLayerable.data(); }
  QCPLayer *layer() const { return mLayer; }
  bool antialiased() const { return mAntialiased; }
  
  // setters:
  void setVisible(bool on);
  Q_SLOT bool setLayer(QCPLayer *layer);
  bool setLayer(const QString &layerName);
  void setAntialiased(bool enabled);
  
  // introduced virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const;

  // non-property methods:
  bool realVisibility() const;
  
signals:
  void layerChanged(QCPLayer *newLayer);
  
protected:
  // property members:
  bool mVisible;
  QCustomPlot *mParentPlot;
  QPointer<QCPLayerable> mParentLayerable;
  QCPLayer *mLayer;
  bool mAntialiased;
  
  // introduced virtual methods:
  virtual void parentPlotInitialized(QCustomPlot *parentPlot);
  virtual QCP::Interaction selectionCategory() const;
  virtual QRect clipRect() const;
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const = 0;
  virtual void draw(QCPPainter *painter) = 0;
  // selection events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged);
  virtual void deselectEvent(bool *selectionStateChanged);
  // low-level mouse events:
  virtual void mousePressEvent(QMouseEvent *event, const QVariant &details);
  virtual void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos);
  virtual void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos);
  virtual void mouseDoubleClickEvent(QMouseEvent *event, const QVariant &details);
  virtual void wheelEvent(QWheelEvent *event);
  
  // non-property methods:
  void initializeParentPlot(QCustomPlot *parentPlot);
  void setParentLayerable(QCPLayerable* parentLayerable);
  bool moveToLayer(QCPLayer *layer, bool prepend);
  void applyAntialiasingHint(QCPPainter *painter, bool localAntialiased, QCP::AntialiasedElement overrideElement) const;
  
private:
  Q_DISABLE_COPY(QCPLayerable)
  
  friend class QCustomPlot;
  friend class QCPLayer;
  friend class QCPAxisRect;
};

#endif // QCP_LAYER_H
