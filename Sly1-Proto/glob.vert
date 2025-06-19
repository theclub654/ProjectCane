﻿#version 330 core

#define RKO_OneWay 0
#define RKO_ThreeWay 1
#define RKO_CelBorder 2

#define LIGHTK_Direction 0
#define LIGHTK_Position 1
#define LIGHTK_Frustrum 2
#define LIGHTK_Spot 3

#define MAX_LIGHTS 255

#define FOG_PS2 1
#define FOG_PS3 2

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 uv;

struct LIGHT 
{
    int  lightk;
    int  pad1;
    int  pad2;
    int  pad3;
    vec4 pos;
    vec4 dir;
    vec4 color;
    vec4 falloff;
    vec4 ru;
    vec4 du;
};

layout(std140) uniform LIGHTBLK
{
    LIGHT lights[MAX_LIGHTS];
};

uniform int numLights;

struct MATERIAL
{
    float ambient;
    vec3  midtone;
    vec3  light;
};

// Light Shading Material
struct LSM
{
    float uShadow;
    float uMidtone;
}; uniform LSM lsm;

uniform mat4 matWorldToClip;
uniform vec3 cameraPos;
uniform mat4 model;

uniform int   fogType;
uniform float fogNear;
uniform float fogFar;
uniform float fogMax;
uniform float uFog;

uniform int rko;

uniform float usSelfIllum;
uniform int   fDynamic;
uniform vec3  posCenter;

vec4 worldPos;
vec3 normalWorld;

float objectShadow;
float objectMidtone;
vec4  light;

out vec4 vertexColor;
out vec2 texcoord;

out MATERIAL material;
out float fogIntensity;

void StartThreeWay();
void InitGlobLighting();
vec4 AddDirectionLight(LIGHT dirlight);
vec4 AddDynamicLight(vec4 dir, vec4 color, vec4 ru, vec4 du);
vec4 AddPositionLight(LIGHT pointlight);
vec4 AddPositionLightDynamic(LIGHT pointlight);
//vec4 AddFrustrumLight(FRUSTUMLIGHT frustumlight);
//vec4 AddFrustrumLightDynamic(FRUSTUMLIGHT frustumlight);
void ProcessGlobLighting();
// This uses the PS2 Style fog
void CalculateFogPS2();
// This uses the PS3 style fog
void CalculateFogPS3();

void main()
{
    vertexColor = color;
    texcoord    = uv;

    worldPos = model * vec4(vertex, 1.0);

    if (rko == RKO_ThreeWay)
    {
        InitGlobLighting();
        StartThreeWay();
    }

    switch (fogType)
    {
        case FOG_PS2:
        CalculateFogPS2();
        break;

        case FOG_PS3:
        CalculateFogPS3();
        break;
    }

    gl_Position = matWorldToClip * worldPos;
}

void StartThreeWay()
{
    if (fDynamic != 1)
    {
        for (int i = 0; i < numLights; i++)
        {
            switch (lights[i].lightk)
            {
                case LIGHTK_Direction:
                light += AddDirectionLight(lights[i]);
                break;

                case LIGHTK_Position:
                light += AddPositionLight(lights[i]);
                break;
            }
        }
    }

    else
    {
        for (int i = 0; i < numLights; i++)
        {
            switch (lights[i].lightk)
            {
                case LIGHTK_Direction:
                light += AddDynamicLight(lights[i].dir, lights[i].color, lights[i].ru, lights[i].du);
                break;

                case LIGHTK_Position:
                light += AddPositionLightDynamic(lights[i]);
                break;
            }
        }
     }
     
     ProcessGlobLighting();
}

void InitGlobLighting()
{
    objectShadow  = lsm.uShadow;
    objectMidtone = lsm.uMidtone + usSelfIllum * 0.000031;
    light = vec4(0.0);

    normalWorld = normalize(mat3(model) * normal);
}

