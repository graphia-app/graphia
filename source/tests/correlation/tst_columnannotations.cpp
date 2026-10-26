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

#include "plugins/correlation/columnannotation.h"
#include "plugins/correlation/ui/qml/Graphia/Plugins/Correlation/qcpcolumnannotations.h"

#include <qcustomplot.h>

#include <QTest>
#include <QColor>
#include <QString>
#include <QStringList>

using namespace Qt::Literals::StringLiterals;

class TestColumnAnnotations : public QObject
{
    Q_OBJECT

private slots:
    void emptyValuesAreTransparent_data()
    {
        QTest::addColumn<QStringList>("values");

        QTest::newRow("strings") << QStringList{u"x"_s, u""_s, u"y"_s};
        QTest::newRow("numbers") << QStringList{u"1"_s, u""_s, u"3"_s};
    }

    void emptyValuesAreTransparent()
    {
        QFETCH(const QStringList, values);

        const QCustomPlot plot;
        auto* annotations = new QCPColumnAnnotations(plot.xAxis, plot.yAxis);

        const ColumnAnnotation annotation(u"Annotation"_s, {values.begin(), values.end()});
        annotations->setData(0, {0, 1, 2}, true, 0, &annotation);
        annotations->resolveRects();

        const auto* empty = annotations->rectAt(1, annotation);
        QVERIFY(empty != nullptr);
        QCOMPARE(empty->_value, QString());
        QCOMPARE(empty->_color, QColor(Qt::transparent));

        for(const size_t x : {size_t{0}, size_t{2}})
        {
            const auto* rect = annotations->rectAt(x, annotation);
            QVERIFY(rect != nullptr);
            QCOMPARE(rect->_color.alpha(), 255);
        }
    }
};

QTEST_MAIN(TestColumnAnnotations)
#include "tst_columnannotations.moc"
