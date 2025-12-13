#version 430 core

#define RKO_ThreeWay 0
#define RKO_OneWay   1

#define LIGHTK_Direction 0
#define LIGHTK_Position  1
#define LIGHTK_Frustrum  2
#define LIGHTK_Spot      3

#define FOG_PS2 1
#define FOG_PS3 2

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 uv;

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

layout(std430, binding = 0) readonly buffer CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

layout(std140, binding = 1) uniform RO // Render object properties
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
}op;

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

vec4 worldPos;
vec3 normalWorld;

float objectShadow;
float objectMidtone;
vec3  light;

out vec4 vertexColor;
out vec2 texcoord;
out MATERIAL material;
out float fogIntensity;

void StartThreeWay();
void InitGlobLighting();
vec4 AddDirectionLight(LIGHT dirlight);
vec4 AddDynamicLight(vec4 dir, vec4 color, vec4 ru, vec4 du);
bool SphereIntersectsPositionLight(LIGHT L, vec3 center, float radius);
vec4 AddPositionLight(LIGHT pointlight, vec3 toLight, float dist);
vec4 AddPositionLightDynamic(LIGHT pointlight, vec3 direction);
bool SphereIntersectsFrustum(LIGHT L, vec3 center, float radius);
vec4 AddFrustrumLight(LIGHT frustumlight);
vec4 AddFrustrumLightDynamic(LIGHT frustumlight);
void ProcessGlobLighting();
void CalculateFog();
// This uses the PS2 Style fog
void CalculateFogPS2();
// This uses the PS3 style fog
void CalculateFogPS3();

void main()
{
    switch (op.rko)
    {
        case RKO_OneWay:
        worldPos    = op.model * vec4(vertex, 1.0);
        vertexColor = color;
        texcoord    = uv;
        break;

        case RKO_ThreeWay:
        worldPos    = op.model * vec4(vertex, 1.0);
        vertexColor = color;
        texcoord    = uv;
        StartThreeWay();
        break;
    }
    
    if (swp.fogType != 0)
        CalculateFog();

    gl_Position = cm.matWorldToClip * worldPos;
}

