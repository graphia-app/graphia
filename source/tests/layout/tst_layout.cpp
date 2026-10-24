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

#include "app/layout/barneshuttree.h"
#include "app/layout/centreinglayout.h"
#include "app/layout/circlepackcomponentlayout.h"
#include "app/layout/collision.h"
#include "app/layout/fastinitiallayout.h"
#include "app/layout/forcedirectedlayout.h"
#include "app/layout/layoutsettings.h"
#include "app/layout/nodepositions.h"
#include "app/layout/powerof2gridcomponentlayout.h"
#include "app/layout/randomlayout.h"
#include "app/layout/scalinglayout.h"

#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"
#include "app/ui/selectionmanager.h"

#include <QTest>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QVector3D>

#include <cmath>
#include <map>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

using namespace Qt::Literals::StringLiterals;

static bool finite(const QVector3D& v)
{
    return std::isfinite(v.x()) && std::isfinite(v.y()) && std::isfinite(v.z());
}

// "n0-n1", "n1-n2", ...
static QStringList path(int numNodes)
{
    QStringList edges;
    edges.reserve(numNodes - 1);
    for(int i = 0; i < numNodes - 1; i++)
        edges.append(u"n%1-n%2"_s.arg(i).arg(i + 1));

    return edges;
}

// A complete binary tree
static QStringList binaryTree(int numNodes)
{
    QStringList edges;
    edges.reserve(numNodes - 1);
    for(int i = 1; i < numNodes; i++)
        edges.append(u"n%1-n%2"_s.arg((i - 1) / 2).arg(i));

    return edges;
}

// Positions, all distinct, spread over a few clusters
static std::vector<QVector3D> testPositions(size_t numPositions, bool twoDee)
{
    std::vector<QVector3D> positions;

    for(size_t i = 0; i < numPositions; i++)
    {
        const auto f = static_cast<float>(i);
        const float cluster = static_cast<float>(i % 3) * 50.0f;

        positions.emplace_back(cluster + (std::sin(f * 1.7f) * 10.0f),
            -cluster + (std::cos(f * 2.3f) * 10.0f),
            twoDee ? 0.0f : (std::sin(f * 0.9f) * 10.0f));
    }

    return positions;
}

class TestLayout : public QObject
{
    Q_OBJECT

    struct Model
    {
        std::unique_ptr<TestGraphModel> _model;
        std::unique_ptr<NodeLayoutPositions> _positions;

        explicit Model(const QStringList& elements) :
            _model(std::make_unique<TestGraphModel>(elements))
        {
            _model->applyTransforms({});
            _positions = std::make_unique<NodeLayoutPositions>(graph());
        }

        const Graph& graph() const { return _model->graph(); }
        GraphModel& graphModel() const { return _model->graphModel(); }
        NodeId nodeId(const QString& name) const { return _model->nodeId(name); }
        const IGraphComponent& component() const
        {
            return *graph().componentById(graph().componentIdOfLargestComponent());
        }

