#ifndef GRAPHTRANSFORMCONFIGPARSER_H
#define GRAPHTRANSFORMCONFIGPARSER_H

#include "graphtransformconfig.h"
#include "datafield.h"

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

    static QStringList ops(FieldType fieldType);
    static QString opToString(GraphTransformConfig::NumericalOp op);
    static QString opToString(GraphTransformConfig::StringOp op);
    static QString opToString(GraphTransformConfig::BinaryOp op);
};

#endif // GRAPHTRANSFORMCONFIGPARSER_H
