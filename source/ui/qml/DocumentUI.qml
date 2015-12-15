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

    property alias preferencesform: preferencesform


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
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                horizontalAlignment: Text.AlignRight
                text: (graph.numNodes >= 0 ? graph.numNodes + " nodes" : "") +
                      (graph.numEdges >= 0 ? "\n" + graph.numEdges + " edges" : "") +
                      (graph.numComponents >= 0 ? "\n" + graph.numComponents + " components" : "");
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

    PreferencesForm {
        id: preferencesform
        paramList.model: document.layoutParams
        //sldrTension.onValueChanged: document.updatePreferences(sldrTension.value);
    }


}
