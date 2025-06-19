#pragma once
#include "alo.h"

#define MAX_LIGHT 255

// Light type
enum LIGHTK 
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

struct LIGHTBLK
{
	int  lightk;
	glm::ivec3 pad;
	glm::vec4 pos;
	glm::vec4 dir;
	glm::vec4 color;
	glm::vec4 falloff;
	glm::vec4 ru;
	glm::vec4 du;
};


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
	glm::vec3 falloff0Frustum;
	glm::vec3 falloff1Frustum;
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
	int grfzonBeam;
	glm::mat4 matLookAt;
	DLE dleLight;
};

LIGHT*NewLight();
void InitSwLightDl(SW* psw);
void InitLight(LIGHT* plight);
int  GetLightSize();
void OnLightAdd(LIGHT* plight);
void OnLightRemove(LIGHT* plight);
void RebuildLightFrustrum(LIGHT* plight);
void UpdateLightXfWorldHierarchy(LIGHT* plight);
void CloneLight(LIGHT* plight, LIGHT* plightBase);
void AddLightToSw(LIGHT* plight);
void FitLinearFunction(float x0, float y0, float x1, float y1, float &pdu, float &pru);
void FitRecipFunction(float x0, float y0, float x1, float y1, float* pdu, float* pru);
void ConvertFallOff(LM* plm, float* pdu, float* pru);
void ConvertAngleStrength(float deg0, float g0, float deg1, float g1, float &pdu, float &pru);
void RebuildLight(LIGHT* plight);
void*GetLightKind(LIGHT* plight);
void SetLightKind(LIGHT* plight, LIGHTK lightk);
void*GetLightHighlightColor(LIGHT* plight);
void SetLightHighlightColor(LIGHT* plight, glm::vec3 &pvecHighlight);
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
void SetLightDirection(LIGHT* plight, glm::vec3 &pvecDirection);
void*GetLightDynamic(LIGHT* plight);
void SetLightDynamic(LIGHT* plight, int fDynamic);
void*GetLightFallOff(LIGHT* plight);
void SetLightFallOff(LIGHT* plight, LM* plm);
void*GetLightConeAngle(LIGHT* plight);
void SetLightConeAngle(LIGHT* plight, float degCone);
void*GetLightHotSpotAngle(LIGHT* plight);
void SetLightHotSpotAngle(LIGHT* plight, float degHotSpot);
void*GetLightFrustrumUp(LIGHT*plight);
void SetLightFrustrumUp(LIGHT* plight, glm::vec3 &pvecUpLocal);
void RemoveLightFromSw(LIGHT* plight);
void AllocateLightBlkList();
void PrepareSwLights(SW* psw, CM* pcm);
void DeallocateLightBlkList();
void DeleteLight(LIGHT *plight);
void DeallocateLightVector();

extern std::vector<LIGHT*> allSwLights;

extern GLuint g_lightUbo;
extern int numRl;
extern std::vector <LIGHTBLK> lightBlk;