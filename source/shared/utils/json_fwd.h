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

#ifndef JSON_FWD_H
#define JSON_FWD_H

// Use this header when only forward declarations are required

#include <json_fwd.hpp>
using json = nlohmann::json;

#include <vector>

class NodeId;
class EdgeId;
class ComponentId;
class IParser;
class Progressable;

class QByteArray;
class QString;
class QUrl;
class QVariant; //clazy:exclude=qt6-fwd-fixes

void to_json(json& j, const QString& s);
void to_json(json& j, NodeId nodeId);
void to_json(json& j, EdgeId edgeId);
void to_json(json& j, ComponentId componentId);

void from_json(const json& j, QString& s);
void from_json(const json& j, QUrl& url);
void from_json(const json& j, QVariant& v);

json jsonArrayFrom(const std::vector<QString>& container, Progressable* progressable = nullptr);

json parseJsonFrom(const QByteArray& byteArray, IParser* parser = nullptr);
json parseJsonFrom(const QString& filename);

#endif // JSON_FWD_H
