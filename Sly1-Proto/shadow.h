#pragma once
#include "lo.h"
#include "vec.h"

class SHADOW
{
	public:
        struct SHD* pshd;
        float sNearRadius;
        float sFarRadius;
        glm::vec3 posCast;
        glm::vec3 normalCast;
        glm::vec3 vecUp;
        float sNearCast;
        float sFarCast;
        glm::vec3 posEffect;
        float sRadiusEffect;
        glm::mat4 matWorldToUv;
        OID oidDysh;
        struct DYSH* pdysh;
        DLE dle;
        int cqwVifs;
        struct SUR* psur;
};

void InitSwShadowDl(SW* psw);
void InitShadow(SHADOW* pshadow);
void SetShadowShader(SHADOW *pshadow, OID oidShdShadow);
void SetShadowNearRadius(SHADOW* pshadow, float sNearRadius);
void SetShadowFarRadius(SHADOW* pshadow, float sFarRadius);
void SetShadowNearCast(SHADOW* pshadow, float sNearCast);
void SetShadowFarCast(SHADOW* pshadow, float sFarCast);
void SetShadowConeAngle(SHADOW* pshadow, float degConeAngle);
void SetShadowFrustrumUp(SHADOW* pshadow, glm::vec3* pvecUp);
void SetShadowCastPosition(SHADOW* pshadow, const glm::vec3& posCast);
void SetShadowCastNormal(SHADOW* pshadow, const glm::vec3& normalCast);
int  FShadowValid(SHADOW* pshadow, GRFGLOB grfglob);
// Checks if a given sphere (e.g. a model's bounding sphere) intersects with a shadow's effective area.
int  FShadowIntersectsSphere(SHADOW* pshadow, const glm::vec3& pos, float sRadius);
void RebuildShadowRegion(SHADOW* pshadow);

extern DL g_dlShadowPending;