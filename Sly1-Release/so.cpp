#include "so.h"
#include "proxy.h"
#include "bbmark.h"
#include "clip.h"
#include "water.h"
#include "spliceobj.h"
#include "frame.h"
#include "hide.h"
#include "mb.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <intrin.h>
#include <cstdio>
#include <limits>

SO* NewSo()
{
	return new SO{};
}

void InitSwBusySoDl(SW* psw)
{
	InitDl(&psw->dlBusySo, offsetof(SO, dleBusySo));
}

void InitSwRootDl(SW *psw)
{
	InitDl(&psw->dlRoot, offsetof(SO, dleRoot));
}

void InitSo(SO* pso)
{
	CRef ref;

	InitDl(&pso->dlPhys, offsetof(SO, dlePhys));
	InitAlo(pso);

	InitGeom(&pso->geomLocal);

	pso->geomWorld = pso->geomLocal;

	if (pso->paloParent != nullptr)
		SetSoConstraints(pso, CT_Locked, nullptr, CT_Locked, nullptr);

	pso->ipso = -1;
	pso->ipsoRoot = -1;

	if (pso->pframe != nullptr)
	{
		const bool hasBinding6 = pso->pframe->FFindBinding(6, 1, &ref) != 0;
		const bool hasBinding7 = pso->pframe->FFindBinding(7, 1, &ref) != 0;

		if (hasBinding6 || hasBinding7)
			pso->fGenSpliceTouchEvents = true;
	}

	pso->dvGravity = pso->psw->dvGravity;

	RebuildSoPhysHook(pso);

	allSWSoObjs.push_back(pso);
}

int GetSoSize()
{
	return sizeof(SO);
}

void OnSoAdd(SO *pso)
{
	SW* psw = pso->psw;

	psw->cpsoAll++;

	if (pso->paloParent == nullptr)
	{
		AddSwAaobrObject(psw, pso);
		pso->poxa = PoxaAllocSw(psw, pso);

		RecalcSwOxfFilterForObject(psw, pso);
		psw->cpsoRoot++;
		AppendDlEntry(&psw->dlRoot, pso);
	}

	OnAloAdd(pso);
	EnableSoPhys(pso, true);
	
	pso->pvtso->pfnUpdateSoPosWorldPrev(pso);

	if (!pso->fLockedSelf)
		RecalcSoLocked((SO*)pso->paloRoot);
	else
		RecalcSoLocked(pso);

	RebuildSoPhysHook(pso);

}

void OnSoRemove(SO* pso)
{
	SW* psw = pso->psw;
	SO* root = (SO*)pso->paloRoot;

	EnableSoPhys(pso, false);
	OnAloRemove(pso);
	psw->cpsoAll--;

	if (pso->paloParent == nullptr)
	{
		RemoveSwAaobrObject(psw, pso);
		FreeSwPoxa(psw, pso->poxa);

		psw->cpsoRoot--;
		RemoveDlEntry(&psw->dlRoot, pso);
	}

	FreeStsoList(pso->pstso);
	pso->pstso = nullptr;

	if (!pso->fLockedSelf && root != pso)
		RecalcSoLocked(root);
}

void EnableSoPhys(SO* pso, int fPhys)
{
	const bool enablePhys = (fPhys != 0);

	if (enablePhys == pso->fPhys)
		return;

	SO* pso1 = (SO*)pso->paloRoot;

	if (enablePhys)
		AppendDlEntry(&pso1->dlPhys, pso);
	else
		RemoveDlEntry(&pso1->dlPhys, pso);

	pso->fPhys = enablePhys;

	InvalidateSwXpForObject(pso->psw, pso, 7);

	pso->paloRoot->pvtso->pfnUpdateSoBounds((SO*)pso->paloRoot);
}

void RebuildSoPhysHook(SO* pso)
{
	VTSO* vtso = pso->pvtso;

	const bool handlesPhysicsCallbacks =
		vtso->pfnNotifySoImpact != nullptr ||
		vtso->pfnAdjustSoNewXp != nullptr ||
		vtso->pfnAdjustSoXpVelocity != nullptr ||
		vtso->pfnAdjustSoDz != nullptr;

	if (handlesPhysicsCallbacks)
		pso->psoPhysHook = pso;
	else
	{
		if (pso->paloParent == nullptr)
			pso->psoPhysHook = nullptr;
		else
			pso->psoPhysHook = reinterpret_cast<SO*>(pso->paloParent)->psoPhysHook;
	}

	for (SO* child = pso->dlChild.psoFirst; child != nullptr; child = child->dleChild.psoNext)
	{
		// grfcid & 2 means child is an SO
		if ((child->pvtlo->grfcid & 2U) == 0)
			continue;

		RebuildSoPhysHook(child);
	}
}

void CloneSo(SO* pso, SO* psoBase)
{
	DLE savedDleRoot = pso->dleRoot;
	DL  savedDlPhys = pso->dlPhys;
	DLE savedDlePhys = pso->dlePhys;
	DLE savedDleBusySo = pso->dleBusySo;

	OXA* savedPoxa = pso->poxa;
	SO* savedPhysHook = pso->psoPhysHook;

	bool savedFPhys = pso->fPhys;

	CloneAlo(pso, psoBase);

	pso->dleRoot = savedDleRoot;
	pso->dlPhys = savedDlPhys;
	pso->dlePhys = savedDlePhys;
	pso->dleBusySo = savedDleBusySo;

	pso->poxa = savedPoxa;
	pso->psoPhysHook = savedPhysHook;

	pso->momintLocal = psoBase->momintLocal;
	pso->momintInvLocal = psoBase->momintInvLocal;
	pso->dvGravity = psoBase->dvGravity;
	pso->gBuoyancy = psoBase->gBuoyancy;
	pso->gViscosity = psoBase->gViscosity;
	pso->m = psoBase->m;

	pso->posWorldPrev = psoBase->posWorldPrev;

	// The original shallow copy intentionally shared geomLocal's backing arrays.
	// A std::vector assignment duplicates the arrays but leaves SURF/EDGE links
	// pointing into the base object's arrays.  CloneGeom performs the owned-copy
	// equivalent and rebases all of those internal links to this instance.
	CloneGeom(&psoBase->geomLocal, nullptr, &pso->geomLocal);
	pso->geomWorld = psoBase->geomWorld;
	pso->plvo = psoBase->plvo;

	pso->sRadiusSelf = psoBase->sRadiusSelf;
	pso->sRadiusAll = psoBase->sRadiusAll;
	pso->sRadiusPrune = psoBase->sRadiusPrune;
	pso->posPrune = psoBase->posPrune;

	pso->bspc = psoBase->bspc;

	pso->cnpg = psoBase->cnpg;
	pso->anpg = psoBase->anpg;
	pso->mpibspinpg = psoBase->mpibspinpg;

	pso->chsg = psoBase->chsg;
	pso->ahsg = psoBase->ahsg;
	pso->mpisurfihsgMic = psoBase->mpisurfihsgMic;

	pso->posMin = psoBase->posMin;
	pso->posMax = psoBase->posMax;

	pso->constrForce = psoBase->constrForce;
	pso->constrTorque = psoBase->constrTorque;

	pso->dpos = psoBase->dpos;
	pso->drot = psoBase->drot;

	pso->pxa = nullptr;
	pso->pxpInternal = psoBase->pxpInternal;
	pso->grfpvaXpValid = 0;

	pso->ipsoRoot = psoBase->ipsoRoot;
	pso->ipso = psoBase->ipso;

	pso->posComLocal = psoBase->posComLocal;
	pso->zok = psoBase->zok;

	CloneGeom(&psoBase->geomCameraLocal, nullptr, &pso->geomCameraLocal);
	pso->geomCameraWorld = psoBase->geomCameraWorld;
	pso->bspcCamera = psoBase->bspcCamera;

	pso->cmk = psoBase->cmk;
	pso->egk = psoBase->egk;

	pso->fSphere = psoBase->fSphere;
	pso->fClone = psoBase->fClone;
	pso->fNoXpsAll = psoBase->fNoXpsAll;
	pso->fNoXpsSelf = psoBase->fNoXpsSelf;
	pso->fNoXpsCenter = psoBase->fNoXpsCenter;
	pso->fActive = psoBase->fActive;
	pso->fVelcro = psoBase->fVelcro;
	pso->fIgnoreLocked = psoBase->fIgnoreLocked;
	pso->fIceable = psoBase->fIceable;
	pso->fRoot = psoBase->fRoot;

	pso->fPhys = savedFPhys;

	pso->fNoGravity = psoBase->fNoGravity;
	pso->fCenterXp = psoBase->fCenterXp;
	pso->fLockedSelf = psoBase->fLockedSelf;
	pso->fLockedAll = psoBase->fLockedAll;
	pso->fLockedAbove = psoBase->fLockedAbove;
	pso->fCpsoBuildContactGroup = psoBase->fCpsoBuildContactGroup;
	pso->fCpxpBuildArray = psoBase->fCpxpBuildArray;
	pso->fUpdateXaList1 = psoBase->fUpdateXaList1;
	pso->fUpdateXaList2 = psoBase->fUpdateXaList2;
	pso->fRecalcSwXpAll = psoBase->fRecalcSwXpAll;
	pso->fHandleDiveEffect = psoBase->fHandleDiveEffect;
	pso->fGenSpliceTouchEvents = psoBase->fGenSpliceTouchEvents;

	EnableSoPhys(pso, 1);

	if (pso->geomWorld.cpos != 0)
		CloneGeom(&psoBase->geomWorld, nullptr, &pso->geomWorld);

	if (!pso->bspc.absp.empty())
		CloneBspc(&psoBase->geomWorld, &psoBase->bspc, &pso->geomWorld, &pso->bspc);

	if (pso->geomCameraWorld.cpos != 0)
		CloneGeom(&psoBase->geomCameraWorld, nullptr, &pso->geomCameraWorld);

	if (!pso->bspcCamera.absp.empty())
		CloneBspc(&psoBase->geomCameraWorld, &psoBase->bspcCamera, &pso->geomCameraWorld, &pso->bspcCamera);

	pso->pstso = nullptr;
	pso->fClone = true;
}

