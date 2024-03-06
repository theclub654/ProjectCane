#version 330 core

out vec4 FragColor;

in vec3 aNormal;
in vec4 aColor;
in vec2 aTexcoord;

in vec4 result;

uniform sampler2D diffuseTexture;

void main()
{
    vec4 texColor = texture(diffuseTexture, aTexcoord);

    if (texColor.a < 0.1)
        discard;
        
    FragColor = texColor;
}