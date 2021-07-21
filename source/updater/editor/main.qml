/* Copyright © 2013-2021 Graphia Technologies Ltd.
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
        property bool hexEncodeUpdatesString: true
        property string privateKeyFile: ""

        onPrivateKeyFileChanged:
        {
            root.privateKeyFileExists =
                QmlUtils.fileUrlExists(privateKeyFile);
        }

        property string defaultOpenFolder
        property string defaultImageOpenFolder
    }

    property var operatingSystems: []
    property var credentials: []

    property var updatesArray: []

    property string lastUsedFilename: ""
    property bool saveRequired: false

    property bool privateKeyFileExists: false

    function setSaveRequired()
    {
        root.saveRequired = true;
        status.text = "";
    }

    function resetSaveRequired()
    {
        root.saveRequired = false;
    }

    MessageDialog
    {
        id: validateErrorDialog
        icon: StandardIcon.Critical
        title: "Error"
    }

    function extractHostname(url)
    {
        let hostname;

        if (url.indexOf("//") >= 0)
            hostname = url.split('/')[2];
        else
            hostname = url.split('/')[0];

        return hostname.split(':')[0].split('?')[0];
    }

    property var checksums: ({})
    property int numActiveChecksummings: 0
    property bool busy: numActiveChecksummings > 0

    function calculateChecksum(url, onComplete)
    {
        if(root.checksums[url] !== undefined && root.checksums[url].length > 0)
        {
            onComplete(root.checksums[url]);
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
                let httpStatus = xhr.status;
                if(httpStatus >= 200 && httpStatus < 400)
                {
                    // Success
                    root.checksums[url] = QmlUtils.sha256(xhr.response);
                    onComplete(root.checksums[url]);
                }
                else
                {
                    validateErrorDialog.text = "HTTP status " + httpStatus + " while validating " + url + ".";
                    validateErrorDialog.open();
                    status.text = qsTr("Failed to download");

                    for(let index = 0; index < tabView.count; index++)
                    {
                        let updateUI = tabView.getTab(index).item;
                        updateUI.highlightUrl(url);
                    }
                }

                root.numActiveChecksummings--;
            }
        };

        root.numActiveChecksummings++;
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
                        "    awk 'END {print $3}'; exit ${PIPESTATUS[0]}) && " +
                        "(rsync -av \"$VOLUME\"\/*.app $(dirname EXISTING_INSTALL); SYNCED=$?;" +
                        "    (hdiutil detach -force \"$VOLUME\" || exit $?) && exit \"$SYNCED\")"
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
                /*{
                    domain: "",
                    username: "",
                    password: ""
                },*/
            ]);
        }

        root.operatingSystems = JSON.parse(settings.operatingSystems);
        root.credentials = JSON.parse(settings.credentials);

        let tempDir = QmlUtils.tempDirectory();
        if(tempDir.length > 0)
        {
            root._workingDirectory = tempDir;
            root._removeWorkingDirectory = true;
            QmlUtils.cd(root._workingDirectory);
        }

        if(Qt.application.arguments.length > 1 && QmlUtils.fileExists(Qt.application.arguments[1]))
        {
            root.lastUsedFilename = "file://" + Qt.application.arguments[1];
            root.open(lastUsedFilename);
        }

        resetSaveRequired();
    }

    onClosing:
    {
        let closedImmediately = root.confirmSave(function()
        {
            // After save confirmed or rejected (not cancelled), resume closing
            root.close();
        });

        if(!closedImmediately)
        {
            close.accepted = false;
            return;
        }

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
            setSaveRequired();
            settings.defaultImageOpenFolder = imageFileDialog.folder;

            let basename = QmlUtils.baseFileNameForUrl(file);

            let originalFilename = QmlUtils.fileNameForUrl(file);
            let targetFilename = root._workingDirectory + "/" + basename;

            if(!QmlUtils.copy(originalFilename, targetFilename))
                console.log("Copy to " + targetFilename + " failed. Already exists?");

            let currentTab = tabView.getTab(tabView.currentIndex).item;
            currentTab.markdownTextArea.insert(currentTab.markdownTextArea.cursorPosition,
                "![" + basename + "](file:" + basename + ")");
        }
    }

    Labs.FileDialog
    {
        id: keyFileDialog
        fileMode: Labs.FileDialog.OpenFile
        nameFilters: ["PEM files (*.pem)", "All files (*)"]

        onAccepted:
        {
            setSaveRequired();
            settings.privateKeyFile = file;
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

        Menu
        {
            title: qsTr("&Settings")

            MenuItem
            {
                text: settings.privateKeyFile.length > 0 ?
                    (qsTr("Change Private Key (") +
                        QmlUtils.fileNameForUrl(settings.privateKeyFile) +
                        qsTr(")")) :
                    qsTr("Select Private Key");

                onTriggered:
                {
                    keyFileDialog.open();
                }
            }

            MenuItem
            {
                checkable: true
                checked: settings.hexEncodeUpdatesString
                text: qsTr("Hex Encode Updates String")

                onCheckedChanged:
                {
                    settings.hexEncodeUpdatesString = checked;
                    setSaveRequired();
                }
            }
        }
    }

    function open(file)
    {
        let fileContents = QmlUtils.readFromFile(QmlUtils.fileNameForUrl(file));

        try
        {
            let savedObject = JSON.parse(fileContents);
            let jsonString = QmlUtils.isHexString(savedObject.updates) ?
                QmlUtils.hexStringAsString(savedObject.updates) : savedObject.updates;

            root.updatesArray = JSON.parse(jsonString);
        }
        catch(e)
        {
            console.log("Failed to load " + file + " " + e);
            return false;
        }

        while(tabView.count > 0)
            tabView.removeTab(0);

        root.updatesArray.sort(function(a, b)
        {
            return a.version - b.version;
        });

        root.updatesArray.sort(function(a, b)
        {
            if(a.version < b.version)
                return -1;

            if(a.version > b.version)
                return 1;

            return 0;
        });

        for(const update of root.updatesArray)
        {
            let tab = tabView.createTab();

            tab.item.versionText = update.version;
            tab.item.targetVersionRegexText = update.targetVersionRegex;

            tab.item.resetOsControls();

            for(const osName in update.payloads)
            {
                let url = update.payloads[osName].url;
                let checksum = update.payloads[osName].installerChecksum;

                let item = tab.item.osControlsFor(osName);
                item.osEnabledChecked = true;
                item.urlText = url;

                root.checksums[url] = checksum;
            }

            for(const image of update.images)
            {
                let outFilename = root._workingDirectory + "/" + image.filename;
                let content = QmlUtils.byteArrayFromBase64String(image.content);

                if(!QmlUtils.writeToFile(outFilename, content))
                {
                    console.log("Failed to write to " + outFilename);
                    return false;
                }
            }

            tab.item.markdownText = update.changeLog;
        }

        // Newly opened file doesn't need to be saved
        resetSaveRequired();

        return true;
    }

    property bool canSave:
    {
        if(!root.saveRequired)
            return false;

        if(tabView.count === 0)
            return false;

        for(let index = 0; index < tabView.count; index++)
        {
            let updateUI = tabView.getTab(index).item;
            if(updateUI === null || !updateUI.inputValid)
                return false;
        }

        if(!root.privateKeyFileExists)
            return false;

        return !root.busy
    }

    function saveFile(fileUrl, onSaveComplete)
    {
        if(!canSave)
        {
            console.log("save() failed");

            if(onSaveComplete !== undefined && onSaveComplete !== null)
                onSaveComplete()

            return;
        }

        root.updatesArray = [];
        let updatesRemaining = tabView.count;
        let onComplete = function()
        {
            let updatesString = settings.hexEncodeUpdatesString ?
                QmlUtils.stringAsHexString(JSON.stringify(root.updatesArray)) :
                JSON.stringify(root.updatesArray);

            let updatesSignature =
                QmlUtils.rsaSignatureForString(updatesString, settings.privateKeyFile);

            let saveObject =
            {
                updates: updatesString,
                signature: updatesSignature
            };

            if(updatesSignature.length === 0)
            {
                console.log("Failed to sign " + fileUrl);
                status.text = qsTr("Failed to sign");
            }
            else if(!QmlUtils.writeToFile(QmlUtils.fileNameForUrl(fileUrl), JSON.stringify(saveObject)))
            {
                console.log("Failed to write " + fileUrl);
                status.text = qsTr("Failed to save");
            }
            else
            {
                resetSaveRequired();
                status.text = qsTr("Saved ") + QmlUtils.baseFileNameForUrl(fileUrl);
            }

            if(onSaveComplete !== undefined && onSaveComplete !== null)
                onSaveComplete();
        };

        for(let index = 0; index < tabView.count; index++)
        {
            let updateUI = tabView.getTab(index).item;

            updateUI.generateJSON(function(updateObject)
            {
                root.updatesArray.push(updateObject);
                updatesRemaining--;
                if(updatesRemaining === 0)
                    onComplete();
            });
        }
    }

    function save(onSaveComplete)
    {
        if(root.lastUsedFilename.length > 0)
        {
            root.saveFile(root.lastUsedFilename, onSaveComplete);
            return;
        }

        saveDialog.onComplete = onSaveComplete;
        saveDialog.currentFile = root.lastUsedFilename;
        saveDialog.open();
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
            settings.defaultOpenFolder = openDialog.folder;
            root.lastUsedFilename = file;
        }
    }

    Action
    {
        id: openAction
        text: qsTr("&Open")
        shortcut: "Ctrl+O"

        enabled: !root.busy

        onTriggered:
        {
            if(settings.defaultOpenFolder !== undefined)
                openDialog.folder = settings.defaultOpenFolder;

            openDialog.open();
        }
    }

    MessageDialog
    {
        id: saveConfirmDialog

        property var onSaveConfirmedFunction

        title: qsTr("File Changed")
        text: qsTr("Do you want to save changes?")
        icon: StandardIcon.Warning
        standardButtons: StandardButton.Save | StandardButton.Discard | StandardButton.Cancel

        onAccepted:
        {
            if(onSaveConfirmedFunction !== undefined && onSaveConfirmedFunction !== null)
                root.save(onSaveConfirmedFunction);

            onSaveConfirmedFunction = null;
        }

        onDiscard:
        {
            resetSaveRequired();

            if(onSaveConfirmedFunction !== undefined && onSaveConfirmedFunction !== null)
                onSaveConfirmedFunction();

            onSaveConfirmedFunction = null;
        }
    }

    function confirmSave(onSaveConfirmedFunction)
    {
        if(root.saveRequired)
        {
            saveConfirmDialog.onSaveConfirmedFunction = onSaveConfirmedFunction;
            saveConfirmDialog.open();

            return false;
        }

        return true;
    }

    Labs.FileDialog
    {
        id: saveDialog
        title: qsTr("Save File…")
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: "json"

        property var onComplete

        onAccepted:
        {
            root.saveFile(file, saveDialog.onComplete);
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
            root.save();
        }
    }

    Action
    {
        id: saveAsAction
        text: qsTr("Save &As")

        enabled: root.canSave

        onTriggered:
        {
            root.lastUsedFilename = "";
            root.save();
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

            function createTab()
            {
                setSaveRequired();

                let tab = tabView.addTab("", tabComponent);
                tabView.currentIndex = tabView.count - 1;

                tab.title = Qt.binding(function()
                {
                    return tab.item.versionText.length > 0 ?
                        tab.item.versionText : qsTr("New Upgrade");
                });

                return tab;
            }

            Component
            {
                id: tabComponent

                ColumnLayout
                {
                    id: updateUI

                    property alias versionText: version.text
                    property alias targetVersionRegexText: targetVersionRegex.text
                    property alias markdownText: markdownChangelog.text

                    property alias markdownTextArea: markdownChangelog

                    function resetOsControls()
                    {
                        for(let i = 0; i < osControls.count; i++)
                        {
                            let item = osControls.itemAt(i);
                            item.osEnabledChecked = false;
                            item.urlText = "";
                        }
                    }

                    function osControlsFor(name)
                    {
                        for(let i = 0; i < osControls.count; i++)
                        {
                            let item = osControls.itemAt(i);

                            if(item.osName === name)
                                return item;
                        }

                        return null;
                    }

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
                                TextField
                                {
                                    id: version
                                    Layout.fillWidth: true

                                    onTextChanged: { setSaveRequired(); }
                                }

                                Label { text: qsTr("Target Regex:") }
                                TextField
                                {
                                    id: targetVersionRegex
                                    Layout.fillWidth: true

                                    onTextChanged: { setSaveRequired(); }
                                }
                            }

                            TextArea
                            {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                id: markdownChangelog

                                onTextChanged: { setSaveRequired(); }
                            }

                            RowLayout
                            {
                                Item { Layout.fillWidth: true }

                                Button
                                {
                                    text: qsTr("Add Image")
                                    onClicked:
                                    {
                                        if(settings.defaultImageOpenFolder !== undefined)
                                            imageFileDialog.folder = settings.defaultImageOpenFolder;

                                        imageFileDialog.open();
                                    }
                                }
                            }

                            Repeater
                            {
                                id: osControls

                                model: root.operatingSystems

                                delegate: RowLayout
                                {
                                    property string osName: modelData.name
                                    property alias osEnabledChecked: osEnabled.checked
                                    property alias urlText: url.text
                                    property alias urlTextField: url

                                    CheckBox
                                    {
                                        id: osEnabled

                                        text: modelData.name
                                        checked: true

                                        onCheckedChanged: { setSaveRequired(); }
                                    }

                                    TextField
                                    {
                                        id: url

                                        Layout.fillWidth: true

                                        enabled: osEnabled.checked

                                        onTextChanged:
                                        {
                                            if(root.checksums[text] !== undefined)
                                                root.checksums[text] = "";

                                            setSaveRequired();
                                            textColor = "black";
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
                        let enabledOses = [];

                        for(let i = 0; i < osControls.count; i++)
                        {
                            let item = osControls.itemAt(i);

                            if(!item.osEnabledChecked)
                                continue;

                            if(item.urlTextField.length === 0)
                                return false;

                            if(!QmlUtils.urlStringIsValid(item.urlTextField.text))
                                return false;

                            enabledOses.push(item.osName);
                        }

                        return enabledOses.length > 0 &&
                            version.length > 0 &&
                            targetVersionRegex.length > 0 &&
                            markdownChangelog.length > 0;
                    }

                    property var updateObject: ({})

                    function highlightUrl(url)
                    {
                        for(let i = 0; i < osControls.count; i++)
                        {
                            let item = osControls.itemAt(i);

                            if(!item.osEnabledChecked)
                                continue;

                            if(item.urlText === url)
                                item.urlTextField.textColor = "red";
                        }
                    }

                    function generateJSON(onComplete)
                    {
                        updateObject = {};
                        updateObject.version = version.text;
                        updateObject.targetVersionRegex = targetVersionRegex.text;
                        updateObject.changeLog = markdownChangelog.text;
                        updateObject.images = [];

                        let mdImageRegex = /\!\[[^\]]*\]\(([^ \)]*)[^\)]*\)/g;

                        let match;
                        while((match = mdImageRegex.exec(updateObject.changeLog)) !== null)
                        {
                            let image = QmlUtils.fileNameForUrl(match[1]);
                            let encoded = QmlUtils.base64EncodingOf(root._workingDirectory + "/" + image);
                            let baseImage = image.split('/').pop().split('#')[0].split('?')[0];

                            updateObject.images.push({filename: baseImage, content: encoded});
                        }

                        updateObject.payloads = {};

                        for(let i = 0; i < osControls.count; i++)
                        {
                            let item = osControls.itemAt(i);

                            if(!item.osEnabledChecked)
                                continue;

                            let osName = item.osName;
                            let url = item.urlText;
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
                        }

                        let remainingChecksums = Object.keys(updateObject.payloads).length;

                        for(const osName in updateObject.payloads)
                        {
                            let url = updateObject.payloads[osName].url;
                            calculateChecksum(url, function(osName) { return function(checksum)
                            {
                                updateObject.payloads[osName].installerChecksum = checksum;

                                remainingChecksums--;
                                if(remainingChecksums === 0)
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
                onClicked: { tabView.createTab(); }
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

            Text
            {
                id: status
                visible: !root.busy && text.length > 0
            }

            Button
            {
                id: saveButton
                action: saveAction
            }
        }
    }
}
