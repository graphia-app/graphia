import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import QtQuick.Window 2.1

import com.kajeka 1.0

ApplicationWindow
{
    id: mainWindow
    visible: true
    width: 800
    height: 600
    minimumWidth: mainToolBar.implicitWidth
    minimumHeight: 480

    property DocumentUI currentDocument: tabView.currentIndex < tabView.count ?
                                             tabView.getTab(tabView.currentIndex).item : null

    title: (currentDocument ? currentDocument.title + qsTr(" - ") : "") + application.name()

    Component.onCompleted:
    {
        for(var i = 1; i < Qt.application.arguments.length; i++)
        {
            var fileUrl = Qt.resolvedUrl(Qt.application.arguments[i]);
            openFile(fileUrl, true);
        }
    }

    MessageDialog
    {
        id: aboutMessageDialog
        icon: StandardIcon.Information
        title: "About " + application.name()
        text: application.name() + qsTr(" is a tool for the visualisation and analysis of graphs.")
    }

    MessageDialog
    {
        id: unknownFileMessageDialog
        icon: StandardIcon.Critical
        title: qsTr("Unknown File Type")
    }

    MessageDialog
    {
        id: cantOpenFileMessageDialog
        icon: StandardIcon.Critical
        title: qsTr("Can't Open File")
    }

    function openFile(fileUrl, inNewTab)
    {
        var fileTypes = application.fileTypesOf(fileUrl);

        if(fileTypes.length === 0)
        {
            unknownFileMessageDialog.text = application.baseFileNameForUrl(fileUrl) +
                    qsTr(" cannot be loaded as its file type is unknown.");
            unknownFileMessageDialog.open();
            return;
        }

        if(!application.canOpenAnyOf(fileTypes))
        {
            cantOpenFileMessageDialog.text = application.baseFileNameForUrl(fileUrl) +
                    qsTr(" cannot be loaded."); //FIXME more elaborate error message
            cantOpenFileMessageDialog.open();
            return;
        }

        //FIXME handle case where there are multiple possible file types (allow the user to choose one)
        console.assert(fileTypes.length === 1)
        var fileType = fileTypes[0];

        if(currentDocument == null || inNewTab)
            tabView.createTab();

        tabView.openInCurrentTab(fileUrl, fileType);
    }

    FileDialog
    {
        id: fileOpenDialog
        modality: Qt.WindowModal
        title: qsTr("Open File...")
        nameFilters: application.nameFilters
        onAccepted: openFile(fileUrl, false)
    }

    FileDialog
    {
        id: fileOpenInTabDialog
        modality: Qt.WindowModal
        title: qsTr("Open File In New Tab...")
        nameFilters: application.nameFilters
        onAccepted: openFile(fileUrl, true)
    }

    Action
    {
        id: fileOpenAction
        //iconSource: "images/.png"
        iconName: "document-open"
        text: qsTr("&Open...")
        shortcut: "Ctrl+O"
        onTriggered: fileOpenDialog.open()
    }

    Action
    {
        id: fileOpenInTabAction
        //iconSource: "images/.png"
        iconName: "tab-new"
        text: qsTr("Open In New &Tab...")
        shortcut: "Ctrl+T"
        onTriggered: fileOpenInTabDialog.open()
    }

    Action
    {
        id: closeTabAction
        //iconSource: "images/.png"
        iconName: "window-close"
        text: qsTr("&Close Tab")
        shortcut: "Ctrl+W"
        enabled: currentDocument
        onTriggered: tabView.removeTab(tabView.currentIndex)
    }

    Action
    {
        id: quitAction
        //iconSource: "images/.png"
        iconName: "application-exit"
        text: qsTr("&Quit")
        shortcut: "Ctrl+Q"
        onTriggered: Qt.quit()
    }

    Action
    {
        id: undoAction
        //iconSource: "images/.png"
        iconName: "undo"
        text: currentDocument ? currentDocument.nextUndoAction : qsTr("&Undo")
        shortcut: "Ctrl+Z"
        enabled: currentDocument ? currentDocument.canUndo : false
        onTriggered: currentDocument && currentDocument.undo()
    }

    Action
    {
        id: redoAction
        //iconSource: "images/.png"
        iconName: "redo"
        text: currentDocument ? currentDocument.nextRedoAction : qsTr("&Redo")
        shortcut: "Ctrl+Shift+Z"
        enabled: currentDocument ? currentDocument.canRedo : false
        onTriggered: currentDocument && currentDocument.redo()
    }

    Action
    {
        id: deleteAction
        //iconSource: "images/.png"
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
        id: pauseLayoutAction
        //iconSource: "images/.png"
        iconName: currentDocument && currentDocument.layoutIsPaused ? "media-playback-start" : "media-playback-pause"
        text: currentDocument && currentDocument.layoutIsPaused ? qsTr("&Resume Layout") : qsTr("&Pause Layout")
        shortcut: "Pause"
        enabled: currentDocument ? currentDocument.idle : false
        onTriggered: currentDocument && currentDocument.toggleLayout()
    }

    Action
    {
        id: overviewModeAction
        //iconSource: "images/.png"
        iconName: "view-fullscreen"
        text: qsTr("&Overview Mode")
        shortcut: "Esc"
        enabled: currentDocument ? currentDocument.canEnterOverviewMode : false
        onTriggered: currentDocument && currentDocument.switchToOverviewMode()
    }

    Action
    {
        id: resetViewAction
        //iconSource: "images/.png"
        iconName: "view-refresh"
        text: qsTr("&Reset View")
        shortcut: "Ctrl+R"
        enabled: currentDocument ? currentDocument.canResetView : false
        onTriggered: currentDocument && currentDocument.resetView()
    }

    Action
    {
        id: toggleDebugPauserAction
        text: qsTr("Debug Pauser")
        shortcut: "Ctrl+P"
        enabled: application.debugEnabled()
        onTriggered: currentDocument && currentDocument.toggleDebugPauser()
        checkable: true
        checked: currentDocument && currentDocument.debugPauserEnabled
    }

    Action
    {
        id: debugResumeAction
        text: currentDocument ? currentDocument.debugResumeAction : qsTr("&Resume")
        shortcut: "Ctrl+N"
        enabled: application.debugEnabled()
        onTriggered: currentDocument && currentDocument.debugResume()
    }

    menuBar: MenuBar
    {
        Menu
        {
            title: qsTr("&File")
            MenuItem { action: fileOpenAction }
            MenuItem { action: fileOpenInTabAction }
            MenuItem { action: closeTabAction }
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
        }
        Menu
        {
            title: qsTr("&View")
            MenuItem { action: overviewModeAction }
            MenuItem { action: resetViewAction }
        }
        Menu
        {
            title: qsTr("&Layout")
            MenuItem { action: pauseLayoutAction }
        }
        Menu
        {
            title: qsTr("&Debug")
            enabled: application.debugEnabled()
            visible: application.debugEnabled()
            MenuItem { action: toggleDebugPauserAction }
            MenuItem
            {
                action: debugResumeAction
                visible: currentDocument && currentDocument.debugPaused
            }
        }
        Menu
        {
            title: qsTr("&Help")
            MenuItem { text: qsTr("About...") ; onTriggered: aboutMessageDialog.open() }
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

        function createTab()
        {
            var tab = addTab("", tabComponent);
            tabView.currentIndex = tabView.count - 1;

            // Make the tab title match the document title
            tab.title = Qt.binding(function() { return tab.item.title });

            return tab;
        }

        function openInCurrentTab(fileUrl, fileType)
        {
            currentDocument.application = application;
            currentDocument.openFile(fileUrl, fileType);
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
                value: currentDocument ? currentDocument.commandProgress / 100.0 : 0.0
                visible: currentDocument ? currentDocument.commandInProgress : false
                indeterminate: currentDocument ? currentDocument.commandProgress < 0.0 : false
            }

            // Hack to force the RowLayout height to be the maximum of its children
            Rectangle { height: rowLayout.childrenRect.height }
        }
    }

    Application
    {
        id: application
    }
}
