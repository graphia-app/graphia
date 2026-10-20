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
#include "tests/common/testgraphmodel.h"
#include "tests/common/testplugin.h"

#include "app/application.h"
#include "app/headless.h"
#include "app/loading/nativeloader.h"

#include "shared/ui/qml/Graphia/SharedTypes/pairwisecolumntype.h"
#include "shared/utils/json.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>

#include <limits>
#include <memory>

using namespace Qt::Literals::StringLiterals;

// Accepts the data of any version of the real plugins; not in an anonymous namespace,
// as it's the type of one of TestHeadless's members
class AnyVersionPlugin : public TestPlugin
{
    Q_OBJECT

public:
    int dataVersion() const override { return std::numeric_limits<int>::max(); }
};

// Headless mode, i.e. Graphia run with --parameters, which loads files with the real plugins
// and Document, then saves them in the native format, all without a display
class TestHeadless : public QObject
{
    Q_OBJECT

    AnyVersionPlugin _plugin;

    QString write(const QTemporaryDir& dir, const QString& fileName, const QByteArray& content)
    {
        auto filePath = dir.filePath(fileName);
        QFile file(filePath);
        if(!file.open(QIODevice::WriteOnly) || file.write(content) != content.size())
            return {};

        return filePath;
    }

    // Runs headless mode on the files with the parameters, returning whether it finished
    bool run(const QTemporaryDir& dir, const QStringList& filePaths, const json& parameters)
    {
        auto parametersFilePath = write(dir, u"parameters.json"_s, QByteArray::fromStdString(parameters.dump()));
        if(parametersFilePath.isEmpty())
            return false;

        Headless headless(filePaths, parametersFilePath);
        QSignalSpy done(&headless, &Headless::done);
        headless.run();

        const bool finished = QTest::qWaitFor([&done] { return !done.isEmpty(); }, 60000);

        // As main() does when the application quits
        headless.cancel();

        // The Document is deleted later
        QTest::qWait(0);

        return finished;
    }

