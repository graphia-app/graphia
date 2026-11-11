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

// SMAA's blending weight calculation, transcribed from SMAA.hlsl at its ultra settings

#ifdef GL_ES
// Fractions of a pixel across the whole image need more than medium precision
precision highp float;
#endif

layout (location = 0) out vec4 fragWeights;

uniform sampler2D edges;
uniform sampler2D areaTable;
uniform sampler2D searchTable;

uniform vec4 imageMetrics; // 1 / width, 1 / height, width, height

const int MAX_SEARCH_STEPS = 32;
const int MAX_SEARCH_STEPS_DIAGONAL = 16;
const float CORNER_ROUNDING = 0.25;

const float AREA_TABLE_MAX_DISTANCE = 16.0;
const float AREA_TABLE_MAX_DISTANCE_DIAGONAL = 20.0;
const vec2 AREA_TABLE_PIXEL_SIZE = 1.0 / vec2(160.0, 560.0);
const float AREA_TABLE_SUBTABLE_SIZE = 1.0 / 7.0;
const vec2 SEARCH_TABLE_SIZE = vec2(66.0, 33.0);
const vec2 SEARCH_TABLE_PACKED_SIZE = vec2(64.0, 16.0);

void movc(bvec2 condition, inout vec2 variable, vec2 value)
{
    if(condition.x) variable.x = value.x;
    if(condition.y) variable.y = value.y;
}

vec2 edgesAt(vec2 coord)
{
    return textureLod(edges, coord, 0.0).rg;
}

vec2 edgesAtOffset(vec2 coord, ivec2 offset)
{
    return textureLod(edges, coord + (vec2(offset) * imageMetrics.xy), 0.0).rg;
}

// A diagonal fetch lands a quarter of a texel off, so that one bilinear read
// returns two edges; these separate them again
vec2 decodeDiagonalBilinearAccess(vec2 e)
{
    e.r = e.r * abs((5.0 * e.r) - (5.0 * 0.75));
    return floor(e + 0.5);
}

vec4 decodeDiagonalBilinearAccess(vec4 e)
{
    e.rb = e.rb * abs((5.0 * e.rb) - (5.0 * 0.75));
    return floor(e + 0.5);
}

vec2 searchDiagonal1(vec2 coord, vec2 direction, out vec2 e)
{
    vec4 c = vec4(coord, -1.0, 1.0);
    vec3 t = vec3(imageMetrics.xy, 1.0);

    e = vec2(0.0);

    while(c.z < float(MAX_SEARCH_STEPS_DIAGONAL - 1) && c.w > 0.9)
    {
        c.xyz = (t * vec3(direction, 1.0)) + c.xyz;
        e = edgesAt(c.xy);
        c.w = dot(e, vec2(0.5));
    }

    return c.zw;
}

vec2 searchDiagonal2(vec2 coord, vec2 direction, out vec2 e)
{
    vec4 c = vec4(coord, -1.0, 1.0);
    c.x += 0.25 * imageMetrics.x;
    vec3 t = vec3(imageMetrics.xy, 1.0);

    e = vec2(0.0);

    while(c.z < float(MAX_SEARCH_STEPS_DIAGONAL - 1) && c.w > 0.9)
    {
        c.xyz = (t * vec3(direction, 1.0)) + c.xyz;
        e = decodeDiagonalBilinearAccess(edgesAt(c.xy));
        c.w = dot(e, vec2(0.5));
    }

    return c.zw;
}

vec2 areaDiagonal(vec2 dist, vec2 e, float offset)
{
    vec2 coord = (vec2(AREA_TABLE_MAX_DISTANCE_DIAGONAL) * e) + dist;

    coord = (AREA_TABLE_PIXEL_SIZE * coord) + (0.5 * AREA_TABLE_PIXEL_SIZE);

    // The diagonal areas are the right hand half of the table
    coord.x += 0.5;
    coord.y += AREA_TABLE_SUBTABLE_SIZE * offset;

    return textureLod(areaTable, coord, 0.0).rg;
}

