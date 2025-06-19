#version 330 core

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoords;

uniform mat4 u_model;

out vec2 texCoords;

void main()
{
	gl_Position = u_model * vec4(inPos.x, inPos.y, 0.0, 1.0);
	texCoords	= inTexCoords;
}