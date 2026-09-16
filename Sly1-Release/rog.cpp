#include "rog.h"
#include "asega.h"
#include "sm.h"
#include "emitter.h"
#include "xform.h"
#include "coin.h"
#include "expl.h"
#include "dialog.h"
#include "binoc.h"
#include "credit.h"
#include "trunk.h"

ROV* NewRov()
{
	return new ROV{};
}

void InitRov(ROV* prov)
{
	InitPo(prov);

	prov->rovts = ROVTS_Nil;
	prov->svMax = 2000.0;
	prov->rovs  = ROVS_Nil;
}

void SetRovSvMax(ROV* prov, float svMax)
{
	prov->svMax = svMax;
}

void* GetRovSvMax(ROV* prov)
{
	return &prov->svMax;
}

void CloneRov(ROV* prov, ROV* provBase)
{
	ClonePo(prov, provBase); // Assuming ClonePo handles copying the base class (PO) members

	// Shallow copy of value members
	prov->rovs = provBase->rovs;  // Copying the ROVS object
	prov->rovts = provBase->rovts;  // Copying the ROVTS object
	prov->radJoyLeft = provBase->radJoyLeft;
	prov->uJoyDeflectLeft = provBase->uJoyDeflectLeft;
	prov->radJoyRight = provBase->radJoyRight;
	prov->uJoyDeflectRight = provBase->uJoyDeflectRight;
	prov->svMax = provBase->svMax;
	prov->vTarget = provBase->vTarget;

	// Shallow copy of the pointer members (simply copy the pointer, not the data)
	prov->psmBody = provBase->psmBody;
	prov->psmaBody = provBase->psmaBody;
	prov->pambBody = provBase->pambBody;
	prov->paloTurret = provBase->paloTurret;
	prov->psmTurret = provBase->psmTurret;
	prov->psmaTurret = provBase->psmaTurret;
	prov->pambTurret = provBase->pambTurret;
	prov->pemitterLeftGun = provBase->pemitterLeftGun;
	prov->pemitterRightGun = provBase->pemitterRightGun;
	prov->paloProp = provBase->paloProp;
	prov->cpemitterProp = provBase->cpemitterProp;

	// Shallow copy of the array of pointers
	for (int i = 0; i < 3; ++i) {
		prov->apemitterProp[i] = provBase->apemitterProp[i];
	}

	// Copying the linked structure members
	prov->prob = provBase->prob;
	prov->pasegAscend = provBase->pasegAscend;
	prov->pasegDescend = provBase->pasegDescend;
	prov->pasegaDrive = provBase->pasegaDrive;

	// Copying the flag for retracting the drive
	prov->fRetractDrive = provBase->fRetractDrive;
}

int GetRovSize()
{
	return sizeof(ROV);
}

void LoadRovFromBrx(ROV* prov, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prov, pbis);
	SnipAloObjects(prov, 2, s_asnipLoadRov);
}

void PostRovLoad(ROV* prov)
{
	PostAloLoad(prov);

	if (prov->prob == nullptr) 
	{
		prov->pvtlo->pfnRemoveLo(prov);
		return;
	}

	SnipAloObjects(prov, 6, s_asnipPostRovLoad);

	SetSoConstraints(prov, CT_Free, nullptr, CT_Locked, nullptr);

	if (prov->psmBody != nullptr) 
		prov->psmaBody = PsmaApplySm(prov->psmBody, prov, OID_Nil, 0);

	if (prov->psmTurret != nullptr) 
		prov->psmaTurret = PsmaApplySm(prov->psmTurret, prov, OID_Nil, 1);

	prov->cpemitterProp = CploFindSwObjectsByClass(g_psw, 1, CID_EMITTER, prov->paloProp, 3, (LO**)prov->apemitterProp);

	SetRovRovs(prov, ROVS_Descend);
	SetRovRovts(prov, ROVTS_Calm);
}

void UpdateRovActive(ROV* prov, JOY* pjoy, float dt)
{
	if (prov->fRetractDrive) 
	{
		RetractAsega(prov->pasegaDrive);
		prov->pasegaDrive = nullptr;
		prov->fRetractDrive = 0;
	}

	ReadRovJoystick(prov, pjoy);

	SetVectorCylind(&prov->vTarget, prov->radJoyLeft, prov->uJoyDeflectLeft * prov->svMax, 0.0f);

	SMP smpTurret = {};
	smpTurret.svFast = 30.0f;
	smpTurret.svSlow = 0.0f;
	smpTurret.dtFast = 0.2f;

	float radTurret = atan2f(prov->paloTurret->xf.matWorld[0][1], prov->paloTurret->xf.matWorld[0][0]);

	radTurret = RadSmooth(radTurret, prov->radJoyRight, g_clock.dt, &smpTurret, nullptr);

	glm::mat3 matTurret;
	LoadRotateMatrixPanTilt(radTurret, 0.0f, &matTurret);

	ConvertAloMat(nullptr, static_cast<ALO*>(prov), &matTurret, &matTurret);

	prov->paloTurret->pvtalo->pfnRotateAloToMat(prov->paloTurret, &matTurret);
}

void UpdateRov(ROV* prov, float dt)
{
	UpdatePo(prov, dt);

	if (FDrivenAlo(prov)) 
	{
		glm::vec3 posGoal = {};
		glm::vec3 vGoal = {};
		glm::vec3 normal = g_normalX;
		glm::vec3 vTang = {};

		prov->pactPos->pvtact->pfnGetActPositionGoal(prov->pactPos, 0.0f, &posGoal, &vGoal);

		if (prov->rovs == ROVS_Ascend) {
			glm::vec3 dpos = posGoal - prov->xf.pos;
			float dist = glm::length(dpos);

			if (dist >= 0.0001f) {
				normal = dpos / dist;
			}

			float svProject = SProjectVector(&vGoal, &normal, nullptr, &vTang);

			SMP smp = {};
			smp.svFast = prov->psmpaPos->svFast + svProject;
			smp.svSlow = prov->psmpaPos->svSlow + svProject;
			smp.dtFast = prov->psmpaPos->dtFast;

			float svNext = 0.0f;
			float sNext = GSmooth(0.0f, dist, g_clock.dt, &smp, &svNext);

			posGoal = prov->xf.pos + normal * sNext;
			vGoal = vTang + normal * svNext;
		}

		prov->pvtalo->pfnTranslateAloToPos(prov, &posGoal);
		prov->pvtalo->pfnSetAloVelocityVec(prov, &vGoal);

		prov->radJoyLeft = atan2f(vGoal.y, vGoal.x);
	}

	while (true) 
	{
		ROVS rovs = RovsNextRov(prov);

		if (rovs == prov->rovs) 
			break;

		SetRovRovs(prov, rovs);
	}

	while (true) 
	{
		ROVTS rovts = RovtsNextRov(prov);

		if (rovts == prov->rovts) 
			break;

		SetRovRovts(prov, rovts);
	}

	ResolveAlo(prov);
}

void ReadRovJoystick(ROV* prov, JOY* pjoy)
{
	if (pjoy->uDeflect > 0.3f) 
		AdjustCmJoy(g_pcm, pjoy, JOYID_Left, &prov->radJoyLeft);

	prov->uJoyDeflectLeft = pjoy->uDeflect;

	if (pjoy->uDeflect2 > 0.8f) 
		AdjustCmJoy(g_pcm, pjoy, JOYID_Right, &prov->radJoyRight);

	prov->uJoyDeflectRight = pjoy->uDeflect2;
}

ROVS RovsNextRov(ROV* prov)
{
	switch (prov->rovs) {
		case ROVS_Brake:
		if (prov->uJoyDeflectLeft > 0.0f) 
			return ROVS_Drive;
		return ROVS_Brake;

		case ROVS_Drive:
		if (prov->uJoyDeflectLeft != 0.0f)
			return ROVS_Drive;
	
		return ROVS_Brake;

		case ROVS_Descend:
		if (prov->pasegaDrive == nullptr) 
			return ROVS_Drive;
		return ROVS_Descend;

		default:
		return prov->rovs;
	}
}

void SetRovRovs(ROV* prov, ROVS rovs)
{
	ROVS rovsPrev = prov->rovs;

	if (rovsPrev == rovs) {
		return;
	}

	prov->rovs = rovs;

	int oid = -1;
	int sfxid = 333;
	ASEG* paseg = nullptr;
	bool pausePropEmitters = false;

	switch (rovs) {
		case ROVS_Drive:
		oid = 656;
		break;

		case ROVS_Brake:
		oid = 655;
		sfxid = -1;
		pausePropEmitters = true;
		break;

		case ROVS_Descend:
		oid = 656;
		paseg = prov->pasegDescend;
		break;

		case ROVS_Ascend:
		oid = 656;
		paseg = prov->pasegAscend;
		TriggerDefaultExit(0, WIPEK_Keyhole);
		break;

		default:
		break;
	}

	for (int i = 0; i < prov->cpemitterProp; ++i) {
		if (pausePropEmitters) {
			PauseEmitterIndefinite(prov->apemitterProp[i]);
		}
		else {
			UnpauseEmitter(prov->apemitterProp[i]);
		}
	}

	if (prov->psmaBody != nullptr) {
		if (rovsPrev == ROVS_Nil) {
			SeekSma(prov->psmaBody, (OID)oid);
		}
		else {
			SetSmaGoal(prov->psmaBody, (OID)oid);
		}
	}

	if (paseg != nullptr) {
		ApplyAseg(paseg, static_cast<ALO*>(prov), 0.0f, 1.0f, 1, &prov->pasegaDrive);
	}

	if (prov->pambBody != nullptr) {
		if (prov->pambBody->sfxid == sfxid) {
			return;
		}

		StopSound(prov->pambBody, 0);
		prov->pambBody = nullptr;
	}

	if (sfxid != -1) {
		StartSound((SFXID)sfxid, &prov->pambBody, static_cast<ALO*>(prov), nullptr, 10000.0f, 6000.0f, 0.5f, 0.0f, 0.0f, nullptr, nullptr);
	}
}
ROVTS RovtsNextRov(ROV* prov)
{
	switch (prov->rovts) 
	{
		case ROVTS_Calm:
		if (prov->uJoyDeflectRight >= 0.8f) {
			return ROVTS_Firing;
		}
		return ROVTS_Calm;

		case ROVTS_Firing:
		if (prov->uJoyDeflectRight < 0.8f) {
			return ROVTS_Calm;
		}
		return ROVTS_Firing;

		default:
		return prov->rovts;
	}
}

