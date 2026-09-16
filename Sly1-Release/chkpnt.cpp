#include "chkpnt.h"
#include "sm.h"
#include "asega.h"
#include "actseg.h"
#include "po.h"
#include "credit.h"
#include "acs.h"

CHKPNT* NewChkpnt()
{
	return new CHKPNT{};
}

void InitChkpnt(CHKPNT* pchkpnt)
{
	InitSo(pchkpnt);
	pchkpnt->ichkTriggered = IchkAllocChkmgr(&g_chkmgr);
	pchkpnt->oidWarp = OID_Nil;
	pchkpnt->ibitChkpnt = pchkpnt->psw->cchkpntAll;
	pchkpnt->psw->cchkpntAll++;
	SetSoConstraints(pchkpnt, CT_Locked, nullptr, CT_Locked, nullptr);
}

void CloneChkpnt(CHKPNT* pchkpnt, CHKPNT* pchkpntBase)
{
	// These are allocated for the new instance by InitChkpnt and must not be
	// replaced by the base checkpoint's manager indices.
	const int ichkTriggered = pchkpnt->ichkTriggered;
	const int ibitChkpnt = pchkpnt->ibitChkpnt;

	CloneSo(pchkpnt, pchkpntBase);

	pchkpnt->coidAseg = pchkpntBase->coidAseg;
	for (int i = 0; i < 4; ++i)
		pchkpnt->aoidAseg[i] = pchkpntBase->aoidAseg[i];

	pchkpnt->cpaseg = pchkpntBase->cpaseg;
	for (int i = 0; i < 4; ++i)
		pchkpnt->apaseg[i] = pchkpntBase->apaseg[i];

	pchkpnt->coidVol = pchkpntBase->coidVol;
	for (int i = 0; i < 4; ++i)
		pchkpnt->aoidVol[i] = pchkpntBase->aoidVol[i];

	pchkpnt->cpvol = pchkpntBase->cpvol;
	for (int i = 0; i < 4; ++i)
		pchkpnt->apvol[i] = pchkpntBase->apvol[i];

	pchkpnt->oidWarp = pchkpntBase->oidWarp;
	pchkpnt->pwarp = pchkpntBase->pwarp;

	// Retail CloneLo/CloneSo performs a shallow copy of the derived tail, so
	// cloned checkpoints retain the shared SM definition.  The SMA itself is
	// per checkpoint and is recreated by PostChkpntLoad.
	pchkpnt->psm = pchkpntBase->psm;
	pchkpnt->psma = nullptr;

	pchkpnt->ichkTriggered = ichkTriggered;
	pchkpnt->ibitChkpnt = ibitChkpnt;

	for (int i = 0; i < pchkpnt->cpaseg; ++i)
		pchkpnt->apaseg[i] = (ASEG*)PloCloneLo(pchkpnt->apaseg[i], pchkpnt->psw, pchkpnt);

	for (int i = 0; i < pchkpnt->cpvol; ++i)
		pchkpnt->apvol[i] = (VOL*)PloCloneLo(pchkpnt->apvol[i], pchkpnt->psw, pchkpnt);
}

void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pchkpnt, pbis);
	SnipAloObjects(pchkpnt, 1, &asnipChkpnt);

	DLI dli{};

	dli.m_pdl = &pchkpnt->dlChild;
	dli.m_ibDle = pchkpnt->dlChild.ibDle;
	dli.m_pdliNext = s_pdliFirst;

	LO* child = pchkpnt->dlChild.ploFirst;
	dli.m_ppv = child != nullptr ? reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(child) + dli.m_ibDle) : nullptr;

	s_pdliFirst = &dli;

	while (child != nullptr)
	{
		if (FIsBasicDerivedFrom((BASIC*)child, CID_VOL))
		{
			pchkpnt->apvol[pchkpnt->cpvol++] = (VOL*)child;
			SnipLo(child);
		}
		else if (FIsBasicDerivedFrom((BASIC*)child, CID_ASEG))
		{
			pchkpnt->apaseg[pchkpnt->cpaseg++] = (ASEG*)child;
			SnipLo(child);
		}
		else if (FIsBasicDerivedFrom((BASIC*)child, CID_WARP))
		{
			pchkpnt->oidWarp = OidProxyLo(child);

			if (pchkpnt->oidWarp == OID_Nil)
				pchkpnt->oidWarp = child->oid;
		}

		child = dli.m_ppv != nullptr ? (LO*)*dli.m_ppv : nullptr;
		dli.m_ppv = child != nullptr ? reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(child) + dli.m_ibDle) : nullptr;
	}

	s_pdliFirst = dli.m_pdliNext;
}

