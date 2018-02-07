import QtQuick 2.0
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Window
{
    title: qsTr("Enrichment Results")
    height: 200
    width: 800
    property var models;
    ColumnLayout
    {
        anchors.fill: parent
        ToolBar
        {
            anchors.fill: parent
            ToolButton
            {
                iconName: "edit-delete"
                onClicked: models.remove(models.get(tabView.currentIndex))
            }
        }
        Text
        {
            anchors.centerIn: parent
            text: "No Results";
            visible: tabView.count === 0
        }
        TabView
        {
            id: tabView
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: tabView.count > 0
            Repeater
            {
                model: models
                Tab
                {
                    title: "Results " + index
                    TableView
                    {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: qtObject
                        TableViewColumn { role: "Selection"; title: "Selection"; width: 100 }
                        TableViewColumn { role: "Observed"; title: "Observed"; width: 100 }
                        TableViewColumn { role: "Expected"; title: "Expected"; width: 100 }
                        TableViewColumn { role: "ExpectedTrial"; title: "ExpectedTrial"; width: 100 }
                        TableViewColumn { role: "FObs"; title: "FObs"; width: 100 }
                        TableViewColumn { role: "FExp"; title: "FExp"; width: 100 }
                        TableViewColumn { role: "OverRep"; title: "OverRep"; width: 100 }
                        TableViewColumn { role: "ZScore"; title: "ZScore"; width: 100 }
                        TableViewColumn { role: "Fishers"; title: "Fishers"; width: 100 }
                    }
                }
            }
        }
    }

    onModelsChanged:
    {
        for(var i=0; i<tabView.count; i++)
            tabView.getTab(i).children[0].update();
    }
}
