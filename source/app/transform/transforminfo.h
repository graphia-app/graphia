#ifndef TRANSFORMINFO_H
#define TRANSFORMINFO_H

#include "ui/alert.h"

#include <vector>

class TransformInfo
{
private:
    std::vector<Alert> _alerts;

public:
    template<typename... Args>
    void addAlert(Args&&... args)
    {
        _alerts.emplace_back(std::forward<Args>(args)...);
    }

    auto alerts() const { return _alerts; }
};

using TransformInfosMap = std::map<int, TransformInfo>;

#endif // TRANSFORMINFO_H
