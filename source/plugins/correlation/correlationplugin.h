#ifndef CORRELATIONPLUGIN_H
#define CORRELATIONPLUGIN_H

#include "shared/plugins/baseplugin.h"
#include "shared/graph/grapharray.h"

#include <vector>
#include <map>

class CorrelationPluginInstance : public BasePluginInstance
{
    Q_OBJECT

    friend class CorrelationFileParser;

private:
    std::vector<QString> _dataColumnNames;
    std::map<QString, std::vector<QString>> _rowAttributes;
    std::map<QString, std::vector<QString>> _columnAttributes;

    std::vector<double> _data;

    using DataIterator = std::vector<double>::const_iterator;
    using DataRow = std::pair<DataIterator, DataIterator>;

    std::unique_ptr<NodeArray<DataRow>> _dataRows;
    std::unique_ptr<EdgeArray<float>> _edgeWeights;

public:
    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName);
};

class CorrelationPlugin : public BasePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "correlationplugin.json")

public:
    CorrelationPlugin();

    QString name() const { return "Correlation"; }
    QString description() const
    {
        return tr("Correlations.");
    }

    QStringList identifyUrl(const QUrl& url) const;

    bool editable() const { return false; }

    std::unique_ptr<IPluginInstance> createInstance();

    QString qmlPath() const { return "qrc:///qml/correlationplugin.qml"; }
};

#endif // CORRELATIONPLUGIN_H
