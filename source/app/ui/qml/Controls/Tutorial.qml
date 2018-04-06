import QtQuick 2.0

import com.kajeka 1.0

Item
{
    property int visibleHubbleId: -1
    property Item currentHubble:
    {
        if(visibleHubbleId < 0 || visibleHubbleId >= hubbles.length)
            return null;

        return hubbles[visibleHubbleId];
    }

    property bool _hasNextHubble:
    {
        return visibleHubbleId < hubbles.length - 1;
    }

    Preferences
    {
        id: misc
        section: "misc"
        property bool disableHubbles
    }

    // We unconditionally disable tooltip style Hubbles for the duration of
    // the tutorial, but need to track the state of the user preference so
    // it can be restored to the original value once the tutorial is closed
    property bool _tooltipHubbleOriginalDisableState: false

    default property list<Item> hubbles

    onHubblesChanged:
    {
        initialise();
    }

    Component.onCompleted:
    {
        initialise();
    }

    function reset()
    {
        closeCurrentHubble();
        visibleHubbleId = -1;

        misc.disableHubbles = _tooltipHubbleOriginalDisableState;
    }

    function start()
    {
        _tooltipHubbleOriginalDisableState = misc.disableHubbles;
        reset();
        misc.disableHubbles = true;

        gotoNextHubble();
    }

    function initialise()
    {
        reset();

        for(var i = 0; i < hubbles.length; i++)
        {
            hubbles[i].parent = parent;
            hubbles[i].opacity = 0.0;
            hubbles[i].visible = false;
        }
    }

    function gotoNextHubble()
    {
        closeCurrentHubble();

        if(_hasNextHubble)
        {
            visibleHubbleId++;

            if(_hasNextHubble)
            {
                currentHubble.displayNext = true;
                currentHubble.nextClicked.connect(gotoNextHubble);
                currentHubble.skipClicked.connect(reset);
            }
            else
            {
                currentHubble.displayNext = false;
                currentHubble.displayClose = true;
                currentHubble.closeClicked.connect(reset);
            }

            currentHubble.opacity = 1.0;
            currentHubble.visible = true;
        }
    }

    function closeCurrentHubble()
    {
        if(currentHubble !== null)
        {
            currentHubble.nextClicked.disconnect(gotoNextHubble);
            currentHubble.skipClicked.disconnect(closeCurrentHubble);
            currentHubble.opacity = 0.0;
            currentHubble.visible = false;
        }
    }
}
