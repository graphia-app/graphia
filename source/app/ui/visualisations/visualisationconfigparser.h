#ifndef VISUALISATIONCONFIGPARSER_H
#define VISUALISATIONCONFIGPARSER_H

#include "visualisationconfig.h"

#include <QString>

class VisualisationConfigParser
{
private:
    VisualisationConfig _result;
    bool _success = false;
    QString _failedInput;

public:
    bool parse(const QString& text, bool warnOnFailure = true);

    const VisualisationConfig& result() const { return _result; }
    bool success() const { return _success; }
    const QString& failedInput() const { return _failedInput; }
};

#endif // VISUALISATIONCONFIGPARSER_H
