#version 430 core

#define RKO_ThreeWay  0
#define RKO_OneWay    1

#define LIGHTK_Direction 0
#define LIGHTK_Position  1
#define LIGHTK_Frustrum  2
#define LIGHTK_Spot      3

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
}; 
uniform SWP swp;

// Camera block (kept as you had it)
layout(std140) uniform CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

// Render object properties (must match vertex shader)
layout(std140) uniform RO
{
    mat4  model;           // 0..63
	int   rko;             // 64
	float uAlpha;          // 68
	float uFog;            // 72
	float darken;          // 76
	int   fDynamic;        // 80
	float unSelfIllum;	   // 84
	float sRadius;		   // 88
	int   pad;             // 92
	vec4  posCenter;	   // 96..111
	float uAlphaCelBorder; // 112
} op;

// ---- LIGHT & lighting buffers (moved here from vertex shader) ----

struct LIGHT 
{
    int   lightk;
    int   pad1;
    int   pad2;
    int   pad3;
    vec4  pos;
    vec4  dir;
    vec4  color;
    float constant;
    float invDistance;
    float pad4;
    float dst;
    vec4  ru;
    vec4  du;
    mat4  matFrustrum;
    vec4  falloffScale;
    vec4  falloffBias;
};

layout(std430, binding = 2) buffer LIGHTBLK 
{
    LIGHT lights[];
};

layout(std430, binding = 3) buffer ACTIVELIGHTS 
{
    int numLights;
    int lightIndices[];
};

struct MATERIAL
{
    float ambient;
    vec3  midtone;
    vec3  light;
};

uniform int fAlphaTest;

in vec4 vColor;
in vec2 vTexcoord;
in vec3 vWorldPos;
in vec3 vNormalWorld;
in float vFogIntensity;

out vec4 FragColor;

// ---- globals for the per-fragment lighting pass ----
float objectShadow;
float objectMidtone;
vec3  lightAccum;
vec3  normalWS;

// ---- function declarations ----
void DrawOneWay();
void DrawThreeWay();
void ApplyFog();

void StartThreeWay(out MATERIAL matOut);
void InitGlobLighting();
vec4 AddDirectionLight(LIGHT dirlight);
vec4 AddDynamicLight(vec4 dir, vec4 color, vec4 ru, vec4 du);
bool SphereIntersectsPositionLight(LIGHT L, vec3 center, float radius);
vec4 AddPositionLight(LIGHT pointlight, vec3 toLight, float dist);
vec4 AddPositionLightDynamic(LIGHT pointlight, vec3 direction);
bool SphereIntersectsFrustum(LIGHT L, vec3 center, float radius);
vec4 AddFrustrumLight(LIGHT frustumlight);
vec4 AddFrustrumLightDynamic(LIGHT frustumlight);
void ProcessGlobLighting(out MATERIAL matOut);

