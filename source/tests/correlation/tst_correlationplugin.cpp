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

#include "tests/common/documenttest.h"
#include "tests/common/testenvironment.h"

#include <QAbstractTableModel>
#include <QMetaObject>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QUrl>
#include <QVariantMap>

#include <algorithm>
#include <iterator>
#include <limits>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// r2 is r1 doubled, so they correlate perfectly, r3 is r1 reversed, r4 is like r1, and r5 like r3;
// the second row annotates the columns
const QByteArray Csv =
    "Name,Group,c1,c2,c3,c4,c5,c6\n"
    "Kind,,x,x,x,y,y,y\n"
    "r1,g1,1,2,3,4,5,6\n"
    "r2,g1,2,4,6,8,10,12\n"
    "r3,g2,6,5,4,3,2,1\n"
    "r4,g2,1,3,2,5,4,6\n"
    "r5,g2,6,4,5,2,3,1\n";
} // namespace

// The correlation plugin, used through a Document, as in the UI
class TestCorrelationPlugin : public DocumentTest
{
    Q_OBJECT

    using DocumentTest::open;

    bool open(const QVariantMap& parameters = {})
    {
        return open(write(u"data.csv"_s, Csv), u"CorrelationCSV"_s, u"Correlation"_s, parameters);
    }

    QObject* plugin() { return _document->pluginInstance(); }

    QAbstractTableModel* tableModel()
    {
        return plugin()->property("nodeAttributeTableModel").value<QAbstractTableModel*>();
    }

    // The edges of the transformed graph as "source-target", sorted
    QStringList edgeNames()
    {
        QStringList names;
        for(auto edgeId : graph().edgeIds())
        {
            const auto& edge = graph().edgeById(edgeId);
            names.append(u"%1-%2"_s.arg(graphModel()->nodeName(edge.sourceId()),
                graphModel()->nodeName(edge.targetId())));
        }

        names.sort();
        return names;
    }

    double nodeValue(const QString& attributeName, const QString& nodeName)
    {
        return graphModel()->attributeValueByName(attributeName).numericValueOf(nodeId(nodeName));
    }

    double edgeValue(const QString& attributeName, const QString& edgeName)
    {
        const auto names = edgeName.split(u'-');
        for(auto edgeId : graph().edgeIds())
        {
            const auto& edge = graph().edgeById(edgeId);
            if(graphModel()->nodeName(edge.sourceId()) == names.at(0) &&
                graphModel()->nodeName(edge.targetId()) == names.at(1))
            {
                return graphModel()->attributeValueByName(attributeName).numericValueOf(edgeId);
            }
        }

        return std::numeric_limits<double>::quiet_NaN();
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        QVERIFY(createApplication());
    }

    void cleanup()
    {
        closeDocument();
    }

    void cleanupTestCase()
    {
        destroyApplication();
    }

    void load()
    {
        QVERIFY(open());

        QCOMPARE(nodeNames(), (QStringList{u"r1"_s, u"r2"_s, u"r3"_s, u"r4"_s, u"r5"_s}));

        // r1 and r4, and r3 and r5, correlate at 0.886, above the default threshold of 0.85
        QCOMPARE(edgeNames(), (QStringList{u"r1-r2"_s, u"r1-r4"_s, u"r2-r4"_s, u"r3-r5"_s}));
        QCOMPARE(edgeValue(u"PearsonCorrelationValue"_s, u"r1-r2"_s), 1.0);
        QCOMPARE(edgeValue(u"PearsonCorrelationValue"_s, u"r1-r4"_s), 31.0 / 35.0);

        QCOMPARE(_document->transforms(), (QStringList
        {
            u"RemoveEdges where $PearsonCorrelationValue < 0.85"_s,
            u"[pinned] RemoveComponents where $ComponentSize <= 1"_s
        }));

        QCOMPARE(plugin()->property("numContinuousColumns").toInt(), 6);
        QCOMPARE(plugin()->property("numDiscreteColumns").toInt(), 0);
        QCOMPARE(plugin()->property("columnAnnotationNames").toStringList(), QStringList{u"Kind"_s});

        // Each row's statistics
        QCOMPARE(nodeValue(u"MeanDataValue"_s, u"r2"_s), 7.0);
        QCOMPARE(nodeValue(u"MaximumDataValue"_s, u"r3"_s), 6.0);
        QCOMPARE(nodeValue(u"MinimumDataValue"_s, u"r4"_s), 1.0);
        QCOMPARE(graphModel()->attributeValueByName(u"Group"_s).stringValueOf(nodeId(u"r3"_s)), u"g2"_s);

        // The table in the plugin's panel has a column for each data column
        auto* model = tableModel();
        QCOMPARE(model->rowCount(), 5);
        const auto columnNames = model->property("columnNames").toStringList();
        const auto c3 = std::distance(columnNames.begin(), std::ranges::find_if(columnNames,
            [](const auto& name) { return name.startsWith(u"DataValue"_s) && name.endsWith(u"c3"_s); }));
        QVERIFY(c3 < columnNames.size());
        QVERIFY(QTest::qWaitFor([&] { return model->columnCount() == columnNames.size(); }, Timeout));
        QCOMPARE(model->data(model->index(1, static_cast<int>(c3))).toDouble(), 6.0);
    }