vec4 AddDirectionLight(LIGHT dirlight)
{
    // Transform light direction into world space
    vec3 lightDirWorld = normalize(mat3(transpose(model)) * vec3(dirlight.dir));

    // Dot product between normal and light direction (skip normalize if normal is already normalized)
    float NdotL = max(dot(normal, lightDirWorld), 0.0);

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
    vec3 lightDir = normalize(mat3(transpose(model)) * vec3(dir));
    
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

vec4 AddPositionLight(LIGHT pointlight)
{
    // Light vector (non-normalized)
    vec3  toLight  = vec3(pointlight.pos) - worldPos.xyz;
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
    float shadow    = diffuse * pointlight.ru.x + pointlight.du.x;
    float midtone   = diffuse * pointlight.ru.y + pointlight.du.y;
    float highlight = diffuse * pointlight.ru.z + pointlight.du.z;

    // Multiply all by attenuation
    shadow    *= attenuation;
    midtone   *= attenuation;
    highlight *= attenuation;

    // Accumulate shared lighting values
    objectShadow  += max(shadow,  0.0);
    objectMidtone += max(midtone, 0.0);

    // Return final color contribution
    return pointlight.color * max(highlight, 0.0);
}

vec4 AddPositionLightDynamic(LIGHT pointlight)
{
    // Compute direction and distance to light
    vec3  direction = normalize(vec3(pointlight.pos) - posCenter);
    float distance  = length(vec3(pointlight.pos) - posCenter);

    // Compute clamped attenuation
    float attenuation = 1.0 / distance * pointlight.falloff.y + pointlight.falloff.x;
    attenuation = clamp(attenuation, 0.0, 1.0);

    // Apply dynamic lighting
    vec4 color = AddDynamicLight(vec4(direction, 0.0), pointlight.color, pointlight.ru * attenuation, pointlight.du * attenuation) * attenuation;

    return color;
}
//
//vec4 AddFrustrumLight(FRUSTUMLIGHT frustumlight)
//{
//    
//    return vec4(0.0);
//}
//
//vec4 AddFrustrumLightDynamic(FRUSTUMLIGHT frustumlight)
//{
//    
//    return vec4(0.0);
//}

void ProcessGlobLighting()
{
    // Find the brightest RGB channel in the accumulated light color (used to approximate overall brightness)
    float dominantLight = max(max(light.r, light.g), light.b);
    // Invert brightness to get how much shadow range remains (brighter light = less shadow allowed)
    float shadowModifier = 1.0 - dominantLight;
    // Clamp the object's accumulated midtone so it doesn't exceed the remaining light range
    float clampedMidtone = clamp(objectMidtone, 0.0, shadowModifier);
    // Compute how much of the leftover brightness budget can be allocated to shadows
    float shadowContribution = clamp(shadowModifier - clampedMidtone, 0.0, objectShadow);
    // Estimate the base brightness of the vertex color using equal-weighted luminance (grayscale intensity)
    float baseIntensity = dot(vertexColor.rgb, vec3(0.3333333));

    // Set the final ambient term: shadow contribution modulated by base brightness
    material.ambient = shadowContribution * baseIntensity;
    // Set the midtone term: clamped midtone scaled by original vertex color (preserving its hue)
    material.midtone = clampedMidtone * vertexColor.rgb;
    // Set the light (highlight) term: final light color clamped to avoid overflow, scaled by base brightness
    material.light = clamp(light.rgb, 0.0, 1.0) * baseIntensity;
}

void CalculateFogPS2()
{
    // Distance to camera
    float z = length(cameraPos - worldPos.xyz);
    float recipZ = 1.0 / max(z, 1e-4);

    float recipNear = 1.0 / fogNear;
    float recipFar  = 1.0 / fogFar;

    float denom = max(recipNear - recipFar, 1e-6); // avoid divide by 0
    float fog = clamp((recipNear - recipZ) * (1.0 / denom), 0.0, 1.0);

    float fogMult = mix(fogMax, fogMax * uFog, step(0.001, uFog));
    fogIntensity = fog * fogMult;
}

void CalculateFogPS3()
{
    // Compute squared distance for performance
    vec3  offset = cameraPos - worldPos.xyz;
    float distance2 = dot(offset, offset);
    float distanceToCamera = sqrt(distance2);

    // Precompute inverse range for fog mapping
    float invFogRange = 1.0 / max(fogFar - fogNear, 1e-6);

    // Linear fog factor in 0..1 range
    float fog = clamp((distanceToCamera - fogNear) * invFogRange, 0.0, 1.0);

    float fogMult = mix(fogMax, fogMax * uFog, step(0.001, uFog));
    fogIntensity = fog * fogMult;
}