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

#include "shared/utils/json.h"
#include "shared/loading/iparser.h"
#include "shared/graph/elementid.h"

#include <QTest>
#include <QByteArray>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <string>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
class TestParser : public IParser
{
public:
    bool parse(const QUrl&, IGraphModel*) override { return false; }
};
} // namespace

// json values are compared as strings, as QCOMPARE can't compare them on all platforms
class TestJson : public QObject
{
    Q_OBJECT

private slots:
    void toJson()
    {
        json j;
        j["string"] = u"ünïcode \"quoted\""_s;
        j["node"] = NodeId(3);
        j["edge"] = EdgeId(4);
        j["component"] = ComponentId(5);

        QCOMPARE(QString::fromStdString(j.dump()),
            u"{\"component\":5,\"edge\":4,\"node\":3,\"string\":\"ünïcode \\\"quoted\\\"\"}"_s);
    }

    void fromJson()
    {
        auto j = json::parse("{\"s\": \"ünïcode\", \"u\": \"https://graphia.app/x\"}");

        QCOMPARE(j["s"].get<QString>(), u"ünïcode"_s);
        QCOMPARE(j["u"].get<QUrl>(), QUrl(u"https://graphia.app/x"_s));
    }

    void toVariant()
    {
        auto j = json::parse("{\"null\": null, \"bool\": true, \"int\": -3, \"unsigned\": 18446744073709551615, "
            "\"float\": 1.5, \"string\": \"s\", \"array\": [1, \"a\", [2]], \"object\": {\"a\": {\"b\": 1}}}");

        auto variant = j.get<QVariant>();
        QCOMPARE(variant.typeId(), QMetaType::QVariantMap);

        auto map = variant.toMap();
        QCOMPARE(map.size(), 8);
        QVERIFY(!map.value(u"null"_s).isValid());
        QCOMPARE(map.value(u"bool"_s), QVariant(true));
        QCOMPARE(map.value(u"int"_s), QVariant(qint64{-3}));
        QCOMPARE(map.value(u"unsigned"_s), QVariant(quint64{18446744073709551615ULL}));
        QCOMPARE(map.value(u"float"_s), QVariant(1.5));
        QCOMPARE(map.value(u"string"_s), QVariant(u"s"_s));
        QCOMPARE(map.value(u"array"_s).toList(),
            (QVariantList{qint64{1}, u"a"_s, QVariantList{qint64{2}}}));
        QCOMPARE(map.value(u"object"_s).toMap().value(u"a"_s).toMap().value(u"b"_s), QVariant(qint64{1}));
    }

    void jsonArrayFrom()
    {
        auto j = ::jsonArrayFrom({u"a"_s, u"b"_s});
        QCOMPARE(QString::fromStdString(j.dump()), u"[\"a\",\"b\"]"_s);
    }

    void parseJsonFrom()
    {
        TestParser parser;
        auto j = ::parseJsonFrom(QByteArray("{\"a\": [1, 2]}"), &parser);

        QCOMPARE(QString::fromStdString(j.dump()), u"{\"a\":[1,2]}"_s);
        QVERIFY(parser.failureReason().isEmpty());
    }

    void parseJsonFromMalformed()
    {
        TestParser parser;
        auto j = ::parseJsonFrom(QByteArray("{\"a\": [1, 2}"), &parser);

        QVERIFY(j.is_null() || j.is_discarded());
        QVERIFY(!parser.failureReason().isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestJson)
#include "tst_json.moc"
