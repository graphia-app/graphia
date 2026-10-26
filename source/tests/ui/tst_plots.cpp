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

#include "tests/common/testenvironment.h"

#include "app/ui/qml/Graphia/Controls/visualisationmappingplot.h"
#include "app/ui/qml/Graphia/enrichmentheatmap.h"
#include "app/ui/qml/Graphia/enrichmenttablemodel.h"
#include "app/ui/visualisations/visualisationmapping.h"

#include "shared/ui/qml/Graphia/SharedTypes/graphsizeestimateplot.h"
#include "shared/utils/statistics.h"
#include "shared/utils/string.h"

#include <qcustomplot.h>

#include <QApplication>
#include <QCoreApplication>
#include <QHoverEvent>
#include <QList>
#include <QMouseEvent>
#include <QPointF>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QVariantMap>
#include <QVector>

#include <algorithm>
#include <cmath>
#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
const QVector<double> Keys{1.0, 2.0, 3.0, 4.0, 5.0};
const QVector<double> NumNodes{10.0, 20.0, 40.0, 80.0, 160.0};
const QVector<double> NumEdges{100.0, 200.0, 400.0, 800.0, 999.0};
const QVector<double> NumUniqueEdges{50.0, 100.0, 200.0, 400.0, 500.0};

QVariantMap graphSizeEstimate(bool descending = false)
{
    auto keys = Keys;
    auto numNodes = NumNodes;
    auto numEdges = NumEdges;
    auto numUniqueEdges = NumUniqueEdges;

    // k-NN estimates are of decreasing k
    if(descending)
    {
        std::ranges::reverse(keys);
        std::ranges::reverse(numNodes);
        std::ranges::reverse(numEdges);
        std::ranges::reverse(numUniqueEdges);
    }

    return {{u"keys"_s, QVariant::fromValue(keys)}, {u"numNodes"_s, QVariant::fromValue(numNodes)},
        {u"numEdges"_s, QVariant::fromValue(numEdges)}, {u"numUniqueEdges"_s, QVariant::fromValue(numUniqueEdges)}};
}

void sendMouseEvent(QQuickItem* item, QEvent::Type type, const QPointF& position,
    Qt::MouseButtons buttons = Qt::LeftButton)
{
    QMouseEvent event(type, position, position, type == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton,
        buttons, Qt::NoModifier);
    QCoreApplication::sendEvent(item, &event);
}
} // namespace

// The plots drawn with QCustomPlot in QML: the graph size estimate when opening
// a matrix or correlation data, the curve of a visualisation's mapping, and the
// heat map of enrichment results
class TestPlots : public QObject
{
    Q_OBJECT

    // Creates a plot, and finds the QCustomPlot it draws with, a widget of its own
    template<typename Plot>
    static std::unique_ptr<Plot> createPlot(QCustomPlot*& customPlot)
    {
        const auto existingWidgets = QApplication::allWidgets();
        auto plot = std::make_unique<Plot>();

        customPlot = nullptr;
        const auto widgets = QApplication::allWidgets();
        for(auto* widget : widgets)
        {
            if(!existingWidgets.contains(widget) && qobject_cast<QCustomPlot*>(widget) != nullptr)
                customPlot = qobject_cast<QCustomPlot*>(widget);
        }

        plot->setSize({400.0, 300.0});
        return plot;
    }

    static QStringList tickLabels(QCPAxis* axis)
    {
        auto ticker = axis->ticker().dynamicCast<QCPAxisTickerText>();
        return ticker != nullptr ? ticker->ticks().values() : QStringList{};
    }

