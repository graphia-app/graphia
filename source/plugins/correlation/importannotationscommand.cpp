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

#include "importannotationscommand.h"

#include "correlationplugin.h"

#include "shared/utils/string.h"
#include "shared/loading/userelementdata.h"

#include "../crashhandler.h"

using namespace Qt::Literals::StringLiterals;

ImportAnnotationsCommand::ImportAnnotationsCommand(CorrelationPluginInstance* plugin,
    TabularData* data, int keyRowIndex, const std::vector<int>& importRowIndices, bool replace) :
    _plugin(plugin), _data(std::move(*data)),
    _keyRowIndex(static_cast<size_t>(keyRowIndex)), _importRowIndices(importRowIndices),
    _replace(replace)
{}

QString ImportAnnotationsCommand::firstAnnotationName() const
{
    if(!_createdAnnotationNames.empty())
        return _createdAnnotationNames.front();

    if(!_replacedUserDataVectors.empty())
        return _replacedUserDataVectors.begin()->name();

    return {};
}

QString ImportAnnotationsCommand::description() const
{
    return multipleAnnotations() ? QObject::tr("Import Column Annotations") : QObject::tr("Import Column Annotation");
}

QString ImportAnnotationsCommand::verb() const
{
    return multipleAnnotations() ? QObject::tr("Importing Column Annotations") : QObject::tr("Importing Column Annotation");
}

QString ImportAnnotationsCommand::pastParticiple() const
{
    return multipleAnnotations() ?
        QObject::tr("%1 Annotations Imported").arg(numAnnotations()) :
        QObject::tr("Annotation %1 Imported").arg(firstAnnotationName());
}

QString ImportAnnotationsCommand::debugDescription() const
{
    QString text = description();

    return text;
}

bool ImportAnnotationsCommand::execute()
{
    std::map<QString, size_t> map;

    for(size_t column = 0; column < _plugin->numColumns(); column++)
    {
        const auto& columnName = _plugin->columnName(column);

        for(size_t dataColumn = 1; dataColumn < _data.numColumns(); dataColumn++)
        {
            auto keyRowValue = _data.valueAt(dataColumn, _keyRowIndex);

            if(columnName == keyRowValue)
            {
                map[columnName] = dataColumn;
                break;
            }
        }

        setProgress((static_cast<int>(column) * 100) /
            static_cast<int>(_plugin->numColumns()));
    }

    setProgress(-1);

    if(map.empty())
        return false;

    auto& userData = _plugin->userColumnData();

    for(auto rowIndex : _importRowIndices)
    {
        auto annotationName = _data.valueAt(0, static_cast<size_t>(rowIndex));
        auto* existingVector = userData.vector(annotationName);

        const bool replace = _replace && existingVector != nullptr &&
            existingVector->type() == _data.typeIdentity(static_cast<size_t>(rowIndex)).type();

        if(replace)
            _replacedUserDataVectors.emplace_back(*existingVector);
        else
        {
            annotationName = u::findUniqueName(userData.vectorNames(), annotationName);
            _createdVectorNames.emplace(annotationName);
        }

        for(size_t column = 0; column < _plugin->numColumns(); column++)
        {
            const auto& columnName = _plugin->columnName(column);

            auto value = map.contains(columnName) ?
                _data.valueAt(map.at(columnName), static_cast<size_t>(rowIndex)) : QString{};
            userData.setValue(column, annotationName, value);
        }
    }

    _plugin->rebuildColumnAnnotations();

    return true;
}

void ImportAnnotationsCommand::undo()
{
    auto& userData = _plugin->userColumnData();

    for(const auto& vectorName : _createdVectorNames)
        userData.remove(vectorName);

    for(auto&& vector : _replacedUserDataVectors)
        userData.setVector(std::move(vector));

    _createdVectorNames.clear();
    _replacedUserDataVectors.clear();

    _plugin->rebuildColumnAnnotations();
}
