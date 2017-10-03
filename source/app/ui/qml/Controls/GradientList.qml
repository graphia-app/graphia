import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import com.kajeka 1.0
import ".."
import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils

Window
{
    id: root

    property string configuration
    property string selected
    property string initialConfiguration
    // Hack: the window is shared between visualisations so
    // we need some way of knowing which one we're currently
    // changing
    property int visualisationIndex

    signal accepted()
    signal rejected()

    title: qsTr("Pick Gradient")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog

    width: 400 + 2 * Constants.margin
    minimumWidth: 250 + 2 * Constants.margin
    maximumWidth: 500

    height: minimumHeight
    minimumHeight: 520 + 2 * Constants.margin

    onSelectedChanged:
    {
        initialConfiguration = selected;
        gradientPickerItem.initialise(selected);
    }

    SystemPalette
    {
        id: systemPalette
        colorGroup: SystemPalette.Active
    }

    ExclusiveGroup { id: selectedGroup }

    Preferences
    {
        id: visuals
        section: "visuals"
        property string savedGradients
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        GradientPickerItem
        {
            id: gradientPickerItem
            expanded: true
            expandable: false
            Layout.fillWidth: true
            onCurrentGradientChanged:
            {
                root.configuration = currentGradient
            }
            Component.onCompleted:
            {
                if(!selected)
                    gradientPickerItem.initialise("{ \"0.0\" : \"black\" , \"1.0\" : \"white\"}");
            }
        }

        RowLayout
        {
            ToolButton
            {
                iconName: "list-add"
                enabled: gradientListRepeater.count < 32 &&
                         !(findGradient(gradientPickerItem.currentGradient) + 1)
                onClicked:
                {
                    // Thanks listModels..
                    addGradient(gradientPickerItem.currentGradient);
                    gradientListRepeater.model.insert(0, {"gradientObj": gradientPickerItem.currentGradient});
                }
            }

            Text
            {
                visible: true
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Left click bar to add a stop.\n" +
                           "Right click a stop to delete it.\n" +
                           "Double click a stop to choose its colour.");
            }

        }

        ScrollView
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            id: scrollView

            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
            contentItem: Column
            {
                spacing: 0
                id: layout

                move: Transition { NumberAnimation { properties: "y"; duration: 100 } }

                Repeater
                {
                    id: gradientListRepeater
                    property bool defaultOpenPicker: false
                    model: ListModel {}

                    delegate: Item
                    {
                        property alias gradientItem: gradientItem
                        property bool checked: false
                        property ExclusiveGroup exclusiveGroup: selectedGroup

                        id: delegatePickerItem
                        width: scrollView.viewport.width
                        height: gradientItem.height + Constants.margin * 2
                        onExclusiveGroupChanged: { exclusiveGroup.bindCheckable(delegatePickerItem) }

                        Rectangle
                        {
                            color: checked ? Qt.lighter(systemPalette.highlight) : "transparent";
                            width: parent.width
                            height: parent.height
                            RowLayout
                            {
                                id: rowLayout
                                anchors.fill: parent
                                anchors.margins: Constants.margin

                                GradientPickerItem
                                {
                                    id: gradientItem
                                    expandable: false
                                    Layout.fillWidth: true
                                    height: 30
                                    deleteFunction: function()
                                    {
                                        removeGradient(gradientItem.currentGradient);
                                        gradientListRepeater.model.remove(index);
                                    }
                                    onClicked:
                                    {
                                        gradientPickerItem.initialise(gradientItem.currentGradient);
                                    }
                                    Component.onCompleted: initialise(gradientObj)
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            anchors.right: parent.right
            Text
            {
                visible: gradientListRepeater.count >= 32
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Saved gradients limit reached");
            }
            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("OK")
                onClicked:
                {
                    accepted();
                    root.close();
                }
            }
            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Cancel")
                onClicked:
                {
                    if(initialConfiguration !== null && initialConfiguration !== "")
                    {
                        configuration = initialConfiguration;
                        gradientPickerItem.initialise(configuration);
                    }
                    rejected();
                    root.close();
                }
            }
        }
    }
    Component.onCompleted: { populateGradients() }

    function populateGradients()
    {
        var savedGradients = loadGradientsFromPreferences();

        gradientListRepeater.model.clear();
        for(var i = 0; i < savedGradients.length; i++)
            gradientListRepeater.model.append({"gradientObj" : JSON.stringify(savedGradients[i])});
    }

    function loadGradientsFromPreferences()
    {
        var savedGradients;
        try
        {
            savedGradients = JSON.parse(visuals.savedGradients);
        }
        catch(e)
        {
            savedGradients = [];
        }
        return savedGradients;
    }

    function compareGradients(gradientA, gradientB)
    {
        var properties = Object.keys(gradientA).concat(Object.keys(gradientB));
        for(var i = 0; i < properties.length; i++)
        {
            var property = properties[i];
            if(gradientA[property] && gradientB[property])
            {
                if(!Qt.colorEqual(gradientA[property], gradientB[property]))
                    return false;
            }
            else
                return false;
        }
        return true;
    }

    function findGradient(grad)
    {
        var gradientObj = JSON.parse(grad);
        var savedGradients = loadGradientsFromPreferences();
        for(var i = 0; i < savedGradients.length; i++)
        {
            if (compareGradients(gradientObj,savedGradients[i]))
                return i;
        }
        return -1;
    }

    function removeGradient(grad)
    {
        var savedGradients = loadGradientsFromPreferences();
        var index = findGradient(grad);
        if(index > -1)
            savedGradients.splice(index, 1);
        visuals.savedGradients = JSON.stringify(savedGradients);
    }

    function addGradient(grad)
    {
        var savedGradients = loadGradientsFromPreferences();
        savedGradients.unshift(JSON.parse(grad));
        visuals.savedGradients = JSON.stringify(savedGradients);
    }
}
