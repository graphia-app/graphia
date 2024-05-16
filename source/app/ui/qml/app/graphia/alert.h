/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#ifndef ALERT_H
#define ALERT_H

#include "shared/utils/qmlenum.h"

#include <QString>

#include <vector>
#include <map>
#include <utility>

DEFINE_QML_ENUM(
    Q_OBJECT, AlertType,
    None,
    Warning,
    Error);

struct Alert
{
    AlertType _type = AlertType::None;
    QString _text;

    Alert() = default;
    Alert(AlertType type, const QString& text) :
        _type(type), _text(text)
    {}
};

using AlertsMap = std::map<int, std::vector<Alert>>;

#endif // ALERT_H
