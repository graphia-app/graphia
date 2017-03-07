#ifndef VISUALISATIONALERT_H
#define VISUALISATIONALERT_H

#include "utils/qmlenum.h"

#include <QString>

#include <vector>
#include <map>

DEFINE_QML_ENUM(Q_GADGET, VisualisationAlertType,
                None,
                Warning,
                Error);

struct VisualisationAlert
{
    VisualisationAlertType _type;

    QString _text;

    VisualisationAlert(VisualisationAlertType type, const QString& text) :
        _type(type), _text(text)
    {}
};

using VisualisationAlertsMap = std::map<int, std::vector<VisualisationAlert>>;

#endif // VISUALISATIONALERT_H
