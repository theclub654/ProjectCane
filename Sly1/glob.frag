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

in vec4 ambient;
in vec4 illumination;
in vec4 light;

void DrawThreeWay();
void DrawPrelit();
void DrawVolume();

void main()
{
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
    vec4 pixel = vec4(0.0);
    int twps = 2;

    vec4 shadow   = texture2D(shadowMap,   texcoord);
    vec4 diffuse  = texture2D(diffuseMap,  texcoord);
    vec4 saturate = texture2D(saturateMap, texcoord);

    switch (twps)
    {
        case TWPS_Shadow:
        pixel += shadow * ambient;

        pixel.a = vertexColor.a;
        break;
    
        case TWPS_ShadowMidtone:
        pixel += shadow  * ambient;
        pixel += diffuse * illumination;

        pixel.a = vertexColor.a;
        break;
    
        case TWPS_ShadowMidtoneSaturate:
        pixel += shadow   * ambient;
        pixel += diffuse  * illumination;
        pixel += saturate * light;

        pixel.a = vertexColor.a;
        break;
    }

    gl_FragColor = pixel;
}

void DrawPrelit()
{
    vec4 pixel = vec4(0.0);

    vec4 diffuseTexel  = texture(diffuseMap, texcoord);
    
    pixel   = diffuseTexel * vertexColor;
    pixel.a = diffuseTexel.a;

    gl_FragColor = pixel;
}

void DrawVolume()
{
    vec4 pixel = vec4(0.0);

    vec4 diffuseTexel = texture(diffuseMap, texcoord);
    pixel = diffuseTexel;

    gl_FragColor = pixel;
}