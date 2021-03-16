/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#ifndef CORRELATIONPLUGIN_H
#define CORRELATIONPLUGIN_H

#include "shared/plugins/baseplugin.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/grapharray.h"
#include "shared/graph/edgelist.h"
#include "shared/loading/tabulardata.h"
#include "shared/loading/iparser.h"
#include "shared/loading/userdata.h"
#include "shared/loading/userelementdata.h"

#include "loading/correlationfileparser.h"

#include "columnannotation.h"
#include "correlationdatarow.h"
#include "correlationnodeattributetablemodel.h"

#include <vector>
#include <functional>
#include <algorithm>
#include <utility>

#include <QString>
#include <QStringList>
#include <QVector>
#include <QVariantList>
#include <QVariantMap>
#include <QColor>
#include <QRect>

class CorrelationPluginInstance : public BasePluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QAbstractTableModel* nodeAttributeTableModel READ nodeAttributeTableModel CONSTANT)

    Q_PROPERTY(QStringList columnAnnotationNames READ columnAnnotationNames NOTIFY columnAnnotationNamesChanged)

    Q_PROPERTY(QStringList sharedValuesAttributeNames READ sharedValuesAttributeNames
        NOTIFY sharedValuesAttributeNamesChanged)

    Q_PROPERTY(QStringList numericalAttributeNames READ numericalAttributeNames
        NOTIFY numericalAttributeNamesChanged)

    Q_PROPERTY(QVector<int> highlightedRows MEMBER _highlightedRows
        WRITE setHighlightedRows NOTIFY highlightedRowsChanged)

public:
    CorrelationPluginInstance();

private:
    IGraphModel* _graphModel = nullptr;

    size_t _numColumns = 0;
    size_t _numRows = 0;

    std::vector<QString> _dataColumnNames;
    std::vector<ColumnAnnotation> _columnAnnotations;

    UserNodeData _userNodeData;
    UserData _userColumnData;

    CorrelationNodeAttributeTableModel _nodeAttributeTableModel;

    std::vector<double> _data;

    CorrelationDataRows _dataRows;

    std::unique_ptr<EdgeArray<double>> _correlationValues;
    double _minimumCorrelationValue = 0.7;
    double _initialCorrelationThreshold = 0.85;
    bool _transpose = false;
    TabularData _tabularData;
    QRect _dataRect;
    CorrelationType _correlationType = CorrelationType::Pearson;
    CorrelationPolarity _correlationPolarity = CorrelationPolarity::Positive;
    ScalingType _scalingType = ScalingType::None;
    NormaliseType _normaliseType = NormaliseType::None;
    MissingDataType _missingDataType = MissingDataType::Constant;
    ClusteringType _clusteringType = ClusteringType::None;
    EdgeReductionType _edgeReductionType = EdgeReductionType::None;
    double _missingDataReplacementValue = 0.0;
    bool _imputedValues = false;

    QString _correlationAttributeName;
    QString _correlationAbsAttributeName;

    // The rows that are selected in the table view
    QVector<int> _highlightedRows;

    void initialise(const IPlugin* plugin, IDocument* document,
                    const IParserThread* parserThread) override;

    void setDataColumnName(size_t column, const QString& name);
    void makeDataColumnNamesUnique();

    void setData(size_t column, size_t row, double value);

    void finishDataRow(size_t row);

    QAbstractTableModel* nodeAttributeTableModel() { return &_nodeAttributeTableModel; }
    void setNodeAttributeTableModelDataColumns();
    QStringList columnAnnotationNames() const;
    QVector<double> rawData();

    void buildColumnAnnotations();

    const CorrelationDataRow& dataRowForNodeId(NodeId nodeId) const;

    void setHighlightedRows(const QVector<int>& highlightedRows);

    QStringList sharedValuesAttributeNames() const;
    QStringList numericalAttributeNames() const;

public:
    void setDimensions(size_t numColumns, size_t numRows);
    bool loadUserData(const TabularData& tabularData, const QRect& dataRect, IParser& parser);
    bool requiresNormalisation() const { return _normaliseType != NormaliseType::None; }
    void normalise(IParser* parser);
    void finishDataRows();
    void createAttributes();

    EdgeList correlation(double minimumThreshold, IParser& parser);

    double minimumCorrelation() const { return _minimumCorrelationValue; }
    bool transpose() const { return _transpose; }

    bool createEdges(const EdgeList& edges, IParser& parser);

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName) override;
    void applyParameter(const QString& name, const QVariant& value) override;
    QStringList defaultTransforms() const override;
    QStringList defaultVisualisations() const override;

    size_t numColumns() const;
    double dataAt(int row, int column) const;
    QString rowName(int row) const;
    QString columnName(int column) const;
    QColor nodeColorForRow(int row) const;

    const std::vector<ColumnAnnotation>& columnAnnotations() const { return _columnAnnotations; }

    QString attributeValueFor(const QString& attributeName, int row) const;

    QByteArray save(IMutableGraph& graph, Progressable& progressable) const override;
    bool load(const QByteArray& data, int dataVersion, IMutableGraph& graph, IParser& parser) override;

    QString log() const;

private slots:
    void onLoadSuccess();
    void onSelectionChanged(const ISelectionManager* selectionManager);

signals:
    void columnAnnotationNamesChanged();
    void sharedValuesAttributeNamesChanged();
    void numericalAttributeNamesChanged();
    void nodeColorsChanged();
    void highlightedRowsChanged();
};

class CorrelationPlugin : public BasePlugin, public PluginInstanceProvider<CorrelationPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "CorrelationPlugin.json")

public:
    CorrelationPlugin();

    QString name() const override { return QStringLiteral("Correlation"); }
    QString description() const override
    {
        return tr("Creates a graph where nodes represent rows of data, "
                  "and edges represent correlations between said rows.");
    }

    QString imageSource() const override { return QStringLiteral("qrc:///plots.svg"); }

    int dataVersion() const override { return 6; }

    QStringList identifyUrl(const QUrl& url) const override;
    QString failureReason(const QUrl& url) const override;

    bool editable() const override { return true; }
    bool directed() const override { return false; }

    QString parametersQmlPath(const QString&) const override { return QStringLiteral("qrc:///qml/CorrelationParameters.qml"); }
    QString qmlPath() const override { return QStringLiteral("qrc:///qml/CorrelationPlugin.qml"); }
};

#endif // CORRELATIONPLUGIN_H
