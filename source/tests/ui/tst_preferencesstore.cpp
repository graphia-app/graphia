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

#include "app/defaultpreferences.h"
#include "app/ui/qml/Graphia/Utils/preferences.h"

#include "shared/utils/preferences.h"

#include <QCoreApplication>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QTemporaryDir>
#include <QTest>
#include <QVariant>

#include <memory>
#include <thread>

using namespace Qt::Literals::StringLiterals;

class TestPreferencesStore : public QObject
{
    Q_OBJECT

    QTemporaryDir _settingsDir;
    std::unique_ptr<PreferencesStore> _store;

private slots:
    void initTestCase()
    {
        const QTemporaryDir previousRunDir;

        QCoreApplication::setOrganizationName(u"GraphiaTests"_s);
        QCoreApplication::setApplicationName(u"GraphiaTests"_s);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, previousRunDir.path());

        QSettings(QSettings::IniFormat, QSettings::UserScope,
            QCoreApplication::organizationName(), QCoreApplication::applicationName())
            .setValue(u"test/previousRun"_s, 1);

        _store = std::make_unique<PreferencesStore>();

        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, _settingsDir.path());

        initialisePreferences();
    }

    // Runs first, so the event loop can't have saved them since they were defined
    void defaultsAreInStorage()
    {
        QVERIFY(u::getPref(u"servers/redirects"_s).isValid());
    }

    // The settings have moved since they were loaded, so storage can't be what answers
    void readsComeFromMemory()
    {
        QVERIFY(!u::getPref(u"test/previousRun"_s).isValid());
        QVERIFY(u::prefExists(u"test/previousRun"_s));
        QCOMPARE(u::pref(u"test/previousRun"_s).toInt(), 1);
    }

    // On wasm, this hangs if the worker's read needs the main thread
    void readOnWorkerWhileMainThreadWaits()
    {
        u::setPref(u"test/onWorker"_s, 2);

        bool exists = false;
        QVariant value;

        std::thread worker([&exists, &value]
        {
            exists = u::prefExists(u"test/onWorker"_s);
            value = u::pref(u"test/onWorker"_s);
        });

        worker.join();

        QVERIFY(exists);
        QCOMPARE(value.toInt(), 2);
    }

    // On wasm, this hangs if the worker's write needs the main thread
    void writeOnWorkerWhileMainThreadWaits()
    {
        std::thread worker([] { u::setPref(u"test/writtenOnWorker"_s, 3); });
        worker.join();

        QCOMPARE(u::pref(u"test/writtenOnWorker"_s).toInt(), 3);
        QVERIFY(!u::getPref(u"test/writtenOnWorker"_s).isValid());
        QTRY_COMPARE(u::getPref(u"test/writtenOnWorker"_s).toInt(), 3);
    }

    void changesReachStorageFromTheEventLoop()
    {
        u::setPref(u"test/written"_s, 4);
        QVERIFY(!u::getPref(u"test/written"_s).isValid());
        QTRY_COMPARE(u::getPref(u"test/written"_s).toInt(), 4);

        QVERIFY(u::removePref(u"test/written"_s));
        QCOMPARE(u::getPref(u"test/written"_s).toInt(), 4);
        QTRY_VERIFY(!u::getPref(u"test/written"_s).isValid());
    }

    void pendingChangesCanBeSavedImmediately()
    {
        u::setPref(u"test/savedNow"_s, 5);
        u::savePendingPrefs();
        QCOMPARE(u::getPref(u"test/savedNow"_s).toInt(), 5);
    }

    void readingUndefinedLeavesItUndefined()
    {
        QVERIFY(!u::pref(u"test/undefined"_s).isValid());
        QVERIFY(!u::prefExists(u"test/undefined"_s));
    }

    // Destroys the store, so this comes last
    void pendingChangesAreSavedOnDestruction()
    {
        u::setPref(u"test/savedOnDestruction"_s, 6);

        _store.reset();

        // The queued save would otherwise outlive the store
        QCoreApplication::removePostedEvents(QCoreApplication::instance());

        QCOMPARE(u::getPref(u"test/savedOnDestruction"_s).toInt(), 6);
    }
};

QTEST_GUILESS_MAIN(TestPreferencesStore)
#include "tst_preferencesstore.moc"
