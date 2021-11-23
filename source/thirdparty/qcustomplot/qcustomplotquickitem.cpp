/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qcustomplotquickitem.h"

#include <QMouseEvent>
#include <QCoreApplication>

QCustomPlotQuickItem::QCustomPlotQuickItem(int multisamples, QQuickItem* parent) :
    QQuickPaintedItem(parent)
{
#ifdef Q_OS_MACOS
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#warning Check if crash in QOpenGLContext::surface/QWindow::surfaceHandle still occurs on macOS
#endif
    Q_UNUSED(multisamples);
#else
    _customPlot.setOpenGl(true, multisamples);
#endif

    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &QQuickPaintedItem::widthChanged, this, &QCustomPlotQuickItem::updatePlotSize);
    connect(this, &QQuickPaintedItem::heightChanged, this, &QCustomPlotQuickItem::updatePlotSize);
    connect(&_customPlot, &QCustomPlot::afterReplot, this, &QCustomPlotQuickItem::onReplot);
    connect(this, &QQuickPaintedItem::enabledChanged, [this] { update(); });
}

void QCustomPlotQuickItem::paint(QPainter* painter)
{
    if(!isEnabled())
    {
        // Create a desaturated version of the pixmap
        auto image = _customPlot.toPixmap().toImage();
        const auto bytes = image.depth() >> 3;

        for(int y = 0; y < image.height(); y++)
        {
            auto* scanLine = image.scanLine(y);
            for(int x = 0; x < image.width(); x++)
            {
                auto* pixel = reinterpret_cast<QRgb*>(scanLine + (x * bytes));
                const int gray = qGray(*pixel);
                const int alpha = qAlpha(*pixel);
                *pixel = QColor(gray, gray, gray, alpha).rgba();
            }
        }

        // The pixmap that QCustomPlot creates is a mixture of premultipled
        // pixels and pixels with an alpha value, so to keep things simple
        // we just use an alpha value in the destination buffer instead
        painter->setCompositionMode(QPainter::CompositionMode_DestinationOver);

        auto backgroundColor = QColor(Qt::white);
        backgroundColor.setAlpha(127);

        painter->fillRect(0, 0, width(), height(), backgroundColor);
        painter->drawPixmap(0, 0, QPixmap::fromImage(image));
    }
    else
        painter->drawPixmap(0, 0, _customPlot.toPixmap());
}

void QCustomPlotQuickItem::updatePlotSize()
{
    // QML does some spurious resizing, which can result in odd
    // sizes that things get upset with
    if(width() <= 0.0 || height() <= 0.0)
        return;

    _customPlot.setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));

    // Since QCustomPlot is a QWidget, it is never technically visible, so never generates
    // a resizeEvent, so its viewport never gets set, so we must do so manually
    _customPlot.setViewport(_customPlot.geometry());
}

void QCustomPlotQuickItem::onReplot()
{
    update();
}

void QCustomPlotQuickItem::routeMouseEvent(QMouseEvent* event)
{
    auto* newEvent = new QMouseEvent(event->type(), event->localPos(),
        event->button(), event->buttons(), event->modifiers());
    QCoreApplication::postEvent(&customPlot(), newEvent);
}
