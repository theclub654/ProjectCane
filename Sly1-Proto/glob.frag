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

in vec4 vertexColor;
in vec2 texcoord;

uniform int fogType;
uniform vec4 fogcolor;
uniform vec4 rgbaCel;

uniform int rko;

in MATERIAL material;
in float fogIntensity;

out vec4 FragColor;

void CullGlob();
void DrawOneWay();
void DrawThreeWay();
void DrawCelBorder();
void ApplyFogPS2();
void ApplyFogPS3();

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

    switch(fogType)
    {
        case FOG_PS2:
        ApplyFogPS2();
        break;

        case FOG_PS3:
        ApplyFogPS3();
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

    FragColor.rgb += shadow.rgb   * material.ambient;
    FragColor.rgb += diffuse.rgb  * material.midtone.rgb;
    FragColor.rgb += saturate.rgb * material.light.rgb;

    FragColor.a = clamp(vertexColor.a * shadow.a * diffuse.a * saturate.a, 0.0, 1.0);
}

void DrawCelBorder()
{
    FragColor = rgbaCel;
}

void ApplyFogPS2()
{
    FragColor.rgb = mix(FragColor.rgb, fogcolor.rgb, clamp(fogcolor.a * fogIntensity, 0.0, 1.0));
}

void ApplyFogPS3()
{
    FragColor.rgb = mix(FragColor.rgb, fogcolor.rgb, fogIntensity);
}