#version 330 core

const int MAX_LIGHTS = 8;

uniform int numberOfLights;

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

uniform float flatness;

flat in float element;
in vec3 vPosition;
in vec3 vNormal;
in vec3 innerVColor;
in vec3 outerVColor;
in float vSelected;
in vec2 uv;
in float lightOffset;
in float lightScale;
in float projectionScale;

layout (location = 0) out vec4  outColor;
layout (location = 1) out vec2  outElement;
layout (location = 2) out vec4  outSelection;

vec3 adsModel(const in vec3 pos, const in vec3 n, const in vec3 diffuseColor)
{
    vec3 result = vec3(0.0);
    vec3 v = normalize(-pos);

    int minNumberOfLights = min(numberOfLights, MAX_LIGHTS);

    for(int i = 0; i < minNumberOfLights; i++)
    {
        LightInfo light = lights[i];
        light.position *= lightScale;
        light.position.z -= lightOffset;

        vec3 l = vec3(light.position) - pos;

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
        result += light.color.rgb * (material.ka + diffuseColor.rgb * diffuse + material.ks * specular);
    }

    return result;
}

void main()
{
    // Only the UV y is set since that's all we're interested in
    // Used to set the size of the centre stripe
    const float bounds = 0.375;

    float stepMix = step(bounds, uv.y) * step(uv.y, 1.0 - bounds);
    vec3 fragColor = mix(innerVColor, outerVColor, 1.0 - stepMix);
    vec3 normal = normalize(vNormal);
    vec3 color = (flatness * fragColor) +
        ((1.0 - flatness) * adsModel(vPosition, normal, fragColor));

    outColor = vec4(color, 1.0);
    outElement.r = element;
    outElement.g = projectionScale;
    outSelection = vec4(vec3(vSelected), 1.0);
}
