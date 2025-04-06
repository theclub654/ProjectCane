#version 330 core

#define SHDK_ThreeWay 0

#define RKO_OneWay 0
#define RKO_ThreeWay 1
#define RKO_CelBorder 2

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

uniform int shdk;
uniform float usSelfIllum;

uniform int rko;

uniform int fDynamic;
uniform vec3 posCenter;

out vec4 ambient;
out vec4 midtone;
out vec4 light;

float objectShadow;
float objectMidtone;

int fLit;

void StartThreeWay();
void InitGlobLighting();
vec4 AddDirectionLight(DIRLIGHT dirlight);
vec4 AddDynamicLight(vec3 dir, vec3 color, LTFN ltfn);
vec4 AddPositionLight(POINTLIGHT pointlight);
vec4 AddPositionLightDynamic(POINTLIGHT pointlight);
vec4 AddFrustrumLight(FRUSTUMLIGHT frustumlight);
vec4 AddFrustrumLightDynamic(FRUSTUMLIGHT frustumlight);
void ProcessGlobLighting();

void main()
{
    vertexColor = color;
    texcoord    = uv;

    switch (rko)
    {
        case RKO_OneWay:
        gl_Position = matWorldToClip * model * vec4(vertex, 1.0);
        break;

        case RKO_ThreeWay:
        StartThreeWay();
        gl_Position = matWorldToClip * model * vec4(vertex, 1.0);
        break;

        case RKO_CelBorder:
        gl_Position = matWorldToClip * model * vec4(vertex, 1.0);
        break;
    }
}

void StartThreeWay()
{
    if (fDynamic != 1)
        {
            fLit = 1;
            InitGlobLighting();
            for (int i = 0; i < numDirLights; i++)
                light += AddDirectionLight(dirlights[i]);

            for (int i = 0; i < numPointLights; i++)
                light += AddPositionLight(pointlights[i]);

            for (int i = 0; i < numFrustumLights; i++)
                light += AddFrustrumLight(frustumlights[i]);
        }
        else
        {
            fLit = 0;
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
    vec3 direction = mat3(inverse(model)) * dirlight.dir;
    
    float diffuse = dot(normalize(direction), normal);

    diffuse = diffuse + diffuse * diffuse * diffuse;
   
    float lightShadow  = diffuse * dirlight.ltfn.ruShadow  + dirlight.ltfn.duShadow;
    float lightMidtone = diffuse * dirlight.ltfn.ruMidtone + dirlight.ltfn.duMidtone;

    diffuse = diffuse * dirlight.ltfn.ruHighlight + dirlight.ltfn.duHighlight;

    diffuse = max(diffuse, 0.0);

    objectShadow  += max(lightShadow,  0.0);
    objectMidtone += max(lightMidtone, 0.0);

    return vec4(dirlight.color, 0.0) * diffuse;
}

vec4 AddDynamicLight(vec3 dir, vec3 color, LTFN ltfn)
{
    vec3 direction = mat3(inverse(model)) * dir;

    if (fLit == 0)
        InitGlobLighting();

    float diffuse = dot(normalize(direction), normal);
    diffuse = diffuse + diffuse * diffuse * diffuse;

    float lightShadow = ltfn.duShadow + diffuse * ltfn.ruShadow;

    float lightMidtone = ltfn.duMidtone + diffuse * ltfn.ruMidtone;

    diffuse = ltfn.duHighlight + diffuse * ltfn.ruHighlight;
    diffuse = max(diffuse, 0.0);

    objectShadow  += max(lightShadow, 0.0);
    objectMidtone += max(lightMidtone, 0.0);

    fLit = 1;

    return vec4(color, 0.0) * diffuse;
}

vec4 AddPositionLight(POINTLIGHT pointlight)
{
    vec4 pos = model * vec4(vertex, 1.0);
    vec3 posWorld = vec3(pos) / pos.w;
    vec3 normalWorld = mat3(model) * normal;
    
    vec3 direction = normalize(pointlight.pos - posWorld);
    float distance = length(pointlight.pos - posWorld);

    float attenuation = 1.0 / distance * pointlight.falloff.y + pointlight.falloff.x;

    float diffuse = dot(direction, normalize(normalWorld));

    diffuse = diffuse + diffuse * diffuse * diffuse;

    float ruShadow = 0.0;

    if (attenuation < 0.0)
    {
        attenuation = 0.0;
        ruShadow = pointlight.ltfn.ruShadow;
    }
    else
    {
        if (attenuation > 1.0)
            attenuation = 1.0;

        ruShadow = pointlight.ltfn.ruShadow;
    }

    float lightShadow  = diffuse * ruShadow + pointlight.ltfn.duShadow;
    float lightMidtone = diffuse * pointlight.ltfn.ruMidtone + pointlight.ltfn.duMidtone;

    diffuse = diffuse * pointlight.ltfn.ruHighlight + pointlight.ltfn.duHighlight;

    diffuse = max(diffuse, 0.0) * attenuation;

    objectShadow  += max(lightShadow,  0.0) * attenuation;
    objectMidtone += max(lightMidtone, 0.0) * attenuation;

    return vec4(pointlight.color, 0.0) * diffuse;
}

vec4 AddPositionLightDynamic(POINTLIGHT pointlight)
{
    vec4 poscenter = model * vec4(posCenter, 1.0);
    vec3 posCenterWorld = vec3(poscenter) / poscenter.w;

    vec3 direction = normalize(pointlight.pos - posCenterWorld);
    float distance = length(pointlight.pos - posCenterWorld);

    float attenuation = 1.0 / distance * pointlight.falloff.y + pointlight.falloff.x;

    float falloff = 0.0;
    if ((0.0 <= attenuation) && (falloff = attenuation, 1.0 < attenuation))
        falloff = 1.0;

    LTFN ltfn;

    ltfn.ruShadow = pointlight.ltfn.ruShadow * falloff;
    ltfn.duShadow = pointlight.ltfn.duShadow * falloff;

    ltfn.ruMidtone = pointlight.ltfn.ruMidtone * falloff;
    ltfn.duMidtone = pointlight.ltfn.duMidtone * falloff;

    ltfn.ruHighlight = pointlight.ltfn.ruHighlight * falloff;
    ltfn.duHighlight = pointlight.ltfn.duHighlight * falloff;

    return AddDynamicLight(direction, pointlight.color, ltfn);
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
    if (fLit == 0)
    {
        InitGlobLighting();
        fLit = 1;
    }

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