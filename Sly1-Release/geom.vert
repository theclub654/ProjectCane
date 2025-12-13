#version 430 core

layout (location = 0) in vec3 vertex;

layout(std430, binding = 0) readonly buffer CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

layout(std140, binding = 1)  uniform ROGEOM
{
    mat4 model;
}op;

void main()
{
    gl_Position = cm.matWorldToClip * op.model * vec4(vertex, 1.0);;
}