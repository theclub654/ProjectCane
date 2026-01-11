#version 430 core

#define RKO_OneWay   0
#define RKO_ThreeWay 1

uniform sampler2D shadowMap;
uniform sampler2D diffuseMap;
uniform sampler2D saturateMap;

struct SWP // Scene world properties
{
    float uShadow;
    float uMidtone;
    int   fogType;
    float fogNear;
    float fogFar;
    float fogMax;
    vec4  fogColor;
}; uniform SWP swp;

layout(std430, binding = 0) readonly buffer CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

layout(std140, binding = 1) uniform RO // Render object properties
{
    mat4  model;        //   0 -  63
    int   rko;          //  64
    float uAlpha;       //  68
    float uFog;         //  72
    float darken;       //  76
    vec2  uvOffsets;    //  80 -  87
    int   _pad0;        //  88
    int   _pad1;        //  92
    int   warpType;     //  96
    int   warpCmat;     // 100
    int   warpCvtx;     // 104
    int   _padWarp0;    // 108
	int   _padWarp1;    // 112
	int   _padWarp2;    // 116
	int   _padWarp3;    // 120
	int   _padWarp4;    // 124
    mat4  amatDpos[4];  // 128 - 383
    mat4  amatDuv[4];   // 384 - 639
    int   fDynamic;     // 640
    float unSelfIllum;  // 644
    float sRadius;      // 648
    int   _pad2;        // 652
    vec4  posCenter;    // 656 - 671
}op;

struct MATERIAL
{
    float ambient;
    vec3  midtone;
    vec3  light;
};

uniform int fAlphaTest;

in vec4 vertexColor;
in vec2 texcoord;
in MATERIAL material;
in float fogIntensity;

out vec4 FragColor;

void AlphaTest();
void DrawOneWay();
void DrawThreeWay();
void ApplyFog();

void main()
{
    if (fAlphaTest == 1)
        AlphaTest();

    FragColor = vec4(0.0);

    switch (op.rko)
    {
        case RKO_OneWay:
        DrawOneWay();
        break;

        case RKO_ThreeWay:
        DrawThreeWay();
        break;
    }

    if (swp.fogType != 0) 
        ApplyFog();
}

void AlphaTest()
{
    vec4  diffuse = texture(diffuseMap, texcoord);
    float alphaIn = clamp(vertexColor.a * diffuse.a, 0.0, 1.0);

    if (alphaIn < 0.9)
        discard;
}

void DrawOneWay()
{
    vec4 diffuse = texture(diffuseMap, texcoord);

    FragColor.rgb = (vertexColor.rgb * diffuse.rgb) * op.darken;
    FragColor.a = clamp(vertexColor.a * diffuse.a * op.uAlpha, 0.0, 1.0);
}

void DrawThreeWay()
{
    vec4 shadow   = texture(shadowMap,   texcoord);
    vec4 diffuse  = texture(diffuseMap,  texcoord);
    vec4 saturate = texture(saturateMap, texcoord);

    FragColor.rgb = (shadow.rgb * material.ambient + diffuse.rgb * material.midtone.rgb + saturate.rgb * material.light.rgb) * op.darken;
    float finalAlpha = clamp(vertexColor.a * diffuse.a, 0.0, 1.0);
    FragColor.a = clamp(finalAlpha * op.uAlpha, 0.0, 1.0);
}

void ApplyFog()
{
    FragColor.rgb = mix(FragColor.rgb, swp.fogColor.rgb, fogIntensity);
}