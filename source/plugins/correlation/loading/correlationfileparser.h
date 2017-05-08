#ifndef CORRELATIONFILEPARSER_H
#define CORRELATIONFILEPARSER_H

#include "shared/loading/baseparser.h"
#include <QString>

class CorrelationPluginInstance;

class CorrelationFileParser : public BaseParser
{
private:
    CorrelationPluginInstance* _plugin;
    QString _urlTypeName;

public:
    explicit CorrelationFileParser(CorrelationPluginInstance* correlationPluginInstance, const QString& urlTypeName);

    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // CORRELATIONFILEPARSER_H
