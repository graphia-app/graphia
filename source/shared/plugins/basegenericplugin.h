#ifndef BASEGENERICPLUGIN_H
#define BASEGENERICPLUGIN_H

#include "baseplugin.h"

#include "shared/plugins/userelementdata.h"
#include "shared/plugins/nodeattributetablemodel.h"

#include "shared/graph/grapharray.h"

#include <memory>

// This implements some basic functionality that will be common to plugins
// that load from generic graph file formats

class BaseGenericPluginInstance : public BasePluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QString selectedNodeNames READ selectedNodeNames NOTIFY selectedNodeNamesChanged)
    Q_PROPERTY(QAbstractTableModel* nodeAttributeTableModel READ nodeAttributeTableModel CONSTANT)

    Q_PROPERTY(QVector<int> highlightedRows MEMBER _highlightedRows
        WRITE setHighlightedRows NOTIFY highlightedRowsChanged)

private:
    UserNodeData _userNodeData;
    UserEdgeData _userEdgeData;

    NodeAttributeTableModel _nodeAttributeTableModel;
    QAbstractTableModel* nodeAttributeTableModel() { return &_nodeAttributeTableModel; }

public:
    BaseGenericPluginInstance();

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName) override;

    QByteArray save(IMutableGraph&, Progressable&) const override;
    bool load(const QByteArray&, int, IMutableGraph&, IParser& parser) override;

private:
    // The rows that are selected in the table view
    QVector<int> _highlightedRows;

    void initialise(const IPlugin* plugin, IDocument* document,
                    const IParserThread* parserThread) override;

    QString selectedNodeNames() const;
    void setHighlightedRows(const QVector<int>& highlightedRows);

private slots:
    void onLoadSuccess();

    void onSelectionChanged(const ISelectionManager* selectionManager);

signals:
    void selectedNodeNamesChanged();
    void highlightedRowsChanged();
};

class BaseGenericPlugin : public BasePlugin
{
    Q_OBJECT
public:
    BaseGenericPlugin();

    QStringList identifyUrl(const QUrl& url) const override;
    QString failureReason(const QUrl&) const override;

    bool editable() const override { return true; }
};

#endif // BASEGENERICPLUGIN_H
