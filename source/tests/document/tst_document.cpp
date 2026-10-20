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

#include "app/attributes/attributeedits.h"
#include "app/attributes/enrichmenttablemodel.h"
#include "app/layout/nodepositions.h"
#include "app/ui/qml/Graphia/availableattributesmodel.h"
#include "app/ui/qml/Graphia/alert.h"
#include "app/ui/qml/Graphia/availabletransformsmodel.h"
#include "app/ui/qml/Graphia/editattributetablemodel.h"
#include "app/ui/qml/Graphia/findtypes.h"
#include "app/ui/qml/Graphia/importattributeskeydetection.h"
#include "app/ui/qml/Graphia/Utils/preferences.h"

#include "shared/loading/tabulardata.h"
#include "shared/utils/container.h"
#include "shared/utils/json.h"

#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QUrl>
#include <QVector3D>

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
// a -> b -> c -> d, with a -> c and e on its own
const QByteArray Gml = R"(graph
[
    directed 1
    node [ id 0 label "a" score 1 kind "x" ]
    node [ id 1 label "b" score 2 kind "x" ]
    node [ id 2 label "c" score 3 kind "x" ]
    node [ id 3 label "d" score 4 kind "y" ]
    node [ id 4 label "e" score 5 kind "y" ]
    edge [ source 0 target 1 ]
    edge [ source 1 target 2 ]
    edge [ source 2 target 3 ]
    edge [ source 0 target 2 ]
])";
} // namespace

// Document, as the UI uses it
class TestDocument_ : public DocumentTest
{
    Q_OBJECT

    using DocumentTest::open;

    bool open()
    {
        return open(write(u"graph.gml"_s, Gml), u"GML"_s, u"Generic"_s);
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

        QCOMPARE(graph().numNodes(), 5u);
        QCOMPARE(graph().numEdges(), 4u);
        QCOMPARE(graph().numComponents(), 2u);
        QVERIFY(!_document->canUndo());
    }

