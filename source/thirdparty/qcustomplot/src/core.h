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

#ifndef QCP_CORE_H
#define QCP_CORE_H

#include "global.h"
#include "axis/range.h"
#include "axis/axis.h"
#include "paintbuffer.h"
#include "plottable.h"

class QCPPainter;
class QCPLayer;
class QCPAbstractItem;
class QCPGraph;
class QCPLegend;
class QCPAbstractLegendItem;
class QCPSelectionRect;

class QCP_LIB_DECL QCustomPlot : public QWidget
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QRect viewport READ viewport WRITE setViewport)
  Q_PROPERTY(QPixmap background READ background WRITE setBackground)
  Q_PROPERTY(bool backgroundScaled READ backgroundScaled WRITE setBackgroundScaled)
  Q_PROPERTY(Qt::AspectRatioMode backgroundScaledMode READ backgroundScaledMode WRITE setBackgroundScaledMode)
  Q_PROPERTY(QCPLayoutGrid* plotLayout READ plotLayout)
  Q_PROPERTY(bool autoAddPlottableToLegend READ autoAddPlottableToLegend WRITE setAutoAddPlottableToLegend)
  Q_PROPERTY(int selectionTolerance READ selectionTolerance WRITE setSelectionTolerance)
  Q_PROPERTY(bool noAntialiasingOnDrag READ noAntialiasingOnDrag WRITE setNoAntialiasingOnDrag)
  Q_PROPERTY(Qt::KeyboardModifier multiSelectModifier READ multiSelectModifier WRITE setMultiSelectModifier)
  Q_PROPERTY(bool openGl READ openGl WRITE setOpenGl)
  /// \endcond
