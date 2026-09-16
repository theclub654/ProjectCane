#include "hide.h"
#include "tn.h"
#include "jt.h"
#include "dart.h"
#include <cstdio>

void StartupHide()
{
	InitDl(&g_dlHshape, offsetof(HSHAPE, dleHshape));
	InitDl(&g_dlHpnt,   offsetof(HPNT ,dleHpnt));
	InitDl(&g_dlHbsk,   offsetof(HBSK, dleHbsk));
}

void ResetHideList()
{
	ClearDl(&g_dlHshape);
	ClearDl(&g_dlHpnt);
	ClearDl(&g_dlHbsk);
}

HBSK* NewHbsk()
{
	return new HBSK{};
}

void InitHbsk(HBSK* phbsk)
{
	InitSo(phbsk);

	SetAloRotationSpring(phbsk, 3.0);
	SetAloRotationDamping(phbsk, 3.0);
	SetAloPositionSmooth(phbsk, 3.0);
	SetAloRotationSmooth(phbsk, 2.0);
	phbsk->dzJumpMax = 300.0;
}

void* GetHbskSFlattenRadius(HBSK* phbsk)
{
	return &phbsk->sFlattenRadius;
}

void SetHbskSFlattenRadius(HBSK* phbsk, float sFlattenRadius)
{
	phbsk->sFlattenRadius = sFlattenRadius;
}

void* GetHbskCMaxDartsStuck(HBSK* phbsk)
{
	return &phbsk->cMaxDartsStuck;
}

void SetHbskCMaxDartsStuck(HBSK* phbsk, int cMaxDartsStuck)
{
	phbsk->cMaxDartsStuck = cMaxDartsStuck;
}

void* GetHbskDzJumpMax(HBSK* phbsk)
{
	return &phbsk->dzJumpMax;
}

void SetHbskDzJumpMax(HBSK* phbsk, float dzJumpMax)
{
	phbsk->dzJumpMax = dzJumpMax;
}

int GetHbskSize()
{
	return sizeof(HBSK);
}

void OnHbskAdd(HBSK* phbsk)
{
	OnSoAdd(phbsk);
	AppendDlEntry(&g_dlHbsk, phbsk);
}

void OnHbskRemove(HBSK* phbsk)
{
	OnSoRemove(phbsk);
	RemoveDlEntry(&g_dlHbsk, phbsk);
}

void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(static_cast<SO*>(phbsk), pbis);

	if (!phbsk->palox)
		phbsk->palox = std::make_shared<ALOX>();

	phbsk->palox->grfalox |= 1;
	phbsk->palox->matPreRotation = g_matIdentity;

}

void CloneHbsk(HBSK* phbsk, HBSK* phbskBase)
{
	DLE dleHbsk = phbsk->dleHbsk;

	CloneSo(phbsk, phbskBase);

	phbsk->hbsks = phbskBase->hbsks;
	phbsk->tHbsks = phbskBase->tHbsks;
	phbsk->dleHbsk = phbskBase->dleHbsk;
	phbsk->sFlattenRadius = phbskBase->sFlattenRadius;
	phbsk->cMaxDartsStuck = phbskBase->cMaxDartsStuck;
	phbsk->dzJumpMax = phbskBase->dzJumpMax;

	phbsk->dleHbsk = dleHbsk;
}

int FIgnoreHbskIntersection(HBSK* phbsk, JT* psoOther)
{
	if (psoOther == g_pjt && phbsk == psoOther->phbsk)
		return 1;

	return FIgnoreSoIntersection(phbsk, psoOther);
}

void PresetHbskAccel(HBSK* phbsk, float dt)
{
	PresetSoAccel(phbsk, dt);

	if (phbsk->hbsks != HBSKS_Available)
		return;

	glm::mat3 matUpright;
	TiltMatUpright(&phbsk->xf.mat, nullptr, &matUpright);
	AccelSoTowardMatSpring(phbsk, &matUpright, phbsk->pclqRotSpring.get(), &g_vecZero, phbsk->pclqRotDamping.get(), dt);
}

