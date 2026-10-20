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

#include "app/ui/qml/Graphia/availableattributesmodel.h"

#include "shared/loading/tabulardata.h"
#include "shared/utils/json.h"

#include "plugins/correlation/correlationtype.h"

#include <QAbstractTableModel>
#include <QByteArray>
#include <QFile>
#include <QList>
#include <QMetaObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QRect>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QUrl>
#include <QVariantMap>
#include <QVector>

#include <zlib.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <memory>
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

// The content of a file compressed with gzip, as native files are saved
QByteArray gunzip(QByteArray compressed)
{
    z_stream stream = {};
    if(inflateInit2(&stream, MAX_WBITS + 32) != Z_OK) // 32 means read the gzip header
        return {};

    stream.next_in = reinterpret_cast<Bytef*>(compressed.data());
    stream.avail_in = static_cast<uInt>(compressed.size());

    QByteArray content;
    std::vector<char> buffer(16384);
    int result = Z_OK;

    do
    {
        stream.next_out = reinterpret_cast<Bytef*>(buffer.data());
        stream.avail_out = static_cast<uInt>(buffer.size());

        result = inflate(&stream, Z_NO_FLUSH);
        if(result != Z_OK && result != Z_STREAM_END)
        {
            content.clear();
            break;
        }

        content.append(buffer.data(), static_cast<qsizetype>(buffer.size() - stream.avail_out));
    } while(result != Z_STREAM_END);

    inflateEnd(&stream);
    return content;
}

