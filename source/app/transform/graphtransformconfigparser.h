#ifndef GRAPHTRANSFORMCONFIGPARSER_H
#define GRAPHTRANSFORMCONFIGPARSER_H

#include "graphtransformconfig.h"
#include "shared/attributes/valuetype.h"

#include <QString>
#include <QStringList>

class GraphTransformConfigParser
{
private:
    GraphTransformConfig _result;
    bool _success = false;
    QString _failedInput;

public:
    bool parse(const QString& text, bool warnOnFailure = true);

    const GraphTransformConfig& result() const { return _result; }
    bool success() const { return _success; }
    const QString& failedInput() const { return _failedInput; }

    static QStringList ops(ValueType valueType);
    static QString opToString(ConditionFnOp::Equality op);
    static QString opToString(ConditionFnOp::Numerical op);
    static QString opToString(ConditionFnOp::String op);
    static QString opToString(ConditionFnOp::Logical op);
    static QString opToString(ConditionFnOp::Unary op);

    static GraphTransformConfig::TerminalOp stringToOp(const QString& s);

    static bool opIsUnary(const QString& op);

    static bool isAttributeName(const QString& variable);
    static QString attributeNameFor(const QString& variable);
};

#endif // GRAPHTRANSFORMCONFIGPARSER_H
