/* Copyright © 2013-2020 Graphia Technologies Ltd.
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
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

import Qt.labs.platform 1.0 as Labs
import Qt.labs.settings 1.0

import app.graphia 1.0

import "../../shared/ui/qml/Constants.js" as Constants

ApplicationWindow
{
    id: root

    Settings
    {
        id: settings

        property string operatingSystems: ""
        property string credentials: ""
    }

    property var operatingSystems: []
    property var credentials: []

    property var updatesArray: []

    property string lastUsedFilename: ""
    property bool busy: _numActiveChecksummings > 0

    MessageDialog
    {
        id: validateErrorDialog
        icon: StandardIcon.Critical
        title: "Error"
    }

    function extractHostname(url)
    {
        var hostname;

        if (url.indexOf("//") >= 0)
            hostname = url.split('/')[2];
        else
            hostname = url.split('/')[0];

        return hostname.split(':')[0].split('?')[0];
    }

    property var _checksums: ({})
    property int _numActiveChecksummings: 0

    function calculateChecksum(url, onComplete)
    {
        if(root._checksums[url] !== undefined && root._checksums[url].length > 0)
        {
            onComplete(url, root._checksums[url]);
            return;
        }

        let hostname = extractHostname(url);

        let xhr = new XMLHttpRequest();
        xhr.open("GET", url);

        const foundCredential = root.credentials.find(credential => credential.domain === hostname);
        if(foundCredential !== undefined)
        {
            xhr.setRequestHeader("Authorization", "Basic " +
                Qt.btoa(foundCredential.username + ':' + foundCredential.password));
        }

        xhr.responseType = "arraybuffer";
        xhr.onreadystatechange = function()
        {
            if(xhr.readyState === XMLHttpRequest.DONE)
            {
                let status = xhr.status;
                if(status === 0 || (200 >= status && status < 400))
                {
                    // Success
                    root._checksums[url] = QmlUtils.sha256(xhr.response);
                    onComplete(url, root._checksums[url]);
                }
                else
                {
                    validateErrorDialog.text = "HTTP " + status + " while validating " + url + ".";
                    validateErrorDialog.open();
                }

                root._numActiveChecksummings--;
            }
        };

        root._numActiveChecksummings++;
        xhr.send();
    }

    property bool _removeWorkingDirectory: false
    property string _workingDirectory: "."

    Component.onCompleted:
    {
        if(settings.operatingSystems.length === 0)
        {
            settings.operatingSystems = JSON.stringify(
            [
                {
                    name: "winnt",
                    command: "INSTALLER_FILE /S"
                },
                {
                    name: "darwin",
                    command:
                        "VOLUME=$(hdiutil attach -nobrowse 'INSTALLER_FILE' |" +
                        "    awk 'END {print $3}'; exit ${PIPESTATUS[0]}) &&" +
                        "(rsync -a \"$VOLUME\"\/*.app $(dirname EXISTING_INSTALL); SYNCED=$?;" +
                        "    hdiutil detach -quiet \"$VOLUME\"; exit $? || exit \"$SYNCED\")"
                },
                {
                    name: "linux",
                    command:
                        "TARGET=$(readlink -f 'EXISTING_INSTALL') &&" +
                        "TEMP_TARGET=$(tempfile) &&" +
                        "RESTORE_METADATA=$(stat -c\"chmod %a '${TEMP_TARGET}' &&" +
                        "   chown %U:%G '${TEMP_TARGET}'\" \"${TARGET}\") &&" +
                        "COMMAND=\"[ \\\"\\${TAR_FILENAME}\\\" = 'Graphia.AppImage' ] &&" +
                        "   cat > '${TEMP_TARGET}' && ${RESTORE_METADATA} &&" +
                        "   mv '${TEMP_TARGET}' '${TARGET}'\" &&" +
                        "SUDO=$([ ! -w \"${TARGET}\" ] && echo 'pkexec' || echo '') &&" +
                        "${SUDO} tar -xf INSTALLER_FILE --to-command=\"sh -c \\\"${COMMAND}\\\"\" ||" +
                        "   (rm -f \"${TEMP_TARGET}\"; exit 1)"
                },
            ]);
        }

        if(settings.credentials.length === 0)
        {
            settings.credentials = JSON.stringify(
            [
                {
                    domain: "ci.kajeka.com",
                    username: "guest",
                    password: "BPXtis733qQKqeB"
                },
            ]);
        }

        root.operatingSystems = JSON.parse(settings.operatingSystems);
        root.credentials = JSON.parse(settings.credentials);

        var tempDir = QmlUtils.tempDirectory();
        if(tempDir.length > 0)
        {
            root._workingDirectory = tempDir;
            root._removeWorkingDirectory = true;
            QmlUtils.cd(root._workingDirectory);
        }
    }

    onClosing:
    {
        if(root._removeWorkingDirectory)
            QmlUtils.rmdir(root._workingDirectory);
    }

    visible: true
    flags: Qt.Window|Qt.Dialog

    title: qsTr("Update Editor") + (root.lastUsedFilename.length > 0 ?
        qsTr(" - ") + QmlUtils.baseFileNameForUrl(root.lastUsedFilename) : "")

    width: 800
    height: 600
    minimumWidth: 800
    minimumHeight: 600

    Labs.FileDialog
    {
        id: imageFileDialog
        fileMode: Labs.FileDialog.OpenFile
        nameFilters: ["Image files (*.jpg *.jpeg *.png)", "All files (*)"]

        onAccepted:
        {
            var basename = QmlUtils.baseFileNameForUrl(file);

            var originalFilename = QmlUtils.fileNameForUrl(file);
            var targetFilename = root._workingDirectory + "/" + basename;

            if(QmlUtils.copy(originalFilename, targetFilename))
            {
                var currentTab = tabView.getTab(tabView.currentIndex).item;
                currentTab.markdown.insert(currentTab.markdown.cursorPosition,
                    "![" + basename + "](file:" + basename + ")");
            }
        }
    }

    menuBar: MenuBar
    {
        id: mainMenuBar

        Menu
        {
            title: qsTr("&File")
            MenuItem { action: openAction }
            MenuItem { action: saveAction }
            MenuItem { action: saveAsAction }

            MenuItem
            {
                text: qsTr("&Exit")
                shortcut: "Ctrl+Q"
                enabled: !root.busy

                onTriggered: { root.close(); }
            }
        }
    }

    function open(file)
    {
        var fileContents = QmlUtils.readFromFile(QmlUtils.fileNameForUrl(file));

        try
        {
            root.updatesArray = JSON.parse(fileContents);
        }
        catch(e)
        {
            console.log("Failed to load " + file + " " + e);
        }

        //TODO
    }

    property bool canSave:
    {
        if(tabView.count === 0)
            return false;

        for(var index = 0; index < tabView.count; index++)
        {
            var updateUI = tabView.getTab(index).item;
            if(updateUI === null || !updateUI.inputValid)
                return false;
        }

        return !root.busy
    }

    function save(fileUrl)
    {
        if(!canSave)
        {
            console.log("save() failed");
            return;
        }

        root.updatesArray = [];
        var updatesRemaining = tabView.count;
        var onComplete = function()
        {
            let success = QmlUtils.writeToFile(QmlUtils.fileNameForUrl(fileUrl),
                JSON.stringify(root.updatesArray));

            if(!success)
                console.log("Failed to write " + fileUrl);
        };

        for(var index = 0; index < tabView.count; index++)
        {
            var updateUI = tabView.getTab(index).item;

            updateUI.generateJSON(function(updateObject)
            {
                root.updatesArray.push(updateObject);
                updatesRemaining--;
                if(updatesRemaining === 0)
                    onComplete();
            });
        }
    }

    Labs.FileDialog
    {
        id: openDialog
        title: qsTr("Open File…")
        fileMode: Labs.FileDialog.OpenFile
        defaultSuffix: "json"

        onAccepted:
        {
            root.open(file);
            root.lastUsedFilename = file;
        }
    }

    Action
    {
        id: openAction
        text: qsTr("&Open")
        shortcut: "Ctrl+O"

        enabled: !root.busy

        onTriggered: { openDialog.open(); }
    }

    Labs.FileDialog
    {
        id: saveDialog
        title: qsTr("Save File…")
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: "json"

        onAccepted:
        {
            root.save(file);
            root.lastUsedFilename = file;
        }
    }

    Action
    {
        id: saveAction
        text: qsTr("&Save")
        shortcut: "Ctrl+S"

        enabled: root.canSave

        onTriggered:
        {
            if(root.lastUsedFilename.length > 0)
            {
                root.save(root.lastUsedFilename);
                return;
            }

            saveDialog.currentFile = root.lastUsedFilename;
            saveDialog.open();
        }
    }

    Action
    {
        id: saveAsAction
        text: qsTr("Save &As")

        enabled: root.canSave

        onTriggered:
        {
            saveDialog.currentFile = root.lastUsedFilename;
            saveDialog.open();
        }
    }


    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        TabView
        {
            id: tabView
            enabled: !root.busy

            Layout.fillWidth: true
            Layout.fillHeight: true

            Component
            {
                id: tabComponent

                ColumnLayout
                {
                    id: updateUI

                    property string title: version.text
                    property alias markdown: markdownChangelog

                    SplitView
                    {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.margins: Constants.margin

                        orientation: Qt.Horizontal

                        handleDelegate: Item { width: Constants.spacing }

                        ColumnLayout
                        {
                            Layout.minimumWidth: 350
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            GridLayout
                            {
                                Layout.fillWidth: true

                                columns: 2

                                Label { text: qsTr("Version:") }
                                TextField { id: version; Layout.fillWidth: true }

                                Label { text: qsTr("Target Regex:") }
                                TextField { id: targetVersionRegex; Layout.fillWidth: true }
                            }

                            TextArea
                            {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                id: markdownChangelog
                            }

                            RowLayout
                            {
                                Item { Layout.fillWidth: true }

                                Button
                                {
                                    text: qsTr("Add Image")
                                    onClicked: { imageFileDialog.open(); }
                                }
                            }

                            Repeater
                            {
                                id: osControls

                                model: root.operatingSystems

                                delegate: RowLayout
                                {
                                    property string osName: modelData.name

                                    CheckBox
                                    {
                                        id: osEnabled

                                        text: modelData.name
                                        checked: true
                                    }

                                    TextField
                                    {
                                        Layout.fillWidth: true

                                        enabled: osEnabled.checked
                                        validator: RegExpValidator
                                        {
                                            regExp: /^(?:http(s)?:\/\/)?[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$/
                                        }

                                        onTextChanged:
                                        {
                                            if(root._checksums[text] !== undefined)
                                                root._checksums[text] = "";
                                        }
                                    }
                                }
                            }
                        }

                        TextArea
                        {
                            Layout.minimumWidth: 400
                            Layout.fillHeight: true

                            readOnly: true
                            text: markdownChangelog.text

                            textFormat: TextEdit.MarkdownText
                        }
                    }

                    property bool inputValid:
                    {
                        var enabledOses = [];

                        for(var i = 0; i < osControls.count; i++)
                        {
                            var item = osControls.itemAt(i);

                            if(!item.children[0].checked)
                                continue;

                            if(item.children[1].length === 0)
                                return false;

                            if(!item.children[1].acceptableInput)
                                return false;

                            enabledOses.push(item.osName);
                        }

                        return enabledOses.length > 0 &&
                            version.length > 0 &&
                            targetVersionRegex.length > 0 &&
                            markdownChangelog.length > 0;
                    }

                    property var updateObject: ({})

                    function generateJSON(onComplete)
                    {
                        updateObject = {};
                        updateObject.version = version.text;
                        updateObject.targetVersionRegex = targetVersionRegex.text;
                        updateObject.changeLog = markdownChangelog.text;
                        updateObject.images = [];

                        var mdImageRegex = /\!\[[^\]]*\]\(([^ \)]*)[^\)]*\)/g;

                        let match;
                        while((match = mdImageRegex.exec(updateObject.changeLog)) !== null)
                        {
                            let image = QmlUtils.fileNameForUrl(match[1]);
                            let encoded = QmlUtils.base64EncodingOf(root._workingDirectory + "/" + image);
                            let baseImage = image.split('/').pop().split('#')[0].split('?')[0];

                            updateObject.images.push({filename: baseImage, content: encoded});
                        }

                        updateObject.payloads = {};

                        for(var i = 0; i < osControls.count; i++)
                        {
                            let item = osControls.itemAt(i);

                            if(!item.children[0].checked)
                                continue;

                            let osName = item.osName;
                            let url = item.children[1].text;
                            let hostname = extractHostname(url);
                            let filename = url.split('/').pop().split('#')[0].split('?')[0];

                            if(filename.length === 0)
                                filename = "installer";

                            const foundOS = root.operatingSystems.find(os => os.name === osName);
                            if(foundOS === undefined)
                                continue;

                            updateObject.payloads[osName] = {};

                            const foundCredential = root.credentials.find(credential => credential.domain === hostname);
                            if(foundCredential !== undefined)
                            {
                                updateObject.payloads[osName].httpUserName = foundCredential.username;
                                updateObject.payloads[osName].httpPassword = foundCredential.password;
                            }

                            updateObject.payloads[osName].url = url;
                            updateObject.payloads[osName].installerFileName = filename;
                            updateObject.payloads[osName].installerChecksum = "";
                            updateObject.payloads[osName].command = foundOS.command;

                            calculateChecksum(url, function(osName) { return function(url, checksum)
                            {
                                updateObject.payloads[osName].installerChecksum = checksum;
                                onComplete(updateObject);
                            }}(osName));
                        }
                    }
                }
            }
        }

        RowLayout
        {
            Button
            {
                enabled: !root.busy

                text: qsTr("Add Upgrade")
                onClicked:
                {
                    let tab = tabView.addTab("", tabComponent);
                    tabView.currentIndex = tabView.count - 1;

                    tab.title = Qt.binding(function()
                    {
                        return tab.item.title.length > 0 ?
                            tab.item.title : qsTr("New Upgrade");
                    });
                }
            }

            Button
            {
                enabled: !root.busy

                text: qsTr("Remove Upgrade")
                onClicked:
                {
                    tabView.removeTab(tabView.currentIndex);
                }
            }

            Item { Layout.fillWidth: true }

            BusyIndicator
            {
                implicitHeight: saveButton.implicitHeight
                visible: root.busy
            }

            Button
            {
                id: saveButton
                action: saveAction
            }
        }
    }
}
