#include "gomer.h"
#include "waypoint.h"
#include "jt.h"
#include "chkpnt.h"
#include <cstdio>

GOMER* NewGomer()
{
	return new GOMER{};
}

void InitGomer(GOMER* pgomer)
{
	InitStepGuard(pgomer);

	pgomer->lmSDetect.gMin = 1000.0f;
	pgomer->lmSDetect.gMax = 1500.0f;

	pgomer->radAttackMax = 1.0471976f;

	pgomer->lmRadDetect.gMin = 0.7853982f;
	pgomer->lmRadDetect.gMax = 3.1415927f;

	pgomer->sAbandon = 2000.0f;
	pgomer->oidAbandon = OID_Nil;

	pgomer->lmAttackS.gMin = 0.0f;
	pgomer->lmAttackS.gMax = 300.0f;

	pgomer->oidDetect = OID_Nil;
	pgomer->dzAbandon = 300.0f;
}

void* GetGomerSAbandon(GOMER* pgomer)
{
	return &pgomer->sAbandon;
}

void SetGomerSAbandon(GOMER* pgomer, float sAbandon)
{
	pgomer->sAbandon = sAbandon;
}

void* GetGomerDzAbandon(GOMER* pgomer)
{
	return &pgomer->dzAbandon;
}

void SetGomerDzAbandon(GOMER* pgomer, float dzAbandon)
{
	pgomer->dzAbandon = dzAbandon;
}

void* GetGomerOidAbandon(GOMER* pgomer)
{
	return &pgomer->oidAbandon;
}

void SetGomerOidAbandon(GOMER* pgomer, OID oidAbandon)
{
	pgomer->oidAbandon = oidAbandon;
}

void* GetGomerLmSDetect(GOMER* pgomer)
{
	return &pgomer->lmSDetect;
}

void SetGomerLmSDetect(GOMER* pgomer, LM lmSDetect)
{
	pgomer->lmSDetect = lmSDetect;
}

void* GetGomerLmRadDetect(GOMER* pgomer)
{
	return &pgomer->lmRadDetect;
}

void SetGomerLmRadDetect(GOMER* pgomer, LM lmRadDetect)
{
	pgomer->lmRadDetect = lmRadDetect;
}

void* GetGomerOidDetect(GOMER* pgomer)
{
	return &pgomer->oidDetect;
}

void SetGomerOidDetect(GOMER* pgomer, OID oidDetect)
{
	pgomer->oidDetect = oidDetect;
}

void* GetGomerPvolAbandon(GOMER* pgomer)
{
    return &pgomer->pvolAbandon;
}

void SetGomerPvolAbandon(GOMER* pgomer, VOL* pvolAbandon)
{
    pgomer->pvolAbandon = pvolAbandon;
}

int GetGomerSize()
{
	return sizeof(GOMER);
}

void CloneGomer(GOMER* pgomer, GOMER* pgomerBase)
{
    CloneStepguard(pgomer, pgomerBase);

    pgomer->sAbandon = pgomerBase->sAbandon;
    pgomer->dzAbandon = pgomerBase->dzAbandon;
    pgomer->oidAbandon = pgomerBase->oidAbandon;
    pgomer->pvolAbandon = pgomerBase->pvolAbandon;
    pgomer->lmSDetect = pgomerBase->lmSDetect;
    pgomer->lmRadDetect = pgomerBase->lmRadDetect;
    pgomer->oidDetect = pgomerBase->oidDetect;
    pgomer->pvolDetect = pgomerBase->pvolDetect;
    pgomer->fDetectLatch = pgomerBase->fDetectLatch;
    pgomer->fDetectLatchExternal = pgomerBase->fDetectLatchExternal;
    pgomer->fAbandonExternal = pgomerBase->fAbandonExternal;

}

