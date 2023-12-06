#version 330 core

out vec4 FragColor;

in vec4 color;
in vec3 aNormal;

void main()
{
    FragColor = vec4(aNormal, 1.0);
}