void SetRovRovts(ROV* prov, ROVTS rovts)
{
	ROVTS rovtsPrev = prov->rovts;

	if (rovtsPrev == rovts) 
		return;

	prov->rovts = rovts;

	int oid = OID_Nil;
	int sfxid = SFXID_Nil;

	switch (rovts) 
	{
		case ROVTS_Calm:
		oid = 659;
		sfxid = -1;
		break;

		case ROVTS_Firing:
		oid = 660;
		sfxid = 336;
		break;

		default:
		break;
	}

	if (prov->psmaTurret != nullptr) {
		if (rovtsPrev == ROVTS_Nil) {
			SeekSma(prov->psmaTurret, (OID)oid);
		}
		else {
			SetSmaGoal(prov->psmaTurret, (OID)oid);
		}
	}

	if (prov->pambTurret != nullptr) {
		if (prov->pambTurret->sfxid == sfxid) {
			return;
		}

		StopSound(prov->pambTurret, 0);
		prov->pambTurret = nullptr;
	}

	if (sfxid != -1) {
		StartSound((SFXID)sfxid, &prov->pambTurret, static_cast<ALO*>(prov), nullptr, 10000.0f, 6000.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
	}
}

void HandleRovMessage(ROV* prov, MSGID msgid, void* pv)
{
	HandlePoMessage(prov, msgid, pv);

	if (msgid == MSGID_asega_limit) {
		if (pv == prov->pasegaDrive) {
			prov->fRetractDrive = 1;
		}

		return;
	}

	if (msgid != MSGID_label) {
		return;
	}

	ASEGLABELMSG* plabelmsg = static_cast<ASEGLABELMSG*>(pv);
	int label = plabelmsg->label;

	if (label == 661) 
	{
		StartSound((SFXID)338, nullptr, nullptr, &prov->xf.posWorld, 12000.0f, 8000.0f, 0.5f, 0.0f, 0.0f, nullptr, nullptr);
		UnpauseEmitter(prov->pemitterLeftGun);
	}
	else if (label == 662) 
	{
		StartSound((SFXID)338, nullptr, nullptr, &prov->xf.posWorld, 12000.0f, 8000.0f, 0.5f, 0.0f, 0.0f, nullptr, nullptr);
		UnpauseEmitter(prov->pemitterRightGun);
	}
}

void PresetRovAccel(ROV* prov, float dt)
{
	PresetSoAccel(prov, dt);

	glm::mat3 matTarget;
	LoadRotateMatrixPanTilt(prov->radJoyLeft, 0.0f, &matTarget);

	AccelSoTowardMatSmooth(prov, g_clock.dt, &matTarget, &s_smpRovSpin);
}

void AdjustRovXpVelocity(ROV* prov, XP* pxp, int ixpd)
{
	if (FDrivenAlo(prov))
		return;

	glm::vec3 vTangent = prov->vTarget - pxp->normal * glm::dot(prov->vTarget, pxp->normal);

	pxp->axpd[ixpd].v -= vTangent;
}

void AdjustRovNewXp(ROV* prov, XP* pxp, int ixpd)
{
	int fDriveAlo = FDrivenAlo(prov);

	if ((fDriveAlo == 0) && (0.7 < (pxp->normal).z))
		pxp->mu = 20.0;
}

void PropagateRovForce(ROV* prov, GRFSG grfsg, XP* pxp, int ixpd, DZ* pdz, FX* afx)
{
	const CONSTR constrForcePrev = prov->constrForce;
	BASIC* pbasicOther = pxp->axpd[1 - ixpd].psoRoot;

	if (pbasicOther != nullptr && (FIsBasicDerivedFrom(pbasicOther, CID_ROC) || FIsBasicDerivedFrom(pbasicOther, CID_ROH)))
	{
		prov->constrForce.ct = CT_Tangent;
		prov->constrForce.normal = g_normalZ;
	}

	PropagateSoForce(prov, grfsg, pxp, ixpd, pdz, afx);

	prov->constrForce = constrForcePrev;
}

int FIsRovSoundBase()
{
	return 0;
}

void DeleteRov(ROV *prov)
{
	delete prov;
}

ROH* NewRoh()
{
	return new ROH{};
}

void InitRoh(ROH* proh)
{
	InitSo(proh);

	proh->rohs = ROHS_Nil;
	(proh->lmDtWander).gMin = 1.0;
	(proh->lmDtWander).gMax = 2.5;
	SetSoConstraints(proh, CT_Tangent, &g_normalZ, CT_Project, &g_normalZ);
}

void SetRohLmDtWander(ROH* proh, LM lmDtWander)
{
	proh->lmDtWander = lmDtWander;
}

void* GetRohLmDtWander(ROH* proh)
{
	return &proh->lmDtWander;
}

int GetRohSize()
{
	return sizeof(ROH);
}

void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(proh, pbis);
	SnipAloObjects(proh, 6, s_asnipLoadRoh);
	InferExpl(&proh->pexpl, proh);
}

void CloneRoh(ROH* proh, ROH* prohBase)
{
	DLE savedDle = proh->dleRob;

	CloneSo(proh, prohBase); // Assuming CloneSo handles copying the base class (SO) members

	// Shallow copy of value members
	proh->rohs = prohBase->rohs;  // Copying the ROHS object
	proh->tRohs = prohBase->tRohs;
	proh->svMax = prohBase->svMax;
	proh->dradMax = prohBase->dradMax;
	proh->posWander = prohBase->posWander;
	proh->tWanderNext = prohBase->tWanderNext;
	proh->lmDtWander = prohBase->lmDtWander;

	// Shallow copy of the pointer members (simply copy the pointer, not the data)
	proh->prob = prohBase->prob;
	proh->proc = prohBase->proc;
	proh->prost = prohBase->prost;
	proh->pasegMove = prohBase->pasegMove;
	proh->pasegMoveCollect = prohBase->pasegMoveCollect;
	proh->pasegaGrab = prohBase->pasegaGrab;
	proh->pasegDie = prohBase->pasegDie;
	proh->pasega = prohBase->pasega;
	proh->psmHappy = prohBase->psmHappy;
	proh->psmSad = prohBase->psmSad;
	proh->psma = prohBase->psma;
	proh->pexpl = prohBase->pexpl;

	// Shallow copy of the DLE (linked list)
	proh->dleRob = prohBase->dleRob;

	proh->dleRob = savedDle;
}

void PostRohLoad(ROH* proh)
{
	PostAloLoad(proh);
	SetRohRohs(proh, ROHS_Inactive);
	proh->pvtlo->pfnRemoveLo(proh);
}

void UpdateRoh(ROH* proh, float dt)
{
	UpdateSo(proh, dt);

	for (;;) 
	{
		const ROHS nextState = RohsNextRoh(proh);

		if (nextState == proh->rohs) {
			break;
		}

		SetRohRohs(proh, nextState);
	}

	switch (proh->rohs) 
	{
		case ROHS_Inactive:
		case ROHS_Grab:
		case ROHS_Happy:
		case ROHS_Sad:
		case ROHS_Die:
		case ROHS_Dead:
		{
			glm::vec4 zeroVelocity(0.0f);

			proh->pvtalo->pfnSetAloVelocityVec(proh, &g_vecZero);
			proh->pvtalo->pfnSetAloAngularVelocityVec(proh, &g_vecZero);
			break;
		}

		case ROHS_Spawn:
		{
			glm::vec3 spawnPosition;

			GetPntPos(proh->prost->ppnt, &spawnPosition);
			MoveRohToTarget(proh, &spawnPosition, 0);
			break;
		}

		case ROHS_Wander:
		{
			const glm::vec2 currentPosition(proh->xf.posWorld);
			const glm::vec2 wanderPosition(proh->posWander);
			const float distanceToTarget = glm::distance(currentPosition, wanderPosition);

			if (g_clock.t > proh->tWanderNext || distanceToTarget < 50.0f) 
			{
				ChooseRobWanderLocation(proh->prob, proh);
				proh->tWanderNext = g_clock.t + GRandInRange(proh->lmDtWander.gMin, proh->lmDtWander.gMax);
			}

			MoveRohToTarget(proh, &proh->posWander, 0);
			break;
		}

		case ROHS_Collect:
		MoveRohToTarget(proh, &proh->proc->xf.posWorld, 0);
		break;

		case ROHS_Return:
		{
			glm::vec3 returnPosition;
			GetPntPos(proh->prost->ppnt, &returnPosition);
			MoveRohToTarget(proh, &returnPosition, 1);
			break;
		}

		case ROHS_Exit:
		MoveRohToTarget(proh, &proh->prost->xf.posWorld, 1);
		break;
	}
}

void MoveRohToTarget(ROH* proh, glm::vec3* targetPosition, int fBackwards)
{
	glm::vec3 startPosition = proh->xf.posWorld;
	glm::vec3 destination = *targetPosition;

	startPosition.z = 0.0f;
	destination.z = 0.0f;

	const float arenaRadius = proh->prob->sRadiusArenaTube;

	if (arenaRadius > 0.0001f) {
		LSG clippedSegment{};

		if (ClsgClipEdgeToSphere(&proh->prob->posCenterArena, arenaRadius, &startPosition, &destination, 1, &clippedSegment) != 0) {
			const glm::vec3 midpoint = (clippedSegment.apos[0] + clippedSegment.apos[1]) * 0.5f;
			const glm::vec3 directionFromCenter = glm::normalize(midpoint - proh->prob->posCenterArena);

			destination = directionFromCenter * (arenaRadius * 1.5f);
		}
	}

	const glm::vec3 displacement = destination - startPosition;
	const float targetHeading = std::atan2(displacement.y, displacement.x);

	float facingHeading = std::atan2(proh->xf.matWorld[0][1], proh->xf.matWorld[0][0]);

	if (fBackwards != 0) {
		facingHeading = RadNormalize(facingHeading + 3.1415927f);
	}

	const float headingError = RadNormalize(targetHeading - facingHeading);

	if (std::abs(headingError) > 0.05235988f) {
		SetAloVelocityVec(proh, &g_vecZero);
	}
	else {
		const float distance = glm::length(displacement);
		float speed = std::min(distance / g_clock.dt, proh->svMax);

		if (fBackwards != 0) {
			speed = -speed;
		}

		glm::vec3 localVelocity(speed, 0.0f, 0.0f);
		SetAloVelocityLocal(proh, &localVelocity);
	}

	const float angularVelocity = std::clamp(headingError / g_clock.dt, -proh->dradMax, proh->dradMax);
	SetAloAngularVelocityXYZ(proh, 0.0f, 0.0f, angularVelocity);
}

