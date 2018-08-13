#include "matrixfileparser.h"

MatrixFileParser::MatrixFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData)
    : _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{

}

bool MatrixFileParser::parse(const QUrl &url, IGraphModel &graphModel, const ProgressFn &progressFn)
{
    TsvFileParser tsvFileParser(this);
    TabularData* tabularData = nullptr;

    if(!tsvFileParser.parse(url, graphModel, progressFn))
        return false;

    tabularData = &(tsvFileParser.tabularData());
    std::map<size_t, NodeId> tablePositionToNodeId;

    bool hasColumnHeaders = false;
    bool hasRowHeaders = false;
    size_t dataStartRow = 0;
    size_t dataStartColumn = 0;

    // Prepass for headers
    if(tabularData->numRows() > 0 && tabularData->numColumns() > 0)
    {
        // Set temporarily to true
        hasRowHeaders = true;
        hasColumnHeaders = true;
        for(size_t rowIndex = 0; rowIndex < tabularData->numRows(); rowIndex++)
        {
            auto stringValue = tabularData->valueAsQString(0, rowIndex);
            bool success = false;
            stringValue.toDouble(&success);
            if(success)
            {
                // Probably doesnt if I can convert the header to double
                hasRowHeaders = false;
                break;
            }
        }
        // Column Headers
        for(size_t columnIndex = 0; columnIndex < tabularData->numColumns(); columnIndex++)
        {
            auto stringValue = tabularData->valueAsQString(columnIndex, 0);
            bool success = false;
            stringValue.toDouble(&success);
            if(success)
            {
                // Probably doesnt have headers if I can convert the header to double
                hasColumnHeaders = false;
                break;
            }
        }

        if(hasColumnHeaders)
        {
            dataStartRow = 1;
            for(size_t columnIndex = hasRowHeaders ? 1 : 0; columnIndex < tabularData->numColumns(); columnIndex++)
            {
                // Add Node names
                auto nodeId = graphModel.mutableGraph().addNode();
                _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                          tabularData->valueAsQString(columnIndex, 0));
                tablePositionToNodeId[columnIndex] = nodeId;
            }
        }
        if(hasRowHeaders)
        {
            dataStartColumn = 1;
            for(size_t rowIndex = hasColumnHeaders ? 1 : 0; rowIndex < tabularData->numRows(); rowIndex++)
            {
                if(hasColumnHeaders)
                {
                    // Check row and column match (they should!)
                    auto expectedRowName = _userNodeData->valueBy(tablePositionToNodeId[rowIndex], QObject::tr("Node Name"));
                    auto actualRowName = tabularData->valueAsQString(0, rowIndex);
                    if(expectedRowName.toString() != actualRowName)
                        return false;
                }
                else
                {
                    // Add Node names
                    auto nodeId = graphModel.mutableGraph().addNode();
                    _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                              tabularData->valueAsQString(0, rowIndex));
                    tablePositionToNodeId[rowIndex] = nodeId;
                }
            }
        }

        // No headers start from top left
        if(!hasColumnHeaders && !hasRowHeaders)
        {
            // Populate with "Node 1, Node 2..."
            for(size_t rowIndex = 0; rowIndex < tabularData->numRows(); rowIndex++)
            {
                // Node names
                auto nodeId = graphModel.mutableGraph().addNode();
                _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                          QObject::tr("Node %1").arg(rowIndex + 1));
                tablePositionToNodeId[rowIndex] = nodeId;
            }
        }

        for(size_t rowIndex = dataStartRow; rowIndex < tabularData->numRows(); rowIndex++)
        {
            for(size_t columnIndex = dataStartColumn; columnIndex < tabularData->numColumns(); columnIndex++)
            {
                // Edges
                auto qStringValue = tabularData->valueAsQString(columnIndex, rowIndex);
                bool success = false;
                double doubleValue = qStringValue.toDouble(&success);

                if(success && doubleValue != 0.0)
                {
                    auto edgeId = graphModel.mutableGraph().addEdge(tablePositionToNodeId[rowIndex],
                                                                    tablePositionToNodeId[columnIndex]);
                    _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"), QString::number(doubleValue));
                }
            }
        }
    }
    return true;
}
