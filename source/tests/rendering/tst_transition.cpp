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

#include "app/rendering/transition.h"

#include <QSignalSpy>
#include <QStringList>
#include <QTest>

#include <algorithm>
#include <vector>

using namespace Qt::Literals::StringLiterals;

class TestTransition : public QObject
{
    Q_OBJECT

private slots:
    void progressesOverItsDuration()
    {
        Transition transition;
        const QSignalSpy started(&transition, &Transition::started);
        const QSignalSpy finished(&transition, &Transition::finished);
        QVERIFY(!transition.active());

        std::vector<float> progress;
        transition.start(2.0f, Transition::Type::Linear, [&](float f) { progress.push_back(f); });
        QVERIFY(transition.active());
        QCOMPARE(started.size(), 1);

        QVERIFY(transition.update(0.5f));
        QVERIFY(transition.update(1.0f));
        QVERIFY(!transition.update(1.0f));

        QCOMPARE(progress, (std::vector<float>{0.25f, 0.75f, 1.0f}));
        QVERIFY(!transition.active());
        QCOMPARE(finished.size(), 1);

        // Once finished, updates do nothing
        QVERIFY(!transition.update(1.0f));
        QCOMPARE(progress.size(), 3u);
        QCOMPARE(finished.size(), 1);
    }

    void everyTypeEndsAtOne_data()
    {
        QTest::addColumn<int>("type");
        QTest::newRow("Linear") << static_cast<int>(Transition::Type::Linear);
        QTest::newRow("EaseInEaseOut") << static_cast<int>(Transition::Type::EaseInEaseOut);
        QTest::newRow("Power") << static_cast<int>(Transition::Type::Power);
        QTest::newRow("InversePower") << static_cast<int>(Transition::Type::InversePower);
    }

    void everyTypeEndsAtOne()
    {
        QFETCH(const int, type);

        Transition transition;
        std::vector<float> progress;
        transition.start(1.0f, static_cast<Transition::Type>(type), [&](float f) { progress.push_back(f); });

        while(transition.update(0.1f)) {}

        QVERIFY(progress.size() >= 10u);
        QVERIFY(progress.front() > 0.0f);
        QCOMPARE(progress.back(), 1.0f);
        QVERIFY(std::ranges::is_sorted(progress));
    }

    void zeroDurationFinishesOnTheNextUpdate()
    {
        Transition transition;
        float last = 0.0f;
        transition.start(0.0f, Transition::Type::Linear, [&](float f) { last = f; });
        QVERIFY(transition.active());

        QVERIFY(!transition.update(0.01f));
        QCOMPARE(last, 1.0f);
        QVERIFY(!transition.active());
    }

    void finishedFunctionsRunInOrder()
    {
        Transition transition;
        QStringList calls;

        transition.start(1.0f, Transition::Type::Linear, [](float) {})
            .then([&] { calls.append(u"first"_s); })
            .then([&] { calls.append(u"second"_s); })
            .alternativeThen([&] { calls.append(u"replacement"_s); });

        QVERIFY(transition.update(0.5f));
        QVERIFY(calls.isEmpty());

        QVERIFY(!transition.update(0.5f));
        QCOMPARE(calls, (QStringList{u"first"_s, u"replacement"_s}));
    }

    void restartingReplacesTheFunctions()
    {
        Transition transition;
        const QSignalSpy started(&transition, &Transition::started);
        QStringList calls;

        transition.start(1.0f, Transition::Type::Linear, [&](float) { calls.append(u"old"_s); })
            .then([&] { calls.append(u"old finished"_s); });
        QVERIFY(transition.update(0.5f));

        transition.start(1.0f, Transition::Type::Linear, [&](float) { calls.append(u"new"_s); });
        QCOMPARE(started.size(), 1);

        while(transition.update(0.5f)) {}
        QCOMPARE(calls, (QStringList{u"old"_s, u"new"_s, u"new"_s}));
    }

    void cancelStopsTheTransition()
    {
        Transition transition;
        int calls = 0;

        transition.start(1.0f, Transition::Type::Linear, [&](float) { calls++; });
        QVERIFY(transition.update(0.5f));

        transition.cancel();
        QVERIFY(!transition.active());
        QVERIFY(!transition.update(0.5f));
        QCOMPARE(calls, 1);
    }

    // A chain of transitions is signalled as one
    void reusedTransitionSignalsOnce()
    {
        Transition transition;
        const QSignalSpy started(&transition, &Transition::started);
        const QSignalSpy finished(&transition, &Transition::finished);

        transition.start(1.0f, Transition::Type::Linear, [](float) {})
            .then([&] { transition.willBeImmediatelyReused(); });
        QVERIFY(!transition.update(1.0f));
        QCOMPARE(finished.size(), 0);

        transition.start(1.0f, Transition::Type::Linear, [](float) {});
        QCOMPARE(started.size(), 1);

        QVERIFY(!transition.update(1.0f));
        QCOMPARE(started.size(), 1);
        QCOMPARE(finished.size(), 1);
    }
};

QTEST_GUILESS_MAIN(TestTransition)
#include "tst_transition.moc"