void CloneSoPhys(SO* pso, SO* psoPhys, int cposExtra)
{
	glm::mat4 dmat(1.0f);
	GEOM geom;

	float radiusSqMax = 0.0f;

	for (int i = 0; i < psoPhys->geomLocal.cpos; ++i)
	{
		const glm::vec3& pos = psoPhys->geomLocal.apos[i];
		float radiusSq = glm::dot(pos, pos);

		if (radiusSqMax < radiusSq)
			radiusSqMax = radiusSq;
	}

	pso->sRadiusSelf = sqrtf(radiusSqMax);

	geom = psoPhys->geomLocal;

	// GEOM was a shallow aggregate in the original game.  Our GEOM owns its
	// arrays with std::vector, so the assignment above deep-copies the arrays
	// while leaving SURF/EDGE links aimed at psoPhys's storage.  Rebase those
	// links onto the temporary copy before CloneGeom performs its next clone.
	auto rebaseEdge = [&](EDGE* pedge) -> EDGE*
	{
		if (pedge == nullptr || psoPhys->geomLocal.aedge.empty())
			return nullptr;

		const EDGE* pedgeSrc = psoPhys->geomLocal.aedge.data();
		const std::ptrdiff_t iedge = pedge - pedgeSrc;

		if (iedge < 0 || iedge >= static_cast<std::ptrdiff_t>(psoPhys->geomLocal.aedge.size()))
			return nullptr;

		return &geom.aedge[iedge];
	};

	for (SURF& surf : geom.asurf)
	{
		surf.pedge = rebaseEdge(surf.pedge);
		surf.pedgeOther = rebaseEdge(surf.pedgeOther);
	}

	for (EDGE& edge : geom.aedge)
	{
		edge.pedgeNext = rebaseEdge(edge.pedgeNext);
		edge.pedgeOtherNext = rebaseEdge(edge.pedgeOtherNext);
	}

	int cposOriginal = geom.cpos;
	geom.cpos += cposExtra;

	if (geom.cpos < cposOriginal)
		geom.cpos = cposOriginal;

	geom.apos.resize(geom.cpos);

	for (int i = cposOriginal; i < geom.cpos; ++i)
		geom.apos[i] = glm::vec3(0.0f);

	LoadMatrixFromPosRot(&psoPhys->xf.pos, &psoPhys->xf.mat, &dmat);
	CloneGeom(&geom, &dmat, &pso->geomLocal);

	if (pso->geomLocal.apos.size() < static_cast<size_t>(pso->geomLocal.cpos))
		pso->geomLocal.apos.resize(pso->geomLocal.cpos);

	LoadMatrixFromPosRot(&pso->xf.posWorld, &pso->xf.matWorld, &dmat);
	CloneGeom(&pso->geomLocal, &dmat, &pso->geomWorld);

	if (pso->geomWorld.apos.size() < static_cast<size_t>(pso->geomWorld.cpos))
		pso->geomWorld.apos.resize(pso->geomWorld.cpos);

	pso->grfpvaXpValid = 0;

	CloneBspc(&psoPhys->geomWorld, &psoPhys->bspc, &pso->geomWorld, &pso->bspc);
	CloneBspc(&psoPhys->geomCameraWorld, &psoPhys->bspcCamera, &pso->geomCameraWorld, &pso->bspcCamera);

	EnsureSoLvo(pso);
}

void SetSoParent(SO* pso, ALO* paloParent)
{
	if (pso->paloParent == paloParent)
		return;

	SO* oldRoot = (SO*)pso->paloRoot;

	glm::vec3 normalForceWorld;
	glm::vec3 normalTorqueWorld;

	ConvertAloVec(pso->paloParent, nullptr, &pso->constrForce.normal, &normalForceWorld);
	ConvertAloVec(pso->paloParent, nullptr, &pso->constrTorque.normal, &normalTorqueWorld);

	SetAloParent(pso, paloParent);

	ConvertAloVec(nullptr, paloParent, &normalForceWorld, &pso->constrForce.normal);
	ConvertAloVec(nullptr, paloParent, &normalTorqueWorld, &pso->constrTorque.normal);

	if (pso->paloRoot != nullptr)
	{
		if (pso->fLockedSelf && oldRoot != nullptr)
			RecalcSoLocked(oldRoot);

		SO* pso1 = (SO*)pso->paloRoot;

		if (!pso1->fLockedAll)
			RecalcSoLocked(pso);
		else
			RecalcSoLocked((SO*)pso->paloRoot);
	}

	RebuildSoPhysHook(pso);
}

void ApplySoProxy(SO* pso, PROXY* pproxyApply)
{
	ApplyAloProxy(pso, pproxyApply);

	glm::mat3 &mat = pproxyApply->xf.mat;

	glm::vec3 normalForce  = mat * pso->constrForce.normal;
	glm::vec3 normalTorque = mat * pso->constrTorque.normal;

	SetSoConstraints(pso, pso->constrForce.ct, &normalForce, pso->constrTorque.ct, &normalTorque);
}

void SetSoConstraints(SO* pso, CT ctForce, glm::vec3* pnormalForce, CT ctTorque, glm::vec3* pnormalTorque)
{
	bool fLockedSelfOld = pso->fLockedSelf != 0;
	pso->constrForce.ct = ctForce;

	if (pnormalForce != nullptr)
		pso->constrForce.normal = *pnormalForce;

	pso->constrTorque.ct = ctTorque;

	if (pnormalTorque != nullptr)
		pso->constrTorque.normal = *pnormalTorque;

	if (pso->paloRoot != nullptr)
	{
		bool fLockedSelfNew = (ctForce == CT_Locked && ctTorque == CT_Locked);

		if (fLockedSelfOld != fLockedSelfNew)
			RecalcSoLocked((SO*)pso->paloRoot);
	}
}

void EnsureSoLvo(SO* pso)
{
	if (pso->plvo != nullptr)
		return;

	auto plvo = std::make_shared<LVO>();

	plvo->mpiposvLocal.resize(pso->geomWorld.cpos);
	plvo->mpiposdvLocal.resize(pso->geomWorld.cpos);
	plvo->mpisurfsurfx.resize(pso->geomWorld.csurf);

	std::vector<int> mpisurfCpos(pso->geomWorld.csurf, 0);

	for (int iedge = 0; iedge < pso->geomWorld.cedge; ++iedge)
	{
		EDGE& edge = pso->geomWorld.aedge[iedge];

		for (int iside = 0; iside < 2; ++iside)
		{
			int isurf = edge.aisurf[iside];

			if (isurf < 0 || isurf >= pso->geomWorld.csurf)
				continue;

			SURFX& surfx = plvo->mpisurfsurfx[isurf];
			int& cposSurf = mpisurfCpos[isurf];

			for (int i = 0; i < 2; ++i)
			{
				int ipos = edge.aipos[i];

				bool found = false;

				for (int j = 0; j < cposSurf; ++j)
				{
					if (surfx.aipos[j] == ipos)
					{
						found = true;
						break;
					}
				}

				if (!found && cposSurf < 3)
				{
					surfx.aipos[cposSurf] = ipos;
					++cposSurf;
				}
			}
		}
	}

	for (int isurf = 0; isurf < pso->geomWorld.csurf; ++isurf)
	{
		SURFX& surfx = plvo->mpisurfsurfx[isurf];

		if (mpisurfCpos[isurf] < 3)
			continue;

		int ipos0 = surfx.aipos[0];
		int ipos1 = surfx.aipos[1];
		int ipos2 = surfx.aipos[2];

		if (ipos0 < 0 || ipos0 >= pso->geomLocal.cpos ||
			ipos1 < 0 || ipos1 >= pso->geomLocal.cpos ||
			ipos2 < 0 || ipos2 >= pso->geomLocal.cpos)
		{
			continue;
		}

		const glm::vec3& p0 = pso->geomLocal.apos[ipos0];
		const glm::vec3& p1 = pso->geomLocal.apos[ipos1];
		const glm::vec3& p2 = pso->geomLocal.apos[ipos2];

		glm::vec3 dpos10 = p1 - p0;
		glm::vec3 dpos20 = p2 - p0;
		glm::vec3 cross = glm::cross(dpos10, dpos20);

		if (glm::dot(cross, pso->geomLocal.asurf[isurf].normal) < 0.0f)
			std::swap(surfx.aipos[1], surfx.aipos[2]);
	}

	pso->plvo = plvo;
}

void ProjectSoLvo(SO* pso, float dt)
{
	glm::vec3* pos = pso->geomLocal.apos.data();
	glm::vec3* vel = pso->plvo->mpiposvLocal.data();
	glm::vec3* acc = pso->plvo->mpiposdvLocal.data();

	float maxRadiusSq = 0.0f;

	for (int i = 0; i < pso->geomLocal.cpos; ++i)
	{
		pos[i] += vel[i] * dt + acc[i] * (dt * dt * 0.5f);
		vel[i] += acc[i] * dt;

		maxRadiusSq = glm::max(maxRadiusSq, glm::length2(pos[i]));
	}

	pso->sRadiusSelf = sqrtf(maxRadiusSq);

	SURF* surf   = pso->geomLocal.asurf.data();
	SURFX* surfx = pso->plvo->mpisurfsurfx.data();

	for (int i = 0; i < pso->geomLocal.csurf; ++i)
	{
		glm::vec3 dpos10 = pos[surfx[i].aipos[1]] - pos[surfx[i].aipos[0]];
		glm::vec3 dpos20 = pos[surfx[i].aipos[2]] - pos[surfx[i].aipos[0]];

		surf[i].normal = glm::normalize(glm::cross(dpos10, dpos20));
		surf[i].gDot   = glm::dot(surf[i].normal, pos[surf[i].ipos]);
	}
}

void ProjectSoTransform(SO* pso, float dt, int fForce)
{
	if (pso->plvo != nullptr) 
		ProjectSoLvo(pso, dt);
	
	ProjectAloTransform(pso, dt, fForce);
}

void ApplySoConstraintLocal(SO* pso, CONSTR* pconstr, glm::vec3* pvecLocal, glm::vec3* pvecConstr, glm::vec3* pvecRemain)
{
	ApplyConstr(pconstr, pvecLocal, pvecConstr, pvecRemain);
}

void ApplyConstr(CONSTR* constr, glm::vec3* vec, glm::vec3* vecConstr, glm::vec3* vecRemain)
{
	glm::vec3 localConstr;

	if (!vecConstr)
		vecConstr = &localConstr;

	glm::vec3 v = *vec;

	switch (constr->ct)
	{
		case CT_Free:
		{
			if (vecConstr != vec)
				*vecConstr = v;
			break;
		}

		case CT_Tangent:
		{
			glm::vec3 n = constr->normal;
			float d = glm::dot(v, n);

			// Remove component along the normal
			*vecConstr = v - n * d;
			break;
		}

		case CT_Project:
		{
			glm::vec3 n = constr->normal;
			float d = glm::dot(v, n);

			// Keep only component along the normal
			*vecConstr = n * d;
			break;
		}

		case CT_Locked:
		{
			*vecConstr = glm::vec3(0.0f);
			break;
		}

		default:
		{
			*vecConstr = v;
			break;
		}
	}

	if (vecRemain)
		*vecRemain = v - *vecConstr;
}

void AddSoAcceleration(SO* pso, glm::vec3* dv)
{
	pso->xf.dv += *dv;
}

void PresetSoAccel(SO* pso, float dt)
{
	XA* pxa = nullptr;

	if (pso->paloParent == nullptr)
	{
		if (!pso->fNoGravity)
		{
			glm::vec3 dv;

			ApplySoConstraintLocal(pso, &pso->constrForce, &pso->dvGravity, &dv, nullptr);
			AddSoAcceleration(pso, &dv);
		}
	}

	pxa = pso->pxa;

	while (pxa != nullptr)
	{
		SO* source = pxa->psoSource;
		source->pvtso->pfnAddSoExternalAccelerations(source, pxa, dt);
		pxa = pxa->pxaNextSource;
	}

}

void UpdateSoXfWorldHierarchy(SO* pso)
{
	UpdateAloXfWorldHierarchy(pso);

	glm::vec3 pos = pso->xf.posWorld;
	glm::mat3 mat = pso->xf.matWorld;

	UpdateGeomWorld(&pso->geomLocal, &pso->geomWorld, pos, mat);
	UpdateGeomWorld(&pso->geomCameraLocal, &pso->geomCameraWorld, pos, mat);
}

void UpdateSoXfWorld(SO* pso)
{
	UpdateAloXfWorld(pso);
	pso->paloRoot->pvtso->pfnUpdateSoBounds((SO*)pso->paloRoot);
	InvalidateSwXpForObject(pso->psw, pso, 7);
	InvalidateSwAaox(pso->psw);
}

