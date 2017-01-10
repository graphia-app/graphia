import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQml.Models 2.2

import com.kajeka 1.0

import "Constants.js" as Constants

import "Transform"

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
    property bool pluginPoppedOut: false

    property int foundIndex: document.foundIndex
    property int numNodesFound: document.numNodesFound

    property var selectPreviousFoundAction: find.selectPreviousAction
    property var selectNextFoundAction: find.selectNextAction

    property color contrastingColor:
    {
        return document.contrastingColor;
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property string backgroundColor
    }

    property bool _darkBackground: { return Qt.colorEqual(contrastingColor, "white"); }
    property bool _brightBackground: { return Qt.colorEqual(contrastingColor, "black"); }

    function hexToRgb(hex)
    {
        hex = hex.replace("#", "");
        var bigint = parseInt(hex, 16);
        var r = ((bigint >> 16) & 255) / 255.0;
        var g = ((bigint >> 8) & 255) / 255.0;
        var b = (bigint & 255) / 255.0;

        return { r: r, b: b, g: g };
    }

    function colorDiff(a, b)
    {
        if(a === undefined || a === null || a.length === 0)
            return 1.0;

        a = hexToRgb(a);

        var ab = 0.299 * a.r + 0.587 * a.g + 0.114 * a.b;
        var bb = 0.299 * b +   0.587 * b +   0.114 * b;

        return ab - bb;
    }

    property var _lesserContrastingColors:
    {
        var colors = [];

        if(_brightBackground)
            colors = [0.0, 0.4, 0.8];

        colors = [1.0, 0.6, 0.3];

        var color1Diff = colorDiff(visuals.backgroundColor, colors[1]);
        var color2Diff = colorDiff(visuals.backgroundColor, colors[2]);

        // If either of the colors are very similar to the background color,
        // move it towards one of the others, depending on whether it's
        // lighter or darker
        if(Math.abs(color1Diff) < 0.15)
        {
            if(color1Diff < 0.0)
                colors[1] = (colors[0] + colors[1]) * 0.5;
            else
                colors[1] = (colors[1] + colors[2]) * 0.5;
        }
        else if(Math.abs(color2Diff) < 0.15)
        {
            if(color2Diff < 0.0)
                colors[2] = (colors[2]) * 0.5;
            else
                colors[2] = (colors[1] + colors[2]) * 0.5;
        }

        return colors;
    }

    property color lessContrastingColor: { return Qt.rgba(_lesserContrastingColors[1],
                                                          _lesserContrastingColors[1],
                                                          _lesserContrastingColors[1], 1.0); }
    property color leastContrastingColor: { return Qt.rgba(_lesserContrastingColors[2],
                                                           _lesserContrastingColors[2],
                                                           _lesserContrastingColors[2], 1.0); }

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

                    color: root.contrastingColor

                    horizontalAlignment: Text.AlignLeft
                    text: document.fps.toFixed(1) + " fps"
                }
            }

            Transforms
            {
                anchors.right: parent.right
                anchors.top: parent.top

                enabledTextColor: root.contrastingColor
                disabledTextColor: root.lessContrastingColor
                heldColor: root.leastContrastingColor

                document: document
            }

            Label
            {
                visible: toggleGraphMetricsAction.checked

                color: root.contrastingColor

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
                        textColor: root.contrastingColor
                    }
                }
            }
        }
    }

    Preferences
    {
        id: window
        section: "window"
        property alias pluginX: pluginWindow.x
        property alias pluginY: pluginWindow.y
        property int pluginWidth
        property int pluginHeight
        property bool pluginMaximised
        property alias pluginSplitSize: root.pluginSplitSize
        property alias pluginPoppedOut: root.pluginPoppedOut
    }

    Item
    {
        id: plugin
        Layout.minimumHeight: 100
        visible: loaded && enabledChildren

        property var model: document.plugin
        property bool loaded: false

        onLoadedChanged:
        {
            if(root.pluginPoppedOut)
                popOutPlugin();
            else
                popInPlugin();

            loadComplete();
        }

        // At least one enabled direct child
        property bool enabledChildren:
        {
            for(var i = 0; i < children.length; i++)
            {
                if(children[i].enabled)
                    return true;
            }

            return false;
        }
    }

    signal loadComplete()

    property int pluginX: pluginWindow.x
    property int pluginY: pluginWindow.y
    property int pluginSplitSize:
    {
        if(!pluginPoppedOut)
        {
            return splitView.orientation == Qt.Vertical ?
                        plugin.height : plugin.width;
        }
        else
            return window.pluginSplitSize
    }

    function loadPluginWindowState()
    {
        if(window.pluginWidth !== undefined &&
           window.pluginHeight !== undefined)
        {
            pluginWindow.width = window.pluginWidth;
            pluginWindow.height = window.pluginHeight;
        }

        if(window.pluginMaximised !== undefined)
            pluginWindow.visibility = window.pluginMaximised ? Window.Maximized : Window.Windowed;
    }

    function savePluginWindowState()
    {
        window.pluginMaximised = pluginWindow.maximised;

        if(!pluginWindow.maximised)
        {
            window.pluginWidth = pluginWindow.width;
            window.pluginHeight = pluginWindow.height;
        }
    }

    Window
    {
        id: pluginWindow
        width: 800
        height: 600
        minimumWidth: 480
        minimumHeight: 480
        title: application && root.pluginName.length > 0 ?
                   root.pluginName + " - " + application.name : "";
        visible: root.visible && root.pluginPoppedOut && plugin.loaded
        property bool maximised: visibility === Window.Maximized

        //FIXME: window is always on top?
        flags: Qt.Window

        onVisibleChanged:
        {
            if(visible)
                loadPluginWindowState();
            else
                savePluginWindowState();
        }

        onClosing:
        {
            if(visible)
                popInPlugin();
        }
    }

    Component.onDestruction:
    {
        savePluginWindowState();
    }

    function popOutPlugin()
    {
        root.pluginPoppedOut = true;
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
        root.pluginPoppedOut = false;
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
