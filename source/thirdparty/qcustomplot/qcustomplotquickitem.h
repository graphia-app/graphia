/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef QCUSTOMPLOTQUICKITEM_H
#define QCUSTOMPLOTQUICKITEM_H

#include <qcustomplot.h>
#include <qcustomplotcolorprovider.h>

#include <QQuickPaintedItem>

class QMouseEvent;

class QCP_LIB_DECL QCustomPlotQuickItem : public QQuickPaintedItem, protected QCustomPlotColorProvider
{
    Q_OBJECT
public:
    explicit QCustomPlotQuickItem(int multisamples = 4,
        QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

private:
    QCustomPlot _customPlot;

    bool event(QEvent* event) override;

protected:
    QCustomPlot& customPlot() { return _customPlot; }
    const QCustomPlot& customPlot() const { return _customPlot; }

    void updatePlotSize();
    void routeMouseEvent(QMouseEvent* event);

    virtual void buildPlot() = 0;

private slots:
    void onReplot();
};

#endif // QCUSTOMPLOTQUICKITEM_H
