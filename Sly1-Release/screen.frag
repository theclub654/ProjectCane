#version 430 core

layout(location = 0) in vec2 vUV;
uniform sampler2D screenTexture;

layout(location = 0) out vec4 FragColor;

void main() 
{
    FragColor = texture(screenTexture, vUV);
}
