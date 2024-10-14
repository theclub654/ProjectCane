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
uniform vec3 lightFalloff;

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
vec3 MultiplyMatrix4Vector(mat4 pmat4 ,vec3 pvecIn, float gImplied);
// Resets the model lighting
void ClearGlobLighting();
void ProcessGlobLighting();
// NOT DONE, STILL GOTTA DO THE SHADOWS
vec4 AddDirectionLight(vec3 direction, vec3 color, vec3 falloff);
// NOT DONE, STILL GOTTA DO THE SHADOWS
vec4 AddPositionLight( vec3 position,  vec3 color, vec3 falloff);
// NOT DONE
void AddFrustrumLight();

void main()
{
    vertexColor = color;
    texcoord    = uv;

    if (fThreeWay == 1)
    {
        ClearGlobLighting();
        //lighting.lit += AddDirectionLight(lightDir, lightColor, lightFalloff);
        ProcessGlobLighting();
    }

    gl_Position = proj * view * model * vec4(vertex, 1.0);
}

vec3 MultiplyMatrix4Vector(mat4 pmat4 ,vec3 pvecIn, float gImplied)
{
    vec3 sum;

    sum[0] = pmat4[0][0] * pvecIn[0];
    sum[1] = pmat4[0][1] * pvecIn[1];
    sum[2] = pmat4[0][2] * pvecIn[2];

    return sum;
}

void ClearGlobLighting()
{
    shadowSaturation  = uShadow;
    diffuseSaturation = uMidtone + usSelfIllum * 0.31;
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

    float aColor = (vertexColor.r + vertexColor.g + vertexColor.b) * 0.3333333;

    vertexColor.r = litG * aColor;
    vertexColor.g = litG * aColor;
    vertexColor.b = litG * aColor;

    litR = lighting.lit.r;
    litG = lighting.lit.g;
    litB = lighting.lit.b;

    lighting.saturate.r = min(litR, 1.0) * aColor;
    lighting.saturate.g = min(litG, 1.0) * aColor;
    lighting.saturate.b = min(litB, 1.0) * aColor;
    lighting.saturate.a = litG * 255.0;

    ambient  = lighting.ambient;
    saturate = lighting.saturate;
}

vec4 AddDirectionLight(vec3 direction, vec3 color, vec3 falloff)
{
//    float diffuse = dot(normal, direction);
//
//    float lightShadow     = diffuse * ruShadow  + duShadow;
//    float lightMidtone    = diffuse * ruMidtone + duMidtone;
//    float lightHightLight = ruHighlight + duHighlight;
//
//    diffuse += ruHighlight + duHighlight;
//    diffuse = max(diffuse, 0.0);
//
//    shadowSaturation  += max(lightShadow,  0.0);
//    diffuseSaturation += max(lightMidtone, 0.0);
//    
//    return vec4(color, 0.0) * diffuse;

      vec3 normalLocal = MultiplyMatrix4Vector(model, falloff, 0.0);
      normalLocal *= normalLocal;
      
      return vec4(0.0);
}

vec4 AddPositionLight(vec3 position, vec3 color, vec3 falloff)
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

void AddFrustrumLight()
{
    
}