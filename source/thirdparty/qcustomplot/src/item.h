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

#ifndef QCP_ITEM_H
#define QCP_ITEM_H

#include "global.h"
#include "vector2d.h"
#include "layer.h"
#include "axis/axis.h"

class QCPPainter;
class QCustomPlot;
class QCPItemPosition;
class QCPAbstractItem;
class QCPAxisRect;

class QCP_LIB_DECL QCPItemAnchor
{
  Q_GADGET
public:
  QCPItemAnchor(QCustomPlot *parentPlot, QCPAbstractItem *parentItem, const QString &name, int anchorId=-1);
  virtual ~QCPItemAnchor();
  
  // getters:
  QString name() const { return mName; }
  virtual QPointF pixelPosition() const;
  
protected:
  // property members:
  QString mName;
  
  // non-property members:
  QCustomPlot *mParentPlot;
  QCPAbstractItem *mParentItem;
  int mAnchorId;
  QSet<QCPItemPosition*> mChildrenX, mChildrenY;
  
  // introduced virtual methods:
  virtual QCPItemPosition *toQCPItemPosition() { return nullptr; }
  
  // non-virtual methods:
  void addChildX(QCPItemPosition* pos); // called from pos when this anchor is set as parent
  void removeChildX(QCPItemPosition *pos); // called from pos when its parent anchor is reset or pos deleted
  void addChildY(QCPItemPosition* pos); // called from pos when this anchor is set as parent
  void removeChildY(QCPItemPosition *pos); // called from pos when its parent anchor is reset or pos deleted
  
private:
  Q_DISABLE_COPY(QCPItemAnchor)
  
  friend class QCPItemPosition;
};



class QCP_LIB_DECL QCPItemPosition : public QCPItemAnchor
{
  Q_GADGET
public:
  /*!
    Defines the ways an item position can be specified. Thus it defines what the numbers passed to
    \ref setCoords actually mean.
    
    \see setType
  */
  enum PositionType { ptAbsolute        ///< Static positioning in pixels, starting from the top left corner of the viewport/widget.
                      ,ptViewportRatio  ///< Static positioning given by a fraction of the viewport size. For example, if you call setCoords(0, 0), the position will be at the top
                                        ///< left corner of the viewport/widget. setCoords(1, 1) will be at the bottom right corner, setCoords(0.5, 0) will be horizontally centered and
                                        ///< vertically at the top of the viewport/widget, etc.
                      ,ptAxisRectRatio  ///< Static positioning given by a fraction of the axis rect size (see \ref setAxisRect). For example, if you call setCoords(0, 0), the position will be at the top
                                        ///< left corner of the axis rect. setCoords(1, 1) will be at the bottom right corner, setCoords(0.5, 0) will be horizontally centered and
                                        ///< vertically at the top of the axis rect, etc. You can also go beyond the axis rect by providing negative coordinates or coordinates larger than 1.
                      ,ptPlotCoords     ///< Dynamic positioning at a plot coordinate defined by two axes (see \ref setAxes).
                    };
  Q_ENUM(PositionType)
  
  QCPItemPosition(QCustomPlot *parentPlot, QCPAbstractItem *parentItem, const QString &name);
  virtual ~QCPItemPosition() Q_DECL_OVERRIDE;
  
  // getters:
  PositionType type() const { return typeX(); }
  PositionType typeX() const { return mPositionTypeX; }
  PositionType typeY() const { return mPositionTypeY; }
  QCPItemAnchor *parentAnchor() const { return parentAnchorX(); }
  QCPItemAnchor *parentAnchorX() const { return mParentAnchorX; }
  QCPItemAnchor *parentAnchorY() const { return mParentAnchorY; }
  double key() const { return mKey; }
  double value() const { return mValue; }
  QPointF coords() const { return QPointF(mKey, mValue); }
  QCPAxis *keyAxis() const { return mKeyAxis.data(); }
  QCPAxis *valueAxis() const { return mValueAxis.data(); }
  QCPAxisRect *axisRect() const;
  virtual QPointF pixelPosition() const Q_DECL_OVERRIDE;
  
