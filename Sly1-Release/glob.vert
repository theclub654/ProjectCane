#version 430 core

#define RKO_ThreeWay  0
#define RKO_OneWay    1
#define RKO_CelBorder 2
#define RKO_Collision 3

#define LIGHTK_Direction 0
#define LIGHTK_Position  1
#define LIGHTK_Frustrum  2
#define LIGHTK_Spot      3

#define MAX_LIGHTS 255

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
    vec3  falloff;
    float dst;
    vec4  ru;
    vec4  du;
    mat4  matFrustrum;
    vec4  falloffScale;
    vec4  falloffBias;
};

layout(std140) uniform ACTIVELIGHTS
{
    int numLights;
     // pad header to 16 bytes
    int _pad0; 
    int _pad1; 
    int _pad2;

    // Each element has 16B stride in std140
    int lightIndices[MAX_LIGHTS];
}; 

layout(std140) uniform LIGHTBLK
{
    LIGHT lights[MAX_LIGHTS];
};

layout(std140) uniform CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

layout(std140) uniform RO // Render object properties
{
    mat4  model;
	int   rko;
    float uAlpha;
	float uFog;
	float darken;
	int   fDynamic;
	float unSelfIllum;
    float sRadius;
	vec4  posCenter;
}op;

uniform samplerBuffer uEdges;

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
vec4 AddPositionLight(LIGHT pointlight, vec3 toLight, float dist);
vec4 AddPositionLightDynamic(LIGHT pointlight, vec3 direction);
vec4 AddFrustrumLight(LIGHT frustumlight);
//vec4 AddFrustrumLightDynamic(FRUSTUMLIGHT frustumlight);
void ProcessGlobLighting();
void ProcessCelBorders();
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
        CalculateFog();
        break;

        case RKO_ThreeWay:
        worldPos    = op.model * vec4(vertex, 1.0);
        vertexColor = color;
        texcoord    = uv;
        StartThreeWay();
        CalculateFog();
        break;

        case RKO_CelBorder:
        ProcessCelBorders();
        return;

        case RKO_Collision:
        worldPos    = op.model * vec4(vertex, 1.0);
        break;
    }
    
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
                    // Per-object cull (bound center vs light sphere)
                    vec3  dBound    = L.pos.xyz - op.posCenter.xyz;  // bound center (world)
                    float distSqB   = dot(dBound, dBound);
                    float maxDist   = L.dst + op.sRadius;            // linear units
                    float maxDistSq = maxDist * maxDist;

                    if (distSqB > maxDistSq)
                        continue;
 
                    // Per vertex lighting from the actual shaded point
                    vec3  toLight = L.pos.xyz - worldPos.xyz;      // shaded point (world)
                    float distSq  = dot(toLight, toLight);

                    light.rgb += AddPositionLight(L, toLight, distSq).rgb;
                    break;
                }

                case LIGHTK_Frustrum:
                case LIGHTK_Spot:
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
                    vec3  toLight = L.pos.xyz - op.posCenter.xyz; // center of the object’s bound
                    float distSq  = dot(toLight, toLight);
                    float maxDist = L.dst + op.sRadius;   // L.radius is linear (not squared)

                    if (distSq > maxDist * maxDist)
                        continue;

                    light.rgb += AddPositionLightDynamic(L, toLight).rgb;
                    break;
                }
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
    float attenuation = clamp(pointlight.falloff.x + pointlight.falloff.y * invLen, 0.0, 1.0);

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
    float attenuation = 1.0 / distance * pointlight.falloff.y + pointlight.falloff.x;
    attenuation = clamp(attenuation, 0.0, 1.0);

    // Apply dynamic lighting
    vec4 color = AddDynamicLight(vec4(direction, 0.0), pointlight.color * attenuation, pointlight.ru * attenuation, pointlight.du * attenuation) * attenuation;

    return color;
}

