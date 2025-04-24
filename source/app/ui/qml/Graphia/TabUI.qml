/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

import QtQml
import QtQml.Models
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs

import Graphia
import Graphia.Controls
import Graphia.Plugins
import Graphia.SharedTypes
import Graphia.Utils

Item
{
    id: root

    property Application application

    property url url
    property url savedFileUrl
    property string savedFileSaver

    property bool hasBeenSaved: { return Qt.resolvedUrl(savedFileUrl).toString().length > 0; }

    property string baseFileName:
    {
        if(hasBeenSaved)
            return NativeUtils.baseFileNameForUrl(root.savedFileUrl);
        else if(Qt.resolvedUrl(root.url).toString().length > 0)
            return NativeUtils.baseFileNameForUrl(root.url);

        return "";
    }

    property string baseFileNameNoExtension:
    {
        if(hasBeenSaved)
            return NativeUtils.baseFileNameForUrlNoExtension(root.savedFileUrl);
        else if(Qt.resolvedUrl(root.url).toString().length > 0)
            return NativeUtils.baseFileNameForUrlNoExtension(root.url);

        return "";
    }

    property string title:
    {
        let text;

        if(hasBeenSaved)
        {
            // Don't display the file extension when it's a native file
            text = baseFileNameNoExtension;
        }
        else
            text = baseFileName;

        if(baseFileName.length > 0 && saveRequired)
            text = "*" + text;

        return text;
    }

    readonly property string appName: application !== null ? application.name : ""

    property alias document: _document

    property bool pluginPoppedOut: false
    property bool pluginMinimised: false

    onPluginMinimisedChanged:
    {
        if(pluginMinimised)
            pluginToolBarContainer.minimisingOrMinimised = true;

        pluginToolBarContainer.transitioning = true;
    }

    property alias pluginMenu0: pluginMenu0
    property alias pluginMenu1: pluginMenu1
    property alias pluginMenu2: pluginMenu2
    property alias pluginMenu3: pluginMenu3
    property alias pluginMenu4: pluginMenu4

    property var previousAction: find.previousAction
    property var nextAction: find.nextAction

    property bool saveRequired: _document.loadComplete &&
        (!hasBeenSaved || _document.saveRequired || plugin.saveRequired) &&
        !tutorial.activated // The tutorial file should never be saved

    readonly property bool panelVisible: find.showing ||
        addBookmark.showing || layoutSettings.showing

    SortFilterProxyModel
    {
        id: sharedValuesProxyModel

        property var model: null
        sourceModel: model

        filterRoleName: "hasSharedValues"
        filterRegularExpression: /true/
    }

    property int numAttributesWithSharedValues:
    {
        return sharedValuesProxyModel.count;
    }

    onNumAttributesWithSharedValuesChanged:
    {
        // If (for whatever reason) we end up with no shared attribute values, and
        // FBAV is showing, we need to hide it or the UI will end up in a weird state
        if(root.findType === Find.ByAttribute && numAttributesWithSharedValues <= 0)
            hideFind();
    }

    property var sharedValuesAttributeNames:
    {
        let attributeNames = [];

        for(let i = 0; i < sharedValuesProxyModel.count; i++)
            attributeNames.push(sharedValuesProxyModel.get(i));

        return attributeNames;
    }

    property string _lastSharedValueAttributeName

    function _refreshAttributesWithSharedValues()
    {
        sharedValuesProxyModel.model =
            _document.availableAttributesModel(ElementType.Node);

        if(!_document.attributeExists(_lastSharedValueAttributeName))
            _lastSharedValueAttributeName = "";
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property string backgroundColor
    }

    property bool _darkBackground: { return Qt.colorEqual(_document.contrastingColor, "white"); }
    property bool _brightBackground: { return Qt.colorEqual(_document.contrastingColor, "black"); }

    function hexToRgb(hex)
    {
        hex = hex.replace("#", "");
        let bigint = parseInt(hex, 16);
        let r = ((bigint >> 16) & 255) / 255.0;
        let g = ((bigint >> 8) & 255) / 255.0;
        let b = (bigint & 255) / 255.0;

        return { r: r, b: b, g: g };
    }

    function colorDiff(a, b)
    {
        if(a === undefined || a === null || a.length === 0)
            return 1.0;

        a = hexToRgb(a);

        let ab = 0.299 * a.r + 0.587 * a.g + 0.114 * a.b;
        let bb = 0.299 * b +   0.587 * b +   0.114 * b;

        return ab - bb;
    }

    property var _lesserContrastingColors:
    {
        let colors = [];

        if(_brightBackground)
            colors = [0.0, 0.4, 0.8];

        colors = [1.0, 0.6, 0.3];

        let color1Diff = colorDiff(visuals.backgroundColor, colors[1]);
        let color2Diff = colorDiff(visuals.backgroundColor, colors[2]);

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

    function openUrl(url, type, pluginName, parameters)
    {
        if(!_document.openUrl(url, type, pluginName, parameters))
            return false;

        root.url = url;

        if(type === "Native")
        {
            root.savedFileUrl = url;
            root.savedFileSaver = appName;
        }

        return true;
    }

    function saveAsNamedFile(desiredFileUrl, saverName)
    {
        let uiData =
        {
            lastAdvancedFindAttributeName: find.lastAdvancedFindAttributeName,
            lastFindByAttributeName: find.lastFindByAttributeName
        };

        uiData = JSON.stringify(uiData);

        let pluginUiData = plugin.save();

        if(typeof(pluginUiData) === "object")
            pluginUiData = JSON.stringify(pluginUiData);

        _document.saveFile(desiredFileUrl, saverName, uiData, pluginUiData);
    }

    SaveFileDialog
    {
        id: fileSaveDialog

        title: qsTr("Save File…")
        nameFilters:
        {
            let filters = [];
            let saverTypes = application ? application.saverFileTypes() : [];
            for(let i = 0; i < saverTypes.length; i++)
                filters.push(Utils.format(qsTr("{0} files (*.{1})"), saverTypes[i].name, saverTypes[i].extension));

            filters.push(qsTr("All files (*)"));
            return filters;
        }

        onAccepted:
        {
            let saverName = "";
            let saverTypes = application.saverFileTypes();

            // If no saver is found fall back to default saver (All Files filter will do this)
            if(selectedNameFilter.index < saverTypes.length)
                saverName = saverTypes[selectedNameFilter.index].name;
            else
                saverName = appName;

            misc.fileSaveInitialFolder = currentFolder.toString();
            saveAsNamedFile(selectedFile, saverName);
        }
    }

    function saveAsFile()
    {
        let initialFileUrl = NativeUtils.removeExtension(root.hasBeenSaved ? root.savedFileUrl : root.url);

        fileSaveDialog.selectedFile = initialFileUrl;
        fileSaveDialog.currentFolder = misc.fileSaveInitialFolder !== undefined ? misc.fileSaveInitialFolder: "";
        fileSaveDialog.open();
    }

    function saveFile()
    {
        if(!hasBeenSaved)
            saveAsFile();
        else
            saveAsNamedFile(root.savedFileUrl, savedFileSaver);
    }

    MessageDialog
    {
        id: saveConfirmDialog

        property string fileName
        property var onSaveConfirmedFunction

        title: qsTr("File Changed")
        text: Utils.format(qsTr("Do you want to save changes to '{0}'?"), baseFileName)
        buttons: MessageDialog.Save | MessageDialog.Discard | MessageDialog.Cancel
        modality: Qt.ApplicationModal

        onButtonClicked: function(button, role)
        {
            switch(button)
            {
            case MessageDialog.Save:
            {
                // Capture onSaveConfirmedFunction so that it doesn't get overwritten before
                // we use it (though in theory that can't happen)
                let proxyFn = function(fn)
                {
                    return function()
                    {
                        _document.saveComplete.disconnect(proxyFn);
                        fn();
                    };
                }(onSaveConfirmedFunction);
                onSaveConfirmedFunction = null;

                _document.saveComplete.connect(proxyFn);
                saveFile();
                break;
            }

            case MessageDialog.Discard:
                onSaveConfirmedFunction();
                onSaveConfirmedFunction = null;
                break;

            default:
                break;
            }
        }
    }

    function confirmSave(onSaveConfirmedFunction)
    {
        if(saveRequired)
        {
            saveConfirmDialog.onSaveConfirmedFunction = onSaveConfirmedFunction;
            saveConfirmDialog.open();
        }
        else
            onSaveConfirmedFunction();
    }

    function selectSources(nodeId, add)
    {
        _lastSelectType = TabUI.LS_Sources;
        if(typeof(nodeId) !== "undefined" && typeof(add) !== "undefined")
            _document.selectSources(nodeId, add);
        else
            _document.selectSources();
    }

    function selectTargets(nodeId, add)
    {
        _lastSelectType = TabUI.LS_Targets;
        if(typeof(nodeId) !== "undefined" && typeof(add) !== "undefined")
            _document.selectTargets(nodeId, add);
        else
            _document.selectTargets();
    }

    function selectNeighbours(nodeId, add)
    {
        _lastSelectType = TabUI.LS_Neighbours;

        if(typeof(nodeId) !== "undefined" && typeof(add) !== "undefined")
            _document.selectNeighbours(nodeId, add);
        else
            _document.selectNeighbours();
    }

    function selectBySharedAttributeValue(attributeName, nodeId, add)
    {
        _lastSelectType = TabUI.LS_BySharedValue;
        _lastSharedValueAttributeName = attributeName;

        if(typeof(nodeId) !== "undefined" && typeof(add) !== "undefined")
            _document.selectBySharedAttributeValue(attributeName, nodeId, add);
        else
            _document.selectBySharedAttributeValue(attributeName);
    }

    enum LastSelectType
    {
        LS_None,
        LS_Neighbours,
        LS_Sources,
        LS_Targets,
        LS_BySharedValue
    }

    property int _lastSelectType: TabUI.LS_None

    property bool canRepeatLastSelection:
    {
        if(_document.nodeSelectionEmpty)
            return false;

        if(root._lastSelectType === TabUI.LS_BySharedValue)
            return _lastSharedValueAttributeName.length > 0;

        return root._lastSelectType !== TabUI.LS_None;
    }

    property string repeatLastSelectionMenuText:
    {
        switch(root._lastSelectType)
        {
        default:
        case TabUI.LS_None:
            break;
        case TabUI.LS_Neighbours:
            return qsTr("Repeat Last Selection (Neighbours)");
        case TabUI.LS_Sources:
            return qsTr("Repeat Last Selection (Sources)");
        case TabUI.LS_Targets:
            return qsTr("Repeat Last Selection (Targets)");
        case TabUI.LS_BySharedValue:
            if(_lastSharedValueAttributeName.length > 0)
            {
                return Utils.format(qsTr("Repeat Last Selection ({0} Value)"),
                    _lastSharedValueAttributeName);
            }
            break;
        }

        return qsTr("Repeat Last Selection");
    }

    function repeatLastSelection(nodeId, add)
    {
        let clickedNodeId = typeof(nodeId) !== "undefined" ?
            nodeId : contextMenu.clickedNodeId;

        let addToSelection = typeof(add) !== "undefined" ?
            add : undefined;

        switch(root._lastSelectType)
        {
        default:
        case TabUI.LS_None:
            return;
        case TabUI.LS_Neighbours:
            selectNeighbours(clickedNodeId, add);
            return;
        case TabUI.LS_Sources:
            selectSources(clickedNodeId, add);
            return;
        case TabUI.LS_Targets:
            selectTargets(clickedNodeId, add);
            return;
        case TabUI.LS_BySharedValue:
            selectBySharedAttributeValue(_lastSharedValueAttributeName, clickedNodeId, add);
            return;
        }
    }

    function screenshot() { Utils.createWindow(root, captureScreenshotDialog); }

    function copyImageToClipboard()
    {
        graph.grabToImage(function(result)
        {
            application.copyImageToClipboard(result.image);
            _document.status = qsTr("Copied Viewport To Clipboard");
        });
    }

    function showAddBookmark()
    {
        addBookmark.show();
    }

    function gotoBookmark(name)
    {
        hideFind();
        _document.gotoBookmark(name);
    }

    function gotoAllBookmarks()
    {
        hideFind();
        _document.gotoAllBookmarks();
    }

    SaveFileDialog
    {
        id: exportNodePositionsFileDialog

        title: qsTr("Export Node Positions")
        nameFilters: [qsTr("JSON File (*.json)")]

        onAccepted:
        {
            misc.fileSaveInitialFolder = exportNodePositionsFileDialog.currentFolder.toString();
            _document.saveNodePositionsToFile(exportNodePositionsFileDialog.selectedFile);
        }
    }

    function exportNodePositions()
    {
        let folder = misc.fileSaveInitialFolder !== undefined ?
            misc.fileSaveInitialFolder : "";
        let path = Utils.format(qsTr("{0}/{1}-node-positions"), NativeUtils.fileNameForUrl(folder),
            root.baseFileNameNoExtension);

        exportNodePositionsFileDialog.currentFolder = folder;
        exportNodePositionsFileDialog.selectedFile = NativeUtils.urlForFileName(path);
        exportNodePositionsFileDialog.open();
    }

    OpenFileDialog
    {
        id: nodePositionFileImportDialog

        title: qsTr("Import From File…")
        nameFilters: "Graphia Node Positions (*.json)"

        onAccepted:
        {
            misc.fileOpenInitialFolder = currentFolder.toString();
            _document.loadNodePositionsFromFile(selectedFile);
        }
    }

    function importNodePositions()
    {
        let folder = misc.fileOpenInitialFolder !== undefined ?
            misc.fileOpenInitialFolder : "";

        nodePositionFileImportDialog.currentFolder = folder;
        nodePositionFileImportDialog.open();
    }

    function searchWebForNode(nodeId)
    {
        let nodeName = _document.nodeName(nodeId);
        let url = misc.webSearchEngineUrl.indexOf("%1") >= 0 ?
            misc.webSearchEngineUrl.arg(nodeName) : "";

        if(NativeUtils.userUrlStringIsValid(url))
            Qt.openUrlExternally(NativeUtils.urlFrom(url));
    }

    Component
    {
        id: captureScreenshotDialog
        CaptureScreenshotDialog
        {
            graphView: graph
            application: root.application
        }
    }

    ColorDialog
    {
        id: backgroundColorDialog
        title: qsTr("Select a Colour")
        modality: Qt.ApplicationModal

        onSelectedColorChanged:
        {
            visuals.backgroundColor = selectedColor;
        }
    }

    Action
    {
        id: deleteNodeAction
        icon.name: "edit-delete"
        text: Utils.format(qsTr("&Delete '{0}'"), contextMenu.clickedNodeName)
        property bool visible: _document.editable && contextMenu.nodeWasClicked
        enabled: !_document.busy && visible
        onTriggered: { _document.deleteNode(contextMenu.clickedNodeId); }
    }

    Action
    {
        id: selectSourcesOfNodeAction
        text: Utils.format(qsTr("Select Sources of '{0}'"), contextMenu.clickedNodeName)
        property bool visible: _document.directed && contextMenu.nodeWasClicked
        enabled: !_document.busy && visible
        onTriggered: { selectSources(contextMenu.clickedNodeId, false); }
    }

    Action
    {
        id: selectTargetsOfNodeAction
        text: Utils.format(qsTr("Select Targets of '{0}'"), contextMenu.clickedNodeName)
        property bool visible: _document.directed && contextMenu.nodeWasClicked
        enabled: !_document.busy && visible
        onTriggered: { selectTargets(contextMenu.clickedNodeId, false); }
    }

    Action
    {
        id: selectNeighboursOfNodeAction
        text: Utils.format(qsTr("Select Neigh&bours of '{0}'"), contextMenu.clickedNodeName)
        property bool visible: contextMenu.nodeWasClicked
        enabled: !_document.busy && visible
        onTriggered: { selectNeighbours(contextMenu.clickedNodeId, false); }
    }

    SplitView
    {
        id: splitView

        anchors.fill: parent
        orientation: Qt.Vertical

        handle: SplitViewHandle {}

        Item
        {
            id: graphItem

            SplitView.fillHeight: true
            SplitView.minimumHeight: 200

            GraphDisplay
            {
                id: graph
                anchors.fill: parent
                enabled: !_document.graphChanging

                property bool _inComponentMode: !inOverviewMode && numComponents > 1

                PlatformMenu
                {
                    id: contextMenu

                    TextMetrics
                    {
                        id: elidedNodeName

                        elide: Text.ElideMiddle
                        elideWidth: 150
                        text: contextMenu.clickedNodeId !== undefined ?
                            _document.nodeName(contextMenu.clickedNodeId) : ""
                    }

                    property var clickedNodeId
                    property string clickedNodeName: elidedNodeName.elidedText
                    property bool nodeWasClicked: clickedNodeId !== undefined ? !clickedNodeId.isNull : false
                    property bool clickedNodeIsSameAsSelection:
                    {
                        return _document.numHeadNodesSelected === 1 &&
                            nodeWasClicked &&
                            _document.nodeIsSelected(clickedNodeId);
                    }

                    PlatformMenuItem { id: delete1; hidden: !deleteNodeAction.visible; action: deleteNodeAction }
                    PlatformMenuItem { id: delete2; hidden: !deleteAction.visible || contextMenu.clickedNodeIsSameAsSelection; action: deleteAction }
                    PlatformMenuSeparator { hidden: delete1.hidden && delete2.hidden }

                    PlatformMenuItem { hidden: _document.numNodesSelected === graph.numNodes; action: selectAllAction }
                    PlatformMenuItem { hidden: _document.numNodesSelected === graph.numNodes || graph.inOverviewMode; action: selectAllVisibleAction }
                    PlatformMenuItem { hidden: _document.nodeSelectionEmpty; action: selectNoneAction }
                    PlatformMenuItem { hidden: _document.nodeSelectionEmpty; action: invertSelectionAction }

                    PlatformMenuItem { hidden: !selectSourcesOfNodeAction.visible; action: selectSourcesOfNodeAction }
                    PlatformMenuItem { hidden: !selectTargetsOfNodeAction.visible; action: selectTargetsOfNodeAction }
                    PlatformMenuItem { hidden: !selectNeighboursOfNodeAction.visible; action: selectNeighboursOfNodeAction }
                    PlatformMenu
                    {
                        id: sharedValuesOfNodeContextMenu
                        enabled: !_document.busy && !hidden
                        hidden: numAttributesWithSharedValues === 0 || !contextMenu.nodeWasClicked
                        title: Utils.format(qsTr("Select Shared Values of '{0}'"), contextMenu.clickedNodeName)
                        Instantiator
                        {
                            model: sharedValuesAttributeNames
                            PlatformMenuItem
                            {
                                text: modelData
                                onTriggered: { selectBySharedAttributeValue(text, contextMenu.clickedNodeId); }
                            }
                            onObjectAdded: function(index, object) { sharedValuesOfNodeContextMenu.insertItem(index, object); }
                            onObjectRemoved: function(index, object) { sharedValuesOfNodeContextMenu.removeItem(object); }
                        }
                    }

                    PlatformMenuItem { hidden: _document.nodeSelectionEmpty || contextMenu.clickedNodeIsSameAsSelection ||
                        !selectSourcesAction.visible; action: selectSourcesAction }
                    PlatformMenuItem { hidden: _document.nodeSelectionEmpty || contextMenu.clickedNodeIsSameAsSelection ||
                        !selectTargetsAction.visible; action: selectTargetsAction }
                    PlatformMenuItem { hidden: _document.nodeSelectionEmpty || contextMenu.clickedNodeIsSameAsSelection ||
                        !selectNeighboursAction.visible; action: selectNeighboursAction }
                    PlatformMenu
                    {
                        id: sharedValuesSelectionContextMenu
                        enabled: !_document.busy && !hidden
                        hidden: numAttributesWithSharedValues === 0 || _document.nodeSelectionEmpty ||
                            contextMenu.clickedNodeIsSameAsSelection
                        title: qsTr('Select Shared Values of Selection')
                        Instantiator
                        {
                            model: sharedValuesAttributeNames
                            PlatformMenuItem
                            {
                                text: modelData
                                onTriggered: { selectBySharedAttributeValue(text); }
                            }
                            onObjectAdded: function(index, object) { sharedValuesSelectionContextMenu.insertItem(index, object); }
                            onObjectRemoved: function(index, object) { sharedValuesSelectionContextMenu.removeItem(object); }
                        }
                    }
                    PlatformMenuItem { hidden: !repeatLastSelectionAction.enabled; action: repeatLastSelectionAction }

                    PlatformMenuSeparator { hidden: searchWebMenuItem.hidden }
                    PlatformMenuItem
                    {
                        id: searchWebMenuItem
                        hidden: !contextMenu.nodeWasClicked
                        text: Utils.format(qsTr("Search Web for '{0}'"), contextMenu.clickedNodeName)
                        onTriggered: { root.searchWebForNode(contextMenu.clickedNodeId); }
                    }

                    PlatformMenuSeparator { hidden: changeBackgroundColourMenuItem.hidden }
                    PlatformMenuItem
                    {
                        id: changeBackgroundColourMenuItem
                        hidden: contextMenu.nodeWasClicked
                        text: qsTr("Change Background &Colour")
                        onTriggered:
                        {
                            backgroundColorDialog.selectedColor = visuals.backgroundColor;
                            backgroundColorDialog.open();
                        }
                    }
                }

                onClicked: function(button, modifiers, nodeId)
                {
                    switch(button)
                    {
                    case Qt.RightButton:
                        contextMenu.clickedNodeId = nodeId;
                        contextMenu.popup();
                        break;

                    case Qt.MiddleButton:
                        if(!nodeId.isNull)
                            root.repeatLastSelection(nodeId, modifiers & Qt.ShiftModifier);
                        break;
                    }
                }

                Label
                {
                    id: emptyGraphLabel
                    text: qsTr("Empty Graph")
                    font.pixelSize: 48
                    color: _document.contrastingColor
                    opacity: 0.0

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter

                    states: State
                    {
                        name: "showing"
                        when: plugin.loaded && graph.numNodes <= 0
                        PropertyChanges
                        {
                            target: emptyGraphLabel
                            opacity: 1.0
                        }
                    }

                    transitions: Transition
                    {
                        to: "showing"
                        reversible: true
                        NumberAnimation
                        {
                            properties: "opacity"
                            duration: 1000
                            easing.type: Easing.InOutSine
                        }
                    }
                }

                BusyIndicator
                {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter

                    width: parent.height * 0.3
                    height: width

                    visible: !graph.initialised && plugin.loaded
                }
            }

            FloatingButton
            {
                property bool _visible: graph._inComponentMode

                Behavior on opacity { NumberAnimation { easing.type: Easing.InOutQuad } }
                opacity: _visible ? 1.0 : 0.0
                visible: opacity > 0.0

                anchors.verticalCenter: graph.verticalCenter
                anchors.left: graph.left
                anchors.margins: 20

                icon.name: "go-previous"
                text: qsTr("Goto Previous Component");

                onClicked: function(mouse) { _document.gotoPrevComponent(); }
            }

            FloatingButton
            {
                property bool _visible: graph._inComponentMode

                Behavior on opacity { NumberAnimation { easing.type: Easing.InOutQuad } }
                opacity: _visible ? 1.0 : 0.0
                visible: opacity > 0.0

                anchors.verticalCenter: graph.verticalCenter
                anchors.right: graph.right
                anchors.margins: 20

                icon.name: "go-next"
                text: qsTr("Goto Next Component");

                onClicked: function(mouse) { _document.gotoNextComponent(); }
            }

            RowLayout
            {
                visible: graph._inComponentMode

                anchors.horizontalCenter: graph.horizontalCenter
                anchors.bottom: graph.bottom
                anchors.margins: 20

                FloatingButton
                {
                    icon.name: "edit-undo"
                    text: qsTr("Return to Overview Mode")
                    onClicked: function(mouse) { _document.switchToOverviewMode(); }
                }

                ColumnLayout
                {
                    Text
                    {
                        Layout.alignment: Qt.AlignHCenter

                        text:
                        {
                            return Utils.format(qsTr("Component {0} of {1}"),
                                graph.visibleComponentIndex, graph.numComponents);
                        }

                        color: _document.contrastingColor
                    }

                    Text
                    {
                        Layout.alignment: Qt.AlignHCenter
                        visible: _document.numInvisibleNodesSelected > 0

                        text:
                        {
                            let nodeText = _document.numInvisibleNodesSelected === 1 ? qsTr("node") : qsTr("nodes");
                            let numNodes = NativeUtils.formatNumberSIPostfix(_document.numInvisibleNodesSelected);
                            return Utils.format(qsTr("<i>({0} selected {1} not currently visible)</i>"), numNodes, nodeText);

                        }

                        color: _document.contrastingColor

                        textFormat: Text.StyledText
                    }
                }
            }

            Label
            {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: Constants.margin

                visible: toggleFpsMeterAction.checked

                color: _document.contrastingColor

                horizontalAlignment: Text.AlignLeft
                text: Utils.format(qsTr("{0} fps"), _document.fps.toFixed(1))
            }

            Column
            {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: Constants.margin

                GridLayout
                {
                    visible: plugin.loaded && toggleGraphMetricsAction.checked
                    anchors.left: parent.left

                    columns: 2
                    rows: 3

                    rowSpacing: 0

                    Label
                    {
                        color: _document.contrastingColor
                        text: qsTr("Nodes:")
                    }

                    Label
                    {
                        color: _document.contrastingColor
                        Layout.alignment: Qt.AlignRight
                        text:
                        {
                            let s = "";
                            let numNodes = graph.numNodes;
                            let numVisibleNodes = graph.numVisibleNodes;

                            if(numNodes >= 0)
                            {
                                s += NativeUtils.formatNumberSIPostfix(numNodes);
                                if(numVisibleNodes !== numNodes)
                                    s += Utils.format(qsTr(" ({0})"), NativeUtils.formatNumberSIPostfix(numVisibleNodes));
                            }

                            return s;
                        }
                    }

                    Label
                    {
                        color: _document.contrastingColor
                        text: qsTr("Edges:")
                    }

                    Label
                    {
                        color: _document.contrastingColor
                        Layout.alignment: Qt.AlignRight
                        text:
                        {
                            let s = "";
                            let numEdges = graph.numEdges;
                            let numVisibleEdges = graph.numVisibleEdges;

                            if(numEdges >= 0)
                            {
                                s += NativeUtils.formatNumberSIPostfix(numEdges);
                                if(numVisibleEdges !== numEdges)
                                    s += Utils.format(qsTr(" ({0})"), NativeUtils.formatNumberSIPostfix(numVisibleEdges));
                            }

                            return s;
                        }
                    }

                    Label
                    {
                        visible: !graph._inComponentMode
                        color: _document.contrastingColor
                        text: qsTr("Components:")
                    }

                    Label
                    {
                        visible: !graph._inComponentMode
                        color: _document.contrastingColor
                        Layout.alignment: Qt.AlignRight
                        text:
                        {
                            let s = "";

                            if(graph.numComponents >= 0)
                                s += NativeUtils.formatNumberSIPostfix(graph.numComponents);

                            return s;
                        }
                    }
                }
            }

            SlidingPanel
            {
                id: findPanel

                alignment: Qt.AlignTop|Qt.AlignLeft

                anchors.left: parent.left
                anchors.top: parent.top

                initiallyOpen: false
                disableItemWhenClosed: false

                item: Find
                {
                    id: find

                    document: _document

                    onShown: { findPanel.show(); }
                    onHidden: { findPanel.hide(); }
                }
            }

            SlidingPanel
            {
                id: bookmarkPanel

                alignment: Qt.AlignTop

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top

                initiallyOpen: false
                disableItemWhenClosed: false

                item: AddBookmark
                {
                    id: addBookmark

                    document: _document

                    onShown: { bookmarkPanel.show(); }
                    onHidden: { bookmarkPanel.hide(); }
                }
            }

            SlidingPanel
            {
                id: layoutSettingsPanel

                alignment: Qt.AlignBottom|Qt.AlignLeft

                anchors.left: parent.left
                anchors.bottom: parent.bottom

                initiallyOpen: false
                disableItemWhenClosed: false

                item: LayoutSettings
                {
                    id: layoutSettings

                    document: _document

                    onShown: { layoutSettingsPanel.show(); }
                    onHidden: { layoutSettingsPanel.hide(); }

                    onValueChanged: { _document.resumeLayout(); }
                }
            }

            Transforms
            {
                id: transforms
                visible: plugin.loaded
                enabled: !_document.busy

                anchors.right: parent.right
                anchors.top: parent.top

                enabledTextColor: _document.contrastingColor
                disabledTextColor: root.lessContrastingColor
                heldColor: root.leastContrastingColor

                document: _document
            }

            Visualisations
            {
                id: visualisations
                visible: plugin.loaded
                enabled: !_document.busy

                anchors.right: parent.right
                anchors.bottom: parent.bottom

                enabledTextColor: _document.contrastingColor
                disabledTextColor: root.lessContrastingColor
                heldColor: root.leastContrastingColor

                document: _document
            }
        }

        Item
        {
            id: pluginToolBarContainer
            visible: plugin.loaded && !root.pluginPoppedOut

            SplitView.fillWidth: true

            SplitView.minimumHeight:
            {
                let minimumHeight = toolBar.height;

                if(pluginToolBarContainer.minimisingOrMinimised)
                    return minimumHeight;

                if(plugin.content !== undefined && plugin.content.minimumHeight !== undefined)
                    minimumHeight += plugin.content.minimumHeight;
                else
                    minimumHeight += defaultPluginContent.minimumHeight;

                return minimumHeight;
            }

            SplitView.maximumHeight: root.pluginMinimised && !pluginToolBarContainer.transitioning ?
                toolBar.height : Number.POSITIVE_INFINITY;

            SplitView.onPreferredHeightChanged: { window.pluginSplitSize = SplitView.preferredHeight; }

            property real _lastUnminimisedHeight: 0

            onHeightChanged:
            {
                if(!pluginToolBarContainer.minimisingOrMinimised)
                {
                    _lastUnminimisedHeight = height;

                    // This is a hack to avoid some jarring visuals. In summary, Layouts seem to
                    // rely on first setting an item's dimensions past its minima/maxima, and then
                    // correcting. With most controls this is completely fine, but with the
                    // CorrelationPlot and WebView, this results in them being momentarily visible
                    // at the pre-corrected dimensions. This is probably because these items are
                    // rendered in threads, so there is a window where the uncorrected dimensions
                    // are visible. Here we delay setting the height on the plugin by a frame,
                    // so that it has had an event loop to update to the limits.
                    Qt.callLater(function()
                    {
                        let newHeight = height;

                        if(!root.pluginPoppedOut)
                            newHeight -= toolBar.height;

                        pluginContainer.height = newHeight;
                    });
                }
            }

            property bool minimisingOrMinimised: false
            property bool transitioning: false

            Behavior on SplitView.preferredHeight
            {
                // Only enable when actually transitioning or this will interfere
                // when dragging the SplitView manually
                enabled: !plugin.enabled && pluginToolBarContainer.transitioning

                SequentialAnimation
                {
                    NumberAnimation { easing.type: Easing.InOutQuad }

                    ScriptAction
                    {
                        script:
                        {
                            if(!root.pluginMinimised)
                                pluginToolBarContainer.minimisingOrMinimised = false;

                            pluginToolBarContainer.transitioning = false;
                        }
                    }
                }
            }

            ToolBar
            {
                id: toolBar

                // Stick it to the top
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                topPadding: Constants.padding
                bottomPadding: Constants.padding

                RowLayout
                {
                    anchors.fill: parent

                    Label { text: _document.pluginName }

                    Item
                    {
                        id: pluginContainerToolStrip
                        enabled: !root.pluginMinimised && !_document.busy
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    RowLayout
                    {
                        enabled: !pluginToolBarContainer.transitioning

                        ToolBarButton { action: togglePluginMinimiseAction }
                        ToolBarButton { visible: Qt.platform.os !== "wasm"; action: togglePluginWindowAction }
                    }
                }
            }

            Item
            {
                id: pluginContainer
                objectName: "pluginContainer"

                anchors.top: toolBar.bottom
                // anchors.bottom is not bound, instead the height is set manually
                // in the parent's onHeightChanged; see the hack explanation above

                anchors.left: parent.left
                anchors.right: parent.right
            }
        }

        onResizingChanged:
        {
            if(!resizing && typeof plugin.content.onResized === "function")
                plugin.content.onResized();
        }
    }

    Preferences
    {
        id: window
        section: "window"
        property var pluginX
        property var pluginY
        property var pluginWidth
        property var pluginHeight
        property var pluginMaximised
        property int pluginSplitSize
        property bool pluginPoppedOut: false
    }

    Preferences
    {
        id: misc
        section: "misc"

        property var fileOpenInitialFolder
        property var fileSaveInitialFolder
        property string webSearchEngineUrl
        property bool hasSeenTutorial

        property string templates

        function templateFor(name)
        {
            let a = [];

            if(templates.length > 0)
            {
                try { a = JSON.parse(templates); }
                catch(e) { a = []; }
            }

            return a.find(e => e.name === name);
        }
    }

    // This is only here to get at the default values of its properties
    PluginContent { id: defaultPluginContent }

    Item
    {
        id: plugin
        objectName: _document.pluginName

        enabled: !pluginToolBarContainer.minimisingOrMinimised || root.pluginPoppedOut

        anchors.fill: parent

        property var model: _document.plugin
        property var content
        property bool loaded: false

        onLoadedChanged:
        {
            if(root.pluginPoppedOut)
                popOutPlugin();
            else
                popInPlugin();

            plugin.resetSaveRequired();
        }

        property bool saveRequired:
        {
            if(loaded && content !== undefined)
            {
                if(typeof(content.saveRequired) === "boolean")
                    return content.saveRequired;
                else if(typeof(content.saveRequired) === "function")
                    return content.saveRequired();
            }

            return false;
        }

        function resetSaveRequired()
        {
            if(typeof(content.saveRequired) === "boolean")
                content.saveRequired = false;
        }

        function initialise()
        {
            if(typeof(content.initialise) === "function")
                content.initialise();
        }

        function save()
        {
            plugin.resetSaveRequired();

            if(typeof(content.save) === "function")
                return content.save();

            return {};
        }

        function load(data, version)
        {
            // It might be JSON, or it might be a plain string; try both
            try
            {
                data = JSON.parse(data);
            }
            catch(e)
            {
                data = data.toString();
            }

            if(typeof(content.load) === "function")
                content.load(data, version);
        }

        // At least one enabled direct child
        property bool enabledChildren:
        {
            for(let i = 0; i < children.length; i++)
            {
                if(children[i].enabled)
                    return true;
            }

            return false;
        }
    }

    readonly property int pluginX: pluginWindow.x
    readonly property int pluginY: pluginWindow.y

    function loadPluginWindowState()
    {
        if(Qt.platform.os === "wasm")
            return;

        if(window.pluginPoppedOut === undefined)
            return;

        root.pluginPoppedOut = window.pluginPoppedOut;

        if(!window.pluginPoppedOut)
            return;

        if(window.pluginWidth !== undefined &&
           window.pluginHeight !== undefined)
        {
            pluginWindow.x = window.pluginX;
            pluginWindow.y = window.pluginY;
            pluginWindow.width = window.pluginWidth;
            pluginWindow.height = window.pluginHeight;
        }

        if(window.pluginMaximised !== undefined)
        {
            pluginWindow.visibility = Utils.castToBool(window.pluginMaximised) ?
                Window.Maximized : Window.Windowed;
        }
    }

    function savePluginWindowState()
    {
        window.pluginPoppedOut = root.pluginPoppedOut;

        if(!window.pluginPoppedOut)
            return;

        window.pluginMaximised = pluginWindow.maximised;

        if(!pluginWindow.maximised)
        {
            window.pluginX = pluginWindow.x;
            window.pluginY = pluginWindow.y;
            window.pluginWidth = pluginWindow.width;
            window.pluginHeight = pluginWindow.height;
        }
    }

    property bool destructing: false

    ApplicationWindow
    {
        id: pluginWindow
        width: 800
        height: 600
        minimumWidth: 480
        minimumHeight: 480
        title: application && _document.pluginName.length > 0 ?
                   _document.pluginName + " - " + appName : "";
        visible: root.visible && root.pluginPoppedOut && plugin.loaded
        property bool maximised: visibility === Window.Maximized

        onXChanged: { if(x < 0 || x >= Screen.desktopAvailableWidth)  x = 0; }
        onYChanged: { if(y < 0 || y >= Screen.desktopAvailableHeight) y = 0; }

        onClosing: function(close)
        {
            if(visible & !destructing)
                popInPlugin();
        }

        PlatformMenuBar
        {
            PlatformMenu { id: pluginMenu0; hidden: true; enabled: !_document.busy }
            PlatformMenu { id: pluginMenu1; hidden: true; enabled: !_document.busy }
            PlatformMenu { id: pluginMenu2; hidden: true; enabled: !_document.busy }
            PlatformMenu { id: pluginMenu3; hidden: true; enabled: !_document.busy }
            PlatformMenu { id: pluginMenu4; hidden: true; enabled: !_document.busy }
        }

        header: ToolBar
        {
            id: pluginWindowToolStrip
            topPadding: Constants.padding
            bottomPadding: Constants.padding
            visible: plugin.content !== undefined && plugin.content.toolStrip
            enabled: !_document.busy
        }

        Item
        {
            id: pluginWindowContent
            objectName: "pluginWindow"
            anchors.fill: parent
        }

        onWidthChanged: { windowResizedTimer.start(); }
        onHeightChanged: { windowResizedTimer.start(); }

        Timer
        {
            id: windowResizedTimer
            interval: 500

            onTriggered:
            {
                if(plugin.content && typeof plugin.content.onResized === "function")
                    plugin.content.onResized();
            }
        }

        Component.onCompleted:
        {
            if(x === 0 && y === 0)
            {
                x = (pluginWindow.screen.width - width) * 0.5
                y = (pluginWindow.screen.height - height) * 0.5
            }
        }
    }

    Component.onCompleted:
    {
        loadPluginWindowState();
    }

    Component.onDestruction:
    {
        savePluginWindowState();

        // Mild hack to get the plugin window to close before the main window
        destructing = true;
        pluginWindow.close();
    }

    function toggleMinimise()
    {
        // Ensure the animation starting point of the SplitView is
        // where it should be as if the user attempts to drag the
        // handle past its limits, it can be peturbed
        if(!root.pluginMinimised)
        {
            pluginToolBarContainer.SplitView.preferredHeight = Math.max(
                pluginToolBarContainer.SplitView.preferredHeight,
                pluginToolBarContainer.SplitView.minimumHeight);
        }
        else
            pluginToolBarContainer.SplitView.preferredHeight = toolBar.height;

        root.pluginMinimised = !root.pluginMinimised;

        if(!root.pluginMinimised)
            pluginToolBarContainer.SplitView.preferredHeight = pluginToolBarContainer._lastUnminimisedHeight;
        else
            pluginToolBarContainer.SplitView.preferredHeight = toolBar.height;
    }

    function popOutPlugin()
    {
        // This must be set before the window is made visible, otherwise
        // it is created as an MDI sub-window and doesn't minimise to the
        // Task Bar (on Windows)
        pluginWindow.transientParent = null;

        root.pluginPoppedOut = true;
        plugin.parent = pluginWindowContent;

        if(plugin.content.toolStrip)
            plugin.content.toolStrip.parent = pluginWindowToolStrip.contentItem;

        pluginWindow.x = pluginX;
        pluginWindow.y = pluginY;
    }

    function popInPlugin()
    {
        plugin.parent = pluginContainer;

        if(plugin.content.toolStrip)
            plugin.content.toolStrip.parent = pluginContainerToolStrip;

        pluginToolBarContainer.SplitView.preferredHeight = window.pluginSplitSize;

        root.pluginPoppedOut = false;
    }

    function togglePop()
    {
        if(root.pluginPoppedOut)
            root.popInPlugin();
        else
            root.popOutPlugin();
    }

    function createPluginMenu(index, menu)
    {
        if(!plugin.loaded)
            return false;

        // Check the plugin implements createMenu
        if(typeof plugin.content.createMenu === "function")
            return plugin.content.createMenu(index, menu);

        return false;
    }

    readonly property int findType: find.type
    function showFind(findType)
    {
        find.show(findType);
    }

    function hideFind()
    {
        find.hide();
    }

    function showLayoutSettings()
    {
        layoutSettings.toggle();
    }

    MessageDialog
    {
        id: errorSavingFileMessageDialog
        title: qsTr("Error Saving File")
        modality: Qt.ApplicationModal
    }

    Component
    {
        id: applyMethodDialogComponent
        Window
        {
            id: applyMethodDialog

            title: qsTr("Application Method")

            modality: Qt.ApplicationModal
            flags: Constants.defaultWindowFlags

            width: 380
            minimumWidth: width
            maximumWidth: width

            height: 120
            minimumHeight: height
            maximumHeight: height

            property string templateName: ""

            ColumnLayout
            {
                spacing: Constants.spacing
                anchors.fill: parent
                anchors.margins: Constants.margin

                Text
                {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    wrapMode: Text.Wrap
                    text: qsTr("Do you want to apply the template by appending to your existing " +
                        "configuration, or by replacing it?")
                }

                RowLayout
                {
                    Layout.alignment: Qt.AlignHCenter

                    Button
                    {
                        text: qsTr("Append")
                        onClicked:
                        {
                            applyMethodDialog.close();
                            root.applyTemplate(applyMethodDialog.templateName, AddTemplateDialog.Append);
                        }
                    }

                    Button
                    {
                        text: qsTr("Replace")
                        onClicked:
                        {
                            applyMethodDialog.close();
                            root.applyTemplate(applyMethodDialog.templateName, AddTemplateDialog.Replace);
                        }
                    }
                }
            }
        }
    }

    function applyTemplate(name, method)
    {
        let template = misc.templateFor(name);

        if(method !== undefined)
            template.method = method;

        switch(template.method)
        {
        case AddTemplateDialog.AlwaysAsk:
            Utils.createWindow(root, applyMethodDialogComponent, {templateName: name});
            break;

        case AddTemplateDialog.Append:
            _document.update(template.transforms, template.visualisations);
            break;

        case AddTemplateDialog.Replace:
            _document.apply(template.transforms, template.visualisations);
            break;
        }
    }

    Document
    {
        id: _document
        application: root.application
        graphDisplay: graph

        property bool nodeSelectionEmpty: numNodesSelected === 0
        property bool canDeleteSelection: editable && !nodeSelectionEmpty
        property bool significantCommandInProgress: commandInProgress && !commandTimer.running
        property bool canChangeComponent: !busy && graph.numComponents > 1
        property bool hasPluginUI: pluginQmlModule.length > 0

        function shading()
        {
            return projection() === Projection.TwoDee ? shading2D() : shading3D();
        }

        function setShading(_shading)
        {
            if(projection() === Projection.TwoDee)
                setShading2D(_shading);
            else
                setShading3D(_shading);
        }

        onUiDataChanged: function(uiData)
        {
            uiData = JSON.parse(uiData);

            if(uiData.lastAdvancedFindAttributeName !== undefined)
                find.lastAdvancedFindAttributeName = uiData.lastAdvancedFindAttributeName;

            if(uiData.lastFindByAttributeName !== undefined)
                find.lastFindByAttributeName = uiData.lastFindByAttributeName;
        }

        onPluginQmlModuleChanged: function(pluginUiData, pluginUiDataVersion)
        {
            if(_document.pluginQmlModule.length > 0)
            {
                // Destroy anything already there
                while(plugin.children.length > 0)
                    plugin.children[0].destroy();

                let pluginComponent = Qt.createComponent(_document.pluginQmlModule, "Main");
                if(pluginComponent.status !== Component.Ready)
                {
                    console.log(pluginComponent.errorString());
                    return;
                }

                plugin.content = pluginComponent.createObject(plugin);
                plugin.content._mainWindow = mainWindow;
                plugin.content.baseFileName = root.baseFileName;
                plugin.content.baseFileNameNoExtension = root.baseFileNameNoExtension;

                if(plugin.content === null)
                {
                    console.log(_document.pluginQmlModule + ": failed to create instance");
                    return;
                }

                if(plugin.content.toolStrip)
                {
                    // If the plugin toolstrip is itself a ToolBar, we are actually interested
                    // in its contentItem, otherwise we would be adding a ToolBar to a ToolBar
                    if(plugin.content.toolStrip instanceof ToolBar)
                        plugin.content.toolStrip = plugin.content.toolStrip.contentChildren[0];
                }

                plugin.initialise();

                // Restore saved data, if there is any
                if(pluginUiDataVersion >= 0)
                    plugin.load(pluginUiData, pluginUiDataVersion);

                plugin.loaded = true;
                pluginLoadComplete();
            }
        }

        onSaveComplete: function(success, fileUrl, saverName)
        {
            if(!success)
            {
                errorSavingFileMessageDialog.text = Utils.format(qsTr("{0} could not be saved."),
                    NativeUtils.baseFileNameForUrl(root.url));
                errorSavingFileMessageDialog.open();
            }
            else if(Qt.platform.os !== "wasm")
            {
                savedFileUrl = fileUrl;
                savedFileSaver = saverName;
                mainWindow.addToRecentFiles(fileUrl);
            }
        }

        onAttributesChanged: function(addedNames, removedNames, changedValuesNames, graphChanged)
        {
            // If a new attribute has been created and advanced find or FBAV have
            // not yet been used, set the new attribute as the default for both
            while(addedNames.length > 0)
            {
                let lastAddedAttributeName = addedNames[addedNames.length - 1];
                let lastAddedAttribute = attribute(lastAddedAttributeName);

                if(lastAddedAttribute.valueType === ValueType.String)
                {
                    if(find.lastAdvancedFindAttributeName.length === 0)
                        find.lastAdvancedFindAttributeName = lastAddedAttributeName;

                    if(lastAddedAttribute.sharedValues.length > 0 &&
                        find.lastFindByAttributeName.length === 0)
                    {
                        find.lastFindByAttributeName = lastAddedAttributeName;
                    }

                    break;
                }

                addedNames.pop();
            }

            _refreshAttributesWithSharedValues();
        }

        onLoadComplete: function(url, success)
        {
            if(success)
                _refreshAttributesWithSharedValues();

            root.loadComplete(url, success);
        }

        onGraphChanged: function(graph, changeOccurred) { root._refreshAttributesWithSharedValues(); }

        property var _comandProgressSamples: []
        property int commandSecondsRemaining

        onCommandProgressChanged:
        {
            // Reset the sample buffer if the command progress is less than the latest sample (i.e. new command)
            if(_comandProgressSamples.length > 0 && commandProgress <
                _comandProgressSamples[_comandProgressSamples.length - 1].progress)
            {
                _comandProgressSamples.length = 0;
            }

            if(commandProgress < 0)
            {
                commandSecondsRemaining = 0;
                return;
            }

            let sample = {progress: commandProgress, seconds: new Date().getTime() / 1000.0};
            _comandProgressSamples.push(sample);

            // Only keep this many samples
            while(_comandProgressSamples.length > 10)
                _comandProgressSamples.shift();

            // Require a few samples before making the calculation
            if(_comandProgressSamples.length < 5)
            {
                commandSecondsRemaining = 0;
                return;
            }

            let earliestSample = _comandProgressSamples[0];
            let latestSample = _comandProgressSamples[_comandProgressSamples.length - 1];
            let percentDelta = latestSample.progress - earliestSample.progress;
            let timeDelta = latestSample.seconds - earliestSample.seconds;
            let percentRemaining = 100.0 - commandProgress;

            commandSecondsRemaining = percentRemaining * timeDelta / percentDelta;
        }

        onCommandInProgressChanged:
        {
            if(commandInProgress)
                commandTimer.start();
            else
            {
                commandTimer.stop();
                root.commandComplete();
            }
        }
    }

    signal loadComplete(url url, bool success)
    signal pluginLoadComplete()

    signal commandStarted();
    signal commandComplete();

    Timer
    {
        id: commandTimer
        interval: 200

        onTriggered:
        {
            stop();
            root.commandStarted();
        }
    }

    function startTutorial()
    {
        tutorial.start();
    }

    Tutorial
    {
        id: tutorial

        Hubble
        {
            title: qsTr("Introduction")
            x: 10
            y: 10
            Text
            {
                textFormat: Text.StyledText
                text:
                {
                    let s = "";

                    if(!misc.hasSeenTutorial)
                    {
                        s += Utils.format(qsTr("As this is your first time starting {0}, " +
                            "we have opened an example graph.<br>"), appName);
                    }

                    s += qsTr("This example graph represents the <b>London Tube System and River Buses!</b>");

                    if(!misc.hasSeenTutorial)
                    {
                        s += qsTr("<br><br>If you choose to skip the tutorial at this point, it can " +
                            "be restarted later from the <i>Help</i> menu.");
                    }

                    s += qsTr("<br><br>Click <i>Next</i> to start the tutorial.");

                    return s;
                }
            }
        }

        Hubble
        {
            title: qsTr("Nodes and Edges")
            x: (root.width * 0.5) - childrenRect.width * 0.5
            y: 10
            RowLayout
            {
                spacing: Constants.spacing
                Column
                {
                    Image
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        fillMode: Image.PreserveAspectFit
                        width: 75
                        height: 75
                        source: "qrc:///imagery/node.png"
                    }
                    Text
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("A Node")
                    }
                    Image
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        fillMode: Image.PreserveAspectFit
                        width: 100
                        height: 100
                        source: "qrc:///imagery/edge.png"
                    }
                    Text
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("An Edge")
                    }
                }
                Text
                {
                    Layout.preferredWidth: 500
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text:
                    {
                        let preamble = qsTr("A graph consists of <b>Nodes</b> and <b>Edges</b>. " +
                          "In this example, the nodes represent stops, and the edges represent a connection between two stops. " +
                          "<b>Edges</b> always have a direction, however direction can be ignored if desired. " +
                          "<b>Nodes</b> and <b>Edges</b> can also have additional information associated with them. " +
                          "We refer to these as <b>Attributes</b>.<br><br>");

                        let regularMouse = qsTr("<b>Rotate:</b> Left Click and Drag<br>" +
                          "<b>Zoom:</b> Mouse Scrollwheel<br>" +
                          "<b>Pan:</b> Right Click and Drag<br>");

                        let macOsTrackpad = qsTr("<b>Rotate:</b> Two Finger Trackpad Drag <b>or</b> Left Mouse Click and Drag<br>" +
                          "<b>Zoom:</b> Trackpad Pinch <b>or</b> Mouse Scrollwheel<br>" +
                          "<b>Pan:</b> Two Finger Trackpad Click and Drag <b>or</b> Right Mouse Click Drag<br>");

                        let postamble = qsTr("<b>Focus Node:</b> Double Click");

                        return preamble + (Qt.platform.os === "osx" ? macOsTrackpad : regularMouse) + postamble;
                    }
                }
            }
        }

        Hubble
        {
            title: qsTr("Overview Mode")
            x: (root.width * 0.5) - childrenRect.width * 0.5;
            y: 10
            RowLayout
            {
                Image
                {
                    Layout.alignment: Qt.AlignVCenter
                    source: "qrc:///imagery/overview.png"
                    mipmap: true
                    fillMode: Image.PreserveAspectFit
                }

                Text
                {
                    Layout.preferredWidth: 400
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: Utils.format(qsTr("When a graph contains multiple disconnected graphs (<b>Components</b>) {0} " +
                          "opens the file in Overview mode. From Overview mode all components are visible. In this graph the " +
                          "left component is the <b>London Tube map</b>, while the right component is the <b>London Riverbus " +
                          "Network</b>.<br><br>" +
                          "To focus on a particular component and hide others, <b>Double Click</b> it. " +
                          "When viewing a component, on-screen buttons appear to help you navigate. Alternatively, <b>Double Click</b> " +
                          "the background or press <b>Esc</b> to return to Overview mode.<br><br>" +
                          "<b>Component Mode:</b> Double Click Component<br>" +
                          "<b>Overview Mode:</b> Double Click Background or Press Esc<br>" +
                          "<b>Scroll Through Components:</b> PageUp, PageDown<br>" +
                          "<b>Select Node:</b> Click a Node<br>" +
                          "<b>Select Additional Nodes:</b> Hold Shift and Click a Node<br>" +
                          "<b>Select Multiple Nodes:</b> Hold Shift, Click and Drag a Box Around Nodes"), appName)
                }
            }
        }

        Hubble
        {
            id: pluginHubble
            title: qsTr("Node Attributes")
            x: 10
            y: graph.height - height - 10
            RowLayout
            {
                Text
                {
                    Layout.preferredWidth: 500
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("Information associated with the <b>selected</b> node(s) will be displayed in the attribute table. " +
                          "Attributes are sourced from the input file or calculated through the use of transforms.<br><br>" +
                          "Select a node using <b>Left Click</b> and view the node's attributes in the table below.")
                }
                Image
                {
                    source: "qrc:///imagery/attributes.png"
                }
            }
        }

        Hubble
        {
            title: qsTr("Transforms")
            target: transforms
            alignment: Qt.AlignRight | Qt.AlignBottom
            edges: Qt.RightEdge | Qt.TopEdge
            RowLayout
            {
                spacing: Constants.spacing
                Column
                {
                    Image
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:///imagery/mcl.svg"
                        mipmap: true
                        fillMode: Image.PreserveAspectFit
                        width: 150
                    }
                    Text
                    {
                        text: qsTr("An clustering transform with a colour<br>visualisation applied")
                    }
                }
                Text
                {
                    Layout.preferredWidth: 500
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("Transforms are a powerful way to modify the graph and calculate additional attributes. " +
                          "They can be used to remove nodes or edges, calculate metrics and much more. " +
                          "Transforms will appear above. They are applied in order, from top to bottom.<br><br>" +
                          "Click <b>Add Transform</b> to create a new one.<br>" +
                          "Try selecting <b>MCL Cluster</b>.<br>" +
                          "Select <b>Colour</b> for the visualisation and then click <b>OK</b>.")
                }
            }
        }

        Hubble
        {
            title: qsTr("Visualisations")
            target: visualisations
            alignment: Qt.AlignRight | Qt.AlignTop
            edges: Qt.RightEdge | Qt.BottomEdge
            RowLayout
            {
                spacing: Constants.spacing
                Column
                {
                    Image
                    {
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: "qrc:///imagery/visualisations.svg"
                        mipmap: true
                        fillMode: Image.PreserveAspectFit
                        width: 200
                    }
                    Text
                    {
                        text: qsTr("A graph with Colour, Size and Text <br>visualisations applied")
                    }
                }
                Text
                {
                    Layout.preferredWidth: 500
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("Visualisations allow for displaying attribute values by modifying the appearance of the graph elements. " +
                          "Node or edge <b>Colour</b>, <b>Size</b> and <b>Text</b> can all be linked to an attribute. " +
                          "Visualisations can be created here or existing ones modified.")
                }
            }
        }

        Hubble
        {
            title: qsTr("Search Graph")
            x: 10
            y: !findPanel.hidden ? find.y + find.height + 10 : 10
            RowLayout
            {
                Text
                {
                    Layout.preferredWidth: 400
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("To perform a simple search within the graph, click <b>Find</b> on the toolbar above.<br>" +
                          "Try finding <b>Paddington</b>.")
                }
                ToolBarButton
                {
                    icon.name: findAction.icon.name
                }
            }
        }

        Hubble
        {
            title: qsTr("Find By Attribute Value")
            x: 10
            y: !findPanel.hidden ? find.y + find.height + 10 : 10
            RowLayout
            {
                Text
                {
                    Layout.preferredWidth: 400
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: qsTr("If you clustered the graph using MCL, click on " +
                        "<b>Find By Attribute Value</b> to examine individual clusters.<br>")
                }
                ToolBarButton
                {
                    icon.name: findByAttributeAction.icon.name
                }
            }
        }

        Hubble
        {
            title: qsTr("Conclusion")
            x: (root.width * 0.5) - childrenRect.width * 0.5;
            y: 10
            RowLayout
            {
                spacing: Constants.spacing
                Text
                {
                    Layout.preferredWidth: 600
                    wrapMode: Text.WordWrap
                    textFormat: Text.StyledText
                    text: Utils.format(qsTr("This concludes our brief introduction of {0} using the London transport network!<br>" +
                          "{0} can support <b>millions</b> of nodes and edges, this network is just the beginning.<br><br>" +
                          "Utilising Transforms and Visualisations is key to getting the most from your graph.<br><br>" +
                          "If you want to explore some more, please have a look at our {1}."), appName,
                          NativeUtils.redirectLink("example_datasets", qsTr("example datasets")))

                    PointingCursorOnHoverLink {}
                    onLinkActivated: function(link) { Qt.openUrlExternally(link); }
                }
            }
        }
    }
}
