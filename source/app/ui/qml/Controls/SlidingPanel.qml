import QtQuick 2.7
import QtQuick.Controls 1.5

Item
{
    id: root

    property Item item
    property int alignment: Qt.AlignTop

    function _updateAlignment()
    {
        if(item !== null)
        {
            // Align the item so it slides off in the correct direction
            item.anchors.left =
                item.anchors.right =
                item.anchors.top =
                item.anchors.bottom = undefined;

            switch(root.alignment)
            {
            case Qt.AlignTop:    item.anchors.bottom = root.bottom; break;
            case Qt.AlignBottom: item.anchors.top    = root.top;    break;
            case Qt.AlignLeft:   item.anchors.right  = root.right;  break;
            case Qt.AlignRight:  item.anchors.left   = root.left;   break;
            }
        }
    }

    onAlignmentChanged:
    {
        _updateAlignment();
    }

    function _resetDimensionBindings()
    {
        implicitWidth = Qt.binding(function() { return item.width; } );
        implicitHeight = Qt.binding(function() { return item.height; } );
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
        if(implicitHeight >= item.height)
        {
            _resetDimensionBindings();

            if(disableItemWhenClosed)
                item.enabled = true;
        }
        else
        {
            implicitWidth = 0;
            _hidden = true;
        }
    }

    Behavior on implicitHeight
    {
        id: drop
        enabled: false
        NumberAnimation
        {
            id: animation
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

        if(implicitHeight < item.height)
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

        if(implicitHeight >= item.height)
        {
            animation.easing.type = Easing.InBack;

            if(animate)
                drop.enabled = true;

            if(disableItemWhenClosed)
                item.enabled = false;

            implicitHeight = 0;

            if(animate)
                drop.enabled = false;
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
