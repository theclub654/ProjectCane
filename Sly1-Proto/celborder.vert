#version 330 core

layout (location = 0) in vec3 vertex;

uniform mat4 matWorldToClip;
uniform mat4 model;

void main()
{
	gl_Position = matWorldToClip * model * vec4(vertex, 1.0);
}