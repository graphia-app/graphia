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

#ifndef QCP_H
#define QCP_H

#include "global.h"
#include "vector2d.h"
#include "painter.h"
#include "paintbuffer.h"
#include "layer.h"
#include "axis/range.h"
#include "selection.h"
#include "selectionrect.h"
#include "layout.h"
#include "lineending.h"
#include "axis/axisticker.h"
#include "axis/axistickerdatetime.h"
#include "axis/axistickertime.h"
#include "axis/axistickerfixed.h"
#include "axis/axistickertext.h"
#include "axis/axistickerpi.h"
#include "axis/axistickerlog.h"
#include "axis/axis.h"
#include "scatterstyle.h"
#include "datacontainer.h"
#include "plottable.h"
#include "item.h"
#include "core.h"
#include "plottable1d.h"
#include "colorgradient.h"
#include "selectiondecorator-bracket.h"
#include "layoutelements/layoutelement-axisrect.h"
#include "layoutelements/layoutelement-legend.h"
#include "layoutelements/layoutelement-textelement.h"
#include "layoutelements/layoutelement-colorscale.h"
#include "plottables/plottable-graph.h"
#include "plottables/plottable-curve.h"
#include "plottables/plottable-bars.h"
#include "plottables/plottable-statisticalbox.h"
#include "plottables/plottable-colormap.h"
#include "plottables/plottable-financial.h"
#include "plottables/plottable-errorbar.h"
#include "items/item-straightline.h"
#include "items/item-line.h"
#include "items/item-curve.h"
#include "items/item-rect.h"
#include "items/item-text.h"
#include "items/item-ellipse.h"
#include "items/item-pixmap.h"
#include "items/item-tracer.h"
#include "items/item-bracket.h"
#include "polar/radialaxis.h"
#include "polar/layoutelement-angularaxis.h"
#include "polar/polargrid.h"
#include "polar/polargraph.h"

#endif // QCP_H