ROHS RohsNextRoh(ROH* proh)
{
	switch (proh->rohs) 
	{
		case ROHS_Spawn:
		{
			ROC* proc = ProcContactRoh(proh);

			if (proc != nullptr) 
			{
				TakeRobRoc(proh->prob, proh, proc);
				return ROHS_Grab;
			}

			glm::vec3 spawnPosition;
			GetPntPos(proh->prost->ppnt, &spawnPosition);

			const float distanceToSpawn = glm::distance(
				glm::vec2(spawnPosition), glm::vec2(proh->xf.posWorld));

			if (distanceToSpawn < 10.0f) {
				return FChooseRobRoc(proh->prob, proh) != 0 ? ROHS_Collect : ROHS_Wander;
			}

			return ROHS_Spawn;
		}

		case ROHS_Wander:
		case ROHS_Collect:
		{
			ROC* proc = ProcContactRoh(proh);

			if (proc != nullptr) {
				TakeRobRoc(proh->prob, proh, proc);
				return ROHS_Grab;
			}

			return proh->rohs;
		}

		case ROHS_Grab:
		if (proh->pasega != nullptr && proh->pasega->tLocal < proh->pasegaGrab->tMax) {
			return ROHS_Grab;
		}

		return FChooseRobReturnPoint(proh->prob, proh) != 0 ? ROHS_Return : ROHS_Wander;

		case ROHS_Return:
		{
			glm::vec3 spawnPosition;
			GetPntPos(proh->prost->ppnt, &spawnPosition);

			const float distanceToSpawn = glm::distance(
				glm::vec2(proh->xf.posWorld), glm::vec2(spawnPosition));

			return distanceToSpawn < 10.0f ? ROHS_Exit : ROHS_Return;
		}

		case ROHS_Exit:
		{
			const float distanceToExit = glm::distance(
				glm::vec2(proh->xf.posWorld), glm::vec2(proh->prost->xf.posWorld));

			return distanceToExit < 10.0f ? ROHS_Inactive : ROHS_Exit;
		}

		case ROHS_Die:
		return ROHS_Dead;

		default:
		return proh->rohs;
	}
}

void SetRohRohs(ROH* proh, ROHS rohs)
{
	if (proh->rohs == rohs)
		return;

	if (proh->pasega != nullptr) 
	{
		RetractAsega(proh->pasega);
		proh->pasega = nullptr;
	}

	if (proh->psma != nullptr) 
	{
		RetractSma(proh->psma);
		proh->psma = nullptr;
	}

	const ROHS previousState = proh->rohs;
	switch (previousState) 
	{
		case ROHS_Spawn:
		SpawnedRobRoh(proh->prob, proh);
		break;

		case ROHS_Return:
		if (rohs == ROHS_Exit) 
			ReturnedRobRoh(proh->prob, proh);
		break;

		case ROHS_Exit:
		if (rohs == ROHS_Inactive) 
			ExitedRobRoh(proh->prob, proh);
		break;

		case ROHS_Die:
		KilledRobRoh(proh->prob, proh);
		break;

		default:
		break;
	}

	proh->rohs = rohs;
	proh->tRohs = g_clock.t;

	ASEG* paseg = nullptr;
	SM* psm = nullptr;
	OVR aovr[1]{};
	int covr = 0;

	switch (rohs) 
	{
		case ROHS_Spawn:
		case ROHS_Wander:
		case ROHS_Collect:
		paseg = proh->pasegMove;
		break;

		case ROHS_Grab:
		GrabbedRobRoh(proh->prob, proh);
		aovr[0].oid = (OID)672;
		aovr[0].palo = proh->proc;
		covr = 1;
		paseg = proh->pasegaGrab;
		break;

		case ROHS_Return:
		case ROHS_Exit:
		aovr[0].oid = (OID)672;
		aovr[0].palo = proh->proc;
		covr = 1;
		paseg = proh->pasegMoveCollect;
		break;

		case ROHS_Happy:
		psm = proh->psmHappy;
		break;

		case ROHS_Sad:
		psm = proh->psmSad;
		break;

		case ROHS_Die:
		paseg = proh->pasegDie;
		break;

		default:
		break;
	}

	if (paseg != nullptr)
		ApplyAsegOvr(paseg, proh, covr, aovr, 0.0f, 1.0f, 1, &proh->pasega);

	if (psm != nullptr) 
		proh->psma = PsmaApplySm(psm, proh, OID_Nil, 1);
}

int FAbsorbRohWkr(ROH* proh, WKR* pwkr)
{
	if ((pwkr->grfic & 32U) != 0) {
		switch (proh->rohs) 
		{
			case ROHS_Spawn:
			case ROHS_Wander:
			case ROHS_Collect:
			case ROHS_Grab:
			case ROHS_Return:
			case ROHS_Exit:
			case ROHS_Happy:
			case ROHS_Sad:
			SetRohRohs(proh, ROHS_Die);
			break;

			default:
			break;
		}

		if (proh->rohs == ROHS_Die) 
		{
			EXPL* pexpl = proh->pexpl;

			if (pexpl != nullptr) {
				EXPLSO explso{};

				explso.posOrigin = pwkr->pos;
				explso.vec = pwkr->v;
				explso.vec.z += 3000.0f;
				explso.grfexplso = 6;

				pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
			}

			StartSound((SFXID)55, nullptr, nullptr, &pwkr->pos, 12000.0f, 8000.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
		}
	}

	return pwkr->grfic != 0;
}

ROC* ProcContactRoh(ROH* proh)
{
	for (OX* contact = proh->poxa->pox; contact != nullptr; contact = contact->poxNext) {
		if (contact->pxp == nullptr) {
			continue;
		}

		if (FIsBasicDerivedFrom(contact->psoOther, CID_ROC) != 0) {
			return static_cast<ROC*>(contact->psoOther);
		}
	}

	return nullptr;
}

void DeleteRoh(ROH *proh)
{
	delete proh;
}

ROC* NewRoc()
{
	return new ROC{};
}

void InitRoc(ROC* proc)
{
	InitSo(proc);
	SetSoConstraints(proc, CT_Project, &g_normalZ, CT_Locked, nullptr);

	proc->clqBubble.g0 = 1.0;
	proc->dtRocsHitUp = 0.2;
	proc->cHitMax = 8;
	proc->rocs = ROCS_Nil;
	proc->dtRocsHitDown = 1.0;
	proc->clqBubble.g2 = 0.0;
	proc->clqBubble.g1 = 0.0;
}

void SetRocCHitMax(ROC* proc, int cHitMax)
{
	proc->cHitMax = cHitMax;
}

void SetRocDtRocsHitUp(ROC* proc, float dtRocsHitUp)
{
	proc->dtRocsHitUp = dtRocsHitUp;
}

void* GetRocDtRocsHitUp(ROC* proc)
{
	return &proc->dtRocsHitUp;
}

void SetRocDtRocsHitDown(ROC* proc, float dtRocsHitDown)
{
	proc->dtRocsHitDown = dtRocsHitDown;
}

void* GetRocDtRocsHitDown(ROC* proc)
{
	return &proc->dtRocsHitDown;
}

void SetRocClqBubble(ROC* proc, CLQ clqBubble)
{
	proc->clqBubble = clqBubble;
}

void* GetRocClqBubble(ROC* proc)
{
	return &proc->clqBubble;
}

int GetRocSize()
{
	return sizeof(ROC);
}

void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(proc, pbis);
	SnipAloObjects(proc, 1, &s_asnipLoadRoc);
	InferExpl(&proc->pexpl, proc);
}

void CloneRoc(ROC* proc, ROC* procBase)
{
	DLE savedDle = proc->dleRob;

	CloneSo(proc, procBase); // Assuming CloneSo handles copying the base class (SO) members

	// Shallow copy of value members
	proc->rocs = procBase->rocs;  // Copying the ROCS object
	proc->tRocs = procBase->tRocs;
	proc->cHitMax = procBase->cHitMax;
	proc->dtRocsHitDown = procBase->dtRocsHitDown;
	proc->dtRocsHitUp = procBase->dtRocsHitUp;

	// Shallow copy of the pointer members (simply copy the pointer, not the data)
	proc->prob = procBase->prob;
	proc->proh = procBase->proh;
	proc->psm = procBase->psm;
	proc->psma = procBase->psma;
	proc->pexpl = procBase->pexpl;
	proc->pemitterBubble = procBase->pemitterBubble;

	// Shallow copy of the LM and CLQ objects (assuming they are structures)
	proc->lmSvcBubble = procBase->lmSvcBubble;
	proc->clqBubble = procBase->clqBubble;

	// Shallow copy of the DLE (linked list)
	proc->dleRob = procBase->dleRob;

	proc->dleRob = savedDle;
}

void PostRocLoad(ROC* proc)
{
	PostAloLoad(proc);
	SnipAloObjects(proc, 1, &s_asnipPostRocLoad);

	if (proc->pemitterBubble != nullptr)
		proc->lmSvcBubble = proc->pemitterBubble->lmSvcParticle;

	if (proc->psm != nullptr)
		proc->psma = PsmaApplySm(proc->psm, proc, OID_Nil, 0);

	SetRocRocs(proc, ROCS_Rest);
	proc->pvtlo->pfnRemoveLo(proc);
}

void UpdateRoc(ROC* proc, float dt)
{
	UpdateSo(proc, dt);

	for (;;) {
		const ROCS nextState = RocsNextRoc(proc);

		if (nextState == proc->rocs) {
			break;
		}

		SetRocRocs(proc, nextState);
	}

	EMITTER* emitter = proc->pemitterBubble;

	if (emitter == nullptr) {
		return;
	}

	const float downwardSpeed = -proc->xf.v.z;

	if (downwardSpeed <= 0.0001f) {
		PauseEmitterIndefinite(emitter);
		return;
	}

	UnpauseEmitter(emitter);

	const float bubbleCurve = proc->clqBubble.g0 + downwardSpeed * (proc->clqBubble.g1 + downwardSpeed * proc->clqBubble.g2);
	const float bubbleScale = std::clamp(bubbleCurve, g_lmZeroOne.gMin, g_lmZeroOne.gMax);

	emitter->lmSvcParticle.gMin = proc->lmSvcBubble.gMin * bubbleScale;
	emitter->lmSvcParticle.gMax = proc->lmSvcBubble.gMax * bubbleScale;
}

void PresetRocAccel(ROC* proc, float dt)
{
	PresetSoAccel(proc, dt);

	if (proc->paloParent != nullptr) {
		return;
	}

	glm::mat3 uprightMatrix;
	TiltMatUpright(&proc->xf.matWorld, nullptr, &uprightMatrix);
	AccelSoTowardMatSmooth(proc, g_clock.dt, &uprightMatrix, &s_smpRocSpin);
}

void AdjustRocNewXp(ROC* proc, XP* pxp, int ixpd)
{
	pxp->elas = pxp->elas * 0.6;
}

