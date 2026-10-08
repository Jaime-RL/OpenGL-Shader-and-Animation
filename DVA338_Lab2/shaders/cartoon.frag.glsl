#version 410 core

in vec3 fPos;
in vec3 fNorm;
out vec4 fColor;

// WORLD COORDS
uniform vec3 lightPosition;

uniform vec3 ambientLight;
uniform vec3 diffuseLight;  

uniform vec3 ambientColor;
uniform vec3 diffuseColor;

void main(void)
{

    // CALCULATION OF DIFFUSE LAYER
    vec3 N = normalize(fNorm);
    vec3 L = normalize(lightPosition - fPos);

    float alignment = max(dot(N, L), 0.0);
    float levels;

    if (alignment > 0.8)      levels = 1.0;
    else if (alignment > 0.6) levels = 0.8;
    else if (alignment > 0.4) levels = 0.6;
    else if (alignment > 0.2) levels = 0.4;
    else                      levels = 0.2;

    // CALCULATION OF AMBIENT LAYER
    vec3 ambientLayer = ambientLight * ambientColor;

    vec3 combinedLayer = (diffuseColor * diffuseLight) + ambientLayer;
    vec3 finalColor = combinedLayer * levels;

    fColor = vec4(finalColor, 1.0);
}