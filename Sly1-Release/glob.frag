#version 430 core

#define RKO_ThreeWay  0
#define RKO_OneWay    1
#define RKO_CelBorder 2
#define RKO_Collision 3

uniform sampler2D shadowMap;
uniform sampler2D diffuseMap;
uniform sampler2D saturateMap;

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

struct MATERIAL
{
    float ambient;
    vec3  midtone;
    vec3  light;
};

uniform int  fAlphaTest;
uniform vec4 rgbaCel;
uniform vec4 collisionRgba;

in vec4 vertexColor;
in vec2 texcoord;
in MATERIAL material;
in float fogIntensity;

out vec4 FragColor;

void DrawOneWay();
void DrawThreeWay();
void DrawCelBorder();
void DrawCollision();
void ApplyFog();

void main()
{
    FragColor = vec4(0.0);

    switch (op.rko)
    {
        case RKO_OneWay:
        DrawOneWay();
        ApplyFog();
        break;

        case RKO_ThreeWay:
        DrawThreeWay();
        ApplyFog();
        break;

        case RKO_CelBorder:
        DrawCelBorder();
        break;

        case RKO_Collision:
        DrawCollision();
        break;
    }
}

void DrawOneWay()
{
    vec4 diffuse = texture(diffuseMap, texcoord);

    FragColor = vertexColor * diffuse;

    float alphaIn = clamp(vertexColor.a * diffuse.a, 0.0, 1.0);

    if (fAlphaTest == 1 && alphaIn < 0.9)
        discard;

    FragColor.a = clamp(FragColor.a * op.uAlpha, 0.0, 1.0);
}

void DrawThreeWay()
{
    vec4 shadow   = texture(shadowMap,   texcoord);
    vec4 diffuse  = texture(diffuseMap,  texcoord);
    vec4 saturate = texture(saturateMap, texcoord);

    // Alpha test first
    float alphaIn = clamp(vertexColor.a * diffuse.a, 0.0, 1.0);
    if (fAlphaTest == 1 && alphaIn < 0.9)
        discard;

    FragColor.rgb += shadow.rgb   * material.ambient     * op.darken;
    FragColor.rgb += diffuse.rgb  * material.midtone.rgb * op.darken;
    FragColor.rgb += saturate.rgb * material.light.rgb;

    float finalAlpha = clamp(vertexColor.a * diffuse.a, 0.0, 1.0);

    FragColor.a = clamp(finalAlpha * op.uAlpha, 0.0, 1.0);
}

void DrawCelBorder()
{
    FragColor = rgbaCel;
    FragColor.a = clamp(FragColor.a * op.uAlpha, 0.0, 1.0);
}

void DrawCollision()
{
    FragColor = collisionRgba;
}

void ApplyFog()
{
    if (swp.fogType == 0)
        return;

    FragColor.rgb = mix(FragColor.rgb, swp.fogColor.rgb, fogIntensity);
}