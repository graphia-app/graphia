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

#include "app/maths/boundingbox.h"
#include "app/maths/boundingsphere.h"
#include "app/maths/circle.h"
#include "app/maths/conicalfrustum.h"
#include "app/maths/frustum.h"
#include "app/maths/line.h"
#include "app/maths/plane.h"
#include "app/maths/ray.h"

#include <QTest>
#include <QVector2D>
#include <QVector3D>

#include <cmath>
#include <limits>
#include <vector>

static bool fuzzyEqual(const QVector3D& a, const QVector3D& b)
{
    return (a - b).length() < 1e-5f;
}

// A macro, like QCOMPARE, so that a failure returns from the test
// NOLINTNEXTLINE cppcoreguidelines-macro-usage
#define QCOMPARE_VECTOR(actual, expected) \
    QVERIFY2(fuzzyEqual((actual), (expected)), \
        qPrintable(QStringLiteral("(%1, %2, %3) != (%4, %5, %6)") \
            .arg((actual).x()).arg((actual).y()).arg((actual).z()) \
            .arg((expected).x()).arg((expected).y()).arg((expected).z())))

class TestGeometry : public QObject
{
    Q_OBJECT

private slots:
    void boundingBox2D()
    {
        const BoundingBox2D box({{1.0f, 2.0f}, {-1.0f, 5.0f}, {4.0f, -2.0f}});

        QCOMPARE(box.min(), QVector2D(-1.0f, -2.0f));
        QCOMPARE(box.max(), QVector2D(4.0f, 5.0f));
        QCOMPARE(box.xLength(), 5.0f);
        QCOMPARE(box.yLength(), 7.0f);
        QCOMPARE(box.maxLength(), 7.0f);
        QCOMPARE(box.area(), 35.0f);
        QCOMPARE(box.centre(), QVector2D(1.5f, 1.5f));
        QCOMPARE(box.xVector(), QVector2D(5.0f, 0.0f));
        QCOMPARE(box.yVector(), QVector2D(0.0f, 7.0f));

        // Boxes are half open, so that a point is only ever in one of two adjacent boxes
        QVERIFY(box.containsPoint({-1.0f, -2.0f}));
        QVERIFY(box.containsPoint({0.0f, 0.0f}));
        QVERIFY(!box.containsPoint({4.0f, 0.0f}));
        QVERIFY(!box.containsPoint({0.0f, 5.0f}));
        QVERIFY(!box.containsPoint({-2.0f, 0.0f}));

        QVERIFY(box.containsLine({{0.0f, 0.0f}, {3.0f, 4.0f}}));
        QVERIFY(!box.containsLine({{0.0f, 0.0f}, {10.0f, 4.0f}}));

        QCOMPARE((box + QVector2D(1.0f, 1.0f)).min(), QVector2D(0.0f, -1.0f));
        QCOMPARE((box * 2.0f).max(), QVector2D(8.0f, 10.0f));

        auto expanded = box;
        expanded.expandToInclude(QVector2D(10.0f, -10.0f));
        QCOMPARE(expanded.min(), QVector2D(-1.0f, -10.0f));
        QCOMPARE(expanded.max(), QVector2D(10.0f, 5.0f));

        expanded.expandToInclude(BoundingBox2D({-5.0f, 0.0f}, {0.0f, 20.0f}));
        QCOMPARE(expanded.min(), QVector2D(-5.0f, -10.0f));
        QCOMPARE(expanded.max(), QVector2D(10.0f, 20.0f));

        QVERIFY(box.valid());
        QVERIFY(!BoundingBox2D({std::nanf(""), 0.0f}, {1.0f, 1.0f}).valid());
        QVERIFY(!BoundingBox2D({0.0f, 0.0f}, {std::numeric_limits<float>::infinity(), 1.0f}).valid());
    }