void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis)
{
	pso->fFixedPhys = (pbis->U8Read() & 1) != 0;

	ReadGeom(&pso->geomLocal, pbis);

	pso->sRadiusSelf = pso->geomLocal.sRadius;
	pso->sRadiusAll  = pso->geomLocal.sRadius;

	if (!pso->fFixedPhys)
		CloneGeom(&pso->geomLocal, nullptr, &pso->geomWorld);
	else
		pso->geomWorld = pso->geomLocal;

	ReadBspc(&pso->geomWorld, &pso->bspc, pbis);

	pso->m = pbis->F32Read();

	pso->momintLocal =  pbis->ReadMatrix();
	pso->posComLocal = pbis->ReadVector();

	AdjustSoMomint(pso, 1.0);

	pso->cnpg = pbis->U16Read();
	pso->anpg.resize(pso->cnpg);

	for (int i = 0; i < pso->cnpg; i++)
	{
		pso->anpg[i].cmk = pbis->S16Read();
		pso->anpg[i].ipglob = pbis->U16Read();
	}

	const uint32_t bspNodeCount = pbis->U32Read();
	pso->mpibspinpg.resize(bspNodeCount);

	for (size_t i = 0; i < pso->mpibspinpg.size(); ++i)
		pso->mpibspinpg[i] = static_cast<int>(pbis->S16Read());

	pso->chsg = pbis->U32Read();
	pso->ahsg.resize(pso->chsg);

	for (int i = 0; i < pso->chsg; i++)
	{
		pso->ahsg[i].ipglob = pbis->S16Read();
		pso->ahsg[i].ipsubglob = pbis->S16Read();
	}

	if (pso->chsg > 0)
	{
		pso->mpisurfihsgMic.resize(pso->geomWorld.csurf + 1);

		for (int i = 0; i <= pso->geomWorld.csurf; i++)
			pso->mpisurfihsgMic[i] = pbis->S16Read();
	}
	
	ReadGeom(&pso->geomCameraLocal, pbis);

	if (!pso->fFixedPhys)
		CloneGeom(&pso->geomCameraLocal, nullptr, &pso->geomCameraWorld);
	else
		pso->geomCameraWorld = pso->geomCameraLocal;

	ReadBspc(&pso->geomCameraWorld, &pso->bspcCamera, pbis);
	LoadAloFromBrx(pso, pbis);
	EnableSoPhys(pso, true);

	pso->pvtso->pfnUpdateSoPosWorldPrev(pso);

	if (pso->paloParent == nullptr)
	{
		pso->pvtso->pfnUpdateSoBounds(pso);
		RecalcSwOxfFilterForObject(pso->psw, pso);
	}
}

bool FCalculateMuzzleVelocity(const glm::vec3& posLaunch, const glm::vec3& posTarget, float radVert, glm::vec3& muzzleVelocity, const SO* soLaunch)
{
	const glm::vec3& gravity = soLaunch ? soLaunch->dvGravity : g_psw->dvGravity;

	glm::vec3 sight = posTarget - posLaunch;

	float height = sight.z;

	// Horizontal distance only
	glm::vec3 horizontalSight = sight;
	horizontalSight.z = 0.0f;

	float horizontalDist = glm::length(horizontalSight);

	// Base angle to target
	float angle = std::atan2(height, horizontalDist);

	if (angle < 0.0f)
		angle = 0.0f;

	// Blend toward vertical based on radVert
	angle = angle + (glm::half_pi<float>() - angle) * std::sin(radVert);

	float tanAngle = std::tan(angle);

	// Derived from projectile motion:
	// gravity.z / (2 * (height - horizontalDist * tan(angle)))
	float denom = height - horizontalDist * tanAngle;
	float speedSqFactor = gravity.z / (denom + denom);

	if (speedSqFactor <= 0.0f)
		return false;

	float cosAngle = std::cos(angle);

	const float launchSpeed =
		(horizontalDist * std::sqrt(speedSqFactor)) / cosAngle;

	glm::vec3 dir = horizontalSight;

	if (glm::length(dir) > 0.0f)
		dir = glm::normalize(dir);

	// launchSpeed is the magnitude of the complete launch velocity, not its
	// horizontal component.  Resolve it along the chosen elevation angle.
	muzzleVelocity = dir * (launchSpeed * cosAngle);
	muzzleVelocity.z = launchSpeed * std::sin(angle);

	return true;
}

bool FCalculateMuzzleVelocitySpeed(const glm::vec3& posLaunch, const glm::vec3& posTarget, float svMuzzle, glm::vec3& vecMuzzle, SO* psoLaunch)
{
	const glm::vec3 gravity = psoLaunch
		? psoLaunch->dvGravity
		: g_psw->dvGravity;

	glm::vec3 sight = posTarget - posLaunch;

	const float height = sight.z;
	const float clampedHeight = std::max(height, 0.0f);

	sight.z = 0.0f;

	const float horizontalDist = glm::length(sight);

	if (horizontalDist <= 0.0f)
		return false;

	const glm::vec3 horizontalDir = glm::normalize(sight);

	const float g = gravity.z;
	const float v2 = svMuzzle * svMuzzle;

	// Solve quadratic in flight time squared.
	const float b = g * clampedHeight + v2;
	const float a = 0.25f * g * g;

	const float discriminant =
		b * b - 4.0f * a * (horizontalDist * horizontalDist + clampedHeight * clampedHeight);

	if (discriminant < 0.0f)
		return false;

	const float sqrtDisc = std::sqrt(discriminant);

	float t2 = (b - sqrtDisc) / (2.0f * a);

	if (t2 <= 0.0f)
		t2 = (b + sqrtDisc) / (2.0f * a);

	if (t2 <= 0.0f)
		return false;

	const float t = std::sqrt(t2);

	vecMuzzle = horizontalDir * (horizontalDist / t);
	vecMuzzle.z = (clampedHeight - 0.5f * g * t * t) / t;

	return true;
}

bool FCalculateMuzzleVelocityAngle(const glm::vec3& posLaunch, const glm::vec3& posTarget, float radTilt, glm::vec3& vecMuzzle, SO* psoLaunch)
{
	return FCalculateMuzzleVelocity(posLaunch, posTarget, radTilt, vecMuzzle, psoLaunch);
}

void TranslateSoToPos(SO* pso, glm::vec3* ppos)
{
	CM* pcm = g_pcm;
	MSGTRANS msgtrans;
	msgtrans.palo = reinterpret_cast<ALO*>(pso);
	msgtrans.posWorldPrev = pso->xf.posWorld;

	if (pso->fActive)
	{
		const glm::vec3 oldPos(pso->xf.pos.x, pso->xf.pos.y, pso->xf.pos.z);

		const float dist = glm::length(oldPos - *ppos);

		if (dist > 10.0f)
		{
			g_pcm->psoFocusPrev = nullptr;
			SetCmCut(pcm, nullptr);
		}
	}

	pso->xf.pos = *ppos;
	SO* root = (SO*)pso->paloRoot;

	if (root != nullptr)
	{
		if (pso->pvtalo->pfnUpdateAloXfWorld != nullptr)
			pso->pvtalo->pfnUpdateAloXfWorld(pso);

		DiscardSoXps(pso, 1);

		for (SO* phys = root->dlPhys.psoFirst; phys != nullptr; phys = phys->dlePhys.psoNext)
		{
			if (pso == root || pso == phys || FFindLoParent(reinterpret_cast<LO*>(phys), reinterpret_cast<ALO*>(pso)))
				phys->pvtso->pfnUpdateSoPosWorldPrev(phys);
		}
	}

	pso->pvtlo->pfnSendLoMessage(pso, MSGID_translated, &msgtrans);
}

void TranslateSoToPosSafe(SO* pso, glm::vec3* ppos, glm::vec3* pdpos)
{
	int tries = 9;
	glm::vec3 pos = *ppos;

	retry:
	pso->pvtalo->pfnTranslateAloToPos(pso, &pos);
	RecalcSwXpAll((SW*)pso->psw, 0);

	OX* pox = pso->poxa->pox;

	while (pox)
	{
		XP* pxp = pox->pxp;

		while (pxp)
		{
			if (pxp->gDist < -2.0f)
			{
				tries--;
				pos += *pdpos;

				if (tries < 0)
					return;

				goto retry;
			}

			pxp = pxp->pxpNext;
		}

		pox = pox->poxNext;
	}
}

void RotateSoToMat(SO* pso, glm::mat3& pmat)
{
	pso->xf.mat = pmat;

	if (pso->paloRoot != nullptr)
	{
		if (pso->pvtso->pfnUpdateSoXfWorld != nullptr)
			pso->pvtso->pfnUpdateSoXfWorld(pso);

		DiscardSoXps(pso, 1);
	}

	pso->pvtlo->pfnSendLoMessage(pso, MSGID_rotated, pso);
}

void ApplySoImpulse(SO* pso, glm::vec3* ppos, glm::vec3* pv, float sftMax)
{
	glm::vec3 v;

	CalculateAloMovement(pso, nullptr, *ppos, &v, nullptr, nullptr, nullptr);

	glm::vec3 delta = *pv - v;

	glm::vec3 normal;
	if (glm::length(delta) < 0.0001f)
		normal = g_normalZ;
	else
		normal = glm::normalize(delta);

	XP* pxp = nullptr;

	PxpAdd(pso, nullptr, ppos, &normal, 0.0f, XPK_Limited, nullptr, nullptr, &pxp);

	pxp->axpd[1].v = *pv;
	pxp->elas = 0.0f;
	pxp->mu = 1.0f;
	pxp->sfMax = sftMax;

	RecalcSwXpMovement(pso->psw, 1, &pxp);

	SO* root = (SO*)pso->paloRoot;

	SolveSwGroup(pso->psw, 1, &root, 1, &pxp, 0.0f, 2);
	FreeSwXpList(pso->psw, pxp);
}

void UpdateSo(SO* pso, float dt)
{
	UpdateAlo(pso, dt);
}

void DisplaceSo(SO* pso, int fParentDirty)
{
	int fDirty = fParentDirty;

	const glm::vec3 dposAbs = glm::abs(pso->dpos);
	const bool hasDpos =
		dposAbs.x >= 0.0001f ||
		dposAbs.y >= 0.0001f ||
		dposAbs.z >= 0.0001f;

	if (hasDpos)
	{
		pso->xf.pos += pso->dpos;

		if (pso->pactPos)
			AdaptAct(pso->pactPos);

		pso->dpos = glm::vec3(0.0f);
		fDirty = 1;
	}

	const glm::vec3 drotAbs = glm::abs(pso->drot);
	const bool hasDrot =
		drotAbs.x >= 0.0001f ||
		drotAbs.y >= 0.0001f ||
		drotAbs.z >= 0.0001f;

	if (hasDrot)
	{
		glm::mat3 matRot;

		LoadRotateMatrix(&pso->drot, &matRot);

		pso->xf.mat = matRot * pso->xf.mat;

		if (pso->pactRot)
			AdaptAct(pso->pactRot);

		pso->drot = glm::vec3(0.0f);
		fDirty = 1;
	}

	DLI dli;
	DL* pdl = &pso->dlChild;
	LO* plo = pdl->ploFirst;

	dli.m_pdl = pdl;
	dli.m_ibDle = pdl->ibDle;
	dli.m_pdliNext = s_pdliFirst;
	s_pdliFirst = &dli;

	while (plo)
	{
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<char*>(plo) + dli.m_ibDle);

		LO* ploNext = static_cast<LO*>(*dli.m_ppv);

		if ((plo->pvtso->grfcid & 2U) != 0)
			plo->pvtso->pfnDisplaceSo((SO*)plo, fDirty);

		plo = ploNext;
	}

	if (fDirty && !fParentDirty && pso->pvtso->pfnUpdateSoXfWorld)
		pso->pvtso->pfnUpdateSoXfWorld(pso);

	s_pdliFirst = dli.m_pdliNext;
}

