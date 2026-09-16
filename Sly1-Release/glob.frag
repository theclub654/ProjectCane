#version 430 core
#extension GL_ARB_bindless_texture : require

#define RKO_OneWay   0
#define RKO_ThreeWay 1

#define FOG_NONE 0
#define FOG_PS2  1
#define FOG_PS3  2

#define MAX_OBJECT_SHADOWS 16

uniform sampler2D ambientMap;
uniform sampler2D diffuseMap;
uniform sampler2D saturateMap;

struct SWP
{
    float uShadow;
    float uMidtone;
    int   fogType;
    float fogNear;
    float fogFar;
    float fogMax;
    vec4  fogColor;
};
uniform SWP swp;

struct SHADOW
{
    mat4  matWorldToUv;
    mat4  matClipToUv;
    vec4  rgba;
    float wMin;
    float wMax;
    float gReserved;
    float wFadeMin;
    uvec2 textureHandle;
    uvec2 _pad0;
    vec4  posEffect;
    float sRadiusEffect;
    int   fDynamic;
    int   shdk;
    int   _pad1;
    vec4  normalCast;
};

struct MATERIAL
{
    float ambient;
    vec4  midtone;
    vec4  light;
};

layout(std140, binding = 1) uniform RO
{
    mat4  model;
    float uAlpha;
    float uFog;
    float darken;
    int   grfglob;
    int   blotTvLight;
    int   warpType;
    int   warpCmat;
    int   warpCvtx;
    mat4  amatDpos[4];
    mat4  amatDuv[4];
    int   fDynamic;
    float sRadius;
    int   fDynamicLight;
    int   trlk;
    vec4  posCenter;
} op;

layout(std430, binding = 4) readonly buffer SHADOWBLK
{
    int numLevelShadows;
    int padShadowBlk[3];
    SHADOW shadows[];
};

uniform int   fAlphaTest;
uniform float alphaCutOff;
uniform int   rko;

in vec4 worldPos;
in vec3 worldNormal;
in vec4 vertexColor;
in vec2 texcoord;

in MATERIAL material;
flat in int vShadowCount;
flat in int vShadowIndices[MAX_OBJECT_SHADOWS];
in float fogIntensity;

out vec4 FragColor;

void AlphaTest();
void DrawOneWay();
void DrawThreeWay();
bool ShadowIntersectsSphere(vec3 objectCenter, float objectRadius, vec3 shadowCenter, float shadowRadius);
vec3 ApplyProjectedShadow(SHADOW shadow);
void ApplyFog();

void main()
{
    if (fAlphaTest == 1)
        AlphaTest();

    FragColor = vec4(0.0);

    switch (rko)
    {
        case RKO_OneWay:
        DrawOneWay();
        break;

        case RKO_ThreeWay:
        DrawThreeWay();
        break;
    }

    if (swp.fogType != FOG_NONE)
        ApplyFog();
}

void AlphaTest()
{
    vec4  diffuse = texture(diffuseMap, texcoord);
    float alphaIn = diffuse.a * vertexColor.a;

    if (alphaIn < alphaCutOff)
        discard;
}

void DrawOneWay()
{
    vec4 diffuseTex = texture(diffuseMap, texcoord);

    FragColor.rgb = (vertexColor.rgb * diffuseTex.rgb) * op.darken;

    float alpha = diffuseTex.a * vertexColor.a;

    if (alpha > 0.9)
    {
        for (int i = 0; i < vShadowCount; ++i)
        {
            int idx = vShadowIndices[i];
            FragColor.rgb *= ApplyProjectedShadow(shadows[idx]);
        }
    }

    FragColor.a = clamp(alpha * op.uAlpha, 0.0, 1.0);
}

void DrawThreeWay()
{
    vec4 ambientTex  = texture(ambientMap,  texcoord);
    vec4 diffuseTex  = texture(diffuseMap,  texcoord);
    vec4 saturateTex = texture(saturateMap, texcoord);

    vec3  lit = ambientTex.rgb * material.ambient + diffuseTex.rgb * material.midtone.rgb + saturateTex.rgb * material.light.rgb;
    float alpha = clamp(vertexColor.a * diffuseTex.a, 0.0, 1.0);

    if (alpha > 0.9)
    {
        for (int i = 0; i < vShadowCount; ++i)
        {
            int idx = vShadowIndices[i];
            lit.rgb *= ApplyProjectedShadow(shadows[idx]);
        }
    }

    FragColor.rgb = lit * op.darken;
    FragColor.a = clamp(alpha * op.uAlpha, 0.0, 1.0);
}

bool ShadowIntersectsSphere(vec3 objectCenter, float objectRadius, vec3 shadowCenter, float shadowRadius)
{
    vec3  d = objectCenter - shadowCenter;
    float r = objectRadius + shadowRadius;

    return dot(d, d) <= r * r;
}

vec3 ApplyProjectedShadow(SHADOW shadow)
{
    vec4 proj = shadow.matWorldToUv * vec4(worldPos.xyz, 1.0);

    vec4 rgba = clamp(shadow.rgba, 0.0, 1.0);

    float q = proj.w;
    float depth = proj.w;
    float oldDepth = depth;

    vec2 uvq = proj.xy;

    if (shadow.wMax < depth)
    {
        float dw = shadow.wMax - depth;
        float duv = dw * 0.5;

        uvq.x += duv;
        uvq.y += duv;
        depth += dw;
    }
    else if (depth < shadow.wMin)
    {
        float dw = shadow.wMin - depth;
        float duv = dw * 0.5;

        uvq.x += duv;
        uvq.y += duv;
        depth += dw;

        if (oldDepth < shadow.wFadeMin)
            rgba.a = 0.0;
    }

    vec2 uv = uvq / depth;
    
    float mask = texture(sampler2D(shadow.textureHandle), uv).a;
    float As = mask * rgba.a;

    return (shadow.fDynamic == 1) ? vec3(1.0 - As) : vec3(1.0 + As);
}

void ApplyFog()
{
    FragColor.rgb = mix(FragColor.rgb, swp.fogColor.rgb, fogIntensity);
}
