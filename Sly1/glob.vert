#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in int color;
layout (location = 3) in vec2 texCoord;

out vec4 vertexColor;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = proj * view * model * vec4(pos.z, pos.y, pos.x, 1.0);
    vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
}