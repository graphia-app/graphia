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

#ifndef HIERARCHICALCLUSTERINGCOMMAND_H
#define HIERARCHICALCLUSTERINGCOMMAND_H

#include "shared/commands/icommand.h"

#include <QObject>

class CorrelationPluginInstance;

class HierarchicalClusteringCommand : public ICommand
{
private:
    const std::vector<double>* _data = nullptr;
    size_t _numColumns = 0;
    size_t _numRows = 0;

    CorrelationPluginInstance* _correlationPluginInstance = nullptr;

public:
    HierarchicalClusteringCommand(const std::vector<double>& data,
        size_t numColumns, size_t numRows,
        CorrelationPluginInstance& correlationPluginInstance);

    QString description() const override { return QObject::tr("Sorting Columns"); }

    bool execute() override;

    bool cancellable() const override { return true; }
};

#endif // HIERARCHICALCLUSTERINGCOMMAND_H
