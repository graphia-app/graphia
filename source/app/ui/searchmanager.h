/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include "findoptions.h"

#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"
#include "shared/utils/flags.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include <mutex>

class GraphModel;

// What to select when found nodes changes
DEFINE_QML_ENUM(
    Q_GADGET, FindSelectStyle,
    None, First, All);

class SearchManager : public QObject
{
    Q_OBJECT
public:
    explicit SearchManager(const GraphModel& graphModel);

    void findNodes(QString term, Flags<FindOptions> options,
        QStringList attributeNames, FindSelectStyle selectStyle);
    void clearFoundNodeIds();
    void refresh();

    NodeIdSet foundNodeIds() const;

    bool active() const { return !_term.isEmpty(); }

    FindSelectStyle selectStyle() const { return _selectStyle; }

private:
    QString _term;
    Flags<FindOptions> _options;
    QStringList _attributeNames;
    FindSelectStyle _selectStyle = FindSelectStyle::None;

    const GraphModel* _graphModel = nullptr;

    mutable std::recursive_mutex _mutex;
    NodeIdSet _foundNodeIds;

signals:
    void foundNodeIdsChanged(const SearchManager*);
};

#endif // SEARCHMANAGER_H
