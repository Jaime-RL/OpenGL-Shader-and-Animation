#version 420 core

in vec3 vPos;
in vec3 vNorm;
out vec4 color;
out vec4 pos;
uniform mat4 PV;
uniform mat4 W;

void main(void)
{
	color = vec4(vNorm,1);
	pos  = PV * W * vec4(vPos, 1.0f);
	gl_Position = pos;
	
}