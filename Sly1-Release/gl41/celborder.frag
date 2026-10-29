#version 430 core

layout(std140) uniform ROCEL
{
    mat4  model;                 
	vec4  celRgba;         
	float uAlphaCelBorder;
}cbp;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor.rgb = cbp.celRgba.rgb;
	FragColor.a = clamp(cbp.celRgba.a * cbp.uAlphaCelBorder, 0.0, 1.0);
}
