/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include <QColor>
#include <QRect>

#include <vector>
#include <map>

class QCPColumnAnnotations : public QCPAbstractPlottable
{
    Q_OBJECT

private:
    struct Row
    {
        // cppcheck-suppress passedByValue
        Row(std::vector<size_t> indices, bool selected,
            const ColumnAnnotation* columnAnnotation) :
            _indices(std::move(indices)), _selected(selected),
            _columnAnnotation(columnAnnotation)
        {}

        std::vector<size_t> _indices;
        bool _selected = true;
        const ColumnAnnotation* _columnAnnotation;
    };

    std::map<size_t, Row> _rows;

    double _cellWidth = 0.0;
    double _cellHeight = 0.0;
    double _halfCellWidth = 0.0;

public:
    explicit QCPColumnAnnotations(QCPAxis *keyAxis, QCPAxis *valueAxis);

    double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details = nullptr) const override;
    QCPRange getKeyRange(bool& foundRange, QCP::SignDomain inSignDomain = QCP::sdBoth) const override;
    QCPRange getValueRange(bool& foundRange, QCP::SignDomain inSignDomain = QCP::sdBoth,
        const QCPRange& inKeyRange = QCPRange()) const override;

    void setData(size_t y, std::vector<size_t> indices, bool selected,
        const ColumnAnnotation* columnAnnotation);

protected:
    void draw(QCPPainter* painter) override;
    void drawLegendIcon(QCPPainter* painter, const QRectF &rect) const override;

private:
    void renderRect(QCPPainter* painter, size_t x, size_t y,
        size_t w, const QString& value, bool selected,
        std::map<QString, int> valueWidths);
};

#endif // QCPCOLUMNANNOTATIONS_H
