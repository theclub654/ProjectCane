#include "puffer.h"
#include "jt.h"
#include "path.h"
#include "rog.h"
#include "actla.h"
#include "sm.h"
#include "water.h"
#include "timer.h"
#include "puffcharger.h"
#include "crusher.h"

PUFFER* NewPuffer()
{
	return new PUFFER{};
}

void InitPuffer(PUFFER* ppuffer)
{
	InitPo(ppuffer);

	ppuffer->tLastGround = -10.0f;
	ppuffer->normalGround = g_normalZ;
	ppuffer->svPuffMax = SV_PufferGroundMax;
	ppuffer->tGameMax = 120.0f;
	ppuffer->sdvPuffMax = SDV_PufferGroundMax;

	InitDl(&ppuffer->dlRost0, offsetof(ROST, dleRob));
	InitDl(&ppuffer->dlRost1, offsetof(ROST, dleRob));

	g_ppuffer = ppuffer;
}

void SetPuffertGameMax(PUFFER* ppuffer, float tGameMax)
{
	ppuffer->tGameMax = tGameMax;
}

void* GetPufferPsmaShoot(PUFFER* ppuffer)
{
	return &ppuffer->psmaShoot;
}

void* GetPufferPsma(PUFFER* ppuffer)
{
	return &ppuffer->psma;
}

void LoadPufferFromBrx(PUFFER* ppuffer, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(ppuffer, pbis);
	SnipAloObjects(ppuffer, 1, s_asnipLoadPuffer);
}

void ClonePuffer(PUFFER* ppuffer, PUFFER* ppufferBase)
{
	ClonePo(ppuffer, ppufferBase);

	// Shallow copy fields
	ppuffer->normalGround = ppufferBase->normalGround;
	ppuffer->pvolCollect = ppufferBase->pvolCollect;
	ppuffer->pwater = ppufferBase->pwater;
	ppuffer->pvolFire = ppufferBase->pvolFire;

	// Copy the pointers for appntFloor
	for (int i = 0; i < 4; ++i)
	{
		ppuffer->appntFloor[i] = ppufferBase->appntFloor[i];
	}

	// Shallow copy other ALO, ACTLA, EMITTER pointers
	ppuffer->paloFireGun = ppufferBase->paloFireGun;
	ppuffer->paloFireGunTarget = ppufferBase->paloFireGunTarget;
	ppuffer->pactlaFireGun = ppufferBase->pactlaFireGun;
	ppuffer->pemitterFire = ppufferBase->pemitterFire;
	ppuffer->pemitterSpray = ppufferBase->pemitterSpray;
	ppuffer->ppntFireNatural = ppufferBase->ppntFireNatural;
	ppuffer->ppufft = ppufferBase->ppufft;
	ppuffer->paloJt = ppufferBase->paloJt;
	ppuffer->paloGut = ppufferBase->paloGut;
	ppuffer->pactadjGut = ppufferBase->pactadjGut;
	ppuffer->paloHead = ppufferBase->paloHead;
	ppuffer->pactadjHead = ppufferBase->pactadjHead;

	// Copy scalar values
	ppuffer->npuffcEaten = ppufferBase->npuffcEaten;
	ppuffer->npufftLit = ppufferBase->npufftLit;
	ppuffer->npufftMax = ppufferBase->npufftMax;
	ppuffer->tGameMax = ppufferBase->tGameMax;
}

void PostPufferLoad(PUFFER* ppuffer)
{
	LO* aploRost[32];

	PostAloLoad(ppuffer);
	SnipAloObjects(ppuffer, 13, s_asnipPostPufferLoad);

	ppuffer->radTarget = atan2f(ppuffer->xf.mat[0].y, ppuffer->xf.mat[0].x);

	ppuffer->psmaShoot = PsmaApplySm(ppuffer->psmShoot, ppuffer, OID_Nil, 1);

	ppuffer->pactlaFireGun = PactlaNew(ppuffer->psw, ppuffer->paloFireGun, &g_vtactla);
	ppuffer->pactlaFireGun->ackRot = ACK_Velocity;

	InsertAloAct(ppuffer->paloFireGun, ppuffer->pactlaFireGun.get());

	ppuffer->pactlaFireGun->fUseTarget = 1;
	ppuffer->pactlaFireGun->paloTarget = ppuffer->paloFireGunTarget;
	ppuffer->pactlaFireGun->posTargetLocal = glm::vec3(0.0);

	int cpuffc = CploFindSwObjectsByClass(ppuffer->psw, 0x305, CID_PUFFC, nullptr, 0, nullptr);
	ppuffer->appuffc.resize(cpuffc);
	cpuffc = CploFindSwObjectsByClass(ppuffer->psw, 0x105, CID_PUFFC, nullptr, cpuffc, (LO**)ppuffer->appuffc.data());
	ppuffer->appuffc.resize(cpuffc);

	int crost = CploFindSwObjectsByClass(ppuffer->psw, 5, CID_ROST, nullptr, 32, aploRost);

	for (int i = 0; i < crost; i++)
		AppendDlEntry(&ppuffer->dlRost1, aploRost[i]);

	ppuffer->psma = PsmaApplySm(ppuffer->psm, nullptr, OID_Nil, 0);

	SubscribeSmaStruct(ppuffer->psma, ppuffer);
}

