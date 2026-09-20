#pragma once
#include "lo.h"

#define MAX_SHADOWS 255

// Texture units 0-2 are reserved for the ambient, diffuse, and saturate
// material maps. All projected shadows share one array texture on unit 3.
inline constexpr int SHADOW_TEXTURE_UNIT = 3;
inline constexpr int MAX_PROJECTED_SHADOW_LAYERS = 64;

struct SHADOWBLK
{
    glm::mat4 matWorldToUv;
    glm::mat4 matClipToUv;
    glm::vec4 rgba;
    float wMin;
    float wMax;
    float gReserved;
    float wFadeMin;
    int textureSlot;
    int _pad0[3];
    glm::vec4 posEffect;
    float sRadiusEffect;
    int   fDynamic;
    int   shdk;
    int   _pad1;
    glm::vec4 normalCast;
};

struct alignas(16) SHADOWSSBOHEADER
{
    int numShadows;
    int pad[3];
};

struct ACTIVESHADOWS
{
    int numShadows;
    int shadowsIndices[MAX_SHADOWS];
};

class SHADOW
{
    public:
    SHD* pshd;
    float sNearRadius;
    float sFarRadius;
    glm::vec3 posCast;
    glm::vec3 normalCast;
    glm::vec3 vecUp;
    glm::mat4 matShadowProj;
    glm::mat4 matShadowView;
    float sNearCast;
    float sFarCast;
    glm::vec3 posEffect;
    float sRadiusEffect;
    glm::mat4 matWorldToUv;
    OID oidDysh;
    struct DYSH* pdysh;
    DLE dle;
    SHADOWBLK rsh;
    int ssboIndex;
    int textureSlot;
    GLuint glTexture;
};

void InitSwShadowDl(SW* psw);
void InitShadow(SHADOW* pshadow);
void SetShadowShader(SHADOW* pshadow, OID oidShdShadow);
void SetShadowNearRadius(SHADOW* pshadow, float sNearRadius);
void SetShadowFarRadius(SHADOW* pshadow, float sFarRadius);
void SetShadowNearCast(SHADOW* pshadow, float sNearCast);
void SetShadowFarCast(SHADOW* pshadow, float sFarCast);
void SetShadowConeAngle(SHADOW* pshadow, float degConeAngle);
void SetShadowFrustrumUp(SHADOW* pshadow, glm::vec3* pvecUp);
void SetShadowCastPosition(SHADOW* pshadow, const glm::vec3& posCast);
void SetShadowCastNormal(SHADOW* pshadow, const glm::vec3& normalCast);
int  FShadowValid(SHADOW* pshadow, GRFGLOB grfglob);
int  FShadowRadiusSet(SHADOW* shadow);
//   Checks if a given sphere (e.g. a model's bounding sphere) intersects with a shadow's effective area.
int  FShadowIntersectsSphere(SHADOW* pshadow, const glm::vec3& pos, float sRadius);
void FindSwShadows(SW* psw, glm::vec3* ppos, float sRadius, int cpshadowMax, int* pcpshadow, SHADOW** apshadow);
void PostShadowLoad(SHADOW* pshadow);
void RebuildShadowRegion(SHADOW* pshadow);
void CombineShadowEyeLookAtProj(const glm::vec3& posEye, const glm::mat3& matLookAt, const glm::mat4& matProj, glm::mat4& out);
int FFilterFastShadows(void*, void* pvso);
void UpdateShadow(SHADOW* pshadow, float dt);
void RebuildShadow(SHADOW* pshadow);
void AllocateShadows(SW* psw);
void PrepareSwShadows(SW* psw, CM* pcm);
void BindSwShadowTextures(SW* psw);
void DeallocateSwShadows();

extern GLuint shadowSsbo;
extern GLuint activeShadowsSsbo;
extern std::vector <SHADOWBLK> shadowBlk;
extern ACTIVESHADOWS activeShadows;
extern DL g_dlShadowPending;
