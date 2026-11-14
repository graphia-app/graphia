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

#include "app/ui/qml/Graphia/graphrenderertypes.h"

#include "app/ui/qml/Graphia/Utils/preferences.h"

#include <QDesktopServices>
#include <QImage>
#include <QLineF>
#include <QObject>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QUrl>
#include <QVariantMap>

#include <cmath>
#include <map>
#include <numbers>

using namespace Qt::Literals::StringLiterals;

// Stands in for the desktop, which would otherwise be asked to open each screenshot taken
class OpenedUrls : public QObject
{
    Q_OBJECT

public:
    QList<QUrl> _urls;

    Q_INVOKABLE void open(const QUrl& url) { _urls.append(url); }
};

class TestRendering : public RenderTest
{
    Q_OBJECT

    QVariantMap _changedPrefs;

    void setPref(const QString& key, const QVariant& value)
    {
        if(!_changedPrefs.contains(key))
            _changedPrefs.insert(key, u::pref(key));

        u::setPref(key, value);
    }

    void restorePrefs()
    {
        for(const auto& [key, value] : _changedPrefs.asKeyValueRange())
            u::setPref(key, value);

        _changedPrefs.clear();
    }

    QColor background() const { return u::pref(u"visuals/backgroundColor"_s).value<QColor>(); }

    bool isBackground(const QColor& pixel) const { return pixel == background(); }

    // An edge's pixels are neither the background nor any node's colour
    bool isEdge(const QColor& pixel) const
    {
        return !isBackground(pixel) && !showsColour(pixel, Qt::red) && !showsColour(pixel, Qt::green) &&
            !showsColour(pixel, Qt::yellow) && !showsColour(pixel, Qt::blue);
    }

    // How many pixels differ between two grabs, within radius of a point
    static int differences(const QImage& from, const QImage& to, QPointF centre, float radius)
    {
        int count = 0;
        const auto dpr = from.devicePixelRatio();

        for(int y = 0; y < from.height(); y++)
        {
            for(int x = 0; x < from.width(); x++)
            {
                const QPointF position(x / dpr, y / dpr);
                if(QLineF(position, centre).length() > radius)
                    continue;

                if(from.pixel(x, y) != to.pixel(x, y))
                    count++;
            }
        }

        return count;
    }

    // The labels are drawn in a dark colour against the light background
    static int darkPixels(const QImage& image)
    {
        int count = 0;
        for(int y = 0; y < image.height(); y++)
        {
            for(int x = 0; x < image.width(); x++)
            {
                const auto pixel = image.pixelColor(x, y);
                if(std::max({pixel.redF(), pixel.greenF(), pixel.blueF()}) < 0.3f)
                    count++;
            }
        }

        return count;
    }

    static float radiusOf(const Patch& patch)
    {
        return std::sqrt(static_cast<float>(patch._pixels) / std::numbers::pi_v<float>);
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        recordOpenGLMessages();
        QVERIFY(createApplication());
    }

    void cleanup()
    {
        close();
        restorePrefs();

        QCOMPARE(takeOpenGLMessages(), QStringList{});
    }

    void cleanupTestCase() { destroyApplication(); }

    void nodesAndEdgesAreDrawn()
    {
        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        const auto image = grab();
        const auto a = find(image, Qt::red);
        const auto b = find(image, Qt::green);
        const auto c = find(image, Qt::yellow);

        QVERIFY(a._pixels > 100);
        QVERIFY(b._pixels > 100);
        QVERIFY(c._pixels > 100);

        // The view looks along -z with y up, and screen y runs down
        QVERIFY(a._centre.x() < c._centre.x() && c._centre.x() < b._centre.x());
        QVERIFY(c._centre.y() < a._centre.y());
        QVERIFY(std::abs(a._centre.y() - b._centre.y()) < 2.0);

        QVERIFY(isEdge(pixelAt(image, (a._centre + b._centre) / 2.0)));
        QVERIFY(isEdge(pixelAt(image, (b._centre + c._centre) / 2.0)));
        QVERIFY(isBackground(pixelAt(image, (a._centre + c._centre) / 2.0)));

        QVERIFY(isBackground(pixelAt(image, {1.0, 1.0})));
        QVERIFY(isBackground(pixelAt(image, {Width - 2.0, Height - 2.0})));
    }

    void colourVisualisation()
    {
        QVERIFY(openTriangle());

        auto image = grab();
        QVERIFY(find(image, Qt::blue)._pixels > 300);
        QCOMPARE(find(image, Qt::red)._pixels, 0);

        QVERIFY(colourTriangle());

        image = grab();
        QCOMPARE(find(image, Qt::blue)._pixels, 0);
        QVERIFY(find(image, Qt::red)._pixels > 100);
        QVERIFY(find(image, Qt::green)._pixels > 100);
        QVERIFY(find(image, Qt::yellow)._pixels > 100);

        QVERIFY(changeVisuals([this]
        {
            _document->removeVisualisation(0);
            _document->update();
        }));

        image = grab();
        QVERIFY(find(image, Qt::blue)._pixels > 300);
        QCOMPARE(find(image, Qt::red)._pixels, 0);
    }

