/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.7
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

import app.graphia 1.0
import SortFilterProxyModel 0.2

import "../../../../shared/ui/qml/Constants.js" as Constants

Item
{
    id: root

    property var application: null

    property bool _storingPreference: false

    Preferences
    {
        id: defaults
        section: "defaults"

        function _stringToObject(s, storeFn)
        {
            let o = {};

            if(s.length > 0)
            {
                try { o = JSON.parse(s); }
                catch(e) { o = {}; }
            }

            if(storeFn !== undefined)
                o.store = function() { storeFn(JSON.stringify(o)); };

            return o;
        }

        property string extensions
        onExtensionsChanged: { root.syncControlsToExtensionsPreference(); }

        function extensionsAsObject()
        {
            return _stringToObject(defaults.extensions, function(v)
            {
                root._storingPreference = true;
                defaults.extensions = v;
                root._storingPreference = false;
            });
        }

        property string plugins
        onPluginsChanged: { root.syncControlsToPluginsPreference(); }

        function pluginsAsObject()
        {
            return _stringToObject(defaults.plugins, function(v)
            {
                root._storingPreference = true;
                defaults.plugins = v;
                root._storingPreference = false;
            });
        }
    }

    function syncControlsToExtensionsPreference()
    {
        if(root._storingPreference)
            return;

        let extensionsObject = defaults.extensionsAsObject();

        for(let i = 0; i < urlTypeSelectors.count; i++)
        {
            let comboBox = urlTypeSelectors.itemAt(i);
            let urlType = extensionsObject[comboBox.extension];

            if(urlType !== undefined)
                comboBox.currentIndex = application.urlTypesFor(comboBox.extension).indexOf(urlType) + 1;
        }
    }

    function syncControlsToPluginsPreference()
    {
        if(root._storingPreference)
            return;

        let pluginsObject = defaults.pluginsAsObject();

        for(let j = 0; j < pluginSelectors.count; j++)
        {
            let comboBox = pluginSelectors.itemAt(j);
            let plugin = pluginsObject[comboBox.urlType];

            if(plugin !== undefined)
                comboBox.currentIndex = application.pluginNames(comboBox.urlType).indexOf(plugin) + 1;
        }
    }

    Component.onCompleted:
    {
        root.syncControlsToExtensionsPreference();
        root.syncControlsToPluginsPreference();
    }

    property var ambiguousExtensions: SortFilterProxyModel
    {
        sourceModel: root.application.loadableExtensions
        filters: [ ExpressionFilter { expression: { return model.index >= 0 && application.urlTypesFor(model.display).length > 1; } } ]
    }

    property var ambiguousUrlTypes: SortFilterProxyModel
    {
        sourceModel: root.application.urlTypeDetails
        filters: [ ExpressionFilter { expression: { return model.index >= 0 && application.pluginNames(model.name).length > 1; } } ]
    }

    property bool ambiguityInExtensionsOrPlugins: ambiguousExtensions.count > 0 || ambiguousUrlTypes.count > 0

    RowLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        Text
        {
            Layout.preferredWidth: 240
            Layout.alignment: Qt.AlignTop
            wrapMode: Text.Wrap
            textFormat: Text.StyledText

            text: qsTr("The pluggable nature of ") + application.name +
                qsTr(" means that it may be the case that some ") +
                qsTr("file types are interpretable in several different ways, ") +
                qsTr("or can be loaded by more than one plugin.<br><br>") +
                qsTr("When opening a file of such a type you will be prompted to ") +
                qsTr("choose how to proceed, when it is necessary to do so. ") +
                qsTr("Alternatively, you can avoid these prompts by selecting ") +
                qsTr("default actions, here:")
        }

        Item
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !root.ambiguityInExtensionsOrPlugins

            Label
            {
                anchors.centerIn: parent

                font.italic: true
                text: qsTr("No Ambiguities with Enabled Plugins")
            }
        }

        Rectangle
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.ambiguityInExtensionsOrPlugins

            border.width: scrollView.needsFrame ? 1 : 0
            border.color: systemPalette.dark

            ScrollView
            {
                id: scrollView
                anchors.fill: parent
                anchors.margins: 1
                clip: true

                property bool needsFrame: scrollView.contentHeight > scrollView.availableHeight

                RowLayout
                {
                    width: scrollView.width

                    ColumnLayout
                    {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.margins: scrollView.needsFrame ? Constants.margin : 0
                        spacing: Constants.spacing

                        Label
                        {
                            font.bold: true
                            text: qsTr("File Types")
                        }

                        GridLayout
                        {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: Constants.spacing
                            rowSpacing: Constants.spacing

                            Repeater
                            {
                                model: root.ambiguousExtensions

                                Label
                                {
                                    Layout.row: index
                                    Layout.column: 0

                                    font.italic: true
                                    text: "." + model.display
                                }
                            }

                            Repeater
                            {
                                id: urlTypeSelectors
                                model: root.ambiguousExtensions

                                ComboBox
                                {
                                    Layout.fillWidth: true
                                    Layout.row: index
                                    Layout.column: 1

                                    model: [qsTr("Always Ask…"), ...urlTypes.map(
                                        urlType => application.descriptionForUrlType(urlType))]

                                    property string extension: /*model.*/display
                                    property var urlTypes: { return application.urlTypesFor(extension); }

                                    onCurrentIndexChanged:
                                    {
                                        if(count === 0)
                                            return;

                                        let extensionsObject = defaults.extensionsAsObject();

                                        if(currentIndex === 0 && extensionsObject[extension] !== undefined)
                                            delete extensionsObject[extension];
                                        else if(currentIndex > 0)
                                            extensionsObject[extension] = urlTypes[currentIndex - 1];

                                        extensionsObject.store();
                                    }
                                }
                            }
                        }

                        Label
                        {
                            Layout.topMargin: Constants.margin * 2

                            font.bold: true
                            text: qsTr("Plugins")
                        }

                        GridLayout
                        {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: Constants.spacing
                            rowSpacing: Constants.spacing

                            Repeater
                            {
                                model: root.ambiguousUrlTypes

                                Label
                                {
                                    Layout.row: index
                                    Layout.column: 0

                                    font.italic: true
                                    text: model.collectiveDescription
                                }
                            }

                            Repeater
                            {
                                id: pluginSelectors
                                model: root.ambiguousUrlTypes

                                ComboBox
                                {
                                    Layout.fillWidth: true
                                    Layout.row: index
                                    Layout.column: 1

                                    model: [qsTr("Always Ask…"), ...applicablePlugins]

                                    property string urlType: /*model.*/name
                                    property var applicablePlugins: { return application.pluginNames(urlType); }

                                    onCurrentIndexChanged:
                                    {
                                        if(count === 0)
                                            return;

                                        let pluginsObject = defaults.pluginsAsObject();

                                        if(currentIndex === 0 && pluginsObject[urlType] !== undefined)
                                            delete pluginsObject[urlType];
                                        else if(currentIndex > 0)
                                            pluginsObject[urlType] = applicablePlugins[currentIndex - 1];

                                        pluginsObject.store();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
