import QtQuick 2.0

Item
{
    property int visibleHubbleId: 0
    default property list<Item> hubbles

    onVisibleChanged:
    {
        if(visible)
        {
            currentHubble().opacity = 1.0;
            currentHubble().visible = true;
        }
        else
        {
            currentHubble().opacity = 0.0;
            currentHubble().visible = false;
        }
    }

    onHubblesChanged:
    {
        init();
    }

    Component.onCompleted:
    {
        init();
    }

    function init()
    {
        for(var i=0; i < hubbles.length; i++)
        {
            hubbles[i].parent = parent;
            hubbles[i].visible = false;
            currentHubble().opacity = 0.0;
        }
        currentHubble().displayNext = true;
        currentHubble().nextClicked.connect(nextHubble);
        currentHubble().skipClicked.connect(skipAll);

        if(visible)
        {
            currentHubble().opacity = 1.0;
            currentHubble().visible = true;
        }
        else
        {
            currentHubble().opacity = 0.0;
            currentHubble().visible = false;
        }
    }

    function nextHubble()
    {
        currentHubble().nextClicked.disconnect(nextHubble);
        currentHubble().skipClicked.disconnect(skipAll);

        if(visibleHubbleId < hubbles.length - 1)
        {
            currentHubble().opacity = 0;
            currentHubble().visible = false;

            visibleHubbleId++;

            currentHubble().displayNext = true;
            currentHubble().nextClicked.connect(nextHubble);
            currentHubble().skipClicked.connect(skipAll);
            currentHubble().visible = true;
            currentHubble().opacity = 1.0;
        }
        if(visibleHubbleId == hubbles.length - 1)
        {
            currentHubble().displayNext = false;
            currentHubble().displayClose = true;
            currentHubble().closeClicked.connect(skipAll);
        }
    }

    function skipAll()
    {
        currentHubble().nextClicked.disconnect(nextHubble);
        currentHubble().opacity = 0;
        currentHubble().visible = false;
        visibleHubbleId = 0;
    }

    function currentHubble()
    {
        return hubbles[visibleHubbleId];
    }
}
