// This is substantially based on the d3 circle packing algorithm:
// https://github.com/mbostock/d3/blob/master/src/layout/pack.js

/*
Copyright (c) 2010-2016, Michael Bostock
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* The name Michael Bostock may not be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL MICHAEL BOSTOCK BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "circlepackcomponentlayout.h"

#include "graph/graph.h"
#include "graph/graphcomponent.h"
#include "graph/componentmanager.h"

#include "app/preferences.h"

#include <QVariant>

#include <algorithm>
#include <cmath>

struct Links
{
    ComponentId _prev;
    ComponentId _next;
};

static void insert(ComponentArray<Links>& links, ComponentId position, ComponentId value)
{
    auto c = links[position]._next;
    links[position]._next = value;
    links[value]._prev = position;
    links[value]._next = c;
    links[c]._prev = value;
}

static void join(ComponentArray<Links>& links, ComponentId a, ComponentId b)
{
    links[a]._next = b;
    links[b]._prev = a;
}

static void placeThirdCircleTangentially(const Circle& a, const Circle& b, Circle& c)
{
    float db = a.radius() + c.radius();
    float dx = b.x() - a.x();
    float dy = b.y() - a.y();

    if(db != 0.0f && (dx != 0.0f || dy != 0.0f))
    {
        float da = b.radius() + c.radius();
        float dc = dx * dx + dy * dy;

        da *= da;
        db *= db;

        float x = 0.5f + (db - da) / (2.0f * dc);
        float value = (2 * da * (db + dc)) - ((db - dc) * (db - dc)) - (da * da);

        float y = std::sqrt(std::max(0.0f, value)) / (2.0f * dc);

        c.setX(a.x() + x * dx + y * dy);
        c.setY(a.y() + x * dy - y * dx);
    }
    else
    {
        c.setX(a.x() + db);
        c.setY(a.y());
    }
}

static void circlePack(const std::vector<ComponentId>& componentIds,
                       ComponentLayoutData& componentLayoutData,
                       ComponentArray<Links>& links)
{
    if(componentIds.empty())
        return;

    auto numComponents = static_cast<int>(componentIds.size());

    ComponentId a = componentIds[0];
    ComponentId b;
    ComponentId c;
    componentLayoutData[a].setX(-componentLayoutData[a].radius());
    componentLayoutData[a].setY(0.0f);

    if(numComponents > 1)
    {
        b = componentIds[1];
        componentLayoutData[b].setX(componentLayoutData[b].radius());
        componentLayoutData[b].setY(0.0f);
    }

    if(numComponents > 2)
    {
        c = componentIds[2];
        placeThirdCircleTangentially(componentLayoutData[a],
                                     componentLayoutData[b],
                                     componentLayoutData[c]);

        insert(links, a, c);
        links[a]._prev = c;
        insert(links, c, b);
        b = links[a]._next;
    }

    for(int i = 3; i < numComponents; i++)
    {
        c = componentIds[i];
        placeThirdCircleTangentially(componentLayoutData[a],
                                     componentLayoutData[b],
                                     componentLayoutData[c]);

        const float EPSILON = 0.01f;
        bool intersects = false;
        int s1 = 1;
        ComponentId j, k;

        for(j = links[b]._next;
            j != b;
            j = links[j]._next, s1++)
        {
            if(componentLayoutData[c].distanceToSq(componentLayoutData[j]) < -EPSILON)
            {
                intersects = true;
                break;
            }
        }

        if(intersects)
        {
            int s2 = 1;

            for(k = links[a]._prev;
                k != links[j]._prev;
                k = links[k]._prev, s2++)
            {
                if(componentLayoutData[c].distanceToSq(componentLayoutData[k]) < -EPSILON)
                    break;
            }

            if(s1 < s2 || (s1 == s2 && componentLayoutData[b].radius() <
                                       componentLayoutData[a].radius()))
            {
                b = j;
                join(links, a, b);
            }
            else
            {
                a = k;
                join(links, a, b);
            }

            i--;
        }
        else
        {
            insert(links, a, c);
            b = c;
        }
    }
}

void CirclePackComponentLayout::executeReal(const Graph& graph, const std::vector<ComponentId> &componentIds,
                                            ComponentLayoutData& componentLayoutData)
{
    if(graph.numComponents() == 0)
        return;

    // Find the number of nodes in the largest component
    auto largestComponentId = graph.componentIdOfLargestComponent();
    int maxNumNodes = graph.componentById(largestComponentId)->numNodes();

    for(auto componentId : componentIds)
    {
        const auto* component = graph.componentById(componentId);
        float size = (static_cast<float>(component->numNodes()) * 100.0f) /
            static_cast<float>(maxNumNodes);
        componentLayoutData[componentId].setRadius(size);
    }

    auto sortedComponentIds = componentIds;
    std::stable_sort(sortedComponentIds.begin(), sortedComponentIds.end(),
                     [&componentLayoutData](const ComponentId& a, const ComponentId& b)
    {
        if(componentLayoutData[a].radius() == componentLayoutData[b].radius())
            return a < b;

        return componentLayoutData[a].radius() > componentLayoutData[b].radius();
    });

    ComponentArray<Links> links(graph);

    auto minimumComponentRadius = u::pref(QStringLiteral("visuals/minimumComponentRadius")).toFloat();
    for(auto componentId : sortedComponentIds)
    {
        componentLayoutData[componentId].setRadius(std::max(componentLayoutData[componentId].radius(),
                                                            minimumComponentRadius));
        links[componentId]._prev = links[componentId]._next = componentId;
    }

    circlePack(sortedComponentIds, componentLayoutData, links);
}
