import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4
import QtWebEngine 1.3

Item
{
    anchors.fill: parent
    visible: true

    ColumnLayout
    {
        anchors.fill: parent

        ComboBox
        {
            id: urlTemplate
            editable: true
            model:
            [
                "https://en.wikipedia.org/wiki/%1",
                "https://www.google.co.uk/#q=%1",
                "https://www.google.co.uk/maps/search/%1",
                "https://twitter.com/%1",
                "https://www.reddit.com/search?q=%1",
                "http://www.ncbi.nlm.nih.gov/gene/?term=%1",
                "https://www.youtube.com/results?search_query=%1",
                "https://uk.finance.yahoo.com/lookup;?s=%1",
                "https://www.amazon.co.uk/s/ref=nb_sb_noss_2?field-keywords=%1",
                "http://www.ebay.co.uk/sch/i.html?_nkw=%1"
            ]
            Layout.fillWidth: true
        }

        ProgressBar
        {
            height: 3
            Layout.fillWidth: true
            style: ProgressBarStyle { background: Item {} }
            z: -2;
            minimumValue: 0
            maximumValue: 100
            value: webEngineView.loadProgress < 100 ? webEngineView.loadProgress : 0
        }

        WebEngineView
        {
            id: webEngineView
            Layout.fillWidth: true
            Layout.fillHeight: true
            url: urlTemplate.currentText.indexOf("%1") >= 0 ?
                     urlTemplate.currentText.arg(plugin.model.selectedNodeNames) : ""
        }
    }
}
