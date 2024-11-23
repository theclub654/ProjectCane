#version 330 core

#define SHDK_ThreeWay 0

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 uv;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform mat4 matWorldToClip;

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

uniform int numDirLights;
uniform int numPointLights;

uniform int shdk;
uniform float usSelfIllum;

out vec4 vertexColor;
out vec2 texcoord;

out vec4 ambient;
out vec4 illumination;
out vec4 light;

vec4 lit;

float objectShadow;
float objectIllum;

// Multiplys a mat4 with vec3
vec4 MultiplyMatrixVector(mat4 matrix4 ,vec3 vecIn, float gImplied);
// Resets the model lighting
void ClearGlobLighting();
void ProcessGlobLighting();
// NOT DONE, STILL GOTTA DO THE SHADOWS
vec4 AddDirectionLight(DIRLIGHT dirlight);
// NOT DONE, STILL GOTTA DO THE SHADOWS
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

//        for (int i = 0; i < numPointLights; i++)
//            lit += AddPositionLight(pointlights[i]);

        ProcessGlobLighting();
    }

    gl_Position = proj * view * model * vec4(vertex, 1.0);
    //gl_Position = matWorldToClip * model * vec4(vertex, 1.0);
}

vec4 MultiplyMatrixVector(mat4 matrix4 ,vec3 vecIn, float gImplied)
{
    return matrix4 * vec4(vecIn, gImplied);
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

    illumination = objectIllum * vertexColor;

    float intensity = (vertexColor.r + vertexColor.g + vertexColor.b) * 0.3333333;

    ambient.r = litG * intensity;
    ambient.g = litG * intensity;
    ambient.b = litG * intensity;

    light.r = min(lit.r, 1.0) * intensity;
    light.g = min(lit.g, 1.0) * intensity;
    light.b = min(lit.b, 1.0) * intensity;
    light.a = litG * 255.0;
}

vec4 AddDirectionLight(DIRLIGHT dirlight)
{
    vec4 orientation = MultiplyMatrixVector(model, dirlight.dir, 0.0);

    orientation.x = length(orientation);
    
    if (orientation.x < 0.0001)
        orientation = vec4(0.0);
    else
        orientation = 1.0 / orientation.x * vec4(dirlight.dir, 0.0);

    vec4 aNormal = vec4(normal, 1.0) * orientation;
    dirlight.dir.x = 1.0;

    float temp = (aNormal.y + aNormal.x);
    aNormal.x  =  (aNormal.z * dirlight.dir.z) + temp;

    float diffuse = aNormal.x;
    diffuse = diffuse + diffuse * diffuse * diffuse;
   
    float lightShadow  = diffuse * dirlight.ruShadow  + dirlight.duShadow;
    float lightMidtone = diffuse * dirlight.ruMidtone + dirlight.duMidtone;

    diffuse = diffuse * dirlight.ruHighlight + dirlight.duHighlight;

    diffuse = max(diffuse, 0.0);

    objectShadow += max(lightShadow,  0.0);
    objectIllum  += max(lightMidtone, 0.0);

    return vec4(dirlight.color, 1.0) * diffuse;
}

vec4 AddPositionLight(POINTLIGHT pointlight)
{
    vec4 posWorld = MultiplyMatrixVector(model, vertex, 1.0);
    vec4 normalWorld = MultiplyMatrixVector(model, normal, 0.0);

    vec4 aNormal = normalWorld;
    aNormal *= aNormal;
    vec4 pos;
    pos.x = 1.0;
    aNormal.x = aNormal.x + aNormal.y;
    aNormal.x = (aNormal.z * pos.x) + aNormal.x;
    aNormal.x = sqrt(aNormal.x);

    if (aNormal.x >= 0.0001)
        aNormal = 1.0 / aNormal.x * vec4(normal, 0.0);

    normalWorld = aNormal;
    pos = posWorld;

    vec4 aVertex = vec4(vertex, 0.0);
    aVertex = aVertex - pos;
    pos = aVertex * aVertex;

    vec4 aNormal1;
    aNormal1.x = 1.0;

    //float temp = 

    return vec4(0.0);
}

void AddFrustrumLight(vec3 color, mat4 matFrustum, vec3 direction, vec3 falloff)
{
    
}