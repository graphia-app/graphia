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

// Scalable Ambient Obscurance (McGuire et al.), from the depths alone, smoothed
// across each element and multiplied into the layer's colour

#ifdef GL_ES
// Element numbers and depths are too large for medium precision to hold exactly
precision highp float;
#endif

#if defined(AO_SMOOTH_ACROSS) || defined(AO_DARKEN)

// The unsmoothed occlusion, from the first build
uniform sampler2D occlusion;

uniform sampler2D elementTexture;

// How many pixels either side are smoothed over, which is even
uniform int smoothingReach;

// Every rotation of the samples' spiral is in each 4 by 4 block, so an even reach with
// half the weight on the outermost sees each equally, leaving no blotches
float smoothedAlong(ivec2 p, ivec2 step, float element)
{
    ivec2 highest = textureSize(occlusion, 0) - ivec2(1);
    float sum = 0.0;
    float total = 0.0;

    for(int i = -smoothingReach; i <= smoothingReach; i++)
    {
        ivec2 q = clamp(p + (step * i), ivec2(0), highest);

        // Another element is another surface, and mixing where one lies over another
        // would move the darkness off the line where they meet
        if(texelFetch(elementTexture, q, 0).r != element)
            continue;

        float weight = abs(i) == smoothingReach ? 0.5 : 1.0;

        sum += texelFetch(occlusion, q, 0).r * weight;
        total += weight;
    }

    // The pixel itself is always counted
    return sum / total;
}

#ifdef AO_SMOOTH_ACROSS

layout (location = 0) out float outOcclusion;

void main()
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    float element = texelFetch(elementTexture, p, 0).r;

    // No traced element covers this pixel, and nothing reads it
    if(element == 0.0)
    {
        outOcclusion = 1.0;
        return;
    }

    outOcclusion = smoothedAlong(p, ivec2(1, 0), element);
}

#else

layout (location = 0) out vec4 outColor;

void main()
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    float element = texelFetch(elementTexture, p, 0).r;

    // No traced element covers this pixel
    if(element == 0.0)
        discard;

    float open = smoothedAlong(p, ivec2(0, 1), element);

    // Blended to multiply what is there
    outColor = vec4(vec3(open), 1.0);
}

#endif

#else

#include "componentdata.glsl"
#include "element.glsl"
#include "projection.glsl"
#include "viewport.glsl"

// How open each pixel is to the light
layout (location = 0) out float outOcclusion;

uniform sampler2D depthTexture;
uniform sampler2D elementTexture;

// The component each of the layer's nodes is in, then each of its edges, four to a texel
uniform TexelBuffer elementComponents;
uniform int numNodes;

// How far, in view space, what is near shuts out light
uniform float radius;

// Turns the samples' spiral, differently for each sample gathered
uniform float rotation;

// The radius on screen, in pixels, beyond which the samples are too sparse to say anything
uniform float maximumPixels;

const int NUM_SAMPLES = 24;
const float NUM_SPIRAL_TURNS = 7.0;
const float INTENSITY = 0.25;
const float TWO_PI = 6.2831853;

// Narrower than this, the nodes drawn when moving leave no depth, so it fades in
// from here to twice it, as the gathered picture does
const float FADE_IN_PIXELS = 3.0;

int componentAt(ivec2 p)
{
    float element = texelFetch(elementTexture, p, 0).r;

    if(element == NO_ELEMENT)
        return -1;

    int index = isNodeElement(element) ? elementIndex(element) : numNodes + elementIndex(element);
    vec4 texel = texelAt(elementComponents, index / 4);

    return int(texel[index % 4]);
}

// The view space position of a pixel, at its depth, as the projection put it there
vec3 viewPositionAt(ivec2 p, mat4 projectionMatrix, mat2 zToClip)
{
    ivec2 clamped = clamp(p, ivec2(0), ivec2(viewportSize) - ivec2(1));
    float depth = texelFetch(depthTexture, clamped, 0).r;
    vec2 ndc = (((vec2(clamped) + 0.5) / viewportSize) * 2.0) - 1.0;

    return unproject(projectionMatrix, zToClip, ndc, zAtDepth(zToClip, depth));
}

void main()
{
    ivec2 p = ivec2(gl_FragCoord.xy);
    int component = componentAt(p);

    if(component < 0)
    {
        outOcclusion = 1.0;
        return;
    }

    mat4 projectionMatrix = componentProjectionMatrix(component);
    mat2 zToClip = zToClipOf(projectionMatrix);
    vec3 position = viewPositionAt(p, projectionMatrix, zToClip);

    // The surface's normal, across the neighbours on whichever side is nearer
    // in depth, so that a silhouette's is of the surface it is part of
    vec3 left = position - viewPositionAt(p - ivec2(1, 0), projectionMatrix, zToClip);
    vec3 right = viewPositionAt(p + ivec2(1, 0), projectionMatrix, zToClip) - position;
    vec3 below = position - viewPositionAt(p - ivec2(0, 1), projectionMatrix, zToClip);
    vec3 above = viewPositionAt(p + ivec2(0, 1), projectionMatrix, zToClip) - position;
    vec3 across = abs(left.z) < abs(right.z) ? left : right;
    vec3 up = abs(below.z) < abs(above.z) ? below : above;
    vec3 normal = normalize(cross(across, up));

    if(dot(normal, towardsEye(zToClip, position)) < 0.0)
        normal = -normal;

    float pixelsPerUnit = focalLengthPixels(projectionMatrix) / max(abs(clipWAt(zToClip, position.z)), 0.0001);
    float radiusPixels = radius * pixelsPerUnit;
    float fade = smoothstep(FADE_IN_PIXELS, 2.0 * FADE_IN_PIXELS, radiusPixels);

    if(fade <= 0.0)
    {
        outOcclusion = 1.0;
        return;
    }

    // A radius wider than maximumPixels on screen is narrowed in view space to match
    float sampledRadius = radius * min(maximumPixels / radiusPixels, 1.0);
    radiusPixels = min(radiusPixels, maximumPixels);

    float radiusSquared = sampledRadius * sampledRadius;
    float bias = 0.02 * sampledRadius;
    float epsilon = 0.01 * radiusSquared;

    // Each of a 4 by 4 block of pixels turns its spiral differently, which the
    // second build smooths away
    const int ORDER[16] = int[16](0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5);
    float turn = (float(ORDER[((p.y & 3) * 4) + (p.x & 3)]) + rotation) * (TWO_PI / 16.0);

    float sum = 0.0;

    for(int i = 0; i < NUM_SAMPLES; i++)
    {
        float along = (float(i) + 0.5) / float(NUM_SAMPLES);
        float angle = (along * NUM_SPIRAL_TURNS * TWO_PI) + turn;
        vec2 offset = vec2(cos(angle), sin(angle)) * along * radiusPixels;
        ivec2 q = p + ivec2(floor(offset + 0.5));

        vec3 v = viewPositionAt(q, projectionMatrix, zToClip) - position;
        float vv = dot(v, v);
        float vn = dot(v, normal);

        float falloff = max(radiusSquared - vv, 0.0);
        sum += falloff * falloff * falloff * max((vn - bias) / (epsilon + vv), 0.0);
    }

    // Each sample's part is of the order of the radius to the fifth, so this is
    // the same at any scale
    float r5 = radiusSquared * radiusSquared * sampledRadius;
    float open = max(1.0 - (sum * INTENSITY * 5.0 / (r5 * float(NUM_SAMPLES))), 0.0);

    outOcclusion = mix(1.0, open, fade);
}

#endif
