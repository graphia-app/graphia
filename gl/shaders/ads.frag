#version 330 core

struct LightInfo
{
    vec4 position;  // Light position in eye coords.
    vec3 intensity; // A,D,S intensity
};
uniform LightInfo light;

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

layout ( location = 0 ) out vec4 outColor;
layout ( location = 1 ) out vec4 outSelection;

vec3 adsModel( const in vec3 pos, const in vec3 norm )
{
    vec3 n = norm;

    // Calculate the vector from the light to the fragment
    vec3 s = normalize( vec3( light.position ) - pos );

    // Calculate the vector from the fragment to the eye position
    // (origin since this is in "eye" or "camera" space)
    vec3 v = normalize( -pos );

    // Reflect the light beam using the normal at this fragment
    vec3 r = reflect( -s, n );

    // Calculate the diffuse component
    float diffuse = max( dot( s, n ), 0.0 );

    // Calculate the specular component
    float specular = 0.0;
    if ( dot( s, n ) > 0.0 )
        specular = pow( max( dot( r, v ), 0.0 ), material.shininess );

    // Combine the ambient, diffuse and specular contributions
    return light.intensity * ( material.ka + vColor * diffuse + material.ks * specular );
}

void main()
{
    vec3 color = adsModel( position, normalize( normal ) );
    outColor = vec4( color, 1.0 );
    outSelection = vOutlineColor;
}