    void applyTransformUndoRedo()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({u"RemoveNodes where $score > 3"_s}); }));
        QCOMPARE(graph().numNodes(), 3u);
        QCOMPARE(_document->transforms().size(), 1);
        QVERIFY(_document->canUndo());

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(graph().numNodes(), 5u);
        QVERIFY(_document->transforms().isEmpty());
        QVERIFY(_document->canRedo());

        QVERIFY(execute([this] { _document->redo(); }));
        QCOMPARE(graph().numNodes(), 3u);
        QCOMPARE(_document->transforms().size(), 1);
    }

    void selection()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->selectAll(); }));
        QCOMPARE(_document->property("numNodesSelected").toInt(), 5);

        QVERIFY(execute([this] { _document->selectNone(); }));
        QCOMPARE(_document->property("numNodesSelected").toInt(), 0);

        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"b"_s)); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        QVERIFY(execute([this] { _document->invertSelection(); }));
        QCOMPARE(selected(), (QStringList{u"d"_s, u"e"_s}));

        QVERIFY(execute([this] { _document->selectSources(nodeId(u"c"_s), false); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        QVERIFY(execute([this] { _document->selectTargets(nodeId(u"a"_s), false); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        QVERIFY(execute([this] { _document->selectTargets(nodeId(u"c"_s), true); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s}));

        // Selection isn't undoable
        QVERIFY(!_document->canUndo());
    }

    void find()
    {
        QVERIFY(open());

        QVERIFY(execute([this]
        {
            _document->find(u"[ab]"_s, static_cast<int>(FindOptions::MatchUsingRegex),
                {u"NodeName"_s}, static_cast<int>(FindSelectStyle::All));
        }));

        QCOMPARE(_document->property("numNodesFound").toInt(), 2);
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s}));

        QVERIFY(execute([this] { _document->resetFind(); }));
        QCOMPARE(_document->property("numNodesFound").toInt(), 0);
    }

    void deleteNodes()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->deleteNode(nodeId(u"c"_s)); }));
        QCOMPARE(nodeNames(), (QStringList{u"a"_s, u"b"_s, u"d"_s, u"e"_s}));
        QCOMPARE(graph().numEdges(), 1u);
        QVERIFY(_document->property("saveRequired").toBool());

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(graph().numNodes(), 5u);
        QCOMPARE(graph().numEdges(), 4u);

        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"a"_s), false); }));
        QVERIFY(execute([this] { _document->deleteSelectedNodes(); }));
        QCOMPARE(nodeNames(), (QStringList{u"d"_s, u"e"_s}));
        QCOMPARE(_document->property("numNodesSelected").toInt(), 0);
    }

    void bookmarks()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"d"_s), false); }));
        _document->addBookmark(u"cd"_s);
        QCOMPARE(_document->bookmarks(), QStringList{u"cd"_s});

        QVERIFY(execute([this] { _document->selectNone(); }));
        QVERIFY(execute([this] { _document->gotoBookmark(u"cd"_s); }));
        QCOMPARE(selected(), (QStringList{u"c"_s, u"d"_s}));

        _document->renameBookmark(u"cd"_s, u"Renamed"_s);
        QCOMPARE(_document->bookmarks(), QStringList{u"Renamed"_s});

        _document->removeBookmarks({u"Renamed"_s});
        QVERIFY(_document->bookmarks().isEmpty());
    }

    void attributes()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->cloneAttribute(u"score"_s, u"Copy"_s); }));
        QVERIFY(_document->attributeExists(u"Copy"_s));
        QCOMPARE(_document->allAttributeValues(u"Copy"_s).size(), 5);

        QVERIFY(execute([this] { _document->removeAttributes({u"score"_s}); }));
        QVERIFY(!_document->attributeExists(u"score"_s));

        QVERIFY(execute([this] { _document->undo(); }));
        QVERIFY(_document->attributeExists(u"score"_s));

        QVERIFY(execute([this] { _document->undo(); }));
        QVERIFY(!_document->attributeExists(u"Copy"_s));
    }

    // Editing an attribute that a transform refers to applies the transforms again
    void editAttributeUsedByTransform()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({u"RemoveNodes where $score > 3"_s}); }));
        QCOMPARE(nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        AttributeEdits edits;
        edits.setNodeValue(nodeId(u"a"_s), u"10"_s);
        QVERIFY(execute([&] { _document->editAttribute(u"score"_s, edits, ValueType::Unknown, {}); }));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s}));

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));

        // The transforms are unchanged throughout
        QCOMPARE(_document->transforms().size(), 1);
    }

    // As the context menu's "Select Shared Values", and selecting by an attribute's value
    void selectByValue()
    {
        QVERIFY(open());

        // The nodes that share d's kind
        QVERIFY(execute([this] { _document->selectBySharedAttributeValue(u"kind"_s, nodeId(u"d"_s), false); }));
        QCOMPARE(selected(), (QStringList{u"d"_s, u"e"_s}));

        // ...and, adding the selection's, a's score
        QVERIFY(execute([this] { _document->selectBySharedAttributeValue(u"score"_s, nodeId(u"a"_s), true); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"d"_s, u"e"_s}));

        // The selection's
        QVERIFY(execute([this] { _document->selectBySharedAttributeValue(u"kind"_s); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s, u"e"_s}));

        QVERIFY(execute([this] { _document->selectByAttributeValue(u"kind"_s, u"^y$"_s); }));
        QCOMPARE(selected(), (QStringList{u"d"_s, u"e"_s}));

        QVERIFY(execute([this] { _document->selectByAttributeValue(u"score"_s, u"^[12]$"_s); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s}));

        // Nothing matches, or there's no such attribute, so nothing is selected
        _document->selectByAttributeValue(u"kind"_s, u"z"_s);
        QVERIFY(QTest::qWaitFor([this] { return selected().isEmpty(); }));

        QVERIFY(execute([this] { _document->selectAll(); }));
        _document->selectByAttributeValue(u"Not an attribute"_s, u"x"_s);
        QVERIFY(QTest::qWaitFor([this] { return selected().isEmpty(); }));
    }

    // What the UI shows of the selection
    void selectionInfo()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->selectSources(nodeId(u"c"_s), false); }));
        QCOMPARE(_document->property("numHeadNodesSelected").toInt(), 3);

        const auto selectedNodeIds = _document->property("selectedNodeIds").toList();

        QStringList names;
        names.reserve(selectedNodeIds.size());
        for(const auto& id : selectedNodeIds)
            names.append(_document->nodeName(id.value<QmlNodeId>()));
        names.sort();
        QCOMPARE(names, (QStringList{u"a"_s, u"b"_s, u"c"_s}));
        QCOMPARE(_document->property("selectedHeadNodeIds").toList().size(), 3);

        QVERIFY(_document->nodeIsSelected(nodeId(u"a"_s)));
        QVERIFY(!_document->nodeIsSelected(nodeId(u"d"_s)));

        QVERIFY(_document->property("directed").toBool());
        QVERIFY(_document->property("editable").toBool());

        QCOMPARE(_document->graphSizeSummary(),
            u"Mutable Graph Nodes: 5 Edges: 4\nTransformed Graph Nodes: 5 Edges: 4 Components: 2"_s);
    }

    // Stepping through the nodes found, as the find panel's buttons do
    void findNavigation()
    {
        QVERIFY(open());

        QVERIFY(execute([this]
        {
            _document->find(u"[abd]"_s, static_cast<int>(FindOptions::MatchUsingRegex),
                {u"NodeName"_s}, static_cast<int>(FindSelectStyle::None));
        }));
        QCOMPARE(_document->property("numNodesFound").toInt(), 3);

        // With nothing selected, the first found node is selected
        QCOMPARE(selected(), QStringList{u"a"_s});
        QCOMPARE(_document->property("foundIndex").toInt(), 0);

        // The nodes found are in order, by component, wrapping around
        QVERIFY(execute([this] { _document->selectNextFound(); }));
        QCOMPARE(selected(), QStringList{u"b"_s});
        QVERIFY(execute([this] { _document->selectNextFound(); }));
        QCOMPARE(selected(), QStringList{u"d"_s});
        QCOMPARE(_document->property("foundIndex").toInt(), 2);
        QVERIFY(execute([this] { _document->selectNextFound(); }));
        QCOMPARE(selected(), QStringList{u"a"_s});

        QVERIFY(execute([this] { _document->selectPrevFound(); }));
        QCOMPARE(selected(), QStringList{u"d"_s});

        QVERIFY(execute([this] { _document->selectFirstFound(); }));
        QCOMPARE(selected(), QStringList{u"a"_s});

        QVERIFY(execute([this] { _document->selectAllFound(); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"d"_s}));
        QCOMPARE(_document->property("foundIndex").toInt(), -1);
    }

    void gotoAllBookmarks()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->selectSources(nodeId(u"b"_s), false); }));
        _document->addBookmark(u"ab"_s);
        QVERIFY(execute([this] { _document->selectBySharedAttributeValue(u"kind"_s, nodeId(u"e"_s), false); }));
        _document->addBookmark(u"de"_s);

        QVERIFY(execute([this] { _document->selectNone(); }));
        QVERIFY(execute([this] { _document->gotoAllBookmarks(); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"d"_s, u"e"_s}));

        // Bookmarked nodes that have been removed are skipped
        QVERIFY(execute([this] { _document->update({uR"(RemoveNodes where $NodeName == "a")"_s}); }));
        QVERIFY(execute([this] { _document->selectNone(); }));
        QVERIFY(execute([this] { _document->gotoAllBookmarks(); }));
        QCOMPARE(selected(), (QStringList{u"b"_s, u"d"_s, u"e"_s}));
    }

    // The details the transform and visualisation panels show
    void transformAndVisualisationDetails()
    {
        QVERIFY(open());

        auto k = _document->transformParameter(u"KNN"_s, u"k"_s);
        QCOMPARE(k.value(u"name"_s).toString(), u"k"_s);
        QCOMPARE(k.value(u"displayName"_s).toString(), u"k"_s);
        QCOMPARE(k.value(u"valueType"_s).toInt(), static_cast<int>(ValueType::Int));
        QCOMPARE(k.value(u"hasMinimumValue"_s).toBool(), true);
        QCOMPARE(k.value(u"minimumValue"_s).toInt(), 1);
        QCOMPARE(k.value(u"hasMaximumValue"_s).toBool(), false);
        QCOMPARE(k.value(u"initialValue"_s).toInt(), 5);

        auto rankOrder = _document->transformParameter(u"KNN"_s, u"RankOrder"_s);
        QCOMPARE(rankOrder.value(u"initialIndex"_s).toInt(), 0);
        QCOMPARE(rankOrder.value(u"displayName"_s).toString(), u"Rank Order"_s);
        QVERIFY(_document->transformParameter(u"KNN"_s, u"Not a parameter"_s).isEmpty());
        QVERIFY(_document->transformParameter(u"Not a transform"_s, u"k"_s).isEmpty());

        const auto knn = _document->transform(u"KNN"_s);
        QCOMPARE(knn.value(u"displayName"_s).toString(), u"k-NN"_s);
        QCOMPARE(knn.value(u"attributeParameterNames"_s).toStringList(), QStringList{u"Ranked Attribute"_s});
        const auto rankedAttribute = knn.value(u"attributeParameters"_s).toList().at(0).toMap();
        QCOMPARE(rankedAttribute.value(u"elementType"_s).toInt(), static_cast<int>(ElementType::Edge));
        QCOMPARE(rankedAttribute.value(u"valueType"_s).toInt(), static_cast<int>(ValueType::Numerical));

        const auto parsed = _document->parseGraphTransform(u"RemoveNodes where $score > 3"_s);
        QCOMPARE(parsed.value(u"action"_s).toString(), u"RemoveNodes"_s);
        QCOMPARE(parsed.value(u"displayName"_s).toString(), u"Remove Nodes"_s);
        QVERIFY(_document->parseGraphTransform(u"not a transform"_s).isEmpty());

        QVERIFY(execute([this] { _document->update({u"RemoveNodes where $score > 3"_s}); }));
        QCOMPARE(_document->transformInfoAtIndex(0).value(u"alertType"_s).toInt(), static_cast<int>(AlertType::None));

        // Of the nodes that remain
        QVERIFY(execute([this]
        {
            _document->update({u"RemoveNodes where $score > 3"_s},
                {u"score Size"_s, u"kind Colour"_s});
        }));
        auto info = _document->visualisationInfoAtIndex(0);
        QCOMPARE(info.value(u"minimumNumericValue"_s).toDouble(), 1.0);
        QCOMPARE(info.value(u"maximumNumericValue"_s).toDouble(), 3.0);
        QCOMPARE(info.value(u"hasNumericRange"_s).toBool(), true);
        QCOMPARE(info.value(u"alertType"_s).toInt(), static_cast<int>(AlertType::None));

        auto values = _document->visualisationNumericValuesAtIndex(0);
        std::ranges::sort(values);
        QCOMPARE(values, (QList<double>{1.0, 2.0, 3.0}));

        auto strings = _document->visualisationStringValuesAtIndex(1);
        strings.removeDuplicates();
        QCOMPARE(strings, QStringList{u"x"_s});

        const auto parsedVisualisation = _document->parseVisualisation(u"kind Colour"_s);
        QCOMPARE(parsedVisualisation.value(u"attribute"_s).toString(), u"kind"_s);
        QCOMPARE(parsedVisualisation.value(u"channel"_s).toString(), u"Colour"_s);

        QVERIFY(_document->visualisationsAreValid({u"kind Colour"_s}));
        QVERIFY(!_document->visualisationsAreValid({u"kind Colour"_s, uR"("kind" "Not a channel")"_s}));
        QVERIFY(!_document->visualisationDefaultParameters(static_cast<int>(ValueType::Float), u"Colour"_s).isEmpty());

        // Attributes like another, for the visualisation panel's menu of alternatives
        const std::unique_ptr<AvailableAttributesModel> similar(_document->attributesSimilarTo(u"score"_s, 0));
        QVERIFY(similar != nullptr);
        QVERIFY(_document->attributesSimilarTo(u"Not an attribute"_s, 0) == nullptr);
    }

    // The layout's settings, as the layout settings panel shows them
    void layoutSettings()
    {
        QVERIFY(open());

        const auto names = _document->property("layoutSettingNames").toStringList();
        QVERIFY(!names.isEmpty());

        const auto setting = _document->layoutSetting(names.at(0));
        QCOMPARE(setting.value(u"name"_s).toString(), names.at(0));
        QVERIFY(setting.value(u"minimumValue"_s).toFloat() <= setting.value(u"value"_s).toFloat());
        QVERIFY(setting.value(u"value"_s).toFloat() <= setting.value(u"maximumValue"_s).toFloat());
        QVERIFY(_document->layoutSetting(u"Not a setting"_s).isEmpty());
    }

    // What the Application tells the UI about opening files, and links
    void application()
    {
        auto* application = _application.get();

        const auto gml = write(u"graph.gml"_s, Gml);
        QCOMPARE(application->urlTypesOf(gml), QStringList{u"GML"_s});
        QCOMPARE(application->urlTypesFor(u"gml"_s), QStringList{u"GML"_s});
        QVERIFY(application->urlTypesFor(u"csv"_s).contains(u"CorrelationCSV"_s));
        QVERIFY(application->pluginNames(u"GML"_s).contains(u"Generic"_s));
        QCOMPARE(application->pluginNames(u"CorrelationCSV"_s), QStringList{u"Correlation"_s});
        QVERIFY(application->pluginNames(u"Not a type"_s).isEmpty());

        QVERIFY(application->canOpen(u"GML"_s));
        QVERIFY(application->canOpen(QString::fromLatin1(Application::NativeFileType)));
        QVERIFY(!application->canOpen(u"Not a type"_s));
        QVERIFY(application->canOpenAnyOf({u"Not a type"_s, u"GML"_s}));
        QVERIFY(!application->canOpenAnyOf({u"Not a type"_s}));

        // A file type is identified by its description and extensions, as the open file dialog lists them
        const auto description = application->descriptionForUrlType(u"GML"_s);
        QVERIFY(!description.isEmpty());
        QCOMPARE(application->urlTypeFor(description, {u"gml"_s}), u"GML"_s);
        QVERIFY(application->urlTypeFor(description, {u"xml"_s}).isEmpty());

        // Why a file couldn't be identified, here as its extension doesn't match its content
        const auto tabs = write(u"tabs.csv"_s, "Name\ta\tb\nr1\t1\t2\nr2\t3\t4\n");
        QVERIFY(!application->urlTypesOf(tabs).contains(u"CorrelationCSV"_s));
        const auto reasons = application->failureReasons(tabs);
        QVERIFY(std::ranges::any_of(reasons, [](const auto& reason) { return reason.contains(u"resembles a 'TSV' file"_s); }));
        QVERIFY(application->failureReasons(write(u"unknown.xyz"_s, "?")).isEmpty());

        auto saverNames = [application]
        {
            const auto saverFileTypes = application->saverFileTypes();

            QStringList names;
            names.reserve(saverFileTypes.size());
            for(const auto& saver : saverFileTypes)
                names.append(saver.toMap().value(u"extension"_s).toString());
            return names;
        }();
        QVERIFY(saverNames.contains(u"gml"_s));
        QVERIFY(saverNames.contains(u"graphml"_s));

        const std::unique_ptr<QAbstractItemModel> plugins(application->pluginDetailsModel());
        QCOMPARE(plugins->rowCount(), 3);
        QVERIFY(!plugins->data(plugins->index(0, 0), Qt::DisplayRole).toString().isEmpty());
        const std::unique_ptr<QAbstractItemModel> urlTypes(application->urlTypeDetailsModel());
        QVERIFY(urlTypes->rowCount() > 0);

        // Links to Graphia itself
        const QUrl link(u"%1://v2/open/a%20b/"_s.arg(Application::nativeExtension()));
        QVERIFY(application->isNativeLink(link));
        QCOMPARE(application->linkVersionFor(link), 2);
        QCOMPARE(application->linkArgumentsFor(link), (QStringList{u"open"_s, u"a b"_s}));
        QCOMPARE(application->linkVersionFor(QUrl(u"%1://x/open"_s.arg(Application::nativeExtension()))), -1);
        QVERIFY(!application->isNativeLink(QUrl(u"https://graphia.app"_s)));
        QCOMPARE(application->linkVersionFor(QUrl(u"https://graphia.app"_s)), -1);

        // The files that come with Graphia, here the tests in the build directory
        QVERIFY(application->isResourceFile(QCoreApplication::applicationFilePath()));
        QVERIFY(!application->isResourceFile(gml.toLocalFile()));
        QVERIFY(!application->isResourceFileUrl(gml));

        // Beside the build directory, and in one whose name starts with the build directory's
        for(const auto& name : {u"tst_document-XXXXXX"_s, QFileInfo(QStringLiteral(GRAPHIA_BUILD_DIR)).fileName() + u"-XXXXXX"_s})
        {
            const QTemporaryDir besideBuildDir(QFileInfo(QStringLiteral(GRAPHIA_BUILD_DIR)).dir().filePath(name));
            QVERIFY(besideBuildDir.isValid());

            QFile file(besideBuildDir.filePath(u"Tutorial.graphia"_s));
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.close();

            QVERIFY2(!application->isResourceFile(file.fileName()), qPrintable(file.fileName()));
        }
    }

    // The transforms offered when adding one, by category, with the user's favourites first
    void availableTransforms()
    {
        QVERIFY(open());

        const std::unique_ptr<AvailableTransformsModel> model(_document->availableTransforms());
        QVERIFY(model != nullptr);
        QVERIFY(model->roleNames().values().contains("category"));

        auto valueFor = [&model](const QString& transformName, const QByteArray& roleName)
        {
            for(int row = 0; row < model->rowCount({}); row++)
            {
                auto index = model->index(row);
                if(model->get(index).toString() == transformName)
                    return model->data(index, model->roleNames().key(roleName));
            }

            return QVariant{};
        };

        QCOMPARE(model->rowCount({}), static_cast<int>(graphModel()->availableTransformNames().size()));
        QCOMPARE(valueFor(u"PageRank"_s, "category").toString(), u"Metrics"_s);
        QCOMPARE(valueFor(u"PageRank"_s, "isFavourite").toBool(), false);
        QVERIFY(!model->data(model->index(model->rowCount({})), Qt::DisplayRole).isValid());

        QSignalSpy modelReset(model.get(), &AvailableTransformsModel::modelReset);
        u::setPref(u"misc/favouriteTransforms"_s, uR"(["PageRank", 1])"_s);
        QVERIFY(QTest::qWaitFor([&] { return !modelReset.isEmpty(); }));
        QCOMPARE(valueFor(u"PageRank"_s, "category").toString(), u"Favourites"_s);
        QCOMPARE(valueFor(u"PageRank"_s, "isFavourite").toBool(), true);
        QCOMPARE(valueFor(u"Betweenness"_s, "category").toString(), u"Metrics"_s);

        // Not JSON
        u::setPref(u"misc/favouriteTransforms"_s, u"PageRank"_s);
        QTest::qWait(0);
        QCOMPARE(valueFor(u"PageRank"_s, "isFavourite").toBool(), true);

        u::setPref(u"misc/favouriteTransforms"_s, u"[]"_s);
        QVERIFY(QTest::qWaitFor([&] { return modelReset.count() == 2; }));
        QCOMPARE(valueFor(u"PageRank"_s, "isFavourite").toBool(), false);
    }

    void visualisations()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({}, {u"score Colour"_s}); }));
        QCOMPARE(_document->visualisations().size(), 1);
        QVERIFY(_document->visualisations().at(0).startsWith(u"score Colour"_s));
        QVERIFY(_document->hasVisualisationInfo());

        QVERIFY(execute([this] { _document->undo(); }));
        QVERIFY(_document->visualisations().isEmpty());
    }

    // As when dragging one in the visualisation panel; there can be more visualisations than transforms
    void moveVisualisation()
    {
        QVERIFY(open());

        const QStringList visualisations
        {
            u"score Colour"_s,
            u"kind Text"_s,
            u"score Size"_s,
        };

        // No transforms
        QVERIFY(execute([&] { _document->update({}, visualisations); }));
        QVERIFY(_document->transforms().isEmpty());

        auto channels = [this]
        {
            const auto documentVisualisations = _document->visualisations();

            QStringList result;
            result.reserve(documentVisualisations.size());
            for(const auto& visualisation : documentVisualisations)
                result.append(_document->parseVisualisation(visualisation).value(u"channel"_s).toString());

            return result;
        };

        QCOMPARE(channels(), (QStringList{u"Colour"_s, u"Text"_s, u"Size"_s}));

        _document->moveVisualisation(0, 2);
        QVERIFY(QTest::qWaitFor([&] { return channels() == QStringList{u"Text"_s, u"Size"_s, u"Colour"_s}; }));
        QVERIFY(waitForCommands());

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(channels(), (QStringList{u"Colour"_s, u"Text"_s, u"Size"_s}));
    }

    // As the transform panel edits them; the edits are only applied when the panel applies them
    void transformsEdited()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->update({u"RemoveNodes where $score > 3"_s}); }));
        QVERIFY(execute([this] { _document->update({uR"(RemoveNodes where $NodeName == "a")"_s}); }));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s}));

        auto applyFromUI = [this]
        {
            _document->apply(_document->property("transforms").toStringList(),
                _document->property("visualisations").toStringList());
        };

        _document->setGraphTransform(0, u"RemoveNodes where $score > 4"_s);
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s}));
        QVERIFY(execute(applyFromUI));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s, u"d"_s}));

        _document->removeGraphTransform(0);
        QVERIFY(execute(applyFromUI));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s, u"d"_s, u"e"_s}));
        QCOMPARE(_document->transforms().size(), 1);

        QVERIFY(execute([this] { _document->undo(); }));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s, u"d"_s}));

        const auto transforms = _document->transforms();
        QVERIFY(execute([this] { _document->moveGraphTransform(0, 1); }));
        QCOMPARE(_document->transforms(), (QStringList{transforms.at(1), transforms.at(0)}));
        QCOMPARE(nodeNames(), (QStringList{u"b"_s, u"c"_s, u"d"_s}));
    }

    // What's saved is what's loaded, including the transforms, visualisations and bookmarks
    void saveAndLoad()
    {
        QVERIFY(open());

        QVERIFY(execute([this]
        {
            _document->update({u"RemoveNodes where $score > 4"_s}, {u"score Size"_s});
        }));

        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"a"_s), false); }));
        _document->addBookmark(u"Around a"_s);

        const auto transforms = _document->transforms();
        const auto visualisations = _document->visualisations();

        auto url = QUrl::fromLocalFile(_dir.filePath(u"saved.%1"_s.arg(Application::nativeExtension())));
        QSignalSpy saveComplete(_document.get(), &Document::saveComplete);
        QVERIFY(execute([&] { _document->saveFile(url, Application::name(), {}, {}); }));
        QVERIFY(QTest::qWaitFor([&] { return !saveComplete.isEmpty(); }, Timeout));
        QVERIFY(saveComplete.at(0).at(0).toBool());
        QVERIFY(!_document->property("saveRequired").toBool());

        QVERIFY(open(url, QString::fromLatin1(Application::NativeFileType), {}));

        QCOMPARE(nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s}));
        QCOMPARE(_document->transforms(), transforms);
        QCOMPARE(_document->visualisations(), visualisations);
        QCOMPARE(_document->bookmarks(), QStringList{u"Around a"_s});
        QVERIFY(execute([this] { _document->gotoBookmark(u"Around a"_s); }));
        QCOMPARE(selected(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));
    }

    void loadFailure()
    {
        QVERIFY(!open(write(u"broken.gml"_s, "graph [ node [ id 0 "), u"GML"_s, u"Generic"_s));
        QVERIFY(!_document->property("failureReason").toString().isEmpty());

        QVERIFY(!open(write(u"graph.gml"_s, Gml), u"GML"_s, u"Nothing"_s));
    }

    // The table in the dialog for editing an attribute's values
    void editAttributeTable()
    {
        QVERIFY(open());

        EditAttributeTableModel model;
        model.setProperty("document", QVariant::fromValue(_document.get()));
        model.setProperty("attributeName", u"kind"_s);

        auto value = [&model](int row, int column)
            { return model.data(model.index(row, column), Qt::DisplayRole); };

        // Sorted by name, by default
        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(value(0, 0).toString(), u"a"_s);
        QCOMPARE(value(4, 0).toString(), u"e"_s);
        QCOMPARE(value(3, 1).toString(), u"y"_s);

        model.setProperty("ascendingSortOrder", false);
        QCOMPARE(value(0, 0).toString(), u"e"_s);
        model.setProperty("ascendingSortOrder", true);

        model.editValue(1, u"z"_s);
        QVERIFY(model.property("hasEdits").toBool());
        QVERIFY(model.rowIsEdited(1));
        QVERIFY(!model.rowIsEdited(0));
        QCOMPARE(value(1, 1).toString(), u"z"_s);

        model.resetRowValue(1);
        QVERIFY(!model.property("hasEdits").toBool());
        QCOMPARE(value(1, 1).toString(), u"x"_s);

        // Editing a value back to what it was is no edit
        model.editValue(1, u"z"_s);
        model.editValue(1, u"x"_s);
        QCOMPARE(value(1, 1).toString(), u"x"_s);
        QVERIFY(!model.rowIsEdited(1));
        QVERIFY(!model.property("hasEdits").toBool());

        // Numbers too, though what's typed is a string
        model.setProperty("attributeName", u"score"_s);
        model.editValue(1, u"9"_s);
        model.editValue(1, u"2"_s);
        QVERIFY(!model.rowIsEdited(1));
        model.setProperty("attributeName", u"kind"_s);

        // Only the selected nodes, if any
        QVERIFY(execute([this] { _document->selectNeighbours(nodeId(u"d"_s), false); }));
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(value(0, 0).toString(), u"c"_s);
        QVERIFY(execute([this] { _document->selectNone(); }));
        QCOMPARE(model.rowCount(), 5);

        // Each distinct value, with how many nodes have it
        model.setProperty("combineSharedValues", true);
        QCOMPARE(model.rowCount(), 2);

        const int xRow = value(0, 1).toString() == u"x"_s ? 0 : 1;
        QCOMPARE(value(xRow, 0).toInt(), 3);
        QCOMPARE(value(1 - xRow, 0).toInt(), 2);

        model.editValue(xRow, u"x"_s);
        QVERIFY(!model.rowIsEdited(xRow));

        model.editValue(xRow, u"w"_s);
        QVERIFY(model.rowIsEdited(xRow));

        // Applying the edits, as the dialog does
        const auto edits = model.property("edits").value<AttributeEdits>();
        QCOMPARE(edits.nodeValues().size(), 3u);
        QVERIFY(execute([&] { _document->editAttribute(u"kind"_s, edits, ValueType::Unknown, {}); }));

        const auto values = _document->allAttributeValues(u"kind"_s);
        QCOMPARE(values.count(u"w"_s), 3);
        QCOMPARE(values.count(u"y"_s), 2);
    }

    // Finding which column of a table to match with which attribute, then importing it
    void importAttributes()
    {
        QVERIFY(open());

        auto data = std::make_shared<TabularData>();
        const QList<QStringList> rows
        {
            {u"Id"_s,   u"Group"_s,   u"Value"_s},
            {u"a"_s,    u"p"_s,       u"1.5"_s},
            {u"b"_s,    u"q"_s,       u"2.5"_s},
            {u"c"_s,    u"p"_s,       u"3.5"_s},
            {u"z"_s,    u"q"_s,       u"4.5"_s},
        };

        data->reserve(3, static_cast<size_t>(rows.size()));
        for(qsizetype row = 0; row < rows.size(); row++)
        {
            for(qsizetype column = 0; column < 3; column++)
                data->setValueAt(static_cast<size_t>(column), static_cast<size_t>(row), rows.at(row).at(column));
        }

        ImportAttributesKeyDetection keyDetection;
        keyDetection.setProperty("document", QVariant::fromValue(_document.get()));
        keyDetection.setProperty("tabularData", QVariant::fromValue(data));

        QSignalSpy resultChanged(&keyDetection, &ImportAttributesKeyDetection::resultChanged);
        keyDetection.start();
        QVERIFY(QTest::qWaitFor([&] { return !resultChanged.isEmpty() && !keyDetection.busy(); }, Timeout));

        // Three of the five nodes are in the table
        const auto result = keyDetection.property("result").toMap();
        QCOMPARE(result.value(u"attributeName"_s).toString(), u"NodeName"_s);
        QCOMPARE(result.value(u"column"_s).toInt(), 0);
        QCOMPARE(result.value(u"percent"_s).toInt(), 60);

        QVERIFY(execute([&] { _document->importAttributesFromTable(u"NodeName"_s, data, 0, {2}, false); }));
        QVERIFY(_document->attributeExists(u"Value"_s));
        QCOMPARE(graphModel()->attributeValueByName(u"Value"_s).numericValueOf(nodeId(u"c"_s)), 3.5);
        QVERIFY(graphModel()->attributeValueByName(u"Value"_s).valueMissingOf(nodeId(u"d"_s)));

        QVERIFY(execute([this] { _document->undo(); }));
        QVERIFY(!_document->attributeExists(u"Value"_s));

        keyDetection.reset();
        QVERIFY(keyDetection.property("result").toMap().isEmpty());
    }

    // Enrichment results are kept with the Document, and saved with it
    void enrichment()
    {
        QVERIFY(open());

        QVERIFY(execute([this] { _document->performEnrichment(u"kind"_s, u"kind"_s); }));

        auto tableModels = _document->property("enrichmentTableModels").toList();
        QCOMPARE(tableModels.size(), 1);

        auto* tableModel = tableModels.at(0).value<EnrichmentTableModel*>();
        QVERIFY(tableModel != nullptr);
        const auto rowCount = tableModel->rowCount();
        QVERIFY(rowCount > 0);
        QVERIFY(_document->property("saveRequired").toBool());

        auto url = QUrl::fromLocalFile(_dir.filePath(u"enrichment.%1"_s.arg(Application::nativeExtension())));
        QVERIFY(execute([&] { _document->saveFile(url, Application::name(), {}, {}); }));
        QVERIFY(open(url, QString::fromLatin1(Application::NativeFileType), {}));

        // The results are added once loading is complete
        QVERIFY(QTest::qWaitFor([this]
            { return !_document->property("enrichmentTableModels").toList().isEmpty(); }, Timeout));

        tableModels = _document->property("enrichmentTableModels").toList();
        QCOMPARE(tableModels.size(), 1);
        QCOMPARE(tableModels.at(0).value<EnrichmentTableModel*>()->rowCount(), rowCount);

        _document->removeEnrichmentResults(0);
        QVERIFY(_document->property("enrichmentTableModels").toList().isEmpty());
    }

    // What the transform and visualisation panels ask of the Document
    void queries()
    {
        QVERIFY(open());

        auto numerical = _document->availableAttributeNames(static_cast<int>(ElementType::Node),
            static_cast<int>(ValueType::Numerical));
        QVERIFY(numerical.contains(u"score"_s));
        QVERIFY(numerical.contains(u"NodeDegree"_s));
        QVERIFY(!numerical.contains(u"kind"_s));

        // Edges can be filtered by their nodes' attributes
        auto edge = _document->availableAttributeNames(static_cast<int>(ElementType::Edge),
            static_cast<int>(ValueType::All));
        QVERIFY(edge.contains(u"source.kind"_s));
        QVERIFY(edge.contains(u"target.score"_s));

        auto score = _document->attribute(u"score"_s);
        QVERIFY(score.value(u"isValid"_s).toBool());
        QVERIFY(score.value(u"userDefined"_s).toBool());
        QCOMPARE(score.value(u"minimumValue"_s).toDouble(), 1.0);
        QCOMPARE(score.value(u"maximumValue"_s).toDouble(), 5.0);
        QVERIFY(score.value(u"ops"_s).toStringList().contains(u">"_s));

        auto sourceKind = _document->attribute(u"source.kind"_s);
        QCOMPARE(sourceKind.value(u"name"_s).toString(), u"source.kind"_s);
        QCOMPARE(sourceKind.value(u"sharedValues"_s).toStringList(), (QStringList{u"x"_s, u"y"_s}));

        QVERIFY(!_document->attribute(u"Not an attribute"_s).value(u"isValid"_s).toBool());

        auto removeNodes = _document->transform(u"RemoveNodes"_s);
        QVERIFY(removeNodes.value(u"requiresCondition"_s).toBool());
        QVERIFY(!removeNodes.value(u"description"_s).toString().isEmpty());

        auto pageRank = _document->transform(u"PageRank"_s);
        QVERIFY(!pageRank.value(u"requiresCondition"_s).toBool());
        QVERIFY(!pageRank.value(u"defaultVisualisations"_s).toMap().isEmpty());

        QVERIFY(_document->transform(u"Not a transform"_s).isEmpty());

        const auto transform = uR"(RemoveNodes where $score > 3 and $kind == "x")"_s;
        QVERIFY(_document->graphTransformIsValid(transform));
        QVERIFY(!_document->graphTransformIsValid(u"RemoveNodes where"_s));
        auto referenced = _document->attributeNamesReferencedBy(transform);
        referenced.sort();
        QCOMPARE(referenced, (QStringList{u"kind"_s, u"score"_s}));
        QVERIFY(!_document->displayTextForGraphTransform(transform).isEmpty());

        QVERIFY(_document->availableVisualisationChannelNames(static_cast<int>(ElementType::Node),
            static_cast<int>(ValueType::Int)).contains(u"Colour"_s));
        QVERIFY(_document->visualisationIsValid(u"score Size"_s));
        QVERIFY(!_document->visualisationIsValid(u"score Nothing"_s));

        std::unique_ptr<AvailableAttributesModel> model(_document->availableAttributesModel(
            static_cast<int>(ElementType::Node), static_cast<int>(ValueType::Numerical)));
        QVERIFY(model != nullptr);
        QVERIFY(model->rowCount() > 0);
    }

    // Tables, such as those of node attributes or enrichment results, exported to files
    void exportTable_data()
    {
        QTest::addColumn<QString>("extension");
        QTest::addColumn<QString>("expected");

        QTest::newRow("CSV") << u"csv"_s <<
            u"Name,\"Value, in units\"\r\n"
            "plain,1.5\r\n"
            "\"a,b\",42\r\n"
            "\"say \"\"hi\"\"\",\r\n"
            "\"two\nlines\",x\r\n"_s;

        QTest::newRow("TSV") << u"tsv"_s <<
            u"Name\tValue, in units\r\n"
            "plain\t1.5\r\n"
            "a,b\t42\r\n"
            "say \"hi\"\t\r\n"
            "two lines\tx\r\n"_s;
    }

    void exportTable()
    {
        QFETCH(QString, extension);
        QFETCH(const QString, expected);

        QVERIFY(open());

        QStandardItemModel model(0, 2);
        auto appendRow = [&model](const QString& name, const QVariant& value)
        {
            auto* nameItem = new QStandardItem(name);
            auto* valueItem = new QStandardItem;
            valueItem->setData(value, Qt::DisplayRole);
            model.appendRow({nameItem, valueItem});
        };

        appendRow(u"plain"_s, 1.5);
        appendRow(u"a,b"_s, 42);
        appendRow(u"say \"hi\""_s, {});

        // Line breaks would split the row
        appendRow(u"two\nlines"_s, u"x"_s);

        auto fileName = _dir.filePath(u"table.%1"_s.arg(extension));
        QVERIFY(execute([&]
        {
            _document->writeTableModelToFile(&model, QUrl::fromLocalFile(fileName),
                extension, {u"Name"_s, u"Value, in units"_s});
        }));

        QFile file(fileName);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(QString::fromUtf8(file.readAll()), expected);
    }

    std::vector<QVector3D> positions()
    {
        std::vector<QVector3D> v;
        v.reserve(graph().nodeIds().size());
        for(auto nodeId : graph().nodeIds())
            v.push_back(graphModel()->nodePositions().get(nodeId));

        return v;
    }

    // Node positions saved, then loaded again
    void nodePositions()
    {
        QVERIFY(open());

        // Let the layout put the nodes in different places
        QVERIFY(QTest::qWaitFor([this]
        {
            const auto& positions = graphModel()->nodePositions();
            return positions.get(nodeId(u"a"_s)) != positions.get(nodeId(u"c"_s));
        }, Timeout));

        _document->setUserLayoutPaused(true);
        QVERIFY(waitForCommands());

        const auto saved = QUrl::fromLocalFile(_dir.filePath(u"positions.json"_s));
        QVERIFY(execute([&] { _document->saveNodePositionsToFile(saved); }));

        QFile file(saved.toLocalFile());
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto content = parseJsonFrom(file.readAll());
        QVERIFY(content.is_array());
        QCOMPARE(content.size(), 5u);

        // Each node's position as the layout has it, before it's scaled for display
        std::map<QString, QVector3D> savedPositions;
        for(const auto& node : content)
        {
            const auto& position = node["position"];
            savedPositions.emplace(QString::fromStdString(node["name"].get<std::string>()),
                QVector3D(position.at(0).get<float>(), position.at(1).get<float>(), position.at(2).get<float>()));
        }

        QVERIFY(savedPositions.at(u"a"_s) != savedPositions.at(u"c"_s));

        auto displayed = [this](const QString& name) { return graphModel()->nodePositions().get(nodeId(name)); };
        const auto scale = graphModel()->nodePositions().scale();

        // Move a, then load the saved positions, which puts it back
        const auto moved = write(u"moved.json"_s,
            QByteArray::fromStdString(json::array({{{"id", static_cast<int>(nodeId(u"a"_s))},
            {"position", {100.0, 200.0, 300.0}}}}).dump()));
        QVERIFY(execute([&] { _document->loadNodePositionsFromFile(moved); }));
        QCOMPARE(displayed(u"a"_s), QVector3D(100.0f, 200.0f, 300.0f) * scale);

        QVERIFY(execute([&] { _document->loadNodePositionsFromFile(saved); }));

        for(const auto& [name, position] : savedPositions)
            QCOMPARE(displayed(name), position * scale);
    }

    void malformedNodePositions_data()
    {
        QTest::addColumn<QByteArray>("content");

        QTest::newRow("Not JSON") << QByteArray("[");
        QTest::newRow("Not an array") << QByteArray(R"({"id": 0})");
        QTest::newRow("Unknown node") << QByteArray(R"([{"id": 1000, "position": [1, 2, 3]}])");
        QTest::newRow("No position") << QByteArray(R"([{"id": 0}])");
        QTest::newRow("Short position") << QByteArray(R"([{"id": 0, "position": [1, 2]}])");
        QTest::newRow("String id") << QByteArray(R"([{"id": "0", "position": [1, 2, 3]}])");
        QTest::newRow("String coordinate") << QByteArray(R"([{"id": 0, "position": [1, "2", 3]}])");
        QTest::newRow("Null coordinate") << QByteArray(R"([{"id": 0, "position": [1, null, 3]}])");
    }

    // A file that can't be loaded changes nothing
    void malformedNodePositions()
    {
        QFETCH(QByteArray, content);

        QVERIFY(open());

        _document->setUserLayoutPaused(true);
        QVERIFY(waitForCommands());

        const auto before = positions();
        QVERIFY(execute([&] { _document->loadNodePositionsFromFile(write(u"positions.json"_s, content)); }));
        QCOMPARE(positions(), before);
    }
};

QTEST_MAIN(TestDocument_)
#include "tst_document.moc"