void main()
{
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

// =====================================================
//         RKO_OneWay: no lighting, just texture
// =====================================================
void DrawOneWay()
{
    vec4 diffuse = texture(diffuseMap, vTexcoord);

    FragColor = vColor * diffuse;

    float alphaIn = clamp(vColor.a * diffuse.a, 0.0, 1.0);

    if (fAlphaTest == 1 && alphaIn < 0.9)
        discard;

    FragColor.a = clamp(FragColor.a * op.uAlpha, 0.0, 1.0);
}

// =====================================================
//         RKO_ThreeWay: per-pixel lighting here
// =====================================================
void DrawThreeWay()
{
    vec4 shadow   = texture(shadowMap,   vTexcoord);
    vec4 diffuse  = texture(diffuseMap,  vTexcoord);
    vec4 saturate = texture(saturateMap, vTexcoord);

    // Alpha test first
    float alphaIn = clamp(vColor.a * diffuse.a, 0.0, 1.0);
    if (fAlphaTest == 1 && alphaIn < 0.9)
        discard;

    // Per-fragment lighting
    MATERIAL mat;
    StartThreeWay(mat);

    FragColor.rgb += shadow.rgb   * mat.ambient     * op.darken;
    FragColor.rgb += diffuse.rgb  * mat.midtone     * op.darken;
    FragColor.rgb += saturate.rgb * mat.light;

    float finalAlpha = clamp(vColor.a * diffuse.a, 0.0, 1.0);
    FragColor.a = clamp(finalAlpha * op.uAlpha, 0.0, 1.0);
}

void ApplyFog()
{
    FragColor.rgb = mix(FragColor.rgb, swp.fogColor.rgb, vFogIntensity);
}

// =====================================================
//                 Lighting ported to fragment
// =====================================================

void InitGlobLighting()
{
    objectShadow  = swp.uShadow;
    objectMidtone = swp.uMidtone + op.unSelfIllum * 0.000031;
    lightAccum    = vec3(0.0);

    normalWS = normalize(vNormalWorld);
}

void StartThreeWay(out MATERIAL matOut)
{
    InitGlobLighting();

    if (op.fDynamic != 1)
    {
        // STATIC lighting – computed per fragment now
        for (int i = 0; i < numLights; ++i)
        {
            int idx = lightIndices[i];
            LIGHT L = lights[idx];

            switch (L.lightk)
            {
                case LIGHTK_Direction:
                    lightAccum.rgb += AddDirectionLight(L).rgb;
                    break;

                case LIGHTK_Position:
                {
                    if (!SphereIntersectsPositionLight(L, op.posCenter.xyz, op.sRadius))
                        continue;

                    vec3 toLight = L.pos.xyz - vWorldPos; // per-fragment position
                    lightAccum.rgb += AddPositionLight(L, toLight, dot(toLight,toLight)).rgb;
                    break;
                }

                case LIGHTK_Frustrum:
                case LIGHTK_Spot:
                    if (!SphereIntersectsFrustum(L, op.posCenter.xyz, op.sRadius))
                        continue;

                    lightAccum.rgb += AddFrustrumLight(L).rgb;
                    break;
            }
        }
    }
    else
    {
        // DYNAMIC lighting – still uses your quick paths
        for (int i = 0; i < numLights; ++i)
        {
            int idx = lightIndices[i];
            LIGHT L = lights[idx];

            switch (L.lightk)
            {
                case LIGHTK_Direction:
                    lightAccum.rgb += AddDynamicLight(L.dir, L.color, L.ru, L.du).rgb;
                    break;

                case LIGHTK_Position:
                {
                    if (!SphereIntersectsPositionLight(L, op.posCenter.xyz, op.sRadius))
                        continue;

                    vec3 toLight = L.pos.xyz - op.posCenter.xyz;
                    lightAccum.rgb += AddPositionLightDynamic(L, toLight).rgb;
                    break;
                }

                case LIGHTK_Frustrum:
                case LIGHTK_Spot:
                    if (!SphereIntersectsFrustum(L, op.posCenter.xyz, op.sRadius))
                        continue;

                    lightAccum.rgb += AddFrustrumLightDynamic(L).rgb;
                    break;
            }
        }
    }

    ProcessGlobLighting(matOut);
}

vec4 AddDirectionLight(LIGHT dirlight)
{
    float NdotL = max(dot(normalWS, dirlight.dir.xyz), 0.0);

    float diffuse = NdotL * (1.0 + NdotL * NdotL);

    float shadow    = diffuse * dirlight.ru.x + dirlight.du.x;
    float midtone   = diffuse * dirlight.ru.y + dirlight.du.y;
    float highlight = diffuse * dirlight.ru.z + dirlight.du.z;

    shadow    = max(shadow,    0.0);
    midtone   = max(midtone,   0.0);
    highlight = max(highlight, 0.0);

    objectShadow  += shadow;
    objectMidtone += midtone;

    return vec4(dirlight.color * highlight);
}

// dynamic direction light, adapted to per-fragment world space
vec4 AddDynamicLight(vec4 dir, vec4 color, vec4 ru, vec4 du)
{
    vec3 Ldir = normalize(dir.xyz);

    float NdotL = dot(normalWS, Ldir);
    float diffuse = NdotL + NdotL * NdotL * NdotL;

    float shadow    = diffuse * ru.x + du.x;
    float midtone   = diffuse * ru.y + du.y;
    float highlight = diffuse * ru.z + du.z;

    objectShadow  += max(shadow,    0.0);
    objectMidtone += max(midtone,   0.0);
    highlight      = max(highlight, 0.0);

    return color * highlight;
}

bool SphereIntersectsPositionLight(LIGHT L, vec3 center, float radius)
{
    vec3  toLight = L.pos.xyz - center;
    float distSq  = dot(toLight, toLight);

    float maxDist = L.dst + radius;
    return distSq <= maxDist * maxDist;
}

vec4 AddPositionLight(LIGHT pointlight, vec3 toLight, float dist)
{
    float d2 = dot(toLight, toLight);
    float invLen = (d2 > 1e-12) ? inversesqrt(d2) : 0.0;
    vec3  L = toLight * invLen;

    vec3 N = normalWS;
    float n2 = dot(N, N);
    if (n2 > 1e-12) N *= inversesqrt(n2); else N = vec3(0.0);

    float ndotl = dot(N, L);
    float diffuse = ndotl + ndotl * ndotl * ndotl;

    float attenuation = clamp(pointlight.constant + pointlight.invDistance * invLen, 0.0, 1.0);

    float shadow    = max(diffuse * pointlight.ru.x + pointlight.du.x, 0.0) * attenuation;
    float midtone   = max(diffuse * pointlight.ru.y + pointlight.du.y, 0.0) * attenuation;
    float highlight = max(diffuse * pointlight.ru.z + pointlight.du.z, 0.0) * attenuation;

    objectShadow  += shadow;
    objectMidtone += midtone;

    return vec4(pointlight.color * highlight);
}

vec4 AddPositionLightDynamic(LIGHT pointlight, vec3 direction)
{
    float distance = length(direction);

    float attenuation = 1.0 / distance * pointlight.invDistance + pointlight.constant;
    attenuation = clamp(attenuation, 0.0, 1.0);

    vec4 color = AddDynamicLight(vec4(direction, 0.0),
                                 pointlight.color * attenuation,
                                 pointlight.ru * attenuation,
                                 pointlight.du * attenuation) * attenuation;

    return color;
}

bool SphereIntersectsFrustum(LIGHT L, vec3 center, float radius)
{
    mat4 M = L.matFrustrum;

    vec4 planes[6];
    planes[0] = M[3] + M[0]; // left
    planes[1] = M[3] - M[0]; // right
    planes[2] = M[3] + M[1]; // bottom
    planes[3] = M[3] - M[1]; // top
    planes[4] = M[3] + M[2]; // near
    planes[5] = M[3] - M[2]; // far

    for (int i = 0; i < 6; ++i)
    {
        vec3 n = planes[i].xyz;
        float len = length(n);

        if (len < 1e-6) continue;

        n /= len;
        float d = planes[i].w / len;

        float dist = dot(n, center) + d;

        if (dist < -radius)
            return false;
    }

    return true;
}

vec4 AddFrustrumLight(LIGHT frustumlight)
{
    vec4 clipL = frustumlight.matFrustrum * vec4(vWorldPos, 1.0);

    if (clipL.w <= 0.0) 
        return vec4(0.0);
    if (abs(clipL.x) > clipL.w) 
        return vec4(0.0);
    if (abs(clipL.y) > clipL.w) 
        return vec4(0.0);
    if (clipL.z < 0.0 || clipL.z > clipL.w) 
        return vec4(0.0);

    float invW = 1.0 / clipL.w;

    float u  = abs(clipL.x * invW);
    float v  = abs(clipL.y * invW);
    float r2 = u * u + v * v;
    float wv = clipL.w;

    float fx = clamp(frustumlight.falloffScale.x * u  + frustumlight.falloffBias.x, 0.0, 1.0);
    float fy = clamp(frustumlight.falloffScale.y * v  + frustumlight.falloffBias.y, 0.0, 1.0);
    float fr = clamp(frustumlight.falloffScale.z * r2 + frustumlight.falloffBias.z, 0.0, 1.0);
    float fw = clamp(frustumlight.falloffScale.w * wv + frustumlight.falloffBias.w, 0.0, 1.0);

    float mask = fx * fy * fr * fw;

    if (mask <= 0.0) 
        return vec4(0.0);

    vec3 Ldir = frustumlight.dir.xyz;
    float len2 = dot(Ldir, Ldir);
    Ldir = (len2 > 1e-8) ? Ldir * inversesqrt(len2) : vec3(0.0);

    float NL = dot(normalWS, Ldir);
    NL = NL + NL*NL*NL;

    float shadow    = max(0.0, NL * frustumlight.ru.x + frustumlight.du.x);
    float midtone   = max(0.0, NL * frustumlight.ru.y + frustumlight.du.y);
    float highlight = max(0.0, NL * frustumlight.ru.z + frustumlight.du.z);

    objectShadow  += shadow  * mask;
    objectMidtone += midtone * mask;

    return frustumlight.color * (highlight * mask);
}

vec4 AddFrustrumLightDynamic(LIGHT frustumlight)
{
    vec4 clipL = frustumlight.matFrustrum * vec4(op.posCenter.xyz, 1.0);

    if (clipL.w <= 0.0) 
        return vec4(0.0);
    if (abs(clipL.x) > clipL.w) 
        return vec4(0.0);
    if (abs(clipL.y) > clipL.w) 
        return vec4(0.0);
    if (clipL.z < 0.0 || clipL.z > clipL.w) 
        return vec4(0.0);

    float invW = 1.0 / clipL.w;

    float u  = abs(clipL.x * invW);
    float v  = abs(clipL.y * invW);
    float r2 = u * u + v * v;
    float wv = clipL.w;

    float fx = clamp(frustumlight.falloffScale.x * u  + frustumlight.falloffBias.x, 0.0, 1.0);
    float fy = clamp(frustumlight.falloffScale.y * v  + frustumlight.falloffBias.y, 0.0, 1.0);
    float fr = clamp(frustumlight.falloffScale.z * r2 + frustumlight.falloffBias.z, 0.0, 1.0);
    float fw = clamp(frustumlight.falloffScale.w * wv + frustumlight.falloffBias.w, 0.0, 1.0);

    float mask = fx * fy * fr * fw;
    if (mask <= 0.0) 
        return vec4(0.0);

    vec4 ruScaled = frustumlight.ru * mask;
    vec4 duScaled = frustumlight.du * mask;

    return AddDynamicLight(frustumlight.dir, frustumlight.color, ruScaled, duScaled);
}

void ProcessGlobLighting(out MATERIAL matOut)
{
    float dominantLight = max(max(lightAccum.r, lightAccum.g), lightAccum.b);
    float shadowModifier = 1.0 - dominantLight;
    float clampedMidtone = clamp(objectMidtone, 0.0, shadowModifier);
    float shadowContribution = max(min(shadowModifier - clampedMidtone, objectShadow), 0.0);
    float baseIntensity = dot(vColor.rgb, vec3(0.3333333));

    matOut.ambient = shadowContribution * baseIntensity;
    matOut.midtone = clampedMidtone * vColor.rgb;
    matOut.light   = min(lightAccum, vec3(1.0)) * baseIntensity;
}
