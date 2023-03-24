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

#include "shared/utils/threadpool.h"

#include <QObject>
#include <QTest>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace Qt::Literals::StringLiterals;

class TestThreadPool : public QObject
{
    Q_OBJECT

private:
    struct HintedElement
    {
        uint64_t _cost = 1;

        uint64_t computeCostHint() const { return _cost; }
    };

    struct UnhintedElement {};

    template<typename Element>
    static int elementsGivenToFirstOfTwoThreads(const std::vector<Element>& elements)
    {
        ThreadPool threadPool(u"TestThreadPool"_s, 2);
        std::atomic<int> count(0);

        threadPool.parallel_for(elements.begin(), elements.end(),
        [&count](const Element&, size_t threadIndex)
        {
            if(threadIndex == 0)
                count++;
        });

        return count;
    }

private slots:
    void costHintsWeightTheSplitBetweenThreads()
    {
        std::vector<HintedElement> elements(100);
        elements.front()._cost = 1000;

        QCOMPARE(elementsGivenToFirstOfTwoThreads(elements), 1);
    }

    void elementsWithoutCostHintsAreSplitEvenly()
    {
        const std::vector<UnhintedElement> elements(100);

        QCOMPARE(elementsGivenToFirstOfTwoThreads(elements), 50);
    }
};

QTEST_GUILESS_MAIN(TestThreadPool)
#include "tst_threadpool.moc"
