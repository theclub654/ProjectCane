#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 uv;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform float uShadow;
uniform float uMidtone;

uniform vec3 lightDir;
uniform vec3 lightColor;

uniform float ruShadow;
uniform float ruMidtone;
uniform float ruHighlight;

uniform float duShadow;
uniform float duMidtone;
uniform float duHighlight;

uniform int   fThreeWay;
uniform float usSelfIllum;

out vec4 vertexColor;
out vec2 texcoord;

out vec4 ambient;
out vec4 saturate;
out vec4 lit;

float shadowSaturation;
float diffuseSaturation;

struct MATERIAL
{
    vec4 ambient;
    vec4 lit;
    vec4 saturate;
};

MATERIAL lighting;

// Multiplys a mat4 with vec3
vec4 MultiplyMatrix4Vector(mat4 matrix4 ,vec3 vecIn, float gImplied);
// Resets the model lighting
void ClearGlobLighting();
void ProcessGlobLighting();
// NOT DONE, STILL GOTTA DO THE SHADOWS
vec4 AddDirectionLight(vec3 color, vec3 direction);
// NOT DONE, STILL GOTTA DO THE SHADOWS
vec4 AddPositionLight(vec3 color, vec3 position, vec3 falloff);
// NOT DONE
void AddFrustrumLight(vec3 color, mat4 matFrustum, vec3 direction, vec3 falloff);

void main()
{
    vertexColor = color;
    texcoord    = uv;

    if (fThreeWay == 1)
    {
        ClearGlobLighting();
        lighting.lit += AddDirectionLight(lightColor, lightDir);
        ProcessGlobLighting();
    }

    gl_Position = proj * view * model * vec4(vertex, 1.0);
}

vec4 MultiplyMatrix4Vector(mat4 matrix4 ,vec3 vecIn, float gImplied)
{
    return matrix4 * vec4(vecIn, gImplied);
}

void ClearGlobLighting()
{
    shadowSaturation  = uShadow;
    //diffuseSaturation = uMidtone + usSelfIllum * 0.31;
    diffuseSaturation = uMidtone + usSelfIllum * 0.000031;

    lighting.lit.r = 0.0;
    lighting.lit.g = 0.0;
    lighting.lit.b = 0.0;
}

void ProcessGlobLighting()
{
    float litR = lighting.lit.r;
    float litG = lighting.lit.g;
    float litB = lighting.lit.b;

	if (litG < litR)
		litG = max(litB, litR);
	else
	    litG = max(litB, litG);

	litG = 1.0 - litG;

	litB = litG - diffuseSaturation;

	diffuseSaturation = min(diffuseSaturation, litG);
	diffuseSaturation = max(diffuseSaturation, 0.0);
	litG = min(litB, shadowSaturation);

	litG = max(litG, 0.0);

    lighting.ambient.r = diffuseSaturation * vertexColor.r;
    lighting.ambient.g = diffuseSaturation * vertexColor.g;
    lighting.ambient.b = diffuseSaturation * vertexColor.b;
    lighting.ambient.a = diffuseSaturation * 255.0;

    float intensity = (vertexColor.r + vertexColor.g + vertexColor.b) * 0.3333333;

    vertexColor.r = litG * intensity;
    vertexColor.g = litG * intensity;
    vertexColor.b = litG * intensity;

    litR = lighting.lit.r;
    litG = lighting.lit.g;
    litB = lighting.lit.b;

    lighting.saturate.r = min(litR, 1.0) * intensity;
    lighting.saturate.g = min(litG, 1.0) * intensity;
    lighting.saturate.b = min(litB, 1.0) * intensity;
    lighting.saturate.a = litG * 255.0;

    ambient  = lighting.ambient;
    saturate = lighting.saturate;
    lit      = lighting.lit;
}

vec4 AddDirectionLight(vec3 color, vec3 direction)
{
    // Very sloppy will redo this in a simpler way when i get the time
    vec4 normalLocal = MultiplyMatrix4Vector(model, direction, 0.0);
    normalLocal *= normalLocal;
    vec4 falloff0;
    falloff0.x = 1.0;
    normalLocal.x = normalLocal.x + normalLocal.y;
    normalLocal.x = (normalLocal.z * falloff0.x) + normalLocal.x;
    normalLocal.x = sqrt(normalLocal.x);

    normalLocal = (normalLocal.x / 1.0) * vec4(direction, 1.0);

    vec4 aNormal = normalLocal * vec4(normal, 1.0);
    direction.x = 1.0;

    float temp = (aNormal.y + aNormal.x);
    aNormal.x =  (aNormal.z * direction.z) + temp;

    float diffuse = aNormal.x;
    diffuse = diffuse + diffuse * diffuse * diffuse;
    
    float shadow  = diffuse * ruShadow  + duShadow;
    float midtone = diffuse * ruMidtone + duMidtone;

    diffuse = diffuse * ruHighlight + duHighlight;

    diffuse = max(diffuse, 0.0);

    shadowSaturation  += max(shadow,  0.0);
    diffuseSaturation += max(midtone, 0.0);

    return vec4(color, 0.0) * diffuse;
}

vec4 AddPositionLight(vec3 color, vec3 position, vec3 falloff)
{
//    vec3  lightDirection = normalize(position - vertex);
//    float distance = length(position - vertex);
//    float attenuation = 1.0 / distance * (falloff.x + falloff.y);
//
//    if (attenuation < 0.0)
//        attenuation = 0.0;
//    else if (0.1 < attenuation)
//        attenuation = 1.0;
//
//    float diffuse = dot(normal, lightDirection) * attenuation;
//
//    return vec4(color, 1.0) * diffuse;
      return vec4(0.0);
}

void AddFrustrumLight(vec3 color, mat4 matFrustum, vec3 direction, vec3 falloff)
{
    
}