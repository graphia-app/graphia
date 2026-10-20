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

#include "app/layout/forcedirectedlayout.h"
#include "app/layout/layoutthread.h"
#include "app/layout/nodepositions.h"
#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"

#include <QElapsedTimer>
#include <QTest>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QVector3D>

#include <cmath>
#include <map>
#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
QStringList path(const QString& prefix, int numNodes)
{
    QStringList edges;
    edges.reserve(numNodes - 1);
    for(int i = 0; i < numNodes - 1; i++)
        edges.append(u"%1%2-%1%3"_s.arg(prefix).arg(i).arg(i + 1));

    return edges;
}

bool finite(const QVector3D& v)
{
    return std::isfinite(v.x()) && std::isfinite(v.y()) && std::isfinite(v.z());
}

const int TIMEOUT = 60000;
} // namespace

class TestLayoutThread : public QObject
{
    Q_OBJECT

    std::unique_ptr<TestGraphModel> _model;
    std::unique_ptr<LayoutThread> _layoutThread;

    void create(const QStringList& elements)
    {
        _model = std::make_unique<TestGraphModel>(elements);
        _model->applyTransforms({});

        _layoutThread = std::make_unique<LayoutThread>(_model->graphModel(),
            std::make_unique<ForceDirectedLayoutFactory>(&_model->graphModel()));
    }

    QVector3D position(const QString& name) const
    {
        return _model->graphModel().nodePositions().get(_model->nodeId(name));
    }

    bool waitUntilFinished()
    {
        return QTest::qWaitFor([this] { return _layoutThread->paused() && _layoutThread->finished(); }, TIMEOUT);
    }