public:
  /*!
    Defines how a layer should be inserted relative to an other layer.

    \see addLayer, moveLayer
  */
  enum LayerInsertMode { limBelow  ///< Layer is inserted below other layer
                         ,limAbove ///< Layer is inserted above other layer
                       };
  Q_ENUM(LayerInsertMode)
  
  /*!
    Defines with what timing the QCustomPlot surface is refreshed after a replot.

    \see replot
  */
  enum RefreshPriority { rpImmediateRefresh ///< Replots immediately and repaints the widget immediately by calling QWidget::repaint() after the replot
                         ,rpQueuedRefresh   ///< Replots immediately, but queues the widget repaint, by calling QWidget::update() after the replot. This way multiple redundant widget repaints can be avoided.
                         ,rpRefreshHint     ///< Whether to use immediate or queued refresh depends on whether the plotting hint \ref QCP::phImmediateRefresh is set, see \ref setPlottingHints.
                         ,rpQueuedReplot    ///< Queues the entire replot for the next event loop iteration. This way multiple redundant replots can be avoided. The actual replot is then done with \ref rpRefreshHint priority.
                       };
  Q_ENUM(RefreshPriority)
  
  explicit QCustomPlot(QWidget *parent = nullptr);
  virtual ~QCustomPlot() Q_DECL_OVERRIDE;
  
  // getters:
  QRect viewport() const { return mViewport; }
  double bufferDevicePixelRatio() const { return mBufferDevicePixelRatio; }
  QPixmap background() const { return mBackgroundPixmap; }
  bool backgroundScaled() const { return mBackgroundScaled; }
  Qt::AspectRatioMode backgroundScaledMode() const { return mBackgroundScaledMode; }
  QCPLayoutGrid *plotLayout() const { return mPlotLayout; }
  QCP::AntialiasedElements antialiasedElements() const { return mAntialiasedElements; }
  QCP::AntialiasedElements notAntialiasedElements() const { return mNotAntialiasedElements; }
  bool autoAddPlottableToLegend() const { return mAutoAddPlottableToLegend; }
  const QCP::Interactions interactions() const { return mInteractions; }
  int selectionTolerance() const { return mSelectionTolerance; }
  bool noAntialiasingOnDrag() const { return mNoAntialiasingOnDrag; }
  QCP::PlottingHints plottingHints() const { return mPlottingHints; }
  Qt::KeyboardModifier multiSelectModifier() const { return mMultiSelectModifier; }
  QCP::SelectionRectMode selectionRectMode() const { return mSelectionRectMode; }
  QCPSelectionRect *selectionRect() const { return mSelectionRect; }
  bool openGl() const { return mOpenGl; }
  
  // setters:
  void setViewport(const QRect &rect);
  void setBufferDevicePixelRatio(double ratio);
  void setBackground(const QPixmap &pm);
  void setBackground(const QPixmap &pm, bool scaled, Qt::AspectRatioMode mode=Qt::KeepAspectRatioByExpanding);
  void setBackground(const QBrush &brush);
  void setBackgroundScaled(bool scaled);
  void setBackgroundScaledMode(Qt::AspectRatioMode mode);
  void setAntialiasedElements(const QCP::AntialiasedElements &antialiasedElements);
  void setAntialiasedElement(QCP::AntialiasedElement antialiasedElement, bool enabled=true);
  void setNotAntialiasedElements(const QCP::AntialiasedElements &notAntialiasedElements);
  void setNotAntialiasedElement(QCP::AntialiasedElement notAntialiasedElement, bool enabled=true);
  void setAutoAddPlottableToLegend(bool on);
  void setInteractions(const QCP::Interactions &interactions);
  void setInteraction(const QCP::Interaction &interaction, bool enabled=true);
  void setSelectionTolerance(int pixels);
  void setNoAntialiasingOnDrag(bool enabled);
  void setPlottingHints(const QCP::PlottingHints &hints);
  void setPlottingHint(QCP::PlottingHint hint, bool enabled=true);
  void setMultiSelectModifier(Qt::KeyboardModifier modifier);
  void setSelectionRectMode(QCP::SelectionRectMode mode);
  void setSelectionRect(QCPSelectionRect *selectionRect);
  void setOpenGl(bool enabled, int multisampling=16, QSurface *surface=0);
  
  // non-property methods:
  // plottable interface:
  QCPAbstractPlottable *plottable(int index);
  QCPAbstractPlottable *plottable();
  bool removePlottable(QCPAbstractPlottable *plottable);
  bool removePlottable(int index);
  int clearPlottables();
  int plottableCount() const;
  QList<QCPAbstractPlottable*> selectedPlottables() const;
  template<class PlottableType>
  PlottableType *plottableAt(const QPointF &pos, bool onlySelectable=false, int *dataIndex=nullptr) const;
  QCPAbstractPlottable *plottableAt(const QPointF &pos, bool onlySelectable=false, int *dataIndex=nullptr) const;
  bool hasPlottable(QCPAbstractPlottable *plottable) const;
 
  // specialized interface for QCPGraph:
  QCPGraph *graph(int index) const;
  QCPGraph *graph() const;
  QCPGraph *addGraph(QCPAxis *keyAxis=nullptr, QCPAxis *valueAxis=nullptr);
  bool removeGraph(QCPGraph *graph);
  bool removeGraph(int index);
  int clearGraphs();
  int graphCount() const;
  QList<QCPGraph*> selectedGraphs() const;

  // item interface:
  QCPAbstractItem *item(int index) const;
  QCPAbstractItem *item() const;
  bool removeItem(QCPAbstractItem *item);
  bool removeItem(int index);
  int clearItems();
  int itemCount() const;
  QList<QCPAbstractItem*> selectedItems() const;
  template<class ItemType>
  ItemType *itemAt(const QPointF &pos, bool onlySelectable=false) const;
  QCPAbstractItem *itemAt(const QPointF &pos, bool onlySelectable=false) const;
  bool hasItem(QCPAbstractItem *item) const;
  
  // layer interface:
  QCPLayer *layer(const QString &name) const;
  QCPLayer *layer(int index) const;
  QCPLayer *currentLayer() const;
  bool setCurrentLayer(const QString &name);
  bool setCurrentLayer(QCPLayer *layer);
  int layerCount() const;
  bool addLayer(const QString &name, QCPLayer *otherLayer=nullptr, LayerInsertMode insertMode=limAbove);
  bool removeLayer(QCPLayer *layer);
  bool moveLayer(QCPLayer *layer, QCPLayer *otherLayer, LayerInsertMode insertMode=limAbove);
  
  // axis rect/layout interface:
  int axisRectCount() const;
  QCPAxisRect* axisRect(int index=0) const;
  QList<QCPAxisRect*> axisRects() const;
  QCPLayoutElement* layoutElementAt(const QPointF &pos) const;
  QCPAxisRect* axisRectAt(const QPointF &pos) const;
  Q_SLOT void rescaleAxes(bool onlyVisiblePlottables=false);
  
  QList<QCPAxis*> selectedAxes() const;
  QList<QCPLegend*> selectedLegends() const;
  Q_SLOT void deselectAll();
  
  bool savePdf(const QString &fileName, int width=0, int height=0, QCP::ExportPen exportPen=QCP::epAllowCosmetic, const QString &pdfCreator=QString(), const QString &pdfTitle=QString());
  bool savePng(const QString &fileName, int width=0, int height=0, double scale=1.0, int quality=-1, int resolution=96, QCP::ResolutionUnit resolutionUnit=QCP::ruDotsPerInch);
  bool saveJpg(const QString &fileName, int width=0, int height=0, double scale=1.0, int quality=-1, int resolution=96, QCP::ResolutionUnit resolutionUnit=QCP::ruDotsPerInch);
  bool saveBmp(const QString &fileName, int width=0, int height=0, double scale=1.0, int resolution=96, QCP::ResolutionUnit resolutionUnit=QCP::ruDotsPerInch);
  bool saveRastered(const QString &fileName, int width, int height, double scale, const char *format, int quality=-1, int resolution=96, QCP::ResolutionUnit resolutionUnit=QCP::ruDotsPerInch);
  QPixmap toPixmap(int width=0, int height=0, double scale=1.0);
  void toPainter(QCPPainter *painter, int width=0, int height=0);
  Q_SLOT void replot(QCustomPlot::RefreshPriority refreshPriority=QCustomPlot::rpRefreshHint);
  double replotTime(bool average=false) const;
  
  QCPAxis *xAxis, *yAxis, *xAxis2, *yAxis2;
  QCPLegend *legend;
  
