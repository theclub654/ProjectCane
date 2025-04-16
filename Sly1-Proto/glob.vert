#version 330 core

#define RKO_OneWay 0
#define RKO_ThreeWay 1
#define RKO_CelBorder 2

#define FOG_PS2 1
#define FOG_PS3 2

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 uv;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform vec3 cameraPos;

out vec4 vertexColor;
out vec2 texcoord;

struct LSM
{
    float uShadow;
    float uMidtone;
}; uniform LSM lsm;

struct LTFN
{
    float ruShadow;
    float ruMidtone;
    float ruHighlight;

    float duShadow;
    float duMidtone;
    float duHighlight;
};

struct DIRLIGHT
{
    vec3 dir;
    vec3 color;

    LTFN ltfn;

}; uniform DIRLIGHT dirlights[5];

uniform int numDirLights;

struct POINTLIGHT
{
    vec3 pos;
    vec3 color;
    vec3 falloff;

    LTFN ltfn;

}; uniform POINTLIGHT pointlights[100];

uniform int numPointLights;

struct FRUSTUMLIGHT
{
   vec3 dir;
   vec3 color;
   vec3 falloff;

   LTFN ltfn; 
}; uniform FRUSTUMLIGHT frustumlights[5];

uniform int numFrustumLights;

struct MATERIAL
{
    float ambient;
    vec3  midtone;
    vec3  light;
};

uniform float fogNear;
uniform float fogFar;
uniform float fogMax;
uniform int fogType;

uniform float uFog;

uniform int rko;
uniform float usSelfIllum;
uniform int fDynamic;
uniform vec3 posCenter;

float objectShadow;
float objectMidtone;
vec4  light;

out MATERIAL material;
out float fogIntensity;

vec4 worldPos;
void StartThreeWay();
void InitGlobLighting();
vec4 AddDirectionLight(DIRLIGHT dirlight);
vec4 AddDynamicLight(vec3 dir, vec3 color, LTFN ltfn);
vec4 AddPositionLight(POINTLIGHT pointlight);
vec4 AddPositionLightDynamic(POINTLIGHT pointlight);
vec4 AddFrustrumLight(FRUSTUMLIGHT frustumlight);
vec4 AddFrustrumLightDynamic(FRUSTUMLIGHT frustumlight);
void ProcessGlobLighting();
// This uses the PS2 Style fog
void CalculateFogPS2(vec4 worldPos);
// This uses the PS3 style fog
void CalculateFogPS3(vec4 worldPos);

void main()
{
    vertexColor = color;
    texcoord    = uv;

    if (rko == RKO_ThreeWay)
    {
        InitGlobLighting();
        StartThreeWay();
    }

   vec4 worldPos = model * vec4(vertex, 1.0);

    switch (fogType)
    {
        case FOG_PS2:
        CalculateFogPS2(worldPos);
        break;

        case FOG_PS3:
        CalculateFogPS3(worldPos);
        break;
    }

    gl_Position = proj * view * worldPos;
}

void StartThreeWay()
{
    if (fDynamic != 1)
    {
        for (int i = 0; i < numDirLights; i++)
            light += AddDirectionLight(dirlights[i]);
            
        for (int i = 0; i < numPointLights; i++)
            light += AddPositionLight(pointlights[i]);

        for (int i = 0; i < numFrustumLights; i++)
            light += AddFrustrumLight(frustumlights[i]);
    }
    else
    {
        for (int i = 0; i < numDirLights; i++)
            light += AddDynamicLight(dirlights[i].dir, dirlights[i].color, dirlights[i].ltfn);
            
        for (int i = 0; i < numPointLights; i++)
            light += AddPositionLightDynamic(pointlights[i]);

        for (int i = 0; i < numFrustumLights; i++)
            light += AddFrustrumLightDynamic(frustumlights[i]);
     }
     
     ProcessGlobLighting();
}

void InitGlobLighting()
{
    objectShadow  = lsm.uShadow;
    objectMidtone = lsm.uMidtone + usSelfIllum * 3.060163e-05;
    light = vec4(0.0);
}

vec4 AddDirectionLight(DIRLIGHT dirlight)
{
    // Transform light direction into world space
    vec3 lightDirWorld = normalize(mat3(inverse(model)) * dirlight.dir);

    // Dot product between normal and light direction (skip normalize if normal is already normalized)
    float NdotL = max(dot(normal, lightDirWorld), 0.0);

    // Stylized boost: N + N^3 approximation
    float diffuse = NdotL * (1.0 + NdotL * NdotL);

    // Ramp contributions without branching
    float shadow    = diffuse * dirlight.ltfn.ruShadow   + dirlight.ltfn.duShadow;
    float midtone   = diffuse * dirlight.ltfn.ruMidtone  + dirlight.ltfn.duMidtone;
    float highlight = diffuse * dirlight.ltfn.ruHighlight + dirlight.ltfn.duHighlight;

    // Clamp results
    shadow    = max(shadow, 0.0);
    midtone   = max(midtone, 0.0);
    highlight = max(highlight, 0.0);

    // Accumulate tone contributions
    objectShadow  += shadow;
    objectMidtone += midtone;

    // Final output color
    return vec4(dirlight.color * highlight, 0.0);
}