    void selectionOutline()
    {
        // Selected nodes are otherwise labelled
        setPref(u"visuals/showNodeText"_s, static_cast<int>(TextState::Off));

        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        const auto unselected = grab();
        const auto a = find(unselected, Qt::red);
        const auto b = find(unselected, Qt::green);

        QVERIFY(changeVisuals([this] { _document->selectNodes({nodeId(u"b"_s)}); }));
        const auto selected = grab();

        QCOMPARE(differences(unselected, selected, a._centre, radiusOf(a) * 1.5f), 0);
        QVERIFY(differences(unselected, selected, b._centre, radiusOf(b) * 1.5f) > 20);
    }

    void filteredNodesAreNotDrawn()
    {
        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());
        QVERIFY(find(grab(), Qt::yellow)._pixels > 100);

        QVERIFY(execute([this] { _document->update({uR"(RemoveNodes where $NodeName == "c")"_s}); }));

        const auto image = grab();
        QCOMPARE(find(image, Qt::yellow)._pixels, 0);
        QVERIFY(find(image, Qt::red)._pixels > 100);
        QVERIFY(find(image, Qt::green)._pixels > 100);
    }

    void overviewDrawsEveryComponent()
    {
        const QByteArray components = R"(graph
[
    node [ id 0 label "a" ]
    node [ id 1 label "b" ]
    node [ id 2 label "c" ]
    node [ id 3 label "d" ]
    node [ id 4 label "e" ]
    edge [ source 0 target 1 ]
    edge [ source 2 target 3 ]
]
)";

        QVERIFY(open(components));
        QVERIFY(pin({{u"a"_s, {-5.0f, 0.0f, 0.0f}}, {u"b"_s, {5.0f, 0.0f, 0.0f}},
            {u"c"_s, {-5.0f, 0.0f, 0.0f}}, {u"d"_s, {5.0f, 0.0f, 0.0f}}, {u"e"_s, {}}}));
        QVERIFY(colour({{u"a"_s, Qt::red}, {u"c"_s, Qt::green}, {u"e"_s, Qt::yellow}}));

        if(!_graphDisplay->property("inOverviewMode").toBool())
            _document->switchToOverviewMode();

        QVERIFY(QTest::qWaitFor([this] { return _graphDisplay->property("inOverviewMode").toBool(); }, Timeout));

        const auto image = grab();
        const auto a = find(image, Qt::red);
        const auto c = find(image, Qt::green);
        const auto e = find(image, Qt::yellow);

        QVERIFY(a._pixels > 20);
        QVERIFY(c._pixels > 20);
        QVERIFY(e._pixels > 20);

        // Each in a place of its own
        QVERIFY(QLineF(a._centre, c._centre).length() > radiusOf(a));
        QVERIFY(QLineF(a._centre, e._centre).length() > radiusOf(a));
        QVERIFY(QLineF(c._centre, e._centre).length() > radiusOf(c));
    }

    void settings_data()
    {
        QTest::addColumn<QVariantMap>("prefs");
        QTest::addColumn<int>("projection");

        const auto onlyDetailLevels = [](const QString& nodes, const QString& edges)
        {
            QVariantMap prefs;
            for(const auto* level : {"tracedNodes", "splattedNodes", "tracedEdges",
                "approximateTracedEdges", "splattedEdges", "fastSplattedEdges"})
            {
                const auto name = QString::fromLatin1(level);
                prefs.insert(u"debug/"_s + name, name == nodes || name == edges);
            }

            return prefs;
        };

        const int unchanged = static_cast<int>(Projection::Unset);

        QTest::newRow("defaults") << QVariantMap{} << unchanged;
        QTest::newRow("traced nodes and edges") <<
            onlyDetailLevels(u"tracedNodes"_s, u"tracedEdges"_s) << unchanged;
        QTest::newRow("splatted nodes, approximate traced edges") <<
            onlyDetailLevels(u"splattedNodes"_s, u"approximateTracedEdges"_s) << unchanged;
        QTest::newRow("splatted edges") <<
            onlyDetailLevels(u"tracedNodes"_s, u"splattedEdges"_s) << unchanged;
        QTest::newRow("fast splatted edges") <<
            onlyDetailLevels(u"splattedNodes"_s, u"fastSplattedEdges"_s) << unchanged;
        QTest::newRow("no ambient occlusion") << QVariantMap{{u"visuals/ambientOcclusion"_s,
            static_cast<int>(AmbientOcclusionMode::Off)}} << unchanged;
        QTest::newRow("ambient occlusion") << QVariantMap{{u"visuals/ambientOcclusion"_s,
            static_cast<int>(AmbientOcclusionMode::On)}} << unchanged;
        QTest::newRow("no antialiasing") << QVariantMap{{u"visuals/enableAntialiasing"_s, false}} << unchanged;
        QTest::newRow("orthographic") << QVariantMap{} << static_cast<int>(Projection::Orthographic);
        QTest::newRow("2D") << QVariantMap{} << static_cast<int>(Projection::TwoDee);
    }

    void settings()
    {
        QFETCH(const QVariantMap, prefs);
        QFETCH(const int, projection);

        for(const auto& [key, value] : prefs.asKeyValueRange())
            setPref(key, value);

        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        if(projection != static_cast<int>(Projection::Unset))
            _document->setProjection(projection);

        const auto image = grab();
        QVERIFY(find(image, Qt::red)._pixels > 100);
        QVERIFY(find(image, Qt::green)._pixels > 100);
        QVERIFY(find(image, Qt::yellow)._pixels > 100);
    }

    void screenshot_data()
    {
        QTest::addColumn<QSize>("size");
        QTest::addColumn<int>("dpi");
        QTest::addColumn<bool>("fillSize");

        QTest::newRow("window's aspect") << QSize(800, 600) << 72 << false;
        QTest::newRow("taller, fitted") << QSize(800, 800) << 300 << false;
        QTest::newRow("taller, filled") << QSize(800, 800) << 96 << true;
        QTest::newRow("two tiles wide") << QSize(1100, 825) << 72 << false;
    }

    void screenshot()
    {
        QFETCH(const QSize, size);
        QFETCH(const int, dpi);
        QFETCH(const bool, fillSize);

        OpenedUrls openedUrls;
        QDesktopServices::setUrlHandler(u""_s, &openedUrls, "open");
        QDesktopServices::setUrlHandler(u"file"_s, &openedUrls, "open");
        const auto unsetUrlHandlers = qScopeGuard([]
        {
            QDesktopServices::unsetUrlHandler(u""_s);
            QDesktopServices::unsetUrlHandler(u"file"_s);
        });

        // Nothing is opened on the desktop
        QDesktopServices::openUrl(QUrl(_dir.filePath(u"check.png"_s)).toLocalFile());
        QCOMPARE(openedUrls._urls.size(), 1);

        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());
        grab();

        const auto path = QUrl::fromLocalFile(_dir.filePath(u"screenshot.png"_s));
        QVERIFY(execute([&]
        {
            _graphDisplay->captureScreenshot(size.width(), size.height(), path.toString(), dpi, fillSize);
        }));

        QCOMPARE(openedUrls._urls.size(), 2);

        const QImage image(path.toLocalFile());
        QCOMPARE(image.size(), size);
        QCOMPARE(image.dotsPerMeterX(), static_cast<int>(dpi * 39.3700787));

        QVERIFY(find(image, Qt::red)._pixels > 100);
        QVERIFY(find(image, Qt::green)._pixels > 100);
        QVERIFY(find(image, Qt::yellow)._pixels > 100);

        // Fitting the window's shape into a taller picture leaves bands of background above and below
        const bool banded = !fillSize && size.width() * Height < size.height() * Width;
        if(banded)
        {
            const int bandHeight = (size.height() - (size.width() * Height / Width)) / 2;
            for(int x = 0; x < size.width(); x++)
            {
                QVERIFY(isBackground(image.pixelColor(x, 0)));
                QVERIFY(isBackground(image.pixelColor(x, bandHeight - 1)));
            }
        }
    }

    void labels()
    {
        setPref(u"visuals/showNodeText"_s, static_cast<int>(TextState::Off));

        QVERIFY(openTriangle());
        QVERIFY(colourTriangle());

        const auto hidden = grab();
        const auto a = find(hidden, Qt::red);
        const auto b = find(hidden, Qt::green);
        const auto c = find(hidden, Qt::yellow);

        QVERIFY(changeVisuals([this] { setPref(u"visuals/showNodeText"_s, static_cast<int>(TextState::All)); }));
        const auto shown = grab();

        QVERIFY(darkPixels(shown) > darkPixels(hidden) + 100);

        // Each label is to the right of its node
        for(const auto& patch : {a, b, c})
        {
            const auto right = patch._centre + QPointF(radiusOf(patch) * 2.0, 0.0);
            QVERIFY(differences(hidden, shown, right, radiusOf(patch)) > 5);
        }

        QVERIFY(changeVisuals([this] { setPref(u"visuals/showNodeText"_s, static_cast<int>(TextState::Off)); }));
        QCOMPARE(grab(), hidden);
    }
};

QTEST_MAIN(TestRendering)
#include "tst_rendering.moc"
