#include "qcpcolumnannotations.h"

#include "shared/utils/color.h"

QCPColumnAnnotations::QCPColumnAnnotations(QCPAxis* keyAxis, QCPAxis* valueAxis) :
    QCPAbstractPlottable(keyAxis, valueAxis)
{}

double QCPColumnAnnotations::selectTest(const QPointF&, bool, QVariant*) const
{
    // We don't care about selection
    return -1.0;
}

QCPRange QCPColumnAnnotations::getKeyRange(bool& foundRange, QCP::SignDomain) const
{
    foundRange = true;
    return mKeyAxis->range();
}

QCPRange QCPColumnAnnotations::getValueRange(bool& foundRange, QCP::SignDomain, const QCPRange&) const
{
    foundRange = true;
    return mValueAxis->range();
}

void QCPColumnAnnotations::setData(size_t y, std::vector<size_t> indices,
    bool selected, const ColumnAnnotation* columnAnnotation)
{
    _rows.emplace(y, Row(std::move(indices), selected, columnAnnotation));
}

void QCPColumnAnnotations::renderRect(QCPPainter* painter, size_t x, size_t y, size_t w, QColor color)
{
    const auto cellWidth = mKeyAxis->coordToPixel(1.0) -  mKeyAxis->coordToPixel(0.0);
    const auto cellHeight = mValueAxis->coordToPixel(0.0) - mValueAxis->coordToPixel(1.0);
    const auto halfCellWidth = cellWidth * 0.5;

    auto xPixel = mKeyAxis->coordToPixel(x);
    auto yPixel = mValueAxis->coordToPixel(y);

    QRect rect(xPixel - halfCellWidth, yPixel - cellHeight, (w * cellWidth), cellHeight);

    painter->setPen(color);
    painter->setBrush(color);
    painter->drawRect(rect);
}

static QColor colorForValue(const QString& value, bool selected)
{
    QColor color = u::colorForString(value);

    if(value.isEmpty())
        color = Qt::transparent;
    else if(!selected)
        color = QColor::fromHsl(color.hue(), 20, std::max(color.lightness(), 150));

    return color;
}

void QCPColumnAnnotations::draw(QCPPainter* painter)
{
    for(const auto& [y, row] : _rows)
    {
        if(row._indices.empty())
            continue;

        size_t left = 0;
        size_t right = 0;
        size_t width = 0;

        auto currentValue = row._columnAnnotation->valueAt(row._indices.at(0));

        for(auto index : row._indices)
        {
            const auto& value = row._columnAnnotation->valueAt(index);

            if(value != currentValue)
            {
                renderRect(painter, left, y, width,
                    colorForValue(currentValue, row._selected));

                left = right;
                currentValue = value;
            }

            right++;
            width = right - left;
        }

        renderRect(painter, left, y, width,
            colorForValue(currentValue, row._selected));
    }
}

void QCPColumnAnnotations::drawLegendIcon(QCPPainter*, const QRectF&) const
{
    // Don't need a legend
}
