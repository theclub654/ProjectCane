#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 texcoord;

out vec3 aNormal;
out vec4 aVertexColor;
out vec2 aTexcoord;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = proj * view * model * vec4(pos, 1.0);

    aNormal      = normal;
    aVertexColor = color;
    aTexcoord    = texcoord;
}