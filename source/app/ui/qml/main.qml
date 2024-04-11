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

import QtQml
import QtQml.Models
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs

import Qt.labs.platform as Labs

import app.graphia
import app.graphia.Controls
import app.graphia.Shared
import app.graphia.Shared.Controls

import "Loading"
import "Options"
import "Enrichment"

ApplicationWindow
{
    id: mainWindow
    visible: false

    width: 1024
    height: 768
    minimumWidth: mainToolBar.visible ? mainToolBar.implicitWidth : 640
    minimumHeight: 480

    readonly property bool maximised: mainWindow.visibility === Window.Maximized

    property TabUI currentTab: tabBar.count > 0 && tabBar.currentIndex < tabLayout.count ?
        tabLayout.get(tabBar.currentIndex) : null

    property bool directed: currentTab ? currentTab.document.directed : false

    property var recentFiles: []

    property bool _anyTabsBusy:
    {
        for(let index = 0; index < tabBar.count; index++)
        {
            let tab = tabLayout.get(index);
            if(tab !== null && tab.document.busy)
                return true;
        }

        return false;
    }

    title:
    {
        let text = "";
        if(currentTab !== null && currentTab.title.length > 0)
            text += currentTab.title + qsTr(" - ");

        text += application.name;

        return text;
    }

    Application { id: application }

    Labs.MessageDialog
    {
        id: noUpdatesMessageDialog
        title: qsTr("No Updates")
        text: qsTr("There are no updates available at this time.")
        modality: Qt.ApplicationModal
    }

    // Use Connections to avoid an M16 JS lint error
    Connections
    {
        target: application

        function onNoNewUpdateAvailable(existing)
        {
            if(checkForUpdatesAction.active)
            {
                if(existing)
                {
                    // While there is no /new/ update, there is an existing update
                    // available that the user has previously dismissed
                    newUpdate.visible = true;
                }
                else
                    noUpdatesMessageDialog.open();
            }

            checkForUpdatesAction.active = false;
        }

        function onNewUpdateAvailable()
        {
            checkForUpdatesAction.active = false;
            newUpdate.visible = true;
        }

        function onChangeLogStored()
        {
            changeLog.refresh();
        }

        function onDownloadComplete(url, filename)
        {
            let fileUrl = QmlUtils.urlForFileName(filename);

            if(mainWindow.activeFocusItem !== null)
            {
                processArguments([fileUrl]);
                application.resumeDownload();
            }
            else
                downloadUI.blockedUrl = fileUrl;
        }

        function onDownloadError(url, error)
        {
            errorOpeningFileMessageDialog.text = Utils.format(
                qsTr("{0} could not be opened:\n\n{1}"), userTextForUrl(url), error);
            errorOpeningFileMessageDialog.open();
        }
    }

    Tracking
    {
        id: tracking

        anchors.fill: parent

        visible: !validValues

        onTrackingDataEntered: { submit(); }

        function submit()
        {
            if(!validValues)
                return;

            application.submitTrackingData();

            // Deal with any arguments now that
            // we've collected a tracking identity
            processOnePendingArgument();
        }
    }

    function currentState()
    {
        let s = tabBar.count + " tabs";

        for(let index = 0; index < tabBar.count; index++)
        {
            if(s.length !== 0)
                s += "\n\n";

            s += "Tab " + index + ": ";
            let tab = tabLayout.get(index);
            if(tab === null)
            {
                s += "null";
                continue;
            }

            s += tab.title;
            s += "\n\n" + tab.document.graphSizeSummary();
            s += "\n\nParse Log:\n" + tab.document.log;

            let stack = tab.document.commandStackSummary();
            if(stack.length > 0)
                s += "\n\nCommand Stack:\n" + stack;

            let listToString = function(list, title)
            {
                if(list.length > 0)
                {
                    s += "\n\n" + title;
                    for(let row = 0; row < list.length; row++)
                        s += "\n  " + list[row];
                }
            };

            listToString(tab.document.transforms, "Transforms:");
            listToString(tab.document.visualisations, "Visualisations:");
        }

        s += "\n\nEnvironment:\n" + mainWindow.environment;

        return s;
    }

    property var _processedArguments: []
    property var _pendingArguments: []

    // This is called with the arguments of a second instance of the app,
    // when it starts then immediately exits
    function processArguments(arguments)
    {
        _pendingArguments.push(...arguments);
        processOnePendingArgument();
    }

    function processOnePendingArgument()
    {
        if(_pendingArguments.length === 0)
            return;

        let argument = "";
        do
        {
            // Pop
            argument = _pendingArguments[0];
            _pendingArguments.shift();
            _processedArguments.push(argument);
        }
        while(argument[0] === "-" && _pendingArguments.length > 0);

        // Ignore option style arguments
        if(argument.length === 0 || argument[0] === "-")
            return;

        let url = QmlUtils.urlForUserInput(argument);
        Qt.callLater(function()
        {
            // Queue this up to do later in the event loop, in case there
            // is any other plugin initialisation to do be done beforehand
            openUrl(url, true);
        });
    }

    Component.onCompleted:
    {
        if(misc.recentFiles.length > 0)
            mainWindow.recentFiles = JSON.parse(misc.recentFiles);
        else
            mainWindow.recentFiles = [];

        if(windowPreferences.width !== undefined &&
           windowPreferences.height !== undefined &&
           windowPreferences.x !== undefined &&
           windowPreferences.y !== undefined)
        {
            mainWindow.width = windowPreferences.width;
            mainWindow.height = windowPreferences.height;
            mainWindow.x = windowPreferences.x;
            mainWindow.y = windowPreferences.y;

            // Make sure that the window doesn't appear off screen
            // This is basically a workaround for QTBUG-58419
            let rightEdge = mainWindow.x + mainWindow.width;
            let bottomEdge = mainWindow.y + mainWindow.height;

            if(mainWindow.x < 0)
                mainWindow.x = 0;
            else if(rightEdge > Screen.desktopAvailableWidth)
                mainWindow.x -= (rightEdge - Screen.desktopAvailableWidth);

            if(mainWindow.y < 0)
                mainWindow.y = 0;
            else if(bottomEdge > Screen.desktopAvailableHeight)
                mainWindow.y -= (bottomEdge - Screen.desktopAvailableHeight);
        }

        if(application.runningWasm)
            mainWindow.showFullScreen();
        else if(windowPreferences.maximised !== undefined && Utils.castToBool(windowPreferences.maximised))
            mainWindow.showMaximized();
        else
            mainWindow.showNormal();

        // Arguments minus the executable
        _pendingArguments = Qt.application.arguments.slice(1);

        if(!misc.hasSeenTutorial)
        {
            let exampleFile = application.resourceFile("examples/Tutorial.graphia");

            if(QmlUtils.fileExists(exampleFile))
            {
                // Add it to the pending arguments, because it's highly
                // likely we're currently showing the tracking UI
                _pendingArguments.push(exampleFile);
            }
        }

        tracking.submit();
    }

    property bool _restartOnExit: false

    function restart()
    {
        _restartOnExit = true;
        mainWindow.close();
    }

    onClosing: function(close)
    {
        if(tabBar.count > 0)
        {
            // Capture _restartOnExit so that we can restore its value after a non-cancel exit
            let closeTabFunction = function(restartOnExit)
            {
                return function()
                {
                    tabBar.removeTab(0);
                    _restartOnExit = restartOnExit;

                    // callLater is used because (apparently) ApplicationWindow::close()
                    // doesn't like to be invoked recursively
                    Qt.callLater(mainWindow.close);
                };
            }(_restartOnExit);

            // Reset the value of _restartOnExit so that if the user cancels an exit, any
            // subsequent future exit doesn't then also restart
            _restartOnExit = false;

            // If any tabs are open, close the first one and cancel the window close, followed
            // by (recursive) calls to closeTabFunction, assuming the user doesn't cancel
            tabBar.closeTab(0, closeTabFunction);

            close.accepted = false;
            return;
        }

        // Prevent any further access to enrichmentResults during destruction
        enrichmentResults.models = [];

        windowPreferences.maximised = mainWindow.maximised;

        if(!mainWindow.maximised)
        {
            windowPreferences.width = mainWindow.width;
            windowPreferences.height = mainWindow.height;
            windowPreferences.x = mainWindow.x;
            windowPreferences.y = mainWindow.y;
        }

        Qt.exit(!_restartOnExit ?
            ExitType.NormalExit :
            ExitType.Restart);
    }

    function availableAttributesModel()
    {
        if(!currentTab)
            return null;

        return currentTab.document.availableAttributesModel();
    }

    function find(term, options, attributeNames, findSelectStyle)
    {
        if(!currentTab)
            return null;

        currentTab.hideFind();
        return currentTab.document.find(term, options, attributeNames, findSelectStyle);
    }

    function selectByAttributeValue(attributeName, value)
    {
        if(!currentTab)
            return null;

        currentTab.hideFind();
        return currentTab.document.selectByAttributeValue(attributeName, value);
    }

    Labs.MessageDialog
    {
        id: errorOpeningFileMessageDialog
        title: qsTr("Error Opening File")
        modality: Qt.ApplicationModal

        onAccepted:
        {
            // Even if a file failed to load, there may be more to process
            processOnePendingArgument();
        }
    }

    OptionsDialog
    {
        id: optionsDialog
        application: application

        enabled: !mainWindow._anyTabsBusy
    }

    AboutDialog
    {
        id: aboutDialog
        application: application

        onHiddenSwitchActivated:
        {
            console.log("Debug menu enabled");
            mainWindow.debugMenuUnhidden = true;
        }
    }

    AboutPluginsDialog { id: aboutpluginsDialog }

    ChangeLog { id: changeLog }
    LatestChangesDialog
    {
        id: latestChangesDialog

        text: changeLog.text
        version: application.version
    }

    readonly property string environment:
    {
        let s = "";
        let environment = application.environment;

        for(let i = 0; i < environment.length; i++)
        {
            if(s.length !== 0)
                s += "\n";

            s += environment[i];
        }

        return s;
    }

    TextDialog
    {
        id: environmentDialog
        text: mainWindow.environment
    }

    TextDialog
    {
        id: openGLInfoDialog
        text: application.openGLInfo
    }

    ShowPaletteDialog { id: showPaletteDialog }

    TextDialog
    {
        id: provenanceLogDialog
        title: qsTr("Provenance Log")
        text: currentTab !== null ? currentTab.document.log : ""
    }

    Preferences
    {
        id: windowPreferences
        section: "window"
        property var width
        property var height
        property var maximised
        property var x
        property var y
    }

    Preferences
    {
        id: misc
        section: "misc"
        property alias showGraphMetrics: toggleGraphMetricsAction.checked

        property var fileOpenInitialFolder
        property string recentFiles
        property bool hasSeenTutorial
        property string update
    }

    Templates { id: templates }

    Preferences
    {
        id: visuals
        section: "visuals"
        property int edgeVisualType:
        {
            return toggleEdgeDirectionAction.checked ?
                EdgeVisualType.Arrow : EdgeVisualType.Cylinder;
        }
        property int showNodeText:
        {
            switch(nodeTextDisplay.checkedAction)
            {
            default:
            case hideNodeTextAction:         return TextState.Off;
            case showFocusedNodeTextAction:  return TextState.Focused;
            case showSelectedNodeTextAction: return TextState.Selected;
            case showAllNodeTextAction:      return TextState.All;
            }
        }
        property int showEdgeText:
        {
            switch(edgeTextDisplay.checkedAction)
            {
            default:
            case hideEdgeTextAction:         return TextState.Off;
            case showSelectedEdgeTextAction: return TextState.Selected;
            case showAllEdgeTextAction:      return TextState.All;
            }
        }
        property alias showMultiElementIndicators: showMultiElementIndicatorsAction.checked
        property alias showEdges: showEdgesAction.checked

        property double defaultNormalNodeSize
        property double defaultNormalEdgeSize

        // If the default sizes are changed, reflect the sizes in the current open document,
        // so the user has some idea about what size they're choosing
        onDefaultNormalNodeSizeChanged:
        {
            if(currentTab)
                currentTab.document.nodeSize = defaultNormalNodeSize;
        }

        onDefaultNormalEdgeSizeChanged:
        {
            if(currentTab)
                currentTab.document.edgeSize = defaultNormalEdgeSize;
        }
    }

    Preferences
    {
        id: defaults
        section: "defaults"

        function _stringToObject(s)
        {
            let o = {};

            if(s.length > 0)
            {
                try { o = JSON.parse(s); }
                catch(e) { o = {}; }
            }

            return o;
        }

        property string extensions
        function urlTypeFor(extension) { return _stringToObject(defaults.extensions)[extension]; }

        function setUrlTypeFor(extension, urlType)
        {
            let o = _stringToObject(defaults.extensions);
            o[extension] = urlType;
            extensions = JSON.stringify(o);
        }

        property string plugins
        function pluginFor(urlType) { return _stringToObject(defaults.plugins)[urlType]; }

        function setPluginFor(urlType, pluginName)
        {
            let o = _stringToObject(defaults.plugins);
            o[urlType] = pluginName;
            plugins = JSON.stringify(o);
        }
    }

    Preferences
    {
        section: "debug"
        property alias showFpsMeter: toggleFpsMeterAction.checked
        property alias saveGlyphMaps: toggleGlyphmapSaveAction.checked
    }

    function addToRecentFiles(fileUrl)
    {
        if(application.runningWasm)
        {
            console.log("WARNING: Can't call addToRecentFiles under WASM");
            return;
        }

        let fileUrlString = fileUrl.toString();

        if(mainWindow.recentFiles === undefined)
            mainWindow.recentFiles = [];

        let localRecentFiles = mainWindow.recentFiles;

        // Remove any duplicates
        for(let i = 0; i < localRecentFiles.length; i++)
        {
            if(localRecentFiles[i] === fileUrlString)
            {
                localRecentFiles.splice(i, 1);
                break;
            }
        }

        // Add to the top
        localRecentFiles.unshift(fileUrlString);

        let MAX_RECENT_FILES = 10;
        while(localRecentFiles.length > MAX_RECENT_FILES)
            localRecentFiles.pop();

        mainWindow.recentFiles = localRecentFiles;
        misc.recentFiles = JSON.stringify(localRecentFiles);
    }

    function userTextForUrl(url)
    {
        return QmlUtils.urlIsFile(url) ? QmlUtils.baseFileNameForUrl(url) : url;
    }

    Component
    {
        id: chooserDialogComponent
        ChooserDialog {}
    }

    function openUrl(url, inNewTab)
    {
        // If the URL is empty, avoid doing anything with it
        if(url.toString().trim().length === 0)
            return;

        // The URL is a web link, handle it as such
        if(application.isNativeLink(url))
        {
            if(application.linkVersionFor(url) === 1)
            {
                let arguments = application.linkArgumentsFor(url);
                processArguments(arguments);
            }
            else
            {
                errorOpeningFileMessageDialog.title = qsTr("Unknown URL Scheme");
                errorOpeningFileMessageDialog.text = Utils.format(qsTr("The hyperlink is not of a known version. " +
                    "Please upgrade {0}, or check that the link is well-formed."), application.name);
                errorOpeningFileMessageDialog.open();
            }

            return;
        }

        if(QmlUtils.urlIsFile(url) && !QmlUtils.fileUrlExists(url))
        {
            errorOpeningFileMessageDialog.title = qsTr("File Not Found");
            errorOpeningFileMessageDialog.text =
                Utils.format(qsTr("{0} does not exist."), QmlUtils.baseFileNameForUrl(url));
            errorOpeningFileMessageDialog.open();
            return;
        }

        let types = application.urlTypesOf(url);

        // The application and/or plugins can't load the URL, but it might be downloadable
        if((types.length === 0 || !application.canOpenAnyOf(types)) && QmlUtils.urlIsDownloadable(url))
        {
            application.download(url);
            return;
        }

        if(types.length === 0)
        {
            errorOpeningFileMessageDialog.text = "";

            let failureReasons = application.failureReasons(url);
            if(failureReasons.length === 0)
            {
                errorOpeningFileMessageDialog.title = QmlUtils.urlIsFile(url) ?
                    qsTr("Unknown File Type") : qsTr("Unknown URL Type");
                errorOpeningFileMessageDialog.text = Utils.format(
                    qsTr("{0} cannot be loaded as its type is unknown."), userTextForUrl(url));
            }
            else
            {
                errorOpeningFileMessageDialog.title = qsTr("Failed To Load");
                if(failureReasons.length > 0)
                    errorOpeningFileMessageDialog.text += failureReasons[0];
            }

            errorOpeningFileMessageDialog.open();
            return;
        }

        if(!application.canOpenAnyOf(types))
        {
            errorOpeningFileMessageDialog.title = QmlUtils.urlIsFile(url) ?
                qsTr("Can't Open File") : qsTr("Can't Open URL");
            errorOpeningFileMessageDialog.text = Utils.format(
                qsTr("{0} cannot be loaded."), userTextForUrl(url)); //FIXME more elaborate error message

            errorOpeningFileMessageDialog.open();
            return;
        }

        if(types.length > 1)
        {
            let extension = QmlUtils.extensionForUrl(url);
            let defaultType = defaults.urlTypeFor(extension);
            if(defaultType === undefined || types.indexOf(defaultType) === -1)
            {
                let model = application.urlTypeDetailsModel();
                model.filter = types;

                let urlTypeChooserDialog = chooserDialogComponent.createObject(mainWindow,
                {
                    "title": qsTr("Type Ambiguous"),
                    "explanationText": Utils.format(
                        qsTr("{0} may be interpreted as two or more possible formats. " +
                        "Please select how you wish to proceed below."), userTextForUrl(url)),
                    "choiceLabelText": qsTr("Open As:"),
                    "model": model,
                    "displayRole": "individualDescription",
                    "valueRole": "name"
                });

                urlTypeChooserDialog.open(function(selectedType, remember)
                {
                    if(remember)
                        defaults.setUrlTypeFor(extension, selectedType);

                    openUrlOfType(url, selectedType, inNewTab);
                });
            }
            else
                openUrlOfType(url, defaultType, inNewTab);
        }
        else
            openUrlOfType(url, types[0], inNewTab);
    }

    function openUrlOfType(url, type, inNewTab)
    {
        let onSaveConfirmed = function()
        {
            let pluginNames = application.pluginNames(type);

            if(pluginNames.length > 1)
            {
                let defaultPlugin = defaults.pluginFor(type);
                if(defaultPlugin === undefined || pluginNames.indexOf(defaultPlugin) === -1)
                {
                    let model = application.pluginDetailsModel();
                    model.filter = pluginNames;

                    let pluginChooserDialog = chooserDialogComponent.createObject(mainWindow,
                    {
                        "title": qsTr("Multiple Plugins Applicable"),
                        "explanationText": Utils.format(
                            qsTr("{0} may be loaded by two or more plugins. " +
                            "Please select how you wish to proceed below."), userTextForUrl(url)),
                        "choiceLabelText": qsTr("Open With Plugin:"),
                        "model": model,
                        "displayRole": "name",
                        "valueRole": "name"
                    });

                    pluginChooserDialog.open(function(selectedPlugin, remember)
                    {
                        if(remember)
                            defaults.setPluginFor(type, selectedPlugin);

                        openUrlOfTypeWithPlugin(url, type, selectedPlugin, inNewTab);
                    });
                }
                else
                    openUrlOfTypeWithPlugin(url, type, defaultPlugin, inNewTab);
            }
            else if(pluginNames.length === 1)
                openUrlOfTypeWithPlugin(url, type, pluginNames[0], inNewTab);
            else
                openUrlOfTypeWithPlugin(url, type, "", inNewTab);
        };

        if(currentTab !== null && !inNewTab)
            currentTab.confirmSave(onSaveConfirmed);
        else
            onSaveConfirmed();
    }

    function openUrlOfTypeWithPlugin(url, type, pluginName, inNewTab)
    {
        let parametersQmlPath = application.parametersQmlPathForPlugin(pluginName, type);

        if(parametersQmlPath.length > 0)
        {
            let component = Qt.createComponent(parametersQmlPath);
            if(component.status !== Component.Ready)
            {
                console.log("Error loading parameters QML: " + component.errorString());
                return;
            }

            let contentObject = component.createObject(mainWindow);
            if(contentObject === null)
            {
                console.log(parametersQmlPath + ": failed to create instance");
                return;
            }

            if(!isValidParameterDialog(contentObject))
            {
                console.log("Failed to load Parameters dialog for " + pluginName);
                console.log("Parameters QML must use BaseParameterDialog as root object");
                return;
            }

            contentObject.url = url
            contentObject.type = type;
            contentObject.pluginName = pluginName;
            contentObject.plugin = application.qmlPluginForName(pluginName);
            contentObject.inNewTab = inNewTab;

            mainWindow.data.push(contentObject);

            contentObject.accepted.connect(function()
            {
                openUrlOfTypeWithPluginAndParameters(contentObject.url,
                    contentObject.type, contentObject.pluginName,
                    contentObject.parameters, contentObject.inNewTab);
            });

            //FIXME: We should be doing this, but it seems to cause crashes
            // after opening multiple (3 or so) files one after another
            /*contentObject.closing.connect(function()
            {
                contentObject.destroy();
            });*/

            contentObject.initialised();
            contentObject.show();
        }
        else
            openUrlOfTypeWithPluginAndParameters(url, type, pluginName, {}, inNewTab);
    }

    function isValidParameterDialog(element)
    {
        if (element['parameters'] === undefined ||
            element['url'] === undefined ||
            element['type'] === undefined ||
            element['pluginName'] === undefined ||
            element['plugin'] === undefined ||
            element['inNewTab'] === undefined ||
            element['show'] === undefined ||
            element['accepted'] === undefined)
        {
            return false;
        }

        return true;
    }

    function openUrlOfTypeWithPluginAndParameters(url, type, pluginName, parameters, inNewTab)
    {
        let openInCurrentTab = function()
        {
            tabBar.openInCurrentTab(url, type, pluginName, parameters);
        };

        if(currentTab !== null && !inNewTab)
            tabBar.replaceTab(openInCurrentTab);
        else
            tabBar.createTab(openInCurrentTab);
    }

    OpenFileDialog
    {
        id: fileOpenDialog
        nameFilters: application.nameFilters
        onAccepted:
        {
            misc.fileOpenInitialFolder = currentFolder.toString();

            if(type.length !== 0)
                openUrlOfType(selectedFile, type, inTab);
            else
                openUrl(selectedFile, inTab);
        }

        Connections
        {
            target: fileOpenDialog.selectedNameFilter

            function onIndexChanged()
            {
                fileOpenDialog.type = application.urlTypeFor(
                    fileOpenDialog.selectedNameFilter.name,
                    fileOpenDialog.selectedNameFilter.extensions);
            }
        }

        property bool inTab: false
        property string type: ""
    }

    OpenUrlDialog
    {
        id: openUrlDialog
        onAccepted: { openUrl(openUrlDialog.url, true); }
    }

    Action
    {
        id: fileOpenAction
        icon.name: "document-open"
        text: qsTr("&Open…")
        shortcut: "Ctrl+O"
        onTriggered: function(source)
        {
            fileOpenDialog.title = qsTr("Open File…");
            fileOpenDialog.inTab = false;
            fileOpenDialog.type = "";

            if(misc.fileOpenInitialFolder !== undefined)
                fileOpenDialog.currentFolder = misc.fileOpenInitialFolder;

            fileOpenDialog.open();
        }
    }

    Action
    {
        id: fileOpenInTabAction
        icon.name: "tab-new"
        text: qsTr("Open In New &Tab…")
        shortcut: "Ctrl+T"
        onTriggered: function(source)
        {
            fileOpenDialog.title = qsTr("Open File In New Tab…");
            fileOpenDialog.inTab = true;
            fileOpenDialog.type = "";

            if(misc.fileOpenInitialFolder !== undefined)
                fileOpenDialog.currentFolder = misc.fileOpenInitialFolder;

            fileOpenDialog.open();
        }
    }

    Action
    {
        id: urlOpenAction
        icon.name: "network-server"
        text: qsTr("Open &URL…")
        onTriggered: function(source) { openUrlDialog.show(); }
    }

    Action
    {
        id: fileSaveAction
        icon.name: "document-save"
        text: qsTr("&Save")
        shortcut: "Ctrl+S"
        enabled: currentTab && !currentTab.document.busy
        onTriggered: function(source)
        {
            if(currentTab === null)
                return;

            if(application.runningWasm)
                currentTab.saveAsFile();
            else
                currentTab.saveFile();
        }
    }

    Action
    {
        id: fileSaveAsAction
        icon.name: "document-save-as"
        text: qsTr("&Save As…")
        enabled: currentTab && !currentTab.document.busy
        onTriggered: function(source)
        {
            if(currentTab === null)
                return;

            currentTab.saveAsFile();
        }
    }

    Action
    {
        id: closeTabAction
        icon.name: "window-close"
        text: qsTr("&Close Tab")
        shortcut: "Ctrl+W"
        enabled: currentTab !== null
        onTriggered: function(source) { tabBar.closeTab(tabBar.currentIndex); }
    }

    Action
    {
        id: closeAllTabsAction
        icon.name: "window-close"
        text: qsTr("Close &All Tabs")
        shortcut: "Ctrl+Shift+W"
        enabled: currentTab !== null
        onTriggered: function(source)
        {
            if(tabBar.count > 0)
            {
                // If any tabs are open, close the first one...
                tabBar.closeTab(0, function()
                {
                    // ...then (recursively) resume closing if the user doesn't cancel
                    tabBar.removeTab(0);
                    closeAllTabsAction.trigger();
                });
            }
        }
    }

    Action
    {
        id: quitAction
        icon.name: "application-exit"
        text: qsTr("&Quit")
        shortcut: "Ctrl+Q"
        onTriggered: function(source) { mainWindow.close(); }
    }

    Action
    {
        id: undoAction
        icon.name: "edit-undo"
        text: currentTab ? currentTab.document.nextUndoAction : qsTr("&Undo")
        shortcut: "Ctrl+Z"
        enabled: currentTab ? currentTab.document.canUndo : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.undo();
        }
    }

    Action
    {
        id: redoAction
        icon.name: "edit-redo"
        text: currentTab ? currentTab.document.nextRedoAction : qsTr("&Redo")
        shortcut: "Ctrl+Shift+Z"
        enabled: currentTab ? currentTab.document.canRedo : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.redo();
        }
    }

    Action
    {
        id: deleteAction
        icon.name: "edit-delete"
        text: qsTr("&Delete Selection")
        shortcut: "Del"
        property bool visible: currentTab ?
            currentTab.document.canDeleteSelection : false
        enabled: currentTab ? !currentTab.document.busy && visible : false
        onTriggered: function(source) { currentTab.document.deleteSelectedNodes(); }
    }

    Action
    {
        id: selectAllAction
        icon.name: "edit-select-all"
        text: qsTr("Select &All")
        shortcut: "Ctrl+Shift+A"
        enabled: currentTab ? !currentTab.document.busy : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.selectAll();
        }
    }

    Action
    {
        id: selectAllVisibleAction
        icon.name: "edit-select-all"
        text: qsTr("Select All &Visible")
        shortcut: "Ctrl+A"
        enabled: currentTab ? !currentTab.document.busy : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.selectAllVisible();
        }
    }

    Action
    {
        id: selectNoneAction
        text: qsTr("Select &None")
        shortcut: "Ctrl+N"
        enabled: currentTab ? !currentTab.document.busy : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.selectNone();
        }
    }

    Action
    {
        id: selectSourcesAction
        text: qsTr("Select Sources of Selection")
        property bool visible: currentTab ?
            currentTab.document.directed && !currentTab.document.nodeSelectionEmpty : false
        enabled: currentTab ? !currentTab.document.busy && visible : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.selectSources();
        }
    }

    Action
    {
        id: selectTargetsAction
        text: qsTr("Select Targets of Selection")
        property bool visible: currentTab ?
            currentTab.document.directed && !currentTab.document.nodeSelectionEmpty : false
        enabled: currentTab ? !currentTab.document.busy && visible : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.selectTargets();
        }
    }

    Action
    {
        id: selectNeighboursAction
        text: qsTr("Select Neigh&bours of Selection")
        shortcut: "Ctrl+B"
        property bool visible: currentTab ?
            !currentTab.document.nodeSelectionEmpty : false
        enabled: currentTab ? !currentTab.document.busy && visible : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.selectNeighbours();
        }
    }

    Action
    {
        id: repeatLastSelectionAction
        text: currentTab ? currentTab.repeatLastSelectionMenuText :
            qsTr("Repeat Last Selection")

        shortcut: "Ctrl+R"
        enabled: currentTab && currentTab.canRepeatLastSelection
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.repeatLastSelection();
        }
    }

    Action
    {
        id: invertSelectionAction
        text: qsTr("&Invert Selection")
        shortcut: "Ctrl+I"
        enabled: currentTab ? !currentTab.document.busy : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.invertSelection();
        }
    }

    Action
    {
        id: findAction
        icon.name: "edit-find"
        text: qsTr("&Find")
        shortcut: "Ctrl+F"
        enabled: currentTab ? !currentTab.document.busy : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.showFind(Find.Simple);
        }
    }

    Action
    {
        id: advancedFindAction
        icon.name: "edit-find"
        text: qsTr("Advanced Find")
        shortcut: "Ctrl+Shift+F"
        enabled: currentTab ? !currentTab.document.busy : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.showFind(Find.Advanced);
        }
    }

    Action
    {
        id: findByAttributeAction
        icon.name: "edit-find-replace"
        text: qsTr("Find By Attribute Value")
        shortcut: "Ctrl+H"
        enabled:
        {
            if(currentTab)
                return !currentTab.document.busy && currentTab.numAttributesWithSharedValues > 0;

            return false;
        }

        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.showFind(Find.ByAttribute);
        }
    }

    Action
    {
        id: prevComponentAction
        text: qsTr("Goto &Previous Component")
        shortcut: "PgUp"
        enabled: currentTab ? currentTab.document.canChangeComponent : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.gotoPrevComponent();
        }
    }

    Action
    {
        id: nextComponentAction
        text: qsTr("Goto &Next Component")
        shortcut: "PgDown"
        enabled: currentTab ? currentTab.document.canChangeComponent : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.gotoNextComponent();
        }
    }

    Action
    {
        id: optionsAction
        enabled: !mainWindow._anyDocumentsBusy && !tracking.visible
        icon.name: "applications-system"
        text: qsTr("&Options…")
        onTriggered: function(source)
        {
            optionsDialog.raise();
            optionsDialog.show();
        }
    }

    Action
    {
        id: enrichmentAction
        text: qsTr("Enrichment…")
        enabled: currentTab !== null && !currentTab.document.busy
        onTriggered: function(source)
        {
            if(currentTab !== null)
            {
                if(enrichmentResults.models.length > 0)
                    enrichmentResults.show();
                else
                    enrichmentWizard.show();
            }
        }
    }

    TextMetrics
    {
        id: elidedNodeName

        elide: Text.ElideMiddle
        elideWidth: 200
        text: searchWebAction.enabled ?
            currentTab.document.nodeName(searchWebAction._selectedNodeId) : ""
    }

    Action
    {
        id: searchWebAction
        text: enabled ? Utils.format(qsTr("Search Web for '{0}'…"), elidedNodeName.elidedText) :
            qsTr("Search Web for Selected Node…")

        property var _selectedNodeId:
        {
            if(currentTab === null || currentTab.document.numHeadNodesSelected !== 1)
                return null;

            return currentTab.document.selectedHeadNodeIds[0];
        }

        enabled: currentTab !== null && _selectedNodeId !== null
        onTriggered: function(source)
        {
            currentTab.searchWebForNode(_selectedNodeId);
        }
    }

    function attributeIsEditable(attributeName)
    {
        if(!currentTab || currentTab.document.busy)
            return false;

        let attribute = currentTab.document.attribute(attributeName);
        return attribute.isValid && attribute.editable;
    }

    ImportAttributesDialog
    {
        id: importAttributesDialog
        document: currentTab && currentTab.document
    }

    OpenFileDialog
    {
        id: importAttributesFileOpenDialog
        nameFilters:
        [
            "All Files (*.csv *.tsv *.ssv *.xlsx)",
            "CSV Files (*.csv)",
            "TSV Files (*.tsv)",
            "SSV Files (*.ssv)",
            "Excel Files (*.xlsx)"
        ]

        onAccepted:
        {
            misc.fileOpenInitialFolder = currentFolder.toString();
            importAttributesDialog.open(selectedFile);
        }
    }

    CloneAttributeDialog
    {
        id: cloneAttributeDialog
        document: currentTab && currentTab.document
    }

    Action
    {
        id: cloneAttributeAction
        text: qsTr("Clone Attribute…")
        enabled: currentTab !== null && !currentTab.document.busy
        onTriggered: function(source)
        {
            cloneAttributeDialog.sourceAttributeName = "";
            cloneAttributeDialog.show();
        }
    }

    function cloneAttribute(attributeName)
    {
        if(!cloneAttributeAction.enabled)
            return;

        cloneAttributeDialog.sourceAttributeName = attributeName;
        cloneAttributeDialog.show();
    }

    EditAttributeDialog
    {
        id: editAttributeDialog
        document: currentTab && currentTab.document
    }

    Action
    {
        id: editAttributeAction
        text: qsTr("Edit Attribute…")
        enabled: currentTab !== null && !currentTab.document.busy
        onTriggered: function(source)
        {
            editAttributeDialog.attributeName = "";
            editAttributeDialog.show();
        }
    }

    function editAttribute(attributeName)
    {
        if(!editAttributeAction.enabled)
            return;

        editAttributeDialog.attributeName = attributeName;
        editAttributeDialog.show();
    }

    RemoveAttributesDialog
    {
        id: removeAttributesDialog
        document: currentTab && currentTab.document
    }

    Action
    {
        id: removeAttributesAction
        text: qsTr("Remove Attributes…")
        enabled: currentTab !== null && !currentTab.document.busy
        onTriggered: function(source) { removeAttributesDialog.show(); }
    }

    function removeAttributes(attributeNames)
    {
        if(!removeAttributesAction.enabled)
            return;

        currentTab.document.removeAttributes(attributeNames);
    }

    Action
    {
        id: importAttributesAction
        text: qsTr("Import Attributes From Table…")
        enabled: currentTab !== null && !currentTab.document.busy
        onTriggered: function(source)
        {
            if(misc.fileOpenInitialFolder !== undefined)
                importAttributesFileOpenDialog.currentFolder = misc.fileOpenInitialFolder;

            importAttributesFileOpenDialog.open();
        }
    }

    Action
    {
        id: pauseLayoutAction
        icon.name:
        {
            let layoutPauseState = currentTab ? currentTab.document.layoutPauseState : -1;

            switch(layoutPauseState)
            {
            case LayoutPauseState.Paused:          return "media-playback-start";
            case LayoutPauseState.RunningFinished: return "media-playback-stop";
            default:
            case LayoutPauseState.Running:         return "media-playback-pause";
            }
        }

        text: currentTab && currentTab.document.layoutPauseState === LayoutPauseState.Paused ?
                  qsTr("&Resume Layout") : qsTr("&Pause Layout")
        shortcut: "Pause"
        enabled: currentTab ? !currentTab.document.busy : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.toggleLayout();
        }
    }

    Action
    {
        id: toggleLayoutSettingsAction
        icon.name: "preferences-desktop"
        text: qsTr("Layout Settings…")
        shortcut: "Ctrl+L"
        enabled: currentTab && !currentTab.document.busy

        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.showLayoutSettings();
        }
    }

    Action
    {
        id: exportNodePositionsAction
        icon.name: "document-save"
        text: qsTr("Export To File…")
        enabled: currentTab && !currentTab.document.busy

        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.exportNodePositions();
        }
    }

    Action
    {
        id: importNodePositionsAction
        icon.name: "document-open"
        text: qsTr("Import From File…")
        enabled: currentTab && !currentTab.document.busy

        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.importNodePositions();
        }
    }

    Action
    {
        id: overviewModeAction
        icon.name: "view-fullscreen"
        text: qsTr("&Overview Mode")
        enabled: currentTab ? currentTab.document.canEnterOverviewMode : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.switchToOverviewMode();
        }
    }

    Action
    {
        id: resetViewAction
        icon.name: "view-refresh"
        text: qsTr("&Reset View")
        enabled: currentTab ? currentTab.document.canResetView : false
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.resetView();
        }
    }

    Action
    {
        id: toggleGraphMetricsAction
        text: qsTr("Show Graph Metrics")
        checkable: true
    }

    Action
    {
        id: toggleEdgeDirectionAction
        text: qsTr("Show Edge Direction")
        checkable: true

        Component.onCompleted:
        {
            toggleEdgeDirectionAction.checked = !(visuals.edgeVisualType === EdgeVisualType.Cylinder);
        }
    }

    Action
    {
        id: addBookmarkAction
        icon.name: "list-add"
        text: qsTr("Add Bookmark…")
        shortcut: "Ctrl+D"
        enabled: currentTab ? !currentTab.document.busy && currentTab.document.numNodesSelected > 0 : false
        onTriggered: function(source)
        {
            if(currentTab !== null)
                currentTab.showAddBookmark();
        }
    }

    ManageNamedListDialog
    {
        id: manageBookmarks

        title: qsTr("Manage Bookmarks")
        model: currentTab ? currentTab.document.bookmarks : []

        onRemove: function(names) { currentTab.document.removeBookmarks(names); }
        onRename: function(from, to) { currentTab.document.renameBookmark(from, to); }
    }

    Action
    {
        id: manageBookmarksAction
        text: qsTr("Manage Bookmarks…")
        enabled: currentTab ? !currentTab.document.busy && currentTab.document.bookmarks.length > 0 : false
        onTriggered: function(source)
        {
            manageBookmarks.raise();
            manageBookmarks.show();
        }
    }

    Action
    {
        id: activateAllBookmarksAction
        text: qsTr("Activate All Bookmarks")
        enabled: currentTab ? !currentTab.document.busy && currentTab.document.bookmarks.length > 1 : false
        onTriggered: function(source)
        {
            if(currentTab !== null)
                currentTab.gotoAllBookmarks();
        }
    }

    AddTemplateDialog
    {
        id: addTemplateDialog
        document: currentTab && currentTab.document

        onAccepted: function(name, method, transforms, visualisations)
        {
            templates.add(name, method, transforms, visualisations);
        }
    }

    Action
    {
        id: addTemplateAction
        icon.name: "list-add"
        text: qsTr("Add Template…")
        enabled: currentTab ? !currentTab.document.busy &&
            (currentTab.document.transforms.length > 0 ||
            currentTab.document.visualisations.length > 0) : false
        onTriggered: function(source)
        {
            addTemplateDialog.raise();
            addTemplateDialog.show();
        }
    }

    ManageNamedListDialog
    {
        id: manageTemplates

        title: qsTr("Manage Templates")
        model: { return templates.namesAsArray(); }

        onRemove: function(names) { templates.remove(names); }
        onRename: function(from, to) { templates.rename(from, to); }
    }

    Action
    {
        id: manageTemplatesAction
        text: qsTr("Manage Templates…")
        enabled: { return templates.namesAsArray().length > 0; }
        onTriggered: function(source)
        {
            manageTemplates.raise();
            manageTemplates.show();
        }
    }

    ActionGroup
    {
        id: nodeTextDisplay

        Action { id: hideNodeTextAction; text: qsTr("None"); checkable: true; }
        Action { id: showFocusedNodeTextAction; text: qsTr("Focused"); checkable: true; }
        Action { id: showSelectedNodeTextAction; text: qsTr("Selected"); checkable: true; }
        Action { id: showAllNodeTextAction; text: qsTr("All"); checkable: true; }

        Component.onCompleted:
        {
            switch(visuals.showNodeText)
            {
            default:
            case TextState.Off:      nodeTextDisplay.checkedAction = hideNodeTextAction; break;
            case TextState.Focused:  nodeTextDisplay.checkedAction = showFocusedNodeTextAction; break;
            case TextState.Selected: nodeTextDisplay.checkedAction = showSelectedNodeTextAction; break;
            case TextState.All:      nodeTextDisplay.checkedAction = showAllNodeTextAction; break;
            }
        }
    }

    ActionGroup
    {
        id: edgeTextDisplay

        Action { id: hideEdgeTextAction; text: qsTr("None"); checkable: true; }
        Action { id: showSelectedEdgeTextAction; text: qsTr("Selected"); checkable: true; }
        Action { id: showAllEdgeTextAction; text: qsTr("All"); checkable: true; }

        Component.onCompleted:
        {
            switch(visuals.showEdgeText)
            {
            default:
            case TextState.Off:      edgeTextDisplay.checkedAction = hideEdgeTextAction; break;
            case TextState.Selected: edgeTextDisplay.checkedAction = showSelectedEdgeTextAction; break;
            case TextState.All:      edgeTextDisplay.checkedAction = showAllEdgeTextAction; break;
            }
        }
    }

    ActionGroup
    {
        id: projection

        Action
        {
            id: perspecitveProjectionAction
            enabled: currentTab ? !currentTab.document.busy : false
            text: qsTr("Perspective")
            checkable: true
            onCheckedChanged: function(checked)
            {
                if(currentTab !== null && checked)
                {
                    currentTab.document.setProjection(Projection.Perspective);
                    updateShadingMode(currentTab.document);
                }
            }
        }

        Action
        {
            id: orthographicProjectionAction
            enabled: currentTab ? !currentTab.document.busy : false
            text: qsTr("Orthographic")
            checkable: true
            onCheckedChanged: function(checked)
            {
                if(currentTab !== null && checked)
                {
                    currentTab.document.setProjection(Projection.Orthographic);
                    updateShadingMode(currentTab.document);
                }
            }
        }

        Action
        {
            id: twoDeeProjectionAction
            enabled: currentTab ? !currentTab.document.busy : false
            text: qsTr("2D")
            checkable: true
            onCheckedChanged: function(checked)
            {
                if(currentTab !== null && checked)
                {
                    currentTab.document.setProjection(Projection.TwoDee);
                    updateShadingMode(currentTab.document);
                }
            }
        }
    }

    ActionGroup
    {
        id: shading

        Action
        {
            id: smoothShadingAction
            enabled: currentTab ? !currentTab.document.busy : false
            text: qsTr("Smooth Shading")
            checkable: true
            onCheckedChanged: function(checked)
            {
                if(currentTab !== null && checked)
                    currentTab.document.setShading(Shading.Smooth);
            }
        }

        Action
        {
            id: flatShadingAction
            enabled: currentTab ? !currentTab.document.busy : false
            text: qsTr("Flat Shading")
            checkable: true
            onCheckedChanged: function(checked)
            {
                if(currentTab !== null && checked)
                    currentTab.document.setShading(Shading.Flat);
            }
        }
    }

    Action
    {
        id: showMultiElementIndicatorsAction
        text: qsTr("Show Multi-Element Indicators")
        checkable: true
    }

    Action
    {
        id: showEdgesAction
        text: qsTr("Show Edges")
        checkable: true
    }

    Action
    {
        id: testConsoleOutputAction
        text: qsTr("Test Console Output")
        onTriggered: application.testConsoleOutput()
    }

    Action
    {
        id: dumpGraphAction
        text: qsTr("Dump graph to qDebug")
        enabled: application.debugEnabled
        onTriggered: currentTab && currentTab.document.dumpGraph()
    }

    Action
    {
        id: dumpCommandStackAction
        text: qsTr("Dump command stack to qDebug")
        enabled: application.debugEnabled
        onTriggered: function(source)
        {
            if(currentTab)
                console.log(currentTab.document.commandStackSummary());
        }
    }

    Action
    {
        id: reportScopeTimersAction
        text: qsTr("Report Scope Timers")
        onTriggered: function(source) { application.reportScopeTimers(); }
    }

    Action
    {
        id: restartAction
        text: qsTr("Restart")
        onTriggered: function(source) { mainWindow.restart(); }
    }

    Labs.MessageDialog
    {
        id: commandLineArgumentsMessageDialog
        title: qsTr("Command Line Arguments")
        modality: Qt.ApplicationModal
    }

    Action
    {
        id: showCommandLineArgumentsAction
        text: qsTr("Show Command Line Arguments")
        onTriggered: function(source)
        {
            commandLineArgumentsMessageDialog.text = "Arguments:\n\n" +
                JSON.stringify(mainWindow._processedArguments, null, 4);

            commandLineArgumentsMessageDialog.open();
        }
    }

    Action
    {
        id: showEnvironmentAction
        text: qsTr("Show Environment")
        onTriggered: function(source) { environmentDialog.show(); }
    }

    Action
    {
        id: showOpenGLInfoAction
        text: qsTr("Show OpenGL Info")
        onTriggered: function(source) { openGLInfoDialog.show(); }
    }

    Action
    {
        id: showPaletteAction
        text: qsTr("Show Palette")
        onTriggered: function(source) { showPaletteDialog.show(); }
    }

    Action
    {
        id: testCommandAction
        text: qsTr("Test Command")
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.document.startTestCommand();
        }
    }

    Action
    {
        id: testInfiniteParseAction
        text: qsTr("Test Infinite Parse")
        onTriggered:
        {
            let infiniteParseUrl = QmlUtils.urlForFileName(application.resourceFile(
                "debug/infiniteParse.graphia"));

            if(QmlUtils.fileUrlExists(infiniteParseUrl))
                openUrl(infiniteParseUrl, true);
        }
    }

    Action
    {
        id: saveImageAction
        icon.name: "camera-photo"
        text: qsTr("Save As Image…")
        enabled: currentTab && !currentTab.document.busy
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.screenshot();
        }
    }

    Action
    {
        id: toggleFpsMeterAction
        text: qsTr("Show FPS Meter")
        checkable: true
    }

    Action
    {
        id: toggleGlyphmapSaveAction
        text: qsTr("Save Glyphmaps on Regeneration")
        checkable: true
    }

    Action
    {
        id: togglePluginMinimiseAction
        shortcut: "Ctrl+M"
        icon.name: currentTab && currentTab.pluginMinimised ? "go-top" : "go-bottom"
        text: currentTab ? Utils.format(currentTab.pluginMinimised ? qsTr("Restore {0}") : qsTr("Minimise {0}"),
            currentTab.document.pluginName) : ""
        enabled: currentTab && currentTab.document.hasPluginUI && !currentTab.pluginPoppedOut

        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.toggleMinimise();
        }
    }

    Shortcut
    {
        enabled: currentTab && !currentTab.panelVisible
        sequence: "Esc"
        onActivated:
        {
            if(currentTab.document.canEnterOverviewMode)
                overviewModeAction.trigger();
            else if(currentTab.document.canResetView)
                resetViewAction.trigger();
            else if(currentTab.document.hasPluginUI && !currentTab.pluginPoppedOut)
                togglePluginMinimiseAction.trigger();
       }
    }

    Action
    {
        id: togglePluginWindowAction
        icon.name: "preferences-system-windows"
        text: currentTab ? Utils.format(qsTr("Display {0} In Separate &Window"), currentTab.document.pluginName) : ""
        checkable: true
        checked: currentTab && currentTab.pluginPoppedOut
        enabled: currentTab && currentTab.document.hasPluginUI && !mainWindow._anyDocumentsBusy
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.togglePop();
        }
    }

    Action
    {
        id: showProvenanceLogAction
        text: qsTr("Show Provenance Log…")
        enabled: currentTab !== null && !currentTab.document.busy
        onTriggered: function(source)
        {
            provenanceLogDialog.raise();
            provenanceLogDialog.show();
        }
    }

    Action
    {
        id: aboutPluginsAction
        // Don't ask...
        text: Qt.platform.os === "osx" ? qsTr("Plugins…") : qsTr("About Plugins…")
        onTriggered: function(source)
        {
            aboutpluginsDialog.pluginDetails = application.pluginDetailsModel();
            aboutpluginsDialog.raise();
            aboutpluginsDialog.show();
        }
    }

    Action
    {
        id: aboutAction
        text: qsTr("About " + application.name + "…")
        onTriggered: function(source)
        {
            aboutDialog.raise();
            aboutDialog.show();
        }
    }

    Action
    {
        id: aboutQtAction
        text: Qt.platform.os === "osx" ? qsTr("Qt…") : qsTr("About Qt…")
        onTriggered: function(source)
        {
            // For some reason calling aboutQt prevents the
            // menu from closing, so do it in advance
            if(source instanceof MenuItem)
                source.menu.dismiss();

            application.aboutQt();
        }
    }

    Action
    {
        id: checkForUpdatesAction
        enabled: !newUpdate.visible

        text: qsTr("Check For Updates")

        property bool active: false

        onTriggered: function(source)
        {
            active = true;
            application.checkForUpdates();
            newUpdate.visible = false;
        }
    }

    Action
    {
        id: showLatestChangesAction
        enabled: changeLog.available

        text: qsTr("Latest Changes…")

        onTriggered: function(source)
        {
            latestChangesDialog.raise();
            latestChangesDialog.show();
        }
    }

    Action
    {
        id: copyImageToClipboardAction
        text: qsTr("Copy Viewport To Clipboard")
        shortcut: "Ctrl+C"
        enabled: currentTab
        onTriggered: function(source)
        {
            if(currentTab)
                currentTab.copyImageToClipboard();
        }
    }

    Action
    {
        id: onlineHelpAction
        text: qsTr("Online Help")
        shortcut: "F1"
        onTriggered: function(source) { Qt.openUrlExternally(QmlUtils.redirectUrl("help")); }
    }

    Action
    {
        id: exampleDataSetsAction
        text: qsTr("Example Datasets")
        onTriggered: function(source) { Qt.openUrlExternally(QmlUtils.redirectUrl("example_datasets")); }
    }

    property bool debugMenuUnhidden: false

    PlatformMenuBar
    {
        id: mainMenuBar

        visible: !tracking.visible

        PlatformMenu
        {
            title: qsTr("&File")
            PlatformMenuItem { action: fileOpenAction }
            PlatformMenuItem { action: fileOpenInTabAction }
            PlatformMenuItem { action: urlOpenAction }
            PlatformMenu
            {
                id: recentFileMenu
                hidden: application.runningWasm
                title: qsTr("&Recent Files")

                Instantiator
                {
                    model: mainWindow.recentFiles
                    delegate: PlatformMenuItem
                    {
                        // FIXME: This fires with a -1 index onOpenFile
                        text: index > -1 ? QmlUtils.fileNameForUrl(mainWindow.recentFiles[index]) : "";
                        onTriggered:
                        {
                            openUrl(QmlUtils.urlForFileName(text), true);
                        }
                    }
                    onObjectAdded: function(index, object) { recentFileMenu.insertItem(index, object); }
                    onObjectRemoved: function(index, object) { recentFileMenu.removeItem(object); }
                }
            }
            PlatformMenuSeparator {}
            PlatformMenuItem { hidden: application.runningWasm; action: fileSaveAction }
            PlatformMenuItem { action: fileSaveAsAction }
            PlatformMenuItem { action: saveImageAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: closeTabAction }
            PlatformMenuItem { action: closeAllTabsAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: quitAction }
        }
        PlatformMenu
        {
            title: qsTr("&Edit")
            PlatformMenuItem { action: undoAction }
            PlatformMenuItem { action: redoAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: deleteAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: selectAllAction }
            PlatformMenuItem { action: selectAllVisibleAction }
            PlatformMenuItem { action: selectNoneAction }
            PlatformMenuItem { action: invertSelectionAction }
            PlatformMenuItem { hidden: !selectSourcesAction.enabled; action: selectSourcesAction }
            PlatformMenuItem { hidden: !selectTargetsAction.enabled; action: selectTargetsAction }
            PlatformMenuItem { action: selectNeighboursAction }
            PlatformMenu
            {
                id: sharedValuesMenu
                title: qsTr("Select Shared Values of Selection")
                enabled: currentTab !== null && !currentTab.document.nodeSelectionEmpty &&
                    currentTab.numAttributesWithSharedValues > 0

                Instantiator
                {
                    model: currentTab !== null ? currentTab.sharedValuesAttributeNames : []
                    PlatformMenuItem
                    {
                        text: modelData
                        onTriggered: { currentTab.selectBySharedAttributeValue(text); }
                    }
                    onObjectAdded: function(index, object) { sharedValuesMenu.insertItem(index, object); }
                    onObjectRemoved: function(index, object) { sharedValuesMenu.removeItem(object); }
                }
            }
            PlatformMenuItem { action: repeatLastSelectionAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: findAction }
            PlatformMenuItem { action: advancedFindAction }
            PlatformMenuItem { action: findByAttributeAction }
            PlatformMenuItem
            {
                action: currentTab ? currentTab.previousAction : null
                hidden: !currentTab
            }
            PlatformMenuItem
            {
                action: currentTab ? currentTab.nextAction : null
                hidden: !currentTab
            }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: prevComponentAction }
            PlatformMenuItem { action: nextComponentAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: optionsAction }
        }
        PlatformMenu
        {
            title: qsTr("&View")
            PlatformMenuItem { action: overviewModeAction }
            PlatformMenuItem { action: resetViewAction }
            PlatformMenuItem
            {
                action: togglePluginWindowAction
                hidden: !currentTab || !currentTab.document.hasPluginUI
            }
            PlatformMenuItem
            {
                action: togglePluginMinimiseAction
                hidden: !currentTab || !currentTab.document.hasPluginUI
            }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: toggleGraphMetricsAction }
            PlatformMenu
            {
                title: qsTr("Show Node Text")
                PlatformMenuItem { action: hideNodeTextAction }
                PlatformMenuItem { action: showFocusedNodeTextAction }
                PlatformMenuItem { action: showSelectedNodeTextAction }
                PlatformMenuItem { action: showAllNodeTextAction }
            }
            PlatformMenu
            {
                title: qsTr("Show Edge Text")

                PlatformMenuItem
                {
                    id: edgeTextWarning

                    enabled: false
                    hidden: !currentTab || currentTab.document.hasValidEdgeTextVisualisation ||
                        visuals.showEdgeText === TextState.Off
                    text: qsTr("⚠ Visualisation Required For Edge Text")
                }

                PlatformMenuSeparator { hidden: edgeTextWarning.hidden }

                PlatformMenuItem { action: hideEdgeTextAction }
                PlatformMenuItem { action: showSelectedEdgeTextAction }
                PlatformMenuItem { action: showAllEdgeTextAction }
            }
            PlatformMenuItem
            {
                action: toggleEdgeDirectionAction
                hidden: !currentTab || !currentTab.document.directed
            }
            PlatformMenuItem { action: showMultiElementIndicatorsAction }
            PlatformMenuItem { action: showEdgesAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: perspecitveProjectionAction }
            PlatformMenuItem { action: orthographicProjectionAction }
            PlatformMenuItem { action: twoDeeProjectionAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: smoothShadingAction }
            PlatformMenuItem { action: flatShadingAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: copyImageToClipboardAction }
        }
        PlatformMenu
        {
            title: qsTr("&Layout")
            PlatformMenuItem { action: pauseLayoutAction }
            PlatformMenuItem { action: toggleLayoutSettingsAction }
            PlatformMenuItem { action: exportNodePositionsAction }
            PlatformMenuItem { action: importNodePositionsAction }
        }
        PlatformMenu
        {
            title: qsTr("T&ools")
            PlatformMenuItem { action: enrichmentAction }
            PlatformMenuItem { action: searchWebAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: cloneAttributeAction }
            PlatformMenuItem { action: editAttributeAction }
            PlatformMenuItem { action: removeAttributesAction }
            PlatformMenuItem { action: importAttributesAction }
            PlatformMenuSeparator {}
            PlatformMenuItem { action: showProvenanceLogAction }
        }
        PlatformMenu
        {
            id: bookmarksMenu

            title: qsTr("&Bookmarks")
            PlatformMenuItem { action: addBookmarkAction }
            PlatformMenuItem { action: manageBookmarksAction }
            PlatformMenuSeparator { hidden: currentTab ? currentTab.document.bookmarks.length === 0 : true }

            PlatformMenuItem
            {
                action: activateAllBookmarksAction
                hidden: currentTab ? currentTab.document.bookmarks.length === 1 : true
            }

            Instantiator
            {
                model: currentTab ? currentTab.document.bookmarks : []
                delegate: PlatformMenuItem
                {
                    id: bookmarkMenuItem

                    // The Action is used so that a shortcut can be assigned; this is
                    // functionality that isn't (necessarily) available in a plain MenuItem
                    action: Action
                    {
                        enabled: currentTab ? !currentTab.document.busy : false
                        text: index > -1 ? currentTab.document.bookmarks[index] : "";

                        shortcut:
                        {
                            if(index >= 0 && index < 9)
                                return "Ctrl+" + (index + 1);
                            else if(index === 9)
                                return "Ctrl+0";

                            return "";
                        }

                        onTriggered: function(source)
                        {
                            if(source && source.menu)
                                source.menu.dismiss();

                            currentTab.gotoBookmark(bookmarkMenuItem.text);
                        }
                    }
                }

                onObjectAdded: function(index, object) { bookmarksMenu.insertItem(4/* first menu items */ + index, object); }
                onObjectRemoved: function(index, object) { bookmarksMenu.removeItem(object); }
            }
        }

        PlatformMenu
        {
            id: templatesMenu

            title: qsTr("T&emplates")
            PlatformMenuItem { action: addTemplateAction }
            PlatformMenuItem { action: manageTemplatesAction }
            PlatformMenuSeparator { hidden: { return templates.namesAsArray().length === 0; } }

            Instantiator
            {
                id: templatesInstantiator
                model: { return templates.namesAsArray(); }

                delegate: PlatformMenuItem
                {
                    id: templateMenuItem

                    text: templatesInstantiator.model[index]

                    enabled:
                    {
                        if(!currentTab || currentTab.document.busy)
                            return false;

                        let template = templates.templateFor(text);
                        if(!template)
                            return false;

                        return currentTab.document.graphTransformsAreValid(template.transforms) &&
                            currentTab.document.visualisationsAreValid(template.visualisations);
                    }

                    onTriggered: function(source)
                    {
                        if(source && source.menu)
                            source.menu.dismiss();

                        currentTab.applyTemplate(templateMenuItem.text);
                    }
                }

                onObjectAdded: function(index, object) { templatesMenu.insertItem(3/* first menu items */ + index, object); }
                onObjectRemoved: function(index, object) { templatesMenu.removeItem(object); }
            }
        }

        PlatformMenu { id: pluginMenu0; hidden: true; enabled: currentTab && !currentTab.document.busy }
        PlatformMenu { id: pluginMenu1; hidden: true; enabled: currentTab && !currentTab.document.busy }
        PlatformMenu { id: pluginMenu2; hidden: true; enabled: currentTab && !currentTab.document.busy }
        PlatformMenu { id: pluginMenu3; hidden: true; enabled: currentTab && !currentTab.document.busy }
        PlatformMenu { id: pluginMenu4; hidden: true; enabled: currentTab && !currentTab.document.busy }

        PlatformMenu
        {
            title: qsTr("&Debug")
            hidden: !application.debugEnabled && !mainWindow.debugMenuUnhidden
            PlatformMenu
            {
                title: qsTr("&Crash")
                PlatformMenuItem
                {
                    text: qsTr("Null Pointer Deference");
                    onTriggered: { application.crash(CrashType.NullPtrDereference); }
                }
                PlatformMenuItem
                {
                    text: qsTr("C++ Exception");
                    onTriggered: { application.crash(CrashType.CppException); }
                }
                PlatformMenuItem
                {
                    text: qsTr("std::exception");
                    onTriggered: { application.crash(CrashType.StdException); }
                }
                PlatformMenuItem
                {
                    text: qsTr("Fatal Error");
                    onTriggered: { application.crash(CrashType.FatalError); }
                }
                PlatformMenuItem
                {
                    text: qsTr("Infinite Loop");
                    onTriggered: { application.crash(CrashType.InfiniteLoop); }
                }
                PlatformMenuItem
                {
                    text: qsTr("Deadlock");
                    onTriggered: { application.crash(CrashType.Deadlock); }
                }
                PlatformMenuItem
                {
                    text: qsTr("Hitch");
                    onTriggered: { application.crash(CrashType.Hitch); }
                }
                PlatformMenuItem
                {
                    hidden: Qt.platform.os !== "windows"
                    text: qsTr("Windows Exception");
                    onTriggered: { application.crash(CrashType.Win32Exception); }
                }
                PlatformMenuItem
                {
                    hidden: Qt.platform.os !== "windows"
                    text: qsTr("Windows Exception Non-Continuable");
                    onTriggered: { application.crash(CrashType.Win32ExceptionNonContinuable); }
                }
                PlatformMenuItem
                {
                    text: qsTr("Silent Submit");
                    onTriggered: { application.crash(CrashType.SilentSubmit); }
                }
            }
            PlatformMenuItem { action: testConsoleOutputAction }
            PlatformMenuItem { action: dumpGraphAction }
            PlatformMenuItem { action: dumpCommandStackAction }
            PlatformMenuItem { action: toggleFpsMeterAction }
            PlatformMenuItem { action: toggleGlyphmapSaveAction }
            PlatformMenuItem { action: reportScopeTimersAction }
            PlatformMenuItem { action: showCommandLineArgumentsAction }
            PlatformMenuItem { action: showEnvironmentAction }
            PlatformMenuItem { action: showOpenGLInfoAction }
            PlatformMenuItem { action: showPaletteAction }
            PlatformMenuItem { action: testCommandAction }
            PlatformMenuItem { action: testInfiniteParseAction }
            PlatformMenuItem { action: restartAction }
        }

        PlatformMenu
        {
            title: qsTr("&Help")

            PlatformMenuItem { action: onlineHelpAction }
            PlatformMenuItem { action: exampleDataSetsAction }

            PlatformMenuItem
            {
                text: qsTr("Show Tutorial…")
                onTriggered:
                {
                    let exampleFileUrl = QmlUtils.urlForFileName(application.resourceFile(
                        "examples/Tutorial.graphia"));

                    if(QmlUtils.fileUrlExists(exampleFileUrl))
                    {
                        let tutorialAlreadyOpen = tabBar.findAndActivateTab(exampleFileUrl);
                        openUrl(exampleFileUrl, !tutorialAlreadyOpen);
                    }
                }
            }

            PlatformMenuSeparator {}
            PlatformMenuItem { action: aboutAction }
            PlatformMenuItem { action: aboutPluginsAction }
            PlatformMenuItem { action: aboutQtAction }

            PlatformMenuSeparator {}
            PlatformMenuItem { action: checkForUpdatesAction }
            PlatformMenuItem { action: showLatestChangesAction }
        }
    }

    function clearPluginMenu(menu)
    {
        menu.hidden = true;
        MenuUtils.clear(menu);
    }

    function clearPluginMenus()
    {
        if(currentTab !== null)
        {
            clearPluginMenu(currentTab.pluginMenu0);
            clearPluginMenu(currentTab.pluginMenu1);
            clearPluginMenu(currentTab.pluginMenu2);
            clearPluginMenu(currentTab.pluginMenu3);
            clearPluginMenu(currentTab.pluginMenu4);
        }

        clearPluginMenu(pluginMenu0);
        clearPluginMenu(pluginMenu1);
        clearPluginMenu(pluginMenu2);
        clearPluginMenu(pluginMenu3);
        clearPluginMenu(pluginMenu4);
    }

    function updatePluginMenu(index, menu)
    {
        clearPluginMenu(menu);

        if(currentTab !== null)
        {
            if(currentTab.createPluginMenu(index, menu))
                menu.hidden = false;
        }
    }

    function updatePluginMenus()
    {
        clearPluginMenus();

        if(currentTab !== null && currentTab.pluginPoppedOut)
        {
            updatePluginMenu(0, currentTab.pluginMenu0);
            updatePluginMenu(1, currentTab.pluginMenu1);
            updatePluginMenu(2, currentTab.pluginMenu2);
            updatePluginMenu(3, currentTab.pluginMenu3);
            updatePluginMenu(4, currentTab.pluginMenu4);
        }
        else
        {
            updatePluginMenu(0, pluginMenu0);
            updatePluginMenu(1, pluginMenu1);
            updatePluginMenu(2, pluginMenu2);
            updatePluginMenu(3, pluginMenu3);
            updatePluginMenu(4, pluginMenu4);
        }
    }

    function updateShadingMode(document)
    {
        switch(document.shading())
        {
        default:
        case Shading.Smooth:    shading.checkedAction = smoothShadingAction; break;
        case Shading.Flat:      shading.checkedAction = flatShadingAction; break;
        }
    }

    function onDocumentShown(document)
    {
        enrichmentResults.models = document.enrichmentTableModels;

        switch(document.projection())
        {
        default:
        case Projection.Perspective:    projection.checkedAction = perspecitveProjectionAction; break;
        case Projection.Orthographic:   projection.checkedAction = orthographicProjectionAction; break;
        case Projection.TwoDee:         projection.checkedAction = twoDeeProjectionAction; break;
        }

        updateShadingMode(document);
    }

    onCurrentTabChanged:
    {
        updatePluginMenus();

        if(currentTab !== null)
            onDocumentShown(currentTab.document);
    }

    function writeTableModelToFile(model, fileName, type, columnNames)
    {
        if(!currentTab || currentTab.document.busy)
            return;

        currentTab.document.writeTableModelToFile(model, fileName, type, columnNames);
    }

    function copyTableModelColumnToClipboard(model, column, rows)
    {
        if(!currentTab || currentTab.document.busy)
            return;

        currentTab.document.copyTableModelColumnToClipboard(model, column, rows);
    }

    EnrichmentResults
    {
        id: enrichmentResults
        wizard: enrichmentWizard
        models: currentTab ? currentTab.document.enrichmentTableModels : []

        onRemoveResults: function(index)
        {
            currentTab.document.removeEnrichmentResults(index);
        }
    }

    EnrichmentWizard
    {
        id: enrichmentWizard
        document: currentTab && currentTab.document
        onAccepted:
        {
            if(currentTab !== null)
                currentTab.document.performEnrichment(selectedAttributeGroupA, selectedAttributeGroupB);

            enrichmentWizard.reset();
        }
    }

    Connections
    {
        target: currentTab

        function onPluginLoadComplete() { updatePluginMenus(); }
        function onPluginPoppedOutChanged() { updatePluginMenus(); }
    }

    Connections
    {
        target: currentTab && currentTab.document

        function onEnrichmentTableModelsChanged()
        {
            enrichmentResults.models = currentTab.document.enrichmentTableModels;
        }

        function onEnrichmentAnalysisComplete() { enrichmentResults.visible = true; }

        // Plugin menus may reference attributes, so regenerate menus when these change
        function onAttributesChanged() { updatePluginMenus(); }
    }

    header: ToolBar
    {
        id: mainToolBar
        topPadding: Constants.padding
        bottomPadding: Constants.padding

        visible: !tracking.visible

        RowLayout
        {
            anchors.fill: parent

            ToolBarButton { action: fileOpenAction }
            ToolBarButton { action: fileOpenInTabAction }
            ToolBarButton { action: fileSaveAction }
            ToolBarSeparator {}
            ToolBarButton
            {
                id: pauseLayoutButton
                action: pauseLayoutAction
                text: ""
            }
            ToolBarButton { action: toggleLayoutSettingsAction }
            ToolBarSeparator {}
            ToolBarButton { action: deleteAction }
            ToolBarButton { action: findAction }
            ToolBarButton { action: findByAttributeAction }
            ToolBarButton { action: undoAction }
            ToolBarButton { action: redoAction }
            ToolBarSeparator {}
            ToolBarButton { action: resetViewAction }
            ToolBarButton { action: optionsAction }

            Item { Layout.fillWidth: true }

            // This is only displayed if the user is checking for updates manually
            RowLayout
            {
                id: updateProgressIndicator
                visible: checkForUpdatesAction.active

                Text
                {
                    text: application.updateDownloadProgress >= 0 ?
                        qsTr("Downloading Update:") : qsTr("Checking For Update:")
                }

                ProgressBar
                {
                    visible: parent.visible
                    indeterminate: application.updateDownloadProgress < 0
                    value: !indeterminate ? application.updateDownloadProgress / 100.0 : 0.0
                }
            }

            NewUpdate
            {
                id: newUpdate
                visible: false

                onRestartClicked: { mainWindow.restart(); }
            }
        }
    }

    DropArea
    {
        anchors.fill: parent
        onDropped: function(drop)
        {
            let arguments = [];

            if(drop.hasText && drop.text.length > 0)
            {
                let text = drop.text.trim();
                text = QmlUtils.urlIsValid(text) ? Qt.resolvedUrl(text) : text;
                arguments.push(text);
            }

            if(drop.hasUrls)
            {
                let resolvedUrls = drop.urls.map(url => Qt.resolvedUrl(url));
                arguments.push(...resolvedUrls);
            }

            // Remove duplicates
            arguments = [...new Set(arguments)];

            processArguments(arguments);
        }

        Component
        {
            id: tabButtonComponent

            TabBarButton
            {
                showCloseButton: true
                property var tab: null

                onCloseButtonClicked:
                {
                    let index = tabBar.findTabIndex(tab);
                    if(index >= 0)
                        tabBar.closeTab(index);
                }
            }
        }

        Component
        {
            id: tabComponent

            TabUI
            {
                onLoadComplete: function(url, success)
                {
                    if(success)
                    {
                        processOnePendingArgument();

                        if(application.isResourceFileUrl(url) &&
                            QmlUtils.baseFileNameForUrlNoExtension(url) === "Tutorial")
                        {
                            // Mild hack: if it looks like the tutorial file,
                            // it probably is, so start the tutorial
                            startTutorial();
                        }
                        else if(!application.downloaded(url) && !application.runningWasm)
                            addToRecentFiles(url);

                        if(currentTab !== null)
                            onDocumentShown(currentTab.document);
                    }
                    else
                    {
                        tabBar.onLoadFailure(tabBar.findTabIndex(this), url);
                        onLoadFailure();
                    }
                }

                property var onLoadFailure: function()
                {
                    // Remove the tab that was created but won't be used
                    tabBar.removeTab(tabBar.findTabIndex(this));
                }
            }
        }

        ColumnLayout
        {
            anchors.fill: parent
            spacing: 0

            visible: !tracking.visible

            TabBar
            {
                id: tabBar

                Layout.topMargin: 4
                visible: count > 1

                onCountChanged:
                {
                    if(count === 0)
                        lastTabClosed();
                }

                function insertTabAtIndex(index)
                {
                    let tab = tabComponent.createObject(null);
                    tabLayout.insert(index, tab);

                    let button = tabButtonComponent.createObject(tabBar, {"tab": tab});
                    tabBar.insertItem(index, button);
                    tabBar.currentIndex = index;

                    // Make the tab title match the document title
                    button.text = Qt.binding(function() { return tab.title });

                    return tab;
                }

                function removeTab(index)
                {
                    if(index >= tabBar.count)
                    {
                        console.log("removeTab called with out of range index: " + index);
                        return;
                    }

                    tabBar.removeItem(tabBar.itemAt(index));
                    let tab = tabLayout.get(index);
                    tabLayout.remove(index);

                    // callLater avoids some "hitchiness"
                    Qt.callLater(tab.destroy);
                }

                function createTab(onCreateFunction)
                {
                    let tab = insertTabAtIndex(tabBar.count);

                    if(typeof(onCreateFunction) !== "undefined")
                        onCreateFunction();

                    return tab;
                }

                function replaceTab(onReplaceFunction)
                {
                    let oldIndex = tabBar.currentIndex;

                    removeTab(oldIndex);
                    let tab = insertTabAtIndex(oldIndex);

                    if(onReplaceFunction !== "undefined")
                        onReplaceFunction();

                    return tab;
                }

                // This is called if the file can't be opened immediately, or
                // if the load has been attempted but it failed later
                function onLoadFailure(index, url)
                {
                    let tab = tabLayout.get(index);

                    let loadWasCancelled = tab.document.commandIsCancelling;
                    if(!loadWasCancelled)
                    {
                        if(tab.document.failureReason.length > 0)
                        {
                            errorOpeningFileMessageDialog.text = Utils.format(
                                qsTr("{0} could not be opened:\n\n{1}"),
                                userTextForUrl(url), tab.document.failureReason);
                        }
                        else
                        {
                            errorOpeningFileMessageDialog.text = Utils.format(
                                qsTr("{0} could not be opened due to an unspecified error."),
                                userTextForUrl(url));
                        }

                        errorOpeningFileMessageDialog.open();
                    }
                }

                function openInCurrentTab(url, type, pluginName, parameters)
                {
                    let tab = currentTab;
                    tab.application = application;
                    tab.openUrl(url, type, pluginName, parameters);
                }

                function closeTab(index, onCloseFunction)
                {
                    if(index < 0 || index >= tabBar.count)
                    {
                        console.log("closeTab called with out of range index: " + index);
                        return false;
                    }

                    if(typeof(onCloseFunction) === "undefined")
                        onCloseFunction = function() { removeTab(index); }

                    tabBar.currentIndex = index;
                    let tab = tabLayout.get(index);

                    // If we're currently busy, cancel and wait before closing
                    if(tab.document.commandInProgress)
                    {
                        if(!tab.document.loadComplete)
                        {
                            tab.onLoadFailure = onCloseFunction;

                            // Still loading, cancel it
                            tab.document.cancelCommand();
                        }
                        else
                        {
                            let closeTabFunction = function()
                            {
                                tab.commandComplete.disconnect(closeTabFunction);

                                // Invoke the close again, now that the command is cancelled
                                tabBar.closeTab(index, onCloseFunction);
                            };

                            tab.commandComplete.connect(closeTabFunction);

                            if(tab.document.commandIsCancellable)
                                tab.document.cancelCommand();
                        }
                    }
                    else
                        tab.confirmSave(onCloseFunction);
                }

                function findTabIndex(tab)
                {
                    for(let index = 0; index < count; index++)
                    {
                        if(tabLayout.get(index) === tab)
                            return index;
                    }

                    return -1;
                }

                function findAndActivateTab(url)
                {
                    for(let index = 0; index < count; index++)
                    {
                        let tab = tabLayout.get(index);
                        if(tab.url === url)
                        {
                            currentIndex = index;
                            return true;
                        }
                    }

                    return false;
                }
            }

            Rectangle
            {
                Layout.fillWidth: true
                visible: tabBar.count > 1

                height: 1
                color: ControlColors.mid
            }

            StackLayout
            {
                id: stackLayout

                Layout.fillWidth: true
                Layout.fillHeight: true

                currentIndex: tabBar.currentIndex

                Repeater { model: ObjectModel { id: tabLayout } }
            }
        }
    }

    signal lastTabClosed()

    footer: ToolBar
    {
        visible: !tracking.visible
        topPadding: 3
        bottomPadding: 2

        RowLayout
        {
            id: toolbarLayout
            anchors.fill: parent

            // Status
            Label
            {
                Layout.fillWidth: true
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
                textFormat: Text.PlainText
                text: currentTab ? currentTab.document.status : ""
            }

            // Hack to force the RowLayout implicitHeight to be the maximum of its children
            Item
            {
                implicitHeight:
                {
                    let maxHeight = 0;
                    for(let i = 0; i < toolbarLayout.children.length; i++)
                    {
                        let child = toolbarLayout.children[i];

                        if(child === this)
                            continue;

                        maxHeight = Math.max(maxHeight, child.height);
                    }

                    return  maxHeight;
                }
            }

            // Progress
            Label
            {
                text: currentTab && currentTab.document.significantCommandInProgress ? currentTab.document.commandVerb : ""
            }

            ProgressBar
            {
                id: progressBar
                value: currentTab && currentTab.document.commandProgress >= 0.0 ? currentTab.document.commandProgress / 100.0 : 0.0
                visible: currentTab ? currentTab.document.significantCommandInProgress : false
                indeterminate: currentTab ? currentTab.document.commandProgress < 0.0 ||
                    currentTab.document.commandProgress === 100.0 : false
            }

            Connections
            {
                target: currentTab && currentTab.document

                function onCommandSecondsRemainingChanged()
                {
                    // Only show the time remaining when it's above a threshold value
                    if(currentTab.document.commandSecondsRemaining > 10)
                        timerLabel.currentCommandVerb = currentTab.document.commandVerb;
                }

                function onCommandVerbChanged()
                {
                    // If we start a new command, reset the existing one
                    if(timerLabel.currentCommandVerb !== currentTab.document.commandVerb)
                        timerLabel.currentCommandVerb = "";
                }
            }

            Label
            {
                id: timerLabel

                property string currentCommandVerb

                visible: currentTab !== null &&
                    currentTab.document.significantCommandInProgress &&
                    currentTab.document.commandSecondsRemaining > 0 &&
                    currentCommandVerb.length > 0

                text:
                {
                    if(currentTab === null)
                        return "";

                    let minutes = Math.floor(currentTab.document.commandSecondsRemaining / 60);
                    let seconds = String(currentTab.document.commandSecondsRemaining % 60);
                    if(seconds.length < 2)
                        seconds = qsTr("0") + seconds;

                    return minutes + qsTr(":") + seconds;
                }
            }

            ToolBarButton
            {
                id: cancelButton

                implicitHeight: progressBar.implicitHeight
                implicitWidth: implicitHeight

                icon.name: "process-stop"
                text: qsTr("Cancel")

                visible: currentTab ? currentTab.document.significantCommandInProgress &&
                    currentTab.document.commandIsCancellable &&
                    !currentTab.document.commandIsCancelling : false

                onClicked: function(mouse)
                {
                    currentTab.document.cancelCommand();
                }
            }

            BusyIndicator
            {
                implicitWidth: cancelButton.implicitWidth
                implicitHeight: cancelButton.implicitHeight

                enabled: visible
                visible: currentTab ? currentTab.document.commandIsCancelling : false
            }

            DownloadProgress
            {
                id: downloadUI

                visible: application.downloadActive || waitingForOpen
                progress: application.downloadProgress

                onOpenClicked: function(url)
                {
                    mainWindow.openUrl(url, true);
                    application.resumeDownload();
                }

                onCancelClicked: { application.cancelDownload(); }
            }
        }
    }

    Hubble
    {
        title: qsTr("Resume/Pause Layout")
        alignment: Qt.AlignBottom | Qt.AlignLeft
        edges: Qt.LeftEdge | Qt.TopEdge
        target: pauseLayoutButton
        tooltipMode: true
        RowLayout
        {
            spacing: Constants.spacing
            Column
            {
                ToolBarButton { icon.name: "media-playback-start" }
                ToolBarButton { icon.name: "media-playback-stop" }
                ToolBarButton { icon.name: "media-playback-pause" }
            }
            Text
            {
                Layout.preferredWidth: 500
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                text: qsTr("The graph layout system can be resumed or paused from here.<br>" +
                      "The layout system uses a <b>force-directed</b> model to position nodes. " +
                      "This improves the graph's visual navigability.<br><br>" +
                      "The process will automatically stop when it converges on a stable layout.");
            }
        }
    }

    function alertWhenCommandComplete()
    {
        alert(0);
    }

    onActiveChanged:
    {
        if(!currentTab)
            return;

        // Notify the user that a command is complete when the window isn't active
        if(active)
            currentTab.commandComplete.disconnect(alertWhenCommandComplete);
        else if(currentTab.document.significantCommandInProgress)
            currentTab.commandComplete.connect(alertWhenCommandComplete);
    }
}
