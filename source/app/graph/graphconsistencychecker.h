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

#ifndef GRAPHCONSISTENCYCHECKER_H
#define GRAPHCONSISTENCYCHECKER_H

#include <QObject>

class Graph;

class GraphConsistencyChecker : public QObject
{
    Q_OBJECT

public:
    explicit GraphConsistencyChecker(const Graph& graph);

    void toggle();
    void enable();
    void disable();
    bool enabled() const { return _enabled; }

private:
    const Graph* _graph;
    bool _enabled = false;

private slots:
    void onGraphChanged(const Graph* graph);
};

#endif // GRAPHCONSISTENCYCHECKER_H
