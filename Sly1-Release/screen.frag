#version 430 core

layout(location = 0) in vec2 vUV;
uniform sampler2D screenTexture;
uniform float outputGamma;

layout(location = 0) out vec4 FragColor;

void main() 
{
    vec4 color = texture(screenTexture, vUV);
    color.rgb = pow(max(color.rgb, vec3(0.0)), vec3(1.0 / max(outputGamma, 0.01)));
    FragColor = color;
}
