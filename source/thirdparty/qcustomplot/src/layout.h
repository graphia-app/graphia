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

#ifndef QCP_LAYOUT_H
#define QCP_LAYOUT_H

#include "global.h"
#include "layer.h"

class QCPLayout;
class QCPLayoutElement;
class QCustomPlot;

class QCP_LIB_DECL QCPMarginGroup : public QObject
{
  Q_OBJECT
public:
  explicit QCPMarginGroup(QCustomPlot *parentPlot);
  virtual ~QCPMarginGroup();
  
  // non-virtual methods:
  QList<QCPLayoutElement*> elements(QCP::MarginSide side) const { return mChildren.value(side); }
  bool isEmpty() const;
  void clear();
  
protected:
  // non-property members:
  QCustomPlot *mParentPlot;
  QHash<QCP::MarginSide, QList<QCPLayoutElement*> > mChildren;
  
  // introduced virtual methods:
  virtual int commonMargin(QCP::MarginSide side) const;
  
  // non-virtual methods:
  void addChild(QCP::MarginSide side, QCPLayoutElement *element);
  void removeChild(QCP::MarginSide side, QCPLayoutElement *element);
  
private:
  Q_DISABLE_COPY(QCPMarginGroup)
  
  friend class QCPLayoutElement;
};


class QCP_LIB_DECL QCPLayoutElement : public QCPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QCPLayout* layout READ layout)
  Q_PROPERTY(QRect rect READ rect)
  Q_PROPERTY(QRect outerRect READ outerRect WRITE setOuterRect)
  Q_PROPERTY(QMargins margins READ margins WRITE setMargins)
  Q_PROPERTY(QMargins minimumMargins READ minimumMargins WRITE setMinimumMargins)
  Q_PROPERTY(QSize minimumSize READ minimumSize WRITE setMinimumSize)
  Q_PROPERTY(QSize maximumSize READ maximumSize WRITE setMaximumSize)
  Q_PROPERTY(SizeConstraintRect sizeConstraintRect READ sizeConstraintRect WRITE setSizeConstraintRect)
  /// \endcond
public:
  /*!
    Defines the phases of the update process, that happens just before a replot. At each phase,
    \ref update is called with the according UpdatePhase value.
  */
  enum UpdatePhase { upPreparation ///< Phase used for any type of preparation that needs to be done before margin calculation and layout
                     ,upMargins    ///< Phase in which the margins are calculated and set
                     ,upLayout     ///< Final phase in which the layout system places the rects of the elements
                   };
  Q_ENUM(UpdatePhase)
  
  /*!
    Defines to which rect of a layout element the size constraints that can be set via \ref
    setMinimumSize and \ref setMaximumSize apply. The outer rect (\ref outerRect) includes the
    margins (e.g. in the case of a QCPAxisRect the axis labels), whereas the inner rect (\ref rect)
    does not.
    
    \see setSizeConstraintRect
  */
  enum SizeConstraintRect { scrInnerRect ///< Minimum/Maximum size constraints apply to inner rect
                            , scrOuterRect ///< Minimum/Maximum size constraints apply to outer rect, thus include layout element margins
                          };
  Q_ENUM(SizeConstraintRect)

  explicit QCPLayoutElement(QCustomPlot *parentPlot=nullptr);
  virtual ~QCPLayoutElement() Q_DECL_OVERRIDE;
  
  // getters:
  QCPLayout *layout() const { return mParentLayout; }
  QRect rect() const { return mRect; }
  QRect outerRect() const { return mOuterRect; }
  QMargins margins() const { return mMargins; }
  QMargins minimumMargins() const { return mMinimumMargins; }
  QCP::MarginSides autoMargins() const { return mAutoMargins; }
  QSize minimumSize() const { return mMinimumSize; }
  QSize maximumSize() const { return mMaximumSize; }
  SizeConstraintRect sizeConstraintRect() const { return mSizeConstraintRect; }
  QCPMarginGroup *marginGroup(QCP::MarginSide side) const { return mMarginGroups.value(side, nullptr); }
  QHash<QCP::MarginSide, QCPMarginGroup*> marginGroups() const { return mMarginGroups; }
  
  // setters:
  void setOuterRect(const QRect &rect);
  void setMargins(const QMargins &margins);
  void setMinimumMargins(const QMargins &margins);
  void setAutoMargins(QCP::MarginSides sides);
  void setMinimumSize(const QSize &size);
  void setMinimumSize(int width, int height);
  void setMaximumSize(const QSize &size);
  void setMaximumSize(int width, int height);
  void setSizeConstraintRect(SizeConstraintRect constraintRect);
  void setMarginGroup(QCP::MarginSides sides, QCPMarginGroup *group);
  
  // introduced virtual methods:
  virtual void update(UpdatePhase phase);
  virtual QSize minimumOuterSizeHint() const;
  virtual QSize maximumOuterSizeHint() const;
  virtual QList<QCPLayoutElement*> elements(bool recursive) const;
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  
protected:
  // property members:
  QCPLayout *mParentLayout;
  QSize mMinimumSize, mMaximumSize;
  SizeConstraintRect mSizeConstraintRect;
  QRect mRect, mOuterRect;
  QMargins mMargins, mMinimumMargins;
  QCP::MarginSides mAutoMargins;
  QHash<QCP::MarginSide, QCPMarginGroup*> mMarginGroups;
  
  // introduced virtual methods:
  virtual int calculateAutoMargin(QCP::MarginSide side);
  virtual void layoutChanged();
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const Q_DECL_OVERRIDE { Q_UNUSED(painter) }
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE { Q_UNUSED(painter) }
  virtual void parentPlotInitialized(QCustomPlot *parentPlot) Q_DECL_OVERRIDE;