void PresetPufferAccel(PUFFER* ppuffer, float dt)
{
	glm::mat3 matDesired;
	glm::mat3 dmatNormal;
	glm::mat3 dmatUpright;
	glm::mat3 matUpright;
	glm::mat3 matSpin;
	glm::vec3 dv;

	PresetSoAccel(ppuffer, dt);

	LoadRotateMatrixRad(ppuffer->radTarget, &g_normalZ, &matDesired);
	BuildRotateVectorsMatrix(&g_normalZ, &ppuffer->normalGround, &dmatNormal);

	matDesired = dmatNormal * matDesired;
	dmatUpright = matDesired;
	matUpright = matDesired;

	AccelSoTowardMatSmooth(ppuffer, dt, &matDesired, &s_smpPuffer);

	TiltMatUpright(&ppuffer->xf.mat, &dmatUpright, &matUpright);

	LoadRotateMatrixRad(ppuffer->radTarget, &g_normalZ, &matSpin);
	// Retail MatMulMatTransMat(dmatUpright, matSpin) evaluates
	// transpose(dmatUpright) * matSpin.
	matSpin = glm::transpose(dmatUpright) * matSpin;

	float drad = atan2f(matUpright[0].y, matUpright[0].x);
	drad = RadNormalize(drad - ppuffer->radTarget);

	float uTurn = s_clqDradToUPuff.g0 + fabsf(drad) * (s_clqDradToUPuff.g1 + fabsf(drad) * s_clqDradToUPuff.g2);
	uTurn = glm::clamp(uTurn, 0.0f, 1.0f);

	float uDtGround = (g_clock.t - ppuffer->tLastGround) / DT_PufferPuffAfterGround;

	float uGround = s_clqUDtGroundToUPuff.g0 + uDtGround * (s_clqUDtGroundToUPuff.g1 + uDtGround * s_clqUDtGroundToUPuff.g2);
	uGround = glm::clamp(uGround, 0.0f, 1.0f);

	dv = matSpin[0] * (uTurn * uGround * ppuffer->sdvPuff);
	AddSoAcceleration(ppuffer, &dv);

	glm::vec3 v = ppuffer->xf.v;
	v.z = 0.0f;

	float speed = glm::length(v);
	float drag = (-ppuffer->sdvPuffMax / (ppuffer->svPuffMax * ppuffer->svPuffMax)) * uGround * speed;

	dv = v * drag;
	AddSoAcceleration(ppuffer, &dv);
}

int FFilterPuffer(PUFFER* ppuffer, SO* psoOther)
{
	if (psoOther->fNoXpsSelf && !FIsBasicDerivedFrom(psoOther, CID_WATER))
		return 0;

	return psoOther->paloRoot != (SO*)ppuffer;
}

void UpdatePuffer(PUFFER* ppuffer, float dt)
{
	UpdatePo(ppuffer, dt);

	XP* pxpGround = PxpFindSoGround(ppuffer, nullptr, nullptr);

	if (pxpGround != nullptr || ppuffer->pwater != nullptr) {
		ppuffer->tLastGround = g_clock.t;

		if (pxpGround != nullptr) {
			ppuffer->svPuffMax = SV_PufferGroundMax;
			ppuffer->sdvPuffMax = SDV_PufferGroundMax;
		}
		else {
			ppuffer->svPuffMax  = SV_PufferWaterMax;
			ppuffer->sdvPuffMax = SDV_PufferWaterMax;
		}
	}

	glm::vec3 normalSurface = g_vecZero;

	for (int i = 0; i < 4; i++) 
	{
		glm::vec3 posPnt;
		glm::vec3 posPntMin;
		glm::vec3 posPntMax;
		glm::vec3 normalCur;
		glm::mat3 mat;
		LSG lsg;
		std::vector<SO*> apso;

		GetPntPos(ppuffer->appntFloor[i], &posPnt);

		posPntMax = posPnt;
		posPntMin = posPnt;

		posPntMax.z += DZ_PointCheckMax;
		posPntMin.z += DZ_PointCheckMin;

		IntersectSwBoundingBox(ppuffer->psw, nullptr, &posPntMax, &posPntMin, (PFNFILTER)FFilterPuffer, ppuffer, apso);

		SO* psoHit = PsoHitTestLineObjects(1, &posPntMax, &posPntMin, apso, &lsg);

		if (psoHit == nullptr) {
			normalCur = ppuffer->normalGround;
		}
		else 
		{
			normalCur = lsg.anormal[0];

			if (FIsBasicDerivedFrom(psoHit, CID_WATER) && psoHit->globset.pwrbgFirst != nullptr) 
			{
				glm::vec3 axis0;
				glm::vec3 axis1;
				glm::vec3 axis2 = normalCur;

				GetNormalVectors(axis2, axis0, axis1, g_normalX, g_normalZ);

				glm::mat3 mat;
				mat[0] = axis0;
				mat[1] = axis1;
				mat[2] = axis2;

				WarpWrTransform(psoHit->globset.pwrbgFirst->pwr, 50.0f, lsg.apos, &mat, nullptr, &mat, nullptr);

				normalCur = glm::cross(mat[0], mat[1]);

				if (glm::length(normalCur) < 0.0001f)
					normalCur = g_normalZ;
				else
					normalCur = glm::normalize(normalCur);
			}

			if (normalCur.z < 0.7f)
				normalCur = ppuffer->normalGround;
		}

		normalSurface += normalCur * 0.25f;
	}

	ppuffer->normalGround = glm::normalize(normalSurface);

	ResolveAlo(ppuffer);
}

