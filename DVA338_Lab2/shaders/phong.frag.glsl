#version 410 core

in vec3 fPos;
in vec3 fNorm;
out vec4 fColor;

// WORLD COORDS
uniform vec3 lightPosition;

uniform vec3 ambientLight;  
uniform vec3 diffuseLight;  
uniform vec3 specularLight; 

uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;

uniform vec3 viewPos;


void main(void)
{
    // CALCULATION OF AMBIENT LAYER
    vec3 ambientLayer = ambientColor * ambientLight;

    // CALCULATION OF DIFFUSE LAYER
    vec3 N = normalize(fNorm);
    vec3 L = normalize(lightPosition - fPos);

    float alignmentDiffuse = max(dot(N, L), 0.0);

    vec3 diffuseLayer = alignmentDiffuse * diffuseColor * diffuseLight;

    // CALCULATION OF SPECULAR LAYER
    vec3 V = normalize(viewPos - fPos);
    vec3 R = reflect(-L, N);

    float aligmentSpecular = max(dot(V, R), 0.0);
    float shining = pow(aligmentSpecular, shininess);
    vec3 specularLayer = shining * specularLight * specularColor;

    vec3 finalColor = ambientLayer + diffuseLayer + specularLayer;
    
    fColor = vec4(finalColor, 1.0);
}