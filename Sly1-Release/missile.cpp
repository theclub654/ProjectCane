#include "missile.h"

MISSILE* NewMissile()
{
	return new MISSILE{};
}

void InitMissile(MISSILE* pmissile)
{
	InitBomb(pmissile);
	pmissile->fFollowTrajectory = 1;
}

void SetMissileSvMissileLaunch(MISSILE* pmissile, float svMissileLaunch)
{
	pmissile->svMissileLaunch = svMissileLaunch;
}

void* GetMissileSvMissileLaunch(MISSILE* pmissile)
{
	return &pmissile->svMissileLaunch;
}

void SetMissilePaloTarget(MISSILE* pmissile, ALO* paloTarget)
{
	pmissile->paloTarget = paloTarget;
}

void* GetMissilePaloTarget(MISSILE* pmissile)
{
	return &pmissile->paloTarget;
}

void SetMissilePosTargetLocal(MISSILE* pmissile, glm::vec3 posTargetLocal)
{
	pmissile->posTargetLocal = posTargetLocal;
}

void* GetMissilePosTargetLocal(MISSILE* pmissile)
{
	return &pmissile->posTargetLocal;
}

void SetMissileFFollowTrajectory(MISSILE* pmissile, int fFollowTrajectory)
{
	pmissile->fFollowTrajectory = fFollowTrajectory;
}

void* GetMissileFFollowTrajectory(MISSILE* pmissile)
{
	return &pmissile->fFollowTrajectory;
}

int GetMissileSize()
{
	return sizeof(MISSILE);
}

void OnMissileRemove(MISSILE* pmissile)
{
	OnSoRemove(pmissile);

	if (pmissile->pasegaCur != nullptr) 
	{
		RetractAsega(pmissile->pasegaCur);
		pmissile->pasegaCur = nullptr;
	}
}

void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis)
{
	LoadBombFromBrx(pmissile, pbis);
	SnipAloObjects(pmissile, 1, &s_asnipMissile);
	pmissile->fNoFreeze = 1;
}

void CloneMissile(MISSILE* pmissile, MISSILE* pmissileBase)
{
	CloneBomb(pmissile, pmissileBase);

	pmissile->svMissileLaunch = pmissileBase->svMissileLaunch;
	pmissile->paloTarget = pmissileBase->paloTarget;
	pmissile->posTargetLocal = pmissileBase->posTargetLocal;
	pmissile->pasegFire = pmissileBase->pasegFire;
	pmissile->pasegaCur = pmissileBase->pasegaCur;
	pmissile->fFollowTrajectory = pmissileBase->fFollowTrajectory;
}

void ProjectMissileTransform(MISSILE* pmissile, float dt, int fParentDirty)
{
	if (pmissile->fFollowTrajectory)
		LoadLookAtMatrix(&pmissile->xf.v, &pmissile->xf.mat);

	ACT* pactPos = pmissile->pactPos;
	ACT* pactRot = pmissile->pactRot;

	pmissile->pactPos = nullptr;
	pmissile->pactRot = nullptr;

	ProjectSoTransform(pmissile, dt, fParentDirty);

	pmissile->pactRot = pactRot;
	pmissile->pactPos = pactPos;
}

void FireMissile(MISSILE* pmissile, ALO* paloTarget, glm::vec3* pposTargetLocal)
{
	pmissile->paloTarget = paloTarget;
	pmissile->posTargetLocal = *pposTargetLocal;

	if (pmissile->pasegFire != nullptr)
		ApplyAseg(pmissile->pasegFire, pmissile, 0.0f, 1.0f, 0, &pmissile->pasegaCur);

	if (pmissile->fFollowTrajectory)
	{
		const glm::vec3& velocity = pmissile->xf.v;
		const bool fStationary = std::abs(velocity.x) < 0.0001f && std::abs(velocity.y) < 0.0001f && std::abs(velocity.z) < 0.0001f;

		if (!fStationary)
		{
			glm::mat3 mat;
			LoadLookAtMatrix(&pmissile->xf.v, &mat);
			pmissile->pvtalo->pfnRotateAloToMat(pmissile, &mat);
		}
	}
}

