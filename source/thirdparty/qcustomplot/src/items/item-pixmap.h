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

#ifndef QCP_ITEM_PIXMAP_H
#define QCP_ITEM_PIXMAP_H

#include "../global.h"
#include "../item.h"

class QCPPainter;
class QCustomPlot;

class QCP_LIB_DECL QCPItemPixmap : public QCPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)
  Q_PROPERTY(bool scaled READ scaled WRITE setScaled)
  Q_PROPERTY(Qt::AspectRatioMode aspectRatioMode READ aspectRatioMode)
  Q_PROPERTY(Qt::TransformationMode transformationMode READ transformationMode)
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  /// \endcond
public:
  explicit QCPItemPixmap(QCustomPlot *parentPlot);
  virtual ~QCPItemPixmap() Q_DECL_OVERRIDE;
  
  // getters:
  QPixmap pixmap() const { return mPixmap; }
  bool scaled() const { return mScaled; }
  Qt::AspectRatioMode aspectRatioMode() const { return mAspectRatioMode; }
  Qt::TransformationMode transformationMode() const { return mTransformationMode; }
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  
  // setters;
  void setPixmap(const QPixmap &pixmap);
  void setScaled(bool scaled, Qt::AspectRatioMode aspectRatioMode=Qt::KeepAspectRatio, Qt::TransformationMode transformationMode=Qt::SmoothTransformation);
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  
  QCPItemPosition * const topLeft;
  QCPItemPosition * const bottomRight;
  QCPItemAnchor * const top;
  QCPItemAnchor * const topRight;
  QCPItemAnchor * const right;
  QCPItemAnchor * const bottom;
  QCPItemAnchor * const bottomLeft;
  QCPItemAnchor * const left;
  
protected:
  enum AnchorIndex {aiTop, aiTopRight, aiRight, aiBottom, aiBottomLeft, aiLeft};
  
  // property members:
  QPixmap mPixmap;
  QPixmap mScaledPixmap;
  bool mScaled;
  bool mScaledPixmapInvalidated;
  Qt::AspectRatioMode mAspectRatioMode;
  Qt::TransformationMode mTransformationMode;
  QPen mPen, mSelectedPen;
  
  // reimplemented virtual methods:
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  virtual QPointF anchorPixelPosition(int anchorId) const Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  void updateScaledPixmap(QRect finalRect=QRect(), bool flipHorz=false, bool flipVert=false);
  QRect getFinalRect(bool *flippedHorz=nullptr, bool *flippedVert=nullptr) const;
  QPen mainPen() const;
};

#endif // QCP_ITEM_PIXMAP_H
