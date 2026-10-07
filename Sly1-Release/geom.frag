#version 430 core

layout(location = 0) out vec4 FragColor;

uniform vec4 debugColor;

void main()
{
	FragColor = debugColor;
}
