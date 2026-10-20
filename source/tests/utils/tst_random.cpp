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

#include "shared/utils/random.h"
#include "app/attributes/enrichmentcalculator.h"

#include <QTest>
#include <QColor>

#include <cmath>
#include <random>
#include <vector>

class TestRandom : public QObject
{
    Q_OBJECT

    static std::vector<float> sequence()
    {
        std::vector<float> values;
        values.reserve(10);
        for(int i = 0; i < 10; i++)
            values.push_back(u::rand(0.0f, 100.0f));

        values.push_back(static_cast<float>(u::rand(0, 1000)));
        values.push_back(u::randQVector3D(-1.0f, 1.0f).z());

        return values;
    }

private slots:
    void seed()
    {
        u::setRandomSeed(42);
        auto a = sequence();

        u::setRandomSeed(42);
        auto b = sequence();
        QCOMPARE(a, b);

        u::setRandomSeed(43);
        auto c = sequence();
        QVERIFY(a != c);
    }

    void seededRandomGenerator()
    {
        u::setRandomSeed(7);

        // Each generator starts from the same seed
        auto a = u::seededRandomGenerator();
        auto b = u::seededRandomGenerator();
        QCOMPARE(a(), b());
        QCOMPARE(a(), b());

        // A different seed gives a different sequence
        u::setRandomSeed(8);
        QVERIFY(u::seededRandomGenerator()() != std::mt19937(7)()); // NOLINT cert-msc32-c,cert-msc51-cpp
    }

    void ranges()
    {
        for(int i = 0; i < 1000; i++)
        {
            auto f = u::rand(-2.0f, 3.0f);
            QVERIFY(f >= -2.0f && f <= 3.0f);

            // Inclusive
            auto n = u::rand(1, 3);
            QVERIFY(n >= 1 && n <= 3);

            auto v = u::randQVector2D(0.0f, 1.0f);
            QVERIFY(v.x() >= 0.0f && v.x() <= 1.0f && v.y() >= 0.0f && v.y() <= 1.0f);

            QVERIFY(u::randQColor().isValid());
        }
    }

    void enrichmentSamplingIsReproducible()
    {
        u::setRandomSeed(1);
        auto a = EnrichmentCalculator::doRandomSampling(20, 0.3);
        auto b = EnrichmentCalculator::doRandomSampling(20, 0.3);
        QCOMPARE(a, b);

        // The mean observed frequency is about the expected frequency
        QVERIFY(std::abs(a.at(2) - 0.3) < 0.02);
    }
};

QTEST_GUILESS_MAIN(TestRandom)
#include "tst_random.moc"
