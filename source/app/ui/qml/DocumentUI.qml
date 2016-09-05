import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2

import com.kajeka 1.0

import "Constants.js" as Constants

Item
{
    id: root

    property Application application

    property url fileUrl
    property string fileType
    property string pluginName

    property string title: document.title
    property string status: document.status

    property bool idle: document.idle
    property bool canDelete: document.canDelete

    property bool commandInProgress: document.commandInProgress
    property int commandProgress: document.commandProgress
    property string commandVerb: document.commandVerb

    property int layoutPauseState: document.layoutPauseState

    property bool canUndo : document.canUndo
    property string nextUndoAction: document.nextUndoAction
    property bool canRedo: document.canRedo
    property string nextRedoAction: document.nextRedoAction

    property bool canResetView: document.canResetView
    property bool canEnterOverviewMode: document.canEnterOverviewMode

    property bool debugPauserEnabled: document.debugPauserEnabled
    property bool debugPaused: document.debugPaused

    property string debugResumeAction: document.debugResumeAction

    property bool hasPluginUI: document.pluginQmlPath
    property bool poppedOut: false

    property int foundIndex: document.foundIndex
    property int numNodesFound: document.numNodesFound

    function brightness(c)
    {
        return 0.299 * c.r + 0.587 * c.g + 0.114 * c.b
    }

    property color textColor:
    {
        return document.textColor;
    }

    property color disabledTextColor:
    {
        if(textColor === "black")
            return Qt.lighter("black");
        else if(textColor === "white")
            return Qt.darker("white");

        return "grey";
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property color backgroundColor
    }

    function openFile(fileUrl, fileType, pluginName)
    {
        if(document.openFile(fileUrl, fileType, pluginName))
        {
            this.fileUrl = fileUrl;
            this.fileType = fileType;
            this.pluginName = pluginName;
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
    function gotoNextComponent() { document.gotoNextComponent(); }
    function gotoPrevComponent() { document.gotoPrevComponent(); }

    function selectAllFound() { document.selectAllFound(); }
    function selectNextFound() { document.selectNextFound(); }
    function selectPrevFound() { document.selectPrevFound(); }
    function find(text) { document.find(text); }

    function toggleDebugPauser() { document.toggleDebugPauser(); }
    function debugResume() { document.debugResume(); }
    function dumpGraph() { document.dumpGraph(); }

    SplitView
    {
        id: splitView

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
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: Constants.margin
                spacing: Constants.spacing

                Find
                {
                    id: find
                    visible: false
                    document: root
                }

                Label
                {
                    visible: toggleFpsMeterAction.checked

                    color: root.textColor

                    horizontalAlignment: Text.AlignLeft
                    text: document.fps.toFixed(1) + " fps"
                }
            }

            Column
            {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Constants.margin

                Repeater
                {
                    model: document.transforms
                    Transform
                    {
                        color: root.textColor
                        disabledColor: root.disabledTextColor
                        enabled: document.idle

                        // Not entirely sure why parent is ever null, but it is
                        anchors.right: parent ? parent.right : undefined
                    }
                }
            }

            Label
            {
                visible: toggleGraphMetricsAction.checked

                color: root.textColor

                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: Constants.margin

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
                anchors.margins: Constants.margin

                Repeater
                {
                    model: document.layoutSettings
                    LayoutSettingUI
                    {
                        textColor: root.textColor
                    }
                }
            }
        }
    }

    Item
    {
        id: plugin
        Layout.minimumHeight: 100
        visible: loaded && document.pluginQmlPath

        property var model: document.plugin
        property bool loaded: false

        onLoadedChanged:
        {
            //FIXME: restore window geometry of pluginWindow and
            //either pop in or pop out depending on preference
            var rootCoords = root.mapToItem(null, 0, 0)
            pluginWindow.x = rootCoords.x;
            pluginWindow.y = rootCoords.y;
            pluginWindow.width = 640;
            pluginWindow.height = 480;
            popInPlugin();
        }
    }

    property int pluginX: pluginWindow.x
    property int pluginY: pluginWindow.y
    property int pluginSplitSize:
    {
        if(!poppedOut)
        {
            return splitView.orientation == Qt.Vertical ?
                        plugin.height : plugin.width;
        }
        else
            return pluginSplitSize;
    }

    Window
    {
        id: pluginWindow
        title: application && root.pluginName.length > 0 ?
                   root.pluginName + " - " + application.name : "";
        visible: root.visible && root.poppedOut && plugin.loaded

        //FIXME: window is always on top?
        flags: Qt.Window

        onClosing:
        {
            if(visible)
                popInPlugin();
        }
    }

    function popOutPlugin()
    {
        root.poppedOut = true;
        splitView.removeItem(plugin);
        plugin.parent = pluginWindow.contentItem;
        plugin.anchors.fill = plugin.parent;

        pluginWindow.x = pluginX;
        pluginWindow.y = pluginY;
    }

    function popInPlugin()
    {
        plugin.parent = null;
        plugin.anchors.fill = null;

        if(splitView.orientation == Qt.Vertical)
            plugin.height = pluginSplitSize;
        else
            plugin.width = pluginSplitSize;

        splitView.addItem(plugin);
        root.poppedOut = false;
    }

    function togglePop()
    {
        if(pluginWindow.visible)
            popInPlugin();
        else
            popOutPlugin();
    }

    property bool findVisible: find.visible
    function showFind()
    {
        find.show();
    }

    Document
    {
        id: document
        application: root.application
        graph: graph

        onPluginQmlPathChanged:
        {
            if(document.pluginQmlPath)
            {
                // Destroy anything already there
                while(plugin.children.length > 0)
                    plugin.children[0].destroy();

                var pluginComponent = Qt.createComponent(document.pluginQmlPath);

                if(pluginComponent.status !== Component.Ready)
                {
                    console.log(pluginComponent.errorString());
                    return;
                }

                var pluginObject = pluginComponent.createObject(plugin);

                if(pluginObject === null)
                {
                    console.log(document.pluginQmlPath + ": failed to create instance");
                    return;
                }

                plugin.loaded = true;
            }
        }
    }

    property var comandProgressSamples: []
    property int commandSecondsRemaining

    onCommandProgressChanged:
    {
        // Reset the sample buffer if the command progress is less than the latest sample (i.e. new command)
        if(comandProgressSamples.length > 0 && commandProgress < comandProgressSamples[comandProgressSamples.length - 1].progress)
            comandProgressSamples.length = 0;

        if(commandProgress < 0)
        {
            commandSecondsRemaining = 0;
            return;
        }

        var sample = {progress: commandProgress, seconds: new Date().getTime() / 1000.0};
        comandProgressSamples.push(sample);

        // Only keep this many samples
        while(comandProgressSamples.length > 10)
            comandProgressSamples.shift();

        // Require a few samples before making the calculation
        if(comandProgressSamples.length < 5)
        {
            commandSecondsRemaining = 0;
            return;
        }

        var earliestSample = comandProgressSamples[0];
        var latestSample = comandProgressSamples[comandProgressSamples.length - 1];
        var percentDelta = latestSample.progress - earliestSample.progress;
        var timeDelta = latestSample.seconds - earliestSample.seconds;
        var percentRemaining = 100.0 - currentDocument.commandProgress;

        commandSecondsRemaining = percentRemaining * timeDelta / percentDelta;
    }

    signal commandComplete()

    onCommandInProgressChanged:
    {
        if(!commandInProgress)
            commandComplete();
    }
}