int FAbsorbRocWkr(ROC* proc, WKR* pwkr)
{
	if ((pwkr->grfic & 16U) == 0)
		return pwkr->grfic != 0;

	if (proc->paloParent != nullptr || proc->rocs == ROCS_Destroyed)
		return pwkr->grfic != 0;

	if (g_clock.t < proc->tRocs + proc->dtRocsHitUp)
		return pwkr->grfic != 0;

	const ROCS nextHitState = static_cast<ROCS>(static_cast<int>(proc->rocs) + static_cast<int>(ROCS_Hit1));

	if (static_cast<int>(nextHitState) >= proc->cHitMax) 
	{
		SetRocRocs(proc, ROCS_Destroyed);

		EXPL* pexpl = proc->pexpl;

		if (pexpl != nullptr) {
			EXPLSO explso{};

			explso.posOrigin = pwkr->pos;
			explso.vec = pwkr->v;
			explso.vec.z += 1000.0f;
			explso.grfexplso = 6;

			pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
		}

		StartSound((SFXID)35, nullptr, nullptr, &pwkr->pos, 12000.0f, 8000.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
	}
	else 
	{
		SetRocRocs(proc, nextHitState);
		StartSound((SFXID)335, nullptr, nullptr, &pwkr->pos, 12000.0f, 8000.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
	}

	return pwkr->grfic != 0;
}

ROCS RocsNextRoc(ROC* proc)
{
	const ROCS currentState = proc->rocs;

	if (currentState == ROCS_Destroyed)
		return ROCS_Rest;

	if (currentState <= ROCS_Rest || currentState >= ROCS_Destroyed)
		return currentState;

	if (g_clock.t - proc->tRocs <= proc->dtRocsHitDown)
		return currentState;

	return static_cast<ROCS>(static_cast<int>(currentState) + static_cast<int>(ROCS_Nil));
}

void SetRocRocs(ROC* proc, ROCS rocs)
{
	const ROCS previousState = proc->rocs;

	if (previousState == rocs)
		return;

	if (previousState == ROCS_Destroyed)
		DestroyedRobRoc(proc->prob, proc);

	proc->rocs = rocs;
	proc->tRocs = g_clock.t;

	if (proc->psma == nullptr) {
		return;
	}

	const OID stateOid = (OID)s_mprocsoidState[static_cast<int>(rocs)];

	if (previousState == ROCS_Nil) 
		SeekSma(proc->psma, stateOid);
	else
		SetSmaGoal(proc->psma, stateOid);
}

void DeleteRoc(ROC *proc)
{
	delete proc;
}

ROST* NewRost()
{
	return new ROST{};
}

void InitRost(ROST* prost)
{
	InitSo(prost);

	prost->rosts = ROSTS_Nil;
}

int GetRostSize()
{
	return sizeof(ROST);
}

void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prost, pbis);
	SnipAloObjects(prost, 1, &s_asnipLoadRost);
}

void CloneRost(ROST* prost, ROST* prostBase)
{
	DLE savedDle = prost->dleRob;

	CloneSo(prost, prostBase); // Assuming CloneSo handles copying the base class (SO) members

	// Shallow copy of value members
	prost->rosts = prostBase->rosts;  // Copying the ROSTS object
	prost->tRosts = prostBase->tRosts;

	// Shallow copy of pointer members (simply copy the pointer, not the data)
	prost->prob = prostBase->prob;
	prost->psm = prostBase->psm;
	prost->psma = prostBase->psma;
	prost->ppnt = prostBase->ppnt;

	// Shallow copy of the DLE (linked list)
	prost->dleRob = prostBase->dleRob;

	prost->dleRob = savedDle;
}

void PostRostLoad(ROST* prost)
{
	PostAloLoad(prost);

	if (prost->psm != nullptr)
		prost->psma = PsmaApplySm(prost->psm, prost, (OID)-1, 1);

	LO* pchild;

	for (pchild = prost->dlChild.ploFirst; pchild != nullptr; pchild = pchild->dleChild.ploNext) {
		if (FIsBasicDerivedFrom(pchild, CID_PNT)) 
		{
			prost->ppnt = (PNT*)pchild;
			break;
		}
	}

	SetRostRosts(prost, ROSTS_Close);
}

void UpdateRost(ROST* prost, float dt)
{
	ROSTS rostsNext;

	UpdateSo(prost, dt);

	while ((rostsNext = RostsNextRost(prost)) != prost->rosts)
		SetRostRosts(prost, rostsNext);
}

ROSTS RostsNextRost(ROST* prost)
{
	return prost->rosts;
}

void SetRostRosts(ROST* prost, ROSTS rosts)
{
	ROSTS rostsPrev = prost->rosts;

	if (rostsPrev == rosts) {
		return;
	}

	prost->rosts = rosts;
	prost->tRosts = g_clock.t;

	if (prost->psma != nullptr) 
	{
		if (rostsPrev == ROSTS_Nil)
			SeekSma(prost->psma, (OID)s_aoidStateNext[rosts]);
		else 
			SetSmaGoal(prost->psma, (OID)s_aoidStateNext[rosts]);
	}
}

void DeleteRost(ROST *prost)
{
	delete prost;
}

ROP* NewRop()
{
	return new ROP{};
}

void InitRop(ROP* prop)
{
	InitSo(prop);
	prop->rops = ROPS_Nil;
	SetSoConstraints(prop, CT_Locked, nullptr, CT_Locked, nullptr);
}

int GetRopSize()
{
	return sizeof(ROP);
}

void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prop, pbis);

	SnipAloObjects(prop, 1, &s_asnipLoadRop);
	InferExpl(&prop->pexpl, prop);
}

void CloneRop(ROP* prop, ROP* probBase)
{
	CloneSo(prop, probBase); // Assuming CloneSo handles copying the base class (SO) members

	// Shallow copy of value members
	prop->rops = probBase->rops;  // Copying the ROPS object
	prop->tRops = probBase->tRops;

	// Shallow copy of pointer members (simply copy the pointer, not the data)
	prop->prob = probBase->prob;
	prop->pkey = probBase->pkey;
	prop->pasegReveal = probBase->pasegReveal;
	prop->pasega = probBase->pasega;
	prop->pexpl = probBase->pexpl;
}

void PostRopLoad(ROP* prop)
{
	PostAloLoad(prop);

	if (prop->prob == nullptr) 
	{
		prop->pvtlo->pfnRemoveLo(prop);
		return;
	}

	prop->pkey = (KEY*)PloFindSwObjectByClass(prop->psw, 257, CID_KEY, prop->dlChild.ploFirst);
	SetRopRops(prop, ROPS_StayPut);
}

void UpdateRop(ROP* prop, float dt)
{
	ROPS ropsNext;

	UpdateSo(prop, dt);

	while ((ropsNext = RopsNextRop(prop)) != prop->rops) {
		SetRopRops(prop, ropsNext);
	}

	if (prop->rops != ROPS_Collect)
		return;

	glm::vec3 dpos = glm::vec3(prop->prob->prov->xf.posWorld) - glm::vec3(prop->pkey->xf.posWorld);

	if (glm::length(dpos) < 0.0001f) {
		dpos = glm::vec3(g_normalX);
	}
	else {
		dpos = glm::normalize(dpos);
	}

	SMP smp;
	smp.svFast = 3000.0f;
	smp.svSlow = 0.0f;
	smp.dtFast = 0.2f;

	float speed = glm::length(glm::vec3(prop->pkey->xf.v));
	speed = GSmooth(speed, 3000.0f, g_clock.dt, &smp, NULL);

	glm::vec3 velocity = glm::vec4(dpos * speed, 0.0f);
	glm::mat3 mat;

	prop->pkey->pvtalo->pfnSetAloVelocityVec(prop->pkey, &velocity);
	LoadLookAtMatrix(&dpos, &mat);
	prop->pkey->pvtalo->pfnRotateAloToMat(prop->pkey, &mat);
}

void SetRopRops(ROP* prop, ROPS rops)
{
	if (prop->rops == rops) {
		return;
	}

	if (prop->pasega != nullptr) {
		RetractAsega(prop->pasega);
		prop->pasega = nullptr;
	}

	prop->rops = rops;
	prop->tRops = g_clock.t;

	ASEG* paseg = nullptr;

	if (rops == ROPS_Reveal) {
		paseg = prop->pasegReveal;

		if (prop->pexpl != nullptr) {
			EXPLSO explso{};
			explso.posOrigin = prop->xf.posWorld;
			explso.grfexplso = 4;

			prop->pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)prop->pexpl, &explso);
		}
	}
	else if (rops == ROPS_Collect) {
		prop->pkey->pvtlo->pfnSetLoParent(prop->pkey, nullptr);
		prop->pkey->pvtkey->pfnSetKeyDprizes(prop->pkey, DPRIZES_Collect);
	}

	if (paseg != nullptr)
		ApplyAseg(paseg, prop, 0.0f, 1.0f, 0, &prop->pasega);
}

ROPS RopsNextRop(ROP* prop)
{
	ROPS rops = prop->rops;

	switch (rops) 
	{
		case ROPS_Reveal:
		if (prop->pasega == nullptr || prop->pasega->tLocal >= prop->pasega->paseg->tMax)
			rops = ROPS_Collect;
		break;

		case ROPS_Collect:
		break;

		default:
		return rops;
	}

	if ((g_plsCur->grfls & 2U) != 0) 
		rops = ROPS_Collected;

	return rops;
}

void DeleteRop(ROP *prop)
{
	delete prop;
}

ROB* NewRob()
{
	return new ROB{};
}

void InitRob(ROB* prob)
{
	InitAlo(prob);

	InitDl(&prob->dlRocOwned, offsetof(ROC, dleRob));
	InitDl(&prob->dlRocUnowned, offsetof(ROC, dleRob));
	InitDl(&prob->dlRocInactive, offsetof(ROC, dleRob));
	InitDl(&prob->dlRohActive, offsetof(ROH, dleRob));
	InitDl(&prob->dlRohInactive, offsetof(ROH, dleRob));
	InitDl(&prob->dlRostAvailable, offsetof(ROST, dleRob));
	InitDl(&prob->dlRostUnavailable, offsetof(ROST, dleRob));

	prob->crocCreateMax = 40;

	prob->robs = ROBS_Nil;
	prob->oidCollectible = OID_Nil;
	prob->oidHarvester = OID_Nil;
	prob->oidVehicle = OID_Nil;
	prob->oidPrize = OID_Nil;
	prob->robk = ROBK_Nil;

	RODD& rodd = prob->arodd[0];

	rodd = {};

	rodd.crocActiveMaxEasy = 2.0f;
	rodd.crocActiveMaxHard = 5.0f;

	rodd.clqCrocActive.g0 = 0.0f;
	rodd.clqCrocActive.g1 = 1.0f;
	rodd.clqCrocActive.g2 = 0.0f;
	rodd.clqCrocActive.gUnused = 0.0f;

	rodd.crohActiveMaxEasy = 0.0f;
	rodd.crohActiveMaxHard = 8.0f;

	rodd.clqCrohActive.g0 = 0.0f;
	rodd.clqCrohActive.g1 = 1.0f;
	rodd.clqCrohActive.g2 = 0.0f;
	rodd.clqCrohActive.gUnused = 0.0f;

	rodd.lmDtAddRocEasy.gMin = 1.0f;
	rodd.lmDtAddRocEasy.gMax = 2.5f;

	rodd.lmDtAddRocHard.gMin = 0.1f;
	rodd.lmDtAddRocHard.gMax = 0.3f;

	rodd.clqDtAddRoc.g0 = 0.0f;
	rodd.clqDtAddRoc.g1 = 1.0f;
	rodd.clqDtAddRoc.g2 = 0.0f;
	rodd.clqDtAddRoc.gUnused = 0.0f;

	rodd.lmDtAddRohEasy.gMin = 1.5f;
	rodd.lmDtAddRohEasy.gMax = 3.0f;

	rodd.lmDtAddRohHard.gMin = 0.15f;
	rodd.lmDtAddRohHard.gMax = 0.4f;

	rodd.clqDtAddRoh.g0 = 0.0f;
	rodd.clqDtAddRoh.g1 = 1.0f;
	rodd.clqDtAddRoh.g2 = 0.0f;
	rodd.clqDtAddRoh.gUnused = 0.0f;

	rodd.svRohMaxEasy = 400.0f;
	rodd.svRohMaxHard = 1500.0f;

	rodd.clqSvRoh.g0 = 0.0f;
	rodd.clqSvRoh.g1 = 1.0f;
	rodd.clqSvRoh.g2 = 0.0f;
	rodd.clqSvRoh.gUnused = 0.0f;

	rodd.dradRohMaxEasy = 3.0f;
	rodd.dradRohMaxHard = 6.0f;

	rodd.clqDradRoh.g0 = 0.0f;
	rodd.clqDradRoh.g1 = 1.0f;
	rodd.clqDradRoh.g2 = 0.0f;
	rodd.clqDradRoh.gUnused = 0.0f;

	std::memmove(&prob->arodd[1], &prob->arodd[0], sizeof(RODD));

	prob->clqTune = {};

	prob->clqTune.g0 = 0.0f;
	prob->clqTune.g1 = 1.0f;
	prob->clqTune.g2 = 0.0f;
	prob->clqTune.gUnused = 0.0f;

	prob->sCollectibleDropHeight = 3000.0;
	prob->sRadiusArenaWander = 2300.0;
	prob->sRadiusArenaRoc = 1500.0;
	prob->rAspectArena = 1.46;
	prob->sRadiusArenaTube = 0.0;
}

