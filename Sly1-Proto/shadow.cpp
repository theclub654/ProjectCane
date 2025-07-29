#include "shadow.h"

void InitSwShadowDl(SW* psw)
{
	InitDl(&psw->dlShadow, offsetof(SHADOW, dle));
}

void InitShadow(SHADOW* pshadow)
{
	pshadow->sNearRadius = -1.0f;
	pshadow->sFarRadius = -1.0f;
	pshadow->sNearCast = 100.0f;
	pshadow->sFarCast = 400.0f;
	pshadow->oidDysh = OID_Nil;

	// Initialize up vector to g_normalY
	pshadow->vecUp = g_normalY;

	// Compute normalCast = g_normalZ * -1.0f
	pshadow->normalCast.x = -g_normalZ.x;
	pshadow->normalCast.y = -g_normalZ.y;
	pshadow->normalCast.z = -g_normalZ.z;
	//pshadow->normalCast.gUnused = -g_normalZ.gUnused;
}

void SetShadowShader(SHADOW* pshadow, OID oidShdShadow)
{
	OID oid = OID_shd_stock_shadow;

	if (oidShdShadow != OID_Nil) {
		oid = oidShdShadow;
	}

	pshadow->pshd = PshdFindShader(oid);
}

void SetShadowNearRadius(SHADOW* pshadow, float sNearRadius)
{
	pshadow->sNearRadius = sNearRadius;
	if (pshadow->sFarRadius < 0.0) {
		pshadow->sFarRadius = sNearRadius;
	}
}

void SetShadowFarRadius(SHADOW* pshadow, float sFarRadius)
{
	pshadow->sFarRadius = sFarRadius;
	if (pshadow->sNearRadius < 0.0) {
		pshadow->sNearRadius = sFarRadius;
	}
}

void SetShadowNearCast(SHADOW* pshadow, float sNearCast)
{
	pshadow->sNearCast = sNearCast;
	RebuildShadowRegion(pshadow);
}

void SetShadowFarCast(SHADOW* pshadow, float sFarCast)
{
	pshadow->sFarCast = sFarCast;
	RebuildShadowRegion(pshadow);
}

void SetShadowConeAngle(SHADOW* pshadow, float degConeAngle)
{
	// Convert degrees to radians: 0.008726647 / 360
	float coneTangent = std::tanf(degConeAngle * 0.008726647f);

	pshadow->sNearRadius = coneTangent * pshadow->sNearCast;
	pshadow->sFarRadius = coneTangent * pshadow->sFarCast;
}

void SetShadowFrustrumUp(SHADOW* pshadow, glm::vec3* pvecUp)
{
	pshadow->vecUp = *pvecUp;
}

void SetShadowCastPosition(SHADOW* pshadow, const glm::vec3& posCast)
{
	if (!glm::all(glm::epsilonEqual(pshadow->posCast, posCast, 0.0001f))) {
		pshadow->posCast = posCast;
		RebuildShadowRegion(pshadow);
	}
}

void SetShadowCastNormal(SHADOW* pshadow, const glm::vec3& normalCast)
{
	// Avoid unnecessary updates if the normal hasn't changed significantly
	if (!glm::all(glm::epsilonEqual(pshadow->normalCast, normalCast, 0.0001f))) {
		pshadow->normalCast = normalCast;
		RebuildShadowRegion(pshadow);
	}
}

int FShadowValid(SHADOW* pshadow, GRFGLOB grfglob)
{
	SHD* pshd = pshadow->pshd;
	uint32_t shdZones = pshd->grfzon;

	// Shadow is valid if it matches global zones
	bool zoneMatch = (shdZones & 0x10000000) != 0 || (g_pcm->grfzon & shdZones) == g_pcm->grfzon;
	if (!zoneMatch) return 0;

	// Check type match based on grfglob
	if ((grfglob & 1u) && pshd->shdk == 0x02) return 1; // Dynamic
	if ((grfglob & 2u) && pshd->shdk == 0x03) return 1; // Static
}

int FShadowIntersectsSphere(SHADOW* pshadow, const glm::vec3& pos, float sRadius)
{
	uint32_t zon = pshadow->pshd->grfzon;
	bool zoneValid = (zon & 0x10000000) != 0 || (g_pcm->grfzon & zon) == g_pcm->grfzon;

	if (!zoneValid) return 0;

	// Convert posEffect (assumed to be glm::vec4 or VECTOR) to vec3 for distance
	glm::vec3 shadowPos = glm::vec3(pshadow->posEffect.x, pshadow->posEffect.y, pshadow->posEffect.z);

	float totalRadius = sRadius + pshadow->sRadiusEffect;
	float distSq = glm::distance2(pos, shadowPos);

	return distSq <= totalRadius * totalRadius;
}

void RebuildShadowRegion(SHADOW* pshadow)
{
	float nearRadius = pshadow->sNearRadius;
	float farRadius  = pshadow->sFarRadius;
	float nearCast   = pshadow->sNearCast;
	float farCast    = pshadow->sFarCast;

	float effectDistance = 0.0f;
	float effectRadius = 0.0f;

	if (!FFloatsNear(nearRadius, farRadius, 0.0001f)) {
		float dCast = farCast - nearCast;

		// Solve for the base offset of the shadow parabola
		float base = (nearCast * farRadius - farCast * nearRadius) / (nearRadius - farRadius);

		// Compute the midpoint position along the parabola
		float height = std::sqrt(dCast * dCast + farRadius * farRadius);
		float z1 = ((nearCast + base) * height) / dCast;
		float focus = (z1 * (z1 + 0.5f * height)) / (nearCast + base);

		// Compute final effect position and radius
		float z2 = (farCast + base) - focus;
		effectRadius = std::sqrt(z2 * z2 + farRadius * farRadius);
		effectDistance = focus - base;
	}
	else {
		// Fallback: cone case or degenerate radius
		float halfLength = std::abs(farCast - nearCast) * 0.5f;
		effectDistance = nearCast + halfLength;
		effectRadius = halfLength;
	}

	pshadow->sRadiusEffect = effectRadius;

	// Calculate posEffect = posCast + normalCast * effectDistance
	pshadow->posEffect.x = pshadow->posCast.x + pshadow->normalCast.x * effectDistance;
	pshadow->posEffect.y = pshadow->posCast.y + pshadow->normalCast.y * effectDistance;
	pshadow->posEffect.z = pshadow->posCast.z + pshadow->normalCast.z * effectDistance;
	//pshadow->posEffect.gUnused = pshadow->posCast.gUnused + pshadow->normalCast.gUnused * effectDistance;
}

DL g_dlShadowPending;