PUFFT* PpufftChoosePuffer(PUFFER* ppuffer)
{
	PUFFT* appufft[32];
	PUFFT* ppufftBest = nullptr;

	glm::mat3 matProj;
	glm::vec3 posProj;
	glm::vec3 posNatural;
	glm::vec3 posNaturalProj;
	glm::vec3 dposAdjust;
	glm::vec3 posPufftParentProj;
	glm::mat3 matPufftParentProj;
	glm::vec3 posFire;
	glm::vec3 posFireProj;

	float sBest = S_PufftChooseMax;

	LoadRotateMatrixRad(ppuffer->radTarget, &g_normalZ, &matProj);

	posProj = ppuffer->xf.pos + ppuffer->xf.v * DT_PufftChooseProj;

	GetPntPos(ppuffer->ppntFireNatural, &posNatural);

	posNaturalProj = posProj + matProj * ppuffer->ppntFireNatural->posLocal;
	dposAdjust = posNaturalProj - posNatural;

	int cpufft = CploFindSwObjectsByClass(ppuffer->psw, 5, CID_PUFFT, nullptr, 32, (LO**)appufft);

	for (int i = 0; i < cpufft; i++) {
		PUFFT* ppufft = appufft[i];

		if (ppufft->fLit)
			continue;

		PredictAloTransform(ppufft->ppntFire->paloParent, nullptr, DT_PufftChooseProj, &posPufftParentProj, &matPufftParentProj, nullptr, nullptr);

		GetPntPos(ppufft->ppntFire, &posFire);
		posFire += dposAdjust;

		posFireProj = posPufftParentProj + matPufftParentProj * ppufft->ppntFire->posLocal;

		float u;
		float s;

		FindClosestPointOnLineSegment(&posNaturalProj, &posFire, &posFireProj, &u, &s);

		if (s >= sBest)
			continue;

		glm::vec3 dpos = posFire - ppuffer->xf.posWorld;

		float rad = atan2f(dpos.y, dpos.x);
		float drad = RadNormalize(rad - ppuffer->radTarget);

		if (fabsf(drad) >= DRAD_PufftChooseMax)
			continue;

		sBest = s;
		ppufftBest = ppufft;
	}

	return ppufftBest;
}

void UpdatePufferDeathDifficulty(PUFFER* ppuffer)
{
	float u = (float)ppuffer->cpufftLift / (float)ppuffer->cpufftMax;
	u = glm::clamp(u, 0.0f, 1.0f);

	OnDifficultyPlayerDeath(&g_difficulty ,1.0f - u * 0.8f);
}

void HandlePufferTimerExpiration(PUFFER* ppuffer, int fActive)
{
	if (!fActive || g_ppuffer == nullptr || g_ppuffer->psma == nullptr)
		return;

	int oidCur;
	int oidGoal;

	GetSmaCur(g_ppuffer->psma, (OID*)&oidCur);
	GetSmaGoal(g_ppuffer->psma, (OID*)&oidGoal);

	if (oidCur == 1012 || oidGoal == 1012)
		return;

	SetSmaGoal(g_ppuffer->psma, (OID)1013);
	UpdatePufferDeathDifficulty(g_ppuffer);
}

void OnPufferActive(PUFFER* ppuffer, int fActive)
{
	OnPoActive(ppuffer, fActive, nullptr);
}

void UpdatePufferActive(PUFFER* ppuffer, JOY* pjoy, float dt)
{
	if (pjoy->uDeflect > 0.0f)
		AdjustCmJoy(g_pcm, pjoy, JOYID_Left, &ppuffer->radTarget);

	ppuffer->sdvPuff = pjoy->uDeflect * ppuffer->sdvPuffMax;

	if (pjoy->IsPressed(BTN_CROSS) || pjoy->IsPressed(BTN_SQUARE))
	{
		OID oidShootCurrent;
		OID oidShootGoal;

		GetSmaCur(ppuffer->psmaShoot, &oidShootCurrent);
		GetSmaGoal(ppuffer->psmaShoot, &oidShootGoal);

		if (oidShootCurrent == 987 && oidShootGoal == 987 && ppuffer->npuffcEaten > 0)
		{
			ppuffer->ppufft = nullptr;
			ppuffer->pemitterFire->pemitb->emitp.emitblip.pexploSplineTarget = ppuffer->pexploSplineTarget;
			SetSmaGoal(ppuffer->psmaShoot, (OID)988);
			--ppuffer->npuffcEaten;
			ppuffer->tPufftFired = g_clock.t;
		}
	}

	PUFFT* ppufft = ppuffer->ppufft;

	if (ppufft != nullptr && ppuffer->tPufftFired + DT_PufftIgniteDelay < g_clock.t)
	{
		ppufft->fLit = true;
		ppufft->pemitterFire->pvtlo->pfnAddLo(ppufft->pemitterFire);
		ppuffer->pemitterFire->pvtlo->pfnRemoveLo(ppuffer->pemitterFire);

		++ppuffer->cpufftLift;

		if (ppuffer->cpufftLift >= ppuffer->cpufftMax)
		{
			if (ppuffer->psma != nullptr)
			{
				OID oidCurrent;
				OID oidGoal;

				GetSmaCur(ppuffer->psma, &oidCurrent);
				GetSmaGoal(ppuffer->psma, &oidGoal);

				if (oidCurrent != 1013 && oidGoal != 1013)
					SetSmaGoal(ppuffer->psma, (OID)1012);

				StopTimer(&g_timer);
			}
		}

		ppuffer->ppufft = nullptr;
	}

	if (!FIsLoInWorld(ppuffer->pemitterFire))
	{
		ppuffer->pactlaFireGun->paloTarget = ppuffer->paloFireGunTarget;
		ppuffer->pactlaFireGun->posTargetLocal = g_vecZero;
	}
	else if (ppuffer->ppufft == nullptr)
	{
		ppuffer->ppufft = PpufftChoosePuffer(ppuffer);

		if (ppuffer->ppufft != nullptr)
		{
			PUFFT* ppufftTarget = ppuffer->ppufft;
			PNT* ppntFire = ppufftTarget->ppntFire;
			glm::vec3 posTarget;

			GetPntPos(ppntFire, &posTarget);

			const glm::vec3 dposTarget = posTarget - ppuffer->xf.posWorld;

			ppuffer->radTarget = atan2f(dposTarget.y, dposTarget.x);
			ppuffer->pactlaFireGun->paloTarget = ppntFire->paloParent;
			ppuffer->pactlaFireGun->posTargetLocal = ppntFire->posLocal;
			ppuffer->pemitterFire->pemitb->emitp.emitblip.pexploSplineTarget = ppufftTarget->pexplo;
		}
	}

	OID oidCurrent;
	GetSmaCur(ppuffer->psma, &oidCurrent);

	if (oidCurrent == 1011)
		UpdatePufferPuffcEat(ppuffer);
}