void ImpactSo(SO* pso, int fParentDirty)
{
	if (glm::length2(pso->dpos) >= 0.0001f * 0.0001f)
	{
		pso->xf.v += pso->dpos;

		if (pso->pactPos)
			AdaptAct(pso->pactPos);

		pso->dpos = glm::vec3(0.0f);
		fParentDirty = 1;
	}

	if (glm::length2(pso->drot) >= 0.0001f * 0.0001f)
	{
		pso->xf.w += pso->drot;

		if (pso->pactRot)
			AdaptAct(pso->pactRot);

		pso->drot = glm::vec3(0.0f);
		fParentDirty = 1;
	}

	DLI dli;
	DL* pdl = &pso->dlChild;
	LO* plo = pdl->ploFirst;

	dli.m_pdl = pdl;
	dli.m_ibDle = pdl->ibDle;
	dli.m_pdliNext = s_pdliFirst;
	s_pdliFirst = &dli;

	while (plo)
	{
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<char*>(plo) + dli.m_ibDle);

		LO* ploNext = static_cast<LO*>(*dli.m_ppv);

		if ((plo->pvtso->grfcid & 2U) != 0)
			plo->pvtso->pfnImpactSo((SO*)plo, fParentDirty);

		plo = ploNext;
	}

	s_pdliFirst = dli.m_pdliNext;
}

void PivotSo(SO* pso, int fParentDirty)
{
	if (pso == nullptr)
		return;

	int fDirty = fParentDirty;

	const glm::vec3 dposAbs = glm::abs(pso->dpos);
	const bool hasDpos = dposAbs.x >= 0.0001f || dposAbs.y >= 0.0001f || dposAbs.z >= 0.0001f;

	if (hasDpos)
	{
		pso->xf.v += pso->dpos;
		pso->xf.pos += pso->dpos * (g_clock.dt * 0.5f);

		if (pso->pactPos != nullptr)
			AdaptAct(pso->pactPos);

		pso->dpos = glm::vec3(0.0f);
		fDirty = 1;
	}

	const glm::vec3 drotAbs = glm::abs(pso->drot);
	const bool hasDrot = drotAbs.x >= 0.0001f || drotAbs.y >= 0.0001f || drotAbs.z >= 0.0001f;

	if (hasDrot)
	{
		pso->xf.w += pso->drot;

		glm::vec3 rot = pso->drot * (g_clock.dt * 0.5f);
		glm::mat3 dmat(1.0f);

		LoadRotateMatrix(&rot, &dmat);

		pso->xf.mat = dmat * pso->xf.mat;

		if (pso->pactRot != nullptr)
			AdaptAct(pso->pactRot);

		pso->drot = glm::vec3(0.0f);
		fDirty = 1;
	}

	DLI dli{};

	dli.m_pdl = &pso->dlChild;
	dli.m_ibDle = pso->dlChild.ibDle;
	dli.m_pdliNext = s_pdliFirst;

	LO* plo = pso->dlChild.ploFirst;

	dli.m_ppv = plo != nullptr ? reinterpret_cast<void**>(reinterpret_cast<std::byte*>(plo) + dli.m_ibDle) : nullptr;

	s_pdliFirst = &dli;

	while (plo != nullptr)
	{
		if (plo->pvtlo != nullptr && (plo->pvtlo->grfcid & 2U) != 0)
		{
			SO* psoChild = static_cast<SO*>(static_cast<ALO*>(plo));

			if (psoChild->pvtso != nullptr && psoChild->pvtso->pfnPivotSo != nullptr)
				psoChild->pvtso->pfnPivotSo(psoChild, fDirty);
		}

		plo = dli.m_ppv != nullptr ? static_cast<LO*>(*dli.m_ppv) : nullptr;

		dli.m_ppv = plo != nullptr ? reinterpret_cast<void**>(reinterpret_cast<std::byte*>(plo) + dli.m_ibDle) : nullptr;
	}

	if (fDirty != 0 && fParentDirty == 0 && pso->pvtso != nullptr && pso->pvtso->pfnUpdateSoXfWorld != nullptr)
		pso->pvtso->pfnUpdateSoXfWorld(pso);

	s_pdliFirst = dli.m_pdliNext;
}
void ExtendSoBounds(SO* pso, glm::vec3* ppos, float sRadius)
{
	glm::vec3 pos = glm::vec3(ppos->x, ppos->y, ppos->z);
	glm::vec3 posWorld = glm::vec3( pso->xf.posWorld.x, pso->xf.posWorld.y, pso->xf.posWorld.z);

	glm::vec3 radiusVec(sRadius);

	// Extend min/max bounds around this position.
	glm::vec3 posMin = glm::vec3(pso->posMin.x, pso->posMin.y, pso->posMin.z);
	glm::vec3 posMax = glm::vec3(pso->posMax.x, pso->posMax.y, pso->posMax.z);

	posMin = glm::min(posMin, pos - radiusVec);
	posMax = glm::max(posMax, pos + radiusVec);

	pso->posMin = glm::vec3(posMin.x, posMin.y, posMin.z);
	pso->posMax = glm::vec3(posMax.x, posMax.y, posMax.z);

	// Radius from object/world center to this point.
	float radiusAll = glm::length(pos - posWorld) + sRadius;

	if (pso->sRadiusAll < radiusAll)
		pso->sRadiusAll = radiusAll;
	else if (radiusAll > 0.0f)
		pso->sRadiusAll = radiusAll;

	// Radius from prune center to this point.
	glm::vec3 posPrune = glm::vec3(pso->posPrune.x, pso->posPrune.y, pso->posPrune.z);

	float radiusPrune = glm::length(pos - posPrune) + sRadius;

	if (pso->sRadiusPrune < radiusPrune)
		pso->sRadiusPrune = radiusPrune;
}

void SetSoCenterXp(SO* pso, int fCenterXp)
{
	pso->fCenterXp = (fCenterXp & 1) != 0;
}

void GetSoCenterXp(SO* pso, int* pfCenterXp)
{
	*pfCenterXp = pso->fCenterXp;
}

void RenumberSo(SO* pso, int* pcpso, SO** apso)
{
	pso->ipso = *pcpso;
	apso[*pcpso] = pso;
	++(*pcpso);

	for (LO* plo = pso->dlChild.ploFirst; plo != nullptr; plo = plo->dleChild.ploNext)
	{
		if ((plo->pvtlo->grfcid & 2U) != 0)
			plo->pvtso->pfnRenumberSo((SO*)plo, pcpso, apso);
	}
}

int FIgnoreSoIntersection(SO* pso, SO* psoOther)
{
	if (pso->fIgnoreLocked || pso->fLockedAll)
	{
		if (psoOther->fLockedAll)
			return 1;
	}

	return 0;
}

void RecalcSoLocked(SO* pso)
{
	bool fLockedSelf = pso->constrForce.ct == CT_Locked && pso->constrTorque.ct == CT_Locked;
	bool fLockedAbove = fLockedSelf && (pso->paloParent == nullptr || ((SO*)pso->paloParent)->fLockedAbove);
	bool fLockedAll = fLockedSelf;

	pso->fLockedSelf = fLockedSelf;
	pso->fLockedAbove = fLockedAbove;

	for (SO* psoChild = pso->dlChild.psoFirst; psoChild != nullptr; psoChild = psoChild->dleChild.psoNext) {
		if ((psoChild->pvtlo->grfcid & 2) == 0) {
			continue;
		}

		RecalcSoLocked(psoChild);

		if (!psoChild->fLockedAll) {
			fLockedAll = false;
		}
	}

	if (pso->fLockedAll != fLockedAll) {
		pso->fLockedAll = fLockedAll;

		if (pso->paloParent == nullptr && pso->paloRoot != nullptr) {
			RecalcSwOxfFilterForObject(pso->psw, pso);
		}
	}
}
void AdjustSoXpLocal(SO* pso, XP* pxp, int ixpd)
{
	if (!pso->plvo)
		return;

	glm::vec3* apos = pso->geomWorld.apos.data();
	SURF* asurf = pso->geomWorld.asurf.data();

	int aipos[3]{};
	float weights[3]{};
	int cipos = 0;

	if (ixpd == 0)
	{
		switch (pxp->xpk)
		{
			case XPK_PointToSurface:
			cipos = 1;
			aipos[0] = int(static_cast<glm::vec3*>(pxp->pv0) - apos);
			break;

			case XPK_EdgeToEdge:
			case XPK_EdgeToSurface:
			cipos = 2;
			aipos[0] = pxp->pedge0->aipos[0];
			aipos[1] = pxp->pedge0->aipos[1];
			break;
		}
	}
	else
	{
		switch (pxp->xpk)
		{
			case XPK_PointToSurface:
			case XPK_EdgeToSurface:
			case XPK_SphereToSurface:
			case XPK_CenterToSurface:
			{
				cipos = 3;
				int isurf = int(static_cast<SURF*>(pxp->pv1) - asurf);

				aipos[0] = pso->plvo->mpisurfsurfx[isurf].aipos[0];
				aipos[1] = pso->plvo->mpisurfsurfx[isurf].aipos[1];
				aipos[2] = pso->plvo->mpisurfsurfx[isurf].aipos[2];
				break;
			}

			case XPK_EdgeToEdge:
			case XPK_SphereToEdge:
			cipos = 2;
			aipos[0] = pxp->pedge1->aipos[0];
			aipos[1] = pxp->pedge1->aipos[1];
			break;

			case XPK_SphereToPoint:
			cipos = 1;
			aipos[0] = int(static_cast<glm::vec3*>(pxp->pv1) - apos);
			break;
		}
	}

	if (cipos == 0)
		return;

	XPD* pxpd = &pxp->axpd[ixpd];

	if (cipos == 1)
		weights[0] = 1.0f;
	else if (cipos == 2)
	{
		float u = UProjectVector(&apos[aipos[0]], &apos[aipos[1]], &pxpd->pos);
		weights[1] = glm::clamp(u, 0.0f, 1.0f);
		weights[0] = 1.0f - weights[1];
	}
	else
	{
		for (int i = 0; i < 2; ++i)
		{
			int i0 = aipos[i];
			int i1 = aipos[i + 1];
			int i2 = aipos[(i + 2) % 3];

			float u = UProjectVector(&apos[i0], &apos[i1], &apos[i2]);
			glm::vec3 v = apos[i0] * u - apos[i1] * u + apos[i2];

			weights[i] = UProjectVector(&apos[i0], &v, &pxpd->pos);
		}

		weights[0] = glm::clamp(weights[0], 0.0f, 1.0f);
		weights[1] = glm::clamp(weights[1], 0.0f, 1.0f - weights[0]);
		weights[2] = 1.0f - weights[0] - weights[1];
	}

	if (pso->plvo->mpiposvLocal.size() == 0)
		return;

	glm::vec3 v(0.0f);

	for (int i = 0; i < cipos; ++i)
		v += pso->plvo->mpiposvLocal[aipos[i]] * weights[i];

	ConvertAloVec(pso, nullptr, &v, &v);

	pxpd->v += v;
}

