#version 430 core
#define RKO_OneWay   0
#define RKO_ThreeWay 1

#define FOG_NONE 0
#define FOG_PS2  1
#define FOG_PS3  2

#define MAX_OBJECT_SHADOWS 12

uniform sampler2D ambientMap;
uniform sampler2D diffuseMap;
uniform sampler2D saturateMap;
uniform sampler2DArray shadowTextureArray;

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
    int   textureSlot;
    int   clampS;
    int   clampT;
    int   padTexture2;
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
uniform vec4  projectedVolumeColor;
uniform int   projectedVolumeFinalPass;

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
float SampleShadowTexture(SHADOW shadow, vec2 uv);
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

    if (projectedVolumeFinalPass == 1)
    {
        FragColor = vec4(projectedVolumeColor.rgb * op.darken,
                         clamp(projectedVolumeColor.a * op.uAlpha, 0.0, 1.0));
        return;
    }

    // The middle projected-volume packet has no constant RGBA payload. Its
    // source color and alpha are the VU-generated per-vertex values only.
    // rgbaVolume belongs exclusively to the final untextured packet.
    if (projectedVolumeFinalPass == 2)
    {
        FragColor = vec4(vertexColor.rgb * diffuseTex.rgb * op.darken,
                         clamp(vertexColor.a * diffuseTex.a * op.uAlpha, 0.0, 1.0));
        return;
    }

    FragColor.rgb = (vertexColor.rgb * diffuseTex.rgb) * projectedVolumeColor.rgb * op.darken;

    float alpha = diffuseTex.a * vertexColor.a * projectedVolumeColor.a;

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

    // A caster may disappear between shadow preparation and geometry drawing.
    // Never allow an invalid projection to contaminate the material color.
    if (any(isnan(proj)) || any(isinf(proj)))
        return vec3(1.0);

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

    if (isnan(depth) || isinf(depth) || abs(depth) < 0.00001)
        return vec3(1.0);

    vec2 uv = uvq / depth;

    if (any(isnan(uv)) || any(isinf(uv)))
        return vec3(1.0);
    
    float mask = SampleShadowTexture(shadow, uv);
    float As = mask * rgba.a;

    return (shadow.fDynamic == 1) ? vec3(1.0 - As) : vec3(1.0 + As);
}

float SampleShadowTexture(SHADOW shadow, vec2 uv)
{
    if (shadow.textureSlot < 0)
        return 0.0;

    // A dynamic layer is a single frame-local silhouette render target. It
    // must be transparent beyond that image; repeating it tiles the character
    // shadow over every surface touched by the projector.
    if (shadow.fDynamic != 0 &&
        (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0))
        return 0.0;

    // Bindless textures carried independent GL_REPEAT/GL_CLAMP_TO_EDGE state.
    // The array itself repeats. Clamp exceptional layers to texel centers so
    // linear filtering cannot wrap across the opposite edge.
    vec2 halfTexel = 0.5 / vec2(textureSize(shadowTextureArray, 0).xy);
    if (shadow.fDynamic != 0 || shadow.clampS != 0)
        uv.x = clamp(uv.x, halfTexel.x, 1.0 - halfTexel.x);
    if (shadow.fDynamic != 0 || shadow.clampT != 0)
        uv.y = clamp(uv.y, halfTexel.y, 1.0 - halfTexel.y);

    return texture(shadowTextureArray,
        vec3(uv, float(shadow.textureSlot))).a;
}

void ApplyFog()
{
    FragColor.rgb = mix(FragColor.rgb, swp.fogColor.rgb, fogIntensity);
}