signals:
  void mouseDoubleClick(QMouseEvent *event);
  void mousePress(QMouseEvent *event);
  void mouseMove(QMouseEvent *event);
  void mouseRelease(QMouseEvent *event);
  void mouseWheel(QWheelEvent *event);
  
  void plottableClick(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);
  void plottableDoubleClick(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);
  void itemClick(QCPAbstractItem *item, QMouseEvent *event);
  void itemDoubleClick(QCPAbstractItem *item, QMouseEvent *event);
  void axisClick(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event);
  void axisDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event);
  void legendClick(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event);
  void legendDoubleClick(QCPLegend *legend,  QCPAbstractLegendItem *item, QMouseEvent *event);
  
  void selectionChangedByUser();
  void beforeReplot();
  void afterLayout();
  void afterReplot();
  
protected:
  // property members:
  QRect mViewport;
  double mBufferDevicePixelRatio;
  QCPLayoutGrid *mPlotLayout;
  bool mAutoAddPlottableToLegend;
  QList<QCPAbstractPlottable*> mPlottables;
  QList<QCPGraph*> mGraphs; // extra list of plottables also in mPlottables that are of type QCPGraph
  QList<QCPAbstractItem*> mItems;
  QList<QCPLayer*> mLayers;
  QCP::AntialiasedElements mAntialiasedElements, mNotAntialiasedElements;
  QCP::Interactions mInteractions;
  int mSelectionTolerance;
  bool mNoAntialiasingOnDrag;
  QBrush mBackgroundBrush;
  QPixmap mBackgroundPixmap;
  QPixmap mScaledBackgroundPixmap;
  bool mBackgroundScaled;
  Qt::AspectRatioMode mBackgroundScaledMode;
  QCPLayer *mCurrentLayer;
  QCP::PlottingHints mPlottingHints;
  Qt::KeyboardModifier mMultiSelectModifier;
  QCP::SelectionRectMode mSelectionRectMode;
  QCPSelectionRect *mSelectionRect;
  bool mOpenGl;
  
  // non-property members:
  QList<QSharedPointer<QCPAbstractPaintBuffer> > mPaintBuffers;
  QPoint mMousePressPos;
  bool mMouseHasMoved;
  QPointer<QCPLayerable> mMouseEventLayerable;
  QPointer<QCPLayerable> mMouseSignalLayerable;
  QVariant mMouseEventLayerableDetails;
  QVariant mMouseSignalLayerableDetails;
  bool mReplotting;
  bool mReplotQueued;
  double mReplotTime, mReplotTimeAverage;
  int mOpenGlMultisamples;
  QCP::AntialiasedElements mOpenGlAntialiasedElementsBackup;
  bool mOpenGlCacheLabelsBackup;
#ifdef QCP_OPENGL_FBO
  QSharedPointer<QOpenGLContext> mGlContext;
  QSharedPointer<QSurface> mGlSurface;
  QSharedPointer<QOpenGLPaintDevice> mGlPaintDevice;
#endif
  
  // reimplemented virtual methods:
  virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE;
  virtual QSize sizeHint() const Q_DECL_OVERRIDE;
  virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
  virtual void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
  
  // introduced virtual methods:
  virtual void draw(QCPPainter *painter);
  virtual void updateLayout();
  virtual void axisRemoved(QCPAxis *axis);
  virtual void legendRemoved(QCPLegend *legend);
  Q_SLOT virtual void processRectSelection(QRect rect, QMouseEvent *event);
  Q_SLOT virtual void processRectZoom(QRect rect, QMouseEvent *event);
  Q_SLOT virtual void processPointSelection(QMouseEvent *event);
  
  // non-virtual methods:
  bool registerPlottable(QCPAbstractPlottable *plottable);
  bool registerGraph(QCPGraph *graph);
  bool registerItem(QCPAbstractItem* item);
  void updateLayerIndices() const;
  QCPLayerable *layerableAt(const QPointF &pos, bool onlySelectable, QVariant *selectionDetails=nullptr) const;
  QList<QCPLayerable*> layerableListAt(const QPointF &pos, bool onlySelectable, QList<QVariant> *selectionDetails=nullptr) const;
  void drawBackground(QCPPainter *painter);
  void setupPaintBuffers();
  QCPAbstractPaintBuffer *createPaintBuffer();
  bool hasInvalidatedPaintBuffers();
  bool setupOpenGl(QSurface *surface);
  void freeOpenGl();
  
  friend class QCPLegend;
  friend class QCPAxis;
  friend class QCPLayer;
  friend class QCPAxisRect;
  friend class QCPAbstractPlottable;
  friend class QCPGraph;
  friend class QCPAbstractItem;
};
Q_DECLARE_METATYPE(QCustomPlot::LayerInsertMode)
Q_DECLARE_METATYPE(QCustomPlot::RefreshPriority)