void AdjustSoZapXps(SO* psoRoot, XP** ppxpZap)
{
	XP* pxpFree = nullptr;

	if (psoRoot->paloRoot != psoRoot)
		return;

	for (OX* pox = psoRoot->poxa->pox; pox != nullptr; pox = pox->poxNext) {
		XP** ppxp = &pox->pxp;

		while (*ppxp != nullptr) {
			XP* pxp = *ppxp;

			SO* psoRootXp = pxp->axpd[0].psoRoot;
			int leafIndex = (psoRootXp == psoRoot) ? 1 : 0;

			SO* psoLeaf = pxp->axpd[leafIndex].psoLeaf;

			while (psoLeaf != nullptr && psoLeaf->zok == ZOK_Inherit) {
				psoLeaf = static_cast<SO*>(psoLeaf->paloParent);
			}

			if (psoLeaf == nullptr) {
				ppxp = &pxp->pxpNext;
				continue;
			}

			XP* pxpNext = pxp->pxpNext;

			switch (psoLeaf->zok) 
			{
				case ZOK_Ignore:
				*ppxp = pxpNext;
				pxp->pxpNext = pxpFree;
				pxpFree = pxp;
				break;

				case ZOK_Zap: 
				{
					auto pxpCopyOwned = std::make_unique<XP>(*pxp);
					XP* pxpCopy = pxpCopyOwned.get();

					psoRoot->psw->axp.push_back(std::move(pxpCopyOwned));

					pxpCopy->pxpNext = *ppxpZap;
					*ppxpZap = pxpCopy;

					ppxp = &pxp->pxpNext;
					break;
				}

				case ZOK_ZapAndIgnore:
				*ppxp = pxpNext;
				pxp->pxpNext = *ppxpZap;
				*ppxpZap = pxp;
				break;

				default:
				ppxp = &pxp->pxpNext;
				break;
			}
		}

		pox->poxOther->pxp = pox->pxp;
	}

	FreeSwXpList(psoRoot->psw, pxpFree);
}

float UProjectVector(glm::vec3* ppos0, glm::vec3* ppos1, glm::vec3* ppos)
{
	glm::vec3 dpos = *ppos - *ppos0;
	glm::vec3 dpos01 = *ppos1 - *ppos0;

	return glm::dot(dpos, dpos01) / glm::dot(dpos01, dpos01);
}

int FInflictSoZap(SO* pso, XP* pxp, ZPR* pzpr)
{
	return 0;
}

void UpdateSoBounds(SO* pso)
{
	glm::vec3 posMin(FLT_MAX);
	glm::vec3 posMax(-FLT_MAX);

	float sRadiusAll = 0.0f;

	const glm::vec3 rootPos(pso->xf.posWorld.x, pso->xf.posWorld.y, pso->xf.posWorld.z);

	for (SO* child = pso->dlPhys.psoFirst; child != nullptr; child = child->dlePhys.psoNext)
	{
		if (!child->fSphere)
		{
			if (child->bspc.absp.size() == 0)
				continue;

			for (int i = 0; i < child->geomWorld.cpos; ++i)
			{
				glm::vec3& v = child->geomWorld.apos[i];
				glm::vec3 p(v.x, v.y, v.z);

				posMin = glm::min(posMin, p);
				posMax = glm::max(posMax, p);
			}

			for (int i = 0; i < child->geomCameraWorld.cpos; ++i)
			{
				glm::vec3& v = child->geomCameraWorld.apos[i];
				glm::vec3 p(v.x, v.y, v.z);

				posMin = glm::min(posMin, p);
				posMax = glm::max(posMax, p);
			}
		}
		else
		{
			glm::vec3 center(child->xf.posWorld.x, child->xf.posWorld.y, child->xf.posWorld.z);
			glm::vec3 radius(child->sRadiusSelf);

			posMin = glm::min(posMin, center - radius);
			posMax = glm::max(posMax, center + radius);
		}

		if (!child->fNoXpsCenter)
		{
			glm::vec3 prev(child->posWorldPrev.x, child->posWorldPrev.y, child->posWorldPrev.z);

			posMin = glm::min(posMin, prev);
			posMax = glm::max(posMax, prev);
		}

		glm::vec3 childPos(child->xf.posWorld.x, child->xf.posWorld.y, child->xf.posWorld.z);

		const float radius = glm::length(childPos - rootPos) + child->sRadiusSelf;

		if (sRadiusAll < radius)
			sRadiusAll = radius;
	}

	if (sRadiusAll == 0.0f)
	{
		pso->sRadiusAll = 0.0f;
		pso->sRadiusPrune = 0.0f;

		pso->posMin = pso->xf.posWorld;
		pso->posMax = pso->xf.posWorld;
		pso->posPrune = pso->xf.posWorld;

		return;
	}

	pso->sRadiusAll = sRadiusAll;

	pso->posMin.x = posMin.x;
	pso->posMin.y = posMin.y;
	pso->posMin.z = posMin.z;

	pso->posMax.x = posMax.x;
	pso->posMax.y = posMax.y;
	pso->posMax.z = posMax.z;

	glm::vec3 posPrune = (posMin + posMax) * 0.5f;

	pso->posPrune.x = posPrune.x;
	pso->posPrune.y = posPrune.y;
	pso->posPrune.z = posPrune.z;

	float sRadiusPrune = 0.0f;

	for (SO* child = pso->dlPhys.psoFirst; child != nullptr; child = child->dlePhys.psoNext)
	{
		if (!child->fSphere && child->bspc.absp.size() == 0)
			continue;

		glm::vec3 childPos(child->xf.posWorld.x, child->xf.posWorld.y, child->xf.posWorld.z);

		float radius = glm::length(childPos - posPrune) + child->sRadiusSelf;

		if (sRadiusPrune < radius)
			sRadiusPrune = radius;

		if (!child->fNoXpsCenter)
		{
			glm::vec3 prev(child->posWorldPrev.x, child->posWorldPrev.y, child->posWorldPrev.z);

			float prevDistSq = glm::dot(prev - posPrune, prev - posPrune);
			float currentMaxSq = sRadiusPrune * sRadiusPrune;

			if (currentMaxSq < prevDistSq)
				sRadiusPrune = sqrtf(prevDistSq);
		}
	}

	pso->sRadiusPrune = sRadiusPrune;
}
void UpdateSoImpacts(SO* pso)
{
	OX* pox = pso->poxa->pox;

	while (pox != nullptr) {
		for (XP* pxp = pox->pxp; pxp != nullptr; pxp = pxp->pxpNext) {
			if (pxp->sftImpact <= 0.0001f) {
				continue;
			}

			for (int ixpd = 0; ixpd < 2; ixpd++) {
				SO* psoHook = PsoFindSoPhysHook(pxp->axpd[ixpd].psoLeaf, offsetof(VTSO, pfnNotifySoImpact));

				if (psoHook != nullptr && psoHook->pvtso->pfnNotifySoImpact != nullptr) {
					psoHook->pvtso->pfnNotifySoImpact(psoHook, pxp, ixpd);
				}
			}

			pxp->sftImpact = 0.0f;
		}

		pox = pox->poxNext;
	}
}

void UpdateSoPosWorldPrev(SO* pso)
{
	pso->posWorldPrev = pso->xf.posWorld;
}

SO* PsoHitTestLineObjects(GRFHTL grfhtl, glm::vec3* ppos0, glm::vec3* ppos1, std::vector <SO*> apso, LSG* plsgBest)
{
	SO* psoBest = nullptr;
	float uBest = std::numeric_limits<float>::max();

	for (SO* pso : apso)
	{
		if (pso == nullptr)
			continue;

		LSG alsg[2];
		int clsg = ClsgClipEdgeToObject(pso, ppos0, ppos1, 2, alsg);

		if (clsg <= 0)
			continue;

		int ilsg = 0;

		/*
		 * When the line begins inside/on an object, bit 0 requests the
		 * second intersection—the point where the line exits the object.
		 */
		if (alsg[0].au[0] == 0.0f && (grfhtl & 1) != 0)
			ilsg = 1;

		if (ilsg >= clsg)
			continue;

		float uHit = alsg[ilsg].au[0];

		if (uHit >= uBest)
			continue;

		uBest = uHit;
		psoBest = pso;

		if (plsgBest != nullptr)
			*plsgBest = alsg[ilsg];
	}

	return psoBest;
}

void GetSoCpdefi(SO *pso, float dt, CPDEFI* pcpdefi)
{
	(void)dt;
	pcpdefi->posBase = pso->xf.posWorld;
	pcpdefi->vBase = pso->xf.v;

	glm::mat3 uprightMat;
	TiltMatUpright(&pso->xf.matWorld, nullptr, &uprightMat);
	
	pcpdefi->radHome = atan2f(-uprightMat[0][1], -uprightMat[0][0]);

	pcpdefi->sAdjust = 100.0f;
	pcpdefi->grfcpd = 0;
	pcpdefi->cfk = CFK_Behind;
	pcpdefi->tMoveLast = g_clock.t;
}

void AddSoWaterAcceleration(SO* pso, WATER* pwater, float dt)
{
	glm::vec3 posSurface;
	glm::vec3 normalSurface;

	float submerged = UGetWaterSubmerged(pwater, pso, &posSurface, &normalSurface);

	if (submerged < 0.0001f) {
		return;
	}

	if (pwater->globset.pwrbgFirst != nullptr) {
		glm::mat3 normalBasis;

		normalBasis[2] = normalSurface;

		GetNormalVectors(&normalSurface, &normalBasis[0], &normalBasis[1], &g_normalX);
		WarpWrTransform(pwater->globset.pwrbgFirst->pwr, 50.0f, &posSurface, &normalBasis, nullptr, &normalBasis, nullptr);

		normalSurface = glm::cross(normalBasis[0], normalBasis[1]);

		if (glm::length(normalSurface) < 0.0001f) {
			normalSurface = g_normalZ;
		}
		else {
			normalSurface = glm::normalize(normalSurface);
		}
	}

	float buoyancy = 0.0f;

	if (!pso->fNoGravity) {
		buoyancy = pso->gBuoyancy * pwater->gBuoyancy;
	}

	float viscosity = pso->gViscosity * pwater->gViscosity;

	if (buoyancy != 0.0f) {
		glm::vec3 dv = pso->dvGravity * (-buoyancy * submerged);

		ApplySoConstraintLocal(pso, &pso->constrForce, &dv, &dv, nullptr);
		AddSoAcceleration(pso, &dv);
	}

	if (viscosity != 0.0f) {
		glm::vec3 currentLinear;
		glm::vec3 currentAngular;

		CalculateWaterCurrent(pwater, &pso->xf.posWorld, &currentLinear, &currentAngular);

		glm::vec3 dv = currentLinear - pso->xf.v;
		glm::vec3 dvNormal = glm::dot(dv, normalSurface) * normalSurface;
		glm::vec3 dvTangent = dv - dvNormal;

		float normalScale = 1.0f;

		if (submerged != 1.0f && glm::dot(dv, normalSurface) <= 0.0f) {
			normalScale = submerged * 0.5f + 0.5f;
		}

		CLQ clq;

		clq.g0 = s_clqDampV.g0 * normalScale * viscosity;
		clq.g1 = s_clqDampV.g1 * normalScale * viscosity;
		clq.g2 = s_clqDampV.g2 * normalScale * viscosity;

		glm::vec3 vNormal = pso->xf.v + dvNormal;
		AccelSoTowardPosSpring(pso, nullptr, nullptr, &vNormal, &clq, dt);

		clq.g0 = s_clqDampV.g0 * submerged * viscosity;
		clq.g1 = s_clqDampV.g1 * submerged * viscosity;
		clq.g2 = s_clqDampV.g2 * submerged * viscosity;

		glm::vec3 vTangent = pso->xf.v + dvTangent;
		AccelSoTowardPosSpring(pso, nullptr, nullptr, &vTangent, &clq, dt);

		glm::vec3 dw = -pso->xf.w;
		glm::vec3 dwNormal = glm::dot(dw, normalSurface) * normalSurface;
		glm::vec3 dwTangent = dw - dwNormal;

		clq.g0 = s_clqDampW.g0 * submerged * viscosity;
		clq.g1 = s_clqDampW.g1 * submerged * viscosity;
		clq.g2 = s_clqDampW.g2 * submerged * viscosity;

		glm::vec3 wNormal = pso->xf.w + dwNormal;
		AccelSoTowardMatSpring(pso, nullptr, nullptr, &wNormal, &clq, dt);

		clq.g0 = s_clqDampW.g0 * viscosity;
		clq.g1 = s_clqDampW.g1 * viscosity;
		clq.g2 = s_clqDampW.g2 * viscosity;

		glm::vec3 wTangent = pso->xf.w + dwTangent;
		AccelSoTowardMatSpring(pso, nullptr, nullptr, &wTangent, &clq, dt);
	}

	if (buoyancy != 0.0f) {
		glm::mat3 matAdjusted = pso->matOrig;
		glm::vec3 posAdjusted;

		pso->pvtalo->pfnAdjustAloRotation(pso, &matAdjusted, &posAdjusted);

		glm::mat3 surfaceRotation;
		BuildRotateVectorsMatrix(&g_normalZ, &normalSurface, &surfaceRotation);

		glm::mat3 matTarget = surfaceRotation * matAdjusted;

		glm::vec3 worldZ = pso->xf.matWorld[2];
		glm::vec3 targetZ = matTarget[2];

		float dot = glm::clamp(glm::dot(worldZ, targetZ), -1.0f, 1.0f);
		float angle = acosf(dot);

		glm::vec3 axis = glm::cross(worldZ, targetZ);

		if (glm::length(axis) < 0.0001f) {
			axis = g_normalX;
		}
		else {
			axis = glm::normalize(axis);
		}

		glm::vec3 angularAccel = axis * (angle * buoyancy * 3.0f * submerged);
		AddSoAngularAcceleration(pso, &angularAccel);
	}

	if (submerged > 0.0f) {
		if (submerged < 1.0f && normalSurface.z > 0.9f) {
			glm::vec3 currentLinear;
			glm::vec3 currentAngular;

			CalculateWaterCurrent(pwater, &pso->xf.posWorld, &currentLinear, &currentAngular);
			WakeSoWater(pso, pwater, dt, &currentLinear, &posSurface, 0.1f, ((pso->posMax.x - pso->posMin.x) + (pso->posMax.y - pso->posMin.y)) * 0.25f);
			return;
		}
	}

	/*if ((pso->pvtlo->grfcid & 8U) != 0) {
		StopSound(pso->pambWater, 0);
	}*/
}

