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

#include "app/ui/qml/Graphia/Utils/modelcompleter.h"
#include "app/ui/qml/Graphia/Utils/sortfilterproxymodel.h"

#include <QModelIndex>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QVariant>

#include <memory>
#include <utility>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr int KindRole = Qt::UserRole + 1;

// A list of names, each of a kind, as the lists of attributes in dialogs have
std::unique_ptr<QStandardItemModel> createModel(const std::vector<std::pair<QString, QString>>& namesAndKinds)
{
    auto model = std::make_unique<QStandardItemModel>();
    model->setItemRoleNames({{Qt::DisplayRole, "display"}, {KindRole, "kind"}});

    for(const auto& [name, kind] : namesAndKinds)
    {
        auto* item = new QStandardItem(name);
        item->setData(kind, KindRole);
        model->appendRow(item);
    }

    return model;
}

QStringList namesOf(const QAbstractItemModel& model)
{
    QStringList names;
    names.reserve(model.rowCount());
    for(int row = 0; row < model.rowCount(); row++)
        names.append(model.data(model.index(row, 0)).toString());

    return names;
}

QStringList namesOf(const QList<QModelIndex>& indexes)
{
    QStringList names;
    names.reserve(indexes.size());
    for(const auto& index : indexes)
        names.append(index.data().toString());

    return names;
}
} // namespace

// The models QML uses to sort, filter and complete lists
class TestModelUtils : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        TestEnvironment::initialise();
    }

    void sortFilterProxyModel()
    {
        auto model = createModel({{u"d"_s, u"x"_s}, {u"b"_s, u"y"_s}, {u"c"_s, u"x"_s}, {u"a"_s, u"y"_s}});

        SortFilterProxyModel proxy;
        const QSignalSpy countChanged(&proxy, &SortFilterProxyModel::countChanged);
        proxy.setSourceModel(model.get());
        QVERIFY(!countChanged.isEmpty());
        QCOMPARE(proxy.property("count").toInt(), 4);

        QCOMPARE(proxy.role(u"kind"_s), KindRole);
        QCOMPARE(proxy.role({}), static_cast<int>(Qt::DisplayRole));

        // By default, by name
        QCOMPARE(namesOf(proxy), (QStringList{u"a"_s, u"b"_s, u"c"_s, u"d"_s}));
        QCOMPARE(proxy.get(0).toString(), u"a"_s);
        QCOMPARE(proxy.get(0, u"kind"_s).toString(), u"y"_s);

        proxy.setProperty("ascendingSortOrder", false);
        QCOMPARE(namesOf(proxy), (QStringList{u"d"_s, u"c"_s, u"b"_s, u"a"_s}));

        // By kind; equal kinds keep their order in the source model
        proxy.setProperty("ascendingSortOrder", true);
        proxy.setProperty("sortRoleName", u"kind"_s);
        QCOMPARE(namesOf(proxy), (QStringList{u"d"_s, u"c"_s, u"b"_s, u"a"_s}));

        // Filtering
        proxy.setProperty("filterRoleName", u"kind"_s);
        proxy.setFilterRegularExpression(QRegularExpression(u"^x$"_s));
        QCOMPARE(namesOf(proxy), (QStringList{u"d"_s, u"c"_s}));
        QCOMPARE(proxy.property("count").toInt(), 2);

        // Rows added to the source are sorted and filtered
        auto* item = new QStandardItem(u"e"_s);
        item->setData(u"x"_s, KindRole);
        model->appendRow(item);
        QCOMPARE(namesOf(proxy), (QStringList{u"d"_s, u"c"_s, u"e"_s}));
        QCOMPARE(proxy.property("count").toInt(), 3);
    }

    void modelCompleter()
    {
        auto model = createModel({{u"Node Degree"_s, {}}, {u"Node In Degree"_s, {}}, {u"Edge Weight"_s, {}}});

        // A child, as attributes have parameters, such as an edge's source node's attributes
        auto* child = new QStandardItem(u"node degree source"_s);
        model->item(2)->appendRow(child);

        ModelCompleter completer;
        completer.setProperty("model", QVariant::fromValue<QAbstractItemModel*>(model.get()));
        QVERIFY(completer.property("candidates").value<QList<QModelIndex>>().isEmpty());

        // Case insensitively
        completer.setProperty("startsWith", u"node"_s);
        QCOMPARE(namesOf(completer.property("candidates").value<QList<QModelIndex>>()),
            (QStringList{u"Node Degree"_s, u"Node In Degree"_s, u"node degree source"_s}));
        QCOMPARE(completer.property("commonPrefix").toString(), u"Node "_s);
        QCOMPARE(completer.property("closestMatch").toModelIndex().data().toString(), u"Node Degree"_s);

        completer.setProperty("startsWith", u"Node D"_s);
        QCOMPARE(completer.property("commonPrefix").toString(), u"Node Degree"_s);

        completer.setProperty("startsWith", u"x"_s);
        QVERIFY(completer.property("candidates").value<QList<QModelIndex>>().isEmpty());
        QVERIFY(completer.property("commonPrefix").toString().isEmpty());
        QVERIFY(!completer.property("closestMatch").toModelIndex().isValid());
    }
};

QTEST_GUILESS_MAIN(TestModelUtils)
#include "tst_modelutils.moc"
