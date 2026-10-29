#pragma once
#include "alo.h"

#define MAX_LIGHTS 256

// Light type
enum LIGHTK : int
{
	LIGHTK_Nil = -1,
	LIGHTK_Direction = 0,
	LIGHTK_Position = 1,
	LIGHTK_Frustrum = 2,
	LIGHTK_Spot = 3,
	LIGHTK_Max = 4
};

struct LTFN
{
	float ruShadow;
	float ruMidtone;
	float ruHighlight;
	float ruUnused;

	float duShadow;
	float duMidtone;
	float duHighlight;
	float duUnused;
};

struct alignas(16) LightSSBOHeader
{
	int numLights;
	int pad[3];
};
#ifdef CANE_GL41
static_assert(sizeof(LightSSBOHeader) == 16, "the header is lightTex texel 0");
#endif

struct LIGHTBLK
{
	int  lightk;
	int  fExcludeDynamicObjects;
	int  fActive;
	int  fDynamic;
	glm::vec4 pos;
	glm::vec4 dir;
	glm::vec4 color;
	float constant;
	float invDst;
	float pad4;
	float dst;
	glm::vec4 ru;
	glm::vec4 du;
	glm::mat4 matFrustrum;
	glm::vec4 falloffScale;
	glm::vec4 falloffBias;
};

#ifdef CANE_GL41
// LIGHTBLK is uploaded as-is and read by glob.vert's FetchLightAt as 13
// RGBA32UI texels (kLightTextureFormat); these pin the texel each field lands in.
static_assert(sizeof(LIGHTBLK) == 208, "LIGHTBLK is 13 RGBA32UI texels");
static_assert(offsetof(LIGHTBLK, lightk) == 0, "texel 0.x");
static_assert(offsetof(LIGHTBLK, fExcludeDynamicObjects) == 4, "texel 0.y");
static_assert(offsetof(LIGHTBLK, fActive) == 8, "texel 0.z");
static_assert(offsetof(LIGHTBLK, fDynamic) == 12, "texel 0.w");
static_assert(offsetof(LIGHTBLK, pos) == 16, "texel 1");
static_assert(offsetof(LIGHTBLK, dir) == 32, "texel 2");
static_assert(offsetof(LIGHTBLK, color) == 48, "texel 3");
static_assert(offsetof(LIGHTBLK, constant) == 64, "texel 4.x");
static_assert(offsetof(LIGHTBLK, invDst) == 68, "texel 4.y");
static_assert(offsetof(LIGHTBLK, pad4) == 72, "texel 4.z");
static_assert(offsetof(LIGHTBLK, dst) == 76, "texel 4.w");
static_assert(offsetof(LIGHTBLK, ru) == 80, "texel 5");
static_assert(offsetof(LIGHTBLK, du) == 96, "texel 6");
static_assert(offsetof(LIGHTBLK, matFrustrum) == 112, "texels 7-10");
static_assert(offsetof(LIGHTBLK, falloffScale) == 176, "texel 11");
static_assert(offsetof(LIGHTBLK, falloffBias) == 192, "texel 12");

#endif
struct ACTIVELIGHTS
{
	int numStaticLights;
	int staticLightIndices[MAX_LIGHTS];
	int numDynamicLights;
	int dynamicLightIndices[MAX_LIGHTS];
};
#ifdef CANE_GL41
// Read by glob.vert as activeLightTex, one R32I texel per int.
static_assert(sizeof(ACTIVELIGHTS) == 2056, "ACTIVELIGHTS is 514 R32I texels");
static_assert(offsetof(ACTIVELIGHTS, staticLightIndices) == 4, "texels 1-256");
static_assert(offsetof(ACTIVELIGHTS, numDynamicLights) == 1028, "texel 257");
static_assert(offsetof(ACTIVELIGHTS, dynamicLightIndices) == 1032, "texels 258-513");
#endif

class LIGHT : public ALO
{
	public:
	LIGHTK lightk;
	TWPS twps;
	// Light color (HSV)
	glm::vec3 vecHighlight;
	// Light color (RGBA)
	glm::vec3 rgbaColor;
	LTFN ltfn;
	glm::vec3 agFallOff;
	glm::vec4 falloffScale;
	glm::vec4 falloffBias;
	glm::mat4 frustum;
	float gMidtone;
	float gShadow;
	float degHighlight;
	float degMidtone;
	float degShadow;
	int fDynamic;
	int fExcludeDynamicObjects;
	glm::vec3 vecDirectionOrig;
	glm::vec3 normalLocal;
	LM lmFallOffS;
	float degCone;
	float degHotSpot;
	glm::vec3 vecUpLocal;
	float rx;
	float ry;
	LM lmFallOffPenumbra;
	LM lmFallOffAbsX;
	LM lmFallOffAbsY;
	glm::vec4 avecFrustrum[6];
	GRFZON grfzonBeam;
	glm::mat4 matLookAt;
	DLE dleLight;
};