int FIsSoTouching(SO* pso, SO* psoTarget)
{
	XP* pxp = PxpFirstFromSoSo(pso, psoTarget);
	SO* psoRoot = (SO*)pso->paloRoot;

	while (pxp)
	{
		int side = (psoRoot != pxp->axpd[0].psoRoot) ? 1 : 0;
		int otherSide = 1 - side;

		if (FFindLoParent((LO*)pxp->axpd[side].psoLeaf, (ALO*)pso) && FFindLoParent((LO*)pxp->axpd[otherSide].psoLeaf, (ALO*)psoTarget))
			return 1;

		pxp = pxp->pxpNext;
	}

	return 0;
}

SO* PsoFirstSoTouching(SO* pso)
{
	SO* psoRoot = (SO*)pso->paloRoot;
	OX* pox = psoRoot->poxa->pox;

	while (pox)
	{
		XP* pxp = pox->pxp;

		while (pxp)
		{
			int side = (psoRoot != pxp->axpd[0].psoRoot) ? 1 : 0;
			int otherSide = 1 - side;

			if (FFindLoParent((LO*)pxp->axpd[side].psoLeaf, (ALO*)pso))
				return pxp->axpd[otherSide].psoLeaf;

			pxp = pxp->pxpNext;
		}

		pox = pox->poxNext;
	}

	return nullptr;
}

void ApplySoConstraintWorld(SO* pso, CONSTR* pconstr, glm::vec3* pvecWorld, glm::vec3* pvecConstr, glm::vec3* pvecRemain)
{
	glm::vec3 vecConstr;
	glm::vec3* pvecOut = pvecConstr ? pvecConstr : &vecConstr;

	switch (pconstr->ct)
	{
		case CT_Free:
		{
			if (pvecOut != pvecWorld)
				*pvecOut = *pvecWorld;
			break;
		}

		case CT_Tangent:
		{
			glm::vec3 normal = pconstr->normal;

			if (pso->paloParent)
				normal = glm::mat3(pso->paloParent->xf.matWorld) * normal;

			float dot = glm::dot(*pvecWorld, normal);
			*pvecOut = *pvecWorld - normal * dot;
			break;
		}

		case CT_Project:
		{
			glm::vec3 normal = pconstr->normal;

			if (pso->paloParent)
				normal = glm::mat3(pso->paloParent->xf.matWorld) * normal;

			float dot = glm::dot(*pvecWorld, normal);
			*pvecOut = normal * dot;
			break;
		}

		case CT_Locked:
		{
			*pvecOut = glm::vec3(0.0f);
			break;
		}
	}

	if (pvecRemain)
		*pvecRemain = *pvecWorld - *pvecOut;
}

void AddSoXa(SO* pso, XA* pxaAdd)
{
	pxaAdd->pxaNextSource = (XA*)pso->pxa;
	pso->pxa = pxaAdd;
	ResolveAlo(pso);
}

void RemoveSoXa(SO* pso, XA* pxaRemove)
{
	if (pso->pxa)
	{
		if (pso->pxa == pxaRemove)
		{
			pso->pxa = pxaRemove->pxaNextSource;
			pxaRemove->pxaNextSource = nullptr;
		}
		else
		{
			XA* pxa = pso->pxa;

			while (pxa->pxaNextSource)
			{
				if (pxa->pxaNextSource == pxaRemove)
				{
					pxa->pxaNextSource = pxaRemove->pxaNextSource;
					pxaRemove->pxaNextSource = nullptr;
					break;
				}

				pxa = pxa->pxaNextSource;
			}
		}
	}

	ResolveAlo(pso);
}

void SetSoSphere(SO* pso, float sRadius)
{
	pso->sRadiusSelf = sRadius;
	pso->fSphere = 1;
}

void SetSoNoInteract(SO* pso, int fNoInteract)
{
	SetSoConstraints(pso, CT_Locked, nullptr, CT_Locked, nullptr);
}

void ConstrFromCnstr(CNSTR cnstr, CT* pct, glm::vec3* pnormal)
{
	*pct = s_dncnstr[cnstr].ct;
	*pnormal = *s_dncnstr[cnstr].pnormal;
}

void SetSoCnstrForce(SO* pso, CNSTR cnstr)
{
	CT ct;
	glm::vec3 normalLocal;
	ConstrFromCnstr(cnstr, &ct, &normalLocal);

	glm::vec3 normal = glm::mat3(pso->xf.mat) * normalLocal;
	SetSoConstraints(pso, ct, &normal, pso->constrTorque.ct, &pso->constrTorque.normal);
}

void SetSoCnstrTorque(SO* pso, CNSTR cnstr)
{
	CT ct;
	glm::vec3 normalLocal;
	ConstrFromCnstr(cnstr, &ct, &normalLocal);

	glm::vec3 normal = glm::mat3(pso->xf.mat) * normalLocal;
	SetSoConstraints(pso, pso->constrForce.ct, &pso->constrForce.normal, ct, &normal);
}

void *GetSoMass(SO* pso)
{
	return &pso->m;
}

void* GetSoDvGravity(SO* pso)
{
	return &pso->dvGravity;
}

void SetSoDvGravity(SO* pso, glm::vec3* pdvgravity)
{
	pso->dvGravity = *pdvgravity;
}

void* GetSoGbuoyancy(SO* pso)
{
	return &pso->gBuoyancy;
}

void SetSoGbuoyancy(SO* pso, float* pgbuoyancy)
{
	pso->gBuoyancy = *pgbuoyancy;
}

void* GetSoGviscosity(SO* pso)
{
	return &pso->gViscosity;
}

void SetSoGviscosity(SO* pso, float* pgviscosity)
{
	pso->gViscosity = *pgviscosity;
}

void* GetSoZok(SO* pso)
{
	return &pso->zok;
}

void SetSoZok(SO* pso, ZOK* pzok)
{
	pso->zok = *pzok;
}

void* GetSoSradiusAll(SO* pso)
{
	return &pso->sRadiusAll;
}

void SetSoCameraStyle(SO* pso, CMK cmk)
{
	pso->cmk = cmk;
}

void SetSoIgnoreLocked(SO* pso, int fIgnoreLocked)
{
	pso->fIgnoreLocked = fIgnoreLocked;
}

void SetSoIceable(SO* pso, int fIceable)
{
	pso->fIceable = fIceable;
}

void SetSoMtlk(SO* pso, MTLK mtlk)
{
	pso->mtlk = mtlk;
}

void SetSoEdgeGrab(SO* pso, EGK egk)
{
	pso->egk = egk;
}

void* AccessSoMass(SO* pso)
{
	return &pso->m;
}

void* AccessSoDvGravity(SO* pso)
{
	return &pso->dvGravity;
}

void* AccessSoGbuoyancy(SO* pso)
{
	return &pso->gBuoyancy;
}

void* AccessSoGviscosity(SO* pso)
{
	return &pso->gViscosity;
}

void* AccessSoZok(SO* pso)
{
	return &pso->zok;
}

void* AccessSoSradiusAll(SO* pso)
{
	return &pso->sRadiusAll;
}

void SendSoMessage(SO* pso, MSGID msgid, void* pv)
{
	SendLoMessage(pso, msgid, pv);

	XA* pxa = pso->pxa;
	while (pxa != nullptr)
	{
		// The message handler may unlink and free this XA (notably when an
		// SO is removed from water), so fetch the next source first.
		XA* pxaNext = pxa->pxaNextSource;
		SO* psoSource = pxa->psoSource;

		if (psoSource != pso && psoSource->pvtlo->pfnHandleLoMessage != nullptr) {
			psoSource->pvtlo->pfnHandleLoMessage(psoSource, msgid, pv);
		}

		pxa = pxaNext;
	}
}

XP* PxpFindSoGround(SO* psoRoot, SO* psoLeaf, int* pixpd)
{
	XP* pxpGround = nullptr;
	int ixpdGround = 0;
	float zBest = FLT_MAX;

	for (OX* pox = psoRoot->poxa->pox; pox != nullptr; pox = pox->poxNext) 
	{
		for (XP* pxp = pox->pxp; pxp != nullptr; pxp = pxp->pxpNext) 
		{
			int ixpd = pxp->axpd[0].psoRoot != psoRoot;

			if (psoLeaf != nullptr && pxp->axpd[ixpd].psoLeaf != psoLeaf)
				continue;

			if (ixpd == 0) {
				if (pxp->normal.z <= 0.7f)
					continue;
			}
			else {
				if (pxp->normal.z >= -0.7f)
					continue;
			}

			float z = pxp->axpd[ixpd].pos.z;

			if (z < zBest) {
				zBest = z;
				pxpGround = pxp;
				ixpdGround = ixpd;
			}
		}
	}

	if (pixpd != nullptr)
		*pixpd = ixpdGround;

	return pxpGround;
}

void SetSoNoGravity(SO* pso, int fNoGravity)
{
	pso->fNoGravity = fNoGravity;
}

void SetSoNoXpsAll(SO* pso, int fNoXpsAll)
{
	pso->fNoXpsAll = (fNoXpsAll != 0);

	DiscardSoXps(pso, fNoXpsAll ? 1 : 0);
}

