#version 430 core

layout(location = 0) in vec4 vertexColor;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, vertexColor.a);
}
