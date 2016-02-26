#include "gmlfileparser.h"
#include "../graph/mutablegraph.h"

#ifdef _MSC_VER
#pragma warning(disable:4503) // AXE makes a lot of these
#endif

#include "../thirdparty/axe/include/axe.h"

#include <QTime>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <fstream>

template<class It> bool GmlFileParser::parse(MutableGraph &graph, It begin, It end)
{
    int size = std::distance(begin, end);

    // Progress Capture event (Fired on gml_value rule match)
    auto captureCount = axe::e_ref([&size, &begin, this](It, It i2)
    {
        int lengthLeft = (std::distance(begin, i2)) * 100 / size;
        int newPercentage = lengthLeft;
        if (_percentage < newPercentage)
        {
            _percentage = newPercentage;
            emit progress(_percentage);
        }
    });

    // General GML structure rules
    axe::r_rule<It> keyValue;
    double d;
    auto whitespace = *axe::r_any(" \t\n\r");

    // If this is declared and initialised on the same line, the move constructor for
    // axe::r_rule is called which (for some reason) infinitely recurses, whereas using
    // operator= doesn't
    axe::r_rule<It> quotedString;
    quotedString = '"' & *("\\\"" | (axe::r_any() - '"')) & '"';

    auto key = (+axe::r_alnum());
    auto value = axe::r_double(d) | quotedString;

    auto keyValueList = whitespace & key & whitespace & '[' & whitespace &
            axe::r_many(keyValue - axe::r_char(']'), axe::r_any(" \t\n\r"), 0) & whitespace & ']';
    auto keyValuePair = (whitespace & key & whitespace & value);

    // Node State
    int id;
    bool isIdSet = false;
    std::map<int, NodeId> nodeIndexMap;

    // Node Capture Events
    auto captureNodeId = axe::e_ref([&id, &isIdSet](It i1, It i2)
    {
        id = std::stoi(std::string(i1, i2));
        isIdSet = true;
    });
    auto captureNode = axe::e_ref([&nodeIndexMap, &id, &graph, &isIdSet](It, It)
    {
        if(isIdSet)
        {
            nodeIndexMap[id] = graph.addNode();
            isIdSet = false;
        }
    });

    // Node Rules
    axe::r_rule<It> nodeKeyValuePair;
    auto nodeKeyValueList = (whitespace & "node" & whitespace & '[' & whitespace &
                           axe::r_many(nodeKeyValuePair - axe::r_char(']'), axe::r_any(" \t\n\r"), 0) & whitespace
                           & ']') >> captureNode;
    auto nodeId = whitespace & "id" & whitespace & value >> captureNodeId;
    nodeKeyValuePair = nodeId | keyValuePair | keyValueList;

    // Edge State
    int source, target;
    bool isSourceSet = false, isTargetSet = false;

    // Edge Capture Events
    auto captureEdgeSource = axe::e_ref([&source, &isSourceSet](It i1, It i2)
    {
        source = std::stoi(std::string(i1,i2));
        isSourceSet = true;
    });
    auto captureEdgeTarget = axe::e_ref([&target, &isTargetSet](It i1, It i2)
    {
        target = std::stoi(std::string(i1,i2));
        isTargetSet = true;
    });
    auto captureEdge = axe::e_ref([&source, &target, &nodeIndexMap, &graph, &isSourceSet, &isTargetSet](It, It)
    {
        // Check if Target and Source values are set
        if(isTargetSet && isSourceSet)
        {
            NodeId sourceId = nodeIndexMap[source];
            NodeId targetId = nodeIndexMap[target];
            graph.addEdge(sourceId, targetId);
            isTargetSet = false;
            isSourceSet = false;
        }
    });

    // Edge Rules
    axe::r_rule<It> edgeKeyValuePair;
    auto edgeKeyValueList =  (whitespace & "edge" & whitespace & '[' & whitespace &
                            axe::r_many(edgeKeyValuePair - axe::r_char(']'), axe::r_any(" \t\n\r"), 0) & whitespace &
                            ']' ) >> captureEdge;
    auto edgeSource = (whitespace & "source" & whitespace & value >> captureEdgeSource);
    auto edgeTarget = (whitespace & "target" & whitespace & value >> captureEdgeTarget);
    edgeKeyValuePair = edgeSource | edgeTarget | keyValuePair | keyValueList;

    // Full GML file rule
    auto file = +(keyValue & axe::r_any(" \t\n\r"));
    bool succeeded = true;

    // Failure capture event
    auto onFail = [&succeeded, &begin](It, It)
    {
        succeeded = false;
    };

    // All GML keyValue options
    keyValue = ((edgeKeyValueList | nodeKeyValueList | keyValueList | keyValuePair | +axe::r_any(" \t\n\r") |
              axe::r_end() | axe::r_fail(onFail)) >> captureCount);

    // Perform file rule against begin & end iterators
    (file >> axe::e_ref([this](It, It)
    {
        emit progress(100);
    }) & axe::r_end())(begin, end);

    return succeeded;
}

bool GmlFileParser::parse(MutableGraph &graph)
{
    std::ifstream stream(_filename.toStdString());
    stream.unsetf(std::ios::skipws);

    std::istreambuf_iterator<char> startIt(stream.rdbuf());

    QFileInfo info(_filename);

    std::vector<char> vec(startIt, std::istreambuf_iterator<char>());
    if(!info.exists())
        return false;

    return parse(graph, vec.begin(), vec.end());;
}