float* ProddCurRobCrocActiveMaxEasy(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->crocActiveMaxEasy;
}

float* ProddCurRobCrocActiveMaxHard(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->crocActiveMaxHard;
}

CLQ* ProddCurRobClqCrocActive(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->clqCrocActive;
}

float* ProddCurRobCrohActiveMaxEasy(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->crohActiveMaxEasy;
}

float* ProddCurRobCrohActiveMaxHard(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->crohActiveMaxHard;
}

CLQ* ProddCurRobClqCrohActive(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->clqCrohActive;
}

LM* ProddCurRobLmDtAddRocEasy(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->lmDtAddRocEasy;
}

LM* ProddCurRobLmDtAddRocHard(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->lmDtAddRocHard;
}

CLQ* ProddCurRobClqDtAddRoc(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->clqDtAddRoc;
}

LM* ProddCurRobLmDtAddRohEasy(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->lmDtAddRohEasy;
}

LM* ProddCurRobLmDtAddRohHard(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->lmDtAddRohHard;
}

CLQ* ProddCurRobClqDtAddRoh(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->clqDtAddRoh;
}

float* ProddCurRobSvRohMaxEasy(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->svRohMaxEasy;
}

float* ProddCurRobSvRohMaxHard(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->svRohMaxHard;
}

CLQ* ProddCurRobClqSvRoh(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->clqSvRoh;
}

float* ProddCurRobDradRohMaxEasy(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->dradRohMaxEasy;
}

float* ProddCurRobDradRohMaxHard(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->dradRohMaxHard;
}

CLQ* ProddCurRobClqDradRoh(ROB* prob, ENSK ensk)
{
	return &ProddCurRob(prob, ensk)->clqDradRoh;
}

void SetRobPosCenterArena(ROB* prob, glm::vec3 posCenterArena)
{
	prob->posCenterArena = posCenterArena;
}

void* GetRobPosCenterArena(ROB* prob)
{
	return &prob->posCenterArena;
}

void SetRobSRadiusArenaTube(ROB* prob, float sRadiusArenaTube)
{
	prob->sRadiusArenaTube = sRadiusArenaTube;
}

void* GetRobSRadiusArenaTube(ROB* prob)
{
	return &prob->sRadiusArenaTube;
}

void SetRobSRadiusArenaRoc(ROB* prob, float aRadiusArenaRoc)
{
	prob->sRadiusArenaRoc = aRadiusArenaRoc;
}

void* GetRobSRadiusArenaRoc(ROB* prob)
{
	return &prob->sRadiusArenaRoc;
}

void SetRobSRadiusArenaWander(ROB* prob, float sRadiusArenaWander)
{
	prob->sRadiusArenaWander = sRadiusArenaWander;
}

void* GetRobSRadiusArenaWander(ROB* prob)
{
	return &prob->sRadiusArenaWander;
}

void SetRobRAspectArena(ROB* prob, float rAspectArena)
{
	prob->rAspectArena = rAspectArena;
}

void* GetRobRAspectArena(ROB* prob)
{
	return &prob->rAspectArena;
}

void SetRobSCollectibleDropHeight(ROB* prob, float sCollectibleDropHeight)
{
	prob->sCollectibleDropHeight = sCollectibleDropHeight;
}

void* GetRobSCollectibleDropHeight(ROB* prob)
{
	return &prob->sCollectibleDropHeight;
}

void SetRobCrocCreateMax(ROB* prob, int crocCreateMax)
{
	prob->crocCreateMax = crocCreateMax;
}

void* GetRobCrocCreateMax(ROB* prob)
{
	return &prob->crocCreateMax;
}

void SetRobRobk(ROB* prob, ROBK robk)
{
	prob->robk = robk;
}

void* GetRobRobk(ROB* prob)
{
	return &prob->robk;
}

void SetRobIrodd(ROB* prob, int irodd)
{
	prob->irodd = irodd;
}

void* GetRobIrodd(ROB* prob)
{
	return &prob->irodd;
}

void SetRobClqTune(ROB* prob, glm::vec4 clqTune)
{
	prob->clqTune.g0 = clqTune.x;
	prob->clqTune.g1 = clqTune.y;
	prob->clqTune.g2 = clqTune.z;
	//prob->clqTune.g3 = clqTune.w;
}

void* GetRobClqTune(ROB* prob)
{
	return &prob->clqTune;
}

int GetRobSize()
{
	return sizeof(ROB);
}

void SetRobCollectible(ROB* prob, OID oidCollectible)

{
	prob->oidCollectible = oidCollectible;
	return;
}

void SetRobHarvester(ROB* prob, OID oidHarvester)

{
	prob->oidHarvester = oidHarvester;
	return;
}


void SetRobVehicle(ROB* prob, OID oidVehicle)

{
	prob->oidVehicle = oidVehicle;
	return;
}

void SetRobPrize(ROB* prob, OID oidPrize)

{
	prob->oidPrize = oidPrize;
	return;
}

void AddRobSpawnTunnel(ROB* prob, OID oidSpawnTunnel)
{
	prob->aoidRost[prob->coidRost++] = oidSpawnTunnel;
}

ROBK RobkCur()
{
	if ((g_plsCur->grfls & 4U) != 0) {
		return ROBK_Tertiary;
	}
	return (ROBK)((g_plsCur->grfls & 2U) != 0);
}

void CloneRob(ROB* prob, ROB* probBase)
{
	CloneAlo(prob, probBase); // Assuming CloneAlo handles copying the base class (ALO) members

	// Shallow copy of value members
	prob->oidCollectible = probBase->oidCollectible;
	prob->oidHarvester = probBase->oidHarvester;
	prob->oidVehicle = probBase->oidVehicle;
	prob->oidPrize = probBase->oidPrize;
	prob->coidRost = probBase->coidRost;
	prob->posCenterArena = probBase->posCenterArena;
	prob->sRadiusArenaTube = probBase->sRadiusArenaTube;
	prob->sRadiusArenaRoc = probBase->sRadiusArenaRoc;
	prob->sRadiusArenaWander = probBase->sRadiusArenaWander;
	prob->rAspectArena = probBase->rAspectArena;
	prob->sCollectibleDropHeight = probBase->sCollectibleDropHeight;
	prob->crocActive = probBase->crocActive;
	prob->crocCreateMax = probBase->crocCreateMax;
	prob->crocCreated = probBase->crocCreated;
	prob->crocReturned = probBase->crocReturned;
	prob->crocDestroyed = probBase->crocDestroyed;
	prob->irodd = probBase->irodd;
	prob->crocActiveMax = probBase->crocActiveMax;
	prob->crohActiveMax = probBase->crohActiveMax;
	prob->tAddRoc = probBase->tAddRoc;
	prob->tAddRoh = probBase->tAddRoh;
	prob->svRohMax = probBase->svRohMax;
	prob->dradRohMax = probBase->dradRohMax;
	prob->tRobs = probBase->tRobs;

	// Shallow copy of pointer members (simply copy the pointers)
	prob->prov = probBase->prov;
	prob->prop = probBase->prop;
	prob->dlRocOwned = probBase->dlRocOwned;
	prob->dlRocUnowned = probBase->dlRocUnowned;
	prob->dlRocInactive = probBase->dlRocInactive;
	prob->dlRohActive = probBase->dlRohActive;
	prob->dlRohInactive = probBase->dlRohInactive;
	prob->dlRostAvailable = probBase->dlRostAvailable;
	prob->dlRostUnavailable = probBase->dlRostUnavailable;
	prob->clqTune = probBase->clqTune;
	prob->rodd = probBase->rodd;
	prob->pdialog = probBase->pdialog;
	// Assuming the arrays like arodd and aoidRost are shallow-copied by reference
	std::copy(std::begin(probBase->arodd), std::end(probBase->arodd), std::begin(prob->arodd));
	std::copy(std::begin(probBase->aoidRost), std::end(probBase->aoidRost), std::begin(prob->aoidRost));
}

void BindRob(ROB* prob)
{
	BindAlo(prob);

	if (prob->robk != RobkCur())
		return;

	prob->prov = reinterpret_cast<ROV*>(PloFindSwNearest(g_psw, (OID)prob->oidVehicle, prob));

	if (prob->prov != nullptr)
		prob->prov->prob = prob;

	ROC* procTemplate = reinterpret_cast<ROC*>(PloFindSwNearest(g_psw, (OID)prob->oidCollectible, prob));
	ROH* prohTemplate = reinterpret_cast<ROH*>(PloFindSwNearest(g_psw, (OID)prob->oidHarvester, prob));

	if (procTemplate != nullptr)
		procTemplate->prob = prob;

	if (prohTemplate != nullptr)
		prohTemplate->prob = prob;

	if (prob->oidPrize != OID_Nil)
	{
		prob->prop = reinterpret_cast<ROP*>(PloFindSwNearest(g_psw, (OID)prob->oidPrize, prob));

		if (prob->prop != nullptr)
			prob->prop->prob = prob;
	}

	const float crocInactiveMax = std::max(prob->arodd[0].crocActiveMaxHard, prob->arodd[1].crocActiveMaxHard);

	if (procTemplate != nullptr)
	{
		for (int iroc = 0; static_cast<float>(iroc) < crocInactiveMax; ++iroc)
		{
			ROC* proc = procTemplate;

			if (iroc != 0)
				proc = reinterpret_cast<ROC*>(PloCloneLo(procTemplate, g_psw, procTemplate->paloParent));

			if (proc != nullptr)
				AppendDlEntry(&prob->dlRocInactive, proc);
		}
	}

	const float crohInactiveMax = std::max(prob->arodd[0].crohActiveMaxHard, prob->arodd[1].crohActiveMaxHard);

	if (prohTemplate != nullptr)
	{
		for (int iroh = 0; static_cast<float>(iroh) < crohInactiveMax; ++iroh)
		{
			ROH* proh = prohTemplate;

			if (iroh != 0)
				proh = reinterpret_cast<ROH*>(PloCloneLo(prohTemplate, g_psw, prohTemplate->paloParent));

			if (proh != nullptr)
				AppendDlEntry(&prob->dlRohInactive, proh);
		}
	}

	for (int irost = 0; irost < prob->coidRost; ++irost)
	{
		LO* aplo[2]{};
		const int cplo = CploFindSwObjects(g_psw, 0x104, (OID)prob->aoidRost[irost], prob, 2, aplo);

		ROST* prost = nullptr;

		for (int iplo = 0; iplo < cplo; ++iplo)
		{
			if (FIsBasicDerivedFrom(aplo[iplo], CID_ROST) != 0)
			{
				prost = reinterpret_cast<ROST*>(aplo[iplo]);
				break;
			}
		}

		if (prost == nullptr)
			continue;

		prost->prob = prob;
		AppendDlEntry(&prob->dlRostAvailable, prost);
	}
}