void BindChkpnt(CHKPNT* pchkpnt)
{
	BindAlo(pchkpnt);

	for (int i = 0; i < pchkpnt->coidAseg; ++i)
	{
		ASEG* paseg = (ASEG*)PloFindSwNearest(pchkpnt->psw, (OID)pchkpnt->aoidAseg[i], (LO*)pchkpnt);

		if (paseg != nullptr)
			pchkpnt->apaseg[pchkpnt->cpaseg++] = paseg;
	}

	for (int i = 0; i < pchkpnt->coidVol; ++i)
	{
		VOL* pvol = (VOL*)PloFindSwNearest(pchkpnt->psw, (OID)pchkpnt->aoidVol[i], (LO*)pchkpnt);

		if (pvol != nullptr)
			pchkpnt->apvol[pchkpnt->cpvol++] = pvol;
	}
}

void PostChkpntLoad(CHKPNT* pchkpnt)
{
	PostAloLoad(pchkpnt);
	bool fSnipCheckpointSm = false;

	// The retail checkpoint SM is normally captured by asnipChkpnt (OID 0x545)
	// during LoadChkpntFromBrx.  Proxy/cloned checkpoint hierarchies can make the
	// OID-based child lookup miss even though the SM is still a direct child.
	if (pchkpnt->psm == nullptr)
	{
		for (LO* child = pchkpnt->dlChild.ploFirst;
			 child != nullptr;
			 child = child->dleChild.ploNext)
		{
			if (FIsBasicDerivedFrom(child, CID_SM))
			{
				pchkpnt->psm = static_cast<SM*>(child);
				fSnipCheckpointSm = true;
				break;
			}
		}
	}

	for (int i = 0; i < pchkpnt->cpaseg; ++i)
		SnipLo(pchkpnt->apaseg[i]);

	for (int i = 0; i < pchkpnt->cpvol; ++i)
		SnipLo(pchkpnt->apvol[i]);

	if (pchkpnt->oidWarp == OID_Nil)
	{
		DLI dli{};

		dli.m_pdl = &pchkpnt->dlChild;
		dli.m_ibDle = pchkpnt->dlChild.ibDle;
		dli.m_pdliNext = s_pdliFirst;

		LO* child = pchkpnt->dlChild.ploFirst;
		dli.m_ppv = child != nullptr ? reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(child) + dli.m_ibDle) : nullptr;

		s_pdliFirst = &dli;

		while (child != nullptr)
		{
			if (FIsBasicDerivedFrom(child, CID_WARP))
			{
				pchkpnt->pwarp = (WARP*)child;
				pchkpnt->oidWarp = OidProxyLo(child);

				if (pchkpnt->oidWarp == OID_Nil)
					pchkpnt->oidWarp = child->oid;
			}

			child = dli.m_ppv != nullptr ? (LO*)*dli.m_ppv : nullptr;
			dli.m_ppv = child != nullptr ? reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(child) + dli.m_ibDle) : nullptr;
		}

		s_pdliFirst = dli.m_pdliNext;
	}
	else
		pchkpnt->pwarp = (WARP*)PloFindSwNearest(pchkpnt->psw, (OID)pchkpnt->oidWarp, pchkpnt);

	if (pchkpnt->psm != nullptr)
	{
		pchkpnt->psma = PsmaApplySm(pchkpnt->psm, (ALO*)pchkpnt, (OID)1350, 0);

		if (fSnipCheckpointSm)
			SnipLo(pchkpnt->psm);

		if (FGetChkmgrIchk(&g_chkmgr, pchkpnt->ichkTriggered))
			SetSmaGoal(pchkpnt->psma, (OID)1351);
	}
}

int GetChkpntSize()
{
	return sizeof(CHKPNT);
}