void SetSoNoXpsSelf(SO* pso, int fNoXpsSelf)
{
	pso->fNoXpsSelf = (fNoXpsSelf != 0);

	DiscardSoXps(pso, fNoXpsSelf ? 2 : 0);
}

void SetSoNoXpsCenter(SO* pso, int fNoXpsCenter)
{
	pso->fNoXpsCenter = (fNoXpsCenter != 0);

	DiscardSoXps(pso, fNoXpsCenter ? 3 : 0);
}

SO* PsoFindSoPhysHook(SO* psoLeaf, int ib)
{
	while (psoLeaf != nullptr) {
		SO* psoHook = psoLeaf->psoPhysHook;

		if (psoHook == nullptr) {
			break;
		}

		if (*(int*)((char*)&psoHook->pvtbasic->pvtSuper + ib) != 0) {
			return psoHook;
		}

		psoLeaf = (SO*)psoHook->paloParent;
	}

	return nullptr;
}

int FGetSoContactList(SO* pso, void* pvstate)
{
	SO* psoRoot = (SO*)pso->paloRoot;

	for (OX* pox = psoRoot->poxa->pox; pox != nullptr; pox = pox->poxNext)
	{
		for (XP* pxp = pox->pxp; pxp != nullptr; pxp = pxp->pxpNext)
		{
			int ixpd = (pxp->axpd[0].psoRoot != psoRoot) ? 1 : 0;

			if (pxp->axpd[ixpd].psoLeaf != pso)
				continue;

			SO* psoInContact = pxp->axpd[1 - ixpd].psoRoot;

			if (FSpliceListElementExists(pvstate, &psoInContact))
				continue;

			if (!FAppendSpliceListElement(pvstate, &psoInContact))
				return 0;
		}
	}

	return 1;
}

void GetSoContacts(SO* pso, int* pcpso, SO*** papso)
{
	*pcpso = 0;
	*papso = nullptr;

	if (pso == nullptr || pso->paloRoot == nullptr)
		return;

	SO* psoRoot = (SO*)pso->paloRoot;
	if (psoRoot->poxa == nullptr)
		return;

	int cpsoMax = 0;

	for (OX* pox = psoRoot->poxa->pox; pox != nullptr; pox = pox->poxNext) {
		for (XP* pxp = pox->pxp; pxp != nullptr; pxp = pxp->pxpNext) {
			int ixpdSelf = pxp->axpd[0].psoRoot != psoRoot ? 1 : 0;

			if (pxp->axpd[ixpdSelf].psoLeaf == pso) {
				cpsoMax++;
			}
		}
	}

	if (cpsoMax == 0)
		return;

	SO** apso = new SO*[cpsoMax];
	int cpso = 0;

	for (OX* pox = psoRoot->poxa->pox; pox != nullptr; pox = pox->poxNext) {
		for (XP* pxp = pox->pxp; pxp != nullptr; pxp = pxp->pxpNext) {
			int ixpdSelf = pxp->axpd[0].psoRoot != psoRoot ? 1 : 0;

			if (pxp->axpd[ixpdSelf].psoLeaf != pso) {
				continue;
			}

			int ixpdOther = 1 - ixpdSelf;
			SO* psoOther = pxp->axpd[ixpdOther].psoRoot;

			if (psoOther == nullptr)
				continue;

			bool fAlreadyAdded = false;

			for (int ipso = 0; ipso < cpso; ipso++) {
				if (apso[ipso] == psoOther) {
					fAlreadyAdded = true;
					break;
				}
			}

			if (!fAlreadyAdded) {
				apso[cpso] = psoOther;
				cpso++;
			}
		}
	}

	*papso = apso;
	*pcpso = cpso;
}

int FSoInStsoList(STSO* pstsoFirst, SO* pso)
{
	while (pstsoFirst)
	{
		if (pstsoFirst->psoTouch == pso)
			return 1;

		pstsoFirst = pstsoFirst->pstsoNext.get();
	}

	return 0;
}

std::shared_ptr <STSO> PstsoAlloc()
{
	return std::make_shared <STSO>();
}

void GenerateSoSpliceTouchingEvents(SO* pso)
{
	if (!pso || !pso->paloRoot)
		return;

	SO* psoRoot = (SO*)pso->paloRoot;
	OXA* poxa = psoRoot->poxa;

	const auto GetContactObject = [pso, psoRoot](XP* pxp) -> SO*
	{
		const int iSelf = pxp->axpd[0].psoRoot != psoRoot;

		if (pxp->axpd[iSelf].psoLeaf != pso)
			return nullptr;

		return pxp->axpd[1 - iSelf].psoLeaf;
	};

	// Generate events for contacts that have just started.
	if (poxa)
	{
		for (OX* pox = poxa->pox; pox; pox = pox->poxNext)
		{
			for (XP* pxp = pox->pxp; pxp; pxp = pxp->pxpNext)
			{
				SO* psoInContact = GetContactObject(pxp);

				if (!psoInContact)
					continue;

				bool fAlreadyTouching = false;

				for (std::shared_ptr <STSO> pstso = pso->pstso; pstso; pstso = pstso->pstsoNext)
				{
					if (pstso->psoTouch == psoInContact)
					{
						fAlreadyTouching = true;
						break;
					}
				}

				if (!fAlreadyTouching)
				{
					void* pvArgs = &psoInContact;
					HandleLoSpliceEvent(pso, 6, 1, &pvArgs);
				}
			}
		}
	}

	// Generate events for contacts that have ended.
	for (std::shared_ptr<STSO> pstso = pso->pstso; pstso; pstso = pstso->pstsoNext)
	{
		bool fStillTouching = false;

		if (poxa)
		{
			for (OX* pox = poxa->pox; pox && !fStillTouching; pox = pox->poxNext)
			{
				for (XP* pxp = pox->pxp; pxp; pxp = pxp->pxpNext)
				{
					if (GetContactObject(pxp) == pstso->psoTouch)
					{
						fStillTouching = true;
						break;
					}
				}
			}
		}

		if (!fStillTouching)
		{
			void* pvArgs = pstso.get();
			HandleLoSpliceEvent(pso, 7, 1, &pvArgs);
		}
	}

	// Destroy the previous contact list.
	FreeStsoList(pso->pstso);

	// Build the contact list for the next frame.
	std::shared_ptr <STSO>* ppstsoNext = &pso->pstso;

	if (poxa)
	{
		for (OX* pox = poxa->pox; pox; pox = pox->poxNext)
		{
			for (XP* pxp = pox->pxp; pxp; pxp = pxp->pxpNext)
			{
				SO* psoInContact = GetContactObject(pxp);

				if (!psoInContact)
					continue;

				std::shared_ptr <STSO> pstso = PstsoAlloc();
				pstso->psoTouch = psoInContact;

				*ppstsoNext = pstso;
				ppstsoNext = &pstso->pstsoNext;
			}
		}
	}
}

void FreeStsoList(std::shared_ptr <STSO> pstspFirst)
{
	if (pstspFirst != nullptr)
		pstspFirst.reset();
}

void CalculateSoTrajectoryApex(SO* pso, glm::vec3* pposTarget, float dzMax, glm::vec3* pv)
{
	float zStart = pso->xf.posWorld.z;
	float zTarget = pposTarget->z;
	float zApex = glm::max(zStart, zTarget) + dzMax;

	pv->z = std::sqrt(pso->dvGravity.z * (zApex - zStart) * -2.0f);

	float at[2];
	CSolveQuadratic(pso->dvGravity.z * 0.5f, pv->z, zStart - zTarget, at);

	float t = glm::max(at[0], at[1]);

	pv->x = (pposTarget->x - pso->xf.posWorld.x) / t;
	pv->y = (pposTarget->y - pso->xf.posWorld.y) / t;
}

void CalculateSoEffect(SO* pso, glm::vec3* ppos, FX* pfx, glm::vec3* pdv)
{
	glm::vec3 r = *ppos - pso->xf.posWorld;
	*pdv = pfx->dv + glm::cross(pfx->dw, r);
}

void PropagateSoForce(SO* psoRoot, GRFSG grfsg, XP* pxp, int ixpd, DZ* pdz, FX* afx)
{
	glm::vec3 f = pdz->f;

	if (ixpd != 0)
		f = -f;

	SO* pso = pxp->axpd[ixpd].psoLeaf;

	while (pso)
	{
		if (pso->fLockedSelf)
		{
			if (pso->fLockedAbove)
				break;

			pso = (SO*)pso->paloParent;
			continue;
		}

		int ipso = pso->ipso;

		glm::vec3 r = pxp->axpd[ixpd].pos - pso->xf.posWorld;
		glm::vec3 tau = glm::cross(r, f);

		glm::vec3 fConstr;
		ApplySoConstraintWorld(pso, &pso->constrForce, &f, &fConstr, &f);

		// EE DIV.S saturates division by zero to the largest finite float.
		// Native IEEE division produces infinity and poisons the solve group.
		const float rm = pso->m == 0.0f ? std::copysign((std::numeric_limits<float>::max)(), pso->m) : 1.0f / pso->m;
		const double maxFloat = static_cast<double>((std::numeric_limits<float>::max)());
		auto MulEe = [maxFloat](float value, float scale)
		{
			const double product = static_cast<double>(value) * scale;
			if (product > maxFloat)
				return (std::numeric_limits<float>::max)();
			if (product < -maxFloat)
				return -(std::numeric_limits<float>::max)();
			return static_cast<float>(product);
		};
		afx[ipso].dv = glm::vec3(MulEe(fConstr.x, rm), MulEe(fConstr.y, rm), MulEe(fConstr.z, rm));

		CalculateSoAngularEffectWorld(pso, &tau, &afx[ipso].dw);
		ApplySoConstraintWorld(pso, &pso->constrTorque, &afx[ipso].dw, &afx[ipso].dw, nullptr);

		pso = (SO*)pso->paloParent;
	}
}

void DistributeSoEffects(SO* psoRoot, GRFSG grfsg, int cpso, SO** apso, FX* afx)
{
	for (int i = 1; i < cpso; i++)
	{
		SO* pso = apso[i];

		FX* pfx = &afx[pso->ipso];
		SO* parentSo = (SO*)pso->paloParent;
		FX* pfxParent = &afx[parentSo->ipso];

		glm::vec3 dv;
		CalculateSoEffect((SO*)pso->paloParent, &pso->xf.posWorld, pfxParent, &dv);

		if (pso->fLockedSelf)
		{
			pfx->dv += dv;
			pfx->dw += pfxParent->dw;
			continue;
		}

		glm::vec3 dw;

		ApplySoConstraintWorld(pso, &pso->constrForce, &dv, nullptr, &dv);
		ApplySoConstraintWorld(pso, &pso->constrTorque, &pfxParent->dw, nullptr, &dw);

		if (pso->constrTorque.ct != CT_Locked)
		{
			if (glm::length(pso->posComLocal) > 0.0001f)
			{
				glm::mat3 matWorld = glm::mat3(pso->xf.matWorld);

				glm::vec3 dvLocal = glm::transpose(matWorld) * dv;
				glm::vec3 rLocal = pso->posComLocal;

				glm::vec3 normal = glm::cross(rLocal, dvLocal);
				float normalLen = glm::length(normal);

				if (normalLen > 0.0001f)
				{
					normal /= normalLen;

					glm::vec3 moi = glm::mat3(pso->momintLocal) * normal;
					float inertiaAlongNormal = glm::dot(moi, normal);

					float rLocalLenSq = glm::dot(rLocal, rLocal);

					glm::vec3 rWorld = matWorld * rLocal;
					float rWorldLenSq = glm::dot(rWorld, rWorld);

					glm::vec3 coupleDir = glm::cross(dv, rWorld);

					float scale =
						(pso->m / (pso->m + inertiaAlongNormal / rLocalLenSq)) /
						rWorldLenSq;

					glm::vec3 dwCouple = coupleDir * scale;

					ApplySoConstraintWorld(pso, &pso->constrTorque, &dwCouple, &dwCouple, nullptr);

					dw += dwCouple;
				}
			}
		}

		pfx->dv += dv;
		pfx->dw += dw;
	}
}

