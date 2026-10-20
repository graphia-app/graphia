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

#include "shared/utils/circularbuffer.h"
#include "shared/utils/container.h"
#include "shared/utils/fixedsizestack.h"
#include "shared/utils/flags.h"

#include <QTest>

#include <algorithm>
#include <vector>

namespace
{
enum class Colour : unsigned char
{
    None    = 0x0,
    Red     = 0x1,
    Green   = 0x2,
    Blue    = 0x4,
    Cyan    = Green|Blue
};

template<typename T, size_t Size>
std::vector<T> contentsOf(const CircularBuffer<T, Size>& buffer)
{
    std::vector<T> contents;
    contents.reserve(buffer.size());
    for(size_t i = 0; i < buffer.size(); i++)
        contents.push_back(buffer.at(i));

    return contents;
}
} // namespace

class TestDataStructures : public QObject
{
    Q_OBJECT

private slots:
    void circularBuffer()
    {
        CircularBuffer<int, 3> buffer;
        QCOMPARE(buffer.size(), 0u);
        QVERIFY(!buffer.full());

        buffer.push_back(1);
        buffer.push_back(2);
        QCOMPARE(contentsOf(buffer), (std::vector<int>{1, 2}));
        QCOMPARE(buffer.oldest(), 1);
        QCOMPARE(buffer.newest(), 2);
        QVERIFY(!buffer.full());

        buffer.push_back(3);
        QVERIFY(buffer.full());
        QCOMPARE(contentsOf(buffer), (std::vector<int>{1, 2, 3}));

        // The oldest values are overwritten
        buffer.push_back(4);
        buffer.push_back(5);
        QCOMPARE(buffer.size(), 3u);
        QCOMPARE(contentsOf(buffer), (std::vector<int>{3, 4, 5}));
        QCOMPARE(buffer.oldest(), 3);
        QCOMPARE(buffer.newest(), 5);

        buffer.at(0) = 30;
        QCOMPARE(buffer.oldest(), 30);

        buffer.clear();
        QCOMPARE(buffer.size(), 0u);
        buffer.push_back(6);
        QCOMPARE(contentsOf(buffer), (std::vector<int>{6}));
    }

    void circularBufferWrapsRepeatedly()
    {
        CircularBuffer<int, 4> buffer;

        for(int i = 0; i < 23; i++)
        {
            buffer.push_back(i);

            const int first = std::max(0, i - 3);
            std::vector<int> expected(static_cast<size_t>(i - first + 1));
            u::iota(expected, first);

            QCOMPARE(contentsOf(buffer), expected);
        }
    }

    void circularBufferFill()
    {
        CircularBuffer<int, 3> buffer;
        buffer.push_back(1);
        buffer.fill(7);

        QVERIFY(buffer.full());
        QCOMPARE(contentsOf(buffer), (std::vector<int>{7, 7, 7}));

        buffer.push_back(8);
        QCOMPARE(contentsOf(buffer), (std::vector<int>{7, 7, 8}));
        QCOMPARE(buffer.newest(), 8);
    }

    void circularBufferMean()
    {
        CircularBuffer<float, 4> buffer;

        for(const float v : {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f})
            buffer.push_back(v);

        // Holds 3, 4, 5, 6
        QCOMPARE(buffer.mean(), 4.5f);
        QCOMPARE(buffer.mean(2), 5.5f);
        QCOMPARE(buffer.mean(10), 4.5f);
    }

    void fixedSizeStack()
    {
        FixedSizeStack<int> stack(2);
        QCOMPARE(stack.size(), 2u);
        QVERIFY(stack.empty());

        stack.push(1);
        stack.push(2);
        QVERIFY(!stack.empty());
        QCOMPARE(stack.top(), 2);

        stack.top() = 3;
        QCOMPARE(stack.pop(), 3);
        QCOMPARE(stack.pop(), 1);
        QVERIFY(stack.empty());

        stack.push(4);
        QCOMPARE(stack.top(), 4);
    }

    void flags()
    {
        Flags<Colour> flags;
        QVERIFY(!flags.test(Colour::Red));
        QCOMPARE(*flags, Colour::None);

        flags.set(Colour::Red, Colour::Blue);
        QVERIFY(flags.test(Colour::Red));
        QVERIFY(!flags.test(Colour::Green));
        QVERIFY(flags.test(Colour::Blue));
        QVERIFY(flags.anyOf(Colour::Green, Colour::Blue));
        QVERIFY(!flags.anyOf(Colour::Green));
        QVERIFY(flags.allOf(Colour::Red, Colour::Blue));
        QVERIFY(!flags.allOf(Colour::Red, Colour::Green));

        flags.reset(Colour::Red);
        QCOMPARE(*flags, Colour::Blue);

        flags.setState(Colour::Green, true);
        QCOMPARE(*flags, Colour::Cyan);
        flags.setState(Colour::Green, false);
        QCOMPARE(*flags, Colour::Blue);

        QCOMPARE(Flags<Colour>::combine(Colour::Green, Colour::Blue), Colour::Cyan);
        QVERIFY(Flags<Colour>(Colour::Red, Colour::Green) != Flags<Colour>(Colour::Red)); // clazy:exclude=compare-member-check
        QVERIFY(!(Flags<Colour>(Colour::Cyan) != Flags<Colour>(Colour::Green, Colour::Blue))); // clazy:exclude=compare-member-check

        // Testing a combination tests for any of its flags
        QVERIFY(Flags<Colour>(Colour::Green).test(Colour::Cyan));
    }
};

QTEST_GUILESS_MAIN(TestDataStructures)
#include "tst_datastructures.moc"