vec2 calculateDiagonalWeights(vec2 coord, vec2 e)
{
    vec2 weights = vec2(0.0);

    vec4 d;
    vec2 end;

    if(e.r > 0.0)
    {
        d.xz = searchDiagonal1(coord, vec2(-1.0, 1.0), end);
        d.x += end.y > 0.9 ? 1.0 : 0.0;
    }
    else
        d.xz = vec2(0.0);

    d.yw = searchDiagonal1(coord, vec2(1.0, -1.0), end);

    if((d.x + d.y) > 2.0)
    {
        vec4 coords = (vec4(-d.x + 0.25, d.x, d.y, -d.y - 0.25) * imageMetrics.xyxy) + coord.xyxy;
        vec4 c;
        c.xy = edgesAtOffset(coords.xy, ivec2(-1, 0));
        c.zw = edgesAtOffset(coords.zw, ivec2(1, 0));
        c.yxwz = decodeDiagonalBilinearAccess(c.xyzw);

        vec2 cc = (vec2(2.0) * c.xz) + c.yw;

        movc(bvec2(step(0.9, d.zw)), cc, vec2(0.0));

        weights += areaDiagonal(d.xy, cc, 0.0);
    }

    d.xz = searchDiagonal2(coord, vec2(-1.0, -1.0), end);

    if(edgesAtOffset(coord, ivec2(1, 0)).r > 0.0)
    {
        d.yw = searchDiagonal2(coord, vec2(1.0), end);
        d.y += end.y > 0.9 ? 1.0 : 0.0;
    }
    else
        d.yw = vec2(0.0);

    if((d.x + d.y) > 2.0)
    {
        vec4 coords = (vec4(-d.x, -d.x, d.y, d.y) * imageMetrics.xyxy) + coord.xyxy;
        vec4 c;
        c.x = edgesAtOffset(coords.xy, ivec2(-1, 0)).g;
        c.y = edgesAtOffset(coords.xy, ivec2(0, -1)).r;
        c.zw = edgesAtOffset(coords.zw, ivec2(1, 0)).gr;
        vec2 cc = (vec2(2.0) * c.xz) + c.yw;

        movc(bvec2(step(0.9, d.zw)), cc, vec2(0.0));

        weights += areaDiagonal(d.xy, cc, 0.0).gr;
    }

    return weights;
}

// How much further than its last step a search reached, which the pattern of
// edges it stopped on decides; the table's halves are left and right, and its rows run upside down
float searchLength(vec2 e, float offset)
{
    vec2 scale = SEARCH_TABLE_SIZE * vec2(0.5, -1.0);
    vec2 bias = SEARCH_TABLE_SIZE * vec2(offset, 1.0);

    scale += vec2(-1.0, 1.0);
    bias += vec2(0.5, -0.5);

    scale *= 1.0 / SEARCH_TABLE_PACKED_SIZE;
    bias *= 1.0 / SEARCH_TABLE_PACKED_SIZE;

    return textureLod(searchTable, (scale * e) + bias, 0.0).r;
}

float searchXLeft(vec2 coord, float end)
{
    vec2 e = vec2(0.0, 1.0);

    while(coord.x > end && e.g > 0.8281 && e.r == 0.0)
    {
        e = edgesAt(coord);
        coord = (-vec2(2.0, 0.0) * imageMetrics.xy) + coord;
    }

    float offset = (-(255.0 / 127.0) * searchLength(e, 0.0)) + 3.25;
    return (imageMetrics.x * offset) + coord.x;
}

float searchXRight(vec2 coord, float end)
{
    vec2 e = vec2(0.0, 1.0);

    while(coord.x < end && e.g > 0.8281 && e.r == 0.0)
    {
        e = edgesAt(coord);
        coord = (vec2(2.0, 0.0) * imageMetrics.xy) + coord;
    }

    float offset = (-(255.0 / 127.0) * searchLength(e, 0.5)) + 3.25;
    return (-imageMetrics.x * offset) + coord.x;
}

float searchYUp(vec2 coord, float end)
{
    vec2 e = vec2(1.0, 0.0);

    while(coord.y > end && e.r > 0.8281 && e.g == 0.0)
    {
        e = edgesAt(coord);
        coord = (-vec2(0.0, 2.0) * imageMetrics.xy) + coord;
    }

    float offset = (-(255.0 / 127.0) * searchLength(e.gr, 0.0)) + 3.25;
    return (imageMetrics.y * offset) + coord.y;
}

float searchYDown(vec2 coord, float end)
{
    vec2 e = vec2(1.0, 0.0);

    while(coord.y < end && e.r > 0.8281 && e.g == 0.0)
    {
        e = edgesAt(coord);
        coord = (vec2(0.0, 2.0) * imageMetrics.xy) + coord;
    }

    float offset = (-(255.0 / 127.0) * searchLength(e.gr, 0.5)) + 3.25;
    return (-imageMetrics.y * offset) + coord.y;
}

vec2 area(vec2 dist, float e1, float e2, float offset)
{
    // Rounded, so that the bilinear read lands squarely on the entry meant
    vec2 coord = (vec2(AREA_TABLE_MAX_DISTANCE) * floor((4.0 * vec2(e1, e2)) + 0.5)) + dist;

    coord = (AREA_TABLE_PIXEL_SIZE * coord) + (0.5 * AREA_TABLE_PIXEL_SIZE);
    coord.y = (AREA_TABLE_SUBTABLE_SIZE * offset) + coord.y;

    return textureLod(areaTable, coord, 0.0).rg;
}

