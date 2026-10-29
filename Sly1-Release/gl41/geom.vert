#version 430 core

layout (location = 0) in vec3 vertex;

layout(std140) uniform CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

uniform mat4 modelToClip;

void main()
{
    gl_Position = modelToClip * vec4(vertex, 1.0);;
}