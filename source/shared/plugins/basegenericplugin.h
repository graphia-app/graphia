/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BASEGENERICPLUGIN_H
#define BASEGENERICPLUGIN_H

#include "baseplugin.h"

#include "shared/loading/userelementdata.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/pairwisecolumntype.h"

#include "shared/plugins/nodeattributetablemodel.h"

#include "shared/graph/grapharray.h"

#include <QList>

#include <memory>

// This implements some basic functionality that will be common to plugins
// that load from generic graph file formats

class BaseGenericPluginInstance : public BasePluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QString selectedNodeNames READ selectedNodeNames NOTIFY selectedNodeNamesChanged)
    Q_PROPERTY(QAbstractTableModel* nodeAttributeTableModel READ nodeAttributeTableModel CONSTANT)

    Q_PROPERTY(QList<int> highlightedRows MEMBER _highlightedRows
        WRITE setHighlightedRows NOTIFY highlightedRowsChanged)

private:
    IGraphModel* _graphModel = nullptr;

    struct AdjacencyMatrixParameters
    {
        double _minimumAbsEdgeWeight = 0.0;
        double _initialAbsEdgeWeightThreshold = 0.0;
        bool _filterEdges = false;
        bool _skipDuplicates = false;
    } _adjacencyMatrixParameters;

    struct PairwiseParameters
    {
        bool _firstRowIsHeader = false;
        PairwiseColumnsConfiguration _columns;
    } _pairwiseParameters;

    TabularData _preloadedTabularData;

    NodeAttributeTableModel _nodeAttributeTableModel;
    QAbstractTableModel* nodeAttributeTableModel() { return &_nodeAttributeTableModel; }

public:
    BaseGenericPluginInstance();

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName) override;
    void applyParameter(const QString& name, const QVariant& value) override;
    QStringList defaultTransforms() const override;

private:
    // The rows that are selected in the table view
    QList<int> _highlightedRows;

    void initialise(const IPlugin* plugin, IDocument* document,
                    const IParserThread* parserThread) override;

    QString selectedNodeNames() const;
    void setHighlightedRows(const QList<int>& highlightedRows);

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

    QString parametersQmlType(const QString& urlType) const override;
};

#endif // BASEGENERICPLUGIN_H