LIGHT* NewLight();
void InitSwLightDl(SW* psw);
void InitLight(LIGHT* plight);
int  GetLightSize();
void OnLightAdd(LIGHT* plight);
void OnLightRemove(LIGHT* plight);
void RebuildLightFrustrum(LIGHT* plight);
void UpdateLightXfWorldHierarchy(LIGHT* plight);
void UpdateLightBeamGrfzon(LIGHT* plight);
void CloneLight(LIGHT* plight, LIGHT* plightBase);
void AddLightToSw(LIGHT* plight);
void FitLinearFunction(float x0, float y0, float x1, float y1, float& pdu, float& pru);
void FitRecipFunction(float x0, float y0, float x1, float y1, float* pdu, float* pru);
void ConvertFallOff(LM* plm, float* pdu, float* pru);
void ConvertAngleStrength(float deg0, float g0, float deg1, float g1, float& pdu, float& pru);
void RebuildLight(LIGHT* plight);
void*GetLightKind(LIGHT* plight);
void SetLightKind(LIGHT* plight, LIGHTK lightk);
void*GetLightHighlightColor(LIGHT* plight);
void SetLightHighlightColor(LIGHT* plight, glm::vec3& pvecHighlight);
void*GetLightMidtoneStrength(LIGHT* plight);
void SetLightMidtoneStrength(LIGHT* plight, float gMidtone);
void*GetLightShadowStrength(LIGHT* plight);
void SetLightShadowStrength(LIGHT* plight, float gShadow);
void*GetLightHighlightAngle(LIGHT* plight);
void SetLightHighlightAngle(LIGHT* plight, float degHighlight);
void*GetLightMidtoneAngle(LIGHT* plight);
void SetLightMidtoneAngle(LIGHT* plight, float degMidtone);
void*GetLightShadowAngle(LIGHT* plight);
void SetLightShadowAngle(LIGHT* plight, float degShadow);
void*GetLightDirection(LIGHT* plight);
void SetLightDirection(LIGHT* plight, glm::vec3& pvecDirection);
void*GetLightDynamic(LIGHT* plight);
void SetLightDynamic(LIGHT* plight, int fDynamic);
void SetLightFExcludeDynamicObjects(LIGHT* plight, int fExcludeDynamicObjects);
void*GetLightFExcludeDynamicObjects(LIGHT* plight);
void*GetLightFallOff(LIGHT* plight);
void SetLightFallOff(LIGHT* plight, LM* plm);
void*GetLightConeAngle(LIGHT* plight);
void SetLightConeAngle(LIGHT* plight, float degCone);
void*GetLightHotSpotAngle(LIGHT* plight);
void SetLightHotSpotAngle(LIGHT* plight, float degHotSpot);
void*GetLightFrustrumUp(LIGHT* plight);
void SetLightFrustrumUp(LIGHT* plight, glm::vec3& pvecUpLocal);
void RemoveLightFromSw(LIGHT* plight);
void AllocateLightBlkList();
bool FindSwDynamicLights(glm::vec3* pposCenter, float sRadius);
void PrepareSwLights(SW* psw, CM* pcm);
void UpdateTvLightGpu(LIGHT* plight, int iTvLight);
#ifdef CANE_GL41
void BindSwLightTextures();
#endif
void DeallocateLightBlkList();
void DeleteLight(LIGHT* plight);
void DeallocateLightVector();

extern std::vector<LIGHT*> allSwLights;
extern std::vector<LIGHT*> allSwDynamicLights;
extern ACTIVELIGHTS activeLights;
extern GLuint g_lightSsbo;
extern GLuint g_activeLightsSsbo;
extern GLuint g_tvLightSsbo;
extern int numSwLights;
extern std::vector <LIGHTBLK> lightBlk;

extern glm::vec3 g_vecHighlight;
extern float g_degHighlight;
extern float g_gMidtone;
extern float g_degMidtone;
extern float g_gShadow;
extern float g_degShadow;
extern glm::vec3 s_vecDirectionDefault;
extern LM s_lmFallOffDefault;
extern glm::vec3 s_posDefault;
extern GRFDFL g_grfdfl;
