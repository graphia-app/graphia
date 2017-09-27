import QtQuick 2.7
import QtQuick.Controls 1.5

Item
{
    id: root

    property Item item
    property int alignment: Qt.AlignTop
    property int direction: Qt.Vertical

    property int horizontalOffset: 0
    property int verticalOffset: 0

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

    property int _expandedWidth: item.width + root.horizontalOffset;
    property int _expandedHeight: item.height + root.verticalOffset;

    property int _expandedDimension:
    {
        if(direction === Qt.Vertical)
            return _expandedHeight;

        return _expandedWidth;
    }

    property int _currentDimension:
    {
        if(direction === Qt.Vertical)
            return implicitHeight;

        return implicitWidth;
    }

    function _resetDimensionBindings()
    {
        implicitWidth = Qt.binding(function() { return _expandedWidth; } );
        implicitHeight = Qt.binding(function() { return _expandedHeight; } );
    }

    onItemChanged:
    {
        item.parent = root;

        _resetDimensionBindings();
        _updateAlignment();
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
        if(_currentDimension >= _expandedDimension)
        {
            _resetDimensionBindings();

            if(disableItemWhenClosed)
                item.enabled = true;
        }
        else
            _hidden = true;
    }

    Behavior on implicitWidth
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

    Behavior on implicitHeight
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
        if(animate === undefined)
            animate = true;

        var animation; var drop;
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

        if(_currentDimension < _expandedDimension)
        {
            _hidden = false;
            animation.easing.type = Easing.OutBack;

            if(animate)
                drop.enabled = true;

            _resetDimensionBindings();

            if(animate)
                drop.enabled = false;
        }
    }

    function hide(animate)
    {
        if(animate === undefined)
            animate = true;

        var animation; var drop;
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

        if(_currentDimension >= _expandedDimension)
        {
            animation.easing.type = Easing.InBack;

            if(animate)
                drop.enabled = true;

            if(disableItemWhenClosed)
                item.enabled = false;

            if(direction === Qt.Vertical)
                implicitHeight = 0;
            else
                implicitWidth = 0;

            if(animate)
                drop.enabled = false;
            else
                _hidden = true;
        }
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