vec4 AddDynamicLight(vec3 dir, vec3 color, LTFN ltfn)
{
    // Transform light direction into model space
    vec3 lightDir = normalize(mat3(inverse(model)) * dir);
    
    // Compute stylized diffuse term
    float diffuse = dot(lightDir, normal);
    diffuse += diffuse * diffuse * diffuse; // Enhance with stylized curve

    // Compute lighting components
    float shadow  = ltfn.duShadow  + diffuse * ltfn.ruShadow;
    float midtone = ltfn.duMidtone + diffuse * ltfn.ruMidtone;
    float highlight = ltfn.duHighlight + diffuse * ltfn.ruHighlight;
    highlight = max(highlight, 0.0);

    // Accumulate lighting contributions
    objectShadow  += max(shadow, 0.0);
    objectMidtone += max(midtone, 0.0);

    // Return final color modulated by stylized highlight
    return vec4(color, 0.0) * highlight;
}

vec4 AddPositionLight(POINTLIGHT pointlight)
{
    // World-space position and normal
    vec3 posWorld    = (model * vec4(vertex, 1.0)).xyz;
    vec3 normalWorld = normalize(mat3(model) * normal);

    // Light vector (non-normalized)
    vec3 toLight   = pointlight.pos - posWorld;
    float distSqr  = dot(toLight, toLight);

    // Approximate inverse sqrt for light direction (normalize)
    float invLen   = inversesqrt(distSqr);
    vec3 lightDir  = toLight * invLen;

    // Diffuse
    float NdotL = max(dot(lightDir, normalWorld), 0.0);
    float diffuse = NdotL * (1.0 + NdotL * NdotL); // N + N³ ≈ stylized bump

    // Custom attenuation
    float attenuation = clamp(pointlight.falloff.x + pointlight.falloff.y * invLen, 0.0, 1.0);

    // Ramp contributions
    float shadow    = diffuse * pointlight.ltfn.ruShadow   + pointlight.ltfn.duShadow;
    float midtone   = diffuse * pointlight.ltfn.ruMidtone  + pointlight.ltfn.duMidtone;
    float highlight = diffuse * pointlight.ltfn.ruHighlight + pointlight.ltfn.duHighlight;

    // Multiply all by attenuation
    shadow    *= attenuation;
    midtone   *= attenuation;
    highlight *= attenuation;

    // Accumulate shared lighting values
    objectShadow  += max(shadow, 0.0);
    objectMidtone += max(midtone, 0.0);

    // Return final color contribution
    return vec4(pointlight.color * max(highlight, 0.0), 0.0);
}

vec4 AddPositionLightDynamic(POINTLIGHT pointlight)
{
    // Transform light center to world space
    vec3 posCenterWorld = vec3(model * vec4(posCenter, 1.0));

    // Compute direction and distance to light
    vec3 direction = normalize(pointlight.pos - posCenterWorld);
    float distance = length(pointlight.pos - posCenterWorld);

    // Compute clamped attenuation
    float attenuation = 1.0 / distance * pointlight.falloff.y + pointlight.falloff.x;
    attenuation = clamp(attenuation, 0.0, 1.0);

    // Scale light tone functions by attenuation
    LTFN ltfn;

    ltfn.ruShadow = pointlight.ltfn.ruShadow * attenuation;
    ltfn.duShadow = pointlight.ltfn.duShadow * attenuation;

    ltfn.ruMidtone = pointlight.ltfn.ruMidtone * attenuation;
    ltfn.duMidtone = pointlight.ltfn.duMidtone * attenuation;

    ltfn.ruHighlight = pointlight.ltfn.ruHighlight * attenuation;
    ltfn.duHighlight = pointlight.ltfn.duHighlight * attenuation;

    // Apply dynamic lighting
    return AddDynamicLight(direction, pointlight.color, ltfn) * attenuation;
}

vec4 AddFrustrumLight(FRUSTUMLIGHT frustumlight)
{
    
    return vec4(0.0);
}

vec4 AddFrustrumLightDynamic(FRUSTUMLIGHT frustumlight)
{
    
    return vec4(0.0);
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

void CalculateFogPS2(vec4 worldPos)
{
    // Distance to camera
    float z = length(worldPos.xyz - cameraPos);

    float recipZ = 1.0 / max(z, 0.0001); // Prevent div by zero

    // Flip the reciprocal mapping
    float recipNear = 1.0 / fogNear;
    float recipFar  = 1.0 / fogFar;

    // remap recipZ from recipNear..recipFar to 0..1
    float fog = clamp((recipNear - recipZ) / (recipNear - recipFar), 0.0, 1.0);

    if (uFog > 0.0)
        fogIntensity = clamp(fog * fogMax * uFog, 0.0, 1.0);
    else
        fogIntensity = clamp(fog * fogMax, 0.0, 1.0);
}

void CalculateFogPS3(vec4 worldPos)
{
    // Compute the distance from the camera to the world-space position of the vertex
    float distanceToCamera = length(worldPos.xyz - cameraPos);

    // Linearly map the distance into a 0..1 fog range
    // fog is 0 when at fogNear and 1 when at fogFar
    float fog = clamp((distanceToCamera - fogNear) / (fogFar - fogNear), 0.0, 1.0);

    // Scale the fog intensity by fogMax (a scalar that controls overall fog strength)
    // and clamp it to stay within the [0..1] valid fog range
    if (uFog > 0.0)
        fogIntensity = clamp(fog * fogMax * uFog, 0.0, 1.0);
    else
        fogIntensity = clamp(fog * fogMax, 0.0, 1.0);
}