private:
  Q_DISABLE_COPY(QCPLayoutElement)
  
  friend class QCustomPlot;
  friend class QCPLayout;
  friend class QCPMarginGroup;
};
Q_DECLARE_METATYPE(QCPLayoutElement::UpdatePhase)


class QCP_LIB_DECL QCPLayout : public QCPLayoutElement
{
  Q_OBJECT
public:
  explicit QCPLayout();
  
  // reimplemented virtual methods:
  virtual void update(UpdatePhase phase) Q_DECL_OVERRIDE;
  virtual QList<QCPLayoutElement*> elements(bool recursive) const Q_DECL_OVERRIDE;
  
  // introduced virtual methods:
  virtual int elementCount() const = 0;
  virtual QCPLayoutElement* elementAt(int index) const = 0;
  virtual QCPLayoutElement* takeAt(int index) = 0;
  virtual bool take(QCPLayoutElement* element) = 0;
  virtual void simplify();
  
  // non-virtual methods:
  bool removeAt(int index);
  bool remove(QCPLayoutElement* element);
  void clear();
  
protected:
  // introduced virtual methods:
  virtual void updateLayout();
  
  // non-virtual methods:
  void sizeConstraintsChanged() const;
  void adoptElement(QCPLayoutElement *el);
  void releaseElement(QCPLayoutElement *el);
  QVector<int> getSectionSizes(QVector<int> maxSizes, QVector<int> minSizes, QVector<double> stretchFactors, int totalSize) const;
  static QSize getFinalMinimumOuterSize(const QCPLayoutElement *el);
  static QSize getFinalMaximumOuterSize(const QCPLayoutElement *el);
  
private:
  Q_DISABLE_COPY(QCPLayout)
  friend class QCPLayoutElement;
};


class QCP_LIB_DECL QCPLayoutGrid : public QCPLayout
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(int rowCount READ rowCount)
  Q_PROPERTY(int columnCount READ columnCount)
  Q_PROPERTY(QList<double> columnStretchFactors READ columnStretchFactors WRITE setColumnStretchFactors)
  Q_PROPERTY(QList<double> rowStretchFactors READ rowStretchFactors WRITE setRowStretchFactors)
  Q_PROPERTY(int columnSpacing READ columnSpacing WRITE setColumnSpacing)
  Q_PROPERTY(int rowSpacing READ rowSpacing WRITE setRowSpacing)
  Q_PROPERTY(FillOrder fillOrder READ fillOrder WRITE setFillOrder)
  Q_PROPERTY(int wrap READ wrap WRITE setWrap)
  /// \endcond
