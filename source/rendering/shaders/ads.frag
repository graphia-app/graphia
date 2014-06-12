#version 330 core

const int MAX_LIGHTS = 8;
uniform int numberOfLights;

struct LightInfo
{
    vec4 position;  // Light position in eye coords.
    vec3 intensity; // A,D,S intensity
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

in vec3 position;
in vec3 normal;
in vec3 vColor;
in vec4 vOutlineColor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outSelection;

vec3 adsModel(const in vec3 pos, const in vec3 n)
{
    vec3 result = vec3(0.0);
    vec3 v = normalize(-pos);

    int minNumberOfLights = min(numberOfLights, MAX_LIGHTS);

    for(int i = 0; i < minNumberOfLights; i++)
    {
        LightInfo light = lights[i];

        vec3 l = vec3(light.position) - pos;
        float d = length(l);

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

        float kc = 1.0;
        float kl = 0.0 * d;
        float kq = 0.0002 * d * d;
        float attenuation = 1.0 / (kc + kl + kq);
        attenuation = clamp(attenuation, 0.5, 1.0);

        // Combine the ambient, diffuse and specular contributions
        result += attenuation * light.intensity * (material.ka + vColor * diffuse + material.ks * specular);
    }

    return result;
}

void main()
{
    vec3 color = adsModel(position, normalize(normal));
    outColor = vec4(color, 1.0);
    outSelection = vOutlineColor;
}
