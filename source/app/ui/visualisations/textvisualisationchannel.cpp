#include "textvisualisationchannel.h"
#include "visualisationinfo.h"

#include "rendering/graphrenderer.h"

#include "shared/utils/preferences.h"

#include <QObject>

void TextVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._text = QString::number(value, 'g', 3);
}

void TextVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    elementVisual._text = value;
}

void TextVisualisationChannel::findErrors(VisualisationInfo& info) const
{
    if(u::pref("visuals/showEdgeText").toInt() == static_cast<int>(TextState::Off))
        info.addAlert(AlertType::Warning, QObject::tr("Edge Text Disabled"));
}

QString TextVisualisationChannel::description(ElementType, ValueType) const
{
    return QObject::tr("The attribute will be visualised as text.");
}
