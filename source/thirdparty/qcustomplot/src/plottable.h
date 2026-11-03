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

#ifndef QCP_PLOTTABLE_H
#define QCP_PLOTTABLE_H

#include "global.h"
#include "vector2d.h"
#include "axis/range.h"
#include "axis/axis.h"
#include "layer.h"
#include "selection.h"
#include "scatterstyle.h"

class QCPPainter;
class QCPAbstractPlottable;
class QCPPlottableInterface1D;
class QCPLegend;

class QCP_LIB_DECL QCPSelectionDecorator
{
  Q_GADGET
public:
  QCPSelectionDecorator();
  virtual ~QCPSelectionDecorator();
  
  // getters:
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  QCPScatterStyle scatterStyle() const { return mScatterStyle; }
  QCPScatterStyle::ScatterProperties usedScatterProperties() const { return mUsedScatterProperties; }
  
  // setters:
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setScatterStyle(const QCPScatterStyle &scatterStyle, QCPScatterStyle::ScatterProperties usedProperties=QCPScatterStyle::spPen);
  void setUsedScatterProperties(const QCPScatterStyle::ScatterProperties &properties);
  
  // non-virtual methods:
  void applyPen(QCPPainter *painter) const;
  void applyBrush(QCPPainter *painter) const;
  QCPScatterStyle getFinalScatterStyle(const QCPScatterStyle &unselectedStyle) const;
  
  // introduced virtual methods:
  virtual void copyFrom(const QCPSelectionDecorator *other);
  virtual void drawDecoration(QCPPainter *painter, QCPDataSelection selection);
  
protected:
  // property members:
  QPen mPen;
  QBrush mBrush;
  QCPScatterStyle mScatterStyle;
  QCPScatterStyle::ScatterProperties mUsedScatterProperties;
  // non-property members:
  QCPAbstractPlottable *mPlottable;
  
  // introduced virtual methods:
  virtual bool registerWithPlottable(QCPAbstractPlottable *plottable);
  
private:
  Q_DISABLE_COPY(QCPSelectionDecorator)
  friend class QCPAbstractPlottable;
};
Q_DECLARE_METATYPE(QCPSelectionDecorator*)


class QCP_LIB_DECL QCPAbstractPlottable : public QCPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(bool antialiasedFill READ antialiasedFill WRITE setAntialiasedFill)
  Q_PROPERTY(bool antialiasedScatters READ antialiasedScatters WRITE setAntialiasedScatters)
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
  Q_PROPERTY(QCPAxis* keyAxis READ keyAxis WRITE setKeyAxis)
  Q_PROPERTY(QCPAxis* valueAxis READ valueAxis WRITE setValueAxis)
  Q_PROPERTY(QCP::SelectionType selectable READ selectable WRITE setSelectable NOTIFY selectableChanged)
  Q_PROPERTY(QCPDataSelection selection READ selection WRITE setSelection NOTIFY selectionChanged)
  Q_PROPERTY(QCPSelectionDecorator* selectionDecorator READ selectionDecorator WRITE setSelectionDecorator)
  /// \endcond
public:
  QCPAbstractPlottable(QCPAxis *keyAxis, QCPAxis *valueAxis);
  virtual ~QCPAbstractPlottable() Q_DECL_OVERRIDE;
  
  // getters:
  QString name() const { return mName; }
  bool antialiasedFill() const { return mAntialiasedFill; }
  bool antialiasedScatters() const { return mAntialiasedScatters; }
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  QCPAxis *keyAxis() const { return mKeyAxis.data(); }
  QCPAxis *valueAxis() const { return mValueAxis.data(); }
  QCP::SelectionType selectable() const { return mSelectable; }
  bool selected() const { return !mSelection.isEmpty(); }
  QCPDataSelection selection() const { return mSelection; }
  QCPSelectionDecorator *selectionDecorator() const { return mSelectionDecorator; }
  
  // setters:
  void setName(const QString &name);
  void setAntialiasedFill(bool enabled);
  void setAntialiasedScatters(bool enabled);
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setKeyAxis(QCPAxis *axis);
  void setValueAxis(QCPAxis *axis);
  Q_SLOT void setSelectable(QCP::SelectionType selectable);
  Q_SLOT void setSelection(QCPDataSelection selection);
  void setSelectionDecorator(QCPSelectionDecorator *decorator);

  // introduced virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE = 0; // actually introduced in QCPLayerable as non-pure, but we want to force reimplementation for plottables
  virtual QCPPlottableInterface1D *interface1D() { return nullptr; }
  virtual QCPRange getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth) const = 0;
  virtual QCPRange getValueRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth, const QCPRange &inKeyRange=QCPRange()) const = 0;
  
  // non-property methods:
  void coordsToPixels(double key, double value, double &x, double &y) const;
  const QPointF coordsToPixels(double key, double value) const;
  void pixelsToCoords(double x, double y, double &key, double &value) const;
  void pixelsToCoords(const QPointF &pixelPos, double &key, double &value) const;
  void rescaleAxes(bool onlyEnlarge=false) const;
  void rescaleKeyAxis(bool onlyEnlarge=false) const;
  void rescaleValueAxis(bool onlyEnlarge=false, bool inKeyRange=false) const;
  bool addToLegend(QCPLegend *legend);
  bool addToLegend();
  bool removeFromLegend(QCPLegend *legend) const;
  bool removeFromLegend() const;
  
signals:
  void selectionChanged(bool selected);
  void selectionChanged(const QCPDataSelection &selection);
  void selectableChanged(QCP::SelectionType selectable);
  
protected:
  // property members:
  QString mName;
  bool mAntialiasedFill, mAntialiasedScatters;
  QPen mPen;
  QBrush mBrush;
  QPointer<QCPAxis> mKeyAxis, mValueAxis;
  QCP::SelectionType mSelectable;
  QCPDataSelection mSelection;
  QCPSelectionDecorator *mSelectionDecorator;
  
  // reimplemented virtual methods:
  virtual QRect clipRect() const Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE = 0;
  virtual QCP::Interaction selectionCategory() const Q_DECL_OVERRIDE;
  void applyDefaultAntialiasingHint(QCPPainter *painter) const Q_DECL_OVERRIDE;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) Q_DECL_OVERRIDE;
  virtual void deselectEvent(bool *selectionStateChanged) Q_DECL_OVERRIDE;
  
  // introduced virtual methods:
  virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const = 0;
  
  // non-virtual methods:
  void applyFillAntialiasingHint(QCPPainter *painter) const;
  void applyScattersAntialiasingHint(QCPPainter *painter) const;

private:
  Q_DISABLE_COPY(QCPAbstractPlottable)
  
  friend class QCustomPlot;
  friend class QCPAxis;
  friend class QCPPlottableLegendItem;
};


#endif // QCP_PLOTTABLE_H