// Corner patterns keep a silhouette's corners sharp
void detectHorizontalCornerPattern(inout vec2 weights, vec4 coord, vec2 d)
{
    vec2 leftRight = step(d.xy, d.yx);
    vec2 rounding = (1.0 - CORNER_ROUNDING) * leftRight;

    rounding /= leftRight.x + leftRight.y;

    vec2 factor = vec2(1.0);
    factor.x -= rounding.x * edgesAtOffset(coord.xy, ivec2(0, 1)).r;
    factor.x -= rounding.y * edgesAtOffset(coord.zw, ivec2(1, 1)).r;
    factor.y -= rounding.x * edgesAtOffset(coord.xy, ivec2(0, -2)).r;
    factor.y -= rounding.y * edgesAtOffset(coord.zw, ivec2(1, -2)).r;

    weights *= clamp(factor, 0.0, 1.0);
}

void detectVerticalCornerPattern(inout vec2 weights, vec4 coord, vec2 d)
{
    vec2 leftRight = step(d.xy, d.yx);
    vec2 rounding = (1.0 - CORNER_ROUNDING) * leftRight;

    rounding /= leftRight.x + leftRight.y;

    vec2 factor = vec2(1.0);
    factor.x -= rounding.x * edgesAtOffset(coord.xy, ivec2(1, 0)).g;
    factor.x -= rounding.y * edgesAtOffset(coord.zw, ivec2(1, 1)).g;
    factor.y -= rounding.x * edgesAtOffset(coord.xy, ivec2(-2, 0)).g;
    factor.y -= rounding.y * edgesAtOffset(coord.zw, ivec2(-2, 1)).g;

    weights *= clamp(factor, 0.0, 1.0);
}

void main()
{
    vec2 texcoord = gl_FragCoord.xy * imageMetrics.xy;
    vec2 pixcoord = texcoord * imageMetrics.zw;

    vec4 weights = vec4(0.0);

    // A little off centre, so that one bilinear read says what two neighbouring edges are doing
    vec4 offset0 = (imageMetrics.xyxy * vec4(-0.25, -0.125, 1.25, -0.125)) + texcoord.xyxy;
    vec4 offset1 = (imageMetrics.xyxy * vec4(-0.125, -0.25, -0.125, 1.25)) + texcoord.xyxy;
    vec4 offset2 = (imageMetrics.xxyy * (vec4(-2.0, 2.0, -2.0, 2.0) * float(MAX_SEARCH_STEPS))) +
        vec4(offset0.xz, offset1.yw);

    vec2 e = edgesAt(texcoord);

    if(e.g > 0.0)
    {
        weights.rg = calculateDiagonalWeights(texcoord, e);

        // Only where no diagonal was found is the horizontal run followed
        if(weights.r == -weights.g)
        {
            vec2 d;
            vec3 coords;

            coords.x = searchXLeft(offset0.xy, offset2.x);
            coords.y = offset1.y;
            d.x = coords.x;

            float e1 = textureLod(edges, coords.xy, 0.0).r;

            coords.z = searchXRight(offset0.zw, offset2.y);
            d.y = coords.z;

            d = abs(floor(((imageMetrics.zz * d) - pixcoord.xx) + 0.5));

            // The table is packed by the square root of the distance
            vec2 sqrtD = sqrt(d);

            float e2 = edgesAtOffset(coords.zy, ivec2(1, 0)).r;

            weights.rg = area(sqrtD, e1, e2, 0.0);

            coords.y = texcoord.y;
            detectHorizontalCornerPattern(weights.rg, coords.xyzy, d);
        }
        else
            e.r = 0.0;
    }

    if(e.r > 0.0)
    {
        vec2 d;
        vec3 coords;

        coords.y = searchYUp(offset1.xy, offset2.z);
        coords.x = offset0.x;
        d.x = coords.y;

        float e1 = textureLod(edges, coords.xy, 0.0).g;

        coords.z = searchYDown(offset1.zw, offset2.w);
        d.y = coords.z;

        d = abs(floor(((imageMetrics.ww * d) - pixcoord.yy) + 0.5));

        vec2 sqrtD = sqrt(d);

        float e2 = edgesAtOffset(coords.xz, ivec2(0, 1)).g;

        weights.ba = area(sqrtD, e1, e2, 0.0);

        coords.x = texcoord.x;
        detectVerticalCornerPattern(weights.ba, coords.xyxz, d);
    }

    fragWeights = weights;
}
