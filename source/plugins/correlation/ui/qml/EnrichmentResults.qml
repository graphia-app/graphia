import QtQuick 2.0
import QtQuick.Window 2.3
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Window
{
    height: 200
    width: 800
    ColumnLayout
    {
        anchors.fill: parent
        ToolBar
        {
            anchors.fill: parent
            ToolButton {
                iconSource: "new.png"
            }
        }
        TableView
        {
            id: tableView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: plugin.model.enrichmentTableModel
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
    Component.onCompleted:
    {
        tableView.update();
        console.log(plugin.model.rowCount);
        console.log(tableView.columnCount);
        console.log(tableView.rowCount);
        console.log(plugin.model.enrichmentTableModel.rowCount());
        console.log(plugin.model.enrichmentTableModel.columnCount());
    }
}
