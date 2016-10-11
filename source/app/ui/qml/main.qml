import QtQml 2.2
import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.0

import com.kajeka 1.0

ApplicationWindow
{
    id: mainWindow
    visible: true
    width: 800
    height: 600
    property bool maximised: visibility === Window.Maximized

    minimumWidth: mainToolBar.implicitWidth
    minimumHeight: 480

    property DocumentUI currentDocument: tabView.currentIndex < tabView.count ?
                                         tabView.getTab(tabView.currentIndex).item : null

    property var recentFiles;

    title: (currentDocument ? currentDocument.title + qsTr(" - ") : "") + application.name

    // This is called when the app is started, but it also receives the arguments
    // of a second instance when it starts then immediately exits
    function processArguments(arguments)
    {
        for(var i = 1; i < arguments.length; i++)
        {
            var fileUrl = application.urlForFileName(arguments[i]);
            openFile(fileUrl, true);
        }
    }

    Component.onCompleted:
    {
        if (recentFiles === undefined)
            recentFiles = [];
        fileOpenDialog.folder = misc.fileOpenInitialFolder;
        processArguments(Qt.application.arguments);
    }

    DropArea
    {
        anchors.fill: parent;
        onDropped:
        {
            openFile(drop.text, true)
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

    PluginsDialog
    {
        id: pluginsDialog
        pluginDetails: application.pluginDetails

        onHiddenSwitchActivated: debugCrash.trigger();
    }

    Preferences
    {
        section: "window"
        property alias x: mainWindow.x
        property alias y: mainWindow.y
        property alias width: mainWindow.width
        property alias height: mainWindow.height
        property alias maximised: mainWindow.maximised
    }

    Preferences
    {
        id: misc
        section: "misc"
        property alias showGraphMetrics: toggleGraphMetricsAction.checked
        property alias showLayoutSettings: toggleLayoutSettingsAction.checked

        property var fileOpenInitialFolder
        property alias recentFilesList: mainWindow.recentFiles;
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property int showNodeNames:
        {
            switch(nodeNameDisplay.current)
            {
            default:
            case hideNodeNamesAction:         return NodeTextState.Off;
            case showSelectedNodeNamesAction: return NodeTextState.Selected;
            case showAllNodeNamesAction:      return NodeTextState.All;
            }
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
        var RECENT_FILES_LENGTH = 5;

        var fileName = application.pathForUrl(fileUrl);

        // Perform a copy and assign back as it's a var element
        var copyRecentFiles = mainWindow.recentFiles;

        // Remove any duplicates
        for(var i=0; i<copyRecentFiles.length; i++)
        {
            if(copyRecentFiles[i] === fileName)
            {
                copyRecentFiles.splice(i, 1);
                break;
            }
        }

        // Add to the top
        copyRecentFiles.unshift(fileName);

        if(copyRecentFiles.length > RECENT_FILES_LENGTH)
            copyRecentFiles.pop();

        mainWindow.recentFiles = copyRecentFiles;
    }


    function openFile(fileUrl, inNewTab)
    {
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
            openFileOfType(fileUrl, fileTypes[0], inNewTab)

        addToRecentFiles(fileUrl);
    }

    FileTypeChooserDialog
    {
        id: fileTypeChooserDialog
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
        model: application.pluginDetails
        onAccepted: openFileOfTypeWithPlugin(fileUrl, fileType, pluginName, inNewTab)
    }

    function openFileOfTypeWithPlugin(fileUrl, fileType, pluginName, inNewTab)
    {
        if(currentDocument != null && !inNewTab)
            tabView.replaceTab();
        else
            tabView.createTab();

        tabView.openInCurrentTab(fileUrl, fileType, pluginName);
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
        onTriggered: currentDocument && currentDocument.showFind()
    }

    Action
    {
        id: prevComponentAction
        text: qsTr("Goto &Previous Component")
        shortcut: "PgUp"
        onTriggered: currentDocument && currentDocument.gotoPrevComponent()
    }

    Action
    {
        id: nextComponentAction
        text: qsTr("Goto &Next Component")
        shortcut: "PgDown"
        onTriggered: currentDocument && currentDocument.gotoNextComponent()
    }

    Action
    {
        id: optionsAction
        text: qsTr("&Options...")
        onTriggered: optionsDialog.show();
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
        shortcut: "Ctrl+R"
        enabled: currentDocument ? currentDocument.canResetView : false
        onTriggered: currentDocument && currentDocument.resetView()
    }

    Action
    {
        id: toggleGraphMetricsAction
        text: qsTr("Show Graph Metrics")
        checkable: true
    }

    ExclusiveGroup
    {
        id: nodeNameDisplay

        Action { id: hideNodeNamesAction; text: qsTr("None"); checkable: true; }
        Action { id: showSelectedNodeNamesAction; text: qsTr("Selected"); checkable: true; }
        Action { id: showAllNodeNamesAction; text: qsTr("All"); checkable: true; }

        Component.onCompleted:
        {
            switch(visuals.showNodeNames)
            {
            default:
            case NodeTextState.Off:      nodeNameDisplay.current = hideNodeNamesAction; break;
            case NodeTextState.Selected: nodeNameDisplay.current = showSelectedNodeNamesAction; break;
            case NodeTextState.All:      nodeNameDisplay.current = showAllNodeNamesAction; break;
            }
        }
    }

    Action
    {
        id: toggleDebugPauserAction
        text: qsTr("Debug Pauser")
        shortcut: "Ctrl+P"
        enabled: application.debugEnabled
        onTriggered: currentDocument && currentDocument.toggleDebugPauser()
        checkable: true
        checked: currentDocument && currentDocument.debugPauserEnabled
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
        id: debugResumeAction
        text: currentDocument ? currentDocument.debugResumeAction : qsTr("&Resume")
        shortcut: "Ctrl+N"
        enabled: application.debugEnabled
        onTriggered: currentDocument && currentDocument.debugResume()
    }

    Action
    {
        id: debugCrash
        text: qsTr("&Crash")
        onTriggered: application.crash()
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
        checked: currentDocument && currentDocument.poppedOut
        enabled: currentDocument && currentDocument.hasPluginUI
        onTriggered: currentDocument && currentDocument.togglePop()
    }

    Action
    {
        // A do nothing action that we use when there
        // is no other valid action available
        id: nullAction
    }

    menuBar: MenuBar
    {
        Menu
        {
            title: qsTr("&File")
            MenuItem { action: fileOpenAction }
            MenuItem { action: fileOpenInTabAction }
            MenuItem { action: closeTabAction }
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
                            text: index > -1 ? mainWindow.recentFiles[index] : "";
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
                title: qsTr("Show Node Names")
                MenuItem { action: hideNodeNamesAction }
                MenuItem { action: showSelectedNodeNamesAction }
                MenuItem { action: showAllNodeNamesAction }
            }
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
            enabled: application.debugEnabled
            visible: application.debugEnabled
            MenuItem { action: debugCrash }
            MenuItem { action: toggleDebugPauserAction }
            MenuItem
            {
                action: debugResumeAction
                visible: currentDocument && currentDocument.debugPaused
            }
            MenuItem { action: dumpGraphAction }
            MenuItem { action: toggleFpsMeterAction }
            MenuItem { action: toggleGlyphmapSaveAction }
        }
        Menu
        {
            title: qsTr("&Help")
            MenuItem { text: qsTr("About Plugins...") ; onTriggered: pluginsDialog.show() }
            MenuItem { text: qsTr("About " + application.name + "...") ; onTriggered: aboutMessageDialog.open() }
        }
    }

    toolBar: ToolBar
    {
        id: mainToolBar
        width: parent.width

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
            ToolButton { action: togglePluginWindowAction }
            ToolButton
            {
                action: debugResumeAction
                visible: currentDocument && currentDocument.debugPaused
            }

            Item { Layout.fillWidth: true }
        }
    }

    TabView
    {
        id: tabView
        anchors.fill: parent
        tabsVisible: count > 1
        frameVisible: count > 1

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

        function openInCurrentTab(fileUrl, fileType, pluginName)
        {
            currentDocument.application = application;
            if(!currentDocument.openFile(fileUrl, fileType, pluginName))
            {
                errorOpeningFileMessageDialog.text = application.baseFileNameForUrl(fileUrl) +
                        qsTr(" could not be opened due to an error.");
                errorOpeningFileMessageDialog.open();
            }
        }

        Component
        {
            id: tabComponent
            DocumentUI
            {
                id: document
            }
        }
    }

    statusBar: StatusBar
    {
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

    Application
    {
        id: application
    }
}
