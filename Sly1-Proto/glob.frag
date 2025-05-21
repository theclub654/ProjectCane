#version 330 core

#define RKO_OneWay 0
#define RKO_ThreeWay 1
#define RKO_CelBorder 2
#define RKO_Collision 3

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

uniform int  fogType;
uniform vec4 fogColor;
uniform vec4 rgbaCel;

uniform float uAlpha;
uniform float rDarken;

uniform int rko;

uniform int fAlphaTest;

uniform vec4 collisionRgba;

in MATERIAL material;
in float fogIntensity;

out vec4 FragColor;

void CullGlob();
void DrawOneWay();
void DrawThreeWay();
void DrawCelBorder();
void DrawCollision();
void ApplyFog();

void main()
{
    CullGlob();

    FragColor = vec4(0.0);
    switch (rko)
    {
        case RKO_OneWay:
        DrawOneWay();

        if (fogType != 0)
            ApplyFog();
        break;

        case RKO_ThreeWay:
        DrawThreeWay();

        if (fogType != 0)
            ApplyFog();
        break;

        case RKO_CelBorder:
        DrawCelBorder();
        break;

        case RKO_Collision:
        DrawCollision();
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

    FragColor.a = clamp(FragColor.a * uAlpha, 0.0, 1.0);

    if (fAlphaTest == 1 && FragColor.a < 0.9)
        discard;
}

void DrawThreeWay()
{
    vec4 shadow   = texture(shadowMap,   texcoord);
    vec4 diffuse  = texture(diffuseMap,  texcoord);
    vec4 saturate = texture(saturateMap, texcoord);

    FragColor.rgb += shadow.rgb   * material.ambient * rDarken;
    FragColor.rgb += diffuse.rgb  * material.midtone.rgb * rDarken;
    FragColor.rgb += saturate.rgb * material.light.rgb;

    float finalAlpha = vertexColor.a * shadow.a * diffuse.a * saturate.a;
   
    FragColor.a = clamp(finalAlpha * uAlpha, 0.0, 1.0);

    if (fAlphaTest == 1 && FragColor.a < 0.9)
        discard;
}

void DrawCelBorder()
{
    FragColor = rgbaCel;
}

void DrawCollision()
{
    FragColor = collisionRgba;
}

void ApplyFog()
{
    FragColor.rgb = mix(FragColor.rgb, fogColor.rgb, fogIntensity);
}