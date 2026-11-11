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

const int MAX_LIGHTS = 8;

uniform highp int numberOfLights;

struct LightInfo
{
    vec3 position;  // Light position in camera coords
    vec4 color;
};
uniform LightInfo lights[MAX_LIGHTS];

struct MaterialInfo
{
    vec3 ka;            // Ambient reflectivity
    vec3 kd;            // Diffuse reflectivity
    vec3 ks;            // Specular reflectivity
    float shininess;    // Specular shininess factor
};
uniform MaterialInfo material;

// 1 where shading is flat, and 0 where it is lit
uniform float flatness;

const float HALF_PI = 1.5707963;
const float SQRT_TWO_PI = 2.5066283;
const float SQRT_TWO_OVER_PI = 0.7978846;

vec3 lightPosition(int i, vec2 lightPlacement)
{
    vec3 position = lights[i].position * lightPlacement.y;
    position.z -= lightPlacement.x;

    return position;
}

vec3 flattened(vec3 lit, vec3 color)
{
    return (flatness * color) + ((1.0 - flatness) * lit);
}

vec3 adsModel(const in vec3 pos, const in vec3 n, const in vec3 diffuseColor,
    const in vec2 lightPlacement)
{
    vec3 result = vec3(0.0);
    vec3 v = normalize(-pos);

    int minNumberOfLights = min(numberOfLights, MAX_LIGHTS);

    for(int i = 0; i < minNumberOfLights; i++)
    {
        vec3 l = lightPosition(i, lightPlacement) - pos;

        // Calculate the vector from the light to the fragment
        vec3 s = normalize(l);

        // Reflection
        vec3 r = reflect(-s, n);

        // Diffuse
        float diffuse = max(dot(s, n), 0.0);

        // Specular
        float specular = 0.0;
        if(dot(s, n) > 0.0)
            specular = pow(max(dot(r, v), 0.0), material.shininess);

        // Combine the ambient, diffuse and specular contributions
        result += lights[i].color.rgb * (material.ka + diffuseColor.rgb * diffuse + material.ks * specular);
    }

    return result;
}

// The standard normal distribution's cumulative, by its tanh approximation
vec4 normalCumulative(vec4 x)
{
    return 0.5 * (1.0 + tanh(SQRT_TWO_OVER_PI * (x + (0.044715 * x * x * x))));
}

vec2 normalCumulative(vec2 x) { return normalCumulative(vec4(x, 0.0, 0.0)).xy; }

// A cylinder's diffuse term across it, |sIn| cos(angle - toLight) where that is
// lit, integrated from the side up to each of the angles
vec4 diffuseAcrossCylinderTo(vec2 sIn, vec4 angles)
{
    if(length(sIn) <= 0.00001)
        return vec4(0.0);

    float toLight = atan(sIn.x, sIn.y);
    vec4 lit = clamp(angles, max(-HALF_PI, toLight - HALF_PI), min(HALF_PI, toLight + HALF_PI));

    return 0.5 * length(sIn) * ((sin((2.0 * lit) - toLight) * 0.5) + (lit * cos(toLight)));
}

// A cylinder's specular term across it, as a Gaussian about its peak angle
// (a Laplace approximation), whose height is 0 where there is none
struct Highlight
{
    float peak;
    float spread;
    float height;
};

Highlight highlightAcrossCylinder(vec3 s, vec3 v, vec2 sIn, vec2 vIn)
{
    Highlight highlight = Highlight(0.0, 1.0, 0.0);

    // The reflection's alignment with the eye is k + m cos(2 (angle - peak))
    vec2 swing = vec2((sIn.y * vIn.y) - (sIn.x * vIn.x), (sIn.x * vIn.y) + (sIn.y * vIn.x));
    float m = length(swing);
    float highest = min(dot(sIn, vIn) - dot(s, v) + m, 1.0);

    if(highest > 0.0 && m > 0.00001)
    {
        float peak = 0.5 * atan(swing.y, swing.x);

        if(dot(sIn, vec2(sin(peak), cos(peak))) > 0.0)
        {
            highlight.peak = peak;
            highlight.spread = sqrt(highest / (4.0 * material.shininess * m));
            highlight.height = pow(highest, material.shininess);
        }
    }

    return highlight;
}

// The ADS model averaged across a cylinder's width as seen; the normal turns from
// across to -across, and cos(angle) is how much of the width each angle takes
vec3 adsModelAcrossCylinder(const in vec3 pos, const in vec3 across, const in vec3 facing,
    const in vec3 diffuseColor, const in vec2 lightPlacement)
{
    vec3 result = vec3(0.0);
    vec3 v = normalize(-pos);

    int minNumberOfLights = min(numberOfLights, MAX_LIGHTS);

    for(int i = 0; i < minNumberOfLights; i++)
    {
        vec3 s = normalize(lightPosition(i, lightPlacement) - pos);

        // The light and the eye in the plane the normal turns in
        vec2 sIn = vec2(dot(s, across), dot(s, facing));
        vec2 vIn = vec2(dot(v, across), dot(v, facing));

        vec2 diffuseTo = diffuseAcrossCylinderTo(sIn, vec4(-HALF_PI, HALF_PI, 0.0, 0.0)).xy;
        float diffuse = diffuseTo.y - diffuseTo.x;

        float specular = 0.0;
        Highlight highlight = highlightAcrossCylinder(s, v, sIn, vIn);

        if(highlight.height > 0.0)
        {
            vec2 to = normalCumulative((vec2(-HALF_PI, HALF_PI) - highlight.peak) / highlight.spread);
            specular = highlight.height * cos(highlight.peak) * highlight.spread * SQRT_TWO_PI * (to.y - to.x);
        }

        // Both are integrated over a width of 2
        result += lights[i].color.rgb * (material.ka + (((diffuseColor * diffuse) + (material.ks * specular)) * 0.5));
    }

    return result;
}
