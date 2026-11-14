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

#include "tests/common/rendertest.h"
#include "tests/common/testenvironment.h"

#include "app/ui/qml/Graphia/qmlelementid.h"

#include <QColor>
#include <QCoreApplication>
#include <QInputDevice>
#include <QLineF>
#include <QList>
#include <QNativeGestureEvent>
#include <QPoint>
#include <QPointingDevice>
#include <QRect>
#include <QSignalSpy>
#include <QStringList>
#include <QTest>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

using namespace Qt::Literals::StringLiterals;

class TestInteraction : public RenderTest
{
    Q_OBJECT

    // Where a node is drawn, found by its colour
    QPoint centreOf(const QColor& color)
    {
        return find(grab(), color)._centre.toPoint();
    }

    bool selects(const QStringList& names)
    {
        return QTest::qWaitFor([&] { return selected() == names && waitForCommands(); }, Timeout);
    }

    void drag(QPoint from, QPoint to, Qt::MouseButton button, Qt::KeyboardModifiers modifiers = {})
    {
        QTest::mousePress(_window.get(), button, modifiers, from);

        const int steps = 10;
        for(int step = 1; step <= steps; step++)
        {
            QTest::mouseEvent(QTest::MouseMove, _window.get(), Qt::NoButton, modifiers,
                from + ((to - from) * step / steps));
        }

        QTest::mouseRelease(_window.get(), button, modifiers, to);
    }

    void wheel(QPoint position, int angle)
    {
        QWheelEvent event(position, _window->mapToGlobal(position), {}, {0, angle},
            Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QCoreApplication::sendEvent(_window.get(), &event);
    }

    QPointingDevice _touchpad{u"touchpad"_s, 1, QInputDevice::DeviceType::TouchPad,
        QPointingDevice::PointerType::Finger,
        QInputDevice::Capability::Position | QInputDevice::Capability::PixelScroll, 2, 0};

    // Two fingers moving across the touchpad, in steps
    void panGesture(QPoint position, QPoint delta)
    {
        const int steps = 10;
        for(int step = 0; step < steps; step++)
        {
            QWheelEvent event(position, _window->mapToGlobal(position), delta / steps, {},
                Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false, Qt::MouseEventNotSynthesized, &_touchpad);
            QCoreApplication::sendEvent(_window.get(), &event);
        }
    }

    void zoomGesture(QPoint position, qreal value)
    {
        QNativeGestureEvent event(Qt::ZoomNativeGesture, &_touchpad, 2, position, position,
            _window->mapToGlobal(position), value, {});
        QCoreApplication::sendEvent(_window.get(), &event);
    }

    // The triangle, the singleton d, and e and f, a pair
    bool openThreeComponents()
    {
        const QByteArray graph = R"(graph
[
    node [ id 0 label "a" ]
    node [ id 1 label "b" ]
    node [ id 2 label "c" ]
    node [ id 3 label "d" ]
    node [ id 4 label "e" ]
    node [ id 5 label "f" ]
    edge [ source 0 target 1 ]
    edge [ source 1 target 2 ]
    edge [ source 4 target 5 ]
]
)";

        return open(graph) &&
            pin({{u"a"_s, {-10.0f, -6.0f, 0.0f}}, {u"b"_s, {10.0f, -6.0f, 0.0f}}, {u"c"_s, {0.0f, 10.0f, 0.0f}},
                {u"d"_s, {}}, {u"e"_s, {-5.0f, 0.0f, 0.0f}}, {u"f"_s, {5.0f, 0.0f, 0.0f}}}) &&
            colour({{u"a"_s, Qt::red}, {u"b"_s, Qt::green}, {u"c"_s, Qt::yellow},
                {u"d"_s, Qt::magenta}, {u"e"_s, Qt::cyan}, {u"f"_s, Qt::cyan}});
    }

    // Which of the three components is drawn, by the colour of one of its nodes
    QColor componentShown()
    {
        const auto image = grab();

        QList<QColor> shown;
        for(const auto& color : {QColor{Qt::red}, QColor{Qt::magenta}, QColor{Qt::cyan}})
        {
            if(find(image, color)._pixels > 0)
                shown.append(color);
        }

        return shown.size() == 1 ? shown.front() : QColor{};
    }

    static QPoint centre() { return {Width / 2, Height / 2}; }

