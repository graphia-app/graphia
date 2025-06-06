/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import Graphia.Controls
import Graphia.Utils

// This dialog is conceptually similar to a tabview in a dialog however the user
// interface consists of a vertical list of tabs with buttons to OPTIONALLY
// progress through. The tab animations are based on Wizard way of animating
//
// Child Items should be a ListTab with a title string
// Failing to do so will leave an empty tab title
Item
{
    id: root
    default property list<Item> listTabs
    property int _currentIndex: 0
    readonly property int currentIndex: _currentIndex
    property bool controlsEnabled: true
    property bool nextEnabled: true
    property bool finishEnabled: true
    property alias animating: numberAnimation.running
    readonly property int _padding: 5
    property Item currentItem:
    {
        return listTabs[_currentIndex];
    }

    //FIXME Set these based on the content tabs
    //minimumWidth: 640
    //minimumHeight: 480

    onWidthChanged: { content.x = _currentIndex * -contentContainer.width }
    onHeightChanged: { content.x = _currentIndex * -contentContainer.width }

    ColumnLayout
    {
        id: containerLayout

        Layout.fillWidth: true
        Layout.fillHeight: true

        spacing: Constants.spacing

        anchors.fill: parent
        anchors.margins: Constants.margin

        RowLayout
        {
            anchors.margins: Constants.margin
            Layout.fillHeight: true
            spacing: Constants.spacing

            ListBox
            {
                id: tabSelectorListBox

                Layout.fillHeight: true
                Layout.preferredWidth: 150

                onSelectedIndexChanged:
                {
                    goToTab(selectedIndex);
                }
            }

            Item
            {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Item
                {
                    id: contentContainer
                    anchors.fill: parent
                    clip: true
                    Item
                    {
                        id: content
                        height: parent.height
                    }
                }
            }
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                id: previousButton
                text: qsTr("Previous")
                onClicked: function(mouse) { root.goToPrevious(); }
                enabled: _currentIndex > 0 && root.controlsEnabled
            }

            Button
            {
                id: nextButton
                text: qsTr("Next")
                onClicked: function(mouse) { root.goToNext(); }
                enabled: (_currentIndex < listTabs.length - 1) ?
                    root.nextEnabled && root.controlsEnabled : false
            }

            Button
            {
                id: finishButton

                property bool _onFinishPage: _currentIndex === listTabs.length - 1

                text:
                {
                    if(!_onFinishPage)
                        return qsTr("Confirm");
                    else
                        return qsTr("Finish");
                }

                onClicked: function(mouse)
                {
                    if(_currentIndex !== (listTabs.length - 1))
                        goToTab(listTabs.length - 1);
                    else
                        root.accepted();
                }

                enabled:
                {
                    if(!root.controlsEnabled)
                        return false;

                    if(_onFinishPage)
                        return finishEnabled;

                    return true;
                }
            }

            Button
            {
                text: qsTr("Cancel")
                enabled: root.controlsEnabled
                onClicked: function(mouse) { root.rejected(); }
            }
        }
    }

    function _updateActiveTab()
    {
        for(let index = 0; index < listTabs.length; index++)
            listTabs[index].active = (index === _currentIndex);

        tabSelectorListBox.select(_currentIndex);
    }

    on_CurrentIndexChanged:
    {
        _updateActiveTab();
    }

    function goToNext()
    {
        if(_currentIndex < listTabs.length - 1)
        {
            _currentIndex++;
            numberAnimation.running = false;
            numberAnimation.running = true;
        }
    }

    function goToPrevious()
    {
        if(_currentIndex > 0)
        {
            _currentIndex--;
            numberAnimation.running = false;
            numberAnimation.running = true;
        }
    }

    function goToTab(index)
    {
        if(index === _currentIndex)
            return;

        if(index <= listTabs.length - 1 && index >= 0)
        {
            _currentIndex = index;
            numberAnimation.running = false;
            numberAnimation.running = true;
        }
    }

    function reset()
    {
        _currentIndex = 0;
        content.x = 0;
    }

    function indexOf(item)
    {
        for(let i = 0; i < listTabs.length; i++)
        {
            if(listTabs[i] === item)
                return i;
        }

        return -1;
    }

    NumberAnimation
    {
        id: numberAnimation
        target: content
        properties: "x"
        to: _currentIndex * -(contentContainer.width)
        easing.type: Easing.OutQuad

        onStarted: { root.animationStarted(); }
        onStopped: { root.animationStopped(); }
    }

    signal animationStarted();
    signal animationStopped();

    onAnimationStopped: { root.listTabChanged(); }
    signal listTabChanged();

    signal accepted();
    signal rejected();

    Component.onCompleted:
    {
        let listBoxModel = [];

        for(let i = 0; i < listTabs.length; i++)
        {
            listTabs[i].parent = content;
            listTabs[i].x = Qt.binding(function()
            {
                return indexOf(this) * contentContainer.width + tabSelectorListBox.implicitWidth;
            });
            listTabs[i].width = Qt.binding(() => contentContainer.width);
            listTabs[i].height = Qt.binding(() => contentContainer.height);

            listBoxModel.push(listTabs[i].title);
        }

        tabSelectorListBox.model = listBoxModel;

        _updateActiveTab();
    }
}
