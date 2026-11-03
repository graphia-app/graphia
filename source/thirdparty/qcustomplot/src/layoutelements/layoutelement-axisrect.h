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

#ifndef QCP_LAYOUTELEMENT_AXISRECT_H
#define QCP_LAYOUTELEMENT_AXISRECT_H

#include "../global.h"
#include "../axis/axis.h"
#include "../layout.h"

class QCPPainter;
class QCustomPlot;
class QCPAxis;
class QCPAbstractPlottable;
class QCPGraph;
class QCPAbstractItem;

class QCP_LIB_DECL QCPAxisRect : public QCPLayoutElement
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPixmap background READ background WRITE setBackground)
  Q_PROPERTY(bool backgroundScaled READ backgroundScaled WRITE setBackgroundScaled)
  Q_PROPERTY(Qt::AspectRatioMode backgroundScaledMode READ backgroundScaledMode WRITE setBackgroundScaledMode)
  Q_PROPERTY(Qt::Orientations rangeDrag READ rangeDrag WRITE setRangeDrag)
  Q_PROPERTY(Qt::Orientations rangeZoom READ rangeZoom WRITE setRangeZoom)
  /// \endcond
public:
  explicit QCPAxisRect(QCustomPlot *parentPlot, bool setupDefaultAxes=true);
  virtual ~QCPAxisRect() Q_DECL_OVERRIDE;
  
  // getters:
  QPixmap background() const { return mBackgroundPixmap; }
  QBrush backgroundBrush() const { return mBackgroundBrush; }
  bool backgroundScaled() const { return mBackgroundScaled; }
  Qt::AspectRatioMode backgroundScaledMode() const { return mBackgroundScaledMode; }
  Qt::Orientations rangeDrag() const { return mRangeDrag; }
  Qt::Orientations rangeZoom() const { return mRangeZoom; }
  QCPAxis *rangeDragAxis(Qt::Orientation orientation);
  QCPAxis *rangeZoomAxis(Qt::Orientation orientation);
  QList<QCPAxis*> rangeDragAxes(Qt::Orientation orientation);
  QList<QCPAxis*> rangeZoomAxes(Qt::Orientation orientation);
  double rangeZoomFactor(Qt::Orientation orientation);
  
  // setters:
  void setBackground(const QPixmap &pm);
  void setBackground(const QPixmap &pm, bool scaled, Qt::AspectRatioMode mode=Qt::KeepAspectRatioByExpanding);
  void setBackground(const QBrush &brush);
  void setBackgroundScaled(bool scaled);
  void setBackgroundScaledMode(Qt::AspectRatioMode mode);
  void setRangeDrag(Qt::Orientations orientations);
  void setRangeZoom(Qt::Orientations orientations);
  void setRangeDragAxes(QCPAxis *horizontal, QCPAxis *vertical);
  void setRangeDragAxes(QList<QCPAxis*> axes);
  void setRangeDragAxes(QList<QCPAxis*> horizontal, QList<QCPAxis*> vertical);
  void setRangeZoomAxes(QCPAxis *horizontal, QCPAxis *vertical);
  void setRangeZoomAxes(QList<QCPAxis*> axes);
  void setRangeZoomAxes(QList<QCPAxis*> horizontal, QList<QCPAxis*> vertical);
  void setRangeZoomFactor(double horizontalFactor, double verticalFactor);
  void setRangeZoomFactor(double factor);
  
  // non-property methods:
  int axisCount(QCPAxis::AxisType type) const;
  QCPAxis *axis(QCPAxis::AxisType type, int index=0) const;
  QList<QCPAxis*> axes(QCPAxis::AxisTypes types) const;
  QList<QCPAxis*> axes() const;
  QCPAxis *addAxis(QCPAxis::AxisType type, QCPAxis *axis=nullptr);
  QList<QCPAxis*> addAxes(QCPAxis::AxisTypes types);
  bool removeAxis(QCPAxis *axis);
  QCPLayoutInset *insetLayout() const { return mInsetLayout; }
  
  void zoom(const QRectF &pixelRect);
  void zoom(const QRectF &pixelRect, const QList<QCPAxis*> &affectedAxes);
  void setupFullAxesBox(bool connectRanges=false);
  QList<QCPAbstractPlottable*> plottables() const;
  QList<QCPGraph*> graphs() const;
  QList<QCPAbstractItem*> items() const;
  
  // read-only interface imitating a QRect:
  int left() const { return mRect.left(); }
  int right() const { return mRect.right(); }
  int top() const { return mRect.top(); }
  int bottom() const { return mRect.bottom(); }
  int width() const { return mRect.width(); }
  int height() const { return mRect.height(); }
  QSize size() const { return mRect.size(); }
  QPoint topLeft() const { return mRect.topLeft(); }
  QPoint topRight() const { return mRect.topRight(); }
  QPoint bottomLeft() const { return mRect.bottomLeft(); }
  QPoint bottomRight() const { return mRect.bottomRight(); }
  QPoint center() const { return mRect.center(); }
  
  // reimplemented virtual methods:
  virtual void update(UpdatePhase phase) Q_DECL_OVERRIDE;
  virtual QList<QCPLayoutElement*> elements(bool recursive) const Q_DECL_OVERRIDE;

protected:
  // property members:
  QBrush mBackgroundBrush;
  QPixmap mBackgroundPixmap;
  QPixmap mScaledBackgroundPixmap;
  bool mBackgroundScaled;
  Qt::AspectRatioMode mBackgroundScaledMode;
  QCPLayoutInset *mInsetLayout;
  Qt::Orientations mRangeDrag, mRangeZoom;
  QList<QPointer<QCPAxis> > mRangeDragHorzAxis, mRangeDragVertAxis;
  QList<QPointer<QCPAxis> > mRangeZoomHorzAxis, mRangeZoomVertAxis;
  double mRangeZoomFactorHorz, mRangeZoomFactorVert;
  
  // non-property members:
  QList<QCPRange> mDragStartHorzRange, mDragStartVertRange;
  QCP::AntialiasedElements mAADragBackup, mNotAADragBackup;
  bool mDragging;
  QHash<QCPAxis::AxisType, QList<QCPAxis*> > mAxes;
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  virtual int calculateAutoMargin(QCP::MarginSide side) Q_DECL_OVERRIDE;
  virtual void layoutChanged() Q_DECL_OVERRIDE;
  // events:
  virtual void mousePressEvent(QMouseEvent *event, const QVariant &details) Q_DECL_OVERRIDE;
  virtual void mouseMoveEvent(QMouseEvent *event, const QPointF &startPos) Q_DECL_OVERRIDE;
  virtual void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos) Q_DECL_OVERRIDE;
  virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
  
  // non-property methods:
  void drawBackground(QCPPainter *painter);
  void updateAxesOffset(QCPAxis::AxisType type);
  
private:
  Q_DISABLE_COPY(QCPAxisRect)
  
  friend class QCustomPlot;
};


#endif // QCP_LAYOUTELEMENT_AXISRECT_H
