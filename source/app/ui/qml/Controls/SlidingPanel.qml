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

    Component.onCompleted: { _resetDimensionBindings(); }

    clip: true

    readonly property bool hidden: false

    function _onAnimationComplete()
    {
        if(implicitHeight >= item.height)
        {
            _resetDimensionBindings();
            item.enabled = true;
        }
        else
            hidden = true;
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

    function show()
    {
        if(implicitHeight < item.height)
        {
            hidden = false;
            animation.easing.type = Easing.OutBack;
            drop.enabled = true;
            implicitHeight = item.height;
            drop.enabled = false;
        }
    }

    function hide()
    {
        if(implicitHeight >= item.height)
        {
            animation.easing.type = Easing.InBack;
            drop.enabled = true;
            item.enabled = false;
            implicitHeight = 0;
            drop.enabled = false;
        }
    }

    function toggle()
    {
        if(hidden)
            show();
        else
            hide();
    }
}
