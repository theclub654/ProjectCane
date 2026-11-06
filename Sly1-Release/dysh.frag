#version 430 core

in vec4 vertexColor;
#ifdef DEKO3D
in float vertexAlpha;
flat in float triangleAlpha;
uniform int constantAlpha;
#endif

layout(location = 0) out vec4 FragColor;

void main()
{
#ifdef DEKO3D
    FragColor = vec4(1.0, 1.0, 1.0, constantAlpha != 0 ? triangleAlpha : vertexAlpha);
#else
    FragColor = vec4(1.0, 1.0, 1.0, vertexColor.a);
#endif
}
