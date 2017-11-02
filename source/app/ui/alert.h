#ifndef ALERT_H
#define ALERT_H

#include "shared/utils/qmlenum.h"

#include <QString>

#include <vector>
#include <map>
#include <utility>

DEFINE_QML_ENUM(
      Q_GADGET, AlertType,
                None,
                Warning,
                Error);

struct Alert
{
    AlertType _type = AlertType::None;
    QString _text;

    Alert() = default;
    Alert(AlertType type, QString text) :
        _type(type), _text(std::move(text))
    {}
};

using AlertsMap = std::map<int, std::vector<Alert>>;

#endif // ALERT_H
