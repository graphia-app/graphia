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

#ifndef QCPCOLUMNANNOTATIONS_H
#define QCPCOLUMNANNOTATIONS_H

#include <qcustomplot.h>

#include "columnannotation.h"

#include "shared/ui/visualisations/colorgradient.h"
#include "shared/ui/visualisations/colorpalette.h"

#include <QColor>
#include <QRect>
#include <QString>

#include <vector>
#include <map>

class QCPColumnAnnotations : public QCPAbstractPlottable
{
    Q_OBJECT

private:
    struct Row
    {
        std::vector<size_t> _indices;
        bool _selected = true;
        size_t _offset = 0;
        const ColumnAnnotation* _columnAnnotation;
    };

    std::map<size_t, Row> _rows;

    struct Rect
    {
        size_t _x = 0;
        size_t _y = 0;
        size_t _width = 0;
        QString _value;
        QColor _color;
        bool _selected = true;
    };

    std::map<QString, std::vector<Rect>> _rects;

    std::map<QString, int> _valueWidths;

    double _cellWidth = 0.0;
    double _cellHeight = 0.0;
    double _halfCellWidth = 0.0;

    ColorGradient _colorGradient;
    ColorPalette _colorPalette;

public:
    explicit QCPColumnAnnotations(QCPAxis *keyAxis, QCPAxis *valueAxis);

    double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details = nullptr) const override;
    QCPRange getKeyRange(bool& foundRange, QCP::SignDomain inSignDomain = QCP::sdBoth) const override;
    QCPRange getValueRange(bool& foundRange, QCP::SignDomain inSignDomain = QCP::sdBoth,
        const QCPRange& inKeyRange = QCPRange()) const override;

    void setData(size_t y, std::vector<size_t> indices, bool selected, size_t offset,
        const ColumnAnnotation* columnAnnotation);
    void resolveRects();
    const Rect* rectAt(size_t x, const ColumnAnnotation& annotation) const;

protected:
    void draw(QCPPainter* painter) override;
    void drawLegendIcon(QCPPainter* painter, const QRectF &rect) const override;

private:
    int widthForValue(const QCPPainter* painter, const QString& value);
    void renderRect(QCPPainter* painter, const Rect& r);
};

#endif // QCPCOLUMNANNOTATIONS_H
