import QtQuick 2.0

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
    }

    function start()
    {
        reset();
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
                currentHubble.skipClicked.connect(closeCurrentHubble);
            }
            else
            {
                currentHubble.displayNext = false;
                currentHubble.displayClose = true;
                currentHubble.closeClicked.connect(closeCurrentHubble);
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