void SetHbskHbsks(HBSK* phbsk, HBSKS hbsks)
{
	if (phbsk->hbsks == hbsks)
		return;

	if (phbsk->hbsks == HBSKS_InUse) 
	{
		phbsk->pvtlo->pfnSetLoParent(phbsk, nullptr);
		SetSoConstraints(phbsk, CT_Free, nullptr, CT_Free, nullptr);
		HandleLoSpliceEvent(phbsk, 3, 0, nullptr);
	}

	if (hbsks == HBSKS_InUse) 
	{
		glm::mat3 matJt;
		glm::mat3 matHbskUpright;
		glm::mat3 dmat;
		glm::vec3 normal;
		float radUpright;

		phbsk->pvtlo->pfnSetLoParent(phbsk, g_pjt);
		phbsk->pvtalo->pfnSetAloVelocityVec(phbsk, &g_vecZero);
		phbsk->pvtalo->pfnSetAloAngularVelocityVec(phbsk, &g_vecZero);
		SetSoConstraints(phbsk, CT_Locked, nullptr, CT_Locked, nullptr);
		TiltMatUpright(&g_pjt->xf.matWorld, nullptr, &matJt);
		TiltMatUpright(&phbsk->xf.matWorld, &dmat, &matHbskUpright);
		DecomposeRotateMatrixRad(&dmat, &radUpright, &normal);
		CalculateDmat(&matJt, &matHbskUpright, &phbsk->palox->matPreRotation);
		HandleLoSpliceEvent(phbsk, 2, 0, nullptr);
	}

	phbsk->hbsks = hbsks;
	phbsk->tHbsks = g_clock.t;
}

int CdartStuckHbsk(HBSK* phbsk)
{
	DLI dliChild;
	int cdartStuck = 0;

	dliChild.m_pdl = &phbsk->dlChild;
	dliChild.m_ibDle = phbsk->dlChild.ibDle;
	dliChild.m_pdliNext = s_pdliFirst;

	DART* pdart = static_cast<DART*>(phbsk->dlChild.ploFirst);
	dliChild.m_ppv = pdart ? reinterpret_cast<void**>(reinterpret_cast<char*>(pdart) + dliChild.m_ibDle) : nullptr;
	s_pdliFirst = &dliChild;

	while (pdart != nullptr) {
		if (FIsBasicDerivedFrom(pdart, CID_DART) && static_cast<unsigned>(pdart->darts - DARTS_Airborne - 1) < 2U)
			++cdartStuck;

		pdart = static_cast<DART*>(*dliChild.m_ppv);
		dliChild.m_ppv = pdart ? reinterpret_cast<void**>(reinterpret_cast<char*>(pdart) + dliChild.m_ibDle) : nullptr;
	}

	s_pdliFirst = dliChild.m_pdliNext;
	return cdartStuck;
}

DART* PdartOldestHbskStuck(HBSK* phbsk)
{
	DLI dliChild;
	DART* pdartOldest = nullptr;
	float tOldest = 0.0f;

	dliChild.m_pdl = &phbsk->dlChild;
	dliChild.m_ibDle = phbsk->dlChild.ibDle;
	dliChild.m_pdliNext = s_pdliFirst;

	DART* pdart = phbsk->dlChild.pdartFirst;
	dliChild.m_ppv = pdart ? reinterpret_cast<void**>(reinterpret_cast<char*>(pdart) + dliChild.m_ibDle) : nullptr;
	s_pdliFirst = &dliChild;

	while (pdart != nullptr) 
	{
		if (FIsBasicDerivedFrom(pdart, CID_DART) && pdart->darts == DARTS_Stuck) {
			if (pdartOldest == nullptr || pdart->tDarts < tOldest) {
				pdartOldest = pdart;
				tOldest = pdart->tDarts;
			}
		}

		pdart = static_cast<DART*>(*dliChild.m_ppv);
		dliChild.m_ppv = pdart ? reinterpret_cast<void**>(reinterpret_cast<char*>(pdart) + dliChild.m_ibDle) : nullptr;
	}

	s_pdliFirst = dliChild.m_pdliNext;
	return pdartOldest;
}

