#version 330 core

out vec4 FragColor;

in vec3 aNormal;
in vec4 aColor;
in vec2 aTexcoord;

uniform sampler2D Texture;

void main()
{
    FragColor = texture(Texture, aTexcoord);
}