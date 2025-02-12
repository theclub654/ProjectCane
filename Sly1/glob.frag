#version 330 core

#define TWPS_Shadow 0
#define TWPS_ShadowMidtone 1
#define TWPS_ShadowMidtoneSaturate 2

#define SHDK_ThreeWay 0
#define SHDK_Prelit 1
#define SHDK_Shadow 2
#define SHDK_SpotLight 3
#define SHDK_ProjectedVolume 4
#define SHDK_CreateTexture 5
#define SHDK_Background 6
#define SHDK_Foreground 7
#define SHDK_WorldMap 8
#define SHDK_MurkClear 9
#define SHDK_MurkFill 10
#define SHDK_Max 11

uniform sampler2D shadowMap;
uniform sampler2D diffuseMap;
uniform sampler2D saturateMap;

in vec4 vertexColor;
in vec2 texcoord;

uniform int shdk;
uniform int twps;

in vec4 ambient;
in vec4 midtone;
in vec4 light;

void DrawThreeWay();
void DrawPrelit();
void DrawVolume();

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0);
    
    switch (shdk)
    {
        case SHDK_ThreeWay:
        DrawThreeWay();
        break;

        case SHDK_Prelit:
        case SHDK_SpotLight:
        case SHDK_Background:
        case SHDK_CreateTexture:
        case SHDK_Foreground:
        case SHDK_WorldMap:
        case SHDK_MurkClear:
        case SHDK_MurkFill:
        case SHDK_Max:
        DrawPrelit();
        break;

        case SHDK_ProjectedVolume:
        DrawVolume();
        break;
    }
}

void DrawThreeWay()
{
    int tempTwps = 2;
    
    vec4 shadow   = texture(shadowMap,   texcoord);
    vec4 diffuse  = texture(diffuseMap,  texcoord);
    vec4 saturate = texture(saturateMap, texcoord);

    switch (tempTwps)
    {
        case TWPS_Shadow:
        FragColor += shadow * ambient;

        FragColor.a = shadow.a * vertexColor.a;
        break;
    
        case TWPS_ShadowMidtone:
        FragColor += shadow  * ambient;
        FragColor += diffuse * midtone;

        FragColor.a = shadow.a * diffuse.a * vertexColor.a;
        break;
    
        case TWPS_ShadowMidtoneSaturate:
        FragColor += shadow   * ambient;
        FragColor += diffuse  * midtone;
        FragColor += saturate * light;

        FragColor.a = vertexColor.a * shadow.a * diffuse.a * saturate.a;

        break;
    }
}

void DrawPrelit()
{
    vec4 diffuse = texture(diffuseMap, texcoord);

    FragColor = diffuse * vertexColor;
}

void DrawVolume()
{
    vec4 diffuse = texture(diffuseMap, texcoord);

    FragColor = diffuse * vertexColor;
}