void PostRobLoad(ROB* prob)
{
	PostAloLoad(prob);

	if (prob->robk != RobkCur())
	{
		if (prob->pvtlo->pfnRemoveLo != nullptr)
			prob->pvtlo->pfnRemoveLo(prob);

		return;
	}

	const float uTune = glm::clamp(prob->clqTune.g0 + g_plsCur->uSuck * (prob->clqTune.g1 + g_plsCur->uSuck * prob->clqTune.g2), 0.0f, 1.0f);


	auto LerpFloat = [uTune](float a, float b)
	{
		return a + (b - a) * uTune;
	};

	auto LerpLm = [&](LM& dst, const LM& a, const LM& b)
	{
		dst.gMin = LerpFloat(a.gMin, b.gMin);
		dst.gMax = LerpFloat(a.gMax, b.gMax);
	};

	auto LerpClq = [&](CLQ& dst, const CLQ& a, const CLQ& b)
	{
		dst.g0 = LerpFloat(a.g0, b.g0);
		dst.g1 = LerpFloat(a.g1, b.g1);
		dst.g2 = LerpFloat(a.g2, b.g2);
		dst.gUnused = LerpFloat(a.gUnused, b.gUnused);
	};

	const RODD& roddA = prob->arodd[0];
	const RODD& roddB = prob->arodd[1];
	RODD& rodd = prob->rodd;

	rodd = {};

	rodd.crocActiveMaxEasy = LerpFloat(roddA.crocActiveMaxEasy, roddB.crocActiveMaxEasy);
	rodd.crocActiveMaxHard = LerpFloat(roddA.crocActiveMaxHard, roddB.crocActiveMaxHard);
	LerpClq(rodd.clqCrocActive, roddA.clqCrocActive, roddB.clqCrocActive);

	rodd.crohActiveMaxEasy = LerpFloat(roddA.crohActiveMaxEasy, roddB.crohActiveMaxEasy);
	rodd.crohActiveMaxHard = LerpFloat(roddA.crohActiveMaxHard, roddB.crohActiveMaxHard);
	LerpClq(rodd.clqCrohActive, roddA.clqCrohActive, roddB.clqCrohActive);

	LerpLm(rodd.lmDtAddRocEasy, roddA.lmDtAddRocEasy, roddB.lmDtAddRocEasy);
	LerpLm(rodd.lmDtAddRocHard, roddA.lmDtAddRocHard, roddB.lmDtAddRocHard);
	LerpClq(rodd.clqDtAddRoc, roddA.clqDtAddRoc, roddB.clqDtAddRoc);

	LerpLm(rodd.lmDtAddRohEasy, roddA.lmDtAddRohEasy, roddB.lmDtAddRohEasy);
	LerpLm(rodd.lmDtAddRohHard, roddA.lmDtAddRohHard, roddB.lmDtAddRohHard);
	LerpClq(rodd.clqDtAddRoh, roddA.clqDtAddRoh, roddB.clqDtAddRoh);

	rodd.svRohMaxEasy = LerpFloat(roddA.svRohMaxEasy, roddB.svRohMaxEasy);
	rodd.svRohMaxHard = LerpFloat(roddA.svRohMaxHard, roddB.svRohMaxHard);
	LerpClq(rodd.clqSvRoh, roddA.clqSvRoh, roddB.clqSvRoh);

	rodd.dradRohMaxEasy = LerpFloat(roddA.dradRohMaxEasy, roddB.dradRohMaxEasy);
	rodd.dradRohMaxHard = LerpFloat(roddA.dradRohMaxHard, roddB.dradRohMaxHard);
	LerpClq(rodd.clqDradRoh, roddA.clqDradRoh, roddB.clqDradRoh);

	AdjustRobDifficulty(prob, 0.0f);
	SetRobRobs(prob, ROBS_Descend);
}

void UpdateRob(ROB* prob, float dt)
{
	UpdateAlo(prob, dt);

	ROBS robs;

	while ((robs = RobsNextRob(prob)) != prob->robs)
		SetRobRobs(prob, robs);

	switch (robs)
	{
		case ROBS_Normal:
		if (prob->crocCreated < prob->crocCreateMax && prob->tAddRoc < g_clock.t)
			AddRobRoc(prob);

		if (prob->tAddRoh < g_clock.t)
			AddRobRoh(prob);

		break;

		case ROBS_Ascend:
		if (prob->prop == nullptr)
			break;

		if (prob->prop->rops != ROPS_StayPut)
			break;

		if (prob->pdialog != nullptr && prob->pdialog->dialogs == DIALOGS_Playing)
			break;

		SetRopRops(prob->prop, ROPS_Reveal);
		break;

		default:
		break;
	}
}

void UpdateRobBlot(ROB* prob)
{
	const bool fShowGoldCounter = prob->robs == ROBS_Normal && FCanShowGameplayBlot();

	if (fShowGoldCounter)
		ShowBlot((BLOT*)&g_goldctr);
	else
		HideBlot((BLOT*)&g_goldctr);
}

ROBS RobsNextRob(ROB* prob)
{
	ROBS robs = prob->robs;

	switch (robs)
	{
		case ROBS_Descend:
		if (static_cast<unsigned int>(prob->prov->rovs) < 2U)
			robs = ROBS_Instruct;

		break;

		case ROBS_Instruct:
		if (prob->pdialog == nullptr || prob->pdialog->dialogs != DIALOGS_Playing)
			robs = ROBS_Normal;

		break;

		case ROBS_Normal:
		if (prob->crocReturned != 0)
			robs = ROBS_Lose;
		else if (prob->crocDestroyed == prob->crocCreateMax)
			robs = ROBS_Ascend;

		break;

		case ROBS_Lose:
		if (prob->pdialog == nullptr || prob->pdialog->dialogs != DIALOGS_Playing)
			robs = ROBS_Win;

		break;

		case ROBS_Ascend:
		if (prob->prop == nullptr)
		{
			const bool delayFinished = prob->tRobs + 5.0f < g_clock.t;
			const bool dialogFinished = prob->pdialog == nullptr || prob->pdialog->dialogs != DIALOGS_Playing;

			if (delayFinished && dialogFinished)
				robs = ROBS_Max;
		}
		else if (prob->prop->rops == ROPS_Collected && prob->prop->tRops + 3.0f < g_clock.t)
			robs = ROBS_Max;

		break;

		default:
		break;
	}

	return robs;
}

void SetRobRobs(ROB* prob, ROBS robs)
{
	if (prob->robs == robs)
		return;

	prob->robs = robs;
	prob->tRobs = g_clock.t;
	prob->pdialog = nullptr;

	switch (robs)
	{
		case ROBS_Nil:
		break;

		case ROBS_Descend:
		break;

		case ROBS_Instruct:
		{
			if (prob->irodd >= 0 && prob->irodd < 3 && g_plsCur->afDialogPlayed[0] == 0)
				prob->pdialog = reinterpret_cast<DIALOG*>(PloFindSwObject(prob->psw, 0x105, static_cast<OID>(827), nullptr));

			if (prob->pdialog != nullptr)
			{
				g_binoc.pdialogPlaying = prob->pdialog;
				PushUiActiveBlot(&g_ui, &g_binoc);
			}

			break;
		}

		case ROBS_Normal:
		g_goldctr.pnActual = &prob->crocDestroyed;
		g_goldctr.pnTotal  = &prob->crocCreateMax;
		SetSwGameplayHud(prob->psw, prob);
		break;

		case ROBS_Lose:
		{
			prob->pdialog = reinterpret_cast<DIALOG*>(PloFindSwObject(prob->psw, 0x105, static_cast<OID>(830), nullptr));

			if (prob->pdialog != nullptr)
			{
				g_binoc.pdialogPlaying = prob->pdialog;
				PushUiActiveBlot(&g_ui, &g_binoc);
			}

			for (ROH* proh = prob->dlRohActive.prohFirst; proh != nullptr; proh = proh->dleRob.prohNext)
			{
				if (proh->proc == nullptr || proh->proc->paloParent != proh)
					SetRohRohs(proh, ROHS_Happy);
			}

			OnDifficultyPlayerDeath(&g_difficulty, 1.0f);
			break;
		}

		case ROBS_Win:
		SetPrompt(&g_prompt, PRP_Basic, (PRK)26);
		break;

		case ROBS_Ascend:
		{
			prob->pdialog = reinterpret_cast<DIALOG*>(PloFindSwObject(prob->psw, 0x105, static_cast<OID>(829), nullptr));

			if (prob->pdialog != nullptr)
			{
				g_binoc.pdialogPlaying = prob->pdialog;
				PushUiActiveBlot(&g_ui, &g_binoc);
			}

			for (ROH* proh = prob->dlRohActive.prohFirst; proh != nullptr; proh = proh->dleRob.prohNext)
				SetRohRohs(proh, ROHS_Sad);

			OnDifficultyCollectKey(&g_difficulty);
			break;
		}

		case ROBS_Max:
		if (prob->prov != nullptr)
			SetRovRovs(prob->prov, ROVS_Ascend);

		g_note.pvtblot->pfnHideBlot(&g_note);
		break;

		default:
		break;
	}
}

