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
    int clampS;
    int clampT;
    int _pad0;
    glm::vec4 posEffect;
    float sRadiusEffect;
    int   fDynamic;
    int   shdk;
    int   _pad1;
    glm::vec4 normalCast;
};

#ifdef CANE_GL41
// SHADOWBLK is uploaded as-is and read by glob's FetchShadow as 14 RGBA32UI
// texels (kShadowTextureFormat); these pin the texel each field lands in.
static_assert(sizeof(SHADOWBLK) == 224, "SHADOWBLK is 14 RGBA32UI texels");
static_assert(offsetof(SHADOWBLK, matWorldToUv) == 0, "texels 0-3");
static_assert(offsetof(SHADOWBLK, matClipToUv) == 64, "texels 4-7");
static_assert(offsetof(SHADOWBLK, rgba) == 128, "texel 8");
static_assert(offsetof(SHADOWBLK, wMin) == 144, "texel 9.x");
static_assert(offsetof(SHADOWBLK, wMax) == 148, "texel 9.y");
static_assert(offsetof(SHADOWBLK, gReserved) == 152, "texel 9.z");
static_assert(offsetof(SHADOWBLK, wFadeMin) == 156, "texel 9.w");
static_assert(offsetof(SHADOWBLK, textureSlot) == 160, "texel 10.x");
static_assert(offsetof(SHADOWBLK, clampS) == 164, "texel 10.y");
static_assert(offsetof(SHADOWBLK, clampT) == 168, "texel 10.z");
static_assert(offsetof(SHADOWBLK, _pad0) == 172, "texel 10.w");
static_assert(offsetof(SHADOWBLK, posEffect) == 176, "texel 11");
static_assert(offsetof(SHADOWBLK, sRadiusEffect) == 192, "texel 12.x");
static_assert(offsetof(SHADOWBLK, fDynamic) == 196, "texel 12.y");
static_assert(offsetof(SHADOWBLK, shdk) == 200, "texel 12.z");
static_assert(offsetof(SHADOWBLK, _pad1) == 204, "texel 12.w");
static_assert(offsetof(SHADOWBLK, normalCast) == 208, "texel 13");

#endif
struct alignas(16) SHADOWSSBOHEADER
{
    int numShadows;
    int pad[3];
};
#ifdef CANE_GL41
static_assert(sizeof(SHADOWSSBOHEADER) == 16, "the header is shadowTex texel 0");
#endif

struct ACTIVESHADOWS
{
    int numShadows;
    int shadowsIndices[MAX_SHADOWS];
};
#ifdef CANE_GL41
static_assert(sizeof(ACTIVESHADOWS) == 1024, "ACTIVESHADOWS is 256 R32I texels");
#endif

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
