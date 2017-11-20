#ifndef CORRELATIONFILEPARSER_H
#define CORRELATIONFILEPARSER_H

#include "shared/loading/iparser.h"
#include <QString>

class CorrelationPluginInstance;

class CorrelationFileParser : public IParser
{
private:
    CorrelationPluginInstance* _plugin;
    QString _urlTypeName;

public:
    explicit CorrelationFileParser(CorrelationPluginInstance* correlationPluginInstance, QString urlTypeName);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progressFn) override;
};

#endif // CORRELATIONFILEPARSER_H
