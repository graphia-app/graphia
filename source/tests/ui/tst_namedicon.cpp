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

#include "app/ui/qml/Graphia/Controls/namedicon.h"

#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QQuickWindow>
#include <QScreen>
#include <QSignalSpy>
#include <QSize>
#include <QString>
#include <QTest>

#include <cmath>

using namespace Qt::Literals::StringLiterals;

// Runs on the screens in namedicon_screens.json, as what's drawn depends on the
// window's screen differing from the application's highest device pixel ratio
class TestNamedIcon : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        QIcon::setThemeSearchPaths({u"" ICON_THEMES_DIR ""_s});
        QIcon::setThemeName(u"Tango"_s);

        QCOMPARE(QGuiApplication::screens().size(), 4);
        QCOMPARE(qApp->devicePixelRatio(), 2.0);
    }

    void drawsOneToOneCentredForTheWindowsScreen_data()
    {
        QTest::addColumn<QString>("screenName");
        QTest::addColumn<int>("itemSize");

        QTest::newRow("1x, downscaled") << u"1x"_s << 24;
        QTest::newRow("1x, exact") << u"1x"_s << 16;
        QTest::newRow("1.5x, smaller than the item") << u"1.5x"_s << 24;
        QTest::newRow("1.75x, smaller than the item") << u"1.75x"_s << 24;
        QTest::newRow("2x, smaller than the item") << u"2x"_s << 24;
        QTest::newRow("2x, exact") << u"2x"_s << 16;
    }

    void drawsOneToOneCentredForTheWindowsScreen()
    {
        QFETCH(const QString, screenName);
        QFETCH(const int, itemSize);

        const auto screens = QGuiApplication::screens();
        QScreen* screen = nullptr;
        for(auto* candidate : std::as_const(screens))
        {
            if(candidate->name() == screenName)
                screen = candidate;
        }
        QVERIFY(screen != nullptr);

        if (screen == nullptr)
        {
            QFAIL("Could not find screen");
            return;
        }

        QQuickWindow window;
        window.setScreen(screen);
        const auto devicePixelRatio = window.devicePixelRatio();
        QCOMPARE(devicePixelRatio, screen->devicePixelRatio());

        const auto iconName = u"document-open"_s;
        NamedIcon namedIcon;
        namedIcon.setParentItem(window.contentItem());
        namedIcon.setSize(QSizeF(itemSize, itemSize));
        namedIcon.setIconName(iconName);
        QVERIFY(namedIcon.valid());

        const auto textureSize = static_cast<int>(std::lround(itemSize * devicePixelRatio));
        QImage painted(textureSize, textureSize, QImage::Format_ARGB32_Premultiplied);
        painted.fill(Qt::transparent);
        QPainter painter(&painted);
        painter.scale(devicePixelRatio, devicePixelRatio);
        namedIcon.paint(&painter);
        painter.end();

        auto source = QIcon::fromTheme(iconName).pixmap(QSize(itemSize, itemSize), devicePixelRatio).toImage();
        source.setDevicePixelRatio(1.0);
        QImage expected(textureSize, textureSize, QImage::Format_ARGB32_Premultiplied);
        expected.fill(Qt::transparent);
        QPainter expectedPainter(&expected);
        expectedPainter.drawImage((textureSize - source.width()) / 2, (textureSize - source.height()) / 2, source);
        expectedPainter.end();

        QCOMPARE(painted, expected);
    }

    void emitsValidChangedOnlyWhenValidityChanges()
    {
        NamedIcon namedIcon;
        const QSignalSpy spy(&namedIcon, &NamedIcon::validChanged);
        QVERIFY(!namedIcon.valid());

        namedIcon.setIconName(u"document-open"_s);
        QVERIFY(namedIcon.valid());
        QCOMPARE(spy.count(), 1);

        namedIcon.setIconName(u"document-save"_s);
        QVERIFY(namedIcon.valid());
        QCOMPARE(spy.count(), 1);

        namedIcon.setIconName(u"no-such-icon"_s);
        QVERIFY(!namedIcon.valid());
        QCOMPARE(spy.count(), 2);

        namedIcon.setIconName(u"another-missing-icon"_s);
        QVERIFY(!namedIcon.valid());
        QCOMPARE(spy.count(), 2);
    }
};

QTEST_MAIN(TestNamedIcon)
#include "tst_namedicon.moc"