vec4 AddFrustrumLight(LIGHT frustumlight)
{
    vec4 clipL = frustumlight.matFrustrum * vec4(worldPos.xyz, 1.0);

    // Homogeneous clip (RH, Z in [0,1]) to mirror EmulateClip’s reject
    if (clipL.w <= 0.0) return vec4(0.0);
    if (abs(clipL.x) > clipL.w) return vec4(0.0);
    if (abs(clipL.y) > clipL.w) return vec4(0.0);
    if (clipL.z < 0.0 || clipL.z > clipL.w) return vec4(0.0);

    float invW = 1.0 / clipL.w;

    float u  = abs(clipL.x * invW);
    float v  = abs(clipL.y * invW);
    float r2 = u*u + v*v;
    float wv = clipL.w;

    float fx = clamp(frustumlight.falloffScale.x * u  + frustumlight.falloffBias.x, 0.0, 1.0);
    float fy = clamp(frustumlight.falloffScale.y * v  + frustumlight.falloffBias.y, 0.0, 1.0);
    float fr = clamp(frustumlight.falloffScale.z * r2 + frustumlight.falloffBias.z, 0.0, 1.0);
    float fw = clamp(frustumlight.falloffScale.w * wv + frustumlight.falloffBias.w, 0.0, 1.0);

    float mask = fx * fy * fr * fw;
    if (mask <= 0.0) return vec4(0.0);

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

//
//vec4 AddFrustrumLightDynamic(FRUSTUMLIGHT frustumlight)
//{
//    
//    return vec4(0.0);
//}

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

void ProcessCelBorders()
{
    // Which edge are we on? (2 vertices per edge)
    int  edgeID = gl_VertexID >> 1;
    bool first  = (gl_VertexID & 1) == 0;

    // Fetch edge block (object space)
    vec3 E0 = texelFetch(uEdges, edgeID * 4 + 0).xyz;
    vec3 E1 = texelFetch(uEdges, edgeID * 4 + 1).xyz;
    vec3 OA = texelFetch(uEdges, edgeID * 4 + 2).xyz;
    vec3 OB = texelFetch(uEdges, edgeID * 4 + 3).xyz;

    // World → Clip
    vec4 A = cm.matWorldToClip * (op.model * vec4(E0, 1.0));
    vec4 B = cm.matWorldToClip * (op.model * vec4(E1, 1.0));
    vec4 C = cm.matWorldToClip * (op.model * vec4(OA, 1.0));
    vec4 D = cm.matWorldToClip * (op.model * vec4(OB, 1.0));

    // Keep the original validity rule
    bool valid = (A.w > 0.0 && B.w > 0.0 && C.w > 0.0 && D.w > 0.0);

    // --- Homogeneous (pre-divide) orientation tests ---
    float sAC = A.x*(B.y*C.w - B.w*C.y)
              - A.y*(B.x*C.w - B.w*C.x)
              + A.w*(B.x*C.y - B.y*C.x);

    float sBD = A.x*(B.y*D.w - B.w*D.y)
              - A.y*(B.x*D.w - B.w*D.x)
              + A.w*(B.x*D.y - B.y*D.x);

    // Border or opposite-facing neighbor means "draw"
    const float kEPS = 1e-6;
    bool isBorder = (abs(sAC) <= kEPS) || (abs(sBD) <= kEPS);
    bool opposite = (sAC * sBD < 0.0);
    
    int fNonCelBorder = int(valid && (isBorder || opposite));

    if (fNonCelBorder == 1)
    {
        // Pushing celborder to clip space so hardware clips it and fragment shader doesnt run
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    // Emit one endpoint of the line for this VS invocation
    vec4 P = first ? A : B;

    // Depth for the line (no reversed-Z, no uniform bias)
    float dA = (A.z / A.w) * 0.5 + 0.5;
    float dB = (B.z / B.w) * 0.5 + 0.5;
    float dC = (C.z / C.w) * 0.5 + 0.5;
    float dD = (D.z / D.w) * 0.5 + 0.5;

    // Place the line at the *farthest* edge endpoint,
    // but never closer than the nearer opposite vertex
    float zEdgeMax = max(dA, dB);
    float zOppNear = min(dC, dD);
    float zLine01  = max(zEdgeMax, zOppNear);

    // Tiny fixed forward bias so it wins against its own surface,
    // but won't jump in front of unrelated, nearer geometry.
    const float kBias01 = 1e-5;   // tune: try 1e-5..2e-4
    float zBiased = clamp(zLine01 - kBias01, 0.0, 1.0);

    // back to clip-space
    float zNDC = zBiased * 2.0 - 1.0;
    P.z        = zNDC * P.w;

    gl_Position = P;
}

void CalculateFog()
{
    if (swp.fogType == 0)
        return;

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