// Rewrites the plugin's data, as saved now, as an earlier version of the plugin saved it
void downgrade(json& data, int version)
{
    auto rename = [&data](const char* from, const char* to)
    {
        data[to] = data[from];
        data.erase(from);
    };

    if(version < 14)
    {
        rename("minimumThreshold", version >= 12 ? "threshold" : "minimumCorrelationValue");
        data.erase("maximumK");
    }

    if(version < 11)
        data.erase("hcOrdering");

    if(version < 10)
    {
        data.erase("clippingType");
        data.erase("clippingValue");
    }

    if(version < 7)
    {
        rename("numContinuousColumns", "numColumns");
        rename("continuousData", "data");
        rename("continuousCorrelationType", "correlationType");

        for(const auto* key : {"numDiscreteColumns", "discreteData", "correlationDataType", "discreteCorrelationType"})
            data.erase(key);
    }

    if(version < 3)
    {
        rename("correlationValues", "pearsonValues");
        data.erase("correlationType");
        data.erase("correlationPolarity");
    }

    if(version < 2)
    {
        // An array of values, indexed by edge id
        json values = json::array();
        for(const auto& element : data["pearsonValues"])
        {
            const size_t id = element["id"];
            while(values.size() <= id)
                values.push_back(0.0);

            values[id] = element["value"];
        }

        data["pearsonValues"] = values;
    }
}
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

    std::unique_ptr<QQmlEngine> _engine;

    // One of the types in the plugin's QML module, as the plugin's QML creates
    std::unique_ptr<QObject> createQmlObject(const QString& typeName)
    {
        QQmlComponent component(_engine.get());
        component.setData(u"import Graphia.Plugins.Correlation\n%1 {}"_s.arg(typeName).toUtf8(), {});

        std::unique_ptr<QObject> object(component.create());
        if(object == nullptr)
            qWarning() << component.errorString();

        return object;
    }

    // Parses a file with the parser the dialog for opening correlation files uses
    std::unique_ptr<QObject> parse(const QUrl& url, const QString& type = u"CorrelationCSV"_s)
    {
        auto parser = createQmlObject(u"CorrelationTabularDataParser"_s);
        if(parser == nullptr)
            return nullptr;

        QSignalSpy complete(parser.get(), SIGNAL(completeChanged()));
        QMetaObject::invokeMethod(parser.get(), "parse", Q_ARG(QUrl, url), Q_ARG(QString, type));

        if(!QTest::qWaitFor([&] { return !complete.isEmpty() && !parser->property("busy").toBool(); }, Timeout))
            return nullptr;

        return parser;
    }

    // Calls a method that finds the data rectangle, and waits for it
    template<typename... Args>
    static QVariantMap dataRect(QObject* parser, const char* method, Args... args)
    {
        const QSignalSpy dataRectChanged(parser, SIGNAL(dataRectChanged()));
        QMetaObject::invokeMethod(parser, method, args...);

        if(!QTest::qWaitFor([&] { return !dataRectChanged.isEmpty() && !parser->property("busy").toBool(); }, Timeout))
            return {};

        return parser->property("dataRect").toMap();
    }

    static QRect rect(const QVariantMap& dataRect) { return dataRect.value(u"asQRect"_s).toRect(); }

    static QVariantMap estimateGraphSize(QObject* parser, const QVariantMap& parameters)
    {
        QSignalSpy estimateChanged(parser, SIGNAL(graphSizeEstimateChanged()));
        QMetaObject::invokeMethod(parser, "estimateGraphSize", Q_ARG(QVariantMap, parameters));

        if(!QTest::qWaitFor([&] { return !estimateChanged.isEmpty(); }, Timeout))
            return {};

        return parser->property("graphSizeEstimate").toMap();
    }

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

    // The Document, saved, as JSON
    json saved()
    {
        auto url = QUrl::fromLocalFile(_dir.filePath(u"saved.%1"_s.arg(Application::nativeExtension())));
        if(!execute([&] { _document->saveFile(url, Application::name(), {}, {}); }))
            return {};

        QFile file(url.toLocalFile());
        if(!file.open(QIODevice::ReadOnly))
            return {};

        return parseJsonFrom(gunzip(file.readAll()));
    }

    bool openNative(const json& content)
    {
        auto url = write(u"edited.%1"_s.arg(Application::nativeExtension()),
            QByteArray::fromStdString(content.dump()));

        return open(url, QString::fromLatin1(Application::NativeFileType), {});
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        QVERIFY(createApplication());

        // The plugin's QML types are registered when it's loaded, by the Application
        _engine = std::make_unique<QQmlEngine>();
    }

    void cleanup()
    {
        closeDocument();
    }

    void cleanupTestCase()
    {
        _engine.reset();
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

        // r3 and r5 are the reverses of r1 and r4; the rest of the negative correlations are -0.886
        QTest::newRow("Negative") <<
            QVariantMap{{u"correlationPolarity"_s, u"Negative"_s},
                {u"minimumThreshold"_s, 0.7}, {u"initialThreshold"_s, 0.9}} <<
            QStringList{u"r1-r3"_s, u"r2-r3"_s, u"r4-r5"_s};

        QTest::newRow("Negative k-NN") <<
            QVariantMap{{u"correlationPolarity"_s, u"Negative"_s}, {u"correlationFilterType"_s, u"Knn"_s},
                {u"minimumThreshold"_s, 0.9}, {u"maximumK"_s, 2}, {u"initialK"_s, 2}} <<
            QStringList{u"r1-r3"_s, u"r2-r3"_s, u"r4-r5"_s};

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

    // Files saved by earlier versions of the plugin
    void earlierVersions_data()
    {
        QTest::addColumn<int>("version");

        for(int version = 1; version <= 15; version++)
            QTest::addRow("%d", version) << version;
    }

    void earlierVersions()
    {
        QFETCH(const int, version);

        QVERIFY(open());
        const auto edges = edgeNames();

        auto content = saved();
        QVERIFY(content.is_array() && content.size() == 2);
        QCOMPARE(content[0]["pluginDataVersion"].get<int>(), 15);

        content[0]["pluginDataVersion"] = version;
        downgrade(content[1]["pluginData"], version);
        QVERIFY(openNative(content));

        QCOMPARE(edgeNames(), edges);
        QCOMPARE(edgeValue(u"PearsonCorrelationValue"_s, u"r1-r4"_s), 31.0 / 35.0);
        QCOMPARE(nodeValue(u"MeanDataValue"_s, u"r2"_s), 7.0);
        QCOMPARE(plugin()->property("numContinuousColumns").toInt(), 6);
        QCOMPARE(tableModel()->rowCount(), 5);
    }

    void missingPluginData_data()
    {
        QTest::addColumn<QString>("key");

        for(const auto* key : {"numContinuousColumns", "numDiscreteColumns", "userColumnData", "dataColumnNames",
            "continuousData", "discreteData", "hcOrdering", "correlationValues", "minimumThreshold", "transpose",
            "clippingType", "correlationDataType", "correlationPolarity"})
        {
            QTest::newRow(key) << QString::fromLatin1(key);
        }
    }

    // Plugin data that's missing something fails to load
    void missingPluginData()
    {
        QFETCH(const QString, key);

        QVERIFY(open());

        auto content = saved();
        QVERIFY(content.is_array() && content.size() == 2);

        content[1]["pluginData"].erase(key.toStdString());
        QVERIFY(!openNative(content));
    }

    // Data that doesn't fill a whole number of rows fails to load
    void partialRow()
    {
        QVERIFY(open());

        auto content = saved();
        QVERIFY(content.is_array() && content.size() == 2);

        content[1]["pluginData"]["continuousData"].erase(0);
        QVERIFY(!openNative(content));
    }

    // As are files with no data
    void noRows()
    {
        QVERIFY(open());

        auto content = saved();
        QVERIFY(content.is_array() && content.size() == 2);

        content[1]["pluginData"]["continuousData"] = json::array();
        QVERIFY(!openNative(content));
    }

    // Correlation values for edges that aren't in the graph are ignored
    void unknownEdge()
    {
        QVERIFY(open());
        const auto edges = edgeNames();

        auto content = saved();
        QVERIFY(content.is_array() && content.size() == 2);

        content[1]["pluginData"]["correlationValues"].push_back({{"id", 1000}, {"value", 0.5}});
        QVERIFY(openNative(content));

        QCOMPARE(edgeNames(), edges);
        QCOMPARE(edgeValue(u"PearsonCorrelationValue"_s, u"r1-r4"_s), 31.0 / 35.0);
    }

    // Saving after deleting a node keeps each row's data with its node
    void deletedNodeSaveAndLoad()
    {
        QVERIFY(open());
        QVERIFY(execute([this] { _document->deleteNode(nodeId(u"r2"_s)); }));

        const auto edges = edgeNames();

        auto url = QUrl::fromLocalFile(_dir.filePath(u"deleted.%1"_s.arg(Application::nativeExtension())));
        QVERIFY(execute([&] { _document->saveFile(url, Application::name(), {}, {}); }));
        QVERIFY(open(url, QString::fromLatin1(Application::NativeFileType), {}));

        QCOMPARE(nodeNames(), (QStringList{u"r1"_s, u"r3"_s, u"r4"_s, u"r5"_s}));
        QCOMPARE(edgeNames(), edges);
        QCOMPARE(nodeValue(u"MeanDataValue"_s, u"r1"_s), 3.5);
        QCOMPARE(nodeValue(u"MeanDataValue"_s, u"r4"_s), 3.5);
        QCOMPARE(nodeValue(u"MaximumDataValue"_s, u"r3"_s), 6.0);
        QCOMPARE(edgeValue(u"PearsonCorrelationValue"_s, u"r1-r4"_s), 31.0 / 35.0);
        QCOMPARE(tableModel()->rowCount(), 4);
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

    // A table of annotations for the data's columns, as from the dialog for importing them
    static std::shared_ptr<TabularData> annotationsTable()
    {
        const QList<QStringList> rows
        {
            {u"Column"_s,   u"c1"_s,  u"c2"_s,  u"c3"_s,  u"c4"_s,  u"c5"_s,  u"c6"_s},
            {u"Kind"_s,     u"p"_s,   u"p"_s,   u"q"_s,   u"q"_s,   u"r"_s,   u"r"_s},
            {u"Batch"_s,    u"1"_s,   u"1"_s,   u"2"_s,   u"2"_s,   u"3"_s,   u"3"_s},
        };

        auto data = std::make_shared<TabularData>();
        data->reserve(7, 3);

        for(qsizetype row = 0; row < rows.size(); row++)
        {
            for(qsizetype column = 0; column < rows.at(row).size(); column++)
                data->setValueAt(static_cast<size_t>(column), static_cast<size_t>(row), rows.at(row).at(column));
        }

        return data;
    }

    bool importAnnotations(const std::vector<int>& rows, bool replace)
    {
        return execute([&]
        {
            QMetaObject::invokeMethod(plugin(), "importAnnotationsFromTable",
                Q_ARG(std::shared_ptr<TabularData>, annotationsTable()), Q_ARG(int, 0),
                Q_ARG(std::vector<int>, rows), Q_ARG(bool, replace));
        });
    }

    void importAnnotations()
    {
        QVERIFY(open());

        auto annotationNames = [this]
        {
            auto names = plugin()->property("columnAnnotationNames").toStringList();
            names.sort();
            return names;
        };

        QVERIFY(importAnnotations({2}, false));
        QCOMPARE(annotationNames(), (QStringList{u"Batch"_s, u"Kind"_s}));
        QCOMPARE(_document->property("status").toString(), u"Annotation Batch Imported"_s);

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(annotationNames(), QStringList{u"Kind"_s});

        QVERIFY(execute([this] { _document->redo(); }));
        QCOMPARE(annotationNames(), (QStringList{u"Batch"_s, u"Kind"_s}));

        // Replacing an annotation of the same type, here numbers
        QVERIFY(importAnnotations({2}, true));
        QCOMPARE(annotationNames(), (QStringList{u"Batch"_s, u"Kind"_s}));
        QCOMPARE(_document->property("status").toString(), u"Annotation Batch Imported"_s);

        // Not replacing an existing annotation gives the import a name of its own
        QVERIFY(importAnnotations({1}, false));
        QCOMPARE(annotationNames(), (QStringList{u"Batch"_s, u"Kind"_s, u"Kind(1)"_s}));
    }
    // The data is scaled, normalised, clipped and imputed as the import dialog's options say
    void preprocessing_data()
    {
        QTest::addColumn<QVariantMap>("parameters");
        QTest::addColumn<QString>("attribute");
        QTest::addColumn<QString>("row");
        QTest::addColumn<double>("value");
        QTest::addColumn<QString>("log");

        // c1 is 1, 2, 6, 1, 6, and c6 is 6, 12, 1, 6, 1
        QTest::newRow("Min/max") << QVariantMap{{u"normalise"_s, u"MinMax"_s}} <<
            u"DataValue.c1"_s << u"r2"_s << 0.2 << u"Normalisation: Min/Max"_s;

        // Half the smallest positive value is added, so that zeros can be logged
        QTest::newRow("Log2") << QVariantMap{{u"scaling"_s, u"Log2"_s}} <<
            u"DataValue.c1"_s << u"r2"_s << std::log2(2.5) << u"Scaling: Log2(x + ε)"_s;

        QTest::newRow("Clipped") << QVariantMap{{u"clippingType"_s, u"Constant"_s}, {u"clippingValue"_s, 5.0}} <<
            u"DataValue.c6"_s << u"r2"_s << 5.0 << u"Clipping: Constant (5)"_s;
    }

    void preprocessing()
    {
        QFETCH(const QVariantMap, parameters);
        QFETCH(const QString, attribute);
        QFETCH(const QString, row);
        QFETCH(const double, value);
        QFETCH(const QString, log);

        QVERIFY(open(parameters));
        QVERIFY(std::abs(nodeValue(attribute, row) - value) < 1e-9);
        QVERIFY2(_document->property("log").toString().contains(log), qPrintable(_document->property("log").toString()));
    }

    // Missing values are imputed
    void imputation_data()
    {
        QTest::addColumn<QString>("missingDataType");
        QTest::addColumn<double>("value");
        QTest::addColumn<QString>("log");

        QTest::newRow("Constant") << u"Constant"_s << 7.0 << u"Imputation: Constant (7)"_s;
        QTest::newRow("Column mean") << u"ColumnAverage"_s << 3.0 << u"Imputation: Column Mean"_s;
        QTest::newRow("Row interpolation") << u"RowInterpolation"_s << 3.0 << u"Imputation: Row Interpolate"_s;
    }

    void imputation()
    {
        QFETCH(QString, missingDataType);
        QFETCH(const double, value);
        QFETCH(const QString, log);

        // r2's c2 is missing; c2's others average 3, and r2's neighbours are 2 and 4
        QVERIFY(open(write(u"imputed.csv"_s,
            "Name,c1,c2,c3\n"
            "r1,1,2,3\n"
            "r2,2,,4\n"
            "r3,3,4,5\n"), u"CorrelationCSV"_s, u"Correlation"_s,
        {
            {u"missingDataType"_s, missingDataType},
            {u"missingDataValue"_s, 7.0},
        }));

        QVERIFY(std::abs(nodeValue(u"DataValue.c2"_s, u"r2"_s) - value) < 1e-9);
        QVERIFY2(_document->property("log").toString().contains(log), qPrintable(_document->property("log").toString()));
    }

    // The Data Value attribute has a value per column, which attribute lists show as its children
    void dataValueAttribute()
    {
        QVERIFY(open());

        const std::unique_ptr<AvailableAttributesModel> model(_document->availableAttributesModel(
            static_cast<int>(ElementType::Node), static_cast<int>(ValueType::All)));
        const auto dataValue = model->find(u"DataValue"_s);
        QVERIFY(dataValue.isValid());
        QCOMPARE(model->data(dataValue, model->roleNames().key("hasParameter")).toBool(), true);
        QCOMPARE(model->rowCount(dataValue), 6);

        const auto c3 = model->index(2, 0, dataValue);
        QCOMPARE(c3.data().toString(), u"c3"_s);
        QCOMPARE(model->get(c3), u"DataValue.c3"_s);
        QCOMPARE(model->find(model->get(c3)), c3);

        // As a transform refers to it
        QCOMPARE(graphModel()->attributeValueByName(u"DataValue.c3"_s).numericValueOf(nodeId(u"r3"_s)), 4.0);
    }

    // Attribute names from files are made valid, as a dot, for example, separates a parameter
    void attributeNameFromFile()
    {
        QVERIFY(open(write(u"dotted.csv"_s,
            "Name,gene.type,c1,c2,c3\n"
            "r1,coding,1,2,3\n"
            "r2,noncoding,2,4,6\n"
            "r3,coding,1,2,3\n"), u"CorrelationCSV"_s, u"Correlation"_s));

        QVERIFY(_document->attributeExists(u"gene_type"_s));
        QCOMPARE(graphModel()->attributeValueByName(uR"("gene_type")"_s).stringValueOf(nodeId(u"r2"_s)), u"noncoding"_s);
        QVERIFY(execute([this] { _document->update({uR"(RemoveNodes where $gene_type == "noncoding")"_s}); }));
        QCOMPARE(nodeNames(), (QStringList{u"r1"_s, u"r3"_s}));
    }

    // Discrete data, e.g. categories, and its log
    void discrete()
    {
        // r1 and r4 are the same, and r2 shares two of their three values
        QVERIFY(open(write(u"words.csv"_s,
            "Name,a,b,c\n"
            "r1,x,y,z\n"
            "r2,x,y,y\n"
            "r3,z,z,z\n"
            "r4,x,y,z\n"), u"CorrelationCSV"_s, u"Correlation"_s,
        {
            {u"correlationDataType"_s, u"Discrete"_s},
            {u"discreteCorrelationType"_s, u"SMC"_s},
            {u"minimumThreshold"_s, 0.5},
            {u"initialThreshold"_s, 0.5},
        }));

        QCOMPARE(plugin()->property("numContinuousColumns").toInt(), 0);
        QCOMPARE(plugin()->property("numDiscreteColumns").toInt(), 3);
        QCOMPARE(edgeNames(), (QStringList{u"r1-r2"_s, u"r1-r4"_s, u"r2-r4"_s}));

        const auto log = _document->property("log").toString();
        QVERIFY(log.contains(u"Data Frame: [ Column: 1 Row: 1 Width: 3 Height: 4 ]"_s));
        QVERIFY(log.contains(u"Minimum Correlation Value: 0.5"_s));
        QVERIFY2(log.contains(u"Discrete Correlation Metric: Simple Matching Coefficient"_s), qPrintable(log));

        // The data is saved with the Document
        const auto edges = edgeNames();
        auto url = QUrl::fromLocalFile(_dir.filePath(u"discrete.%1"_s.arg(Application::nativeExtension())));
        QVERIFY(execute([&] { _document->saveFile(url, Application::name(), {}, {}); }));
        QVERIFY(open(url, QString::fromLatin1(Application::NativeFileType), {}));
        QCOMPARE(edgeNames(), edges);
        QCOMPARE(plugin()->property("numDiscreteColumns").toInt(), 3);
    }

    // The columns are the rows, and the rows' attributes are the columns' annotations
    void transposed()
    {
        QVERIFY(open({{u"transpose"_s, u"true"_s}}));

        // Before the default transforms remove the columns that don't correlate
        QStringList names;
        names.reserve(static_cast<qsizetype>(graphModel()->mutableGraph().nodeIds().size()));
        for(auto id : graphModel()->mutableGraph().nodeIds())
            names.append(graphModel()->nodeName(id));
        names.sort();
        QCOMPARE(names, (QStringList{u"c1"_s, u"c2"_s, u"c3"_s, u"c4"_s, u"c5"_s, u"c6"_s}));
        QCOMPARE(graphModel()->attributeValueByName(u"Kind"_s).stringValueOf(nodeId(u"c4"_s)), u"y"_s);
        QCOMPARE(plugin()->property("numContinuousColumns").toInt(), 5);
        QCOMPARE(plugin()->property("columnAnnotationNames").toStringList(), QStringList{u"Group"_s});
        QVERIFY(_document->property("log").toString().contains(u"Transposed"_s));
    }

    // Columns with the same name are made unique
    void duplicateColumnNames()
    {
        QVERIFY(open(write(u"duplicates.csv"_s,
            "Name,c,c,c,d\n"
            "r1,1,2,3,4\n"
            "r2,2,4,6,8\n"), u"CorrelationCSV"_s, u"Correlation"_s));

        const auto columnNames = tableModel()->property("columnNames").toStringList();
        QStringList dataColumnNames;
        for(const auto& name : columnNames)
        {
            if(name.startsWith(u"DataValue"_s))
                dataColumnNames.append(name);
        }

        QCOMPARE(dataColumnNames.size(), 4);
        QCOMPARE(dataColumnNames.removeDuplicates(), 0);
    }

    // The dialog for opening a correlation file finds the data and estimates the size of the graph
    void importParser()
    {
        auto parser = parse(write(u"data.csv"_s, Csv));
        QVERIFY(parser != nullptr);
        QVERIFY(!parser->property("failed").toBool());
        QVERIFY(parser->property("dataHasNumericalRect").toBool());

        auto* model = parser->property("model").value<QAbstractTableModel*>();
        QVERIFY(model != nullptr);
        QCOMPARE(model->rowCount(), 7);
        QCOMPARE(model->columnCount(), 8);
        QCOMPARE(model->data(model->index(3, 2)).toString(), u"2"_s);

        // The numbers, below the column annotation and right of the row attribute
        auto detected = dataRect(parser.get(), "autoDetectDataRectangle");
        QCOMPARE(rect(detected), QRect(2, 2, 6, 5));
        QCOMPARE(detected.value(u"hasMissingValues"_s).toBool(), false);
        QCOMPARE(detected.value(u"hasDiscreteValues"_s).toBool(), false);
        QCOMPARE(detected.value(u"appearsToBeContinuous"_s).toBool(), true);
        QCOMPARE(detected.value(u"minValue"_s).toDouble(), 1.0);
        QCOMPARE(detected.value(u"maxValue"_s).toDouble(), 12.0);

        // Chosen by the user; the rectangle extends to the bottom right
        auto chosen = dataRect(parser.get(), "setDataRectangle", Q_ARG(size_t, 3), Q_ARG(size_t, 4));
        QCOMPARE(rect(chosen), QRect(3, 4, 5, 3));
        QCOMPARE(chosen.value(u"minValue"_s).toDouble(), 1.0);
        QCOMPARE(chosen.value(u"maxValue"_s).toDouble(), 6.0);

        // Including the column annotation and the row attribute, which aren't numbers, and
        // the annotation's empty value for the row attribute
        chosen = dataRect(parser.get(), "setDataRectangle", Q_ARG(size_t, 1), Q_ARG(size_t, 1));
        QCOMPARE(rect(chosen), QRect(1, 1, 7, 6));
        QCOMPARE(chosen.value(u"hasMissingValues"_s).toBool(), true);
        QCOMPARE(chosen.value(u"hasDiscreteValues"_s).toBool(), true);
        QCOMPARE(chosen.value(u"appearsToBeContinuous"_s).toBool(), false);

        // The first column is always the row names
        chosen = dataRect(parser.get(), "setDataRectangle", Q_ARG(size_t, 0), Q_ARG(size_t, 2));
        QCOMPARE(rect(chosen), QRect(1, 2, 7, 5));

        detected = dataRect(parser.get(), "autoDetectDataRectangle");
        QCOMPARE(rect(detected), QRect(2, 2, 6, 5));

        // Transposing the table transposes the rectangle
        const QSignalSpy dataRectChanged(parser.get(), SIGNAL(dataRectChanged()));
        parser->setProperty("transposed", true);
        QCOMPARE(dataRectChanged.count(), 1);
        QCOMPARE(rect(parser->property("dataRect").toMap()), QRect(2, 2, 5, 6));
        QCOMPARE(model->rowCount(), 8);
        QCOMPARE(model->columnCount(), 7);
        QCOMPARE(model->data(model->index(2, 3)).toString(), u"2"_s);

        detected = dataRect(parser.get(), "autoDetectDataRectangle");
        QCOMPARE(rect(detected), QRect(2, 2, 5, 6));

        parser->setProperty("transposed", false);
        QCOMPARE(rect(parser->property("dataRect").toMap()), QRect(2, 2, 6, 5));
        QCOMPARE(model->rowCount(), 7);

        // Every row is sampled, so the estimate is exact: r1 and r2 correlate perfectly, and
        // three other pairs at 0.886
        auto estimate = estimateGraphSize(parser.get(),
        {
            {u"minimumThreshold"_s, 0.7},
            {u"correlationFilterType"_s, static_cast<int>(CorrelationFilterType::Threshold)},
            {u"correlationDataType"_s, static_cast<int>(CorrelationDataType::Continuous)},
            {u"continuousCorrelationType"_s, static_cast<int>(CorrelationType::Pearson)},
        });

        auto keys = estimate.value(u"keys"_s).value<QVector<double>>();
        auto numNodes = estimate.value(u"numNodes"_s).value<QVector<double>>();
        auto numEdges = estimate.value(u"numUniqueEdges"_s).value<QVector<double>>();
        QVERIFY(!keys.isEmpty());
        QCOMPARE(numNodes.size(), keys.size());
        QCOMPARE(numEdges.size(), keys.size());
        QVERIFY(std::ranges::is_sorted(keys));
        QVERIFY(qFuzzyCompare(keys.front(), 31.0 / 35.0));
        QCOMPARE(numNodes.front(), 5.0);
        QCOMPARE(numEdges.front(), 4.0);
        QVERIFY(qFuzzyCompare(keys.back(), 1.0));
        QCOMPARE(numNodes.back(), 2.0);
        QCOMPARE(numEdges.back(), 1.0);

        // Each row's most correlated other row
        estimate = estimateGraphSize(parser.get(),
        {
            {u"maximumK"_s, 2},
            {u"correlationFilterType"_s, static_cast<int>(CorrelationFilterType::Knn)},
            {u"correlationDataType"_s, static_cast<int>(CorrelationDataType::Continuous)},
            {u"continuousCorrelationType"_s, static_cast<int>(CorrelationType::Pearson)},
        });

        keys = estimate.value(u"keys"_s).value<QVector<double>>();
        numNodes = estimate.value(u"numNodes"_s).value<QVector<double>>();
        QCOMPARE(keys, (QVector<double>{2.0, 1.0}));
        QCOMPARE(numNodes, (QVector<double>{5.0, 5.0}));
    }

    void importParserDiscrete()
    {
        // Few distinct whole numbers, and one missing
        auto parser = parse(write(u"discrete.tsv"_s,
            "Name\ta\tb\tc\n"
            "r1\t1\t1\t2\n"
            "r2\t1\t2\t\n"
            "r3\t1\t1\t2\n"
            "r4\t2\t1\t2\n"
            "r5\t1\t1\t2\n"), u"CorrelationTSV"_s);
        QVERIFY(parser != nullptr);

        auto detected = dataRect(parser.get(), "autoDetectDataRectangle");
        QCOMPARE(rect(detected), QRect(1, 1, 3, 5));
        QCOMPARE(detected.value(u"hasMissingValues"_s).toBool(), true);
        QCOMPARE(detected.value(u"hasDiscreteValues"_s).toBool(), false);
        QCOMPARE(detected.value(u"appearsToBeContinuous"_s).toBool(), false);

        // Words
        parser = parse(write(u"words.csv"_s,
            "Name,a,b,c\n"
            "r1,x,y,z\n"
            "r2,x,y,y\n"
            "r3,z,z,z\n"));
        QVERIFY(parser != nullptr);
        QVERIFY(!parser->property("dataHasNumericalRect").toBool());

        detected = dataRect(parser.get(), "autoDetectDataRectangle");
        QCOMPARE(rect(detected), QRect(1, 1, 3, 3));
        QCOMPARE(detected.value(u"hasDiscreteValues"_s).toBool(), true);

        // r1 has two values in common with r2, and one with r3
        const auto estimate = estimateGraphSize(parser.get(),
        {
            {u"minimumThreshold"_s, 0.1},
            {u"correlationFilterType"_s, static_cast<int>(CorrelationFilterType::Threshold)},
            {u"correlationDataType"_s, static_cast<int>(CorrelationDataType::Discrete)},
            {u"discreteCorrelationType"_s, static_cast<int>(CorrelationType::SMC)},
        });

        const auto keys = estimate.value(u"keys"_s).value<QVector<double>>();
        QCOMPARE(keys.size(), 2);
        QVERIFY(qFuzzyCompare(keys.at(0), 1.0 / 3.0));
        QVERIFY(qFuzzyCompare(keys.at(1), 2.0 / 3.0));
        QCOMPARE(estimate.value(u"numUniqueEdges"_s).value<QVector<double>>(), (QVector<double>{2.0, 1.0}));
    }

    void importParserFailure()
    {
        auto parser = parse(QUrl::fromLocalFile(_dir.filePath(u"missing.csv"_s)));
        QVERIFY(parser != nullptr);
        QVERIFY(parser->property("failed").toBool());
        QVERIFY(parser->property("complete").toBool());

        parser = parse(write(u"data.csv"_s, Csv), u"NotAType"_s);
        QVERIFY(parser != nullptr);
        QVERIFY(parser->property("failed").toBool());
    }

    // Finding the row of a table of annotations that names the data's columns
    void importAnnotationsKeyDetection_data()
    {
        QTest::addColumn<QList<QStringList>>("rows");
        QTest::addColumn<int>("row");
        QTest::addColumn<int>("percent");

        QTest::newRow("First row") << QList<QStringList>
        {
            {u"Column"_s, u"c1"_s, u"c2"_s, u"c3"_s, u"c4"_s, u"c5"_s, u"c6"_s},
            {u"Kind"_s,   u"p"_s,  u"p"_s,  u"q"_s,  u"q"_s,  u"r"_s,  u"r"_s},
        } << 0 << 100;

        QTest::newRow("Partial") << QList<QStringList>
        {
            {u"Kind"_s,   u"p"_s,  u"p"_s,  u"q"_s,  u"q"_s,  u"r"_s,  u"r"_s},
            {u"Column"_s, u"c1"_s, u"c2"_s, u"c3"_s, u"x"_s,  u"y"_s,  u""_s},
        } << 1 << 50;

        // The earlier of two equally good rows
        QTest::newRow("Tie") << QList<QStringList>
        {
            {u"Kind"_s,   u"p"_s,  u"p"_s,  u"q"_s,  u"q"_s,  u"r"_s,  u"r"_s},
            {u"A"_s,      u"c1"_s, u"c2"_s, u"x"_s,  u"x"_s,  u"x"_s,  u"x"_s},
            {u"B"_s,      u"x"_s,  u"x"_s,  u"c3"_s, u"c4"_s, u"x"_s,  u"x"_s},
        } << 1 << 33;

        // Rows of numbers that aren't whole aren't keys
        QTest::newRow("Floats") << QList<QStringList>
        {
            {u"Kind"_s,   u"p"_s,  u"p"_s,  u"q"_s,  u"q"_s,  u"r"_s,  u"r"_s},
            {u"1.5"_s,    u"1.5"_s, u"2.5"_s, u"3.5"_s, u"4.5"_s, u"5.5"_s, u"6.5"_s},
        } << 0 << 0;
    }

    void importAnnotationsKeyDetection()
    {
        QFETCH(const QList<QStringList>, rows);
        QFETCH(const int, row);
        QFETCH(const int, percent);

        QVERIFY(open());

        auto data = std::make_shared<TabularData>();
        data->reserve(static_cast<size_t>(rows.at(0).size()), static_cast<size_t>(rows.size()));
        for(qsizetype r = 0; r < rows.size(); r++)
        {
            for(qsizetype c = 0; c < rows.at(r).size(); c++)
                data->setValueAt(static_cast<size_t>(c), static_cast<size_t>(r), rows.at(r).at(c));
        }

        auto keyDetection = createQmlObject(u"ImportAnnotationsKeyDetection"_s);
        QVERIFY(keyDetection != nullptr);
        QVERIFY(keyDetection->setProperty("plugin", QVariant::fromValue(plugin())));
        QVERIFY(keyDetection->setProperty("tabularData", QVariant::fromValue(data)));

        QSignalSpy resultChanged(keyDetection.get(), SIGNAL(resultChanged()));
        QMetaObject::invokeMethod(keyDetection.get(), "start");
        QVERIFY(QTest::qWaitFor([&] { return !resultChanged.isEmpty() && !keyDetection->property("busy").toBool(); }, Timeout));

        const auto result = keyDetection->property("result").toMap();
        QCOMPARE(result.value(u"row"_s).toInt(), row);
        QCOMPARE(result.value(u"percent"_s).toInt(), percent);
    }
};

QTEST_MAIN(TestCorrelationPlugin)
#include "tst_correlationplugin.moc"