    void boundingBox3D()
    {
        const BoundingBox3D box({{1.0f, 2.0f, 3.0f}, {-1.0f, 5.0f, 0.0f}, {4.0f, -2.0f, 1.0f}});

        QCOMPARE(box.min(), QVector3D(-1.0f, -2.0f, 0.0f));
        QCOMPARE(box.max(), QVector3D(4.0f, 5.0f, 3.0f));
        QCOMPARE(box.maxLength(), 7.0f);
        QCOMPARE(box.volume(), 105.0f);
        QCOMPARE(box.centre(), QVector3D(1.5f, 1.5f, 1.5f));
        QCOMPARE(box.zVector(), QVector3D(0.0f, 0.0f, 3.0f));

        QVERIFY(box.containsPoint({-1.0f, -2.0f, 0.0f}));
        QVERIFY(box.containsPoint({0.0f, 0.0f, 1.0f}));
        QVERIFY(!box.containsPoint({0.0f, 0.0f, 3.0f}));
        QVERIFY(!box.containsPoint({0.0f, 0.0f, -1.0f}));
        QVERIFY(box.containsLine({{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}));
        QVERIFY(!box.containsLine({{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 5.0f}}));

        // Scaling is about the centre
        const auto scaled = box.scaled(2.0f);
        QCOMPARE(scaled.min(), QVector3D(-3.5f, -5.5f, -1.5f));
        QCOMPARE(scaled.max(), QVector3D(6.5f, 8.5f, 4.5f));
        QCOMPARE(scaled.centre(), box.centre());

        auto expanded = box;
        expanded.expandToInclude(BoundingBox3D({0.0f, 0.0f, -4.0f}, {0.0f, 9.0f, 0.0f}));
        QCOMPARE(expanded.min(), QVector3D(-1.0f, -2.0f, -4.0f));
        QCOMPARE(expanded.max(), QVector3D(4.0f, 9.0f, 3.0f));

        QVERIFY(box.valid());
        QVERIFY(!BoundingBox3D({0.0f, 0.0f, std::nanf("")}, {1.0f, 1.0f, 1.0f}).valid());
    }

    void boundingBoxRayIntersection_data()
    {
        QTest::addColumn<QVector3D>("origin");
        QTest::addColumn<QVector3D>("dir");
        QTest::addColumn<bool>("expected");

        QTest::newRow("towards") << QVector3D(-1.0f, 0.5f, 0.5f) << QVector3D(1.0f, 0.0f, 0.0f) << true;
        QTest::newRow("away") << QVector3D(-1.0f, 0.5f, 0.5f) << QVector3D(-1.0f, 0.0f, 0.0f) << false;
        QTest::newRow("inside") << QVector3D(0.5f, 0.5f, 0.5f) << QVector3D(0.0f, 0.0f, 1.0f) << true;
        QTest::newRow("passes by") << QVector3D(-1.0f, 2.0f, 0.5f) << QVector3D(1.0f, 0.0f, 0.0f) << false;
        QTest::newRow("diagonal") << QVector3D(-1.0f, -1.0f, -1.0f) <<
            QVector3D(1.0f, 1.0f, 1.0f).normalized() << true;
        QTest::newRow("diagonal miss") << QVector3D(-1.0f, -1.0f, -1.0f) <<
            QVector3D(1.0f, 1.0f, -1.0f).normalized() << false;
    }

    void boundingBoxRayIntersection()
    {
        QFETCH(const QVector3D, origin);
        QFETCH(const QVector3D, dir);
        QFETCH(const bool, expected);

        const BoundingBox3D box({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});
        QCOMPARE(box.intersects(Ray(origin, dir)), expected);
    }

    void boundingBoxRayIntersectionInterval()
    {
        const BoundingBox3D box({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});
        const Ray ray({-1.0f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f});

        // The ray is in the box for 1 <= t <= 2
        QVERIFY(box.intersects(ray, 0.0f, 1.5f));
        QVERIFY(box.intersects(ray, 1.5f, 3.0f));
        QVERIFY(!box.intersects(ray, 0.0f, 0.5f));
        QVERIFY(!box.intersects(ray, 2.5f, 3.0f));
    }

    void boundingSphere()
    {
        // Centred on the barycentre, so not necessarily the smallest sphere
        const BoundingSphere sphere({{-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 3.0f, 0.0f}});
        QCOMPARE(sphere.centre(), QVector3D(0.0f, 1.0f, 0.0f));
        QCOMPARE(sphere.radius(), 2.0f);

        const BoundingSphere centred({0.0f, 0.0f, 0.0f},
            std::vector<QVector3D>{{-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 3.0f, 0.0f}});
        QCOMPARE(centred.centre(), QVector3D(0.0f, 0.0f, 0.0f));
        QCOMPARE(centred.radius(), 3.0f);

        QCOMPARE(centred.scaled(2.0f).radius(), 6.0f);
        QCOMPARE(centred.scaled(2.0f).centre(), centred.centre());
        QVERIFY(qFuzzyCompare(BoundingSphere({}, 1.0f).volume(), 4.0f * 3.14159265f / 3.0f));

        BoundingSphere s({0.0f, 0.0f, 0.0f}, 1.0f);
        QVERIFY(s.containsPoint({1.0f, 0.0f, 0.0f}));
        QVERIFY(!s.containsPoint({1.0f, 1.0f, 0.0f}));
        QVERIFY(s.containsLine({{0.5f, 0.0f, 0.0f}, {0.0f, -0.5f, 0.0f}}));
        QVERIFY(!s.containsLine({{0.5f, 0.0f, 0.0f}, {0.0f, -1.5f, 0.0f}}));

        // Expanding doesn't move the centre
        s.expandToInclude(QVector3D(0.0f, 0.0f, 5.0f));
        QCOMPARE(s.centre(), QVector3D(0.0f, 0.0f, 0.0f));
        QCOMPARE(s.radius(), 5.0f);

        s.expandToInclude(QVector3D(0.0f, 1.0f, 0.0f));
        QCOMPARE(s.radius(), 5.0f);

        const BoundingSphere other({7.0f, 0.0f, 0.0f}, 1.0f);
        QVERIFY(!s.containsSphere(other));
        s.expandToInclude(other);
        QCOMPARE(s.radius(), 8.0f);
        QVERIFY(s.containsSphere(other));
    }

    void boundingSphereRayIntersection_data()
    {
        QTest::addColumn<QVector3D>("origin");
        QTest::addColumn<QVector3D>("dir");
        QTest::addColumn<std::vector<QVector3D>>("expected");

        const QVector3D x(1.0f, 0.0f, 0.0f);
        const float h = std::sqrt(0.75f);

        QTest::newRow("through") << QVector3D(-3.0f, 0.0f, 0.0f) << x <<
            std::vector<QVector3D>{{-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};
        QTest::newRow("from centre") << QVector3D(0.0f, 0.0f, 0.0f) << x <<
            std::vector<QVector3D>{{1.0f, 0.0f, 0.0f}};
        QTest::newRow("from inside, centre behind") << QVector3D(0.5f, 0.0f, 0.0f) << x <<
            std::vector<QVector3D>{{1.0f, 0.0f, 0.0f}};
        QTest::newRow("from inside, centre ahead") << QVector3D(-0.5f, 0.0f, 0.0f) << x <<
            std::vector<QVector3D>{{1.0f, 0.0f, 0.0f}};
        QTest::newRow("from inside, off axis") << QVector3D(0.5f, 0.5f, 0.0f) << x <<
            std::vector<QVector3D>{{h, 0.5f, 0.0f}};
        QTest::newRow("from inside, off axis, centre ahead") << QVector3D(-0.5f, 0.5f, 0.0f) << x <<
            std::vector<QVector3D>{{h, 0.5f, 0.0f}};
        QTest::newRow("off axis") << QVector3D(-3.0f, 0.5f, 0.0f) << x <<
            std::vector<QVector3D>{{-h, 0.5f, 0.0f}, {h, 0.5f, 0.0f}};
        QTest::newRow("away") << QVector3D(3.0f, 0.0f, 0.0f) << x << std::vector<QVector3D>{};
        QTest::newRow("passes by") << QVector3D(-3.0f, 2.0f, 0.0f) << x << std::vector<QVector3D>{};
    }

    void boundingSphereRayIntersection()
    {
        QFETCH(const QVector3D, origin);
        QFETCH(const QVector3D, dir);
        QFETCH(std::vector<QVector3D>, expected);

        const BoundingSphere sphere({0.0f, 0.0f, 0.0f}, 1.0f);
        auto intersections = sphere.rayIntersection(Ray(origin, dir));

        QCOMPARE(intersections.size(), expected.size());
        for(size_t i = 0; i < expected.size(); i++)
            QCOMPARE_VECTOR(intersections.at(i), expected.at(i));
    }

    void ray()
    {
        const Ray ray({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});

        QCOMPARE(ray.closestPointTo(QVector3D(2.0f, 3.0f, 0.0f)), QVector3D(2.0f, 0.0f, 0.0f));
        QCOMPARE(ray.distanceTo(QVector3D(2.0f, 3.0f, 0.0f)), 3.0f);
        QCOMPARE(ray.distanceTo(QVector3D(5.0f, 0.0f, 0.0f)), 0.0f);

        // Perpendicular, and offset in z
        const Ray other({2.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
        QCOMPARE_VECTOR(ray.closestPointTo(other), QVector3D(2.0f, 0.0f, 0.0f));
        QCOMPARE_VECTOR(other.closestPointTo(ray), QVector3D(2.0f, 0.0f, 1.0f));
        QCOMPARE(ray.distanceTo(other), 1.0f);
        QCOMPARE(other.distanceTo(ray), 1.0f);

        const Ray fromLine(Line3D({1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 5.0f}));
        QCOMPARE(fromLine.origin(), QVector3D(1.0f, 1.0f, 1.0f));
        QCOMPARE(fromLine.dir(), QVector3D(0.0f, 0.0f, 1.0f));
    }

    void line()
    {
        Line3D line({0.0f, 0.0f, 0.0f}, {3.0f, 4.0f, 0.0f});
        QCOMPARE(line.length(), 5.0f);
        QCOMPARE(line.dir(), QVector3D(0.6f, 0.8f, 0.0f));

        // The length is cached, so must be recalculated when the line changes
        line.setEnd({0.0f, 0.0f, 2.0f});
        QCOMPARE(line.length(), 2.0f);
        line.setStart({0.0f, 0.0f, 1.0f});
        QCOMPARE(line.length(), 1.0f);
    }

    void plane()
    {
        const Plane plane(QVector3D(0.0f, 0.0f, 2.0f), QVector3D(0.0f, 0.0f, 1.0f));
        QCOMPARE(plane.distance(), -2.0f);

        QVERIFY(plane.sideForPoint({0.0f, 0.0f, 3.0f}) == Plane::Side::Front);
        QVERIFY(plane.sideForPoint({0.0f, 0.0f, 2.0f}) == Plane::Side::Front);
        QVERIFY(plane.sideForPoint({0.0f, 0.0f, 1.0f}) == Plane::Side::Back);

        // Positive behind the plane
        QCOMPARE(plane.distanceToPoint({7.0f, 7.0f, 5.0f}), -3.0f);
        QCOMPARE(plane.distanceToPoint({7.0f, 7.0f, 0.0f}), 2.0f);

        QCOMPARE(plane.project({3.0f, 4.0f, 7.0f}), QVector3D(3.0f, 4.0f, 2.0f));

        QCOMPARE(plane.rayIntersection(Ray({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f})), QVector3D(0.0f, 0.0f, 2.0f));
        QCOMPARE(plane.rayIntersection(Ray({1.0f, 1.0f, 5.0f}, {0.0f, 0.0f, -1.0f})), QVector3D(1.0f, 1.0f, 2.0f));

        // Anticlockwise, seen from the front
        const Plane fromPoints({0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f});
        QCOMPARE(fromPoints.normal(), QVector3D(0.0f, 0.0f, 1.0f));
        QCOMPARE(fromPoints.distance(), -1.0f);
        QVERIFY(fromPoints.sideForPoint({0.0f, 0.0f, 2.0f}) == Plane::Side::Front);
    }

    void frustum_data()
    {
        QTest::addColumn<QVector3D>("point");
        QTest::addColumn<bool>("expected");

        QTest::newRow("centre") << QVector3D(0.0f, 0.0f, 5.0f) << true;
        QTest::newRow("near") << QVector3D(0.0f, 0.0f, 0.1f) << true;
        QTest::newRow("far") << QVector3D(0.0f, 0.0f, 9.9f) << true;
        QTest::newRow("in front") << QVector3D(0.0f, 0.0f, -1.0f) << false;
        QTest::newRow("beyond") << QVector3D(0.0f, 0.0f, 11.0f) << false;
        QTest::newRow("just inside +x") << QVector3D(1.4f, 0.0f, 5.0f) << true;
        QTest::newRow("just outside +x") << QVector3D(1.6f, 0.0f, 5.0f) << false;
        QTest::newRow("just inside -x") << QVector3D(-1.4f, 0.0f, 5.0f) << true;
        QTest::newRow("just outside -x") << QVector3D(-1.6f, 0.0f, 5.0f) << false;
        QTest::newRow("just inside +y") << QVector3D(0.0f, 1.4f, 5.0f) << true;
        QTest::newRow("just outside +y") << QVector3D(0.0f, 1.6f, 5.0f) << false;
        QTest::newRow("just inside -y") << QVector3D(0.0f, -1.4f, 5.0f) << true;
        QTest::newRow("just outside -y") << QVector3D(0.0f, -1.6f, 5.0f) << false;
    }

    void frustum()
    {
        QFETCH(const QVector3D, point);
        QFETCH(const bool, expected);

        // A square frustum, widening from 2x2 at z = 0 to 4x4 at z = 10, with
        // its corners anticlockwise as seen looking along it
        const Frustum frustum(
            {{-1.0f, -1.0f, 0.0f}, {-2.0f, -2.0f, 10.0f}},
            {{ 1.0f, -1.0f, 0.0f}, { 2.0f, -2.0f, 10.0f}},
            {{ 1.0f,  1.0f, 0.0f}, { 2.0f,  2.0f, 10.0f}},
            {{-1.0f,  1.0f, 0.0f}, {-2.0f,  2.0f, 10.0f}});

        QCOMPARE(frustum.containsPoint(point), expected);
        QCOMPARE(frustum.centreLine().start(), QVector3D(0.0f, 0.0f, 0.0f));
        QCOMPARE(frustum.centreLine().end(), QVector3D(0.0f, 0.0f, 10.0f));
    }

    void conicalFrustum_data()
    {
        QTest::addColumn<QVector3D>("point");
        QTest::addColumn<bool>("expected");

        QTest::newRow("centre") << QVector3D(0.0f, 0.0f, 5.0f) << true;
        QTest::newRow("in front") << QVector3D(0.0f, 0.0f, -1.0f) << false;
        QTest::newRow("beyond") << QVector3D(0.0f, 0.0f, 11.0f) << false;
        QTest::newRow("middle, just inside") << QVector3D(1.9f, 0.0f, 5.0f) << true;
        QTest::newRow("middle, just outside") << QVector3D(2.1f, 0.0f, 5.0f) << false;
        QTest::newRow("middle, just inside, diagonal") << QVector3D(1.3f, -1.3f, 5.0f) << true;
        QTest::newRow("middle, just outside, diagonal") << QVector3D(1.5f, -1.5f, 5.0f) << false;
        QTest::newRow("near, just inside") << QVector3D(1.1f, 0.0f, 1.0f) << true;
        QTest::newRow("near, just outside") << QVector3D(1.3f, 0.0f, 1.0f) << false;
        QTest::newRow("far, just inside") << QVector3D(0.0f, 2.7f, 9.0f) << true;
        QTest::newRow("far, just outside") << QVector3D(0.0f, 2.9f, 9.0f) << false;
    }

    void conicalFrustum()
    {
        QFETCH(const QVector3D, point);
        QFETCH(const bool, expected);

        // Radius 1 at z = 0, widening to 3 at z = 10
        const ConicalFrustum frustum({{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 10.0f}},
            {{1.0f, 0.0f, 0.0f}, {3.0f, 0.0f, 10.0f}});

        QCOMPARE(frustum.containsPoint(point), expected);
    }

    void circle()
    {
        Circle a(0.0f, 0.0f, 1.0f);
        const Circle b(3.0f, 4.0f, 2.0f);

        QCOMPARE(a.distanceToCentreSq(b), 25.0f);
        QCOMPARE(a.distanceTo(b), 2.0f);
        QCOMPARE(a.distanceToSq(b), 16.0f);
        QVERIFY(!a.intersects(b));
        QVERIFY(a.intersects(Circle(1.0f, 1.0f, 1.0f)));

        // Touching isn't intersecting
        QVERIFY(!a.intersects(Circle(2.0f, 0.0f, 1.0f)));

        QCOMPARE(b.boundingBox(), QRectF(1.0, 2.0, 4.0, 4.0));
        QCOMPARE(b.centre(), QPointF(3.0, 4.0));

        a.translate({1.0, 2.0});
        QCOMPARE(a, Circle(1.0f, 2.0f, 1.0f));

        a.scale(2.0f);
        QCOMPARE(a, Circle(2.0f, 4.0f, 2.0f));
        QVERIFY(a != b);
    }
};

QTEST_GUILESS_MAIN(TestGeometry)
#include "tst_geometry.moc"
