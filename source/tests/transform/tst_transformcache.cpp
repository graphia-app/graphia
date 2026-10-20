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

#include "app/attributes/attribute.h"
#include "app/transform/graphtransformconfigparser.h"
#include "app/transform/transformcache.h"

#include "shared/utils/container.h"

#include <QString>
#include <QStringList>
#include <QTest>

#include <memory>

using namespace Qt::Literals::StringLiterals;

// The cache of transforms' results, which lets transforms that are unaffected by a
// change keep their results rather than being applied again
class TestTransformCache : public QObject
{
    Q_OBJECT

    std::unique_ptr<TestGraphModel> _model;

    // A transform's result, which refers to an attribute, and adds one
    static TransformCache::Result result(int index, const QString& referencedAttributeName,
        const QString& addedAttributeName)
    {
        GraphTransformConfigParser parser;
        const auto parsed = parser.parse(uR"(RemoveNodes where $"%1" > 1)"_s.arg(referencedAttributeName));
        Q_ASSERT(parsed);

        TransformCache::Result result;
        result._index = index;
        result._config = parser.result();
        result._addedOrChangedAttributes.emplace(addedAttributeName, Attribute{});

        return result;
    }

    // The attributes added by the results in the cache
    static QStringList cachedAttributeNames(const TransformCache& cache)
    {
        const auto keys = u::keysFor(cache.attributes());
        QStringList names(keys.begin(), keys.end());

        names.sort();
        return names;
    }

    // 0 refers to W and adds A; 1 refers to X and adds B, and depends on nothing 0 does, so
    // its result is in the same set; 2 refers to A, which 0 adds, so starts another, and adds C
    TransformCache createCache()
    {
        TransformCache cache(_model->graphModel());
        cache.add(result(0, u"W"_s, u"A"_s));
        cache.add(result(1, u"X"_s, u"B"_s));
        cache.add(result(2, u"A"_s, u"C"_s));

        return cache;
    }

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
        _model = std::make_unique<TestGraphModel>(QStringList{u"a-b"_s});
    }

    void cleanupTestCase()
    {
        _model.reset();
    }

    void unrelatedAttribute()
    {
        auto cache = createCache();
        cache.attributeAddedOrChanged(u"Y"_s);
        QCOMPARE(cachedAttributeNames(cache), (QStringList{u"A"_s, u"B"_s, u"C"_s}));
    }

    // An attribute that's added by a result invalidates it, as its name may need to change
    void addedAttribute()
    {
        auto cache = createCache();
        cache.attributeAddedOrChanged(u"C"_s);
        QCOMPARE(cachedAttributeNames(cache), (QStringList{u"A"_s, u"B"_s}));
    }

    // Results before an invalid one in its set don't depend on it, so stay cached
    void earlierResults()
    {
        auto cache = createCache();
        cache.attributeAddedOrChanged(u"X"_s);
        QCOMPARE(cachedAttributeNames(cache), QStringList{u"A"_s});
    }

    // Results that depend on an invalid one are invalid too
    void dependentResults()
    {
        auto cache = createCache();
        cache.attributeAddedOrChanged(u"W"_s);
        QVERIFY(cache.empty());
    }
};

QTEST_MAIN(TestTransformCache)
#include "tst_transformcache.moc"