        QVector3D position(const QString& name) const { return _positions->get(nodeId(name)); }
        float distance(const QString& a, const QString& b) const
        {
            return position(a).distanceToPoint(position(b));
        }
    };

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void nodePositions()
    {
        Model m({u"a-b"_s});
        auto& positions = *m._positions;
        auto a = m.nodeId(u"a"_s);
        auto b = m.nodeId(u"b"_s);

        // Nodes start at the origin
        QCOMPARE(positions.get(a), QVector3D());

        positions.set(a, {2.0f, 4.0f, 6.0f});
        positions.set(b, {4.0f, 0.0f, -2.0f});
        QCOMPARE(positions.get(a), QVector3D(2.0f, 4.0f, 6.0f));
        QCOMPARE(positions.centreOfMass({a, b}), QVector3D(3.0f, 2.0f, 2.0f));

        const auto box = positions.boundingBox({a, b});
        QCOMPARE(box.min(), QVector3D(2.0f, 0.0f, -2.0f));
        QCOMPARE(box.max(), QVector3D(4.0f, 4.0f, 6.0f));
        QCOMPARE(positions.boundingBox({}).volume(), 0.0f);

        // NodePositions' view is scaled and smoothed
        const NodePositions& smoothed = positions;
        positions.setScale(2.0f);
        QCOMPARE(smoothed.get(a), QVector3D(4.0f, 8.0f, 12.0f));
        QCOMPARE(positions.get(a), QVector3D(2.0f, 4.0f, 6.0f));
        QCOMPARE(smoothed.centreOfMass({a, b}), QVector3D(6.0f, 4.0f, 4.0f));

        positions.setScale(1.0f);
        positions.setSmoothing(2);
        positions.set(a, {4.0f, 0.0f, 0.0f});
        QCOMPARE(smoothed.get(a), QVector3D(3.0f, 2.0f, 3.0f));
        QCOMPARE(smoothed.get(std::vector<NodeId>{a, b}),
            (std::vector<QVector3D>{{3.0f, 2.0f, 3.0f}, {2.0f, 0.0f, -1.0f}}));

        // Setting from exact positions fills the history, so there's nothing to smooth
        ExactNodePositions exact(m.graph());
        exact.set(a, {1.0f, 1.0f, 1.0f});
        positions.set(std::vector<NodeId>{a}, exact);
        QCOMPARE(smoothed.get(a), QVector3D(1.0f, 1.0f, 1.0f));

        positions.flatten();
        QCOMPARE(smoothed.get(a), QVector3D(1.0f, 1.0f, 0.0f));
        QCOMPARE(smoothed.get(b), QVector3D(2.0f, 0.0f, 0.0f));

        NodePositions copy(m.graph());
        copy.update(positions);
        QCOMPARE(std::as_const(copy).at(a), QVector3D(1.0f, 1.0f, 0.0f));
        QVERIFY(copy.unlocked());
    }

    void layoutSettings()
    {
        LayoutSettings settings;
        settings.registerSetting(u"Linear"_s, u"Linear"_s, u"A linear setting"_s, 0.0f, 20.0f, 10.0f);
        settings.registerSetting(u"Log"_s, u"Log"_s, u"A log setting"_s,
            1000.0f, 1000000000.0f, 1000000.0f, LayoutSettingScaleType::Log);

        const QSignalSpy spy(&settings, &LayoutSettings::settingChanged);

        QCOMPARE(settings.value(u"Linear"_s), 10.0f);
        QCOMPARE(settings.normalisedValue(u"Linear"_s), 0.5f);
        QCOMPARE(settings.normalisedValue(u"Log"_s), 0.5f);

        // Values are clamped
        settings.setValue(u"Linear"_s, 30.0f);
        QCOMPARE(settings.value(u"Linear"_s), 20.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), u"Linear"_s);
        QCOMPARE(spy.at(0).at(1).toFloat(), 20.0f);

        // Setting the same value again changes nothing
        settings.setValue(u"Linear"_s, 25.0f);
        QCOMPARE(spy.count(), 1);

        settings.setNormalisedValue(u"Linear"_s, 0.25f);
        QCOMPARE(settings.value(u"Linear"_s), 5.0f);

        settings.setNormalisedValue(u"Log"_s, 0.0f);
        QCOMPARE(settings.value(u"Log"_s), 1000.0f);
        settings.setNormalisedValue(u"Log"_s, 1.0f / 6.0f);
        QVERIFY(qFuzzyCompare(settings.value(u"Log"_s), 10000.0f));

        settings.resetValue(u"Linear"_s);
        QCOMPARE(settings.value(u"Linear"_s), 10.0f);
        QCOMPARE(spy.count(), 5);

        const auto* setting = settings.setting(u"Log"_s);
        QVERIFY(setting != nullptr);
        QCOMPARE(setting->displayName(), u"Log"_s);
        QCOMPARE(setting->defaultValue(), 1000000.0f);
        QCOMPARE(setting->range(), 1000000000.0f - 1000.0f);