void UpdatePufferBlot(PUFFER* ppuffer)
{
	OID oidState;

	GetSmaCur(ppuffer->psma, &oidState);

	const bool fShowBlots = oidState == static_cast<OID>(1011) && FCanShowGameplayBlot();

	if (fShowBlots)
	{
		ShowBlot(&g_puffchargectr);
		ShowBlot(&g_crusherctr);
		ShowBlot(&g_timer);
	}
	else
	{
		HideBlot(&g_puffchargectr);
		HideBlot(&g_crusherctr);
		HideBlot(&g_timer);
	}
}

void UpdatePufferPuffcEat(PUFFER* ppuffer)
{
	if (ppuffer->tNextPuffcEat > g_clock.t)
		return;

	PUFFC* ppuffc = nullptr;

	for (int i = 0; i < ppuffer->appuffc.size(); i++) 
	{
		if (!FIsLoInWorld(ppuffer->appuffc[i])) 
		{
			ppuffc = ppuffer->appuffc[i];
			break;
		}
	}

	if (ppuffc == nullptr) {
		ppuffer->tNextPuffcEat = D_T_Max;
		return;
	}

	int crost = CPvDl(&ppuffer->dlRost1);

	if (crost <= 0) {
		ppuffer->tNextPuffcEat = D_T_Max;
		return;
	}

	int irost = NRandInRange(0, crost - 1);
	ROST* prost = (ROST*)ppuffer->dlRost1.ploFirst;

	for (int i = 0; i < irost && prost != nullptr; i++)
		prost = (ROST*)prost->dleRob.ploNext;

	if (prost == nullptr) {
		ppuffer->tNextPuffcEat = D_T_Max;
		return;
	}

	RemoveDlEntry(&ppuffer->dlRost1, prost);
	AppendDlEntry(&ppuffer->dlRost0, prost);

	ppuffc->pvtlo->pfnAddLo(ppuffc);
	ppuffc->pvtso->pfnTranslateSoToPos(ppuffc, &prost->xf.posWorld);
	ppuffc->pvtso->pfnRotateSoToMat(ppuffc, prost->xf.matWorld);
	ppuffc->pvtso->pfnSetSoVelocityVec(ppuffc, &g_vecZero);
	ppuffc->pvtso->pfnSetSoAngularVelocityVec(ppuffc, &g_vecZero);

	SetStepguardSgs(ppuffc, SGS_Stun, nullptr);

	ppuffc->prost = prost;

	SetRostRosts(prost, ROSTS_Open);

	ppuffer->tNextPuffcEat = g_clock.t + DT_PuffcEaten;
}

void AdjustPufferNewXp(PUFFER* ppuffer, XP* pxp, int ixpd)
{
	pxp->elas = ELAS_Puffer;
	pxp->mu   = MU_Puffer;
}

