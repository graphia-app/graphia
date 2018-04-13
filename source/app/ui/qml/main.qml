import QtQml 2.8
import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.2

import Qt.labs.platform 1.0 as Labs

import com.kajeka 1.0
import "../../../shared/ui/qml/Utils.js" as Utils

import "Loading"
import "Options"
import "Controls"
import "Enrichment"

ApplicationWindow
{
    id: mainWindow
    visible: false
    property var recentFiles
    property bool debugMenuUnhidden: false
    width: 1024
    height: 768
    minimumWidth: mainToolBar.visible ? mainToolBar.implicitWidth : 640
    minimumHeight: 480
    property bool maximised: mainWindow.visibility === Window.Maximized

    property DocumentUI currentDocument: tabView.currentIndex < tabView.count ?
                                         tabView.getTab(tabView.currentIndex).item : null

    title:
    {
        var text = "";
        if(currentDocument !== null && currentDocument.title.length > 0)
            text += currentDocument.title + qsTr(" - ");

        text += application.name;

        return text;
    }

    property bool _authenticatedAtLeastOnce: false

    Application { id: application }

    // Use Connections to avoid an M16 JS lint error
    Connections
    {
        target: application

        onAuthenticatedChanged:
        {
            if(application.authenticated && !_authenticatedAtLeastOnce)
            {
                _authenticatedAtLeastOnce = true;
                processOnePendingArgument();
            }
        }

        onAuthenticatingChanged:
        {
            if(!application.authenticating)
                authUI.enabled = true;
        }
    }

    Auth
    {
        id: authUI

        visible: !application.authenticated && enabled
        enabled: false
        anchors.fill: parent

        message: application.authenticationMessage
        busy: application.authenticating

        onSignIn:
        {
            application.authenticate(email, password);
        }
    }

    property var _pendingArguments: []

    // This is called with the arguments of a second instance of the app,
    // when it starts then immediately exits
    function processArguments(arguments)
    {
        _pendingArguments = arguments.slice(1);
        processOnePendingArgument();
    }

    function processOnePendingArgument()
    {
        if(_pendingArguments.length === 0)
            return;

        // Pop
        var argument = _pendingArguments[0];
        _pendingArguments.shift();

        var url = QmlUtils.urlForUserInput(argument);
        openFile(url, true);
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
            var rightEdge = mainWindow.x + mainWindow.width;
            var bottomEdge = mainWindow.y + mainWindow.height;

            if(mainWindow.x < 0)
                mainWindow.x = 0;
            else if(rightEdge > Screen.desktopAvailableWidth)
                mainWindow.x -= (rightEdge - Screen.desktopAvailableWidth);

            if(mainWindow.y < 0)
                mainWindow.y = 0;
            else if(bottomEdge > Screen.desktopAvailableHeight)
                mainWindow.y -= (bottomEdge - Screen.desktopAvailableHeight);
        }

        if(windowPreferences.maximised !== undefined)
        {
            mainWindow.visibility = Utils.castToBool(windowPreferences.maximised) ?
                Window.Maximized : Window.Windowed;
        }

        // Arguments minus the executable
        _pendingArguments = Qt.application.arguments.slice(1);

        if(!application.tryToAuthenticateWithCachedCredentials())
        {
            // If we failed immediately, show the authentication UI
            authUI.enabled = true;
        }

        mainMenuBar.updateVisibility();
        mainWindow.visible = true;

        if(!misc.hasSeenTutorial)
        {
            var exampleFile = application.resourceFile("examples/Tutorial.graphia");

            if(QmlUtils.fileExists(exampleFile))
            {
                // Add it to the pending arguments, in case we're in the middle of authenticating
                _pendingArguments.push(exampleFile);
            }
        }
    }

    onClosing:
    {
        if(tabView.count > 0)
        {
            // If any tabs are open, close the first one and cancel the window close...
            tabView.closeTab(0, function()
            {
                // ...then (recursively) resume closing if the user doesn't cancel
                tabView.removeTab(0);
                mainWindow.close();
            });

            close.accepted = false;
            return;
        }

        windowPreferences.maximised = mainWindow.maximised;

        if(!mainWindow.maximised)
        {
            windowPreferences.width = mainWindow.width;
            windowPreferences.height = mainWindow.height;
            windowPreferences.x = mainWindow.x;
            windowPreferences.y = mainWindow.y;
        }
    }

    MessageDialog
    {
        id: aboutMessageDialog
        icon: StandardIcon.Information
        title: qsTr("About ") + application.name
        text: application.name + qsTr("\n\n") +
              application.name + qsTr(" version ") + application.version +
              qsTr(" is a tool for the visualisation and analysis of graphs.\n\n") +
              application.copyright
    }

    MessageDialog
    {
        id: errorOpeningFileMessageDialog
        icon: StandardIcon.Critical
        title: qsTr("Error Opening File")

        onAccepted:
        {
            // Even if a file failed to load, there may be more to process
            processOnePendingArgument();
        }
    }

    OptionsDialog
    {
        id: optionsDialog
    }

    AboutPluginsDialog
    {
        id: aboutpluginsDialog
        pluginDetails: application.pluginDetails

        onHiddenSwitchActivated: { mainWindow.debugMenuUnhidden = true; }
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
        property alias showLayoutSettings: toggleLayoutSettingsAction.checked

        property var fileOpenInitialFolder
        property string recentFiles
        property bool hasSeenTutorial
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property int edgeVisualType:
        {
            return toggleEdgeDirectionAction.checked ? EdgeVisualType.Arrow
                                                     : EdgeVisualType.Cylinder;
        }
        property int showNodeText:
        {
            switch(nodeTextDisplay.current)
            {
            default:
            case hideNodeTextAction:         return TextState.Off;
            case showSelectedNodeTextAction: return TextState.Selected;
            case showAllNodeTextAction:      return TextState.All;
            }
        }
        property int showEdgeText:
        {
            switch(edgeTextDisplay.current)
            {
            default:
            case hideEdgeTextAction:         return TextState.Off;
            case showSelectedEdgeTextAction: return TextState.Selected;
            case showAllEdgeTextAction:      return TextState.All;
            }
        }
        property alias showMultiElementIndicators: toggleMultiElementIndicatorsAction.checked
    }

    Preferences
    {
        section: "debug"
        property alias showFpsMeter: toggleFpsMeterAction.checked
        property alias saveGlyphMaps: toggleGlyphmapSaveAction.checked
    }

    function addToRecentFiles(fileUrl)
    {
        var fileUrlString = fileUrl.toString();

        if(mainWindow.recentFiles === undefined)
            mainWindow.recentFiles = [];

        var localRecentFiles = mainWindow.recentFiles;

        // Remove any duplicates
        for(var i = 0; i < localRecentFiles.length; i++)
        {
            if(localRecentFiles[i] === fileUrlString)
            {
                localRecentFiles.splice(i, 1);
                break;
            }
        }

        // Add to the top
        localRecentFiles.unshift(fileUrlString);

        var MAX_RECENT_FILES = 10;
        while(localRecentFiles.length > MAX_RECENT_FILES)
            localRecentFiles.pop();

        mainWindow.recentFiles = localRecentFiles;
        misc.recentFiles = JSON.stringify(localRecentFiles);
    }

    function openFile(fileUrl, inNewTab)
    {
        fileUrl = fileUrl.toString().trim();

        if(!QmlUtils.fileUrlExists(fileUrl))
        {
            errorOpeningFileMessageDialog.title = qsTr("File Not Found");
            errorOpeningFileMessageDialog.text = QmlUtils.baseFileNameForUrl(fileUrl) +
                    qsTr(" does not exist.");
            errorOpeningFileMessageDialog.open();
            return;
        }

        var fileTypes = application.urlTypesOf(fileUrl);

        if(fileTypes.length === 0)
        {
            errorOpeningFileMessageDialog.title = qsTr("Unknown File Type");
            errorOpeningFileMessageDialog.text = QmlUtils.baseFileNameForUrl(fileUrl) +
                    qsTr(" cannot be loaded as its file type is unknown.");
            errorOpeningFileMessageDialog.open();
            return;
        }

        if(!application.canOpenAnyOf(fileTypes))
        {
            errorOpeningFileMessageDialog.title = qsTr("Can't Open File");
            errorOpeningFileMessageDialog.text = QmlUtils.baseFileNameForUrl(fileUrl) +
                    qsTr(" cannot be loaded."); //FIXME more elaborate error message
            errorOpeningFileMessageDialog.open();
            return;
        }

        if(fileTypes.length > 1)
        {
            fileTypeChooserDialog.fileUrl = fileUrl
            fileTypeChooserDialog.fileTypes = fileTypes;
            fileTypeChooserDialog.inNewTab = inNewTab;
            fileTypeChooserDialog.open();
        }
        else
            openFileOfType(fileUrl, fileTypes[0], inNewTab);
    }

    FileTypeChooserDialog
    {
        id: fileTypeChooserDialog
        application: application
        model: application.urlTypeDetails
        onAccepted: openFileOfType(fileUrl, fileType, inNewTab)
    }

    function openFileOfType(fileUrl, fileType, inNewTab)
    {
        var onSaveConfirmed = function()
        {
            var pluginNames = application.pluginNames(fileType);

            if(pluginNames.length > 1)
            {
                pluginChooserDialog.fileUrl = fileUrl
                pluginChooserDialog.fileType = fileType;
                pluginChooserDialog.pluginNames = pluginNames;
                pluginChooserDialog.inNewTab = inNewTab;
                pluginChooserDialog.open();
            }
            else
                openFileOfTypeWithPlugin(fileUrl, fileType, pluginNames[0], inNewTab);
        };

        if(currentDocument !== null && !inNewTab)
            currentDocument.confirmSave(onSaveConfirmed);
        else
            onSaveConfirmed();
    }

    PluginChooserDialog
    {
        id: pluginChooserDialog
        application: application
        model: application.pluginDetails
        onAccepted: openFileOfTypeWithPlugin(fileUrl, fileType, pluginName, inNewTab)
    }

    function openFileOfTypeWithPlugin(fileUrl, fileType, pluginName, inNewTab)
    {
        var parametersQmlPath = application.parametersQmlPathForPlugin(pluginName);

        if(parametersQmlPath.length > 0)
        {
            var component = Qt.createComponent(parametersQmlPath);
            if(component.status !== Component.Ready)
            {
                console.log(component.errorString());
                return;
            }

            var contentObject = component.createObject(this);
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

            contentObject.fileUrl = fileUrl
            contentObject.fileType = fileType;
            contentObject.pluginName = pluginName;
            contentObject.inNewTab = inNewTab;

            contentObject.accepted.connect(function()
            {
                openFileOfTypeWithPluginAndParameters(contentObject.fileUrl,
                    contentObject.fileType, contentObject.pluginName,
                    contentObject.parameters, contentObject.inNewTab);
            });

            contentObject.show();
        }
        else
            openFileOfTypeWithPluginAndParameters(fileUrl, fileType, pluginName, {}, inNewTab);
    }

    function isValidParameterDialog(element)
    {
        if (element['parameters'] === undefined ||
            element['fileUrl'] === undefined ||
            element['fileType'] === undefined ||
            element['pluginName'] === undefined ||
            element['inNewTab'] === undefined ||
            element['show'] === undefined ||
            element['accepted'] === undefined)
        {
            return false;
        }

        return true;
    }

    function openFileOfTypeWithPluginAndParameters(fileUrl, fileType, pluginName, parameters, inNewTab)
    {
        var openInCurrentTab = function()
        {
            tabView.openInCurrentTab(fileUrl, fileType, pluginName, parameters);
        };

        if(currentDocument != null && !inNewTab)
            tabView.replaceTab(openInCurrentTab);
        else
            tabView.createTab(openInCurrentTab);
    }

    Labs.FileDialog
    {
        id: fileOpenDialog
        nameFilters: application.nameFilters
        onAccepted:
        {
            misc.fileOpenInitialFolder = folder.toString();
            openFile(file, inTab);
        }

        property bool inTab: false
    }

    Action
    {
        id: fileOpenAction
        iconName: "document-open"
        text: qsTr("&Open…")
        shortcut: "Ctrl+O"
        onTriggered:
        {
            fileOpenDialog.title = qsTr("Open File…");
            fileOpenDialog.inTab = false;

            if(misc.fileOpenInitialFolder !== undefined)
                fileOpenDialog.folder = misc.fileOpenInitialFolder;

            fileOpenDialog.open();
        }
    }

    Action
    {
        id: fileOpenInTabAction
        iconName: "tab-new"
        text: qsTr("Open In New &Tab…")
        shortcut: "Ctrl+T"
        onTriggered:
        {
            fileOpenDialog.title = qsTr("Open File In New Tab…");
            fileOpenDialog.inTab = true;

            if(misc.fileOpenInitialFolder !== undefined)
                fileOpenDialog.folder = misc.fileOpenInitialFolder;

            fileOpenDialog.open();
        }
    }

    Action
    {
        id: fileSaveAction
        iconName: "document-save"
        text: qsTr("&Save")
        shortcut: "Ctrl+S"
        enabled: currentDocument
        onTriggered:
        {
            if(currentDocument === null)
                return;

            currentDocument.saveFile();
        }
    }

    Action
    {
        id: fileSaveAsAction
        iconName: "document-save-as"
        text: qsTr("&Save As…")
        enabled: currentDocument
        onTriggered:
        {
            if(currentDocument === null)
                return;

            currentDocument.saveAsFile();
        }
    }

    Action
    {
        id: closeTabAction
        iconName: "window-close"
        text: qsTr("&Close Tab")
        shortcut: "Ctrl+W"
        enabled: currentDocument
        onTriggered:
        {
            // If we're currently busy, cancel and wait before closing
            if(currentDocument.commandInProgress)
            {
                // If a load is cancelled the tab is closed automatically,
                // and there is no command involved anyway, so in that case we
                // don't need to wait for the command to complete
                if(!currentDocument.loading)
                {
                    // Capture the document by value so we can use it to work out
                    // which tab to close once the command is complete
                    var closeTabFunction = function(document)
                    {
                        return function()
                        {
                            document.commandComplete.disconnect(closeTabFunction);
                            tabView.closeTab(tabView.findTabIndex(document));
                        };
                    }(currentDocument);

                    currentDocument.commandComplete.connect(closeTabFunction);
                }

                if(currentDocument.commandIsCancellable)
                    currentDocument.cancelCommand();
            }
            else
                tabView.closeTab(tabView.currentIndex);
        }
    }

    Action
    {
        id: closeAllTabsAction
        iconName: "window-close"
        text: qsTr("Close &All Tabs")
        shortcut: "Ctrl+Shift+W"
        enabled: currentDocument
        onTriggered:
        {
            if(tabView.count > 0)
            {
                // If any tabs are open, close the first one...
                tabView.closeTab(0, function()
                {
                    // ...then (recursively) resume closing if the user doesn't cancel
                    tabView.removeTab(0);
                    closeAllTabsAction.trigger();
                });
            }
        }
    }

    Action
    {
        id: quitAction
        iconName: "application-exit"
        text: qsTr("&Quit")
        shortcut: "Ctrl+Q"
        onTriggered: { mainWindow.close(); }
    }

    Action
    {
        id: undoAction
        iconName: "edit-undo"
        text: currentDocument ? currentDocument.nextUndoAction : qsTr("&Undo")
        shortcut: "Ctrl+Z"
        enabled: currentDocument ? currentDocument.canUndo : false
        onTriggered: currentDocument && currentDocument.undo()
    }

    Action
    {
        id: redoAction
        iconName: "edit-redo"
        text: currentDocument ? currentDocument.nextRedoAction : qsTr("&Redo")
        shortcut: "Ctrl+Shift+Z"
        enabled: currentDocument ? currentDocument.canRedo : false
        onTriggered: currentDocument && currentDocument.redo()
    }

    Action
    {
        id: deleteAction
        iconName: "edit-delete"
        text: qsTr("&Delete Selection")
        shortcut: "Del"
        enabled: currentDocument ? currentDocument.canDeleteSelection : false
        onTriggered: currentDocument.deleteSelectedNodes()
    }

    Action
    {
        id: selectAllAction
        iconName: "edit-select-all"
        text: qsTr("Select &All")
        shortcut: "Ctrl+Shift+A"
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered: currentDocument && currentDocument.selectAll()
    }

    Action
    {
        id: selectAllVisibleAction
        iconName: "edit-select-all"
        text: qsTr("Select All &Visible")
        shortcut: "Ctrl+A"
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered: currentDocument && currentDocument.selectAllVisible()
    }

    Action
    {
        id: selectNoneAction
        text: qsTr("Select &None")
        shortcut: "Ctrl+N"
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered: currentDocument && currentDocument.selectNone()
    }

    Action
    {
        id: selectSourcesAction
        text: qsTr("Select Sources of Selection")
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered: currentDocument && currentDocument.selectSources()
    }

    Action
    {
        id: selectTargetsAction
        text: qsTr("Select Targets of Selection")
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered: currentDocument && currentDocument.selectTargets()
    }

    Action
    {
        id: selectNeighboursAction
        text: qsTr("Select Neigh&bours of Selection")
        shortcut: "Ctrl+B"
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered: currentDocument && currentDocument.selectNeighbours()
    }

    Action
    {
        id: invertSelectionAction
        text: qsTr("&Invert Selection")
        shortcut: "Ctrl+I"
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered: currentDocument && currentDocument.invertSelection()
    }

    Action
    {
        id: findAction
        iconName: "edit-find"
        text: qsTr("&Find")
        shortcut: "Ctrl+F"
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered:
        {
            if(currentDocument)
                currentDocument.showFind(Find.Simple);
        }
    }

    Action
    {
        id: advancedFindAction
        iconName: "edit-find"
        text: qsTr("Advanced Find")
        shortcut: "Ctrl+Shift+F"
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered:
        {
            if(currentDocument)
                currentDocument.showFind(Find.Advanced);
        }
    }

    Action
    {
        id: findByAttributeAction
        iconName: "format-indent-more"
        text: qsTr("Find By Attribute Value")
        shortcut: "Ctrl+H"
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered:
        {
            if(currentDocument)
                currentDocument.showFind(Find.ByAttribute);
        }
    }

    Action
    {
        id: prevComponentAction
        text: qsTr("Goto &Previous Component")
        shortcut: "PgUp"
        enabled: currentDocument ? currentDocument.canChangeComponent : false
        onTriggered: currentDocument && currentDocument.gotoPrevComponent()
    }

    Action
    {
        id: nextComponentAction
        text: qsTr("Goto &Next Component")
        shortcut: "PgDown"
        enabled: currentDocument ? currentDocument.canChangeComponent : false
        onTriggered: currentDocument && currentDocument.gotoNextComponent()
    }

    Action
    {
        id: optionsAction
        iconName: "applications-system"
        text: qsTr("&Options…")
        onTriggered:
        {
            optionsDialog.raise();
            optionsDialog.show();
        }
    }

    Action
    {
        id: enrichmentAction
        text: qsTr("Enrichment")
        onTriggered:
        {
            if(currentDocument != undefined)
            {
                if(enrichmentResults.models.size() > 0)
                    enrichmentResults.show();
                else
                    enrichmentWizard.show();
            }
        }
    }

    Action
    {
        id: pauseLayoutAction
        iconName:
        {
            var layoutPauseState = currentDocument ? currentDocument.layoutPauseState : -1;

            switch(layoutPauseState)
            {
            case LayoutPauseState.Paused:          return "media-playback-start";
            case LayoutPauseState.RunningFinished: return "media-playback-stop";
            default:
            case LayoutPauseState.Running:         return "media-playback-pause";
            }
        }

        text: currentDocument && currentDocument.layoutPauseState === LayoutPauseState.Paused ?
                  qsTr("&Resume Layout") : qsTr("&Pause Layout")
        shortcut: "Pause"
        enabled: currentDocument ? !currentDocument.busy : false
        onTriggered: currentDocument && currentDocument.toggleLayout()
    }

    Action
    {
        id: toggleLayoutSettingsAction
        text: qsTr("Show Layout Settings")
        checkable: true
    }

    Action
    {
        id: overviewModeAction
        iconName: "view-fullscreen"
        text: qsTr("&Overview Mode")
        shortcut: currentDocument && currentDocument.findVisible ? "" : "Esc"
        enabled: currentDocument ? currentDocument.canEnterOverviewMode : false
        onTriggered: currentDocument && currentDocument.switchToOverviewMode()
    }

    Action
    {
        id: resetViewAction
        iconName: "view-refresh"
        text: qsTr("&Reset View")
        shortcut: currentDocument && (currentDocument.findVisible || currentDocument.canEnterOverviewMode) ? "" : "Esc"
        enabled: currentDocument ? currentDocument.canResetView : false
        onTriggered: currentDocument && currentDocument.resetView()
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
        iconName: "list-add"
        text: qsTr("Add Bookmark…")
        shortcut: "Ctrl+D"
        enabled: currentDocument ? !currentDocument.busy && currentDocument.numNodesSelected > 0 : false
        onTriggered:
        {
            if(currentDocument !== null)
                currentDocument.showAddBookmark();
        }
    }

    ManageBookmarks
    {
        id: manageBookmarks
        document: currentDocument
    }

    Action
    {
        id: manageBookmarksAction
        text: qsTr("Manage Bookmarks…")
        enabled: currentDocument ? !currentDocument.busy && currentDocument.bookmarks.length > 0 : false
        onTriggered:
        {
            manageBookmarks.raise();
            manageBookmarks.show();
        }
    }

    ExclusiveGroup
    {
        id: nodeTextDisplay

        Action { id: hideNodeTextAction; text: qsTr("None"); checkable: true; }
        Action { id: showSelectedNodeTextAction; text: qsTr("Selected"); checkable: true; }
        Action { id: showAllNodeTextAction; text: qsTr("All"); checkable: true; }

        Component.onCompleted:
        {
            switch(visuals.showNodeText)
            {
            default:
            case TextState.Off:      nodeTextDisplay.current = hideNodeTextAction; break;
            case TextState.Selected: nodeTextDisplay.current = showSelectedNodeTextAction; break;
            case TextState.All:      nodeTextDisplay.current = showAllNodeTextAction; break;
            }
        }
    }

    ExclusiveGroup
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
            case TextState.Off:      edgeTextDisplay.current = hideEdgeTextAction; break;
            case TextState.Selected: edgeTextDisplay.current = showSelectedEdgeTextAction; break;
            case TextState.All:      edgeTextDisplay.current = showAllEdgeTextAction; break;
            }
        }
    }

    Action
    {
        id: toggleMultiElementIndicatorsAction
        text: qsTr("Show Multi-Element Indicators")
        checkable: true
    }

    Action
    {
        id: dumpGraphAction
        text: qsTr("Dump graph to qDebug")
        enabled: application.debugEnabled
        onTriggered: currentDocument && currentDocument.dumpGraph()
    }

    Action
    {
        id: reportScopeTimersAction
        text: qsTr("Report Scope Timers")
        onTriggered: { application.reportScopeTimers(); }
    }


    Action
    {
        id: saveImageAction
        iconName: "camera-photo"
        text: qsTr("Save As Image…")
        enabled: currentDocument
        onTriggered: currentDocument && currentDocument.screenshot()
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
        id: togglePluginWindowAction
        iconName: "preferences-system-windows"
        text: currentDocument ? qsTr("Display " + currentDocument.pluginName + " In Separate &Window") : ""
        checkable: true
        checked: currentDocument && currentDocument.pluginPoppedOut
        enabled: currentDocument && currentDocument.hasPluginUI
        onTriggered: currentDocument && currentDocument.togglePop()
    }

    Action
    {
        id: aboutPluginsAction
        // Don't ask...
        text: Qt.platform.os === "osx" ? qsTr("Plugins…") : qsTr("About Plugins…")
        onTriggered:
        {
            aboutpluginsDialog.raise();
            aboutpluginsDialog.show();
        }
    }

    Action
    {
        id: signOutAction
        text: qsTr("&Sign Out")
        onTriggered:
        {
            if(tabView.count === 0)
            {
                application.signOut();
                return;
            }

            mainWindow.lastDocumentClosed.connect(function()
            {
                //FIXME if any file closes are cancelled, we shouldn't proceed
                signOut();
            });

            closeAllTabsAction.trigger();
        }

        function signOut()
        {
            mainWindow.lastDocumentClosed.disconnect(signOut);
            application.signOut();
        }
    }

    Action
    {
        id: copyImageToClipboardAction
        text: qsTr("Copy Viewport To Clipboard")
        shortcut: "Ctrl+C"
        enabled: currentDocument
        onTriggered:
        {
            if(currentDocument)
                currentDocument.copyImageToClipboard();
        }
    }

    Action
    {
        // A do nothing action that we use when there
        // is no other valid action available
        id: nullAction
    }

    // Hack to hide the menu bar when we're not authenticated
    Connections
    {
        target: application

        onAuthenticatedChanged:
        {
            mainMenuBar.updateVisibility();
        }
    }

    menuBar: MenuBar
    {
        id: mainMenuBar

        function updateVisibility()
        {
            if(application.authenticated)
                mainWindow.menuBar = mainMenuBar;
            else
            {
                mainWindow.menuBar = null;
                __contentItem.parent = null;
            }
        }

        Menu
        {
            title: qsTr("&File")
            MenuItem { action: fileOpenAction }
            MenuItem { action: fileOpenInTabAction }
            Menu
            {
                id: recentFileMenu;
                title: qsTr("&Recent Files")

                Instantiator
                {
                    model: mainWindow.recentFiles
                    delegate: Component
                    {
                        MenuItem
                        {
                            // FIXME: This fires with a -1 index onOpenFile
                            // BUG: Text overflows MenuItems on Windows
                            // https://bugreports.qt.io/browse/QTBUG-50849
                            text: index > -1 ? QmlUtils.fileNameForUrl(mainWindow.recentFiles[index]) : "";
                            onTriggered:
                            {
                                openFile(QmlUtils.urlForFileName(text), true);
                            }
                        }
                    }
                    onObjectAdded: recentFileMenu.insertItem(index, object)
                    onObjectRemoved: recentFileMenu.removeItem(object)
                }
            }
            MenuSeparator {}
            MenuItem { action: fileSaveAction }
            MenuItem { action: fileSaveAsAction }
            MenuItem { action: saveImageAction }
            MenuSeparator {}
            MenuItem { action: closeTabAction }
            MenuItem { action: closeAllTabsAction }
            MenuSeparator {}
            MenuItem { action: quitAction }
        }
        Menu
        {
            title: qsTr("&Edit")
            MenuItem { action: undoAction }
            MenuItem { action: redoAction }
            MenuSeparator {}
            MenuItem { action: deleteAction }
            MenuSeparator {}
            MenuItem { action: selectAllAction }
            MenuItem { action: selectAllVisibleAction }
            MenuItem { action: selectNoneAction }
            MenuItem { action: invertSelectionAction }
            MenuItem { action: selectSourcesAction }
            MenuItem { action: selectTargetsAction }
            MenuItem { action: selectNeighboursAction }
            MenuSeparator {}
            MenuItem { action: findAction }
            MenuItem { action: advancedFindAction }
            MenuItem { action: findByAttributeAction }
            MenuItem
            {
                action: currentDocument ? currentDocument.previousAction : nullAction
                visible: currentDocument
            }
            MenuItem
            {
                action: currentDocument ? currentDocument.nextAction : nullAction
                visible: currentDocument
            }
            MenuSeparator {}
            MenuItem { action: prevComponentAction }
            MenuItem { action: nextComponentAction }
            MenuSeparator {}
            MenuItem { action: optionsAction }
        }
        Menu
        {
            title: qsTr("&View")
            MenuItem { action: overviewModeAction }
            MenuItem { action: resetViewAction }
            MenuItem
            {
                action: togglePluginWindowAction
                visible: currentDocument && currentDocument.hasPluginUI
            }
            MenuSeparator {}
            MenuItem { action: toggleGraphMetricsAction }
            Menu
            {
                title: qsTr("Show Node Text")
                MenuItem { action: hideNodeTextAction }
                MenuItem { action: showSelectedNodeTextAction }
                MenuItem { action: showAllNodeTextAction }
            }
            Menu
            {
                title: qsTr("Show Edge Text")
                MenuItem { action: hideEdgeTextAction }
                MenuItem { action: showSelectedEdgeTextAction }
                MenuItem { action: showAllEdgeTextAction }
            }
            MenuItem { action: toggleEdgeDirectionAction }
            MenuItem { action: toggleMultiElementIndicatorsAction }
            MenuSeparator {}
            MenuItem { action: copyImageToClipboardAction }
        }
        Menu
        {
            title: qsTr("&Layout")
            MenuItem { action: pauseLayoutAction }
            MenuItem { action: toggleLayoutSettingsAction }
        }
        Menu
        {
            title: qsTr("&Analyses")
            enabled: currentDocument != undefined
            MenuItem { action: enrichmentAction }
        }
        Menu
        {
            id: bookmarksMenu

            title: qsTr("&Bookmarks")
            MenuItem { action: addBookmarkAction }
            MenuItem { action: manageBookmarksAction }
            MenuSeparator { visible: currentDocument ? currentDocument.bookmarks.length > 0 : false }

            Instantiator
            {
                model: currentDocument ? currentDocument.bookmarks : []
                delegate: Component
                {
                    MenuItem
                    {
                        text: index > -1 ? currentDocument.bookmarks[index] : "";
                        shortcut:
                        {
                            if(index >= 0 && index < 10)
                                return "Ctrl+" + (index + 1);
                            else if(index == 10)
                                return "Ctrl+0";

                            return "";
                        }

                        enabled: currentDocument ? !currentDocument.busy : false
                        onTriggered:
                        {
                            currentDocument.gotoBookmark(text);
                        }
                    }
                }
                onObjectAdded: bookmarksMenu.insertItem(index, object)
                onObjectRemoved: bookmarksMenu.removeItem(object)
            }
        }
        Menu { id: pluginMenu0; visible: false }
        Menu { id: pluginMenu1; visible: false }
        Menu { id: pluginMenu2; visible: false }
        Menu { id: pluginMenu3; visible: false }
        Menu { id: pluginMenu4; visible: false }
        Menu
        {
            title: qsTr("&Debug")
            visible: application.debugEnabled || mainWindow.debugMenuUnhidden
            Menu
            {
                title: qsTr("&Crash")
                MenuItem
                {
                    text: qsTr("Null Pointer Deference");
                    onTriggered: application.crash(CrashType.NullPtrDereference);
                }
                MenuItem
                {
                    text: qsTr("C++ Exception");
                    onTriggered: application.crash(CrashType.CppException);
                }
                MenuItem
                {
                    text: qsTr("Fatal Error");
                    onTriggered: application.crash(CrashType.FatalError);
                }
                MenuItem
                {
                    text: qsTr("Infinite Loop");
                    onTriggered: application.crash(CrashType.InfiniteLoop);
                }
                MenuItem
                {
                    text: qsTr("Deadlock");
                    onTriggered: application.crash(CrashType.Deadlock);
                }
                MenuItem
                {
                    text: qsTr("Hitch");
                    onTriggered: application.crash(CrashType.Hitch);
                }
                MenuItem
                {
                    visible: Qt.platform.os === "windows"
                    text: qsTr("Windows Exception");
                    onTriggered: application.crash(CrashType.Win32Exception);
                }
                MenuItem
                {
                    visible: Qt.platform.os === "windows"
                    text: qsTr("Windows Exception Non-Continuable");
                    onTriggered: application.crash(CrashType.Win32ExceptionNonContinuable);
                }
            }
            MenuItem { action: dumpGraphAction }
            MenuItem { action: toggleFpsMeterAction }
            MenuItem { action: toggleGlyphmapSaveAction }
            MenuItem { action: reportScopeTimersAction }
        }
        Menu
        {
            title: qsTr("&Help")
            MenuItem { action: aboutPluginsAction }

            MenuItem
            {
                text: qsTr("About " + application.name + "…")
                onTriggered:
                {
                    aboutMessageDialog.open();
                }
            }

            MenuItem
            {
                text: qsTr("Show Tutorial…")
                onTriggered:
                {
                    var exampleFileUrl = QmlUtils.urlForFileName(application.resourceFile(
                        "examples/Tutorial.graphia"));

                    if(QmlUtils.fileUrlExists(exampleFileUrl))
                    {
                        var tutorialAlreadyOpen = tabView.findAndActivateTab(exampleFileUrl);
                        openFile(exampleFileUrl, !tutorialAlreadyOpen);
                    }
                }
            }

            MenuSeparator {}
            MenuItem { action: signOutAction }
        }
    }

    function clearMenu(menu)
    {
        menu.visible = false;
        while(menu.items.length > 0)
            menu.removeItem(menu.items[0]);
    }

    function clearMenus()
    {
        if(currentDocument !== null)
        {
            clearMenu(currentDocument.pluginMenu0);
            clearMenu(currentDocument.pluginMenu1);
            clearMenu(currentDocument.pluginMenu2);
            clearMenu(currentDocument.pluginMenu3);
            clearMenu(currentDocument.pluginMenu4);
        }

        clearMenu(pluginMenu0);
        clearMenu(pluginMenu1);
        clearMenu(pluginMenu2);
        clearMenu(pluginMenu3);
        clearMenu(pluginMenu4);
    }

    function updatePluginMenu(index, menu)
    {
        clearMenu(menu);

        if(currentDocument !== null)
        {
            if(currentDocument.createPluginMenu(index, menu))
                menu.visible = true;
        }
    }

    function updatePluginMenus()
    {
        clearMenus();

        if(currentDocument !== null && currentDocument.pluginPoppedOut)
        {
            updatePluginMenu(0, currentDocument.pluginMenu0);
            updatePluginMenu(1, currentDocument.pluginMenu1);
            updatePluginMenu(2, currentDocument.pluginMenu2);
            updatePluginMenu(3, currentDocument.pluginMenu3);
            updatePluginMenu(4, currentDocument.pluginMenu4);
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

    onCurrentDocumentChanged:
    {
        updatePluginMenus();
        if(currentDocument != null)
            enrichmentResults.models = currentDocument.enrichmentTableModels;
    }

    EnrichmentResults
    {
        id: enrichmentResults
        wizard: enrichmentWizard
        models: currentDocument ? currentDocument.enrichmentTableModels : []
    }

    EnrichmentWizard
    {
        id: enrichmentWizard
        attributeGroups: currentDocument ? currentDocument.attributeGroupNames : []
        onAccepted:
        {
            if(currentDocument != null)
                currentDocument.performEnrichment(selectedAttributeGroupA, selectedAttributeGroupB)
            enrichmentWizard.reset();
        }
    }

    Connections
    {
        target: currentDocument
        onPluginLoadComplete: updatePluginMenus();
        onPluginPoppedOutChanged: updatePluginMenus();
        onEnrichmentTableModelsChanged:
        {
            enrichmentResults.models = currentDocument.enrichmentTableModels;
        }
        onEnrichmentAnalysisComplete: { enrichmentResults.visible = true; }
    }

    toolBar: ToolBar
    {
        id: mainToolBar

        visible: application.authenticated

        RowLayout
        {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            ToolButton { action: fileOpenAction }
            ToolButton { action: fileOpenInTabAction }
            ToolButton { action: fileSaveAction }
            ToolBarSeparator {}
            ToolButton
            {
                id: pauseLayoutButton
                action: pauseLayoutAction
                tooltip: ""
            }
            ToolBarSeparator {}
            ToolButton { action: deleteAction }
            ToolButton { action: findAction }
            ToolButton { action: undoAction }
            ToolButton { action: redoAction }
            ToolBarSeparator {}
            ToolButton { action: resetViewAction }
            ToolButton { action: optionsAction }
        }
    }

    DropArea
    {
        anchors.fill: parent
        onDropped:
        {
            if(drop.text.length > 0)
                openFile(drop.text, true)
        }

        TabView
        {
            id: tabView

            visible: application.authenticated

            anchors.fill: parent
            tabsVisible: count > 1
            frameVisible: count > 1

            onCountChanged:
            {
                if(count === 0)
                    lastDocumentClosed();
            }

            function insertTabAtIndex(index)
            {
                var tab = insertTab(index, "", tabComponent);
                tab.active = true;
                tabView.currentIndex = index;

                // Make the tab title match the document title
                tab.title = Qt.binding(function() { return tab.item.title });

                return tab;
            }

            function createTab(onCreateFunction)
            {
                var tab = insertTabAtIndex(tabView.count);

                if(typeof(onCreateFunction) !== "undefined")
                    onCreateFunction();

                return tab;
            }

            function replaceTab(onReplaceFunction)
            {
                var oldIndex = tabView.currentIndex;

                removeTab(oldIndex);
                insertTabAtIndex(oldIndex);

                if(onReplaceFunction !== "undefined")
                    onReplaceFunction();
            }

            // This is called if the file can't be opened immediately, or
            // if the load has been attempted but it failed later
            function onLoadFailure(index, fileUrl)
            {
                var document = getTab(index).item;
                var loadWasCancelled = document.commandIsCancelling;

                // Remove the tab that was created but won't be used
                removeTab(index);

                if(!loadWasCancelled)
                {
                    errorOpeningFileMessageDialog.text = QmlUtils.baseFileNameForUrl(fileUrl) +
                            qsTr(" could not be opened due to an error.");
                    errorOpeningFileMessageDialog.open();
                }
            }

            function openInCurrentTab(fileUrl, fileType, pluginName, parameters)
            {
                var document = currentDocument;
                document.application = application;
                if(!document.openFile(fileUrl, fileType, pluginName, parameters))
                    onLoadFailure(findTabIndex(document), fileUrl);
            }

            function closeTab(index, onCloseFunction)
            {
                if(index < 0 || index >= count)
                {
                    console.log("closeTab called with out of range index: " + index);
                    return false;
                }

                if(typeof(onCloseFunction) === "undefined")
                {
                    onCloseFunction = function()
                    {
                        removeTab(index);
                    }
                }

                var tab = getTab(index).item;
                tab.confirmSave(onCloseFunction);
            }

            function findTabIndex(document)
            {
                for(var index = 0; index < count; index++)
                {
                    var tab = getTab(index);
                    if(tab.item === document)
                        return index;
                }

                return -1;
            }

            function findAndActivateTab(fileUrl)
            {
                for(var index = 0; index < count; index++)
                {
                    var tab = getTab(index);
                    if(tab.item.fileUrl === fileUrl)
                    {
                        currentIndex = index;
                        return true;
                    }
                }

                return false;
            }

            Component
            {
                id: tabComponent

                DocumentUI
                {
                    id: document

                    onLoadComplete:
                    {
                        if(success)
                        {
                            addToRecentFiles(fileUrl);
                            processOnePendingArgument();

                            if(application.isResourceFileUrl(fileUrl) &&
                                QmlUtils.baseFileNameForUrlNoExtension(fileUrl) === "Tutorial")
                            {
                                // Mild hack: if it looks like the tutorial file,
                                // it probably is, so start the tutorial
                                startTutorial();
                                misc.hasSeenTutorial = true;
                            }
                        }
                        else
                            tabView.onLoadFailure(tabView.findTabIndex(document), fileUrl);
                    }
                }
            }
        }
    }

    signal lastDocumentClosed()

    statusBar: StatusBar
    {
        visible: application.authenticated

        RowLayout
        {
            id: rowLayout
            width: parent.width

            // Status
            Label
            {
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: currentDocument ? currentDocument.status : ""
            }

            // Progress
            Label
            {
                text: currentDocument && currentDocument.commandInProgress ? currentDocument.commandVerb : ""
            }

            ProgressBar
            {
                id: progressBar
                value: currentDocument && currentDocument.commandProgress >= 0.0 ? currentDocument.commandProgress / 100.0 : 0.0
                visible: currentDocument ? currentDocument.commandInProgress : false
                indeterminate: currentDocument ? currentDocument.commandProgress < 0.0 : false
            }

            Label
            {
                property string currentCommandVerb
                visible:
                {
                    if(!currentDocument)
                        return false;

                    if(!currentDocument.commandInProgress)
                        return false;

                    // Show the time remaining when it's above a threshold value
                    if(currentDocument.commandSecondsRemaining > 10)
                    {
                        currentCommandVerb = currentDocument.commandVerb;
                        return true;
                    }

                    // We've dropped below the time threshold, but we're still doing the
                    // same thing, so keep showing the timer
                    if(currentCommandVerb.length > 0 && currentCommandVerb === currentDocument.commandVerb)
                        return true;

                    currentCommandVerb = "";
                    return false;
                }

                text:
                {
                    if(!currentDocument)
                        return "";

                    var minutes = Math.floor(currentDocument.commandSecondsRemaining / 60);
                    var seconds = String(currentDocument.commandSecondsRemaining % 60);
                    if(seconds.length < 2)
                        seconds = "0" + seconds;

                    return minutes + ":" + seconds;
                }
            }

            ToolButton
            {
                id: cancelButton

                implicitHeight: progressBar.implicitHeight * 0.8
                implicitWidth: implicitHeight

                iconName: "process-stop"
                tooltip: qsTr("Cancel")

                visible: currentDocument ? currentDocument.commandIsCancellable && !currentDocument.commandIsCancelling : false
                onClicked:
                {
                    currentDocument.cancelCommand();
                }
            }

            BusyIndicator
            {
                implicitWidth: cancelButton.implicitWidth
                implicitHeight: cancelButton.implicitHeight

                id: cancelledIndicator
                visible: currentDocument ? currentDocument.commandIsCancelling : false
            }

            // Hack to force the RowLayout height to be the maximum of its children
            Rectangle { height: rowLayout.childrenRect.height }
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
            spacing: 10
            Column
            {
                ToolButton { iconName: "media-playback-start" }
                ToolButton { iconName: "media-playback-stop" }
                ToolButton { iconName: "media-playback-pause" }
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
        if(!currentDocument)
            return;

        // Notify the user that a command is complete when the window isn't active
        if(active)
            currentDocument.commandComplete.disconnect(alertWhenCommandComplete);
        else if(currentDocument.commandInProgress)
            currentDocument.commandComplete.connect(alertWhenCommandComplete);
    }
}