void PostGomerLoad(GOMER* pgomer)
{
	PostStepguardLoad(pgomer);

	if (pgomer->oidAbandon != OID_Nil)
		pgomer->pvolAbandon = (VOL*)PloFindSwNearest(pgomer->psw, (OID)pgomer->oidAbandon, pgomer);

	if (pgomer->oidDetect != OID_Nil)
		pgomer->pvolDetect = (VOL*)PloFindSwNearest(pgomer->psw, (OID)pgomer->oidDetect, pgomer);

	if (pgomer->pasegPatrol != nullptr)
	{
		int cpwaypoint = 0;
		std::vector <WAYPOINT*> waypoints;

		GetAsegWaypoints(pgomer->pasegPatrol, pgomer, &cpwaypoint, waypoints);

		for (int i = 0; i < cpwaypoint; i++)
		{
			WAYPOINT* pwaypoint = waypoints[i];

			if (pwaypoint->pwpsg == nullptr)
			{
				pwaypoint->pwpsg = PwpsgNew();
				AddWpsgWaypoint(pwaypoint->pwpsg.get(), pwaypoint);
			}
		}
	}
}

void RenderGomerSelf(GOMER* pgomer, CM* pcm, RO* pro)
{
	RenderStepSelf(pgomer, pcm, pro);
}

int FDetectGomer(GOMER* pgomer)
{
    if (FSwHandsOff(pgomer->psw) != 0)
        return 0;

    if (pgomer->sgs == SGS_Stun)
        return 0;

    if (pgomer->sgs == SGS_Dying)
        return 0;

    STEP* pstepEnemy = (STEP*)pgomer->pvtstepguard->pfnPsoEnemyStepguard(pgomer);
    BASIC* pbasicEnemy = (BASIC*)pstepEnemy;
    bool fAbandon = true;

    if (!pgomer->fAbandonExternal && pstepEnemy != nullptr) {
        bool fSkipAbandonCheck = false;

        if (FIsBasicDerivedFrom(pbasicEnemy, CID_JT)) {
            int jths = ((JT*)pbasicEnemy)->pvtpo->pfnJthsCurrentPo((JT*)pbasicEnemy);

            if (jths == 2)
                fSkipAbandonCheck = true;
        }

        if (!fSkipAbandonCheck) {
            glm::vec3 posEnemy = pstepEnemy->xf.posWorld;

            if (pgomer->pvolAbandon == nullptr) {
                float dzEnemy = fabsf(posEnemy.z - pgomer->posOrig.z);

                if (dzEnemy <= pgomer->dzAbandon) {
                    glm::vec3 dpos = posEnemy - pgomer->posOrig;
                    float sEnemy = glm::length(dpos);

                    if (sEnemy <= pgomer->sAbandon)
                        fAbandon = false;
                }
            }
            else {
                fAbandon = !FCheckVolPoint(pgomer->pvolAbandon, &posEnemy);
            }
        }
    }

    if (fAbandon) {
        pgomer->fDetectLatch = 0;
        return 0;
    }

    if (pgomer->fDetectLatch || pgomer->fDetectLatchExternal)
        return 1;

    glm::vec3 posEnemy = pstepEnemy->xf.posWorld;

    if (pgomer->pvolDetect == nullptr || !FCheckVolPoint(pgomer->pvolDetect, &posEnemy)) {
        glm::vec3 dpos = pgomer->xf.posWorld - posEnemy;
        float sEnemy = glm::length(dpos);
        float sDetectMax = pgomer->lmSDetect.gMax;

        if (sDetectMax <= sEnemy)
            return 0;

        float sDetectMin = pgomer->lmSDetect.gMin;
        float radDetect = 0.0f;

        if (sEnemy < sDetectMin) {
            radDetect = pgomer->lmRadDetect.gMax;
        }
        else {
            float t = (sEnemy - sDetectMin) / (sDetectMax - sDetectMin);
            radDetect = t * pgomer->lmRadDetect.gMin + (1.0f - t) * pgomer->lmRadDetect.gMax;
        }

        float dradEnemy = DradEnemyStepguard(pgomer);

        if (radDetect < fabsf(dradEnemy))
            return 0;
    }

    if (!FCheckStepguardEnemyHidden(pgomer)) {
        pgomer->fDetectLatch = 1;
        return 1;
    }

    return 0;
}
void DeleteGomer(GOMER* pgomer)
{
	delete pgomer;
}