void AddPufferWaterAcceleration(PUFFER* ppuffer, WATER* pwater, float dt)
{
	glm::vec3 posSurface;
	glm::vec3 normalSurface;

	float uSubmerged = UGetWaterSubmerged(pwater, ppuffer, &posSurface, &normalSurface);

	if (pwater->globset.pwrbgFirst != nullptr) {
		glm::vec3 axis0;
		glm::vec3 axis1;
		glm::vec3 axis2 = normalSurface;
		glm::mat3 mat;

		GetNormalVectors(axis2, axis0, axis1, g_normalX, g_normalZ);

		mat[0] = axis0;
		mat[1] = axis1;
		mat[2] = axis2;

		WarpWrTransform(pwater->globset.pwrbgFirst->pwr, 50.0f, &posSurface, &mat, nullptr, &mat, nullptr);

		normalSurface = glm::cross(mat[0], mat[1]);

		if (glm::length(normalSurface) < 0.0001f)
			normalSurface = g_normalZ;
		else
			normalSurface = glm::normalize(normalSurface);
	}

	float gBuoyancy = ppuffer->gBuoyancy * pwater->gBuoyancy;
	float gViscosity = ppuffer->gViscosity * pwater->gViscosity;

	if (gBuoyancy != 0.0f) {
		glm::vec3 dv = ppuffer->dvGravity * (-gBuoyancy * uSubmerged);

		ApplySoConstraintLocal(ppuffer, &ppuffer->constrForce, &dv, &dv, nullptr);
		AddSoAcceleration(ppuffer, &dv);
	}

	glm::vec3 vCurrent = g_vecZero;
	glm::vec3 wCurrent = g_vecZero;

	if (gViscosity != 0.0f) {
		CalculateWaterCurrent(pwater, &ppuffer->xf.posWorld, &vCurrent, &wCurrent);

		glm::vec3 dv = vCurrent - ppuffer->xf.v;
		glm::vec3 dvNormal;
		glm::vec3 dvTangent;

		SProjectVector(&dv, &normalSurface, &dvNormal, &dvTangent);

		CLQ clq;

		float scale = (uSubmerged + 0.75f) * gViscosity;
		clq.g0 = s_clqDampV.g0 * scale;
		clq.g1 = s_clqDampV.g1 * scale;
		clq.g2 = s_clqDampV.g2 * scale;

		glm::vec3 vNormal = ppuffer->xf.v + dvNormal;
		AccelSoTowardPosSpring(ppuffer, nullptr, nullptr, &vNormal, &clq, dt);

		scale = uSubmerged * gViscosity;
		clq.g0 = s_clqDampV.g0 * scale;
		clq.g1 = s_clqDampV.g1 * scale;
		clq.g2 = s_clqDampV.g2 * scale;

		glm::vec3 vTangent = ppuffer->xf.v + dvTangent;
		AccelSoTowardPosSpring(ppuffer, nullptr, nullptr, &vTangent, &clq, dt);

		glm::vec3 dw = g_vecZero - ppuffer->xf.w;
		glm::vec3 dwNormal;
		glm::vec3 dwTangent;

		SProjectVector(&dw, &normalSurface, &dwNormal, &dwTangent);

		scale = uSubmerged * gViscosity;
		clq.g0 = s_clqDampW.g0 * scale;
		clq.g1 = s_clqDampW.g1 * scale;
		clq.g2 = s_clqDampW.g2 * scale;

		glm::vec3 wNormal = ppuffer->xf.w + dwNormal;
		AccelSoTowardMatSpring(ppuffer, nullptr, nullptr, &wNormal, &clq, dt);

		scale = gViscosity;
		clq.g0 = s_clqDampW.g0 * scale;
		clq.g1 = s_clqDampW.g1 * scale;
		clq.g2 = s_clqDampW.g2 * scale;

		glm::vec3 wTangent = ppuffer->xf.w + dwTangent;
		AccelSoTowardMatSpring(ppuffer, nullptr, nullptr, &wTangent, &clq, dt);
	}

	if (uSubmerged > 0.0f && uSubmerged < 1.0f && normalSurface.z > 0.9f) {
		float gScale = ((ppuffer->posMax.x - ppuffer->posMin.x) + (ppuffer->posMax.y - ppuffer->posMin.y)) * 0.25f;
		WakeSoWater(ppuffer, pwater, dt, &vCurrent, &posSurface, 0.1f, gScale);
		return;
	}

	StopSound(ppuffer->pambWake, 0);
}

void HandlePufferMessage(PUFFER* ppuffer, MSGID msgid, void* pv)
{
	switch (msgid)
	{
		case MSGID_water_entered:
		ppuffer->pwater = *static_cast<WATER**>(pv);
		break;

		case MSGID_water_left:
		ppuffer->pwater = nullptr;
		break;

		case MSGID_sma_transition:
		{
			if (pv != ppuffer->psma)
				break;

			int smaState[4]{};
			GetSmaCur((SMA*)pv, (OID*)smaState);

			switch (smaState[0])
			{
				case 1011:
				g_crusherctr.pnActual = &ppuffer->npuffcEaten;
				g_puffchargectr.pnActual = &ppuffer->cpufftLift;
				g_puffchargectr.pnTotal = &ppuffer->cpufftMax;

				SetSwGameplayHud(ppuffer->psw, ppuffer);

				SetTimer(&g_timer, ppuffer->tGameMax);
				SetTimerExpire(&g_timer, 0.0f,(PFNTN)HandlePufferTimerExpiration);
				StartTimer(&g_timer);
				break;

				case 1012:
				case 1013:
				StopTimer(&g_timer);
				g_puffchargectr.pvtblot->pfnHideBlot(&g_puffchargectr);
				g_crusherctr.pvtblot->pfnHideBlot(&g_crusherctr);
				g_timer.pvtblot->pfnHideBlot(&g_timer);
				SetSwGameplayHud(ppuffer->psw, nullptr);
				break;

				default:
				break;
			}

			break;
		}

		default:
		break;
	}
}

int GetPufferSize()
{
	return sizeof(PUFFER);
}

void DeletePuffer(PUFFER *ppuffer)
{
	delete ppuffer;
}

PUFFB* NewPuffb()
{
	return new PUFFB{};
}

int GetPuffbSize()
{
	return sizeof(PUFFB);
}

void ClonePuffb(PUFFB* ppuffb, PUFFB* ppuffbBase)
{
	CloneAlo(ppuffb, ppuffbBase);

	ppuffb->cppathzone = ppuffbBase->cppathzone;
	ppuffb->cppuffv = ppuffbBase->cppuffv;
	ppuffb->cppuffc = ppuffbBase->cppuffc;
	ppuffb->tSpawnNext = ppuffbBase->tSpawnNext;

	ppuffb->appathzone = ppuffbBase->appathzone;
	ppuffb->mpippathzonecpuffc = ppuffbBase->mpippathzonecpuffc;
	ppuffb->appuffv = ppuffbBase->appuffv;
	ppuffb->appuffc = ppuffbBase->appuffc;
}

