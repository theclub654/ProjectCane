#version 330 core

#define RKO_OneWay 0
#define RKO_ThreeWay 1
#define RKO_CelBorder 2

uniform sampler2D shadowMap;
uniform sampler2D diffuseMap;
uniform sampler2D saturateMap;

in vec4 vertexColor;
in vec2 texcoord;

in vec4 ambient;
in vec4 midtone;
in vec4 light;

uniform vec4 rgbaCel;

uniform int rko;

out vec4 FragColor;

void CullGlob();
void DrawOneWay();
void DrawThreeWay();
void DrawCelBorder();

void main()
{
    FragColor = vec4(0.0);
    
    switch (rko)
    {
        case RKO_OneWay:
        CullGlob();
        DrawOneWay();
        break;

        case RKO_ThreeWay:
        CullGlob();
        DrawThreeWay();
        break;

        case RKO_CelBorder:
        CullGlob();
        DrawCelBorder();
        break;
    }
}

void CullGlob()
{
    if (!gl_FrontFacing)
         discard;
}

void DrawOneWay()
{
    vec4 diffuse = texture(diffuseMap, texcoord);

    FragColor = diffuse * vertexColor;
}

void DrawThreeWay()
{
    vec4 shadow   = texture(shadowMap,   texcoord);
    vec4 diffuse  = texture(diffuseMap,  texcoord);
    vec4 saturate = texture(saturateMap, texcoord);

    FragColor += shadow   * ambient;
    FragColor += diffuse  * midtone;
    FragColor += saturate * light;

    FragColor.a = vertexColor.a * shadow.a * diffuse.a * saturate.a;
}

void DrawCelBorder()
{
    FragColor = rgbaCel;
}