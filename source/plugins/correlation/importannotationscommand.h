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

#ifndef IMPORTANNOTATIONSCOMMAND_H
#define IMPORTANNOTATIONSCOMMAND_H

#include "shared/commands/icommand.h"

#include "shared/loading/tabulardata.h"
#include "shared/loading/userdatavector.h"

#include <QString>

#include <vector>
#include <set>

class CorrelationPluginInstance;

class ImportAnnotationsCommand : public ICommand
{
private:
    CorrelationPluginInstance* _plugin = nullptr;

    TabularData _data;
    size_t _keyRowIndex;
    std::vector<int> _importRowIndices;

    bool _replace = false;
    std::vector<UserDataVector> _replacedUserDataVectors;

    std::set<QString> _createdVectorNames;
    std::vector<QString> _createdAnnotationNames;

    size_t numAnnotations() const { return _importRowIndices.size(); }
    bool multipleAnnotations() const { return numAnnotations() > 1; }
    QString firstAnnotationName() const;

public:
    ImportAnnotationsCommand(CorrelationPluginInstance* plugin,
        TabularData* data, int keyRowIndex, const std::vector<int>& importRowIndices,
        bool replace);

    QString description() const override;
    QString verb() const override;
    QString pastParticiple() const override;

    QString debugDescription() const override;

    bool execute() override;
    void undo() override;
};

#endif // IMPORTANNOTATIONSCOMMAND_H
