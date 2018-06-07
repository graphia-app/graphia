#include "applyvisualisationscommand.h"

#include <QObject>

#include "graph/graphmodel.h"
#include "ui/document.h"

#include "ui/visualisations/visualisationconfigparser.h"

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

void ApplyVisualisationsCommand::apply(const QStringList& visualisations,
                                       const QStringList& previousVisualisations)
{
    _graphModel->buildVisualisations(visualisations);

    _document->executeOnMainThreadAndWait(
    [this, newVisualisations = cancelled() ? previousVisualisations : visualisations]
    {
        _document->setVisualisations(newVisualisations);
    }, QStringLiteral("setVisualisations"));
}

QStringList ApplyVisualisationsCommand::patchedVisualisations() const
{
    if(_transformIndex >= 0 && _visualisations.size() > _previousVisualisations.size())
    {
        // When a transform creates a new attribute, its name may not match the default
        // visualisation that it created for it, so we need to do a bit of patching

        auto createdAttributeNames = _graphModel->createdAttributeNamesAtTransformIndex(_transformIndex);

        if(!createdAttributeNames.empty())
        {
            QStringList patched;

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

                for(const auto& createdAttributeName : createdAttributeNames)
                {
                    if(createdAttributeName.startsWith(visualisationConfig._attributeName))
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