void PostPuffbLoad(PUFFB* ppuffb)
{
	PostAloLoad(ppuffb);
}

void LandPuffb(PUFFER* ppuffer, PUFFC* ppuffc)
{
	ROST* prost = ppuffc->prost;

	if (prost == nullptr)
		return;

	SetRostRosts(prost, ROSTS_Close);
	RemoveDlEntry(&ppuffer->dlRost0, prost);
	AppendDlEntry(&ppuffer->dlRost1, prost);
	ppuffc->prost = nullptr;
}

void UpdatePuffb(PUFFB* ppuffb, float dt)
{
	UpdateAlo(ppuffb, dt);
}

void DeletePuffb(PUFFB* ppuffb)
{
	delete ppuffb;
}

PUFFV* NewPuffv()
{
	return new PUFFV{};
}

void InitPuffv(PUFFV* ppuffv)
{
	InitSo(ppuffv);
}

int GetPuffvSize()
{
	return sizeof(PUFFV);
}

void ClonePuffv(PUFFV* ppuffv, PUFFV* ppuffvBase)
{
	CloneSo(ppuffv, ppuffvBase);

	// Shallow copy of the value members
	ppuffv->oidPathzone = ppuffvBase->oidPathzone;
	ppuffv->oidPuffvJump = ppuffvBase->oidPuffvJump;
	ppuffv->coidPuffvLand = ppuffvBase->coidPuffvLand;
	ppuffv->cppuffvLand = ppuffvBase->cppuffvLand;
	ppuffv->tChosen = ppuffvBase->tChosen;

	// Shallow copy of pointer members
	ppuffv->ppathzone = ppuffvBase->ppathzone;
	ppuffv->ppuffvJump = ppuffvBase->ppuffvJump;
	std::copy(std::begin(ppuffvBase->aoidPuffvLand), std::end(ppuffvBase->aoidPuffvLand), std::begin(ppuffv->aoidPuffvLand));
	std::copy(std::begin(ppuffvBase->appuffvLand), std::end(ppuffvBase->appuffvLand), std::begin(ppuffv->appuffvLand));
}

void PostPuffvLoad(PUFFV* ppuffv)
{
	PostLoLoad(ppuffv);
}

void DeletePuffv(PUFFV* ppuffv)
{
	delete ppuffv;
}

PUFFC* NewPuffc()
{
	return new PUFFC{};
}

int GetPuffcSize()
{
	return sizeof(PUFFC);
}

void ClonePuffc(PUFFC* ppuffc, PUFFC* ppuffcBase)
{
	CloneStepguard(ppuffc, ppuffcBase);

	// Shallow copy of the value members
	ppuffc->posPathzoneNext = ppuffcBase->posPathzoneNext;

	// Shallow copy of the pointer members
	ppuffc->pwater = ppuffcBase->pwater;
	ppuffc->ppathzoneNext = ppuffcBase->ppathzoneNext;
	ppuffc->ppuffvNext = ppuffcBase->ppuffvNext;
	ppuffc->ppuffb = ppuffcBase->ppuffb;
}

void PostPuffcLoad(PUFFC* ppuffc)
{
	PostStepguardLoad(ppuffc);
}

void PresetPuffcAccel(PUFFC* ppuffc, float dt)
{
	if (ppuffc->pwater == nullptr || g_clock.t < ppuffc->tFreefallLand) {
		PresetStepguardAccel(ppuffc, dt);
		return;
	}

	PresetStepAccelBase(ppuffc);

	glm::vec3 vGoal = glm::vec3(ppuffc->xf.matWorld * glm::vec4(-ppuffc->vTarget, 0.0f));

	glm::vec3 accel = (vGoal - ppuffc->xf.v) / dt;

	// The retail code reads ag[1]/ag[0], which are Y/X from the first
	// (forward) matrix vector. GLM is column-major, so both components belong
	// to column zero.
	float radCurrent = atan2f(ppuffc->xf.matWorld[0].y, ppuffc->xf.matWorld[0].x);

	float drad = RadNormalize(ppuffc->radTarget - radCurrent);

	float uTurnLimit = s_clqDradToULimit.g0 + glm::abs(drad) * (s_clqDradToULimit.g1 + glm::abs(drad) * s_clqDradToULimit.g2);

	uTurnLimit = glm::clamp(uTurnLimit, g_lmZeroOne.gMin, g_lmZeroOne.gMax);

	LimitVectorLength(&accel, SDV_PuffcMax * uTurnLimit, &accel);

	AddSoAcceleration(ppuffc, &accel);

	if (ppuffc->paloParent == nullptr && !ppuffc->fNoGravity) 
	{
		glm::vec3 constrainedAccel = ppuffc->dvGravity;
		ApplySoConstraintLocal(ppuffc, &ppuffc->constrForce, &ppuffc->dvGravity, &constrainedAccel, nullptr);
		AddSoAcceleration(ppuffc, &constrainedAccel);
	}

	for (XA* pxa = ppuffc->pxa; pxa != nullptr; pxa = pxa->pxaNextSource) 
	{
		SO* psoSource = pxa->psoSource;
		psoSource->pvtso->pfnAddSoExternalAccelerations(psoSource, pxa, dt);
	}
}

