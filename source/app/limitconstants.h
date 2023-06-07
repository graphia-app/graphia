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

#ifndef LIMITCONSTANTS_H
#define LIMITCONSTANTS_H

#include <QObject>

class LimitConstants : public QObject
{
    Q_OBJECT

    Q_PROPERTY(float minimumNodeSize READ minimumNodeSize CONSTANT)
    Q_PROPERTY(float maximumNodeSize READ maximumNodeSize CONSTANT)

    Q_PROPERTY(float minimumEdgeSize READ minimumEdgeSize CONSTANT)
    Q_PROPERTY(float maximumEdgeSize READ maximumEdgeSize CONSTANT)

    Q_PROPERTY(float minimumTextSize READ minimumTextSize CONSTANT)
    Q_PROPERTY(float maximumTextSize READ maximumTextSize CONSTANT)
    Q_PROPERTY(float defaultTextSize READ defaultTextSize CONSTANT)

    Q_PROPERTY(float minimumMinimumComponentRadius READ minimumMinimumComponentRadius CONSTANT)
    Q_PROPERTY(float maximumMinimumComponentRadius READ maximumMinimumComponentRadius CONSTANT)

    Q_PROPERTY(float minimumTransitionTime READ minimumTransitionTime CONSTANT)
    Q_PROPERTY(float maximumTransitionTime READ maximumTransitionTime CONSTANT)

public:
    static constexpr float minimumNodeSize() { return 0.25f; }
    static constexpr float maximumNodeSize() { return 4.0f; }

    static constexpr float minimumEdgeSize() { return 0.02f; }
    static constexpr float maximumEdgeSize() { return 2.0f; }

    static constexpr float minimumTextSize() { return 0.33f; }
    static constexpr float maximumTextSize() { return 4.0f; }
    static constexpr float defaultTextSize() { return 0.5f; } // Normalised [0,1]

    static constexpr float minimumMinimumComponentRadius() { return 0.05f; }
    static constexpr float maximumMinimumComponentRadius() { return 15.0f; }

    static constexpr float minimumTransitionTime() { return 0.1f; }
    static constexpr float maximumTransitionTime() { return 5.0f; }
};

#endif // LIMITCONSTANTS_H
