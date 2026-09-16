#include "ub.h"
#include "sm.h"
#include "chkpnt.h"
#include "boss.h"

UBG* NewUbg()
{
	return new UBG{};
}

void InitUbg(UBG* pubg)
{
	InitGomer(pubg);

	pubg->oidPatrolGoal = (OID)728;
	pubg->lmRadDetect.gMax = 6.2831855;
	pubg->dzAbandon = 30000.0;
	pubg->sAbandon = 3e+07;
	pubg->lmSDetect.gMax = 3e+06;
	pubg->lmSDetect.gMin = 3e+06;
	pubg->lmRadDetect.gMin = 0.0;
}

int GetUbgSize()
{
	return sizeof(UBG);
}

void CloneUbg(UBG* pubg, UBG* pubgBase)
{
	CloneGomer(pubg, pubgBase);

	pubg->psmWorld = pubgBase->psmWorld;
	pubg->psmaWorld = pubgBase->psmaWorld;
	pubg->oidPatrolGoal = pubgBase->oidPatrolGoal;
	pubg->cJump = pubgBase->cJump;
	pubg->pxfmMiddle = pubgBase->pxfmMiddle;

	for (int i = 0; i < 6; ++i)
		pubg->apsoPads[i] = pubgBase->apsoPads[i];

	pubg->psoLastDunk = pubgBase->psoLastDunk;

	for (int i = 0; i < 4; ++i)
		pubg->aichk[i] = pubgBase->aichk[i];

	pubg->nBossctr = pubgBase->nBossctr;
}

void PostUbgLoad(UBG* pubg)
{
	SnipAloObjects(pubg, 8, s_asnipUbg);
	SetSwGameplayHud(pubg->psw, pubg);

	if (pubg->psmWorld != nullptr)
		pubg->psmaUbg = (SMA*)PsmaApplySm(pubg->psmWorld, 0, (OID)727, 0);

	pubg->nBossctr = 4;

	for (int i = 0; i < 4; i++)
		pubg->aichk[i] = IchkAllocChkmgr(&g_chkmgr);

	PostGomerLoad(pubg);
}

SO* PsoPadUbgClosest(UBG* pubg, glm::vec3* ppos)
{
	SO* psoClosest = nullptr;
	float sDistClosest = FLT_MAX;

	for (int i = 0; i < 6; i++) 
	{
		SO* psoPad = pubg->apsoPads[i];

		if (psoPad == nullptr)
			continue;

		glm::vec3 dpos = psoPad->xf.posWorld - *ppos;
		float sDist = glm::dot(dpos, dpos);

		if (sDist < sDistClosest) {
			sDistClosest = sDist;
			psoClosest = psoPad;
		}
	}

	return psoClosest;
}