void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro)
{
	if (pmissile->fHidden)
		return;

	if (pmissile->pactPos == nullptr && pmissile->pactRot == nullptr)
	{
		RenderAloAll(pmissile, pcm, pro);
		return;
	}

	glm::vec3 pos = g_vecZero;
	glm::vec3 v(0.0f);

	if (pmissile->pactPos != nullptr)
		pmissile->pactPos->pvtact->pfnGetActPositionGoal(pmissile->pactPos, 0.0f, &pos, &v);

	glm::mat3 mat(1.0f);
	glm::vec3 w(0.0f);

	if (pmissile->pactRot != nullptr)
		pmissile->pactRot->pvtact->pfnGetActRotationGoal(pmissile->pactRot, 0.0f, &mat, &w);

	RO ro;
	DupAloRo(pmissile, pro, &ro);

	glm::mat4 matAct;
	LoadMatrixFromPosRot(&pos, &mat, &matAct);
	ro.model = ro.model * matAct;

	RenderAloAll(pmissile, pcm, &ro);
}

int FIgnoreMissileIntersection(MISSILE* pmissile, SO* psoOther)
{
	if (psoOther == pmissile->psoIgnore)
		return 1;

	for (int ioid = 0; ioid < pmissile->coidIgnore; ++ioid)
	{
		if (FMatchesLoName(psoOther, (OID)pmissile->aoidIgnore[ioid]))
			return 1;
	}

	for (int icid = 0; icid < pmissile->ccidIgnore; ++icid)
	{
		if (FIsBasicDerivedFrom(psoOther, (CID)pmissile->acidIgnore[icid]))
			return 1;
	}

	return FIgnoreSoIntersection(pmissile, psoOther);
}

void AddMissileIgnoreObject(MISSILE* pmissile, OID oid)
{
	if (pmissile->coidIgnore >= 4)
		return;

	pmissile->aoidIgnore[pmissile->coidIgnore++] = oid;
}

void AddMissileIgnoreClass(MISSILE* pmissile, CID cid)
{
	if (pmissile->ccidIgnore >= 4)
		return;

	pmissile->acidIgnore[pmissile->ccidIgnore++] = cid;
}

void DeleteMissile(MISSILE *pmissile)
{
	delete pmissile;
}

ACCMISS* NewAccmiss()
{
	return new ACCMISS{};
}

void InitAccmiss(ACCMISS* paccmiss)
{
	InitMissile(paccmiss);
	paccmiss->dvGravity = g_vecZero;
}

int GetAccmissSize()
{
	return sizeof(ACCMISS);
}

void CloneAccmiss(ACCMISS* paccmiss, ACCMISS* paccmissBase)
{
	CloneMissile(paccmiss, paccmissBase);

	paccmiss->svMissileMax = paccmissBase->svMissileMax;
	paccmiss->sdvMissileMax = paccmissBase->sdvMissileMax;
	paccmiss->vTarget = paccmissBase->vTarget;
}

void FireAccmiss(ACCMISS* paccmiss, ALO* paloTarget, glm::vec3* pposTarget)
{
	FireMissile(paccmiss, paloTarget, pposTarget);

	const float speed = glm::length(paccmiss->xf.v);

	if (speed < 0.0001f)
		paccmiss->vTarget = g_normalX * paccmiss->svMissileMax;
	else
		paccmiss->vTarget = (paccmiss->xf.v / speed) * paccmiss->svMissileMax;
}

void PresetAccmissAccel(ACCMISS* paccmiss, float dt)
{
	PresetSoAccel(paccmiss, dt);

	glm::vec3 dv = (paccmiss->vTarget - paccmiss->xf.v) / dt;

	LimitVectorLength(&dv, paccmiss->sdvMissileMax, &dv);
	AddSoAcceleration(paccmiss, &dv);
}

void DeleteAccmiss(ACCMISS* paccmiss)
{
	delete paccmiss;
}

TARMISS* NewTarmiss()
{
	return new TARMISS{};
}

