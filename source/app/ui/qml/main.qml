import QtQml 2.8
import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.2

import com.kajeka 1.0
import "Utils.js" as Utils

import "Loading"
import "Options"
import "Controls"

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

    title: (currentDocument ? currentDocument.title + qsTr(" - ") : "") + application.name

    property bool _authenticatedAtLeastOnce: false

    Application
    {
        id: application

        onAuthenticatedChanged:
        {
            if(authenticated && !_authenticatedAtLeastOnce)
            {
                _authenticatedAtLeastOnce = true;
                processArguments(Qt.application.arguments);
            }
        }
    }

    Auth
    {
        visible: !application.authenticated
        enabled: visible
        anchors.fill: parent

        message: application.authenticationMessage
        busy: application.authenticating

        onSignIn:
        {
            application.authenticate(email, password);
        }
    }

    property var _pendingArguments

    // This is called when the app is started, but it also receives the arguments
    // of a second instance when it starts then immediately exits
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

        var fileUrl = application.urlForFileName(argument);
        openFile(fileUrl, true);
    }

    Component.onCompleted:
    {
        if(misc.recentFiles.length > 0)
            mainWindow.recentFiles = JSON.parse(misc.recentFiles);
        else
            mainWindow.recentFiles = [];

        if(misc.fileOpenInitialFolder !== undefined)
            fileOpenDialog.folder = misc.fileOpenInitialFolder;

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

        mainWindow.visible = true;

        application.tryToAuthenticateWithCachedCredentials();
    }

    onClosing:
    {
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

        if(!application.fileUrlExists(fileUrl))
        {
            errorOpeningFileMessageDialog.title = qsTr("File Not Found");
            errorOpeningFileMessageDialog.text = application.baseFileNameForUrl(fileUrl) +
                    qsTr(" does not exist.");
            errorOpeningFileMessageDialog.open();
            return;
        }

        var fileTypes = application.urlTypesOf(fileUrl);

        if(fileTypes.length === 0)
        {
            errorOpeningFileMessageDialog.title = qsTr("Unknown File Type");
            errorOpeningFileMessageDialog.text = application.baseFileNameForUrl(fileUrl) +
                    qsTr(" cannot be loaded as its file type is unknown.");
            errorOpeningFileMessageDialog.open();
            return;
        }

        if(!application.canOpenAnyOf(fileTypes))
        {
            errorOpeningFileMessageDialog.title = qsTr("Can't Open File");
            errorOpeningFileMessageDialog.text = application.baseFileNameForUrl(fileUrl) +
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
            openFileOfTypeWithPlugin(fileUrl, fileType, pluginNames[0], inNewTab)
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
            return false;
        return true;
    }

    function openFileOfTypeWithPluginAndParameters(fileUrl, fileType, pluginName, parameters, inNewTab)
    {
        if(currentDocument != null && !inNewTab)
            tabView.replaceTab();
        else
            tabView.createTab();

        tabView.openInCurrentTab(fileUrl, fileType, pluginName, parameters);
    }

    FileDialog
    {
        id: fileOpenDialog
        modality: Qt.WindowModal
        nameFilters: application.nameFilters
        onAccepted: openFile(fileUrl, inTab)
        onFolderChanged: misc.fileOpenInitialFolder = folder.toString();
        property bool inTab: false
    }

    Action
    {
        id: fileOpenAction
        iconName: "document-open"
        text: qsTr("&Open...")
        shortcut: "Ctrl+O"
        onTriggered:
        {
            fileOpenDialog.title = qsTr("Open File...");
            fileOpenDialog.inTab = false;
            fileOpenDialog.open()
        }
    }

    Action
    {
        id: fileOpenInTabAction
        iconName: "tab-new"
        text: qsTr("Open In New &Tab...")
        shortcut: "Ctrl+T"
        onTriggered:
        {
            fileOpenDialog.title = qsTr("Open File In New Tab...");
            fileOpenDialog.inTab = true;
            fileOpenDialog.open()
        }
    }

    Action
    {
        id: recentFileOpen
        onTriggered:
        {
            openFile(application.urlForFileName(source.text), true);
        }
    }

    Action
    {
        id: closeTabAction
        iconName: "window-close"
        text: qsTr("&Close Tab")
        shortcut: "Ctrl+W"
        enabled: currentDocument
        onTriggered: tabView.removeTab(tabView.currentIndex)
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
            while(tabView.count > 0)
                tabView.removeTab(0);
        }
    }

    Action
    {
        id: quitAction
        iconName: "application-exit"
        text: qsTr("&Quit")
        shortcut: "Ctrl+Q"
        onTriggered: Qt.quit()
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
        text: qsTr("&Delete")
        shortcut: "Del"
        enabled: currentDocument ? currentDocument.canDelete : false
        onTriggered: currentDocument.deleteSelectedNodes()
    }

    Action
    {
        id: selectAllAction
        text: qsTr("Select &All")
        shortcut: "Ctrl+A"
        enabled: currentDocument ? currentDocument.idle : false
        onTriggered: currentDocument && currentDocument.selectAll()
    }

    Action
    {
        id: selectNoneAction
        text: qsTr("Select &None")
        shortcut: "Ctrl+Shift+A"
        enabled: currentDocument ? currentDocument.idle : false
        onTriggered: currentDocument && currentDocument.selectNone()
    }

    Action
    {
        id: invertSelectionAction
        text: qsTr("&Invert Selection")
        shortcut: "Ctrl+I"
        enabled: currentDocument ? currentDocument.idle : false
        onTriggered: currentDocument && currentDocument.invertSelection()
    }

    Action
    {
        id: findAction
        text: qsTr("&Find")
        shortcut: "Ctrl+F"
        enabled: currentDocument ? currentDocument.idle : false
        onTriggered: currentDocument && currentDocument.showFind()
    }

    Action
    {
        id: prevComponentAction
        text: qsTr("Goto &Previous Component")
        shortcut: "PgUp"
        enabled: currentDocument ? currentDocument.idle : false
        onTriggered: currentDocument && currentDocument.gotoPrevComponent()
    }

    Action
    {
        id: nextComponentAction
        text: qsTr("Goto &Next Component")
        shortcut: "PgDown"
        enabled: currentDocument ? currentDocument.idle : false
        onTriggered: currentDocument && currentDocument.gotoNextComponent()
    }

    Action
    {
        id: optionsAction
        iconName: "applications-system"
        text: qsTr("&Options...")
        onTriggered:
        {
            optionsDialog.raise();
            optionsDialog.show();
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
        enabled: currentDocument ? currentDocument.idle : false
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
        shortcut: "Ctrl+D"
        enabled: application.debugEnabled
        onTriggered: currentDocument && currentDocument.dumpGraph()
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
        text: Qt.platform.os === "osx" ? qsTr("Plugins...") : qsTr("About Plugins...")
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

    MenuBar
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
            MenuItem { action: closeTabAction }
            MenuItem { action: closeAllTabsAction }
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
                            text: index > -1 ? application.fileNameForUrl(mainWindow.recentFiles[index]) : "";
                            action: recentFileOpen
                        }
                    }
                    onObjectAdded: recentFileMenu.insertItem(index, object)
                    onObjectRemoved: recentFileMenu.removeItem(object)
                }
            }
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
            MenuItem { action: selectNoneAction }
            MenuItem { action: invertSelectionAction }
            MenuSeparator {}
            MenuItem { action: findAction }
            MenuItem
            {
                action: currentDocument ? currentDocument.selectPreviousFoundAction : nullAction
                visible: currentDocument
            }
            MenuItem
            {
                action: currentDocument ? currentDocument.selectNextFoundAction : nullAction
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
        }
        Menu
        {
            title: qsTr("&Layout")
            MenuItem { action: pauseLayoutAction }
            MenuItem { action: toggleLayoutSettingsAction }
        }
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
        }
        Menu { id: pluginMenu0; visible: false }
        Menu { id: pluginMenu1; visible: false }
        Menu { id: pluginMenu2; visible: false }
        Menu { id: pluginMenu3; visible: false }
        Menu { id: pluginMenu4; visible: false }
        Menu
        {
            title: qsTr("&Help")
            MenuItem { action: aboutPluginsAction }

            MenuItem
            {
                text: qsTr("About " + application.name + "...")
                onTriggered:
                {
                    aboutMessageDialog.open();
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
    }

    Connections
    {
        target: currentDocument
        onPluginLoaded: updatePluginMenus();
        onPluginPoppedOutChanged: updatePluginMenus();
    }

    toolBar: ToolBar
    {
        id: mainToolBar

        visible: application.authenticated

        RowLayout
        {
            anchors.fill: parent
            ToolButton { action: fileOpenAction }
            ToolButton { action: fileOpenInTabAction }
            ToolButton { action: pauseLayoutAction }
            ToolButton { action: deleteAction }
            ToolButton { action: undoAction }
            ToolButton { action: redoAction }
            ToolButton { action: overviewModeAction }
            ToolButton { action: resetViewAction }
            ToolButton { action: optionsAction }

            Item { Layout.fillWidth: true }
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

            function createTab()
            {
                return insertTabAtIndex(tabView.count);
            }

            function replaceTab()
            {
                var oldIndex = tabView.currentIndex;
                removeTab(tabView.currentIndex);

                return insertTabAtIndex(oldIndex);
            }

            function openInCurrentTab(fileUrl, fileType, pluginName, parameters)
            {
                currentDocument.application = application;
                if(!currentDocument.openFile(fileUrl, fileType, pluginName, parameters))
                {
                    errorOpeningFileMessageDialog.text = application.baseFileNameForUrl(fileUrl) +
                            qsTr(" could not be opened due to an error.");
                    errorOpeningFileMessageDialog.open();
                }
                else
                    addToRecentFiles(fileUrl);
            }

            Component
            {
                id: tabComponent

                DocumentUI
                {
                    id: document

                    Component.onCompleted:
                    {
                        // Continue processing arguments after the document is loaded
                        document.loadComplete.connect(processOnePendingArgument);
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

            ToolButton
            {
                id: cancelButton

                implicitHeight: progressBar.implicitHeight * 0.8
                implicitWidth: implicitHeight

                iconName: "stop"
                visible: currentDocument ? currentDocument.commandIsCancellable && !cancelledIndicator.visible: false
                onClicked:
                {
                    currentDocument.cancelCommand();
                    cancelledIndicator.visible = true;
                }
            }

            BusyIndicator
            {
                implicitWidth: cancelButton.implicitWidth
                implicitHeight: cancelButton.implicitHeight

                id: cancelledIndicator
                visible: false
            }

            Connections
            {
                target: currentDocument
                onCommandInProgressChanged:
                {
                    // Reset the cancellation indicator when a command starts
                    cancelledIndicator.visible = false;
                }
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
                    if(currentCommandVerb === currentDocument.commandVerb)
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

            // Hack to force the RowLayout height to be the maximum of its children
            Rectangle { height: rowLayout.childrenRect.height }
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