void OnUbgEnteringSgs(UBG* pubg, SGS sgsPrev, ASEG* pasegOverride)
{
	if (pubg == nullptr)
		return;

	switch (pubg->sgs)
	{
		case SGS_Patrol:
		{
			if (pubg->psmaUbg != nullptr)
			{
				SetSmaGoal(pubg->psmaUbg, (OID)pubg->oidPatrolGoal);

				SO* psoEnemy = PsoEnemyStepguard(pubg);
				SO* psoClosest = nullptr;

				if (psoEnemy != nullptr)
					psoClosest = PsoPadUbgClosest(pubg, &psoEnemy->xf.posWorld);

				for (int i = 0; i < 6; ++i)
				{
					SO* psoPad = pubg->apsoPads[i];

					if (psoPad == nullptr)
						continue;

					int oidPadGoal = 734;

					switch (pubg->oidPatrolGoal)
					{
						case 727:
						case 728:
						case 730:
						oidPadGoal = 734;
						break;

						case 729:
						case 731:
						case 732:
						case 733:
						oidPadGoal = psoPad == psoClosest ? 734 : 735;
						break;

						default:
						break;
					}

					SMA* psmaPad = PsmaFindAlo(psoPad, OID_Nil);

					if (psmaPad != nullptr)
						SetSmaGoal(psmaPad, static_cast<OID>(oidPadGoal));
				}
			}

			if (pubg->oidPatrolGoal == 730)
				pubg->cJump = 5;
			else
			{
				const glm::vec3 posCurrent(pubg->xf.pos.x, pubg->xf.pos.y, pubg->xf.pos.z);
				const glm::vec3 posOriginal(pubg->posOrig.x, pubg->posOrig.y, pubg->posOrig.z);
				const float distanceFromOrigin = glm::distance(posCurrent, posOriginal);

				pubg->cJump = distanceFromOrigin < 300.0f ? 8 : 5;
			}

			pubg->psoLastDunk = nullptr;
			break;
		}

		case SGS_Taunt:
		case SGS_Attack:
		pubg->cJump = 0;
		break;

		case SGS_Dying:
		{
			if (pubg->psmaUbg != nullptr)
				SetSmaGoal(static_cast<SMA*>(pubg->psmaUbg), static_cast<OID>(733));

			for (int i = 0; i < 6; ++i)
			{
				SO* psoPad = pubg->apsoPads[i];

				if (psoPad == nullptr)
					continue;

				SMA* psmaPad = PsmaFindAlo(reinterpret_cast<ALO*>(psoPad), OID_Nil);

				if (psmaPad != nullptr)
					SetSmaGoal(psmaPad, static_cast<OID>(734));
			}

			pubg->nBossctr = 0;

			DecrementBossHealth(&g_boss);
			SetWorldComplete(GAMEWORLD_Underwater);
			AutosaveCurrentGame(&g_saveData);
			break;
		}

		case SGS_Stun:
		{
			int phaseIndex = -1;

			switch (pubg->oidPatrolGoal)
			{
				case 728:
				pubg->oidPatrolGoal = (OID)729;
				phaseIndex = 0;
				break;

				case 729:
				pubg->oidPatrolGoal = (OID)730;
				phaseIndex = 1;
				break;

				case 730:
				pubg->oidPatrolGoal = (OID)731;
				phaseIndex = 2;
				break;

				case 731:
				pubg->oidPatrolGoal = (OID)732;
				phaseIndex = 3;
				break;

				default:
				break;
			}

			if (phaseIndex >= 0)
			{
				const int checkpoint = pubg->aichk[phaseIndex];

				if (!FGetChkmgrIchk(&g_chkmgr, checkpoint))
					OnDifficultyTriggerCheckpoint(&g_difficulty, nullptr);

				SetChkmgrIchk(&g_chkmgr, checkpoint);

				pubg->nBossctr = 3 - phaseIndex;
				DecrementBossHealth(&g_boss);
			}

			break;
		}

		default:
		break;
	}

	OnStepguardEnteringSgs(pubg, sgsPrev, pasegOverride);
}

SGS SgsNextUbgAI(UBG* pubg)
{
	if (pubg->cJump < 1)
		pubg->tTaunt = 3.4028235e+38;

	if (pubg->sgs == SGS_Taunt) {
		if (pubg->pasegaSgs == nullptr)
			return SGS_LostPlayer;

		return SGS_Taunt;
	}

	return SgsNextStepguardAI(pubg);
}

bool FDetectUbg(UBG* pubg)
{
	if (pubg->fAbandonExternal == 0)
		return pubg->cJump != 0;

	return false;
}

int FDetectUbgStepguard(UBG* pubg)
{
	return FDetectUbg(pubg) ? 1 : 0;
}

int FCanUbgAttack(UBG* pubg)
{
	if (((pubg->oidPatrolGoal == 731) && (pubg->cJump != 5)) && (pubg->cJump != 0)) {
		return 1;
	}
	return 0;
}

void DoUbgFreefallJump(UBG* pubg)
{
	if (pubg->oidPatrolGoal != 730) {
		pubg->cJump--;

		if (pubg->cJump < 1)
			pubg->cJump = 0;
	}

	DoStepguardFreefallJump(pubg);
}

void DoUbgFreefallLanding(UBG* pubg)
{
	if (pubg->oidPatrolGoal == 730 && pubg->sgs == SGS_Pursue && PxpFindSoGround(pubg, 0, 0) != nullptr) {
		SO* psoPad = PsoPadUbgClosest(pubg, &pubg->xf.posWorld);

		if (psoPad != pubg->psoLastDunk) {
			pubg->cJump--;

			if (pubg->cJump < 1)
				pubg->cJump = 0;

			if (pubg->cJump > 0) {
				SMA* psmPad = (SMA*)PsmaFindAlo((ALO*)psoPad, (OID)-1);

				if (psmPad != nullptr)
					SetSmaGoal(psmPad, (OID)736);

				pubg->psoLastDunk = psoPad;
			}
		}
	}

	DoStepguardFreefallLanding(pubg);
}

