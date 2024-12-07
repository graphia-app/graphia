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

#include "applyvisualisationscommand.h"

#include "app/graph/graphmodel.h"
#include "app/ui/document.h"

#include "app/ui/visualisations/visualisationconfigparser.h"

#include <QObject>
#include <QSet>
#include <QRegularExpression>

#include <utility>

using namespace Qt::Literals::StringLiterals;

ApplyVisualisationsCommand::ApplyVisualisationsCommand(GraphModel* graphModel,
                                                       Document* document,
                                                       QStringList previousVisualisations,
                                                       QStringList visualisations, int transformIndex) :
    _graphModel(graphModel),
    _document(document),
    _previousVisualisations(std::move(previousVisualisations)),
    _visualisations(std::move(visualisations)),
    _transformIndex(transformIndex)
{}

QString ApplyVisualisationsCommand::description() const
{
    return QObject::tr("Apply Visualisations");
}

QString ApplyVisualisationsCommand::verb() const
{
    return QObject::tr("Applying Visualisations");
}

QString ApplyVisualisationsCommand::debugDescription() const
{
    auto prev = QSet<QString>(_previousVisualisations.begin(), _previousVisualisations.end());
    auto diff = QSet<QString>(_visualisations.begin(), _visualisations.end());
    diff.subtract(prev);

    auto text = description();

    for(const auto& visualisation : std::as_const(diff))
        text.append(u"\n  %1"_s.arg(visualisation));

    return text;
}

void ApplyVisualisationsCommand::apply(const QStringList& visualisations,
                                       const QStringList& previousVisualisations)
{
    _graphModel->buildVisualisations(visualisations);

    _document->executeOnMainThreadAndWait(
    [this, newVisualisations = cancelled() ? previousVisualisations : visualisations]
    {
        _document->setVisualisations(newVisualisations);
    }, u"setVisualisations"_s);
}

QStringList ApplyVisualisationsCommand::patchedVisualisations() const
{
    //FIXME This whole approach is pretty fragile and needs to be made more robust

    if(_transformIndex >= 0 && _visualisations.size() > _previousVisualisations.size())
    {
        // When a transform creates a new attribute, its name may not match the default
        // visualisation that it created for it, so we need to do a bit of patching

        auto createdAttributeNames = _graphModel->addedOrChangedAttributeNamesAtTransformIndex(_transformIndex);

        if(!createdAttributeNames.empty())
        {
            QStringList patched;
            patched.reserve(_visualisations.size());

            for(int i = 0; i < _visualisations.size(); i++)
            {
                const auto& visualisation = _visualisations.at(i);

                if(i < _previousVisualisations.size())
                {
                    // Just pass through the visualisations that aren't new
                    patched.append(visualisation);
                    continue;
                }

                VisualisationConfigParser p;

                if(!p.parse(visualisation))
                    continue;

                auto visualisationConfig = p.result();

                // c.f. u::findUniqueName
                const QRegularExpression re(uR"(^%1\(\d+\))"_s
                    .arg(visualisationConfig._attributeName));

                for(const auto& createdAttributeName : createdAttributeNames)
                {
                    if(re.match(createdAttributeName).hasMatch()) // clazy:exclude=use-static-qregularexpression
                        visualisationConfig._attributeName = createdAttributeName;
                }

                patched.append(visualisationConfig.asString());
            }

            return patched;
        }
    }

    return _visualisations;
}

bool ApplyVisualisationsCommand::execute()
{
    apply(patchedVisualisations(), _previousVisualisations);
    return true;
}

void ApplyVisualisationsCommand::undo()
{
    apply(_previousVisualisations, patchedVisualisations());
}

void ApplyVisualisationsCommand::replaces(const ICommand* replacee)
{
    const auto* avcReplacee =
        dynamic_cast<const ApplyVisualisationsCommand*>(replacee);
    Q_ASSERT(avcReplacee != nullptr);

    _previousVisualisations = avcReplacee->_previousVisualisations;
}
