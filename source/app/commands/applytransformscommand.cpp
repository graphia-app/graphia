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

#include "applytransformscommand.h"

#include "app/graph/graphmodel.h"
#include "app/ui/document.h"

#include <QObject>
#include <QSet>
#include <QtGlobal>

#include <algorithm>
#include <utility>

using namespace Qt::Literals::StringLiterals;

[[maybe_unused]] static bool transformsValid(const GraphModel& graphModel, const QStringList& transforms)
{
    return std::ranges::all_of(transforms, [&graphModel](const auto& transform)
    {
        return graphModel.graphTransformIsValid(transform);
    });
}

ApplyTransformsCommand::ApplyTransformsCommand(GraphModel* graphModel,
                                               Document* document,
                                               QStringList previousTransformations,
                                               QStringList transformations) :
    _graphModel(graphModel),
    _document(document),
    _previousTransformations(std::move(previousTransformations)),
    _transformations(std::move(transformations))
{
    Q_ASSERT(transformsValid(*_graphModel, _transformations));
}

ApplyTransformsCommand::ApplyTransformsCommand(GraphModel* graphModel, Document* document) :
    _graphModel(graphModel),
    _document(document),
    _deferred(true)
{}

QString ApplyTransformsCommand::description() const
{
    return QObject::tr("Apply Transforms");
}

QString ApplyTransformsCommand::verb() const
{
    return QObject::tr("Applying Transforms");
}

QString ApplyTransformsCommand::debugDescription() const
{
    auto prev = QSet<QString>(_previousTransformations.begin(), _previousTransformations.end());
    auto diff = QSet<QString>(_transformations.begin(), _transformations.end());
    diff.subtract(prev);

    auto text = description();

    for(const auto& transform : std::as_const(diff))
        text.append(u"\n  %1"_s.arg(transform));

    return text;
}

void ApplyTransformsCommand::doTransform(const QStringList& transformations, const QStringList& previousTransformations)
{
    _graphModel->buildTransforms(transformations, this);

    _document->executeOnMainThreadAndWait(
    [this, newTransformations = cancelled() ? previousTransformations : transformations]
    {
        _document->setTransforms(newTransformations);
    }, u"setTransforms"_s);
}

bool ApplyTransformsCommand::execute()
{
    if(_deferred)
    {
        _document->executeOnMainThreadAndWait([this]
        {
            _previousTransformations = _document->transforms();
            _transformations = _document->graphTransformConfigurationsFromUI();
        }, u"ApplyTransformsCommand read transforms"_s);

        Q_ASSERT(transformsValid(*_graphModel, _transformations));
    }

    doTransform(_transformations, _previousTransformations);
    return true;
}

void ApplyTransformsCommand::undo()
{
    doTransform(_previousTransformations, _transformations);
}

void ApplyTransformsCommand::cancel()
{
    ICommand::cancel();

    _graphModel->cancelTransformBuild();
}
