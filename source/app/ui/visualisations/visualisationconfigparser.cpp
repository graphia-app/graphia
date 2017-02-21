#include "visualisationconfigparser.h"

bool VisualisationConfigParser::parse(const QString& text)
{
    auto tokens = text.split(QRegExp(" +(?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)"));
    for(auto& token : tokens)
        token = token.remove(QChar('"'));

    if(tokens.size() != 2)
        return false;

    _result._dataFieldName = tokens.at(0);
    _result._channelName = tokens.at(1);

    return true;
}
