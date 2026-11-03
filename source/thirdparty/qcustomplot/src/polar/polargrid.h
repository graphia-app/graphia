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

#ifndef QCP_POLARGRID_H
#define QCP_POLARGRID_H

#include "../global.h"
#include "radialaxis.h"

class QCPPolarAxisAngular;

class QCP_LIB_DECL QCPPolarGrid :public QCPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  
  /// \endcond
public:
  /*!
    TODO
  */
  enum GridType { gtAngular = 0x01 ///< 
                  ,gtRadial = 0x02 ///< 
                  ,gtAll    = 0xFF ///< 
                  ,gtNone   = 0x00 ///< 
                };
  Q_DECLARE_FLAGS(GridTypes, GridType)
  Q_FLAG(GridTypes)
  
  explicit QCPPolarGrid(QCPPolarAxisAngular *parentAxis);
  
  // getters:
  QCPPolarAxisRadial *radialAxis() const { return mRadialAxis.data(); }
  GridTypes type() const { return mType; }
  GridTypes subGridType() const { return mSubGridType; }
  bool antialiasedSubGrid() const { return mAntialiasedSubGrid; }
  bool antialiasedZeroLine() const { return mAntialiasedZeroLine; }
  QPen angularPen() const { return mAngularPen; }
  QPen angularSubGridPen() const { return mAngularSubGridPen; }
  QPen radialPen() const { return mRadialPen; }
  QPen radialSubGridPen() const { return mRadialSubGridPen; }
  QPen radialZeroLinePen() const { return mRadialZeroLinePen; }
  
  // setters:
  void setRadialAxis(QCPPolarAxisRadial *axis);
  void setType(GridTypes type);
  void setSubGridType(GridTypes type);
  void setAntialiasedSubGrid(bool enabled);
  void setAntialiasedZeroLine(bool enabled);
  void setAngularPen(const QPen &pen);
  void setAngularSubGridPen(const QPen &pen);
  void setRadialPen(const QPen &pen);
  void setRadialSubGridPen(const QPen &pen);
  void setRadialZeroLinePen(const QPen &pen);
  
protected:
  // property members:
  GridTypes mType;
  GridTypes mSubGridType;
  bool mAntialiasedSubGrid, mAntialiasedZeroLine;
  QPen mAngularPen, mAngularSubGridPen;
  QPen mRadialPen, mRadialSubGridPen, mRadialZeroLinePen;
  
  // non-property members:
  QCPPolarAxisAngular *mParentAxis;
  QPointer<QCPPolarAxisRadial> mRadialAxis;
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  void drawRadialGrid(QCPPainter *painter, const QPointF &center, const QVector<double> &coords, const QPen &pen, const QPen &zeroPen=Qt::NoPen);
  void drawAngularGrid(QCPPainter *painter, const QPointF &center, double radius, const QVector<QPointF> &ticksCosSin, const QPen &pen);
  
private:
  Q_DISABLE_COPY(QCPPolarGrid)
  
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCPPolarGrid::GridTypes)
Q_DECLARE_METATYPE(QCPPolarGrid::GridType)


#endif // QCP_POLARGRID_H
