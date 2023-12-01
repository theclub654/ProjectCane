#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 vertexColor;
layout (location = 3) in vec2 texCoord;

out vec4 color;
out vec3 aNormal;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = proj * view * model * vec4(pos.y, pos.x, pos.z, 1.0);
    color = vertexColor / 255.0;
    aNormal = normal;
}