    // Loads a native file, as Document does
    std::unique_ptr<TestGraphModel> load(const QString& filePath, TestPluginInstance& pluginInstance)
    {
        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly))
            return nullptr;

        QString failureReason;
        auto model = TestGraphModel::parse(file.readAll(), [&pluginInstance](auto*, auto*)
        {
            auto loader = std::make_unique<Loader>();
            loader->setPluginInstance(&pluginInstance);
            return loader;
        }, &failureReason);

        if(model == nullptr)
            qWarning() << "Failed to load" << filePath << failureReason;

        return model;
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();

        // The plugins are built in the build directory, where Graphia's executable is
        Application::setAppDir(QStringLiteral(GRAPHIA_BUILD_DIR));
    }

    void gml()
    {
        const QTemporaryDir dir;
        QVERIFY(dir.isValid());

        auto source = write(dir, u"graph.gml"_s, R"(graph
        [
            directed 1
            node [ id 0 label "a" score 0.5 ]
            node [ id 1 label "b" score 1.5 ]
            node [ id 2 label "c" ]
            edge [ source 0 target 1 ]
            edge [ source 1 target 2 ]
        ])");

        QVERIFY(run(dir, {source}, {{"type", "GML"}, {"plugin", {{"name", "Generic"}}}}));

        // With no destination, the native file is written alongside the source
        auto target = dir.filePath(u"graph.%1"_s.arg(Application::nativeExtension()));
        QVERIFY(QFileInfo::exists(target));

        QVERIFY(Loader::canOpen(QUrl::fromLocalFile(target)));
        QCOMPARE(Loader::pluginNameFor(QUrl::fromLocalFile(target)), u"Generic"_s);

        TestPluginInstance pluginInstance(&_plugin);
        auto model = load(target, pluginInstance);
        QVERIFY(model != nullptr);

        QCOMPARE(model->nodeNames(), (QStringList{u"a"_s, u"b"_s, u"c"_s}));
        QCOMPARE(model->edgeNames(), (QStringList{u"a-b"_s, u"b-c"_s}));
        QCOMPARE(model->nodeNumericValue(u"score"_s, u"b"_s), 1.5);
        QVERIFY(!model->nodeHasValue(u"score"_s, u"c"_s));
        QVERIFY(model->graphModel().directed());
    }

    // The columns are given as the import dialog gives them
    void pairwiseColumns()
    {
        const QTemporaryDir dir;
        QVERIFY(dir.isValid());

        auto source = write(dir, u"edges.csv"_s, "From,To,Weight\na,b,1.5\nb,c,2\n");

        const json columns =
        {
            {"0", {{"type", static_cast<int>(PairwiseColumnType::SourceNode)}}},
            {"1", {{"type", static_cast<int>(PairwiseColumnType::TargetNode)}}},
            {"2", {{"type", static_cast<int>(PairwiseColumnType::EdgeAttribute)}, {"name", "Weight"}}}
        };

        QVERIFY(run(dir, {source}, {{"type", "PairwiseCSV"}, {"plugin", {{"name", "Generic"},
            {"parameters", {{"firstRowIsHeader", "true"}, {"columns", columns}}}}}}));

        TestPluginInstance pluginInstance(&_plugin);
        auto model = load(dir.filePath(u"edges.%1"_s.arg(Application::nativeExtension())), pluginInstance);
        QVERIFY(model != nullptr);

        QCOMPARE(model->edgeNames(), (QStringList{u"a-b"_s, u"b-c"_s}));
        QCOMPARE(model->edgeNumericValue(u"Weight"_s, u"a"_s, u"b"_s), 1.5);
    }

    void correlation()
    {
        const QTemporaryDir dir;
        QVERIFY(dir.isValid());

        // r and s are perfectly correlated, t is anticorrelated with both, u with neither
        auto source = write(dir, u"data.csv"_s,
            "Name,c1,c2,c3,c4,c5\n"
            "r,1,2,3,4,5\n"
            "s,2,4,6,8,10\n"
            "t,5,4,3,2,1\n"
            "u,1,5,1,5,1\n");

        const json parameters =
        {
            {"type", "CorrelationCSV"},
            {"plugin",
            {
                {"name", "Correlation"},
                {"parameters",
                {
                    {"minimumThreshold", 0.9},
                    {"initialThreshold", 0.9}
                }}
            }}
        };

        QVERIFY(run(dir, {source}, parameters));

        auto target = dir.filePath(u"data.%1"_s.arg(Application::nativeExtension()));
        QVERIFY(QFileInfo::exists(target));
        QCOMPARE(Loader::pluginNameFor(QUrl::fromLocalFile(target)), u"Correlation"_s);

        TestPluginInstance pluginInstance(&_plugin);
        auto model = load(target, pluginInstance);
        QVERIFY(model != nullptr);

        QCOMPARE(model->nodeNames(), (QStringList{u"r"_s, u"s"_s, u"t"_s, u"u"_s}));
        QCOMPARE(model->edgeNames().size(), 1);
        QVERIFY(model->edgeNames().contains(u"r-s"_s) || model->edgeNames().contains(u"s-r"_s));
    }

    void failures()
    {
        const QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto source = write(dir, u"edges.csv"_s, "a,b\n");

        // A file that doesn't exist
        QVERIFY(run(dir, {dir.filePath(u"missing.csv"_s)}, {{"type", "PairwiseCSV"}, {"plugin", {{"name", "Generic"}}}}));
        QVERIFY(!QFileInfo::exists(dir.filePath(u"missing.%1"_s.arg(Application::nativeExtension()))));

        // A plugin that doesn't exist
        QVERIFY(run(dir, {source}, {{"type", "PairwiseCSV"}, {"plugin", {{"name", "Nothing"}}}}));

        // Malformed parameters
        QVERIFY(run(dir, {source}, {{"plugin", {{"name", "Generic"}}}}));
        QVERIFY(run(dir, {source}, {{"type", "PairwiseCSV"}, {"plugin", json::object()}}));
        QVERIFY(run(dir, {source}, {{"type", "Native"}, {"plugin", {{"name", "Generic"}}}}));

        // Several files must go to a directory
        QVERIFY(run(dir, {source, source}, {{"type", "PairwiseCSV"}, {"plugin", {{"name", "Generic"}}}}));

        // No files
        QVERIFY(run(dir, {}, {{"type", "PairwiseCSV"}, {"plugin", {{"name", "Generic"}}}}));

        QVERIFY(!QFileInfo::exists(dir.filePath(u"edges.%1"_s.arg(Application::nativeExtension()))));
    }
};

QTEST_MAIN(TestHeadless)
#include "tst_headless.moc"