void UpdateChkpnt(CHKPNT* pchkpnt, float dt)
{
	UpdateSo(pchkpnt, dt);

	if (g_note.mtsState >= 2)
	{
		pchkpnt->pvtlo->pfnRemoveLo(pchkpnt);
		return;
	}

	PO* player = PpoCur();

	if (player == nullptr)
		return;

	if (FGetChkmgrIchk(&g_chkmgr, pchkpnt->ichkTriggered))
	{
		if (!g_chkmgr.fChkDirty)
			return;

		float distance = glm::distance(glm::vec3(g_chkmgr.posVolChkpnt), glm::vec3(player->xf.posWorld));

		if (distance < 1000.0f)
			return;

		g_chkmgr.posVolChkpnt = glm::vec3(FLT_MAX);
	}

	for (int i = 0; i < pchkpnt->cpvol; ++i)
	{
		VOL* pvol = pchkpnt->apvol[i];

		if (!FCheckVolPoint(pvol, &player->xf.posWorld))
			continue;

		GetXfmPos(pvol, &g_chkmgr.posVolChkpnt);
		TriggerChkpnt(pchkpnt);
		return;
	}
}

int FAbsorbChkpntWkr(CHKPNT* pchkpnt, WKR* pwkr)
{
	PO* ppoCurrent = (PO*)PpoCur();

	if (pwkr->ploSource != ppoCurrent)
		return FAbsorbSoWkr(pchkpnt, pwkr);

	/*if ((pwkr->grftak & 0x10) == 0) 
		TriggerJoyRumbleRumk(&g_joy, RUMK_MediumThrob, 0.2);*/

	TriggerChkpnt(pchkpnt);
	return 1;
}

void TriggerChkpnt(CHKPNT* pchkpnt)
{
	int smaGoal = 0;
	const int fAlreadyTriggered = FGetChkmgrIchk(&g_chkmgr, pchkpnt->ichkTriggered);

	OnDifficultyTriggerCheckpoint(&g_difficulty, pchkpnt);

	if (pchkpnt->psma != nullptr)
		smaGoal = fAlreadyTriggered ? 1353 : 1352;

	SetChkmgrIchk(&g_chkmgr, pchkpnt->ichkTriggered);

	OID oidWarpContext = OID_Nil;

	if (FFindLoParent(pchkpnt->pwarp, pchkpnt))
	{
		for (ALO* parent = pchkpnt; parent != nullptr; parent = parent->paloParent)
		{
			if (parent->ppxr != nullptr)
				oidWarpContext = parent->ppxr->oidProxyRoot;
		}

		if (oidWarpContext == OID_Nil)
			oidWarpContext = (OID)pchkpnt->oid;
	}

	SaveChkmgrCheckpoint(&g_chkmgr, (OID)pchkpnt->oidWarp, oidWarpContext);

	for (int i = 0; i < pchkpnt->cpaseg; ++i)
		ApplyAseg(pchkpnt->apaseg[i], nullptr, 0.0f, 1.0f, 0, nullptr);

	if (pchkpnt->psma != nullptr)
		SetSmaGoal(pchkpnt->psma, (OID)smaGoal);
}

void AddChkpntVol(CHKPNT* pchkpnt, OID oidVol)
{
	pchkpnt->aoidVol[pchkpnt->coidVol++] = oidVol;
}

void AddChkpntAseg(CHKPNT* pchkpnt, OID oidAseg)
{
	pchkpnt->aoidAseg[pchkpnt->coidAseg++] = oidAseg;
}

void SetChkpntWarp(CHKPNT* pchkpnt, OID oidWarp)
{
	pchkpnt->oidWarp = oidWarp;
}

void DeleteChkpnt(CHKPNT *pchkpnt)
{
	delete pchkpnt;
}

void ResetChkmgrCheckPoints(CHKMGR* pchkmgr)
{
	pchkmgr->cbitChk = 0;

	pchkmgr->abitChk.m_cbitSet = 0;
	pchkmgr->abitChk.m_ab.fill(0);

	pchkmgr->fChkDirty = false;
	pchkmgr->posVolChkpnt = glm::vec3(std::numeric_limits<float>::max());

	pchkmgr->csSaved = {};

	pchkmgr->fCheckpoint = false;
	pchkmgr->csSaved.oidWarpContext = OID_Nil;
	pchkmgr->csSaved.oidWarp = OID_Nil;
}

void SaveChkmgrCheckpoint(CHKMGR* pchkmgr, OID oidWarp, OID oidWarpContext)
{
	pchkmgr->fChkDirty = false;
	pchkmgr->csSaved.abitChk = pchkmgr->abitChk;

	pchkmgr->csSaved.oidWarp = oidWarp;
	pchkmgr->csSaved.oidWarpContext = oidWarpContext;
	pchkmgr->fCheckpoint = true;
}

