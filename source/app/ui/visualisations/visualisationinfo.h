#ifndef VISUALISATIONINFO_H
#define VISUALISATIONINFO_H

#include "ui/alert.h"

#include <vector>
#include <limits>

#include <QString>

class VisualisationInfo
{
private:
    std::vector<Alert> _alerts;
    double _min = std::numeric_limits<double>::max();
    double _max = std::numeric_limits<double>::lowest();
    std::vector<QString> _stringValues;

public:
    template<typename... Args>
    void addAlert(Args&&... args)
    {
        _alerts.emplace_back(std::forward<Args>(args)...);
    }

    auto alerts() const { return _alerts; }

    auto min() const { return _min; }
    auto max() const { return _max; }
    void setMin(double min) { _min = min; }
    void setMax(double max) { _max = max; }

    void resetRange()
    {
        _min = std::numeric_limits<double>::max();
        _max = std::numeric_limits<double>::lowest();
    }

    bool hasRange() const { return _min <= _max; }

    void addStringValue(const QString& value) { _stringValues.emplace_back(value); }
    auto stringValues() const { return _stringValues; }
};

using VisualisationInfosMap = std::map<int, VisualisationInfo>;

#endif // VISUALISATIONINFO_H
