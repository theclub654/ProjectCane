#version 330 core

#define SHDK_ThreeWay 0

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 uv;

uniform mat4 proj;
uniform mat4 view;
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
    vec3  dir;
    vec3  color;

    float ruShadow;
    float ruMidtone;
    float ruHighlight;

    float duShadow;
    float duMidtone;
    float duHighlight;

}; uniform DIRLIGHT dirlights[100];

struct POINTLIGHT
{
    vec3  pos;
    vec3  color;
    vec3  falloff;

    float ruShadow;
    float ruMidtone;
    float ruHighlight;

    float duShadow;
    float duMidtone;
    float duHighlight;

}; uniform POINTLIGHT pointlights[100];

uniform int shdk;
uniform float usSelfIllum;

uniform int numDirLights;
uniform int numPointLights;

out vec4 ambient;
out vec4 illumination;
out vec4 light;

vec4 lit;

float objectShadow;
float objectIllum;

// Resets the model lighting
void ClearGlobLighting();
void ProcessGlobLighting();
// NOT DONE
vec4 AddDirectionLight(DIRLIGHT dirlight);
// NOT DONE
vec4 AddPositionLight(POINTLIGHT pointlight);
// NOT DONE
void AddFrustrumLight(vec3 color, mat4 matFrustum, vec3 direction, vec3 falloff);

void main()
{
    vertexColor = color;
    texcoord    = uv;
    
    if (shdk == SHDK_ThreeWay)
    {
        ClearGlobLighting();

        for (int i = 0; i < numDirLights; i++)
            lit += AddDirectionLight(dirlights[i]);

//        for (int i = 0; i < 1; i++)
//            lit += AddPositionLight(pointlights[0]);

        ProcessGlobLighting();
    }

     gl_Position = proj * view * model * vec4(vertex, 1.0);
}

void ClearGlobLighting()
{
    objectShadow = lsm.uShadow;
    objectIllum  = lsm.uMidtone + usSelfIllum * 0.000031;
}

void ProcessGlobLighting()
{
    float litR = lit.r;
    float litG = lit.g;
    float litB = lit.b;

	if (litG < litR)
		litG = max(litB, litR);
	else
	    litG = max(litB, litG);

	litG = 1.0 - litG;

	litB = litG - objectIllum;

	objectIllum = min(objectIllum, litG);
	objectIllum = max(objectIllum, 0.0);
	litG = min(litB, objectShadow);

	litG = max(litG, 0.0);

    float intensity = (vertexColor.r + vertexColor.g + vertexColor.b) * 0.3333333;
    
    ambient.r = litG * intensity;
    ambient.g = litG * intensity;
    ambient.b = litG * intensity;

    illumination.r = objectIllum * vertexColor.r;
    illumination.g = objectIllum * vertexColor.g;
    illumination.b = objectIllum * vertexColor.b;

    light.r = min(lit.r, 1.0) * intensity;
    light.g = min(lit.g, 1.0) * intensity;
    light.b = min(lit.b, 1.0) * intensity;
    light.a = litG * 255.0;
}

vec4 AddDirectionLight(DIRLIGHT dirlight)
{
    vec3 direction = mat3(transpose(model)) * dirlight.dir;

    float diffuse = dot(normalize(direction), normal);

    diffuse = diffuse + diffuse * diffuse * diffuse;
   
    float lightShadow  = diffuse * dirlight.ruShadow  + dirlight.duShadow;
    float lightMidtone = diffuse * dirlight.ruMidtone + dirlight.duMidtone;

    diffuse = diffuse * dirlight.ruHighlight + dirlight.duHighlight;

    diffuse = max(diffuse, 0.0);

    objectShadow += max(lightShadow,  0.0);
    objectIllum  += max(lightMidtone, 0.0);

    return vec4(dirlight.color, 0.0) * diffuse;
}

vec4 AddPositionLight(POINTLIGHT pointlight)
{
    return vec4(0.0);
}

void AddFrustrumLight(vec3 color, mat4 matFrustum, vec3 direction, vec3 falloff)
{
    
}