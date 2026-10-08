#version 410 core

// LOCAL COORDS
in vec3 vPos;
in vec3 vNorm;
out vec3 fPos;
out vec3 fNorm;

uniform mat4 PV;
uniform mat4 W;

void main(void)
{
    // Calculamos la posición en el mundo para la luz 
    fPos = vec3(W * vec4(vPos, 1.0)); 
    
    // Transformamos la normal al mundo 
    fNorm = vec3(W * vec4(vNorm, 0.0));
    fNorm = normalize(fNorm);

    // Ahora multiplicamos PV * W para la posición final
    gl_Position = PV * W * vec4(vPos, 1.0);
}