void StartThreeWay()
{
    InitGlobLighting();

    if (op.fDynamic != 1)
    {
        for (int i = 0; i < numLights; ++i)
        {
            int idx = lightIndices[i];
            LIGHT L = lights[idx];

            switch (L.lightk)
            {
                case LIGHTK_Direction:
                light.rgb += AddDirectionLight(L).rgb;
                break;

                case LIGHTK_Position:
                {
                    if (!SphereIntersectsPositionLight(L, op.posCenter.xyz, op.sRadius))
                        continue;

                    vec3 toLight = L.pos.xyz - worldPos.xyz; // shaded point
                    light.rgb += AddPositionLight(L, toLight, dot(toLight,toLight)).rgb;
                    break;
                }

                case LIGHTK_Frustrum:
                case LIGHTK_Spot:
                if (!SphereIntersectsFrustum(L, op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddFrustrumLight(L).rgb;
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < numLights; ++i)
        {
            int idx = lightIndices[i];
            LIGHT L = lights[idx];

            switch (L.lightk)
            {
                case LIGHTK_Direction:
                light.rgb += AddDynamicLight(L.dir, L.color, L.ru, L.du).rgb;
                break;

                case LIGHTK_Position:
                {
                    if (!SphereIntersectsPositionLight(L, op.posCenter.xyz, op.sRadius))
                        continue;

                    vec3 toLight = L.pos.xyz - op.posCenter.xyz;
                    light.rgb += AddPositionLightDynamic(L, toLight).rgb;
                    break;
                }

                case LIGHTK_Frustrum:
                case LIGHTK_Spot:
                if (!SphereIntersectsFrustum(L, op.posCenter.xyz, op.sRadius))
                    continue;

                light.rgb += AddFrustrumLightDynamic(L).rgb;
                break;
            }
        }
    }

    ProcessGlobLighting();
}

void InitGlobLighting()
{
    objectShadow  = swp.uShadow;
    objectMidtone = swp.uMidtone + op.unSelfIllum * 0.000031;
    light = vec3(0.0);

    normalWorld = normalize(mat3(op.model) * normal);
}

vec4 AddDirectionLight(LIGHT dirlight)
{
    // Dot product between normal and light direction (skip normalize if normal is already normalized)
    float NdotL = max(dot(normalWorld, dirlight.dir.xyz), 0.0);

    // Stylized boost: N + N^3 approximation
    float diffuse = NdotL * (1.0 + NdotL * NdotL);

    // Ramp contributions without branching
    float shadow    = diffuse * dirlight.ru.x + dirlight.du.x;
    float midtone   = diffuse * dirlight.ru.y + dirlight.du.y;
    float highlight = diffuse * dirlight.ru.z + dirlight.du.z;

    // Clamp results
    shadow    = max(shadow,    0.0);
    midtone   = max(midtone,   0.0);
    highlight = max(highlight, 0.0);

    // Accumulate tone contributions
    objectShadow  += shadow;
    objectMidtone += midtone;

    // Final output color
    return vec4(dirlight.color * highlight);
}

vec4 AddDynamicLight(vec4 dir, vec4 color, vec4 ru, vec4 du)
{
    // Transform light direction into model space
    vec3 lightDir = normalize(mat3(transpose(op.model)) * vec3(dir));
    
    // Compute stylized diffuse term
    float diffuse = dot(lightDir, normal);
    diffuse += diffuse * diffuse * diffuse;

    // Compute lighting components
    float shadow    = diffuse * ru.x + du.x;
    float midtone   = diffuse * ru.y + du.y;
    float highlight = diffuse * ru.z + du.z;

    // Accumulate lighting contributions
    objectShadow  += max(shadow,    0.0);
    objectMidtone += max(midtone,   0.0);
    highlight      = max(highlight, 0.0);

    // Return final color modulated by stylized highlight
    return color * highlight;
}

bool SphereIntersectsPositionLight(LIGHT L, vec3 center, float radius)
{
    // Vector from light to object center
    vec3  toLight = L.pos.xyz - center;

    // Squared distance between centers
    float distSq  = dot(toLight, toLight);

    // Position-light influence radius:
    // L.dst = light falloff (gMax), same as PS2 lmFallOffS.gMax
    float maxDist = L.dst + radius;   // linear units

    // Compare using squared distance
    return distSq <= maxDist * maxDist;
}

vec4 AddPositionLight(LIGHT pointlight, vec3 toLight, float dist)
{
    // Normalize light direction with a zero-length guard (use toLight directly for accuracy)
    float d2 = dot(toLight, toLight);
    float invLen = (d2 > 1e-12) ? inversesqrt(d2) : 0.0; // 1/|d|
    vec3  L = toLight * invLen;

    // Normalize the normal (VU1 guards this too)
    vec3 N = normalWorld;
    float n2 = dot(N, N);
    if (n2 > 1e-12) N *= inversesqrt(n2); else N = vec3(0.0);

    // Raw dot — do NOT clamp before stylized term
    float ndotl = dot(N, L);

    // Stylized boost: n + n^3 (== n * (1 + n^2))
    float diffuse = ndotl + ndotl * ndotl * ndotl;

    // Attenuation: du + ru * (1/|d|), then clamp to [0,1]
    // falloff.x = duAtt, falloff.y = ruAtt  (match your struct)
    float attenuation = clamp(pointlight.constant + pointlight.invDistance * invLen, 0.0, 1.0);

    // Ramps (clamp each ≥ 0, then apply attenuation)
    float shadow    = max(diffuse * pointlight.ru.x + pointlight.du.x, 0.0) * attenuation;
    float midtone   = max(diffuse * pointlight.ru.y + pointlight.du.y, 0.0) * attenuation;
    float highlight = max(diffuse * pointlight.ru.z + pointlight.du.z, 0.0) * attenuation;

    // Accumulate (VU1 adds to running totals)
    objectShadow  += shadow;
    objectMidtone += midtone;

    // Final highlight-scaled light color
    return vec4(pointlight.color * highlight);
}

vec4 AddPositionLightDynamic(LIGHT pointlight, vec3 direction)
{
    float distance = length(direction);

    // Compute clamped attenuation
    float attenuation = 1.0 / distance * pointlight.invDistance + pointlight.constant;
    attenuation = clamp(attenuation, 0.0, 1.0);

    // Apply dynamic lighting
    vec4 color = AddDynamicLight(vec4(direction, 0.0), pointlight.color * attenuation, pointlight.ru * attenuation, pointlight.du * attenuation) * attenuation;

    return color;
}

bool SphereIntersectsFrustum(LIGHT L, vec3 center, float radius)
{
    // Extract frustum planes from clip matrix (RH, Z in [0,1])
    // M maps world -> clip, planes in world space:
    mat4 M = L.matFrustrum;

    // Standard extraction (row-major GLSL: M[column]):
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

        if (len < 1e-6) continue; // degenerate (shouldn’t happen but be safe)

        // Normalize plane so |n|==1
        n /= len;
        float d = planes[i].w / len;

        // Signed distance from sphere center to plane
        float dist = dot(n, center) + d;

        // If sphere is fully outside this plane, no intersection
        if (dist < -radius)
            return false;
    }

    return true; // sphere intersects or is inside all planes
}

vec4 AddFrustrumLight(LIGHT frustumlight)
{
    vec4 clipL = frustumlight.matFrustrum * vec4(worldPos.xyz, 1.0);

    // Homogeneous clip (RH, Z in [0,1]) to mirror EmulateClip’s reject
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

    float NL = dot(normalWorld, Ldir);
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
    // 1) Transform sample point into light clip space
    vec4 clipL = frustumlight.matFrustrum * vec4(op.posCenter.xyz, 1.0);

    // EmulateClip-style frustum test (RH, Z in [0,1])
    if (clipL.w <= 0.0) 
        return vec4(0.0);
    if (abs(clipL.x) > clipL.w) 
        return vec4(0.0);
    if (abs(clipL.y) > clipL.w) 
        return vec4(0.0);
    if (clipL.z < 0.0 || clipL.z > clipL.w) 
        return vec4(0.0);

    // 2) Build falloff coordinates at this single sample
    float invW = 1.0 / clipL.w;

    float u  = abs(clipL.x * invW);   // |x/w|
    float v  = abs(clipL.y * invW);   // |y/w|
    float r2 = u * u + v * v;         // radial^2 in projected space
    float wv = clipL.w;               // S/distance param

    float fx = clamp(frustumlight.falloffScale.x * u  + frustumlight.falloffBias.x, 0.0, 1.0);
    float fy = clamp(frustumlight.falloffScale.y * v  + frustumlight.falloffBias.y, 0.0, 1.0);
    float fr = clamp(frustumlight.falloffScale.z * r2 + frustumlight.falloffBias.z, 0.0, 1.0);
    float fw = clamp(frustumlight.falloffScale.w * wv + frustumlight.falloffBias.w, 0.0, 1.0);

    float mask = fx * fy * fr * fw;

    // If sample is outside effective falloff, light contributes nothing
    if (mask <= 0.0) 
        return vec4(0.0);

    // 3) Scale angle ramps by mask (this is what VU1 does with ru/du)
    vec4 ruScaled = frustumlight.ru * mask;
    vec4 duScaled = frustumlight.du * mask;

    // 4) Call the quick/dynamic light path
    return AddDynamicLight(frustumlight.dir, frustumlight.color, ruScaled, duScaled);
}

void ProcessGlobLighting()
{
    // Find the brightest channel in the light color (used to gauge overall intensity)
    float dominantLight = max(max(light.r, light.g), light.b);
    // Invert brightness to get potential shadow strength (brighter light = less shadow)
    float shadowModifier = 1.0 - dominantLight;
    // Clamp the object's midtone value so it doesn't exceed remaining shadow range
    float clampedMidtone = clamp(objectMidtone, 0.0, shadowModifier);
    // Determine how much of the remaining light can go toward shadows (bounded by objectShadow)
    float shadowContribution = max(min(shadowModifier - clampedMidtone, objectShadow), 0.0);
    // Approximate base light intensity from average of vertex color (grayscale luminance)
    float baseIntensity = dot(vertexColor.rgb, vec3(0.3333333));

    // Assign lighting output
    material.ambient = shadowContribution * baseIntensity;
    material.midtone.rgb = clampedMidtone * vertexColor.rgb;
    material.light   = min(light.rgb, vec3(1.0)) * baseIntensity;
}

void CalculateFog()
{
    switch(swp.fogType)
    {
        case FOG_PS2:
        CalculateFogPS2();
        break;

        case FOG_PS3:
        CalculateFogPS3();
        break;
    }
}

void CalculateFogPS2()
{
    // Distance to camera
    float z = length(cm.cameraPos.xyz - worldPos.xyz);
    float recipZ = 1.0 / max(z, 1e-4);

    float recipNear = 1.0 / swp.fogNear;
    float recipFar  = 1.0 / swp.fogFar;

    float denom = max(recipNear - recipFar, 1e-6); // avoid divide by 0
    float fog = clamp((recipNear - recipZ) * (1.0 / denom), 0.0, 1.0);

    float fogMult = mix(swp.fogMax, swp.fogMax * op.uFog, step(0.001, op.uFog));

    fogIntensity = fog * fogMult;
}

void CalculateFogPS3()
{
    // Compute squared distance for performance
    vec3  offset = cm.cameraPos.xyz - worldPos.xyz;
    float distance2 = dot(offset, offset);
    float distanceToCamera = sqrt(distance2);

    // Precompute inverse range for fog mapping
    float invFogRange = 1.0 / max(swp.fogFar - swp.fogNear, 1e-6);

    // Linear fog factor in 0..1 range
    float fog = clamp((distanceToCamera - swp.fogNear) * invFogRange, 0.0, 1.0);

    float fogMult = mix(swp.fogMax, swp.fogMax * op.uFog, step(0.001, op.uFog));

    fogIntensity = fog * fogMult;
}