    void parameters_data()
    {
        QTest::addColumn<QVariantMap>("parameters");
        QTest::addColumn<QStringList>("edges");

        QTest::newRow("Higher minimum") << QVariantMap{{u"minimumThreshold"_s, 0.9}, {u"initialThreshold"_s, 0.9}} <<
            QStringList{u"r1-r2"_s};

        // Negative correlations too: r3 and r5 are the reverses of r1 and r4
        QTest::newRow("Both polarities") <<
            QVariantMap{{u"correlationPolarity"_s, u"Both"_s}} <<
            QStringList{u"r1-r2"_s, u"r1-r3"_s, u"r1-r4"_s, u"r1-r5"_s, u"r2-r3"_s, u"r2-r4"_s, u"r2-r5"_s,
                u"r3-r4"_s, u"r3-r5"_s, u"r4-r5"_s};

        QTest::newRow("Spearman") << QVariantMap{{u"continuousCorrelationType"_s, u"SpearmanRank"_s}} <<
            QStringList{u"r1-r2"_s, u"r1-r4"_s, u"r2-r4"_s, u"r3-r5"_s};

        // Each row's most correlated other row; r1 and r2 are equally like r4, and the earlier is kept
        QTest::newRow("k-NN") <<
            QVariantMap{{u"correlationFilterType"_s, u"Knn"_s}, {u"maximumK"_s, 1}, {u"initialK"_s, 1}} <<
            QStringList{u"r1-r2"_s, u"r3-r5"_s, u"r4-r1"_s};
    }

    void parameters()
    {
        QFETCH(const QVariantMap, parameters);
        QFETCH(const QStringList, edges);

        QVERIFY(open(parameters));
        QCOMPARE(edgeNames(), edges);
    }

    // Everything the plugin loaded is saved with the Document
    void saveAndLoad()
    {
        QVERIFY(open());

        const auto edges = edgeNames();
        const auto transforms = _document->transforms();

        auto url = QUrl::fromLocalFile(_dir.filePath(u"saved.%1"_s.arg(Application::nativeExtension())));
        QVERIFY(execute([&] { _document->saveFile(url, Application::name(), {}, {}); }));
        QVERIFY(open(url, QString::fromLatin1(Application::NativeFileType), {}));

        QCOMPARE(edgeNames(), edges);
        QCOMPARE(_document->transforms(), transforms);
        QCOMPARE(edgeValue(u"PearsonCorrelationValue"_s, u"r1-r4"_s), 31.0 / 35.0);
        QCOMPARE(nodeValue(u"MeanDataValue"_s, u"r2"_s), 7.0);
        QCOMPARE(plugin()->property("numContinuousColumns").toInt(), 6);
        QCOMPARE(plugin()->property("columnAnnotationNames").toStringList(), QStringList{u"Kind"_s});
        QCOMPARE(tableModel()->rowCount(), 5);
    }

    void hierarchicalClustering()
    {
        QVERIFY(open());

        QSignalSpy complete(plugin(), SIGNAL(hierarchicalClusteringComplete()));
        QVERIFY(execute([this] { QMetaObject::invokeMethod(plugin(), "computeHierarchicalClustering"); }));
        QVERIFY(QTest::qWaitFor([&] { return !complete.isEmpty(); }, Timeout));
    }

    // The rows most like a set of columns, as when finding the rows of interest in the plot
    void rowsOfInterest()
    {
        QVERIFY(open());

        std::vector<int> rows;
        QVERIFY(QMetaObject::invokeMethod(plugin(), "rowsOfInterestByColumns",
            Q_RETURN_ARG(std::vector<int>, rows),
            Q_ARG(std::vector<int>, (std::vector<int>{4, 5})), Q_ARG(std::vector<int>, (std::vector<int>{0, 1, 2, 3, 4})),
            Q_ARG(int, 40), Q_ARG(double, 0.0)));

        // Scored by the sum of the columns' values, less the sum of the rest's, r2 scores 2, and r1 1
        QCOMPARE(rows, (std::vector<int>{1, 0}));
    }
};

QTEST_MAIN(TestCorrelationPlugin)
#include "tst_correlationplugin.moc"
