#version 430 core

#define RKO_ThreeWay 0
#define RKO_OneWay   1

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
}; 
uniform SWP swp;

// Camera / world-to-clip (kept as you had it)
layout(std430, binding = 0) readonly buffer CMGL
{
    mat4 matWorldToClip;
    vec4 cameraPos;
} cm;

// Render object properties
layout(std140, binding = 1) uniform RO
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
	float uAlphaCelBorder; // 112
} op;

out vec4 vColor;
out vec2 vTexcoord;
out vec3 vWorldPos;
out vec3 vNormalWorld;
out float vFogIntensity;

void CalculateFog();
void CalculateFogPS2();
void CalculateFogPS3();

vec4 worldPos;

void main()
{
    // World space position & normal
    worldPos    = op.model * vec4(vertex, 1.0);
    vWorldPos   = worldPos.xyz;
    vNormalWorld = normalize(mat3(op.model) * normal);

    vColor      = color;
    vTexcoord   = uv;

    // Per-vertex fog factor (still fine to interpolate)
    if (swp.fogType != 0)
        CalculateFog();

    gl_Position = cm.matWorldToClip * worldPos;
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

    vFogIntensity = fog * fogMult;
}

void CalculateFogPS3()
{
    vec3  offset = cm.cameraPos.xyz - worldPos.xyz;
    float distance2 = dot(offset, offset);
    float distanceToCamera = sqrt(distance2);

    float invFogRange = 1.0 / max(swp.fogFar - swp.fogNear, 1e-6);

    float fog = clamp((distanceToCamera - swp.fogNear) * invFogRange, 0.0, 1.0);

    float fogMult = mix(swp.fogMax, swp.fogMax * op.uFog, step(0.001, op.uFog));

    vFogIntensity = fog * fogMult;
}