SGS SgsNextPuffcAI(PUFFC* ppuffc)
{
	if (ppuffc->sgs == SGS_PatrolIdle && SggsGetStepguard(ppuffc) == SGGS_Patrol)
		return SGS_ReturnToPatrol;

	if (ppuffc->sgs == SGS_Stun) 
		return ppuffc->pwater != nullptr ? SGS_Reset : SGS_Stun;

	return SgsNextStepguardAI(ppuffc);
}

void UpdatePuffcGoal(PUFFC* ppuffc, int fEnter)
{
	glm::vec3 goal;

	if (ppuffc->sgs == SGS_ReturnToPatrol) {
		if (!fEnter) {
			return;
		}

		do {
			ChoosePathzoneRandomPoint(ppuffc->ppathzone, &goal);
		} while (glm::length(glm::vec2(goal - ppuffc->xf.posWorld)) <= S_PuffcReturnToPatrolMinDist);

		SetStepguardGoal(ppuffc, &goal);
		return;
	}

	if (ppuffc->sgs == SGS_Stun) {
		if (!fEnter) {
			return;
		}

		FindPathzoneClosestPoint(ppuffc->ppathzone, &ppuffc->xf.posWorld, &goal);
		SetStepguardGoal(ppuffc, &goal);
		return;
	}

	if (ppuffc->sgs > SGS_Dying && ppuffc->sgs < SGS_Attack) {
		PO* ppo = PpoCur();

		glm::vec3 offset = ppuffc->xf.posWorld - ppo->xf.posWorld;
		offset.z = 0.0f;

		offset = glm::normalize(offset) * 800.0f;

		ClipPathzoneDirection(1.0f, ppuffc->ppathzone, &ppuffc->xf.posWorld, &offset);

		goal = ppuffc->xf.posWorld + offset;
		SetStepguardGoal(ppuffc, &goal);
		return;
	}

	UpdateStepguardGoal((STEPGUARD*)ppuffc, fEnter);
}
void OnPuffcExitingSgs(PUFFC* ppuffc, SGS sgsNext)
{
	if (ppuffc->sgs == SGS_Stun) 
	{
		PUFFER* ppuffer = static_cast<PUFFER*>(PpoCur());

		LandPuffb(ppuffer, ppuffc);

		glm::vec3 vLanding = ppuffc->xf.v;
		vLanding.z = 0.0f;

		ppuffc->pvtalo->pfnSetAloVelocityVec(ppuffc, &vLanding);
	}

	OnStepguardExitingSgs(ppuffc, sgsNext);
}

void OnPuffcEnteringSgs(PUFFC* ppuffc, SGS sgsPrev, ASEG* pasegOverride)
{
	OnStepguardEnteringSgs(ppuffc, sgsPrev, pasegOverride);

	if (ppuffc->sgs == SGS_Stun) 
		MoveStepguardToGoal(ppuffc);
}

void UpdatePuffcSgs(PUFFC* ppuffc)
{
	UpdateStepguardSgs(ppuffc);

	if (ppuffc->sgs != SGS_Pursue)
		return;

	const float uSuck = g_plsCur->uSuck;

	glm::vec3 speedCurve;
	speedCurve.x = glm::mix(1.8, 1.4, uSuck);
	speedCurve.y = glm::mix(-0.00024999998, -0.0049999999, uSuck);
	speedCurve.z = glm::mix(0.0, 0.0, uSuck);

	float speedMin = glm::mix(1.1, 0.89999998, uSuck);
	float speedMax = glm::mix(1.6, 1.2, uSuck);

	PO* ppo = PpoCur();

	float dist = glm::length(ppuffc->xf.posWorld - ppo->xf.posWorld);

	float speed = speedCurve.x + dist * (speedCurve.y + dist * speedCurve.z);

	speed = glm::clamp(speed, speedMin, speedMax);

	SetAsegaSpeed(ppuffc->pasegaSgs, speed);
}

void HandlePuffcMessage(PUFFC* ppuffc, MSGID msgid, void* pv)
{
	HandleStepguardMessage(static_cast<STEPGUARD*>(ppuffc), msgid, pv);

	if (msgid != MSGID_water_entered && msgid != MSGID_water_left)
		return;

	PUFFERWATERMSG* pmsg = static_cast<PUFFERWATERMSG*>(pv);

	if (pmsg->palo != static_cast<SO*>(ppuffc))
		return;

	ppuffc->pwater = msgid == MSGID_water_entered ? pmsg->pwater : nullptr;
}

void UpdatePuffc(PUFFC* ppuffc, float dt)
{
	PUFFER* ppuffer = (PUFFER*)PpoCur();

	if (FIsBasicDerivedFrom(ppuffer, CID_PUFFER) &&
		ppuffer->pvolCollect != nullptr &&
		FIsLoInWorld(ppuffer->pvolCollect) &&
		FCheckVolPoint(ppuffer->pvolCollect, &ppuffc->xf.posWorld))
	{
		WKR wkr = {};
		wkr.grftak = 2;
		wkr.ploSource = (LO*)ppuffc;
		wkr.ploTarget = (LO*)ppuffc;

		if (PaloAbsorbWkr(&wkr, 0, nullptr) != nullptr) {
			// A collected stunned PUFFC is removed before its normal state-exit
			// callback, so release its occupied ROST here first.
			if (ppuffc->prost != nullptr)
				LandPuffb(ppuffer, ppuffc);

			ppuffc->pvtlo->pfnRemoveLo(ppuffc);
			ppuffer->npuffcEaten = glm::min(ppuffer->npuffcEaten + 1, g_cPuffChargeMax);
			ppuffer->tNextPuffcEat = g_clock.t + DT_PuffcEaten;

			return;
		}
	}

	UpdateStepguard(ppuffc, dt);
}