void ReturnChkmgrToCheckpoint(CHKMGR* pchkmgr)
{
	if (pchkmgr == nullptr)
		return;

	LEVELINFO* plevelCurrent = g_transition.m_plevelCurrent;

	if (plevelCurrent == nullptr && !g_transition.m_worldCurrent.empty())
		plevelCurrent = PlevelinfoFromLevelName(g_transition.m_worldCurrent);

	if (plevelCurrent == nullptr)
	{
		std::printf("CHECKPOINT RETURN ERROR: no current level\n");
		return;
	}

	TRANS trans{};
	trans.fSet = 1;
	trans.plevel = plevelCurrent;

	if (pchkmgr->fCheckpoint)
	{
		trans.oidWarp = pchkmgr->csSaved.oidWarp;
		trans.oidWarpContext = pchkmgr->csSaved.oidWarpContext;
		trans.grftrans = static_cast<GRFTRANS>(0x11);
	}
	else
	{
		trans.oidWarp = OID_Nil;
		trans.oidWarpContext = OID_Nil;
		trans.grftrans = static_cast<GRFTRANS>(0x10);
	}

	ActivateWipe(&g_wipe, &trans, WIPEK_Keyhole);
}

void RestoreChkmgrFromCheckpoint(CHKMGR* pchkmgr)
{
	pchkmgr->cbitChk = 0;
	pchkmgr->abitChk = pchkmgr->csSaved.abitChk;
	pchkmgr->fChkDirty = false;
}

int IchkAllocChkmgr(CHKMGR* pchkmgr)
{
	pchkmgr->cbitChk++;
	return pchkmgr->cbitChk;
}

int FGetChkmgrIchk(CHKMGR* pchkmgr, int ichk)
{
	int ibitByte = ichk >= 0 ? ichk : ichk + 7;
	return (pchkmgr->abitChk.m_ab[ibitByte >> 3] & (1 << (ichk & 7))) != 0;
}

void SetChkmgrIchk(CHKMGR* pchkmgr, int ichk)
{
	uint8_t& bits = pchkmgr->abitChk.m_ab[ichk >> 3];
	uint8_t mask = static_cast<uint8_t>(1u << (ichk & 7));

	if ((bits & mask) == 0) 
	{
		pchkmgr->fChkDirty = true;
		bits |= mask;
		++pchkmgr->abitChk.m_cbitSet;
	}
}

void ClearChkmgrIchk(CHKMGR* pchkmgr, int ichk)
{
	if (ichk < 0 || ichk >= pchkmgr->cbitChk || ichk >= 4096)
		return;

	uint8_t& bits = pchkmgr->abitChk.m_ab[ichk >> 3];
	uint8_t mask = static_cast<uint8_t>(1u << (ichk & 7));

	if ((bits & mask) != 0) {
		bits &= static_cast<uint8_t>(~mask);
		--pchkmgr->abitChk.m_cbitSet;
		pchkmgr->fChkDirty = true;
	}
}

VOL* NewVol()
{
	return new VOL;
}

int GetVolSize()
{
	return sizeof(VOL);
}

void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis)
{
	pvol->matLocal = pbis->ReadMatrix();
	pvol->posLocal = pbis->ReadVector();
	LoadTbspFromBrx(&pvol->ctsurf, pvol->atsurf, &pvol->ctbsp, pvol->atbsp, pbis);
	LoadOptionsFromBrx(pvol, pbis);
}

int FCheckVolPoint(VOL* pvol, glm::vec3* ppos)
{
	glm::vec3 posLocal;
	ConvertXfmWorldToLocal(pvol, ppos, &posLocal);
	return FCheckTbspPoint(pvol->atbsp.data(), &posLocal);
}

void CloneVol(VOL* pvol, VOL* pvolBase)
{
	CloneXfm(pvol, pvolBase);

	pvol->ctsurf = pvolBase->ctsurf;
	pvol->atsurf = pvolBase->atsurf;
	pvol->ctbsp  = pvolBase->ctbsp;
	pvol->atbsp  = pvolBase->atbsp;
}

void DeleteVol(VOL* pvol)
{
	delete pvol;
}

CHKMGR g_chkmgr;
SNIP asnipChkpnt = {2, (OID)0x545, offsetof(CHKPNT, psm)};
