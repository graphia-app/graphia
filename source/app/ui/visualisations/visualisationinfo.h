#ifndef VISUALISATIONINFO_H
#define VISUALISATIONINFO_H

#include "ui/alert.h"

#include <vector>
#include <limits>

class VisualisationInfo
{
private:
    std::vector<Alert> _alerts;
    double _min = std::numeric_limits<double>::max();
    double _max = std::numeric_limits<double>::lowest();

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
};

using VisualisationInfosMap = std::map<int, VisualisationInfo>;

#endif // VISUALISATIONINFO_H
