#version 330 core

#define RKO_OneWay 0
#define RKO_ThreeWay 1
#define RKO_CelBorder 2

#define FOG_PS2 1
#define FOG_PS3 2

uniform sampler2D shadowMap;
uniform sampler2D diffuseMap;
uniform sampler2D saturateMap;

struct MATERIAL
{
    float ambient;
    vec3  midtone;
    vec3  light;
};

uniform vec4 fogcolor;
uniform int fogType;
uniform vec4 rgbaCel;
uniform int rko;

in vec4 vertexColor;
in vec2 texcoord;
in MATERIAL material;
in float fogZ;

out vec4 FragColor;

void CullGlob();
void DrawOneWay();
void DrawThreeWay();
void DrawCelBorder();
void ApplyFog();

void main()
{
    CullGlob();
    FragColor = vec4(0.0);

    switch (rko)
    {
        case RKO_OneWay:
        DrawOneWay();
        break;

        case RKO_ThreeWay:
        DrawThreeWay();
        break;

        case RKO_CelBorder:
        DrawCelBorder();
        break;
    }

    ApplyFog();
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

    FragColor.rgb += shadow.rgb   * material.ambient;
    FragColor.rgb += diffuse.rgb  * material.midtone.rgb;
    FragColor.rgb += saturate.rgb * material.light.rgb;
    
    FragColor.a = clamp(vertexColor.a * shadow.a * diffuse.a * saturate.a, 0.0, 1.0);
}

void DrawCelBorder()
{
    FragColor = rgbaCel;
}

void ApplyFog()
{
    if (fogType == FOG_PS2)
        FragColor.rgb = mix(FragColor.rgb, fogcolor.rgb, clamp(fogcolor.a * fogZ, 0.0, 1.0));
    else if (fogType == FOG_PS3)
        FragColor.rgb = mix(FragColor.rgb, fogcolor.rgb, fogZ);
}