public:
  
  /*!
    Defines in which direction the grid is filled when using \ref addElement(QCPLayoutElement*).
    The column/row at which wrapping into the next row/column occurs can be specified with \ref
    setWrap.

    \see setFillOrder
  */
  enum FillOrder { foRowsFirst    ///< Rows are filled first, and a new element is wrapped to the next column if the row count would exceed \ref setWrap.
                  ,foColumnsFirst ///< Columns are filled first, and a new element is wrapped to the next row if the column count would exceed \ref setWrap.
                };
  Q_ENUM(FillOrder)
  
  explicit QCPLayoutGrid();
  virtual ~QCPLayoutGrid() Q_DECL_OVERRIDE;
  
  // getters:
  int rowCount() const { return mElements.size(); }
  int columnCount() const { return mElements.size() > 0 ? mElements.first().size() : 0; }
  QList<double> columnStretchFactors() const { return mColumnStretchFactors; }
  QList<double> rowStretchFactors() const { return mRowStretchFactors; }
  int columnSpacing() const { return mColumnSpacing; }
  int rowSpacing() const { return mRowSpacing; }
  int wrap() const { return mWrap; }
  FillOrder fillOrder() const { return mFillOrder; }
  
  // setters:
  void setColumnStretchFactor(int column, double factor);
  void setColumnStretchFactors(const QList<double> &factors);
  void setRowStretchFactor(int row, double factor);
  void setRowStretchFactors(const QList<double> &factors);
  void setColumnSpacing(int pixels);
  void setRowSpacing(int pixels);
  void setWrap(int count);
  void setFillOrder(FillOrder order, bool rearrange=true);
  
  // reimplemented virtual methods:
  virtual void updateLayout() Q_DECL_OVERRIDE;
  virtual int elementCount() const Q_DECL_OVERRIDE { return rowCount()*columnCount(); }
  virtual QCPLayoutElement* elementAt(int index) const Q_DECL_OVERRIDE;
  virtual QCPLayoutElement* takeAt(int index) Q_DECL_OVERRIDE;
  virtual bool take(QCPLayoutElement* element) Q_DECL_OVERRIDE;
  virtual QList<QCPLayoutElement*> elements(bool recursive) const Q_DECL_OVERRIDE;
  virtual void simplify() Q_DECL_OVERRIDE;
  virtual QSize minimumOuterSizeHint() const Q_DECL_OVERRIDE;
  virtual QSize maximumOuterSizeHint() const Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  QCPLayoutElement *element(int row, int column) const;
  bool addElement(int row, int column, QCPLayoutElement *element);
  bool addElement(QCPLayoutElement *element);
  bool hasElement(int row, int column);
  void expandTo(int newRowCount, int newColumnCount);
  void insertRow(int newIndex);
  void insertColumn(int newIndex);
  int rowColToIndex(int row, int column) const;
  void indexToRowCol(int index, int &row, int &column) const;
  
protected:
  // property members:
  QList<QList<QCPLayoutElement*> > mElements;
  QList<double> mColumnStretchFactors;
  QList<double> mRowStretchFactors;
  int mColumnSpacing, mRowSpacing;
  int mWrap;
  FillOrder mFillOrder;
  
  // non-virtual methods:
  void getMinimumRowColSizes(QVector<int> *minColWidths, QVector<int> *minRowHeights) const;
  void getMaximumRowColSizes(QVector<int> *maxColWidths, QVector<int> *maxRowHeights) const;
  
private:
  Q_DISABLE_COPY(QCPLayoutGrid)
};
Q_DECLARE_METATYPE(QCPLayoutGrid::FillOrder)


class QCP_LIB_DECL QCPLayoutInset : public QCPLayout
{
  Q_OBJECT
public:
  /*!
    Defines how the placement and sizing is handled for a certain element in a QCPLayoutInset.
  */
  enum InsetPlacement { ipFree            ///< The element may be positioned/sized arbitrarily, see \ref setInsetRect
                        ,ipBorderAligned  ///< The element is aligned to one of the layout sides, see \ref setInsetAlignment
                      };
  Q_ENUM(InsetPlacement)
  
  explicit QCPLayoutInset();
  virtual ~QCPLayoutInset() Q_DECL_OVERRIDE;
  
  // getters:
  InsetPlacement insetPlacement(int index) const;
  Qt::Alignment insetAlignment(int index) const;
  QRectF insetRect(int index) const;
  
  // setters:
  void setInsetPlacement(int index, InsetPlacement placement);
  void setInsetAlignment(int index, Qt::Alignment alignment);
  void setInsetRect(int index, const QRectF &rect);
  
  // reimplemented virtual methods:
  virtual void updateLayout() Q_DECL_OVERRIDE;
  virtual int elementCount() const Q_DECL_OVERRIDE;
  virtual QCPLayoutElement* elementAt(int index) const Q_DECL_OVERRIDE;
  virtual QCPLayoutElement* takeAt(int index) Q_DECL_OVERRIDE;
  virtual bool take(QCPLayoutElement* element) Q_DECL_OVERRIDE;
  virtual void simplify() Q_DECL_OVERRIDE {}
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  void addElement(QCPLayoutElement *element, Qt::Alignment alignment);
  void addElement(QCPLayoutElement *element, const QRectF &rect);
  
protected:
  // property members:
  QList<QCPLayoutElement*> mElements;
  QList<InsetPlacement> mInsetPlacement;
  QList<Qt::Alignment> mInsetAlignment;
  QList<QRectF> mInsetRect;
  
private:
  Q_DISABLE_COPY(QCPLayoutInset)
};
Q_DECLARE_METATYPE(QCPLayoutInset::InsetPlacement)

#endif // QCP_LAYOUT_H