  // setters:
  void setType(PositionType type);
  void setTypeX(PositionType type);
  void setTypeY(PositionType type);
  bool setParentAnchor(QCPItemAnchor *parentAnchor, bool keepPixelPosition=false);
  bool setParentAnchorX(QCPItemAnchor *parentAnchor, bool keepPixelPosition=false);
  bool setParentAnchorY(QCPItemAnchor *parentAnchor, bool keepPixelPosition=false);
  void setCoords(double key, double value);
  void setCoords(const QPointF &pos);
  void setAxes(QCPAxis* keyAxis, QCPAxis* valueAxis);
  void setAxisRect(QCPAxisRect *axisRect);
  void setPixelPosition(const QPointF &pixelPosition);
  
protected:
  // property members:
  PositionType mPositionTypeX, mPositionTypeY;
  QPointer<QCPAxis> mKeyAxis, mValueAxis;
  QPointer<QCPAxisRect> mAxisRect;
  double mKey, mValue;
  QCPItemAnchor *mParentAnchorX, *mParentAnchorY;
  
  // reimplemented virtual methods:
  virtual QCPItemPosition *toQCPItemPosition() Q_DECL_OVERRIDE { return this; }
  
private:
  Q_DISABLE_COPY(QCPItemPosition)
  
};
Q_DECLARE_METATYPE(QCPItemPosition::PositionType)


class QCP_LIB_DECL QCPAbstractItem : public QCPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(bool clipToAxisRect READ clipToAxisRect WRITE setClipToAxisRect)
  Q_PROPERTY(QCPAxisRect* clipAxisRect READ clipAxisRect WRITE setClipAxisRect)
  Q_PROPERTY(bool selectable READ selectable WRITE setSelectable NOTIFY selectableChanged)
  Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectionChanged)
  /// \endcond
public:
  explicit QCPAbstractItem(QCustomPlot *parentPlot);
  virtual ~QCPAbstractItem() Q_DECL_OVERRIDE;
  
  // getters:
  bool clipToAxisRect() const { return mClipToAxisRect; }
  QCPAxisRect *clipAxisRect() const;
  bool selectable() const { return mSelectable; }
  bool selected() const { return mSelected; }
  
  // setters:
  void setClipToAxisRect(bool clip);
  void setClipAxisRect(QCPAxisRect *rect);
  Q_SLOT void setSelectable(bool selectable);
  Q_SLOT void setSelected(bool selected);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE = 0;
  
  // non-virtual methods:
  QList<QCPItemPosition*> positions() const { return mPositions; }
  QList<QCPItemAnchor*> anchors() const { return mAnchors; }
  QCPItemPosition *position(const QString &name) const;
  QCPItemAnchor *anchor(const QString &name) const;
  bool hasAnchor(const QString &name) const;
  
signals:
  void selectionChanged(bool selected);
  void selectableChanged(bool selectable);
  
protected:
  // property members:
  bool mClipToAxisRect;
  QPointer<QCPAxisRect> mClipAxisRect;
  QList<QCPItemPosition*> mPositions;
  QList<QCPItemAnchor*> mAnchors;
  bool mSelectable, mSelected;
  
  // reimplemented virtual methods:
  virtual QCP::Interaction selectionCategory() const Q_DECL_OVERRIDE;
  virtual QRect clipRect() const Q_DECL_OVERRIDE;
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE = 0;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) Q_DECL_OVERRIDE;
  virtual void deselectEvent(bool *selectionStateChanged) Q_DECL_OVERRIDE;
  
  // introduced virtual methods:
  virtual QPointF anchorPixelPosition(int anchorId) const;
  
  // non-virtual methods:
  double rectDistance(const QRectF &rect, const QPointF &pos, bool filledRect) const;
  QCPItemPosition *createPosition(const QString &name);
  QCPItemAnchor *createAnchor(const QString &name, int anchorId);
  
private:
  Q_DISABLE_COPY(QCPAbstractItem)
  
  friend class QCustomPlot;
  friend class QCPItemAnchor;
};

#endif // QCP_ITEM_H
