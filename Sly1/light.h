#pragma once
#include "alo.h"

extern std::vector<LIGHT*> allSwLights;

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

class LIGHT : public ALO
{
	public:
		LIGHTK lightk;
		TWPS twps;
		glm::vec3 vecHighlight;
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
		glm::mat3 matLookAt;
		DLE dleLight;
};

void*NewLight();
void InitSwLightDl(SW* psw);
void InitLight(LIGHT* plight);
int  GetLightSize();
void OnLightAdd(LIGHT* plight);
void OnLightRemove(LIGHT* plight);
void UpdateLightXfWorldHierarchy(LIGHT* plight);
void CloneLight(LIGHT* plight, LIGHT* plightBase);
void AddLightToSw(LIGHT* plight);
void SetLightKind(LIGHT* plight, LIGHTK lightk);
void SetLightHighlightColor(LIGHT* plight, glm::vec3 &pvecHighlight);
void SetLightMidtoneStrength(LIGHT* plight, float gMidtone);
void SetLightShadowStrength(LIGHT* plight, float gShadow);
void SetLightHighlightAngle(LIGHT* plight, float degHighlight);
void SetLightMidtoneAngle(LIGHT* plight, float degMidtone);
void SetLightShadowAngle(LIGHT* plight, float degShadow);
void SetLightDirection(LIGHT* plight, glm::vec3 &pvecDirection);
void SetLightDynamic(LIGHT* plight, int fDynamic);
void SetLightFallOff(LIGHT* plight, LM* plm);
void SetLightConeAngle(LIGHT* plight, float degCone);
void SetLightHotSpotAngle(LIGHT* plight, float degHotSpot);
void SetLightFrustrumUp(LIGHT* plight, glm::vec3 &pvecUpLocal);
void RemoveLightFromSw(LIGHT* plight);
void DeleteLight(LO* plo);