int FDetectPuffc(PUFFC* ppuffc)
{
	PO* ppo = PpoCur();

	if (ppo == nullptr || !FIsBasicDerivedFrom(ppo, CID_PUFFER)) {
		return false;
	}

	float dist = glm::length(glm::vec2(ppo->xf.posWorld - ppuffc->xf.posWorld));

	if (!ppuffc->fNearPlayer) 
	{
		if (dist < S_PuffcNearPlayerDistEnter)
			ppuffc->fNearPlayer = true;
	}
	else 
	{
		if (dist > S_PuffcNearPlayerDistExit) 
			ppuffc->fNearPlayer = false;
	}

	return ppuffc->fNearPlayer;
}

void DeletePuffc(PUFFC* ppuffc)
{
	delete ppuffc;
}

PUFFT* NewPufft()
{
	return new PUFFT{};
}

int GetPufftSize()
{
	return sizeof(PUFFT);
}

void ClonePufft(PUFFT* ppufft, PUFFT* ppufftBase)
{
	CloneSo(ppufft, ppufftBase);

	ppufft->fLit = ppufftBase->fLit;

	ppufft->ppntFire = ppufftBase->ppntFire;
	ppufft->pemitterFire = ppufftBase->pemitterFire;
	ppufft->pexplo = ppufftBase->pexplo;
}

void PostPufftLoad(PUFFT* ppufft)
{
	PostAloLoad(ppufft);
	SnipAloObjects(ppufft, 3, s_asnipPostPufftLoad);

	PUFFER* ppuffer = reinterpret_cast<PUFFER*>(PloFindSwObjectByClass(ppufft->psw, 5, CID_PUFFER, nullptr));

	if (ppuffer != nullptr)
		++ppuffer->cpufftMax;
}

void DeletePufft(PUFFT *ppufft)
{
	delete ppufft;
}

float ELAS_Puffer = 0.3f;
float MU_Puffer = 0.3f;
float SDV_PuffcMax = 5000.0f;
CLQ s_clqDradToULimit{3.0, -2.0, -1.0};
PUFFER* g_ppuffer;
int g_cPuffChargeMax = 5;
float DT_PuffcEaten = 0.5;
float S_PuffcReturnToPatrolMinDist = 500.0;
float S_PuffcNearPlayerDistEnter = 1100.0;
float S_PuffcNearPlayerDistExit = 2000.0;
SNIP s_asnipLoadPuffer[1] = 
{
	0, (OID)0x3D9, offsetof(PUFFER, psmShoot)
};
float SDV_PufferGroundMax = 4000.0;
float SV_PufferGroundMax = 2000.0;

SNIP s_asnipPostPufferLoad[13] =
{
	{ 0x24, (OID)0x3DD, offsetof(PUFFER, pvolCollect) },
	{ 0x20, (OID)0x3DE, offsetof(PUFFER, pvolFire) },
	{ 0x00, (OID)0x3E7, offsetof(PUFFER, appntFloor[0]) },
	{ 0x00, (OID)0x3E8, offsetof(PUFFER, appntFloor[1]) },
	{ 0x00, (OID)0x3E9, offsetof(PUFFER, appntFloor[2]) },
	{ 0x00, (OID)0x3EA, offsetof(PUFFER, appntFloor[3]) },
	{ 0x00, (OID)0x3EC, offsetof(PUFFER, ppntFireNatural) },
	{ 0x04, (OID)0x3ED, offsetof(PUFFER, paloFireGun) },
	{ 0x00, (OID)0x3EE, offsetof(PUFFER, pemitterFire) },
	{ 0x00, (OID)0x3F0, offsetof(PUFFER, pemitterSpray) },
	{ 0x04, (OID)0x3EF, offsetof(PUFFER, paloFireGunTarget) },
	{ 0x24, (OID)0x3EB, offsetof(PUFFER, pexploSplineTarget) },
	{ 0x01, (OID)0x3F1, offsetof(PUFFER, psm) },
};

SMP s_smpPuffer = {10.0, 0.0, 0.1};
CLQ s_clqDradToUPuff = {3.0, -2.0, -1.0};
float DT_PufferPuffAfterGround = 0.25;
// Full drive while grounded, then smoothly remove thrust after the short
// post-ground grace period.  A negative constant made uGround clamp to zero
// on every frame, so steering worked while propulsion never did.
CLQ s_clqUDtGroundToUPuff = {1.2f, 0.0f, -0.2f};
float SDV_PufferWaterMax = 4000.0f;
float SV_PufferWaterMax = 2200;
float DZ_PointCheckMin = -60.0;
float DZ_PointCheckMax = 50.0;
float S_PufftChooseMax = 250.0;
float DT_PufftChooseProj = 0.25;
float DRAD_PufftChooseMax = 0.69999999;
float D_T_Max = 3.4028235E+38;
SNIP s_asnipPostPufftLoad[3] = 
{
	0x0,  (OID)0x3E4, offsetof(PUFFT, pemitterFire),
	0x0,  (OID)0x3E5, offsetof(PUFFT, ppntFire),
    0x20, (OID)0x3e6, offsetof(PUFFT, pexplo)
};

float DT_PufftIgniteDelay = 0.25;
