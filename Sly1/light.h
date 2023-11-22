#pragma once
#include "alo.h"

enum LIGHTK 
{
	LIGHTK_Nil = -1,
	LIGHTK_Direction = 0,
	LIGHTK_Position = 1,
	LIGHTK_Frustrum = 2,
	LIGHTK_Spot = 3,
	LIGHTK_Max = 4
};
enum TWPS
{
	TWPS_Shadow = 0,
	TWPS_ShadowMidtone = 1,
	TWPS_ShadowMidtoneSaturate = 2
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

void* NewLight();
void InitLight(LIGHT* plight);
void OnLightAdd(LIGHT* plight);
void AddLightToSw(LIGHT* plight);
void DeleteLight(LO* plo);