    static QPoint corner() { return {5, 5}; }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        turnOffAmbientOcclusion();
        recordOpenGLMessages();
        QVERIFY(createApplication());
    }

    void cleanup()
    {
        close();
        QCOMPARE(takeOpenGLMessages(), QStringList{});
    }

    void cleanupTestCase() { destroyApplication(); }

    void clickSelectsOnlyThatNode()
    {
        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        QTest::mouseClick(_window.get(), Qt::LeftButton, {}, centreOf(Qt::red));
        QVERIFY(selects({u"a"_s}));

        QTest::mouseClick(_window.get(), Qt::LeftButton, {}, centreOf(Qt::green));
        QVERIFY(selects({u"b"_s}));

        QTest::mouseClick(_window.get(), Qt::LeftButton, {}, corner());
        QVERIFY(selects({}));
    }

    void shiftClickAddsToTheSelection()
    {
        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        QTest::mouseClick(_window.get(), Qt::LeftButton, {}, centreOf(Qt::red));
        QVERIFY(selects({u"a"_s}));

        QTest::mouseClick(_window.get(), Qt::LeftButton, Qt::ShiftModifier, centreOf(Qt::green));
        QVERIFY(selects({u"a"_s, u"b"_s}));

        // Nor does shift-clicking the background clear it
        QTest::mouseClick(_window.get(), Qt::LeftButton, Qt::ShiftModifier, corner());
        QTest::mouseClick(_window.get(), Qt::LeftButton, Qt::ShiftModifier, centreOf(Qt::yellow));
        QVERIFY(selects({u"a"_s, u"b"_s, u"c"_s}));
    }

    void shiftDragSelectsInsideTheRectangle()
    {
        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        const auto a = centreOf(Qt::red);
        const auto b = centreOf(Qt::green);
        const auto c = centreOf(Qt::yellow);
        const int margin = (a.y() - c.y()) / 3;

        drag({a.x() - margin, a.y() - margin}, {b.x() + margin, b.y() + margin},
            Qt::LeftButton, Qt::ShiftModifier);
        QVERIFY(selects({u"a"_s, u"b"_s}));
    }

    void leftDragRotates()
    {
        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        const auto a = centreOf(Qt::red);
        const auto b = centreOf(Qt::green);
        QTest::mouseClick(_window.get(), Qt::LeftButton, {}, a);
        QVERIFY(selects({u"a"_s}));

        // Across the empty bottom of the view, turning the triangle about the vertical
        drag({Width / 4, Height - 20}, {Width * 3 / 4, Height - 20}, Qt::LeftButton);

        const auto rotatedA = centreOf(Qt::red);
        const auto rotatedB = centreOf(Qt::green);
        QVERIFY(QLineF(rotatedA, rotatedB).length() < QLineF(a, b).length() * 0.9);
        QCOMPARE(selected(), QStringList{u"a"_s});
    }

    void rightClickReportsTheNode()
    {
        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        QSignalSpy clicked(_graphDisplay.get(), &GraphDisplay::clicked);
        QTest::mouseClick(_window.get(), Qt::RightButton, {}, centreOf(Qt::green));
        QVERIFY(QTest::qWaitFor([&] { return !clicked.isEmpty(); }, Timeout));

        QCOMPARE(clicked.at(0).at(0).toInt(), static_cast<int>(Qt::RightButton));
        QCOMPARE(static_cast<NodeId>(clicked.at(0).at(2).value<QmlNodeId>()), nodeId(u"b"_s));
        QCOMPARE(selected(), QStringList{});
    }

    void rightDragPans()
    {
        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        const auto a = centreOf(Qt::red);
        const auto b = centreOf(Qt::green);

        drag(corner(), corner() + QPoint(80, 0), Qt::RightButton);
        QVERIFY(QTest::qWaitFor([this] { return !viewIsReset(); }, Timeout));

        // Everything moves the way it was dragged, together
        const auto movedA = centreOf(Qt::red);
        const auto movedB = centreOf(Qt::green);
        QVERIFY(movedA.x() > a.x() + 40);
        QVERIFY(QLineF(QPointF(movedB - movedA), QPointF(b - a)).length() < QLineF(a, b).length() * 0.05);
    }

    void wheelZoomsAndDoubleClickResets()
    {
        QVERIFY(openTriangleAndSingleton());
        QVERIFY(inOverviewMode());

        // Entering a component by its node focuses on that node
        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, centreOf(Qt::red));
        QVERIFY(QTest::qWaitFor([this] { return !inOverviewMode() && !viewIsReset(); }, Timeout));

        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, corner());
        QVERIFY(QTest::qWaitFor([this] { return viewIsReset(); }, Timeout));
        const auto reset = find(grab(), Qt::red);
        QCOMPARE(find(grab(), Qt::magenta)._pixels, 0);

        wheel({Width / 2, Height / 2}, 120);
        QVERIFY(QTest::qWaitFor([this] { return !viewIsReset(); }, Timeout));
        QVERIFY(find(grab(), Qt::red)._pixels > reset._pixels * 1.2);

        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, corner());
        QVERIFY(QTest::qWaitFor([this] { return viewIsReset(); }, Timeout));
        QVERIFY(std::abs(find(grab(), Qt::red)._pixels - reset._pixels) < reset._pixels / 10);

        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, corner());
        QVERIFY(QTest::qWaitFor([this] { return inOverviewMode(); }, Timeout));
        QVERIFY(find(grab(), Qt::magenta)._pixels > 0);
    }

    void doubleClickFocusesOnTheNode()
    {
        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, centreOf(Qt::yellow));
        QVERIFY(QTest::qWaitFor([this] { return !viewIsReset(); }, Timeout));

        QVERIFY(QLineF(centreOf(Qt::yellow), centre()).length() < 3.0);
    }

    void doubleClickInOverviewEntersTheComponent()
    {
        QVERIFY(openTriangleAndSingleton());
        QVERIFY(inOverviewMode());

        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, centreOf(Qt::magenta));
        QVERIFY(QTest::qWaitFor([this] { return !inOverviewMode(); }, Timeout));

        const auto image = grab();
        QVERIFY(find(image, Qt::magenta)._pixels > 100);
        QCOMPARE(find(image, Qt::red)._pixels, 0);
        QCOMPARE(find(image, Qt::green)._pixels, 0);
    }

    void wheelZoomsAndRightDragPansTheOverview()
    {
        QVERIFY(openTriangleAndSingleton());
        QVERIFY(inOverviewMode());
        const auto reset = find(grab(), Qt::red);

        wheel(reset._centre.toPoint(), 120);
        QVERIFY(QTest::qWaitFor([this] { return !viewIsReset(); }, Timeout));
        const auto zoomed = find(grab(), Qt::red);
        QVERIFY(zoomed._pixels > reset._pixels * 1.2);

        drag(corner(), corner() + QPoint(80, 0), Qt::RightButton);
        QVERIFY(find(grab(), Qt::red)._centre.x() > zoomed._centre.x() + 40);

        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, corner());
        QVERIFY(QTest::qWaitFor([this] { return viewIsReset(); }, Timeout));
        const auto again = find(grab(), Qt::red);
        QVERIFY(std::abs(again._pixels - reset._pixels) < reset._pixels / 10);
        QVERIFY(QLineF(again._centre, reset._centre).length() < 3.0);
        QVERIFY(inOverviewMode());
    }

    void trackpadGestures_data()
    {
        QTest::addColumn<bool>("overview");

        QTest::newRow("component") << false;
        QTest::newRow("overview") << true;
    }

    void trackpadGestures()
    {
        QFETCH(const bool, overview);

        QVERIFY(overview ? openTriangleAndSingleton() : (openTriangle() && colourTriangle()));
        QCOMPARE(inOverviewMode(), overview);

        const auto a = centreOf(Qt::red);
        const auto b = centreOf(Qt::green);
        const auto c = centreOf(Qt::yellow);
        const auto middle = (a + b + c) / 3;

        // Panning turns the component under the fingers about the vertical
        panGesture(middle, {Width / 2, 0});
        const auto rotatedA = centreOf(Qt::red);
        const auto rotatedB = centreOf(Qt::green);
        QVERIFY(QLineF(rotatedA, rotatedB).length() < QLineF(a, b).length() * 0.9);

        const auto rotated = find(grab(), Qt::red);
        zoomGesture(rotatedA, 0.5);
        QVERIFY(find(grab(), Qt::red)._pixels > rotated._pixels * 1.2);
    }

    void shiftDragSelectsInTheOverview()
    {
        QVERIFY(openTriangleAndSingleton());

        const auto a = centreOf(Qt::red);
        const auto b = centreOf(Qt::green);
        const auto c = centreOf(Qt::yellow);
        const auto d = centreOf(Qt::magenta);

        const int margin = 10;
        const QRect around(QPoint(std::min({a.x(), b.x(), c.x()}) - margin, std::min({a.y(), b.y(), c.y()}) - margin),
            QPoint(std::max({a.x(), b.x(), c.x()}) + margin, std::max({a.y(), b.y(), c.y()}) + margin));
        QVERIFY(!around.contains(d));

        drag(around.topLeft(), around.bottomRight(), Qt::LeftButton, Qt::ShiftModifier);
        QVERIFY(selects({u"a"_s, u"b"_s, u"c"_s}));
    }

    void nextAndPreviousComponentVisitEachInTurn()
    {
        QVERIFY(openThreeComponents());

        // Entering by a node focuses on it, a view that is restored on coming back
        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, centreOf(Qt::red));
        QVERIFY(QTest::qWaitFor([this] { return !inOverviewMode() && !viewIsReset(); }, Timeout));
        QCOMPARE(componentShown(), QColor(Qt::red));
        const auto focused = find(grab(), Qt::red);

        QList<QColor> forwards;
        for(int step = 0; step < 3; step++)
        {
            _document->gotoNextComponent();
            forwards.append(componentShown());
        }

        QCOMPARE(forwards.last(), QColor(Qt::red));
        QVERIFY(forwards.contains(QColor(Qt::magenta)) && forwards.contains(QColor(Qt::cyan)));

        QVERIFY(!viewIsReset());
        const auto restored = find(grab(), Qt::red);
        QVERIFY(std::abs(restored._pixels - focused._pixels) < focused._pixels / 10);
        QVERIFY(QLineF(restored._centre, focused._centre).length() < 3.0);

        QList<QColor> backwards;
        for(int step = 0; step < 3; step++)
        {
            _document->gotoPrevComponent();
            backwards.append(componentShown());
        }

        QCOMPARE(backwards, (QList<QColor>{forwards.at(1), forwards.at(0), forwards.at(2)}));
    }

    void focusingANodeInAnotherComponent()
    {
        QVERIFY(openThreeComponents());

        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, centreOf(Qt::red));
        QVERIFY(QTest::qWaitFor([this] { return !inOverviewMode(); }, Timeout));

        _document->moveFocusToNode(nodeId(u"d"_s));
        QCOMPARE(componentShown(), QColor(Qt::magenta));
        QVERIFY(QLineF(centreOf(Qt::magenta), centre()).length() < 3.0);

        // A focus asked for during the move to another component waits until it has arrived
        _document->moveFocusToNode(nodeId(u"a"_s));
        QVERIFY(QTest::qWaitFor([this] { return _graphDisplay->property("transitioning").toBool(); }, Timeout));
        _document->moveFocusToNode(nodeId(u"b"_s));

        QCOMPARE(componentShown(), QColor(Qt::red));
        QVERIFY(QLineF(centreOf(Qt::green), centre()).length() < 3.0);
    }

    void focusingNodesInSeveralComponents()
    {
        QVERIFY(openThreeComponents());
        QVERIFY(inOverviewMode());

        const auto before = grab();
        const auto a = find(before, Qt::red);
        const auto d = find(before, Qt::magenta);

        // The overview zooms in on the components they are in
        _document->moveFocusToNodes({nodeId(u"a"_s), nodeId(u"d"_s)});
        QVERIFY(QTest::qWaitFor([this] { return !viewIsReset(); }, Timeout));
        auto image = grab();
        QVERIFY(find(image, Qt::red)._pixels > a._pixels * 1.2);
        QVERIFY(find(image, Qt::magenta)._pixels > d._pixels * 1.2);

        // From component mode, the overview opens zoomed in on them
        QTest::mouseDClick(_window.get(), Qt::LeftButton, {}, centreOf(Qt::red));
        QVERIFY(QTest::qWaitFor([this] { return !inOverviewMode(); }, Timeout));

        _document->moveFocusToNodes({nodeId(u"a"_s), nodeId(u"d"_s)});
        QVERIFY(QTest::qWaitFor([this] { return inOverviewMode(); }, Timeout));
        image = grab();
        QVERIFY(find(image, Qt::red)._pixels > a._pixels * 1.2);
        QVERIFY(find(image, Qt::magenta)._pixels > d._pixels * 1.2);
    }
};

QTEST_MAIN(TestInteraction)
#include "tst_interaction.moc"
