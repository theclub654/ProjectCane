#version 330 core

#define SHDK_ThreeWay 0

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 uv;

uniform mat4 matWorldToClip;
uniform mat4 model;

out vec4 vertexColor;
out vec2 texcoord;

struct LSM
{
    float uShadow;
    float uMidtone;
}; uniform LSM lsm;

struct DIRLIGHT
{
    vec3 pos;
    vec3 dir;
    vec3 color;

    float ruShadow;
    float ruMidtone;
    float ruHighlight;

    float duShadow;
    float duMidtone;
    float duHighlight;

}; uniform DIRLIGHT dirlights[5];

uniform int numDirLights;

struct POINTLIGHT
{
    vec3 pos;
    vec3 color;
    vec3 falloff;

    float ruShadow;
    float ruMidtone;
    float ruHighlight;

    float duShadow;
    float duMidtone;
    float duHighlight;

}; uniform POINTLIGHT pointlights[100];

uniform int numPointLights;

uniform int shdk;
uniform float usSelfIllum;
uniform vec3 posCenter;

out vec4 ambient;
out vec4 midtone;
out vec4 light;

float objectShadow;
float objectMidtone;

void InitGlobLighting();
vec4 AddDirectionLight(DIRLIGHT dirlight);
vec4 AddPositionLight(POINTLIGHT pointlight);
void ProcessGlobLighting();

void main()
{
    vertexColor = color;
    texcoord    = uv;

    if (shdk == SHDK_ThreeWay)
    {
        InitGlobLighting();

        for (int i = 0; i < numDirLights; i++)
            light += AddDirectionLight(dirlights[i]);

        for (int i = 0; i < numPointLights; i++)
            light += AddPositionLight(pointlights[i]);

        ProcessGlobLighting();
    }

    gl_Position = matWorldToClip * model * vec4(vertex, 1.0);
}

void InitGlobLighting()
{
    objectShadow  = lsm.uShadow;
    objectMidtone = lsm.uMidtone + usSelfIllum * 0.000031;
    light = vec4(0.0);
}

vec4 AddDirectionLight(DIRLIGHT dirlight)
{
    vec3 direction = mat3(inverse(model)) * dirlight.dir;

    float diffuse = dot(normalize(direction), normal);

    diffuse = diffuse + diffuse * diffuse * diffuse;
   
    float lightShadow  = diffuse * dirlight.ruShadow  + dirlight.duShadow;
    float lightMidtone = diffuse * dirlight.ruMidtone + dirlight.duMidtone;

    diffuse = diffuse * dirlight.ruHighlight + dirlight.duHighlight;

    diffuse = max(diffuse, 0.0);

    objectShadow  += max(lightShadow,  0.0);
    objectMidtone += max(lightMidtone, 0.0);

    return vec4(dirlight.color, 0.0) * diffuse;
}

vec4 AddPositionLight(POINTLIGHT pointlight)
{
    vec4 posWorld = model * vec4(vertex, 1.0);
    vec3 test = vec3(posWorld) / posWorld.w;

    vec3 normalWorld = mat3(model) * normal;

    vec3 direction = normalize(pointlight.pos - test);
    float distance = length(pointlight.pos - test);

    float diffuse = dot(direction, normalize(normalWorld));

    float attenuation = 1.0 / distance * pointlight.falloff.y + pointlight.falloff.x;

    diffuse = diffuse + diffuse * diffuse * diffuse;

    float ruShadow = 0.0;

    if (attenuation < 0.0)
    {
        attenuation = 0.0;
        ruShadow = pointlight.ruShadow;
    }

    else
    {
        if (attenuation > 1.0)
            attenuation = 1.0;

        ruShadow = pointlight.ruShadow;
    }

    float lightShadow  = diffuse * ruShadow + pointlight.duShadow;
    float lightMidtone = diffuse * pointlight.ruMidtone + pointlight.duMidtone;

    diffuse = diffuse * pointlight.ruHighlight + pointlight.duHighlight;

    diffuse = max(diffuse, 0.0) * attenuation;

    objectShadow  += max(lightShadow,  0.0) * attenuation;
    objectMidtone += max(lightMidtone, 0.0) * attenuation;

    return vec4(pointlight.color, 0.0) * diffuse;
}

void ProcessGlobLighting()
{
    float litR = light.r;
    float litG = light.g;
    float litB = light.b;

	if (litG < litR)
		litG = max(litB, litR);
	else
	    litG = max(litB, litG);

	litG = 1.0 - litG;

	litB = litG - objectMidtone;

	objectMidtone = min(objectMidtone, litG);
	objectMidtone = max(objectMidtone, 0.0);
	litG = min(litB, objectShadow);

	litG = max(litG, 0.0);

    float intensity = (vertexColor.r + vertexColor.g + vertexColor.b) * 0.3333333;
    
    ambient.r = litG * intensity;
    ambient.g = litG * intensity;
    ambient.b = litG * intensity;

    midtone.r = objectMidtone * vertexColor.r;
    midtone.g = objectMidtone * vertexColor.g;
    midtone.b = objectMidtone * vertexColor.b;

    light.r = min(light.r, 1.0) * intensity;
    light.g = min(light.g, 1.0) * intensity;
    light.b = min(light.b, 1.0) * intensity;
}