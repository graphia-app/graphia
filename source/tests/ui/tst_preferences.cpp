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

#include "app/preferences_legacy.h"
#include "app/ui/qml/Graphia/limitconstants.h"
#include "app/ui/qml/Graphia/Utils/preferences.h"

#include "shared/utils/json.h"

#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QVariant>

#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace
{
// Not a raw string, which moc misparses here, missing the test class
const auto legacyTemplates = u"[{\"name\": \"t\", " // NOLINT modernize-raw-string-literal
    "\"transforms\": [\"\\\"Remove Nodes\\\" where $\\\"Node Degree\\\" < 2\"], "
    "\"visualisations\": [\"\\\"Node Degree\\\" \\\"Colour\\\"\"]}]"_s;
} // namespace

// Preferences, as stored in the settings file, and as the UI's Preferences items
// keep their properties in step with them
class TestPreferences : public QObject
{
    Q_OBJECT

    std::unique_ptr<QQmlEngine> _engine;

    // A Preferences item for the section "test", as in the UI's QML
    std::unique_ptr<QObject> createPreferences()
    {
        QQmlComponent component(_engine.get());
        component.setData("import Graphia.Utils\n"
            "Preferences { section: \"test\"; property int count: 1; property string name: \"x\" }", {});

        std::unique_ptr<QObject> object(component.create());
        if(object == nullptr)
            qWarning() << component.errorString();

        return object;
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        _engine = std::make_unique<QQmlEngine>();
    }

    void cleanupTestCase()
    {
        _engine.reset();
    }

    void prefs()
    {
        QVERIFY(!u::prefExists(u"test/a"_s));

        // Defining a preference sets it only if it isn't already set
        u::definePref(u"test/a"_s, 1);
        QCOMPARE(u::pref(u"test/a"_s).toInt(), 1);
        u::definePref(u"test/a"_s, 2);
        QCOMPARE(u::pref(u"test/a"_s).toInt(), 1);

        const PreferencesWatcher watcher;
        const QSignalSpy changed(&watcher, &PreferencesWatcher::preferenceChanged);

        u::setPref(u"test/a"_s, 3);
        QCOMPARE(u::pref(u"test/a"_s).toInt(), 3);
        QCOMPARE(changed.count(), 1);
        QCOMPARE(changed.at(0).at(0).toString(), u"test/a"_s);
        QCOMPARE(changed.at(0).at(1).toInt(), 3);

        // Setting the same value again isn't a change
        u::setPref(u"test/a"_s, 3);
        QCOMPARE(changed.count(), 1);

        QVERIFY(u::removePref(u"test/a"_s));
        QVERIFY(!u::prefExists(u"test/a"_s));
        QVERIFY(!u::removePref(u"test/a"_s));

        QVERIFY(!u::settingsFileName().isEmpty());
    }

    // Absolute node and edge sizes, from old versions, become normalised sizes
    void upgradeLegacySizes()
    {
        u::setPref(u"visuals/defaultNodeSize"_s, LimitConstants::maximumNodeSize());
        u::setPref(u"visuals/defaultEdgeSize"_s, LimitConstants::minimumEdgeSize());

        upgradeLegacyPreferences(TestGraphModel::application());

        QCOMPARE(u::pref(u"visuals/defaultNormalNodeSize"_s).toFloat(), 1.0f);
        QCOMPARE(u::pref(u"visuals/defaultNormalEdgeSize"_s).toFloat(), 0.0f);
        QVERIFY(!u::prefExists(u"visuals/defaultNodeSize"_s));
        QVERIFY(!u::prefExists(u"visuals/defaultEdgeSize"_s));

        // Once upgraded, there's nothing to upgrade
        u::setPref(u"visuals/defaultNormalNodeSize"_s, 0.5f);
        upgradeLegacyPreferences(TestGraphModel::application());
        QCOMPARE(u::pref(u"visuals/defaultNormalNodeSize"_s).toFloat(), 0.5f);
    }

    // Transforms named in favourites and templates by their names from before they were
    // compacted, are renamed; anything that isn't the name of a transform is left alone
    void upgradeLegacyNames()
    {
        u::setPref(u"misc/favouriteTransforms"_s, uR"(["Remove Nodes", "k-NN", "Unknown Transform"])"_s);
        u::setPref(u"misc/templates"_s, legacyTemplates);

        upgradeLegacyPreferences(TestGraphModel::application());

        auto jsonPref = [](const QString& key) { return parseJsonFrom(u::pref(key).toString().toUtf8()); };
        auto asString = [](const json& j) { return QString::fromStdString(j.is_string() ? j.get<std::string>() : j.dump()); };

        QCOMPARE(asString(jsonPref(u"misc/favouriteTransforms"_s)), uR"(["RemoveNodes","KNN","Unknown Transform"])"_s);

        // The names are renamed where they are, keeping their quotes
        const auto templates = jsonPref(u"misc/templates"_s);
        QCOMPARE(asString(templates.at(0)["transforms"].at(0)), uR"("RemoveNodes" where $"NodeDegree" < 2)"_s);
        QCOMPARE(asString(templates.at(0)["visualisations"].at(0)), uR"("NodeDegree" "Colour")"_s);
    }

    void preferencesItem()
    {
        // Values that aren't set yet keep their defaults
        auto a = createPreferences();
        QVERIFY(a != nullptr);
        QCOMPARE(a->property("section").toString(), u"test"_s);
        QCOMPARE(a->property("count").toInt(), 1);
        QCOMPARE(a->property("name").toString(), u"x"_s);

        // Changing a property sets the preference...
        a->setProperty("count", 5);
        QCOMPARE(u::pref(u"test/count"_s).toInt(), 5);

        // ...which an item created later starts with
        auto b = createPreferences();
        QVERIFY(b != nullptr);
        QCOMPARE(b->property("count").toInt(), 5);
        QCOMPARE(b->property("name").toString(), u"x"_s);

        // Setting the preference changes the properties of every item for it
        u::setPref(u"test/name"_s, u"y"_s);
        QCOMPARE(a->property("name").toString(), u"y"_s);
        QCOMPARE(b->property("name").toString(), u"y"_s);

        b->setProperty("count", 6);
        QCOMPARE(a->property("count").toInt(), 6);
    }
};

QTEST_MAIN(TestPreferences)
#include "tst_preferences.moc"
