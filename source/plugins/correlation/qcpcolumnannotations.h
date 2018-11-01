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
