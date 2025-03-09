#version 330 core

in vec2 texCoords;

uniform sampler2D screenTexture;

void main()
{ 
    gl_FragColor = texture(screenTexture, texCoords);
}