// implementation of template functions:

/*!
  Returns the plottable at the pixel position \a pos. The plottable type (a QCPAbstractPlottable
  subclass) that shall be taken into consideration can be specified via the template parameter.

  Plottables that only consist of single lines (like graphs) have a tolerance band around them, see
  \ref setSelectionTolerance. If multiple plottables come into consideration, the one closest to \a
  pos is returned.
  
  If \a onlySelectable is true, only plottables that are selectable
  (QCPAbstractPlottable::setSelectable) are considered.
  
  if \a dataIndex is non-null, it is set to the index of the plottable's data point that is closest
  to \a pos.

  If there is no plottable of the specified type at \a pos, returns \c nullptr.
  
  \see itemAt, layoutElementAt
*/
template<class PlottableType>
PlottableType *QCustomPlot::plottableAt(const QPointF &pos, bool onlySelectable, int *dataIndex) const
{
  PlottableType *resultPlottable = 0;
  QVariant resultDetails;
  double resultDistance = mSelectionTolerance; // only regard clicks with distances smaller than mSelectionTolerance as selections, so initialize with that value
  
  foreach (QCPAbstractPlottable *plottable, mPlottables)
  {
    PlottableType *currentPlottable = qobject_cast<PlottableType*>(plottable);
    if (!currentPlottable || (onlySelectable && !currentPlottable->selectable())) // we could have also passed onlySelectable to the selectTest function, but checking here is faster, because we have access to QCPAbstractPlottable::selectable
      continue;
    if (currentPlottable->clipRect().contains(pos.toPoint())) // only consider clicks where the plottable is actually visible
    {
      QVariant details;
      double currentDistance = currentPlottable->selectTest(pos, false, dataIndex ? &details : nullptr);
      if (currentDistance >= 0 && currentDistance < resultDistance)
      {
        resultPlottable = currentPlottable;
        resultDetails = details;
        resultDistance = currentDistance;
      }
    }
  }
  
  if (resultPlottable && dataIndex)
  {
    QCPDataSelection sel = resultDetails.value<QCPDataSelection>();
    if (!sel.isEmpty())
      *dataIndex = sel.dataRange(0).begin();
  }
  return resultPlottable;
}

/*!
  Returns the item at the pixel position \a pos. The item type (a QCPAbstractItem subclass) that shall be
  taken into consideration can be specified via the template parameter. Items that only consist of single
  lines (e.g. \ref QCPItemLine or \ref QCPItemCurve) have a tolerance band around them, see \ref
  setSelectionTolerance. If multiple items come into consideration, the one closest to \a pos is returned.
  
  If \a onlySelectable is true, only items that are selectable (QCPAbstractItem::setSelectable) are
  considered.
  
  If there is no item at \a pos, returns \c nullptr.
  
  \see plottableAt, layoutElementAt
*/
template<class ItemType>
ItemType *QCustomPlot::itemAt(const QPointF &pos, bool onlySelectable) const
{
  ItemType *resultItem = 0;
  double resultDistance = mSelectionTolerance; // only regard clicks with distances smaller than mSelectionTolerance as selections, so initialize with that value
  
  foreach (QCPAbstractItem *item, mItems)
  {
    ItemType *currentItem = qobject_cast<ItemType*>(item);
    if (!currentItem || (onlySelectable && !currentItem->selectable())) // we could have also passed onlySelectable to the selectTest function, but checking here is faster, because we have access to QCPAbstractItem::selectable
      continue;
    if (!currentItem->clipToAxisRect() || currentItem->clipRect().contains(pos.toPoint())) // only consider clicks inside axis cliprect of the item if actually clipped to it
    {
      double currentDistance = currentItem->selectTest(pos, false);
      if (currentDistance >= 0 && currentDistance < resultDistance)
      {
        resultItem = currentItem;
        resultDistance = currentDistance;
      }
    }
  }
  
  return resultItem;
}



#endif // QCP_CORE_H