    static EnrichmentTableModel::Row row(const QString& a, const QString& b, double adjustedP)
    {
        return {a, b, 1.0, 1.0, 1.0, adjustedP, adjustedP};
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void graphSizeEstimate_data()
    {
        QTest::addColumn<bool>("descending");

        QTest::newRow("Threshold") << false;
        QTest::newRow("k-NN") << true;
    }

    // The estimated size of the graph at the chosen threshold
    void graphSizeEstimate()
    {
        QFETCH(const bool, descending);

        QCustomPlot* customPlot = nullptr;
        auto plot = createPlot<GraphSizeEstimatePlot>(customPlot);
        QVERIFY(customPlot != nullptr);

        plot->setProperty("graphSizeEstimate", ::graphSizeEstimate(descending));
        QCOMPARE(customPlot->graphCount(), 2);
        QCOMPARE(customPlot->graph(0)->name(), u"Nodes"_s);
        QCOMPARE(customPlot->graph(1)->name(), u"Edges"_s);
        QCOMPARE(customPlot->xAxis->rangeReversed(), descending);
        QCOMPARE(customPlot->yAxis->scaleType(), QCPAxis::stLogarithmic);

        // The estimate is that of the nearest key
        const QSignalSpy thresholdChanged(plot.get(), SIGNAL(thresholdChanged()));
        plot->setProperty("threshold", 3.3);
        QCOMPARE(thresholdChanged.count(), 1);
        QCOMPARE(customPlot->xAxis->label(), u"Estimated Graph Size: 40 Nodes, 400 Edges"_s);

        plot->setProperty("threshold", 4.6);
        QCOMPARE(customPlot->xAxis->label(), u"Estimated Graph Size: 160 Nodes, 999 Edges"_s);

        plot->setProperty("uniqueEdgesOnly", true);
        QCOMPARE(customPlot->xAxis->label(), u"Estimated Graph Size: 160 Nodes, 500 Edges"_s);
        QCOMPARE(customPlot->graph(1)->data()->size(), 5);

        double maxEdges = 0.0;
        for(const auto& data : *customPlot->graph(1)->data())
            maxEdges = std::max(maxEdges, data.value);
        QCOMPARE(maxEdges, 500.0);

        // Beyond the keys, the nearest is the last
        plot->setProperty("threshold", 10.0);
        QCOMPARE(customPlot->xAxis->label(), u"Estimated Graph Size: 160 Nodes, 500 Edges"_s);

        // k is a whole number
        plot->setProperty("integralThreshold", true);
        plot->setProperty("threshold", 2.4);
        QCOMPARE(plot->property("threshold").toDouble(), 2.0);
        QCOMPARE(customPlot->xAxis->label(), u"Estimated Graph Size: 20 Nodes, 100 Edges"_s);
    }

    // Clicking or dragging on the plot chooses the threshold
    void graphSizeEstimateDragging()
    {
        QCustomPlot* customPlot = nullptr;
        auto plot = createPlot<GraphSizeEstimatePlot>(customPlot);
        QVERIFY(customPlot != nullptr);

        plot->setProperty("graphSizeEstimate", ::graphSizeEstimate());
        customPlot->replot();

        auto pixelFor = [&](double x) { return QPointF(customPlot->xAxis->coordToPixel(x), 150.0); };

        sendMouseEvent(plot.get(), QEvent::MouseButtonPress, pixelFor(2.0));
        QVERIFY(std::abs(plot->property("threshold").toDouble() - 2.0) < 0.05);

        sendMouseEvent(plot.get(), QEvent::MouseMove, pixelFor(4.0));
        QVERIFY(std::abs(plot->property("threshold").toDouble() - 4.0) < 0.05);

        sendMouseEvent(plot.get(), QEvent::MouseButtonRelease, pixelFor(4.0), Qt::NoButton);
        sendMouseEvent(plot.get(), QEvent::MouseMove, pixelFor(1.0), Qt::NoButton);
        QVERIFY(std::abs(plot->property("threshold").toDouble() - 4.0) < 0.05);
    }

    void mappingCurve_data()
    {
        QTest::addColumn<double>("exponent");
        QTest::addColumn<bool>("invert");

        for(const double exponent : {1.0, 0.25, 4.0})
        {
            QTest::addRow("exponent %g", exponent) << exponent << false;
            QTest::addRow("exponent %g, inverted", exponent) << exponent << true;
        }
    }

    // The curve shows, for each value of the visualisation along the x axis, the attribute
    // value the visualisation maps to it, along the y axis
    void mappingCurve()
    {
        QFETCH(const double, exponent);
        QFETCH(const bool, invert);

        QCustomPlot* customPlot = nullptr;
        auto plot = createPlot<VisualisationMappingPlot>(customPlot);
        QVERIFY(customPlot != nullptr);

        const QList<double> values{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
        plot->setProperty("values", QVariant::fromValue(values));
        plot->setProperty("exponent", exponent);
        plot->setProperty("invert", invert);
        plot->setProperty("maximum", 8.0);
        plot->setProperty("minimum", 2.0);

        const VisualisationMapping mapping(u::findStatisticsFor(values),
            u"{\"min\": 2, \"max\": 8, \"exponent\": %1}"_s.arg(exponent));

        // The curve is the first graph, then the range's limits, then the values
        const auto* curveGraph = customPlot->graph(0);
        QVERIFY(curveGraph != nullptr);
        const auto& curve = *curveGraph->data();
        QVERIFY(curve.size() > 100);

        for(auto point = curve.constBegin(); point != curve.constEnd(); ++point)
        {
            auto value = point->value;
            if(invert)
                value = mapping.min() + mapping.max() - value;

            QVERIFY2(std::abs(mapping.map(value) - point->key) < 1e-6,
                qPrintable(u"at %1, %2 is mapped to %3"_s.arg(point->key).arg(point->value).arg(mapping.map(value))));
        }
    }

    // The range the values are mapped from
    void mappingRange()
    {
        QCustomPlot* customPlot = nullptr;
        auto plot = createPlot<VisualisationMappingPlot>(customPlot);
        QVERIFY(customPlot != nullptr);

        const QList<double> values{1.0, 2.0, 3.0, 4.0, 5.0};
        plot->setProperty("values", QVariant::fromValue(values));

        QVERIFY(QMetaObject::invokeMethod(plot.get(), "setRangeToMinMax"));
        QCOMPARE(plot->property("minimum").toDouble(), 1.0);
        QCOMPARE(plot->property("maximum").toDouble(), 5.0);

        QVERIFY(QMetaObject::invokeMethod(plot.get(), "setRangeToStddev"));
        const auto statistics = u::findStatisticsFor(values);
        QCOMPARE(plot->property("minimum").toDouble(), statistics._mean - statistics._stddev);
        QCOMPARE(plot->property("maximum").toDouble(), statistics._mean + statistics._stddev);

        // The range is within the values
        plot->setProperty("minimum", -10.0);
        QCOMPARE(plot->property("minimum").toDouble(), 1.0);
        plot->setProperty("maximum", 10.0);
        QCOMPARE(plot->property("maximum").toDouble(), 5.0);

        // The minimum stays below the maximum, and the maximum above the minimum
        plot->setProperty("maximum", 4.0);
        plot->setProperty("minimum", 6.0);
        QVERIFY(plot->property("minimum").toDouble() < 4.0);
        QVERIFY(plot->property("minimum").toDouble() > 3.99);
        plot->setProperty("minimum", 3.0);
        plot->setProperty("maximum", 2.0);
        QVERIFY(plot->property("maximum").toDouble() > 3.0);
        QVERIFY(plot->property("maximum").toDouble() < 3.01);

        // The lines showing the range are dragged to change it
        plot->setProperty("minimum", 2.0);
        plot->setProperty("maximum", 4.0);
        customPlot->replot();

        // Each change rebuilds the plot, so the axis is looked up afresh
        auto pixelFor = [&](double y) { return QPointF(200.0, customPlot->graph(0)->valueAxis()->coordToPixel(y)); };

        const QSignalSpy manualChange(plot.get(), SIGNAL(manualChangeToMinMax()));
        sendMouseEvent(plot.get(), QEvent::MouseButtonPress, pixelFor(4.0));
        sendMouseEvent(plot.get(), QEvent::MouseMove, pixelFor(4.5));
        sendMouseEvent(plot.get(), QEvent::MouseButtonRelease, pixelFor(4.5), Qt::NoButton);

        QVERIFY(std::abs(plot->property("maximum").toDouble() - 4.5) < 0.05);
        QCOMPARE(plot->property("minimum").toDouble(), 2.0);
        QVERIFY(!manualChange.isEmpty());
    }

    // The adjusted p-values of the pairs of values of the two attributes
    void enrichmentHeatmap()
    {
        // All the results, not only those that are enriched
        EnrichmentTableModel tableModel;
        tableModel.setProperty("enrichedOnly", false);
        tableModel.setTableData(
        {
            row(u"A1"_s, u"B1"_s, 0.001), row(u"A1"_s, u"B2"_s, 0.5),
            row(u"A10"_s, u"B1"_s, 0.02), row(u"A10"_s, u"B2"_s, 0.04),
            row(u"A2"_s, u"B1"_s, 1.0), row(u"A2"_s, u"B2"_s, 0.03)
        }, u"A"_s, u"B"_s);

        QCustomPlot* customPlot = nullptr;
        auto plot = createPlot<EnrichmentHeatmap>(customPlot);
        QVERIFY(customPlot != nullptr);

        plot->setProperty("model", QVariant::fromValue(&tableModel));
        customPlot->replot();

        // The first attribute's values along the x axis, and the second's down the y axis, in natural order
        QCOMPARE(tickLabels(customPlot->xAxis), (QStringList{u"A1"_s, u"A2"_s, u"A10"_s}));
        QCOMPARE(tickLabels(customPlot->yAxis2), (QStringList{u"B2"_s, u"B1"_s}));

        QCPColorMap* colorMap = nullptr;
        for(int i = 0; i < customPlot->plottableCount(); i++)
        {
            if(auto* map = qobject_cast<QCPColorMap*>(customPlot->plottable(i)); map != nullptr)
                colorMap = map;
        }

        QVERIFY(colorMap != nullptr);

        // The cells are offset by a margin of one cell
        auto cell = [colorMap](int x, int y) { return colorMap->data()->cell(x + 1, y + 1); };
        QCOMPARE(cell(0, 1), 0.001);   // A1, B1
        QCOMPARE(cell(0, 0), 0.5);     // A1, B2
        QCOMPARE(cell(2, 1), 0.02);    // A10, B1
        QCOMPARE(cell(1, 0), 0.03);    // A2, B2

        auto pixelFor = [&](double x, double y)
            { return QPointF(customPlot->xAxis->coordToPixel(x), customPlot->yAxis2->coordToPixel(y)); };

        // Clicking a cell gives its row in the table
        const QSignalSpy clicked(plot.get(), SIGNAL(plotValueClicked(int)));
        sendMouseEvent(plot.get(), QEvent::MouseButtonPress, pixelFor(2.0, 1.0));
        sendMouseEvent(plot.get(), QEvent::MouseButtonRelease, pixelFor(2.0, 1.0), Qt::NoButton);
        QCOMPARE(clicked.count(), 1);

        const auto clickedRow = clicked.at(0).at(0).toInt();
        QCOMPARE(tableModel.data(clickedRow, EnrichmentTableModel::Results::SelectionA).toString(), u"A10"_s);
        QCOMPARE(tableModel.data(clickedRow, EnrichmentTableModel::Results::SelectionB).toString(), u"B1"_s);

        // Hovering over a cell shows its p-value
        QCPItemText* hoverLabel = nullptr;
        for(int i = 0; i < customPlot->itemCount(); i++)
        {
            if(auto* text = qobject_cast<QCPItemText*>(customPlot->item(i)); text != nullptr)
                hoverLabel = text;
        }

        QVERIFY(hoverLabel != nullptr);
        QVERIFY(!hoverLabel->visible());

        const auto position = pixelFor(0.0, 1.0);
        QHoverEvent hover(QEvent::HoverMove, position, position, position);
        QCoreApplication::sendEvent(plot.get(), &hover);
        QVERIFY(hoverLabel->visible());
        QCOMPARE(hoverLabel->text(), u"Adj. P-value: %1"_s.arg(u::formatNumberScientific(0.001)));

        QHoverEvent leave(QEvent::HoverLeave, {}, {}, position);
        QCoreApplication::sendEvent(plot.get(), &leave);
        QVERIFY(!hoverLabel->visible());
    }
};

QTEST_MAIN(TestPlots)
#include "tst_plots.moc"
