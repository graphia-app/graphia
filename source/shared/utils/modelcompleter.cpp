/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "modelcompleter.h"

#include <limits>

ModelCompleter::ModelCompleter()
{
    connect(this, &ModelCompleter::modelChanged, this, &ModelCompleter::update);
    connect(this, &ModelCompleter::termChanged, this, &ModelCompleter::update);
}

void ModelCompleter::update()
{
    if(_model == nullptr)
        return;

    QVector<QModelIndex> candidates;
    QString commonPrefix;
    QModelIndex closestMatch;
    int shortestLength = std::numeric_limits<int>::max();

    auto forEach = [&](const auto& forEach_, QModelIndex parent = {}) -> void
    {
        for(int row = 0; row < _model->rowCount(parent); row++)
        {
            const QModelIndex index = _model->index(row, 0, parent);
            auto name = _model->data(index).toString();

            if(name.startsWith(_term, Qt::CaseInsensitive))
            {
                if(!commonPrefix.isEmpty())
                {
                    for(int i = 0 ; i < std::min(commonPrefix.length(), name.length()); i++)
                    {
                        if(commonPrefix.at(i).toLower() != name.at(i).toLower())
                        {
                            commonPrefix = commonPrefix.left(i);
                            break;
                        }
                    }
                }
                else
                    commonPrefix = name;

                candidates.append(index);

                if(name.length() < shortestLength)
                {
                    shortestLength = static_cast<int>(name.length());
                    closestMatch = index;
                }
            }

            if(_model->hasChildren(index))
                forEach_(forEach_, index);
        }
    };

    if(!_term.isEmpty())
        forEach(forEach);

    _candidates = candidates;
    emit candidatesChanged();

    if(commonPrefix != _commonPrefix)
    {
        _commonPrefix = commonPrefix;
        emit commonPrefixChanged();
    }

    if(closestMatch != _closestMatch)
    {
        _closestMatch = closestMatch;
        emit closestMatchChanged();
    }
}
