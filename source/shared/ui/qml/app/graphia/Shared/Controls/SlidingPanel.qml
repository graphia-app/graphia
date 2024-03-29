/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

Item
{
    id: root

    property Item item
    property int alignment: Qt.AlignTop
    property int direction: Qt.Vertical

    implicitWidth: width
    implicitHeight: height

    function _updateAlignment()
    {
        if(item !== null)
        {
            // Align the item so it slides off in the correct direction
            item.anchors.left =
                item.anchors.right =
                item.anchors.top =
                item.anchors.bottom = undefined;

            if(root.alignment & Qt.AlignTop)
                item.anchors.bottom = root.bottom;
            else if(root.alignment & Qt.AlignBottom)
                item.anchors.top = root.top;

            if(root.alignment & Qt.AlignLeft)
                item.anchors.right = root.right;
            else if(root.alignment & Qt.AlignRight)
                item.anchors.left = root.left;
        }
    }

    onAlignmentChanged:
    {
        _updateAlignment();
    }

    property int _currentDimension:
    {
        if(direction === Qt.Vertical)
            return height;

        return width;
    }

    property bool _opened: _currentDimension > 0

    function _setSizeToItem()
    {
        width = item.implicitWidth;
        height = item.implicitHeight;
    }

    onItemChanged:
    {
        item.parent = root;

        _setSizeToItem();
        _updateAlignment();
    }

    Connections
    {
        target: item
        function onImplicitWidthChanged()  { if(!root._hidden) width = item.implicitWidth; }
        function onImplicitHeightChanged() { if(!root._hidden) height = item.implicitHeight; }
    }

    property bool initiallyOpen: true
    property bool disableItemWhenClosed: true

    Component.onCompleted:
    {
        if(!initiallyOpen)
            hide(false);
        else
            show(false);
    }

    clip: true

    // Public
    readonly property bool hidden: _hidden

    // Internal
    property bool _hidden: false

    function _onAnimationComplete()
    {
        if(_opened)
        {
            _setSizeToItem();

            if(disableItemWhenClosed)
                item.enabled = true;
        }
        else
            _hidden = true;
    }

    Behavior on width
    {
        id: horizontalDrop
        enabled: false
        NumberAnimation
        {
            id: horizontalAnimation
            onRunningChanged:
            {
                // After the animation has finished, restore the property binding
                if(!running)
                    root._onAnimationComplete();
            }
        }
    }

    Behavior on height
    {
        id: verticalDrop
        enabled: false
        NumberAnimation
        {
            id: verticalAnimation
            onRunningChanged:
            {
                // After the animation has finished, restore the property binding
                if(!running)
                    root._onAnimationComplete();
            }
        }
    }

    function show(animate)
    {
        _hidden = false;

        if(_opened)
            return;

        if(animate === undefined)
            animate = true;

        let animation; let drop;
        if(direction === Qt.Vertical)
        {
            animation = verticalAnimation;
            drop = verticalDrop;
        }
        else
        {
            animation = horizontalAnimation;
            drop = horizontalDrop;
        }

        animation.easing.type = Easing.OutBack;

        if(animate)
            drop.enabled = true;

        _setSizeToItem();

        if(animate)
            drop.enabled = false;
    }

    function hide(animate)
    {
        if(!_opened)
        {
            _hidden = true;
            return;
        }

        if(animate === undefined)
            animate = true;

        let animation; let drop;
        if(direction === Qt.Vertical)
        {
            animation = verticalAnimation;
            drop = verticalDrop;
        }
        else
        {
            animation = horizontalAnimation;
            drop = horizontalDrop;
        }

        animation.easing.type = Easing.InBack;

        if(animate)
            drop.enabled = true;

        if(disableItemWhenClosed)
            item.enabled = false;

        if(direction === Qt.Vertical)
            height = 0;
        else
            width = 0;

        if(animate)
            drop.enabled = false;
        else
            _hidden = true;
    }

    function toggle(animate)
    {
        if(animate === undefined)
            animate = true;

        if(hidden)
            show(animate);
        else
            hide(animate);
    }
}
