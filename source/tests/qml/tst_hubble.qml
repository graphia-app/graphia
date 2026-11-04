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
import QtQuick.Controls
import QtQuick.Layouts
import QtTest

import Graphia.Controls

TestCase
{
    name: "Hubble"
    when: windowShown

    ApplicationWindow
    {
        id: window
        width: 400
        height: 300
        visible: true

        header: Item { height: 10 }
    }

    Component
    {
        id: fixtureComponent

        Item
        {
            property alias layout: layout
            property alias itemTarget: itemTarget
            property alias sibling: sibling
            property alias buttonTarget: buttonTarget
            property alias hubble: hubble

            anchors.fill: parent

            RowLayout
            {
                id: layout
                spacing: 10

                // Takes the hover for itself, like a ButtonMenu
                Item
                {
                    id: itemTarget
                    implicitWidth: 50
                    implicitHeight: 50
                    Layout.alignment: Qt.AlignBottom

                    MouseArea { anchors.fill: parent; hoverEnabled: true }
                }

                Rectangle
                {
                    id: sibling
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 80
                }

                Button
                {
                    id: buttonTarget
                    hoverEnabled: true
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 50
                }
            }

            Hubble
            {
                id: hubble
                mainWindow: window
                tooltipMode: true
            }
        }
    }

    property var fixture: null
    property var layout: fixture ? fixture.layout : null
    property var itemTarget: fixture ? fixture.itemTarget : null
    property var sibling: fixture ? fixture.sibling : null
    property var buttonTarget: fixture ? fixture.buttonTarget : null
    property var hubble: fixture ? fixture.hubble : null

    function init()
    {
        fixture = fixtureComponent.createObject(window.contentItem);
        waitForItemPolished(fixture.layout);
        mouseMove(window.contentItem, 395, 295);
    }

    function cleanup()
    {
        fixture.destroy();
        fixture = null;
        wait(0);
    }

    function test_showsOverTargetWhoseChildTakesTheHover()
    {
        hubble.target = itemTarget;

        mouseMove(itemTarget, 25, 25);
        tryVerify(() => hubble.visible);

        mouseMove(window.contentItem, 395, 295);
        tryVerify(() => !hubble.visible);
    }

    function test_showsOverButton()
    {
        hubble.target = buttonTarget;

        mouseMove(buttonTarget, 25, 25);
        tryVerify(() => hubble.visible);

        mouseMove(window.contentItem, 395, 295);
        tryVerify(() => !hubble.visible);
    }

    function test_targetKeepsItsPlaceInTheLayout()
    {
        hubble.target = itemTarget;

        compare(itemTarget.parent, layout);
        compare(itemTarget.y, sibling.height - itemTarget.height);
    }

    function test_hiddenTargetTakesNoSpace()
    {
        hubble.target = itemTarget;

        itemTarget.visible = false;
        waitForItemPolished(layout);

        compare(sibling.x, 0);
    }

    function test_hidingTargetCancelsPendingHubble()
    {
        hubble.target = itemTarget;
        mouseMove(itemTarget, 25, 25);

        itemTarget.visible = false;

        wait(1000);
        verify(!hubble.visible);
    }

    function test_hidingTargetHidesHubble()
    {
        hubble.target = itemTarget;
        mouseMove(itemTarget, 25, 25);
        tryVerify(() => hubble.visible);

        itemTarget.visible = false;

        tryVerify(() => !hubble.visible);
    }

    function test_showsOverDisabledButton()
    {
        hubble.target = buttonTarget;
        buttonTarget.enabled = false;

        mouseMove(buttonTarget, 25, 25);
        tryVerify(() => hubble.visible);
    }

    function test_hoverDoesNotAffectNonTooltip()
    {
        hubble.tooltipMode = false;
        hubble.target = itemTarget;
        hubble.visible = true;

        mouseMove(hubble, 2, 2);
        mouseMove(itemTarget, 25, 25);
        mouseMove(window.contentItem, 395, 295);
        wait(100);

        verify(hubble.visible);
    }

    function test_changingTargetLeavesTheOldOneAlone()
    {
        hubble.target = itemTarget;
        hubble.target = buttonTarget;

        mouseMove(itemTarget, 25, 25);

        wait(1000);
        verify(!hubble.visible);

        mouseMove(buttonTarget, 25, 25);
        tryVerify(() => hubble.visible);
    }
}