void InitTarmiss(TARMISS* ptarmiss)
{
	InitAccmiss(ptarmiss);
}

void* GetTarmissDtLifetimeOverride(TARMISS* ptarmiss)
{
	return &ptarmiss->dtLifetimeOverride;
}

void SetTarmissDtLifetimeOverride(TARMISS* ptarmiss, float dtLifetimeOverride)
{
	ptarmiss->dtLifetimeOverride = dtLifetimeOverride;
}

void* GetTarmissSvMax(TARMISS* ptarmiss)
{
	return &ptarmiss->svMax;
}

void SetTarmissSvMax(TARMISS* ptarmiss, float svMax)
{
	ptarmiss->svMax = svMax;
}

void* GetTarmissRTrajectoryBank(TARMISS* ptarmiss)
{
	return &ptarmiss->rTrajectoryBank;
}

void SetTarmissRTrajectoryBank(TARMISS* ptarmiss, float rTrajectoryBank)
{
	ptarmiss->rTrajectoryBank = rTrajectoryBank;
}

void* GetTarmissVTarget(TARMISS* ptarmiss)
{
	return &ptarmiss->vTarget;
}

void SetTarmissVTarget(TARMISS* ptarmiss, glm::vec3 vTarget)
{
	ptarmiss->vTarget = vTarget;
}

int GetTarmissSize()
{
	return sizeof(TARMISS);
}

void CloneTarmiss(TARMISS* ptarmiss, TARMISS* ptarmissBase)
{
	CloneAccmiss(ptarmiss, ptarmissBase);

	ptarmiss->dtLifetimeOverride = ptarmissBase->dtLifetimeOverride;
	ptarmiss->svMax = ptarmissBase->svMax;
	ptarmiss->rTrajectoryBank = ptarmissBase->rTrajectoryBank;
	ptarmiss->tExpire = ptarmissBase->tExpire;
	ptarmiss->vTarget = ptarmissBase->vTarget;
}

void FireTarmiss(TARMISS* ptarmiss, ALO* paloTarget, glm::vec3* pposTargetLocal)
{
	FireMissile(static_cast<MISSILE*>(ptarmiss), paloTarget, pposTargetLocal);

	if (ptarmiss->dtLifetimeOverride > 0.0f)
	{
		ptarmiss->tExpire = g_clock.t + ptarmiss->dtLifetimeOverride;
		return;
	}

	glm::vec3 posTarget;
	glm::mat3 matTarget;

	PredictAloTransform(ptarmiss->paloTarget, nullptr, DT_TarmissileTargetPrediction, &posTarget, &matTarget, nullptr, nullptr);

	posTarget += matTarget * ptarmiss->posTargetLocal;

	glm::vec3 posLine0 = ptarmiss->xf.v * -0.5f;

	LimitVectorLength(&posLine0, ptarmiss->svMax * 0.75f, &posLine0);

	glm::vec3 dposTarget = posTarget - ptarmiss->xf.pos;
	glm::vec3 posLine1 = posLine0 + dposTarget * 1.5f;

	LSG lsg = {};

	ClsgClipLineToSphere(&g_vecZero, ptarmiss->svMax, &posLine0, &posLine1, 1, &lsg);

	// The original reads local_6c, which is LSG::au[1] at offset 0x44.
	// au[0] is the entry intersection and is negative for this construction.
	float uIntercept = std::max(lsg.au[1], 0.25f);

	ptarmiss->tExpire = g_clock.t + 1.0f / uIntercept;

}

