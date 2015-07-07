import QtQuick 2.2
import QtQuick.Controls 1.2

import com.kajeka 1.0

Item
{
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

    Graph
    {
        id: graph
        anchors.fill: parent
    }

    Document
    {
        id: document
        graph: graph
    }
}