    // Each node of the transformed graph has a position other than where it started
    bool allPositioned() const
    {
        const auto& graph = _model->graph();
        return std::ranges::all_of(graph.nodeIds(), [this](NodeId nodeId)
        {
            auto p = _model->graphModel().nodePositions().get(nodeId);
            return finite(p) && !p.isNull();
        });
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void cleanup()
    {
        // The thread refers to the model, so must go first
        _layoutThread.reset();
        _model.reset();
    }

    void runsUntilFinished()
    {
        create(path(u"n"_s, 10));

        const QSignalSpy initialised(_layoutThread.get(), &LayoutThread::initialised);
        const QSignalSpy executed(_layoutThread.get(), &LayoutThread::executed);

        _layoutThread->start(false);
        QVERIFY(waitUntilFinished());

        QCOMPARE(initialised.count(), 1);
        QVERIFY(!executed.isEmpty());
        QVERIFY(allPositioned());

        // A path is laid out with its ends far apart
        QVERIFY(position(u"n0"_s).distanceToPoint(position(u"n9"_s)) >
            position(u"n0"_s).distanceToPoint(position(u"n1"_s)) * 3.0f);

        // There's nothing to do, so resuming does nothing
        _layoutThread->resume();
        QVERIFY(_layoutThread->paused());
    }

    void twoDee()
    {
        create(path(u"n"_s, 10));

        _layoutThread->setDimensionalityMode(Layout::Dimensionality::TwoDee);
        QCOMPARE(_layoutThread->dimensionalityMode(), Layout::Dimensionality::TwoDee);

        _layoutThread->start(false);
        QVERIFY(waitUntilFinished());
        QVERIFY(allPositioned());

        for(auto nodeId : _model->graph().nodeIds())
            QCOMPARE(_model->graphModel().nodePositions().get(nodeId).z(), 0.0f);
    }

    void startPaused()
    {
        create(path(u"n"_s, 10));

        const QSignalSpy pausedChanged(_layoutThread.get(), &LayoutThread::pausedChanged);
        QSignalSpy initialised(_layoutThread.get(), &LayoutThread::initialised);
        _layoutThread->start(true);

        // It's initialised once it has paused, before doing anything
        QVERIFY(initialised.wait(TIMEOUT));
        QVERIFY(_layoutThread->paused());

        // Nothing is laid out until resumed
        QVERIFY(!_layoutThread->finished());
        QCOMPARE(position(u"n5"_s), QVector3D());

        _layoutThread->resume();
        QVERIFY(waitUntilFinished());
        QVERIFY(allPositioned());
        QVERIFY(!pausedChanged.isEmpty());
    }

    void pauseWhileRunning()
    {
        create(path(u"n"_s, 200));

        _layoutThread->start(false);
        QVERIFY(QTest::qWaitFor([this] { return !_layoutThread->paused(); }, TIMEOUT));

        _layoutThread->pauseAndWait();
        QVERIFY(_layoutThread->paused());

        // Once paused, nothing moves
        const auto before = position(u"n100"_s);
        QTest::qWait(50);
        QCOMPARE(position(u"n100"_s), before);

        _layoutThread->resume();
        QVERIFY(waitUntilFinished());
    }

    void setNodePositions()
    {
        create(path(u"n"_s, 3));

        _layoutThread->start(false);
        QVERIFY(waitUntilFinished());

        ExactNodePositions positions(_model->graph());
        positions.set(_model->nodeId(u"n0"_s), {1.0f, 2.0f, 3.0f});
        positions.set(_model->nodeId(u"n1"_s), {4.0f, 5.0f, 6.0f});
        positions.set(_model->nodeId(u"n2"_s), {7.0f, 8.0f, 9.0f});

        _layoutThread->setNodePositions(positions);

        // The positions are published, scaled as the layout scales them
        const auto scale = _model->graphModel().nodePositions().scale();
        QCOMPARE(position(u"n1"_s), QVector3D(4.0f, 5.0f, 6.0f) * scale);
    }

    void componentsAddedAndRemoved()
    {
        create(path(u"a"_s, 5));

        _layoutThread->start(false);
        QVERIFY(waitUntilFinished());

        // As Document does when the graph changes
        _layoutThread->invalidate();
        _model->graphModel().mutableGraph().performTransaction([](IMutableGraph& graph)
        {
            auto previous = graph.addNode();
            for(int i = 0; i < 4; i++)
            {
                auto next = graph.addNode();
                graph.addEdge(previous, next);
                previous = next;
            }
        });

        QCOMPARE(_model->graph().numComponents(), 2);

        _layoutThread->resume();
        QVERIFY(waitUntilFinished());
        QVERIFY(allPositioned());

        // Remove the first component, while the layout is running
        _layoutThread->invalidate();
        const auto a0 = _model->nodeId(u"a0"_s);
        _model->graphModel().mutableGraph().performTransaction([a0](IMutableGraph& graph)
        {
            graph.removeNode(a0);
        });

        _layoutThread->resume();
        QVERIFY(waitUntilFinished());
        QCOMPARE(_model->graph().numComponents(), 2);
        QVERIFY(allPositioned());
    }

    void settings()
    {
        create(path(u"n"_s, 3));

        QCOMPARE(_layoutThread->layoutName(), u"ForceDirected"_s);
        QVERIFY(!_layoutThread->layoutDisplayName().isEmpty());
        QCOMPARE(_layoutThread->settings().size(), 2u);

        const QSignalSpy settingChanged(_layoutThread.get(), &LayoutThread::settingChanged);

        _layoutThread->setSettingValue(u"LongRangeRepulseTerm"_s, 5.0f);
        QCOMPARE(_layoutThread->setting(u"LongRangeRepulseTerm"_s)->value(), 5.0f);
        QCOMPARE(settingChanged.count(), 1);
        QCOMPARE(settingChanged.at(0).at(0).toString(), u"LongRangeRepulseTerm"_s);

        _layoutThread->setSettingNormalisedValue(u"LongRangeRepulseTerm"_s, 1.0f);
        QCOMPARE(_layoutThread->setting(u"LongRangeRepulseTerm"_s)->value(), 20.0f);

        _layoutThread->resetSettingValue(u"LongRangeRepulseTerm"_s);
        QCOMPARE(_layoutThread->setting(u"LongRangeRepulseTerm"_s)->value(), 10.0f);
        QCOMPARE(settingChanged.count(), 3);

        // Changing a setting after the layout has finished means there's more to do
        _layoutThread->start(false);
        QVERIFY(waitUntilFinished());

        _layoutThread->setSettingValue(u"LongRangeRepulseTerm"_s, 2.0f);
        QVERIFY(!_layoutThread->finished());

        _layoutThread->resume();
        QVERIFY(waitUntilFinished());
    }

    // Visual updates take the positions' lock to place text, as does the layout thread when it
    // publishes positions, so they must take them in the same order as each other, or deadlock
    void visualUpdatesWhileRunning()
    {
        create(path(u"n"_s, 2000));

        // Text visuals make the time each thread holds the locks long enough to collide
        std::map<QString, QString> groups;
        for(int i = 0; i < 2000; i++)
            groups.emplace(u"n%1"_s.arg(i), u"g%1"_s.arg(i / 4));

        _model->addNodeAttribute(u"Group"_s, groups);
        const QStringList visualisations{uR"("Group" "Shared Text")"_s};
        const QStringList colouredVisualisations{uR"("Group" "Shared Text")"_s, uR"("Group" "Colour")"_s};
        _model->graphModel().buildVisualisations(visualisations);
        _model->graphModel().enableVisualUpdates();

        _layoutThread->start(false);
        QVERIFY(QTest::qWaitFor([this] { return !_layoutThread->paused(); }, TIMEOUT));

        QElapsedTimer timer;
        timer.start();

        // Each update changes the visuals, which places the text
        for(int i = 0; timer.elapsed() < 5000; i++)
        {
            // Keep the layout running
            if(_layoutThread->paused())
            {
                _layoutThread->setSettingValue(u"LongRangeRepulseTerm"_s, (i % 2 == 0) ? 5.0f : 10.0f);
                _layoutThread->resume();
            }

            _model->graphModel().buildVisualisations((i % 2 == 0) ? colouredVisualisations : visualisations);
            _model->graphModel().applyPendingVisualUpdates();
        }

        _layoutThread->pauseAndWait();
    }

    void stopWithoutStarting()
    {
        // Destroying a thread that was never started mustn't hang
        create(path(u"n"_s, 3));
    }
};

QTEST_MAIN(TestLayoutThread)
#include "tst_layoutthread.moc"