void AddRobRoc(ROB* prob)
{
	if (prob->crocActive >= prob->crocActiveMax)
	{
		prob->tAddRoc = std::numeric_limits<float>::max();
		return;
	}

	ROC* proc = prob->dlRocInactive.procFirst;

	if (proc == nullptr)
	{
		prob->tAddRoc = std::numeric_limits<float>::max();
		return;
	}

	auto FSpheresOverlap = [](const glm::vec3& posA, float radiusA, const glm::vec3& posB, float radiusB)
	{
		const float radius = radiusA + radiusB;
		return glm::distance2(posA, posB) < radius * radius;
	};

	glm::vec3 pos{};
	bool fFoundPosition = false;

	for (int iTry = 0; iTry < 8; ++iTry)
	{
		const float rad = GRandInRange(0.0f, glm::two_pi<float>());
		const float sXY = GRandInRange(prob->sRadiusArenaTube + proc->sRadiusAll, prob->sRadiusArenaRoc);

		SetVectorCylind(&pos, rad, sXY, 0.0f);

		pos.y *= prob->rAspectArena;
		pos += prob->posCenterArena;

		bool fBlocked = false;

		PO* ppo = PpoCur();

		if (ppo != nullptr && FSpheresOverlap(pos, proc->sRadiusAll, ppo->xf.posWorld, ppo->sRadiusAll))
			fBlocked = true;

		if (!fBlocked)
		{
			for (ROH* proh = prob->dlRohActive.prohFirst; proh != nullptr; proh = proh->dleRob.prohNext)
			{
				if (FSpheresOverlap(pos, proc->sRadiusAll, proh->xf.posWorld, proh->sRadiusAll))
				{
					fBlocked = true;
					break;
				}
			}
		}

		if (!fBlocked)
		{
			for (ROC* procOther = prob->dlRocOwned.procFirst; procOther != nullptr; procOther = procOther->dleRob.procNext)
			{
				if (procOther->paloParent != nullptr)
					continue;

				if (FSpheresOverlap(pos, proc->sRadiusAll, procOther->xf.posWorld, procOther->sRadiusAll))
				{
					fBlocked = true;
					break;
				}
			}
		}

		if (!fBlocked)
		{
			for (ROC* procOther = prob->dlRocUnowned.procFirst; procOther != nullptr; procOther = procOther->dleRob.procNext)
			{
				if (procOther == proc)
					continue;

				if (FSpheresOverlap(pos, proc->sRadiusAll, procOther->xf.posWorld, procOther->sRadiusAll))
				{
					fBlocked = true;
					break;
				}
			}
		}

		if (!fBlocked)
		{
			fFoundPosition = true;
			break;
		}
	}

	if (!fFoundPosition)
		return;

	++prob->crocCreated;
	++prob->crocActive;

	if (proc->pvtlo->pfnAddLo != nullptr)
		proc->pvtlo->pfnAddLo(proc);

	pos.z = prob->sCollectibleDropHeight;

	if (proc->pvtalo->pfnTranslateAloToPos != nullptr)
		proc->pvtalo->pfnTranslateAloToPos(proc, &pos);

	const float rad = GRandInRange(0.0f, glm::two_pi<float>());

	glm::mat3 mat;
	LoadRotateMatrixRad(rad, &g_normalZ, &mat);

	if (proc->pvtalo->pfnRotateAloToMat != nullptr)
		proc->pvtalo->pfnRotateAloToMat(proc, &mat);

	if (proc->pvtalo->pfnSetAloVelocityVec != nullptr)
		proc->pvtalo->pfnSetAloVelocityVec(proc, &g_vecZero);

	if (proc->pvtalo->pfnSetAloAngularVelocityVec != nullptr)
		proc->pvtalo->pfnSetAloAngularVelocityVec(proc, &g_vecZero);

	SetSoConstraints(proc, CT_Project, &g_normalZ, CT_Locked, nullptr);
	SetRocRocs(proc, ROCS_Rest);

	RemoveDlEntry(&prob->dlRocInactive, proc);
	AppendDlEntry(&prob->dlRocUnowned, proc);

	FChooseRobRoh(prob, proc);

	const float dtAdd = GRandInRange(prob->lmDtAddRoc.gMin, prob->lmDtAddRoc.gMax);
	prob->tAddRoc = g_clock.t + dtAdd;
}

