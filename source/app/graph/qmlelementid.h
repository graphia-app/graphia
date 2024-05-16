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

#ifndef QMLELEMENTID_H
#define QMLELEMENTID_H

// This simply provides QML accessible version of the ElementId types

#include "app/application.h"

#include "shared/graph/elementid.h"

#include <QObject>
#include <QQmlEngine>

#define QML_TYPE(Type) Qml ## Type /* NOLINT cppcoreguidelines-macro-usage */
#define QML_ELEMENTID(Type) /* NOLINT cppcoreguidelines-macro-usage */ \
    class QML_TYPE(Type) \
    { \
        Q_GADGET \
        QML_ANONYMOUS \
        Q_PROPERTY(int id READ id CONSTANT) \
        Q_PROPERTY(bool isNull READ isNull CONSTANT) \
    public: \
        QML_TYPE(Type)() = default; \
        QML_TYPE(Type)(Type id) : _id(id) {} \
        operator Type() const { return _id; } \
        bool isNull() const { return _id.isNull(); } \
    private: \
        Type _id; \
        int id() const { return static_cast<int>(_id); } \
    };

// cppcheck-suppress noExplicitConstructor
QML_ELEMENTID(NodeId)
// cppcheck-suppress noExplicitConstructor
QML_ELEMENTID(EdgeId)
// cppcheck-suppress noExplicitConstructor
QML_ELEMENTID(ComponentId)

#endif // QMLELEMENTID_H
