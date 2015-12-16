import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import com.kajeka 1.0

Item
{
    id: root

    property Application application

    property url fileUrl
    property string fileType

    property string title: document.title
    property string status: document.status

    property bool idle: document.idle
    property bool canDelete: document.canDelete

    property bool commandInProgress: document.commandInProgress
    property int commandProgress: document.commandProgress
    property string commandVerb: document.commandVerb

    property bool layoutIsPaused: document.layoutIsPaused

    property bool canUndo : document.canUndo
    property string nextUndoAction: document.nextUndoAction
    property bool canRedo: document.canRedo
    property string nextRedoAction: document.nextRedoAction

    property bool canResetView: document.canResetView
    property bool canEnterOverviewMode: document.canEnterOverviewMode

    property bool debugPauserEnabled: document.debugPauserEnabled
    property bool debugPaused: document.debugPaused
    property string debugResumeAction: document.debugResumeAction

    function openFile(fileUrl, fileType)
    {
        if(document.openFile(fileUrl, fileType))
        {
            this.fileUrl = fileUrl;
            this.fileType = fileType;

            return true;
        }

        return false;
    }

    function toggleLayout() { document.toggleLayout(); }
    function selectAll() { document.selectAll(); }
    function selectNone() { document.selectNone(); }
    function invertSelection() { document.invertSelection(); }
    function undo() { document.undo(); }
    function redo() { document.redo(); }
    function deleteSelectedNodes() { document.deleteSelectedNodes(); }
    function resetView() { document.resetView(); }
    function switchToOverviewMode() { document.switchToOverviewMode(); }

    function toggleDebugPauser() { document.toggleDebugPauser(); }
    function debugResume() { document.debugResume(); }
    function dumpGraph() { document.dumpGraph(); }

    SplitView
    {
        anchors.fill: parent
        orientation: Qt.Vertical

        Item
        {
            id: graphItem

            Layout.fillHeight: true
            Layout.minimumHeight: 100

            Graph
            {
                id: graph
                anchors.fill: parent
            }

            Column
            {
                anchors.right: parent.right
                anchors.top: parent.top

                Repeater
                {
                    model: document.transforms
                    Transform
                    {
                        enabled: document.idle

                        // Not entirely sure why parent is ever null, but it is
                        anchors.right: parent ? parent.right : undefined
                    }
                }
            }

            Text
            {
                visible: toggleGraphMetricsAction.checked

                anchors.right: parent.right
                anchors.bottom: parent.bottom

                horizontalAlignment: Text.AlignRight
                text:
                {
                    // http://stackoverflow.com/questions/9461621
                    function nFormatter(num, digits)
                    {
                        var si =
                            [
                                { value: 1E9,  symbol: "G" },
                                { value: 1E6,  symbol: "M" },
                                { value: 1E3,  symbol: "k" }
                            ], i;

                        for(i = 0; i < si.length; i++)
                        {
                            if(num >= si[i].value)
                                return (num / si[i].value).toFixed(digits).replace(/\.?0+$/, "") + si[i].symbol;
                        }

                        return num;
                    }

                    var s = "";
                    var numNodes = graph.numNodes;
                    var numEdges = graph.numEdges;
                    var numVisibleNodes = graph.numVisibleNodes;
                    var numVisibleEdges = graph.numVisibleEdges;

                    if(numNodes >= 0)
                    {
                        s += nFormatter(numNodes, 1);
                        if(numVisibleNodes !== numNodes)
                            s += " (" + nFormatter(numVisibleNodes, 1) + ")";
                        s += " nodes";
                    }

                    if(numEdges >= 0)
                    {
                        s += "\n" + nFormatter(numEdges, 1);
                        if(numVisibleEdges !== numEdges)
                            s += " (" + nFormatter(numVisibleEdges, 1) + ")";
                        s += " edges";
                    }

                    if(graph.numComponents >= 0)
                        s += "\n" + nFormatter(graph.numComponents, 1) + " components";

                    return s;
                }
            }

            Column
            {
                id: layoutSettings

                visible: toggleLayoutSettingsAction.checked

                anchors.left: parent.left
                anchors.bottom: parent.bottom

                Repeater
                {
                    model: document.layoutSettings
                    LayoutSettingUI { }
                }
            }
        }

        Item
        {
            id: contentItem
            Layout.minimumHeight: 100
            visible: document.contentQmlPath
        }
    }

    Document
    {
        id: document
        application: root.application
        graph: graph

        onContentQmlPathChanged:
        {
            if(document.contentQmlPath)
            {
                // Destroy anything already there
                while(contentItem.children.length > 0)
                    contentItem.children[0].destroy();

                var contentComponent = Qt.createComponent(document.contentQmlPath);
                var contentObject = contentComponent.createObject(contentItem);

                if(contentObject === null)
                    console.log(document.contentQmlPath + " failed to load");
            }
        }
    }
}