        // Unknown settings are ignored
        QTest::ignoreMessage(QtDebugMsg, QRegularExpression(u"not found"_s));
        QCOMPARE(settings.value(u"Unknown"_s), 0.0f);
        QTest::ignoreMessage(QtDebugMsg, QRegularExpression(u"not found"_s));
        settings.setValue(u"Unknown"_s, 1.0f);
        QCOMPARE(spy.count(), 5);
    }

    void simpleLayouts()
    {
        Model m(path(4));
        auto& positions = *m._positions;
        const auto& component = m.component();

        RandomLayout random(component, positions);
        random.setSpread(5.0f);
        QVERIFY(random.execute(true, Layout::Dimensionality::ThreeDee));
        QVERIFY(!random.iterative());

        for(auto nodeId : component.nodeIds())
        {
            const auto& p = positions.get(nodeId);
            QVERIFY(std::abs(p.x()) <= 5.0f && std::abs(p.y()) <= 5.0f && std::abs(p.z()) <= 5.0f);
        }

        const auto before = positions.get(m.nodeId(u"n1"_s));
        ScalingLayout scaling(component, positions);
        scaling.setScale(3.0f);
        scaling.execute(true, Layout::Dimensionality::ThreeDee);
        QCOMPARE(positions.get(m.nodeId(u"n1"_s)), before * 3.0f);

        CentreingLayout centreing(component, positions);
        centreing.execute(true, Layout::Dimensionality::ThreeDee);
        QVERIFY(positions.centreOfMass(component.nodeIds()).length() < 1e-4f);
    }

    void fastInitialLayout_data()
    {
        QTest::addColumn<QStringList>("edges");

        QTest::newRow("Edge") << path(2);
        QTest::newRow("Path") << path(10);
        QTest::newRow("Binary tree") << binaryTree(40);

        QStringList star;
        star.reserve(29);
        for(int i = 1; i < 30; i++)
            star.append(u"n0-n%1"_s.arg(i));
        QTest::newRow("Star") << star;

        // Stars hanging off a path, so non-root nodes with many children
        QStringList stars = path(4);
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 8; j++)
                stars.append(u"n%1-s%1_%2"_s.arg(i).arg(j));
        }
        QTest::newRow("Stars") << stars;
    }

    void fastInitialLayout()
    {
        QFETCH(const QStringList, edges);

        for(auto dimensionality : {Layout::Dimensionality::ThreeDee, Layout::Dimensionality::TwoDee})
        {
            Model m(edges);
            auto& positions = *m._positions;
            const auto& component = m.component();

            FastInitialLayout layout(component, positions);
            QVERIFY(layout.execute(true, dimensionality));

            // As the graphs are trees, every edge is part of the spanning tree the
            // layout follows, and so has the same length
            for(auto edgeId : component.edgeIds())
            {
                const auto& edge = m.graph().edgeById(edgeId);
                const auto& source = positions.get(edge.sourceId());
                const auto& target = positions.get(edge.targetId());

                QVERIFY(finite(source));
                QVERIFY(finite(target));

                if(dimensionality == Layout::Dimensionality::ThreeDee)
                    QVERIFY(std::abs(source.distanceToPoint(target) - 20.0f) < 1e-3f);
                else
                {
                    QCOMPARE(source.z(), 0.0f);
                    QCOMPARE(target.z(), 0.0f);
                }
            }

            // No two siblings are in the same place
            for(auto nodeId : component.nodeIds())
            {
                const auto neighbourIds = m.graph().neighboursOf(nodeId);

                std::vector<QVector3D> neighbours;
                neighbours.reserve(neighbourIds.size());
                for(auto neighbourId : neighbourIds)
                    neighbours.push_back(positions.get(neighbourId));

                for(size_t i = 0; i < neighbours.size(); i++)
                {
                    for(size_t j = i + 1; j < neighbours.size(); j++)
                    {
                        if(dimensionality == Layout::Dimensionality::ThreeDee)
                            QVERIFY(neighbours.at(i).distanceToPoint(neighbours.at(j)) > 1.0f);
                    }
                }
            }
        }
    }

    void forceDirectedLayout_data()
    {
        QTest::addColumn<Layout::Dimensionality>("dimensionality");

        QTest::newRow("3D") << Layout::Dimensionality::ThreeDee;
        QTest::newRow("2D") << Layout::Dimensionality::TwoDee;
    }

    void forceDirectedLayout()
    {
        QFETCH(const Layout::Dimensionality, dimensionality);

        Model m(path(12));
        auto& positions = *m._positions;

        ForceDirectedLayoutFactory factory(&m.graphModel());
        QCOMPARE(factory.name(), u"ForceDirected"_s);
        QVERIFY(factory.setting(u"ShortRangeRepulseTerm"_s) != nullptr);
        QVERIFY(factory.setting(u"LongRangeRepulseTerm"_s) != nullptr);

        auto layout = factory.create(m.graph().componentIdOfLargestComponent(), positions, dimensionality);
        QVERIFY(layout->iterative());

        const int MAX_ITERATIONS = 5000;
        int iteration = 0;
        for(; iteration < MAX_ITERATIONS && !layout->finished(); iteration++)
            layout->execute(iteration == 0, dimensionality);

        QVERIFY2(layout->finished(), "Layout didn't settle");

        for(auto nodeId : m.component().nodeIds())
        {
            QVERIFY(finite(positions.get(nodeId)));

            if(dimensionality == Layout::Dimensionality::TwoDee)
                QCOMPARE(positions.get(nodeId).z(), 0.0f);
        }

        // A path should be laid out more or less straight, with its ends far apart
        float maxEdgeLength = 0.0f;
        for(int i = 0; i < 11; i++)
            maxEdgeLength = std::max(maxEdgeLength, m.distance(u"n%1"_s.arg(i), u"n%1"_s.arg(i + 1)));

        QVERIFY(maxEdgeLength > 0.0f);
        QVERIFY2(m.distance(u"n0"_s, u"n11"_s) > 5.0f * maxEdgeLength,
            qPrintable(u"Ends %1 apart, edges up to %2"_s.arg(m.distance(u"n0"_s, u"n11"_s)).arg(maxEdgeLength)));

        // Once settled, it stays settled
        layout->execute(false, dimensionality);
        QVERIFY(layout->finished());

        // Until told otherwise
        layout->unfinish();
        QVERIFY(!layout->finished());
    }

    void barnesHutTree_data()
    {
        QTest::addColumn<bool>("twoDee");
        QTest::addColumn<float>("theta");
        QTest::addColumn<float>("tolerance");

        // With a theta of 0, every sub volume is opened, so the result is exact
        QTest::newRow("3D exact") << false << 0.0f << 1e-4f;
        QTest::newRow("2D exact") << true << 0.0f << 1e-4f;
        // Theta is compared with the square of the ratio of a sub volume's size
        // to its distance, so 0.8 opens those whose ratio is more than about 0.9
        QTest::newRow("3D") << false << 0.8f << 0.2f;
        QTest::newRow("2D") << true << 0.8f << 0.2f;
        QTest::newRow("3D small theta") << false << 0.05f << 0.02f;
        QTest::newRow("2D small theta") << true << 0.05f << 0.02f;
    }

    void barnesHutTree()
    {
        QFETCH(const bool, twoDee);
        QFETCH(const float, theta);
        QFETCH(const float, tolerance);

        const int numNodes = 60;
        QStringList nodes;
        nodes.reserve(numNodes);
        for(int i = 0; i < numNodes; i++)
            nodes.append(u"n%1"_s.arg(i));

        // The positions must all be in one component
        Model m(path(numNodes));
        auto& positions = *m._positions;
        const auto& component = m.component();
        QCOMPARE(component.numNodes(), static_cast<size_t>(numNodes));

        auto testPosition = testPositions(numNodes, twoDee);
        for(int i = 0; i < numNodes; i++)
            positions.set(m.nodeId(nodes.at(i)), testPosition.at(static_cast<size_t>(i)));

        std::unique_ptr<AbstractBarnesHutTree> tree;
        if(twoDee)
        {
            auto tree2D = std::make_unique<BarnesHutTree2D>();
            tree2D->setTheta(theta);
            tree = std::move(tree2D);
        }
        else
        {
            auto tree3D = std::make_unique<BarnesHutTree3D>();
            tree3D->setTheta(theta);
            tree = std::move(tree3D);
        }

        tree->build(component, positions);

        // An inverse square repulsion, with the mass in w
        auto kernel = [](int mass, const QVector3D& difference, float distanceSq)
        {
            return -difference * (static_cast<float>(mass) / (distanceSq * std::sqrt(distanceSq)));
        };

        for(auto nodeId : component.nodeIds())
        {
            // The forces from the other nodes largely cancel, so the error is
            // relative to their total magnitude rather than to their sum
            QVector3D expected;
            float magnitude = 0.0f;
            for(auto otherNodeId : component.nodeIds())
            {
                if(otherNodeId == nodeId)
                    continue;

                auto difference = positions.get(otherNodeId) - positions.get(nodeId);
                auto force = kernel(1, difference, difference.lengthSquared());
                expected += force;
                magnitude += force.length();
            }

            auto actual = tree->evaluateKernel(positions, nodeId, kernel);
            QVERIFY2((actual - expected).length() <= tolerance * magnitude,
                qPrintable(u"(%1, %2, %3) != (%4, %5, %6)"_s
                    .arg(actual.x()).arg(actual.y()).arg(actual.z())
                    .arg(expected.x()).arg(expected.y()).arg(expected.z())));

            // The total mass is the number of other nodes
            auto mass = tree->evaluateKernel(positions, nodeId,
                [](int count, const QVector3D&, float) { return QVector3D(static_cast<float>(count), 0.0f, 0.0f); });

            if(theta == 0.0f)
                QCOMPARE(mass.x(), static_cast<float>(numNodes - 1));
        }
    }

    void barnesHutTreeCoincidentNodes()
    {
        Model m(path(6));
        auto& positions = *m._positions;
        const auto& component = m.component();

        // Three nodes in the same place, which can't be separated into leaves of their own
        positions.set(m.nodeId(u"n0"_s), {0.0f, 0.0f, 0.0f});
        positions.set(m.nodeId(u"n1"_s), {10.0f, 10.0f, 10.0f});
        positions.set(m.nodeId(u"n2"_s), {10.0f, 10.0f, 10.0f});
        positions.set(m.nodeId(u"n3"_s), {10.0f, 10.0f, 10.0f});
        positions.set(m.nodeId(u"n4"_s), {-5.0f, 3.0f, 1.0f});
        positions.set(m.nodeId(u"n5"_s), {2.0f, -7.0f, 4.0f});

        BarnesHutTree3D tree;
        tree.setTheta(0.0f);
        tree.build(component, positions);

        for(auto nodeId : component.nodeIds())
        {
            auto mass = tree.evaluateKernel(positions, nodeId,
                [](int count, const QVector3D&, float) { return QVector3D(static_cast<float>(count), 0.0f, 0.0f); });
            QCOMPARE(mass.x(), 5.0f);
        }
    }

    void componentLayouts_data()
    {
        QTest::addColumn<QString>("type");

        QTest::newRow("Circle pack") << u"CirclePack"_s;
        QTest::newRow("Power of 2 grid") << u"PowerOf2Grid"_s;
    }

    void componentLayouts()
    {
        QFETCH(const QString, type);

        // Components of various sizes
        QStringList elements;
        const std::vector<int> sizes{1, 1, 2, 3, 3, 5, 8, 13, 21, 34, 2, 1, 50, 4, 4, 4, 7, 1, 1, 1};
        elements.reserve(std::accumulate(sizes.begin(), sizes.end(), 0));
        for(size_t c = 0; c < sizes.size(); c++)
        {
            const int size = sizes.at(c);
            if(size == 1)
                elements.append(u"c%1n0"_s.arg(c));

            for(int i = 0; i < size - 1; i++)
                elements.append(u"c%1n%2-c%1n%3"_s.arg(c).arg(i).arg(i + 1));
        }

        const Model m(elements);
        const auto& graph = m.graph();
        const auto& componentIds = graph.componentIds();
        QCOMPARE(componentIds.size(), sizes.size());

        std::unique_ptr<ComponentLayout> layout;
        if(type == u"CirclePack"_s)
            layout = std::make_unique<CirclePackComponentLayout>();
        else
            layout = std::make_unique<PowerOf2GridComponentLayout>();

        ComponentLayoutData data(graph);
        layout->execute(graph, componentIds, data);

        // The layout starts at 0, 0
        const auto boundingBox = layout->boundingBoxFor(componentIds, data);
        QVERIFY(std::abs(boundingBox.left()) < 1e-3);
        QVERIFY(std::abs(boundingBox.top()) < 1e-3);

        for(auto componentId : componentIds)
        {
            const auto& circle = data[componentId];
            QVERIFY(std::isfinite(circle.x()) && std::isfinite(circle.y()));
            QVERIFY(circle.radius() > 0.0f);
        }

        // Bigger components get circles at least as big
        for(auto a : componentIds)
        {
            for(auto b : componentIds)
            {
                if(graph.componentById(a)->numNodes() > graph.componentById(b)->numNodes())
                    QVERIFY(data[a].radius() >= data[b].radius());
            }
        }

        // No two components overlap
        for(size_t i = 0; i < componentIds.size(); i++)
        {
            for(size_t j = i + 1; j < componentIds.size(); j++)
            {
                const auto& a = data[componentIds.at(i)];
                const auto& b = data[componentIds.at(j)];

                QVERIFY2(a.distanceTo(b) > -1e-2f, qPrintable(u"%1 and %2 overlap by %3"_s
                    .arg(i).arg(j).arg(-a.distanceTo(b))));
            }
        }
    }

    // Picking nodes with a ray from the origin looking along -z: a is on the ray, b is a little to the
    // side of it and nearer the origin, and c is on the ray but behind the origin
    void collision_data()
    {
        QTest::addColumn<float>("bOffset");
        QTest::addColumn<float>("radius");
        QTest::addColumn<QStringList>("inside");
        QTest::addColumn<QString>("nearest");

        // Offsets and radii are in multiples of the size of a node
        QTest::newRow("b touches the ray") << 0.5F << 0.0F << QStringList{u"a"_s, u"b"_s} << u"b"_s;
        QTest::newRow("b misses the ray") << 2.0F << 0.0F << QStringList{u"a"_s} << u"a"_s;
        QTest::newRow("b inside the cylinder") << 3.0F << 2.5F << QStringList{u"a"_s, u"b"_s} << u"b"_s;
        QTest::newRow("b outside the cylinder") << 3.0F << 1.5F << QStringList{u"a"_s} << u"a"_s;
    }

    void collision()
    {
        QFETCH(const float, bOffset);
        QFETCH(const float, radius);
        QFETCH(const QStringList, inside);
        QFETCH(const QString, nearest);

        TestGraphModel model({u"a-b"_s, u"b-c"_s});
        model.applyTransforms({});
        auto& graphModel = model.graphModel();
        graphModel.enableVisualUpdates();
        graphModel.buildVisualisations({});
        graphModel.applyPendingVisualUpdates();

        const auto size = graphModel.nodeVisual(model.nodeId(u"a"_s))._size;
        QVERIFY(size > 0.0F);

        setPositions(model, {{u"a"_s, {0.0F, 0.0F, -10.0F}}, {u"b"_s, {bOffset * size, 0.0F, -5.0F}},
            {u"c"_s, {0.0F, 0.0F, 5.0F}}});

        const QVector3D origin;
        const QVector3D direction(0.0F, 0.0F, -1.0F);
        Collision collision(graphModel, model.graph().componentIdOfNode(model.nodeId(u"a"_s)));

        std::vector<NodeId> nodeIds;
        collision.nodesInsideCylinder(origin, direction, radius * size, nodeIds);
        QCOMPARE(namesOf(model, nodeIds), inside);
        QCOMPARE(model.graphModel().nodeName(collision.nearestNodeInsideCylinder(origin, direction, radius * size)), nearest);

        // A cylinder with no radius is a line
        if(radius <= 0.0F)
        {
            nodeIds.clear();
            collision.nodesIntersectingLine(origin, direction, nodeIds);
            QCOMPARE(namesOf(model, nodeIds), inside);
            QCOMPARE(model.graphModel().nodeName(collision.nearestNodeIntersectingLine(origin, direction)), nearest);
        }

        // a is the closest to the line; c is closer to it, but behind the origin
        QCOMPARE(model.graphModel().nodeName(collision.nodeClosestToLine(origin, direction)), u"a"_s);
    }

    // Nodes left unhighlighted, when others are highlighted, can't be picked unless asked for
    void collisionIgnoresUnhighlightedNodes()
    {
        TestGraphModel model({u"a-b"_s, u"b-c"_s});
        model.applyTransforms({});
        auto& graphModel = model.graphModel();
        graphModel.enableVisualUpdates();
        graphModel.buildVisualisations({});

        // Highlighting only affects selected nodes
        SelectionManager selectionManager(graphModel);
        selectionManager.selectNodes(NodeIdSet{model.nodeId(u"a"_s), model.nodeId(u"b"_s), model.nodeId(u"c"_s)});
        graphModel.onSelectionChanged(&selectionManager);
        graphModel.highlightNodes({model.nodeId(u"a"_s)});
        graphModel.applyPendingVisualUpdates();

        QVERIFY(graphModel.nodeVisual(model.nodeId(u"b"_s)).state().test(VisualFlags::Unhighlighted));
        QVERIFY(!graphModel.nodeVisual(model.nodeId(u"a"_s)).state().test(VisualFlags::Unhighlighted));

        setPositions(model, {{u"a"_s, {0.0F, 0.0F, -10.0F}}, {u"b"_s, {0.0F, 0.0F, -5.0F}},
            {u"c"_s, {0.0F, 0.0F, 5.0F}}});

        const QVector3D origin;
        const QVector3D direction(0.0F, 0.0F, -1.0F);
        auto componentId = model.graph().componentIdOfNode(model.nodeId(u"a"_s));

        Collision collision(graphModel, componentId);
        QCOMPARE(graphModel.nodeName(collision.nearestNodeIntersectingLine(origin, direction)), u"a"_s);

        Collision includingNotFound(graphModel, componentId, true);
        QCOMPARE(graphModel.nodeName(includingNotFound.nearestNodeIntersectingLine(origin, direction)), u"b"_s);
    }

private:
    static void setPositions(TestGraphModel& model, const std::map<QString, QVector3D>& positions)
    {
        NodeLayoutPositions nodePositions(model.graphModel().mutableGraph());
        for(const auto& [name, position] : positions)
            nodePositions.set(model.nodeId(name), position);

        model.graphModel().nodePositions().update(nodePositions);
    }

    static QStringList namesOf(TestGraphModel& model, const std::vector<NodeId>& nodeIds)
    {
        QStringList names;
        names.reserve(static_cast<qsizetype>(nodeIds.size()));
        for(auto nodeId : nodeIds)
            names.append(model.graphModel().nodeName(nodeId));

        names.sort();
        return names;
    }
};

QTEST_MAIN(TestLayout)
#include "tst_layout.moc"
