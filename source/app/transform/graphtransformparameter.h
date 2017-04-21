#ifndef GRAPHTRANSFORMPARAMETER_H
#define GRAPHTRANSFORMPARAMETER_H

#include "attributes/valuetype.h"

#include <QString>

#include <limits>
#include <map>

class GraphTransformParameter
{
public:
    GraphTransformParameter(ValueType type, const QString& description,
                            double min = std::numeric_limits<double>::max(),
                            double max = std::numeric_limits<double>::min()) :
        _type(type), _description(description), _min(min), _max(max)
    {}

    ValueType type() const { return _type; }
    QString description() const { return _description; }

    bool hasMin() const { return _min != std::numeric_limits<double>::max(); }
    bool hasMax() const { return _max != std::numeric_limits<double>::min(); }
    bool hasRange() const { return hasMin() && hasMax(); }

    double min() const { return _min; }
    double max() const { return _max; }

private:
    ValueType _type;
    QString _description;
    double _min;
    double _max;
};

using GraphTransformParameters = std::map<QString, GraphTransformParameter>;

#endif // GRAPHTRANSFORMPARAMETER_H
