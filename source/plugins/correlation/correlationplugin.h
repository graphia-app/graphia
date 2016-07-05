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
    int _numColumns = 0;
    int _numRows = 0;

    std::vector<QString> _dataColumnNames;
    std::map<QString, std::vector<QString>> _rowAttributes;
    std::map<QString, std::vector<QString>> _columnAttributes;

    std::vector<double> _data;

    using DataOffset = std::vector<double>::size_type;

    std::unique_ptr<NodeArray<DataOffset>> _dataRows;
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
