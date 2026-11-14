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

#include "app/rendering/camera.h"

#include "app/maths/conicalfrustum.h"
#include "app/maths/frustum.h"
#include "app/maths/line.h"
#include "app/maths/ray.h"

#include <QLineF>
#include <QPointF>
#include <QQuaternion>
#include <QTest>
#include <QVector3D>
#include <QVector4D>

#include <utility>

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr int Width = 400;
constexpr int Height = 300;

Camera makeCamera(bool perspective)
{
    Camera camera;
    camera.setViewport({0.0, 0.0, Width, Height});
    camera.setFocus({1.0f, 2.0f, 3.0f});
    camera.setRotation(QQuaternion::fromEulerAngles(20.0f, 30.0f, 0.0f));
    camera.setDistance(50.0f);

    if(perspective)
        camera.setPerspectiveProjection(60.0f, static_cast<float>(Width) / Height, 1.0f, 1000.0f);
    else
        camera.setOrthographicProjection(-40.0f, 40.0f, -30.0f, 30.0f, 1.0f, 1000.0f);

    return camera;
}

// Where a point is drawn, in viewport coordinates, whose y runs down
QPointF project(const Camera& camera, const QVector3D& point)
{
    const auto clip = camera.projectionMatrix() * camera.viewMatrix() * QVector4D(point, 1.0f);
    const auto ndc = clip.toVector3DAffine();

    return {(ndc.x() + 1.0f) * 0.5f * Width, (1.0f - ndc.y()) * 0.5f * Height};
}

// A point that is drawn at the given viewport coordinates, as far away as the focus
QVector3D pointAt(const Camera& camera, int x, int y)
{
    const auto line = camera.lineForViewportCoordinates(x, y);
    return line.start() + (line.dir() * camera.distance());
}

bool fuzzyEqual(const QPointF& a, const QPointF& b) { return QLineF(a, b).length() < 1e-2; }
bool fuzzyEqual(const QVector3D& a, const QVector3D& b) { return a.distanceToPoint(b) < 1e-3f; }
} // namespace

class TestCamera : public QObject
{
    Q_OBJECT

private slots:
    void positionIsDistanceBackFromFocus()
    {
        const auto camera = makeCamera(true);

        QVERIFY(fuzzyEqual(camera.viewVector(),
            camera.rotation().rotatedVector({0.0f, 0.0f, -1.0f})));
        QVERIFY(fuzzyEqual(camera.position(), camera.focus() - (camera.viewVector() * 50.0f)));
        QCOMPARE(camera.position().distanceToPoint(camera.focus()), 50.0f);
    }

    void focusIsAtTheCentre_data() { projections(); }
    void focusIsAtTheCentre()
    {
        QFETCH(const bool, perspective);
        const auto camera = makeCamera(perspective);

        QVERIFY(fuzzyEqual(project(camera, camera.focus()), QPointF(Width / 2.0, Height / 2.0)));

        const auto ray = camera.rayForViewportCoordinates(Width / 2, Height / 2);
        QVERIFY(ray.distanceTo(camera.focus()) < 1e-3f);
        QVERIFY(fuzzyEqual(ray.dir(), camera.viewVector()));
    }

    // A point on the ray through some viewport coordinates is drawn at those coordinates
    void raysMatchProjection_data() { projections(); }
    void raysMatchProjection()
    {
        QFETCH(const bool, perspective);
        const auto camera = makeCamera(perspective);

        for(const auto& [x, y] : {std::pair{0, 0}, {Width, Height}, {37, 211}, {390, 12}})
        {
            const auto line = camera.lineForViewportCoordinates(x, y);
            for(const float t : {0.1f, 0.5f, 0.9f})
            {
                const auto point = line.start() + ((line.end() - line.start()) * t);
                QVERIFY2(fuzzyEqual(project(camera, point), QPointF(x, y)),
                    qPrintable(u"(%1, %2) at %3"_s.arg(x).arg(y).arg(t)));
            }
        }

        if(!perspective)
            QVERIFY(fuzzyEqual(camera.rayForViewportCoordinates(37, 211).dir(), camera.viewVector()));
    }

    void frustumContainsWhatIsInsideTheRectangle_data() { projections(); }
    void frustumContainsWhatIsInsideTheRectangle()
    {
        QFETCH(const bool, perspective);
        const auto camera = makeCamera(perspective);

        const auto frustum = camera.frustumForViewportCoordinates(100, 50, 300, 250);
        QVERIFY(frustum.containsPoint(pointAt(camera, 200, 150)));
        QVERIFY(frustum.containsPoint(pointAt(camera, 110, 240)));
        QVERIFY(!frustum.containsPoint(pointAt(camera, 50, 150)));
        QVERIFY(!frustum.containsPoint(pointAt(camera, 200, 280)));

        // Behind the camera
        QVERIFY(!frustum.containsPoint(camera.position() - (camera.viewVector() * 10.0f)));

        // The corners can be given in any order
        const auto reversed = camera.frustumForViewportCoordinates(300, 250, 100, 50);
        QVERIFY(reversed.containsPoint(pointAt(camera, 110, 240)));
        QVERIFY(!reversed.containsPoint(pointAt(camera, 50, 150)));
    }

    void conicalFrustumContainsWhatIsInsideTheCircle_data() { projections(); }
    void conicalFrustumContainsWhatIsInsideTheCircle()
    {
        QFETCH(const bool, perspective);
        const auto camera = makeCamera(perspective);

        const auto frustum = camera.conicalFrustumForViewportCoordinates(200, 150, 10);
        QVERIFY(frustum.containsPoint(pointAt(camera, 200, 150)));
        QVERIFY(frustum.containsPoint(pointAt(camera, 205, 145)));
        QVERIFY(!frustum.containsPoint(pointAt(camera, 215, 150)));
        QVERIFY(!frustum.containsPoint(pointAt(camera, 200, 139)));
    }

private:
    static void projections()
    {
        QTest::addColumn<bool>("perspective");
        QTest::newRow("perspective") << true;
        QTest::newRow("orthographic") << false;
    }
};

QTEST_GUILESS_MAIN(TestCamera)
#include "tst_camera.moc"
