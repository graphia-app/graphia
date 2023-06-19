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

#include "applytransformscommand.h"

#include "graph/graphmodel.h"
#include "ui/document.h"

#include <QObject>
#include <QSet>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

ApplyTransformsCommand::ApplyTransformsCommand(GraphModel* graphModel,
                                               Document* document,
                                               QStringList previousTransformations,
                                               QStringList transformations) :
    _graphModel(graphModel),
    _document(document),
    _previousTransformations(std::move(previousTransformations)),
    _transformations(std::move(transformations))
{
    const bool transformsValid = std::all_of(_transformations.begin(), _transformations.end(), // clazy:exclude=detaching-member
    [graphModel](const auto& transform)
    {
        return graphModel->graphTransformIsValid(transform);
    });

    Q_ASSERT(transformsValid);
}

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

    for(const auto& transform : diff)
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