void ProjectTarmissTransform(TARMISS* ptarmiss, float dt, int fParentDirty)
{
	glm::vec3 acceleration{ 0.0f };

	if (g_clock.t < ptarmiss->tExpire)
	{
		const float dtRemaining = ptarmiss->tExpire - g_clock.t + dt;
		const float dtPredict = std::min(DT_TarmissileTargetPrediction, dtRemaining);

		glm::vec3 posTarget;
		glm::mat3 matTarget;

		PredictAloTransform(ptarmiss->paloTarget, nullptr, dtPredict, &posTarget, &matTarget, nullptr, nullptr);

		posTarget += matTarget * ptarmiss->posTargetLocal;

		glm::vec3 vTarget = ptarmiss->vTarget;

		if (glm::dot(vTarget, vTarget) < 0.0001f * 0.0001f)
			vTarget = (posTarget - ptarmiss->xf.pos) * (1.5f / dtRemaining) - ptarmiss->xf.v * 0.5f;

		glm::vec3 posNew;
		glm::vec3 vNew;

		EvaluateBezierPos(dtRemaining, dt, 1.0f, &ptarmiss->xf.pos, &ptarmiss->xf.v, &posTarget, &vTarget, &posNew, &vNew, &acceleration);

		ptarmiss->xf.pos = posNew;
		ptarmiss->xf.v = vNew;
	}
	else
		ptarmiss->xf.pos += ptarmiss->xf.v * dt;

	if (ptarmiss->fFollowTrajectory && glm::dot(ptarmiss->xf.v, ptarmiss->xf.v) >= 0.0001f * 0.0001f)
	{
		glm::vec3 accelerationProjected;

		SProjectVector(&acceleration, &ptarmiss->xf.mat[0], 0, &accelerationProjected);

		glm::vec3 up = accelerationProjected * ptarmiss->rTrajectoryBank - g_psw->dvGravity;

		BuildOrthonormalMatrixZ(&ptarmiss->xf.v, &up, &ptarmiss->xf.mat);
	}

	DL* pdlChild = &ptarmiss->dlChild;
	const int ibDle = pdlChild->ibDle;

	for (LO* ploChild = pdlChild->ploFirst; ploChild != nullptr;)
	{
		LO* ploNext = *reinterpret_cast<LO**>(reinterpret_cast<uint8_t*>(ploChild) + ibDle);

		if ((ploChild->pvtlo->grfcid & 1U) != 0)
		{
			ALO* paloChild = static_cast<ALO*>(ploChild);

			if (paloChild->pvtalo->pfnProjectAloTransform != nullptr)
				paloChild->pvtalo->pfnProjectAloTransform(paloChild, dt, 1);
		}

		ploChild = ploNext;
	}

	if (!fParentDirty && ptarmiss->pvtso->pfnUpdateSoXfWorld != nullptr)
		ptarmiss->pvtso->pfnUpdateSoXfWorld(ptarmiss);
}

void DeleteTarmiss(TARMISS *ptarmiss)
{
	delete ptarmiss;
}

SPLMISS* NewSplmiss()
{
	return new SPLMISS{};
}

int GetSplmissSize()
{
	return sizeof(SPLMISS);
}

void CloneSplmiss(SPLMISS* psplmiss, SPLMISS* psplmissBase)
{
	CloneMissile(psplmiss, psplmissBase);

	psplmiss->dtImpact = psplmissBase->dtImpact;
	psplmiss->svMissileImpact = psplmissBase->svMissileImpact;
	psplmiss->uBank = psplmissBase->uBank;
	psplmiss->tImpact = psplmissBase->tImpact;
	psplmiss->vImpact = psplmissBase->vImpact;
}

void DeleteSplmiss(SPLMISS *psplmiss)
{
	delete psplmiss;
}

GROUNDMISS* NewGroundmiss()
{
	return new GROUNDMISS{};
}

void InitGroundmiss(GROUNDMISS* pgroundmiss)
{
	InitMissile(pgroundmiss);
	pgroundmiss->fFollowTrajectory = 0;
	pgroundmiss->sdvTrackMax = 2000.0;
}

int GetGroundmissSize()
{
	return sizeof(GROUNDMISS);
}

void CloneGroundmiss(GROUNDMISS* pgroundmiss, GROUNDMISS* pgroundmissBase)
{
	CloneMissile(pgroundmiss, pgroundmissBase);

	pgroundmiss->sdvTrackMax = pgroundmissBase->sdvTrackMax;
}

void DeleteGroundmiss(GROUNDMISS *pgroundmiss)
{
	delete pgroundmiss;
}

SNIP s_asnipMissile = 
{
	2, (OID)0x317, offsetof(MISSILE, pasegFire)
};
