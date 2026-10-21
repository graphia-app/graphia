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

#ifndef LEGACYNAMES_H
#define LEGACYNAMES_H

#include "shared/utils/json_fwd.h"

#include <QString>

#include <functional>
#include <map>
#include <set>
#include <vector>

class GraphTransformFactory;
class GraphTransformParameter;

// Before version 9, transforms, their parameters and choices, visualisation channels and the attributes
// that Graphia, transforms and plugins create were named in en_GB with spaces and punctuation, e.g.
// "Remove Nodes" and "k-NN Source Rank" rather than more simple language agnostic values that we now use
// "RemoveNodes" and "KNNSourceRank"; this renames references to them, leaving any name it doesn't know as it is
class LegacyNames
{
public:
    using TransformFactoryFn = std::function<const GraphTransformFactory*(const QString&)>;
    using AttributeIsDeclaredFn = std::function<bool(const QString&)>;

    LegacyNames(TransformFactoryFn transformFactory, AttributeIsDeclaredFn attributeIsDeclared,
        std::set<QString> userAttributeNames = {});

    QString transformName(const QString& name) const;
    QString attributeName(const QString& name) const;

    // Transforms are renamed in order, before anything that refers to the attributes they create
    QString transform(const QString& transform);
    QString visualisation(const QString& visualisation) const;

    // Renames the attribute names, or table columns named after one, that are the values of
    // the given keys of a JSON object, either as strings or within arrays
    void renameAttributesIn(json& jsonObject, const std::vector<const char*>& keys) const;

private:
    TransformFactoryFn _transformFactory;
    AttributeIsDeclaredFn _attributeIsDeclared;
    std::set<QString> _userAttributeNames;

    // Attributes named after others, which the rule can't rename
    std::map<QString, QString> _attributeNames;

    static QString compactName(QString name);
    static QString parameterName(const GraphTransformFactory* factory, const QString& name);
    static QString choiceValue(const GraphTransformParameter& parameter, const QString& value);
    static QString channelName(const QString& name);

    void renameAttributesInValue(json& jsonValue) const;
};

// Replaces each string in a JSON array with what renameFn returns for it
void renameEachIn(json& jsonArray, const std::function<QString(const QString&)>& renameFn);

#endif // LEGACYNAMES_H