int FAbsorbSoWkr(SO* pso, WKR* pwkr)
{
	if ((pwkr->grfic & 8U) != 0)
		ApplySoImpulse(pso, &pwkr->pos, &pwkr->v, pwkr->sftMax);

	return (pwkr->grfic != 0);
}

void AdjustSoMomint(SO* pso, float r)
{
	pso->momintLocal *= r;

	float det = glm::determinant(pso->momintLocal);

	if (det != 0.0f)
		pso->momintInvLocal = glm::inverse(pso->momintLocal);
	else
		pso->momintInvLocal = glm::mat3(0.0f);
}

void SetSoMass(SO* pso, float m)
{
	float r = m / pso->m;

	pso->m = pso->m * r;
	AdjustSoMomint(pso, r);
}

void SetSoVelocityVec(SO* pso, glm::vec3* pv)
{
	glm::vec3 oldVelocity = pso->xf.v;
	SetAloVelocityVec(pso, pv);

	glm::vec3 dv = *pv - oldVelocity;

	if (glm::abs(dv.x) >= 2.0f || glm::abs(dv.y) >= 2.0f || glm::abs(dv.z) >= 2.0f)
		InvalidateSwXpForObject(pso->psw, pso, 2);
}

void SetSoAngularVelocityVec(SO* pso, glm::vec3* pw)
{
	// Keep base ALO angular velocity in sync.
	SetAloAngularVelocityVec(pso, pw);

	// Original code compares new angular velocity against current SO transform angular velocity.
	glm::vec3 dw = *pw - pso->xf.w;

	// PS2 code invalidates only if the angular velocity changed meaningfully.
	constexpr float kAngularVelEpsilon = 0.02f;

	bool unchanged = std::abs(dw.x) < kAngularVelEpsilon && std::abs(dw.y) < kAngularVelEpsilon && std::abs(dw.z) < kAngularVelEpsilon;

	if (!unchanged)
		InvalidateSwXpForObject(pso->psw, pso, 2);
}

void AddSoAngularAcceleration(SO* pso, glm::vec3* pdw)
{
	pso->xf.dw += *pdw;
}

void CalculateSoAngularEffectWorld(SO* pso, glm::vec3* ptau, glm::vec3* pdw)
{
	glm::mat3 rotation = glm::mat3(pso->xf.matWorld);

	glm::vec3 localTau = glm::transpose(rotation) * *ptau;
	glm::vec3 localDw = glm::mat3(pso->momintInvLocal) * localTau;

	*pdw = rotation * localDw;
}

void AccelSoTowardPosSpring(SO* pso, glm::vec3* pposTarget, CLQ* pclqPos, glm::vec3* pvTarget, CLQ* pclqV, float dt)
{
	glm::vec3 dv(0.0f);

	if (pclqPos)
	{
		glm::vec3 delta = *pposTarget - pso->xf.pos;
		float len = glm::length(delta);

		if (len > 0.0001f)
		{
			float scale = pclqPos->g2 * len + pclqPos->g1 + pclqPos->g0 / len;
			dv = delta * scale;
		}
	}

	if (pclqV)
	{
		glm::vec3 deltaV = *pvTarget - pso->xf.v;
		float len = glm::length(deltaV);

		if (len > 0.0001f)
		{
			float scale = pclqV->g2 * len + pclqV->g1 + pclqV->g0 / len;

			if (scale > 1.0f / dt)
				scale = 1.0f / dt;

			dv += deltaV * scale;
		}
	}

	AddSoAcceleration(pso, &dv);
}

void AccelSoTowardMatSpring(SO* pso, glm::mat3* pmatTarget, CLQ* pclqRot, glm::vec3* pwTarget, CLQ* pclqW, float dt)
{
	glm::vec3 dw(0.0f);

	if (pclqRot)
	{
		glm::mat3 dmat;

		CalculateDmat(&pso->xf.mat, pmatTarget, &dmat);

		float rad;
		glm::vec3 normal;

		DecomposeRotateMatrixRad(&dmat, &rad, &normal);

		if (rad > 0.0001f)
		{
			float scale = pclqRot->g2 * rad * rad + pclqRot->g1 * rad + pclqRot->g0;
			dw = normal * scale;
		}
	}

	if (pclqW)
	{
		glm::vec3 deltaW = *pwTarget - pso->xf.w;
		float len = glm::length(deltaW);

		if (len > 0.0001f)
		{
			float scale = pclqW->g2 * len + pclqW->g1 + pclqW->g0 / len;

			if (scale > 1.0f / dt)
				scale = 1.0f / dt;

			dw += deltaW * scale;
		}
	}

	AddSoAngularAcceleration(pso, &dw);
}

void AccelSoTowardMatSmooth(SO* pso, float dt, glm::mat3* pmat, SMP* psmp)
{
	float gAbsLimit = psmp->svFast;

	glm::vec3 w = pso->xf.w;
	float wLen = glm::length(w);

	float accelLimit = (gAbsLimit - psmp->svSlow) / psmp->dtFast;
	float slowAngle = (gAbsLimit * gAbsLimit) / (accelLimit + accelLimit);

	if (wLen > gAbsLimit)
	{
		glm::vec3 dw = w * (((gAbsLimit - wLen) * 2.5f) / wLen);
		AddSoAngularAcceleration(pso, &dw);
	}

	glm::mat3 dmat;
	CalculateDmat(&pso->xf.mat, pmat, &dmat);

	float rad;
	glm::vec3 normal;
	DecomposeRotateMatrixRad(&dmat, &rad, &normal);

	float wTargetMag = GLimitAbs((rad / slowAngle) * gAbsLimit, gAbsLimit);

	float wAlongNormal = glm::dot(pso->xf.w, normal);

	float radPredicted = rad - (wAlongNormal + wTargetMag) * dt * 0.5f;

	if (radPredicted < slowAngle)
		wTargetMag = GLimitAbs((radPredicted / slowAngle) * gAbsLimit, gAbsLimit);

	glm::vec3 wPredicted = pso->xf.w + pso->xf.dw * dt;
	glm::vec3 wTarget = normal * wTargetMag;

	glm::vec3 dwWanted = wTarget - wPredicted;
	float dwWantedLen = glm::length(dwWanted);

	glm::vec3 dw;

	// A zero smoothing envelope and zero angular error mean no acceleration.
	// Letting that case fall through to accelLimit / dwWantedLen evaluates 0/0
	// and permanently poisons xf.w with NaNs on the next integration step.
	if (dwWantedLen <= 0.0001f)
		dw = glm::vec3(0.0f);
	else if (dwWantedLen < accelLimit * dt)
		dw = dwWanted * (1.0f / dt);
	else
		dw = dwWanted * (accelLimit / dwWantedLen);

	AddSoAngularAcceleration(pso, &dw);
}

void DiscardSoXps(SO* pso, int discardMode)
{
	SO* psoRoot = (SO*)pso->paloRoot;

	if (psoRoot == nullptr)
		return;

	if (discardMode == 0)
	{
		psoRoot->grfpvaXpValid = 0;
		return;
	}

	XP* pxpFree = nullptr;

	for (OX* pox = psoRoot->poxa->pox; pox != nullptr; pox = pox->poxNext)
	{
		XP** ppxp = &pox->pxp;
		XP* pxp = *ppxp;

		while (pxp != nullptr)
		{
			SO* psoRoot0 = (SO*)pxp->axpd[0].psoRoot;
			bool useSide1 = (psoRoot0 != psoRoot);

			SO* psoLeaf = (SO*)pxp->axpd[useSide1 ? 1 : 0].psoLeaf;

			bool remove = false;

			if (discardMode == 1)
			{
				remove = FFindLoParent(psoLeaf, pso) != 0;
			}
			else if (discardMode == 2)
			{
				remove = (psoLeaf == pso);
			}
			else if (discardMode == 3)
			{
				remove =
					(psoLeaf == pso) &&
					!useSide1 &&
					pxp->xpk >= XPK_SphereToSphere &&
					pxp->xpk <= XPK_SphereToPoint;
			}

			if (remove)
			{
				*ppxp = pxp->pxpNext;

				pxp->pxpNext = pxpFree;
				pxpFree = pxp;

				pxp = *ppxp;
			}
			else
			{
				ppxp = &pxp->pxpNext;
				pxp = pxp->pxpNext;
			}
		}

		pox->poxOther->pxp = pox->pxp;
	}

	FreeSwXpList(pso->psw, pxpFree);
	psoRoot->grfpvaXpValid = 0;
}

void FreezeSo(SO* pso, int fFreeze)
{
	if (fFreeze == 0)
	{
		FreezeAlo(pso, 0);
		SetSoConstraints(pso, pso->frz.ctForce, nullptr, pso->frz.ctTorque, nullptr);
	}
	else
	{
		// Save current constraint modes.
		pso->frz.ctForce  = pso->constrForce.ct;
		pso->frz.ctTorque = pso->constrTorque.ct;

		//// Lock movement/rotation while frozen.
		SetSoConstraints(pso, CT_Locked, nullptr, CT_Locked, nullptr);

		//// Clear accumulated velocity deltas.
		pso->xf.dw = glm::vec3{};
		pso->xf.dv = glm::vec3{};

		//// Remove active STSO state while frozen.
		FreeStsoList(pso->pstso);
		pso->pstso = nullptr;

		FreezeAlo(pso, 1);
	}
}

void RenderSoSelf(SO* pso, CM* pcm, RO* pro)
{
	RenderAloSelf(pso, pcm, pro);
}

void DeleteSo(SO *pso)
{
	delete pso;
}

void DeleteSwCollision()
{
	for (int i = 0; i < allSWSoObjs.size(); i++)
	{
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomLocal.VAO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomLocal.VBO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomLocal.EBO);

		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomCameraLocal.VAO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomCameraLocal.VBO);
		glDeleteVertexArrays(1, &allSWSoObjs[i]->geomCameraLocal.EBO);
	}
}

void DeleteSoGeom(SO* pso)
{
	glDeleteVertexArrays(1, &pso->geomLocal.VAO);
	glDeleteVertexArrays(1, &pso->geomLocal.VBO);
	glDeleteVertexArrays(1, &pso->geomLocal.EBO);

	glDeleteVertexArrays(1, &pso->geomCameraLocal.VAO);
	glDeleteVertexArrays(1, &pso->geomCameraLocal.VBO);
	glDeleteVertexArrays(1, &pso->geomCameraLocal.EBO);
}

void DeallocateSoVector()
{
	allSWSoObjs.clear();
	allSWSoObjs.shrink_to_fit();
}

std::vector <SO*> allSWSoObjs;
ECNSTR s_dncnstr[8] =
{
{ &g_vecZero, CT_Free    },
{ &g_vecZero, CT_Locked  },
{ &g_normalX, CT_Project },
{ &g_normalY, CT_Project },
{ &g_normalZ, CT_Project },
{ &g_normalZ, CT_Tangent },
{ &g_normalY, CT_Tangent },
{ &g_normalX, CT_Tangent }
};