void UpdateUbgGoal(UBG* pubg, int fEnter)
{
	if (pubg->sgs == SGS_ReturnToPatrol) {
		glm::vec3 posGoal;

		if (!pubg->fAbandonExternal) {
			GetXfmPos(pubg->pxfmMiddle, &posGoal);

			SetStepguardGoal(pubg, &posGoal);
		}
		else {
			SetStepguardGoal(pubg, &pubg->posOrig);
		}
	}
	else if (pubg->sgs == SGS_LostPlayer) {
		STEP* pstepEnemy = (STEP*)pubg->pvtstepguard->pfnPsoEnemyStepguard(pubg);

		if (pstepEnemy != nullptr)
			SetStepguardGoal(pubg, &pstepEnemy->xf.posWorld);
	}
	else {
		UpdateStepguardGoal(pubg, fEnter);
	}
}

void UpdateUbgSgs(UBG* pubg)
{
	if (pubg->sgs == SGS_LostPlayer)
		LookStepguardAtGoal(pubg);
	else
		UpdateStepguardSgs(pubg);
}

void UpdateUbgBlot()
{
	if (FCanShowGameplayBlot())
		ShowBlot(&g_boss);
	else
		HideBlot(&g_boss);
}

int FIgnoreUbgIntersection(UBG* pubg, SO* psoOther)
{
	for (int i = 0; i < 6; i++) 
	{
		if (psoOther->paloRoot == pubg->apsoPads[i]) 
			return 0;
	}

	return FIgnoreStepguardIntersection(pubg, psoOther);
}

void DeleteUbg(UBG* pubg)
{
	delete pubg;
}

UBP* NewUbp()
{
	return new UBP{};
}

int GetUbpSize()
{
	return sizeof(UBP);
}

void CloneUbp(UBP* pubp, UBP* pubpBase)
{
	CloneSo(pubp, pubpBase);

	pubp->pubv = pubpBase->pubv;
	pubp->ubps = pubpBase->ubps;
	pubp->tUbps = pubpBase->tUbps;
	pubp->pactval = pubpBase->pactval;
	pubp->pbomb = pubpBase->pbomb;
}

void PostUbpLoad(UBP* pubp)
{
	PostAloLoad(pubp);
}

void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro)
{
	RenderUbpAll(pubp, pcm, pro);
}

void DeleteUbp(UBP *pubp)
{
	delete pubp;
}

UBV* NewUbv()
{
	return new UBV{};
}

int GetUbvSize()
{
	return sizeof(UBV);
}

void CloneUbv(UBV* pubv, UBV* pubvBase)
{
	CloneSo(pubv, pubvBase);

	pubv->ubvs = pubvBase->ubvs;
	pubv->tUbvs = pubvBase->tUbvs;

	for (int i = 0; i < 12; ++i)
		pubv->apubp[i] = pubvBase->apubp[i];

	for (int i = 0; i < 2; ++i)
		pubv->apubpBase[i] = pubvBase->apubpBase[i];

	pubv->dposPadRow = pubvBase->dposPadRow;
	pubv->dposPadCol = pubvBase->dposPadCol;
	pubv->dposBase = pubvBase->dposBase;

	pubv->cswap = pubvBase->cswap;
	pubv->iswap = pubvBase->iswap;

	for (int i = 0; i < 24; ++i)
		pubv->aswap[i] = pubvBase->aswap[i];

	for (int i = 0; i < 4; ++i)
		pubv->mprowcolSafe[i] = pubvBase->mprowcolSafe[i];
}

void PostUbvLoad(UBV* pubv)
{
	PostAloLoad(pubv);
}

void DeleteUbv(UBV *pubv)
{
	delete pubv;
}

SNIP s_asnipUbg[8] =
{
	3, (OID)0x2D5, offsetof(UBG, psmWorld),
	3, (OID)0x2E7, offsetof(UBG, pxfmMiddle),
	7, (OID)0x2E1, offsetof(UBG, apsoPads[0]),
	7, (OID)0x2E2, offsetof(UBG, apsoPads[1]),
	7, (OID)0x2E3, offsetof(UBG, apsoPads[2]),
	7, (OID)0x2E4, offsetof(UBG, apsoPads[3]),
	7, (OID)0x2E5, offsetof(UBG, apsoPads[4]),
	7, (OID)0x2E6, offsetof(UBG, apsoPads[5])
};