void GetHbskClosestHidePos(HBSK* phbsk, glm::vec3* ppos, float* prad)
{
	PO* ppo = PpoCur();

	glm::vec3 dpos = ppo->xf.posWorld - phbsk->xf.posWorld;
	dpos.z = 0.0f;

	if (ppos != nullptr) 
	{
		const float distance = glm::length(dpos);
		const glm::vec3 direction = distance < 0.0001f ? g_normalX : dpos / distance;
		*ppos = phbsk->xf.posWorld + direction * (phbsk->sFlattenRadius + 70.0f);
	}

	if (prad != nullptr)
		*prad = RadNormalize(atan2f(dpos.y, dpos.x) + 3.1415927f);
}

void DeleteHbsk(HBSK *phbsk)
{
	delete phbsk;
}

HSHAPE* NewHshape()
{
	return new HSHAPE{};
}

void InitHshape(HSHAPE* phshape)
{
	InitLo(phshape);

	phshape->oidTnHide = OID_Nil;
	phshape->dzHideMax = FLT_MAX;
	phshape->grfhp = 3;
	phshape->fTunnel = 1;
}

void* GetHshapeJthk(HSHAPE* phshape)
{
	return &phshape->jthk;
}

void SetHshapeJthk(HSHAPE* phshape, JTHK jthk)
{
	phshape->jthk = jthk;
}

void* GetHshapeFTunnel(HSHAPE* phshape)
{
	return &phshape->fTunnel;
}

void SetHshapeFTunnel(HSHAPE* phshape, int fTunnel)
{
	phshape->fTunnel = fTunnel;
}

void* GetHshapeOidTnHide(HSHAPE* phshape)
{
	return &phshape->oidTnHide;
}

void SetHshapeOidTnHide(HSHAPE* phshape, int oidTnHide)
{
	phshape->oidTnHide = (OID)oidTnHide;
}

void* GetHshapeGrfhp(HSHAPE* phshape)
{
	return &phshape->grfhp;
}

void SetHshapeGrfhp(HSHAPE* phshape, GRFHP grfhp)
{
	phshape->grfhp = grfhp;
}

void* GetHshapeFDetect(HSHAPE* phshape)
{
	return &phshape->fDetect;
}

void SetHshapeFDetect(HSHAPE* phshape, int fDetect)
{
	phshape->fDetect = fDetect;
}

void* GetHshapeDzHideMax(HSHAPE* phshape)
{
	return &phshape->dzHideMax;
}

void SetHshapeDzHideMax(HSHAPE* phshape, float dzHideMax)
{
	phshape->dzHideMax = dzHideMax;
}

void* GetHshapeDzMax(HSHAPE* phshape)
{
	return &phshape->dzMax;
}

void SetHshapeDzMax(HSHAPE* phshape, int fDzMax)
{
	phshape->dzMax = fDzMax ? 1.0f : 0.0f;
}

int GetHshapeSize()
{
	return sizeof(HSHAPE);
}

void OnHshapeAdd(HSHAPE* phshape)
{
	OnLoAdd(phshape);
	AppendDlEntry(&g_dlHshape, phshape);
}

void OnHshapeRemove(HSHAPE* phshape)
{
	OnLoRemove(phshape);
	RemoveDlEntry(&g_dlHshape, phshape);
}

void CloneHshape(HSHAPE* phshape, HSHAPE* phshapeBase)
{
	DLE dleHshape = phshape->dleHshape;

	CloneShape(phshape, phshapeBase);

	phshape->dleHshape = phshapeBase->dleHshape;
	phshape->jthk = phshapeBase->jthk;
	phshape->oidTnHide = phshapeBase->oidTnHide;
	phshape->ptnHide = phshapeBase->ptnHide;
	phshape->fTunnel = phshapeBase->fTunnel;
	phshape->grfhp = phshapeBase->grfhp;
	phshape->fDetect = phshapeBase->fDetect;
	phshape->dzHideMax = phshapeBase->dzHideMax;
	phshape->dzMax = phshapeBase->dzMax;

	phshape->dleHshape = dleHshape;
}