void AddRobRoh(ROB* prob)
{
	if (prob->crohActive >= prob->crohActiveMax)
	{
		prob->tAddRoh = std::numeric_limits<float>::max();
		return;
	}

	ROH* proh = prob->dlRohInactive.prohFirst;

	if (proh == nullptr)
		return;

	const int crost = CPvDl(&prob->dlRostAvailable);

	if (crost == 0)
		return;

	int irost = NRandInRange(0, crost - 1);
	ROST* prost = prob->dlRostAvailable.prostFirst;

	while (irost-- > 0 && prost != nullptr)
		prost = prost->dleRob.prostNext;

	if (prost == nullptr)
		return;

	RemoveDlEntry(&prob->dlRostAvailable, prost);
	AppendDlEntry(&prob->dlRostUnavailable, prost);
	RemoveDlEntry(&prob->dlRohInactive, proh);
	AppendDlEntry(&prob->dlRohActive, proh);

	proh->prost = prost;
	++prob->crohActive;

	if (proh->pvtlo->pfnAddLo != nullptr)
		proh->pvtlo->pfnAddLo(proh);
	if (proh->pvtalo->pfnTranslateAloToPos != nullptr)
		proh->pvtalo->pfnTranslateAloToPos(proh, &prost->xf.posWorld);

	if (proh->pvtalo->pfnRotateAloToMat != nullptr)
		proh->pvtalo->pfnRotateAloToMat(proh, &prost->xf.matWorld);
	SetRohRohs(proh, ROHS_Spawn);
	SetRostRosts(prost, ROSTS_Open);

	proh->svMax = prob->svRohMax;
	proh->dradMax = prob->dradRohMax;

	const float dtAdd = GRandInRange(prob->lmDtAddRoh.gMin, prob->lmDtAddRoh.gMax);
	prob->tAddRoh = g_clock.t + dtAdd;

	StartSound(SFXID_Nil, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
	StartSound((SFXID)334, nullptr, prost, nullptr, 12000.0f, 5000.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
}

void AdjustRobDifficulty(ROB* prob, float dt)
{
	auto EvaluateClq = [dt](const CLQ& clq)
	{
		const float value =clq.g0 +dt * (clq.g1 + dt * clq.g2);

		return glm::clamp(value, 0.0f, 1.0f);
	};

	auto Lerp = [](float a, float b, float u)
	{
		return a + (b - a) * u;
	};

	float u = EvaluateClq(prob->rodd.clqDtAddRoc);

	prob->lmDtAddRoc.gMin = Lerp(prob->rodd.lmDtAddRocEasy.gMin, prob->rodd.lmDtAddRocHard.gMin,u);

	prob->lmDtAddRoc.gMax = Lerp(prob->rodd.lmDtAddRocEasy.gMax, prob->rodd.lmDtAddRocHard.gMax,u);

	u = EvaluateClq(prob->rodd.clqDtAddRoh);

	prob->lmDtAddRoh.gMin = Lerp(prob->rodd.lmDtAddRohEasy.gMin, prob->rodd.lmDtAddRohHard.gMin,u);

	prob->lmDtAddRoh.gMax = Lerp(prob->rodd.lmDtAddRohEasy.gMax, prob->rodd.lmDtAddRohHard.gMax, u);

	u = EvaluateClq(prob->rodd.clqSvRoh);

	prob->svRohMax = Lerp(prob->rodd.svRohMaxEasy, prob->rodd.svRohMaxHard,u);

	u = EvaluateClq(prob->rodd.clqDradRoh);

	prob->dradRohMax = Lerp(prob->rodd.dradRohMaxEasy, prob->rodd.dradRohMaxHard, u);

	u = EvaluateClq(prob->rodd.clqCrocActive);

	const int crocActiveMax = static_cast<int>(Lerp(prob->rodd.crocActiveMaxEasy, prob->rodd.crocActiveMaxHard, u));

	if (crocActiveMax > prob->crocActiveMax && prob->crocActive == prob->crocActiveMax)
	{
		const float dtAdd = GRandInRange(prob->lmDtAddRoc.gMin, prob->lmDtAddRoc.gMax);

		prob->tAddRoc = g_clock.t + dtAdd;
	}

	prob->crocActiveMax = crocActiveMax;
	u = EvaluateClq(prob->rodd.clqCrohActive);
	const int crohActiveMax = static_cast<int>(Lerp(prob->rodd.crohActiveMaxEasy, prob->rodd.crohActiveMaxHard,u));

	if (crohActiveMax > prob->crohActiveMax && prob->crohActive == prob->crohActiveMax)
	{
		const float dtAdd = GRandInRange(prob->lmDtAddRoh.gMin, prob->lmDtAddRoh.gMax);
		prob->tAddRoh = g_clock.t + dtAdd;
	}

	prob->crohActiveMax = crohActiveMax;
}

void DestroyedRobRoc(ROB* prob, ROC* proc)
{
	++prob->crocDestroyed;

	AdjustRobDifficulty(prob, static_cast<float>(prob->crocDestroyed) / static_cast<float>(prob->crocCreateMax));

	if (proc->proh == nullptr)
		RemoveDlEntry(&prob->dlRocUnowned, proc);
	else
		RemoveDlEntry(&prob->dlRocOwned, proc);

	AppendDlEntry(&prob->dlRocInactive, proc);

	if (proc->proh != nullptr)
	{
		ROH* proh = proc->proh;

		if (FChooseRobRoc(prob, proh) != 0)
			SetRohRohs(proh, ROHS_Collect);
		else
		{
			proh->proc = nullptr;
			SetRohRohs(proh, ROHS_Wander);
		}
	}

	proc->proh = nullptr;

	if (proc->pvtlo->pfnRemoveLo != nullptr)
		proc->pvtlo->pfnRemoveLo(proc);

	if (prob->crocActive == prob->crocActiveMax)
	{
		const float dtAdd = GRandInRange(
			prob->lmDtAddRoc.gMin,
			prob->lmDtAddRoc.gMax);

		prob->tAddRoc = g_clock.t + dtAdd;
	}

	--prob->crocActive;
}

void SpawnedRobRoh(ROB* prob, ROH* proh)
{
	RemoveDlEntry(&prob->dlRostUnavailable, proh->prost);
	AppendDlEntry(&prob->dlRostAvailable, proh->prost);
	SetRostRosts(proh->prost, ROSTS_Close);
	proh->prost = nullptr;
}

void GrabbedRobRoh(ROB* prob, ROH* proh)
{
	ROC* proc = proh->proc;

	proc->pvtlo->pfnSetLoParent(proc, proh);

	proc->pvtalo->pfnSetAloVelocityVec(proc, &g_vecZero);
	proc->pvtalo->pfnSetAloAngularVelocityVec(proc, &g_vecZero);

	SetSoConstraints(proc, CT_Locked, nullptr, CT_Locked, nullptr);

	StartSound((SFXID)337, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
}

void DroppedRobRoh(ROB* prob, ROH* proh)
{
	ROC* proc = proh->proc;

	proc->pvtlo->pfnSetLoParent(proc, nullptr);

	proc->pvtalo->pfnSetAloVelocityVec(proc, &g_vecZero);
	proc->pvtalo->pfnSetAloAngularVelocityVec(proc, &g_vecZero);

	SetSoConstraints(proc, CT_Project, &g_normalZ, CT_Locked, nullptr);
}

void ReturnedRobRoh(ROB* prob, ROH* proh)
{
	SetRostRosts(proh->prost, ROSTS_Open);
}

void ExitedRobRohDuplicate(ROB* prob, ROH* proh)
{
	ROC* proc = proh->proc;
	ROST* prost = proh->prost;

	++prob->crocReturned;

	RemoveDlEntry(&prob->dlRocOwned, proc);
	AppendDlEntry(&prob->dlRocInactive, proc);

	RemoveDlEntry(&prob->dlRohActive, proh);
	AppendDlEntry(&prob->dlRohInactive, proh);

	RemoveDlEntry(&prob->dlRostUnavailable, prost);
	AppendDlEntry(&prob->dlRostAvailable, prost);

	proc->pvtlo->pfnSetLoParent(proc, nullptr);
	proc->pvtlo->pfnRemoveLo(proc);
	proh->pvtlo->pfnRemoveLo(proh);

	SetRostRosts(prost, ROSTS_Close);

	proh->prost = nullptr;
	proc->proh = nullptr;
	proh->proc = nullptr;

	if (prob->crocActive == prob->crocActiveMax)
	{
		const float dtAddRoc = GRandInRange(
			prob->lmDtAddRoc.gMin,
			prob->lmDtAddRoc.gMax);

		prob->tAddRoc = g_clock.t + dtAddRoc;
	}

	--prob->crocActive;

	if (prob->crohActive == prob->crohActiveMax)
	{
		const float dtAddRoh = GRandInRange(
			prob->lmDtAddRoh.gMin,
			prob->lmDtAddRoh.gMax);

		prob->tAddRoh = g_clock.t + dtAddRoh;
	}

	--prob->crohActive;

	StartSound(static_cast<SFXID>(81), nullptr, nullptr, nullptr, 1500.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
}

void KilledRobRoh(ROB* prob, ROH* proh)
{
	ROC* proc = proh->proc;
	ROST* prost = proh->prost;

	RemoveDlEntry(&prob->dlRohActive, proh);
	AppendDlEntry(&prob->dlRohInactive, proh);

	if (prost != nullptr)
	{
		RemoveDlEntry(&prob->dlRostUnavailable, prost);
		AppendDlEntry(&prob->dlRostAvailable, prost);

		SetRostRosts(prost, ROSTS_Close);
	}

	if (proc != nullptr)
	{
		if (proc->paloParent == proh)
			DroppedRobRoh(prob, proh);

		RemoveDlEntry(&prob->dlRocOwned, proc);
		AppendDlEntry(&prob->dlRocUnowned, proc);

		if (FChooseRobRoh(prob, proc) == 0)
			proc->proh = nullptr;
	}

	proh->prost = nullptr;
	proh->proc = nullptr;

	proh->pvtlo->pfnRemoveLo(proh);

	if (prob->crohActive == prob->crohActiveMax)
	{
		const float dtAddRoh = GRandInRange(
			prob->lmDtAddRoh.gMin,
			prob->lmDtAddRoh.gMax);

		prob->tAddRoh = g_clock.t + dtAddRoh;
	}

	--prob->crohActive;
}

void ExitedRobRoh(ROB* prob, ROH* proh)
{
	ROC* proc = proh->proc;
	ROST* prost = proh->prost;

	++prob->crocReturned;

	RemoveDlEntry(&prob->dlRocOwned, proc);
	AppendDlEntry(&prob->dlRocInactive, proc);

	RemoveDlEntry(&prob->dlRohActive, proh);
	AppendDlEntry(&prob->dlRohInactive, proh);

	RemoveDlEntry(&prob->dlRostUnavailable, prost);
	AppendDlEntry(&prob->dlRostAvailable, prost);

	proc->pvtlo->pfnSetLoParent(proc, nullptr);
	proc->pvtlo->pfnRemoveLo(proc);
	proh->pvtlo->pfnRemoveLo(proh);

	SetRostRosts(prost, ROSTS_Close);

	proh->prost = nullptr;
	proc->proh = nullptr;
	proh->proc = nullptr;

	if (prob->crocActive == prob->crocActiveMax)
	{
		const float dtAddRoc = GRandInRange(
			prob->lmDtAddRoc.gMin,
			prob->lmDtAddRoc.gMax);

		prob->tAddRoc = g_clock.t + dtAddRoc;
	}

	--prob->crocActive;

	if (prob->crohActive == prob->crohActiveMax)
	{
		const float dtAddRoh = GRandInRange(
			prob->lmDtAddRoh.gMin,
			prob->lmDtAddRoh.gMax);

		prob->tAddRoh = g_clock.t + dtAddRoh;
	}

	--prob->crohActive;

	StartSound(static_cast<SFXID>(81), nullptr, nullptr, nullptr, 1500.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
}

int FChooseRobRoc(ROB* prob, ROH* proh)
{
	ROC* procBest = nullptr;
	float distanceBestSq = FLT_MAX;

	for (ROC* proc = prob->dlRocUnowned.procFirst;
		proc != nullptr;
		proc = proc->dleRob.procNext)
	{
		const glm::vec3 dpos = proc->xf.posWorld - proh->xf.posWorld;
		const float distanceSq = glm::dot(dpos, dpos);

		if (distanceSq < distanceBestSq)
		{
			distanceBestSq = distanceSq;
			procBest = proc;
		}
	}

	if (procBest == nullptr)
		return 0;

	proh->proc = procBest;
	procBest->proh = proh;

	RemoveDlEntry(&prob->dlRocUnowned, procBest);
	AppendDlEntry(&prob->dlRocOwned, procBest);

	return 1;
}

void TakeRobRoc(ROB* prob, ROH* proh, ROC* proc)
{
	ROC* procOld = proh->proc;

	if (procOld == proc)
		return;

	ROH* prohOld = proc->proh;

	proh->proc = proc;
	proc->proh = proh;

	if (prohOld == nullptr)
	{
		RemoveDlEntry(&prob->dlRocUnowned, proc);
		AppendDlEntry(&prob->dlRocOwned, proc);
	}

	if (procOld != nullptr)
	{
		RemoveDlEntry(&prob->dlRocOwned, procOld);
		AppendDlEntry(&prob->dlRocUnowned, procOld);

		procOld->proh = nullptr;
		FChooseRobRoh(prob, procOld);
	}

	if (prohOld != nullptr)
	{
		prohOld->proc = nullptr;
		SetRohRohs(prohOld, ROHS_Wander);

		if (FChooseRobRoc(prob, prohOld) != 0)
			SetRohRohs(prohOld, ROHS_Collect);
	}
}

int FChooseRobRoh(ROB* prob, ROC* proc)
{
	ROH* prohBest = nullptr;
	float distanceBestSq = FLT_MAX;

	for (ROH* proh = prob->dlRohActive.prohFirst;
		proh != nullptr;
		proh = proh->dleRob.prohNext)
	{
		if (proh->proc != nullptr)
			continue;

		if (proh->rohs != ROHS_Wander)
			continue;

		const glm::vec3 dpos = proc->xf.posWorld - proh->xf.posWorld;
		const float distanceSq = glm::dot(dpos, dpos);

		if (distanceSq < distanceBestSq)
		{
			distanceBestSq = distanceSq;
			prohBest = proh;
		}
	}

	if (prohBest == nullptr)
		return 0;

	proc->proh = prohBest;
	prohBest->proc = proc;

	RemoveDlEntry(&prob->dlRocUnowned, proc);
	AppendDlEntry(&prob->dlRocOwned, proc);

	SetRohRohs(prohBest, ROHS_Collect);

	return 1;
}

int FChooseRobReturnPoint(ROB* prob, ROH* proh)
{
	ROST* prostBest = nullptr;
	float distanceBestSq = FLT_MAX;

	for (ROST* prost = prob->dlRostAvailable.prostFirst; prost != nullptr; prost = prost->dleRob.prostNext)
	{
		const glm::vec3 dpos = prost->xf.posWorld - proh->xf.posWorld;
		const float distanceSq = glm::dot(dpos, dpos);

		if (distanceSq < distanceBestSq)
		{
			distanceBestSq = distanceSq;
			prostBest = prost;
		}
	}

	if (prostBest == nullptr)
		return 0;

	RemoveDlEntry(&prob->dlRostAvailable, prostBest);
	AppendDlEntry(&prob->dlRostUnavailable, prostBest);

	proh->prost = prostBest;

	return 1;
}

void ChooseRobWanderLocation(ROB* prob, ROH* proh)
{
	float rad = GRandInRange(0.0f, 6.2831855f);
	float sXY = GRandInRange(0.0f, prob->sRadiusArenaWander);

	SetVectorCylind(&proh->posWander, rad, sXY, 0.0f);
	proh->posWander.y *= prob->rAspectArena;
}

RODD* ProddCurRob(ROB* prob, ENSK ensk)
{
	return prob->arodd + prob->irodd;
}

void DeleteRob(ROB *prob)
{
	delete prob;
}

SNIP s_asnipLoadRov[2] =
{
	2, (OID)0x28E, offsetof(ROV, psmBody),
	2, (OID)0x292, offsetof(ROV, psmTurret)
};

SNIP s_asnipPostRovLoad[6] = 
{
	4, (OID)0x291, offsetof(ROV, paloTurret),
	4, (OID)0x299, offsetof(ROV, paloProp),
	4, (OID)0x297, offsetof(ROV, pemitterLeftGun),
	4, (OID)0x298, offsetof(ROV, pemitterRightGun),
	2, (OID)0x29A, offsetof(ROV, pasegDescend),
	2, (OID)0x29b, offsetof(ROV, pasegAscend)
};

SNIP s_asnipLoadRoh[6] = 
{
	2, (OID)0x29C, offsetof(ROH, pasegMove),
	2, (OID)0x29D, offsetof(ROH, pasegMoveCollect),
	2, (OID)0x29E, offsetof(ROH, pasegaGrab),
	2, (OID)0x29F, offsetof(ROH, pasegDie),
	2, (OID)0x2A1, offsetof(ROH, psmHappy),
	2, (OID)0x2A2, offsetof(ROH, psmSad)
};

SMP s_smpRovSpin = {20.0, 0.0, 0.3};
SMP  s_smpRocSpin = {2.0, 0.0, 0.69999999};
int s_mprocsoidState[10] =
{
	0x2A4,
	0x2A5,
	0x2A6, 
	0x2A7,
	0x2A8,
	0x2A9,
	0x2AA,
	0x2AB,
	0x2AC,
	0x2AD
};

SNIP s_asnipLoadRoc = 
{
	2, (OID)0x2A3, offsetof(ROC, psm)
};

 SNIP s_asnipPostRocLoad =
{
	6, (OID)0x2AE, offsetof(ROC, pemitterBubble)
};

SNIP s_asnipLoadRost =
{
	2, (OID)0x2AF, offsetof(ROST, psm)
};

int s_aoidStateNext[2] = {0x2B0, 0x2B1};
SNIP s_asnipLoadRop = {2, (OID)0x2B3, offsetof(ROP, pasegReveal)};
