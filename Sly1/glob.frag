#version 330 core

out vec4 FragColor;

in vec4 color;
in vec3 aNormal;
in vec2 TEXCOORD;

uniform sampler2D Texture;

void main()
{
    FragColor = texture(Texture, TEXCOORD);
}