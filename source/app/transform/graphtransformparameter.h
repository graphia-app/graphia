#ifndef GRAPHTRANSFORMPARAMETER_H
#define GRAPHTRANSFORMPARAMETER_H

#include "shared/attributes/valuetype.h"

#include <QString>
#include <QVariant>

#include <limits>
#include <map>
#include <utility>

class GraphTransformParameter
{
public:
    GraphTransformParameter() = default;
    GraphTransformParameter(QString name, ValueType type, QString description,
                            QVariant initialValue = QVariant(""),
                            double min = std::numeric_limits<double>::max(),
                            double max = std::numeric_limits<double>::lowest()) :
        _name(std::move(name)), _type(type), _description(std::move(description)),
        _initialValue(std::move(initialValue)), _min(min), _max(max)
    {}

    QString name() const { return _name; }
    ValueType type() const { return _type; }
    QString description() const { return _description; }

    bool hasMin() const { return _min != std::numeric_limits<double>::max(); }
    bool hasMax() const { return _max != std::numeric_limits<double>::lowest(); }
    bool hasRange() const { return hasMin() && hasMax(); }

    double min() const { return _min; }
    double max() const { return _max; }

    QVariant initialValue() const { return _initialValue; }

private:
    QString _name;
    ValueType _type = ValueType::Unknown;
    QString _description;
    QVariant _initialValue;
    double _min = std::numeric_limits<double>::max();
    double _max = std::numeric_limits<double>::lowest();
};

using GraphTransformParameters = std::vector<GraphTransformParameter>;

#endif // GRAPHTRANSFORMPARAMETER_H
