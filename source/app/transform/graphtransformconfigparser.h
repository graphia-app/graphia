#ifndef GRAPHTRANSFORMCONFIGPARSER_H
#define GRAPHTRANSFORMCONFIGPARSER_H

#include "graphtransformconfig.h"
#include "attributes/valuetype.h"

#include <QString>
#include <QStringList>

class GraphTransformConfigParser
{
private:
    GraphTransformConfig _result;
    bool _success = false;
    QString _failedInput;

public:
    bool parse(const QString& text);

    const GraphTransformConfig& result() const { return _result; }
    bool success() const { return _success; }
    const QString& failedInput() const { return _failedInput; }

    static QStringList ops(ValueType valueType);
    static QString opToString(ConditionFnOp::Numerical op);
    static QString opToString(ConditionFnOp::String op);
    static QString opToString(ConditionFnOp::Binary op);
    static QString opToString(ConditionFnOp::Unary op);

    static bool opIsUnary(const QString& op);
};

#endif // GRAPHTRANSFORMCONFIGPARSER_H
