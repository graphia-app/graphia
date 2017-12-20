#include "gmlfileparser.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/plugins/userelementdata.h"

#include "thirdparty/axe/include/axe.h"

#include <QTime>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

#include <fstream>
#include <cstring>
#include <iterator>

template<typename It> bool parseGml(IGraphModel &graphModel,
                                    UserNodeData* userNodeData,
                                    const std::function<bool ()>& cancelled,
                                    const ProgressFn& progressFn,
                                    It begin, It end)
{
    // General GML structure rules
    axe::r_rule<It> keyValue;
    double d;

    auto whitespace = axe::r_ref([&cancelled](It i1, It i2)
    {
        if(!cancelled())
        {
            return axe::make_result(i1 != i2 && std::strchr(" \t\n\r", *i1),
                                    i1 != i2 ? std::next(i1) : i1, i1);
        }

        return axe::make_result(false, i2);
    });

    // If this is declared and initialised on the same line, the move constructor for
    // axe::r_rule is called which (for some reason) infinitely recurses, whereas using
    // operator= doesn't
    axe::r_rule<It> quotedString;
    quotedString = '"' & *(R"(\")" | (axe::r_any() - '"')) & '"';

    auto key = (+axe::r_alnum());
    auto value = axe::r_double(d) | quotedString;

    auto keyValueList = *whitespace & key & *whitespace & '[' & *whitespace &
            axe::r_many(keyValue - axe::r_char(']'), axe::r_any(" \t\n\r"), 0) & *whitespace & ']';
    auto keyValuePair = (*whitespace & key & *whitespace & value);

    // Node State
    int id = -1;
    std::string label;
    std::map<int, NodeId> nodeIndexMap;

    // Node Capture Events
    auto captureNodeId = axe::e_ref([&id](It i1, It i2)
    {
        id = std::stoi(std::string(i1, i2));
    });

    auto captureLabel = axe::e_ref([&label](It i1, It i2)
    {
        label = std::string(i1, i2);
    });

    auto captureNode = axe::e_ref([&nodeIndexMap, &id, &graphModel, &label,
                                  &userNodeData](It, It)
    {
        if(id >= 0)
        {
            nodeIndexMap[id] = graphModel.mutableGraph().addNode();

            if(userNodeData != nullptr)
            {
                // If we don't have a label, use the id
                if(label.empty())
                    label = std::to_string(id);

                userNodeData->setValue(nodeIndexMap[id], QObject::tr("Node Name"),
                                       QString::fromStdString(label));
                graphModel.setNodeName(nodeIndexMap[id], QString::fromStdString(label));
            }

            label.clear();
            id = -1;
        }
    });

    // Node Rules
    axe::r_rule<It> nodeKeyValuePair;
    auto nodeKeyValueList = (*whitespace & "node" & *whitespace & '[' & *whitespace &
                           axe::r_many(nodeKeyValuePair - axe::r_char(']'), axe::r_any(" \t\n\r"), 0) & *whitespace
                           & ']') >> captureNode;
    auto nodeId = *whitespace & "id" & *whitespace & value >> captureNodeId;
    auto labelElement = *whitespace & "label" & *whitespace & value >> captureLabel;
    nodeKeyValuePair = nodeId | labelElement | keyValuePair | keyValueList;

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

    auto captureEdge = axe::e_ref([&source, &target, &nodeIndexMap, &graphModel, &isSourceSet, &isTargetSet](It, It)
    {
        // Check if Target and Source values are set
        if(isTargetSet && isSourceSet)
        {
            NodeId sourceId = nodeIndexMap[source];
            NodeId targetId = nodeIndexMap[target];
            graphModel.mutableGraph().addEdge(sourceId, targetId);
            isTargetSet = false;
            isSourceSet = false;
        }
    });

    // Edge Rules
    axe::r_rule<It> edgeKeyValuePair;
    auto edgeKeyValueList = (*whitespace & "edge" & *whitespace & '[' & *whitespace &
                            axe::r_many(edgeKeyValuePair - axe::r_char(']'), axe::r_any(" \t\n\r"), 0) & *whitespace &
                            ']' ) >> captureEdge;
    auto edgeSource = (*whitespace & "source" & *whitespace & value >> captureEdgeSource);
    auto edgeTarget = (*whitespace & "target" & *whitespace & value >> captureEdgeTarget);
    edgeKeyValuePair = edgeSource | edgeTarget | keyValuePair | keyValueList;

    // Full GML file rule
    auto file = +(keyValue & axe::r_any(" \t\n\r"));
    bool succeeded = true;

    // Failure capture event
    auto onFail = [&succeeded](It, It)
    {
        succeeded = false;
    };

    // Progress Capture event (Fired on keyValue rule match)
    auto captureCount = axe::e_ref([&begin, &end, &progressFn](It, It i2)
    {
        progressFn((std::distance(begin, i2)) * 100 / std::distance(begin, end));
    });

    // All GML keyValue options
    keyValue = (edgeKeyValueList | nodeKeyValueList | keyValueList | keyValuePair | +whitespace |
        axe::r_end() | axe::r_fail(onFail)) >> captureCount;

    // Perform file rule against begin & end iterators
    (file >> axe::e_ref([&progressFn](It, It)
    {
        progressFn(100);
    }) & axe::r_end())(begin, end);

    return succeeded;
}

GmlFileParser::GmlFileParser(UserNodeData* userNodeData) :
    _userNodeData(userNodeData)
{
    _userNodeData->add(QObject::tr("Node Name"));
}

bool GmlFileParser::parse(const QUrl& url, IGraphModel& graphModel, const ProgressFn& progressFn)
{
    QString localFile = url.toLocalFile();
    std::ifstream stream(localFile.toStdString());
    stream.unsetf(std::ios::skipws);

    std::istreambuf_iterator<char> startIt(stream.rdbuf());

    if(!QFileInfo::exists(localFile))
        return false;

    std::vector<char> vec(startIt, std::istreambuf_iterator<char>());

    progressFn(-1);

    return parseGml(graphModel, _userNodeData, [this] { return cancelled(); },
        progressFn, vec.begin(), vec.end());
}