void BindHshape(HSHAPE* phshape)
{
	if (phshape->oidTnHide == OID_Nil)
		return;

	phshape->ptnHide = static_cast<TN*>(PloFindSwNearest(phshape->psw, (OID)phshape->oidTnHide, phshape->paloParent));
	phshape->ptnHide->tnsOverride = TNS_Out;
}

void GetHshapeHidePos(HSHAPE* phshape, float sParam, glm::vec3* pposHide, float* pradTarget)
{
	glm::vec3 pos{};
	glm::vec3 normal{};

	if (phshape->pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
		phshape->pcrv->pvtcrv->pfnEvaluateCrvFromS(phshape->pcrv.get(), sParam, &pos, &normal);

	ConvertAloPos(phshape->paloParent, nullptr, &pos, &pos);
	ConvertAloVec(phshape->paloParent, nullptr, &normal, &normal);

	float sOffset = 0.0f;

	if (phshape->jthk == JTHK_Flatten)
		sOffset = 20.0f;
	else if (phshape->jthk == JTHK_Duck)
		sOffset = 70.0f;

	if (pposHide != nullptr)
	{
		glm::vec3 tangent = glm::cross(normal, g_normalZ);
		float sTangentSquared = glm::dot(tangent, tangent);

		if (sTangentSquared >= 0.0001f)
			tangent /= std::sqrt(sTangentSquared);
		else
			tangent = glm::vec3(0.0f);

		*pposHide = pos + tangent * sOffset;
	}

	if (pradTarget != nullptr)
	{
		float radTarget = std::atan2(-normal.x, normal.y);

		if (phshape->jthk == JTHK_Duck)
			radTarget = RadNormalize(radTarget + 3.1415927f);

		*pradTarget = radTarget;
	}
}

void GetHshapeClosestHidePos(HSHAPE* phshape, float* psParamHshape, glm::vec3* pposHide, float* pradTarget)
{
	PO* ppo = PpoCur();
	CRV* pcrv = phshape->pcrv.get();

	glm::vec3 pos{};
	CONSTR constr{};

	constr.ct = CT_Tangent;

	ConvertAloPos(nullptr, phshape->paloParent, &ppo->xf.posWorld, &pos);
	ConvertAloVec(nullptr, phshape->paloParent, &g_normalZ, &constr.normal);

	float sParam = 0.0f;

	if (pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
		pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &pos, &constr, nullptr, nullptr, nullptr, &sParam);

	float sMax = pcrv->pvtcrv->pfnSMaxCrv(pcrv);
	float sParamMax = sMax - 30.0f;

	if (sParam < 30.0f)
		sParam = 30.0f;
	else if (sParam > sParamMax)
		sParam = sParamMax;

	GetHshapeHidePos(phshape, sParam, pposHide, pradTarget);

	*psParamHshape = sParam;
}

void DeleteHshape(HSHAPE* phshape)
{
	delete phshape;
}

HPNT* NewHpnt()
{
	return new HPNT{};
}

void InitHpnt(HPNT* phpnt)
{
	InitLo(phpnt);

	phpnt->fTunnel = 1;
	phpnt->oidTnHide = OID_Nil;
}

// Function bodies

void* GetHpntSFlattenRadius(HPNT* phpnt)
{
	return &phpnt->sFlattenRadius;
}

void SetHpntSFlattenRadius(HPNT* phpnt, float sFlattenRadius)
{
	phpnt->sFlattenRadius = sFlattenRadius;
}

void* GetHpntJthk(HPNT* phpnt)
{
	return &phpnt->jthk;
}

void SetHpntJthk(HPNT* phpnt, JTHK jthk)
{
	phpnt->jthk = jthk;
}

void SetHpntOidTnHide(HPNT* phpnt, int oidTnHide)
{
	phpnt->oidTnHide = (OID)oidTnHide;
}

void* GetHpntFTunnel(HPNT* phpnt)
{
	return &phpnt->fTunnel;
}

void SetHpntFTunnel(HPNT* phpnt, int fTunnel)
{
	phpnt->fTunnel = fTunnel;
}

void* GetHpntFDetect(HPNT* phpnt)
{
	return &phpnt->fDetect;
}

void SetHpntFDetect(HPNT* phpnt, int fDetect)
{
	phpnt->fDetect = fDetect;
}

void* GetHpntDzJumpTargetMax(HPNT* phpnt)
{
	return &phpnt->dzJumpTargetMax;
}

void SetHpntDzJumpTargetMax(HPNT* phpnt, float dzJumpTargetMax)
{
	phpnt->dzJumpTargetMax = dzJumpTargetMax;
}

void* GetHpntUnkInt(HPNT* phpnt)
{
	return &phpnt->unkInt;
}

void SetHpntUnkInt(HPNT* phpnt, int unkInt)
{
	phpnt->unkInt = unkInt;
}

int GetHpntSize()
{
	return sizeof(HPNT);
}

void OnHpntAdd(HPNT* phpnt)
{
	OnLoAdd(phpnt);
	AppendDlEntry(&g_dlHpnt, phpnt);
}

void OnHpntRemove(HPNT* phpnt)
{
	OnLoRemove(phpnt);
	RemoveDlEntry(&g_dlHpnt, phpnt);
}

void CloneHpnt(HPNT* phpnt, HPNT* phpntBase)
{
	DLE dleHpnt = phpnt->dleHpnt;

	ClonePnt(phpnt, phpntBase);

	phpnt->dleHpnt = phpntBase->dleHpnt;
	phpnt->jthk = phpntBase->jthk;
	phpnt->sFlattenRadius = phpntBase->sFlattenRadius;
	phpnt->oidTnHide = phpntBase->oidTnHide;
	phpnt->ptnHide = phpntBase->ptnHide;
	phpnt->fTunnel = phpntBase->fTunnel;
	phpnt->fDetect = phpntBase->fDetect;
	phpnt->dzIgnore = phpntBase->dzIgnore;

	phpnt->dleHpnt = dleHpnt;
}

void BindHpnt(HPNT* phpnt)
{
	if (phpnt->oidTnHide != OID_Nil) 
	{
		TN *ptn = (TN*)PloFindSwNearest(phpnt->psw, (OID)phpnt->oidTnHide, phpnt->paloParent);
		phpnt->ptnHide = ptn;
		ptn->tnsOverride = TNS_Out;
	}
}

void GetHpntClosestHidePos(HPNT* phpnt, float sParam, glm::vec3* pposHide, float* pradTarget)
{
	const float rad = sParam / phpnt->sFlattenRadius;

	if (pposHide != nullptr)
	{
		float sRadius = phpnt->sFlattenRadius;

		if (phpnt->jthk == JTHK_Flatten)
			sRadius += 20.0f;
		else if (phpnt->jthk == JTHK_Duck)
			sRadius += 70.0f;

		glm::vec3 offset;
		SetVectorCylind(&offset, rad, sRadius, 0.0f);

		GetPntPos(phpnt, pposHide);
		*pposHide += offset;
	}

	if (pradTarget != nullptr)
	{
		*pradTarget = rad;

		if (phpnt->jthk == JTHK_Duck)
			*pradTarget = RadNormalize(rad + 3.1415927f);
	}
}

void GetHpntHidePos(HPNT* phpnt, float sParam, glm::vec3* pposHide, float* pradTarget)
{
	PO* ppo = PpoCur();

	glm::vec3 posHpnt;
	GetPntPos(phpnt, &posHpnt);

	const glm::vec3 dpos = ppo->xf.posWorld - posHpnt;
	const float rad = std::atan2(dpos.y, dpos.x);
	const float sFlattenParam = rad * phpnt->sFlattenRadius;

	GetHpntClosestHidePos(phpnt, sFlattenParam, pposHide, pradTarget);
}

void DeleteHpnt(HPNT* phpnt)
{
	delete phpnt;
}

DL g_dlHbsk;
DL g_dlHshape;
DL g_dlHpnt;
