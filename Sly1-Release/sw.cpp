#include "sw.h"

SW* NewSw()
{
	return new SW{};
}

void InitSw(SW* psw)
{
	InitLo(psw);

	InitDl(&psw->dlChild, offsetof(LO, dleChild));
	InitDl(&psw->dlMRD, offsetof(ALO, dleMRD));
	InitDl(&psw->dlMRDRealClock, offsetof(ALO, dleMRD));
	InitDl(&psw->dlBusy, offsetof(ALO, dleBusy));
	InitSwBusySoDl(psw);
	InitSwRootDl(psw);
	InitSwAsegaDl(psw);
	InitSwAsegaRealClockDl(psw);
	InitSwAsegaPending(psw);
	InitSwSmaDl(psw);
	InitDl(&psw->dlAmb, 0x1C + 0x60); // GOTTA COME BACK TO THIS
	InitDl(&psw->dlExc, 0x1C + 4); // GOTTA COME BACK TO THIS
	InitSwLightDl(psw);
	InitSwShadowDl(psw);
	InitDl(&psw->dlExplste, 100);// GOTTA COME BACK TO THIS
	InitSwProxyDl(psw);
	InitSwFlyDl(psw);
	InitSwDprizeDl(psw);
	InitSwRatDl(psw);
	InitSwRatholeDl(psw);
	InitSwDartFreeDl(psw);
	InitSwSpireDl(psw);
	InitSwRailDl(psw);
	InitSwLandingDl(psw);
	InitSwLasenDl(psw);
	InitSwBlipgDl(psw);
	InitSwBlipgFreeDl(psw);
	InitSwFaderDl(psw);
	InitSwRealClockFader(psw);
	InitSwCrfodDl(psw);
	InitSwShapeDl(psw);
	InitSwPathzoneDl(psw);

	psw->rDarkenSmooth = 1.0;
	psw->lsmDefault.uShadow = 50.0;
	psw->musid = MUSID_Nil;
	psw->ibnk = -1;
	psw->cisi = 0;
	psw->cpsl = 0;
	psw->rDarken = 1.0;
}

int GetSwSize()
{
	return sizeof(SW);
}

void InitSwDlHash(SW* psw)
{
	for (int i = 0; i < 512; i++)
		InitDl(&psw->adlHash[i], offsetof(LO, dleOid));
}

void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis)
{
	std::cout << "\nLoading World...\n";
	// Setting difficulty for world
	OnDifficultyWorldPreLoad(&g_difficulty);
	// Loading unknown debug flag from file
	pbis->U8Read();
	// Loading index sound bank from file
	psw->ibnk = pbis->S16Read();
	// Loads world table from file
	//LoadWorldTableFromBrx(pbis);
	//// Loads level filenames from file
	//LoadNameTableFromBrx(pbis);
	pbis->U32Read();
	// Making new camera object for world
	g_pcm = (CM*)PloNew(CID_CM, psw, nullptr, OID__CAMERA, -1);
	// Loads all splice script events from binary file
	LoadSwSpliceFromBrx(psw, pbis);
	//LoadOptionFromBrx(psw, pbis);
	LoadOptionsFromBrx(psw, pbis);
	pbis->file.seekg(0x20, SEEK_CUR);
	pbis->file.seekg(pbis->S16Read(), SEEK_CUR);
	pbis->file.seekg(0x20, SEEK_CUR);
	// Loads all textures and shader data from file
	LoadShadersFromBrx(pbis);
	//std::cout << std::hex << pbis->file.tellg();
	// Loads all the static world objects from the binary file
	LoadSwObjectsFromBrx(psw, nullptr, pbis);
	AllocateRpl();
	AllocateLightBlkList();
	// Aligns binary stream to texture data
	pbis->Align(0x10);
	std::cout << "Loading Textures...\n";
	// Loads textures from binary file
	LoadTexturesFromBrx(pbis);
	psw->lsmDefault.uShadow  *= 0.003921569;
	psw->lsmDefault.uMidtone *= 0.003921569;
	//SetupCm(g_pcm);
	std::cout << "World Loaded Successfully\n";
}

void LoadNameTableFromBrx(CBinaryInputStream* pbis)
{
	pbis->U32Read();
}

void LoadWorldTableFromBrx(CBinaryInputStream* pbis)
{
	// Storing number of world tables from binary file
	int worldTableCount = pbis->U32Read();

	// Loading world table from binary file
	for (int i = 0; i < worldTableCount; i++)
		pbis->ReadStringSw();
}

void AddSwProxySource(SW* psw, LO* ploProxySource, int cploClone)
{
	cploClone--;

	PSL psl{};

	psl.cploCloneFree = cploClone;
	psl.aploClone.resize(cploClone);

	for (int i = 0; i < cploClone; i++)
	{
		LO* clonedLocalObject = PloCloneLo(ploProxySource, psw, nullptr);
		psl.aploClone[i] = clonedLocalObject;
	}

	psw->apsl[psw->cpsl] = psl;
	psw->cpsl++;
}

LO* PloGetSwProxySource(SW* psw, int ipsl)
{
	// Loads the psl
	PSL *psl = psw->apsl + ipsl;
	// Returns proxy source LO from that psl
	return psl->aploClone[psl->cploCloneFree -= 1];
}

void GetSwParams(SW* psw, SOP** ppsop)
{

}

void* GetSwIllum(SW *psw)
{
	return &psw->lsmDefault.uMidtone;
}

void* GetSwIllumShadow(SW *psw)
{
	return &psw->lsmDefault.uShadow;
}

void SetSwIllum(SW *psw, float uMidtone)
{
	psw->lsmDefault.uMidtone = uMidtone;
}

void SetSwIllumShadow(SW *psw, float uShadow)
{
	psw->lsmDefault.uShadow = uShadow;
}

void* GetSwDarken(SW* psw)
{
	return &psw->rDarken;
}

void SetSwDarken(SW *psw, float rDarken)
{
	psw->rDarken = rDarken;
	psw->rDarkenSmooth = rDarken;
}

void* GetSwDarkenSmooth(SW *psw)
{
	return &psw->rDarkenSmooth;
}

void SetSwDarkenSmooth(SW* psw, float rDarkenSmooth)
{
	psw->rDarkenSmooth = rDarkenSmooth;
}

void MatchSwObject(ALO* ploMatch, GRFFSO grffsoMask, int fIncludeRemoved, int fProxyMatch, LO* ploContext, int cploMax, int* pcploMatch, LO** aplo, int* pcpaloBest)
{
	ALO* candidate = nullptr;
	ALO* contextRoot = nullptr;
	ALO* current = nullptr;
	ALO* parent = nullptr;
	std::shared_ptr <PXR> proxy = nullptr;
	OID oidA = OID_Nil;
	OID oidB = OID_Nil;

	switch (grffsoMask)
	{
	case 1: { // Search for cpaloFindSwObjects == 1 in parents
		parent = ploMatch->paloParent;
		if (ploMatch->ppxr != nullptr && fProxyMatch == 0) return;
		if (parent == nullptr) return;

		uint64_t flags = *(uint64_t*)&parent->bitfield;
		while (((flags >> 41) & 0xF) == 0) {
			if (parent->ppxr != nullptr) return;
			parent = parent->paloParent;
			if (parent == nullptr) return;
			flags = *(uint64_t*)&parent->bitfield;
		}
		if (((flags >> 41) & 0xF) != 1) return;
		break;
	}

	case 2: { // Check if parent is the context
		parent = ploMatch->paloParent;
		if (ploMatch->ppxr != nullptr && fProxyMatch == 0) return;
		if (parent != (ALO*)ploContext) return;
		break;
	}

	case 3: { // Check if object is in-world and has cpaloFindSwObjects != 0
		if ((ploMatch->pvtlo->grfcid & 1) == 0) return;

		uint64_t flags = *(uint64_t*)&ploMatch->bitfield;
		if ((flags & 0x1E0000000000) == 0) return;
		break;
	}

	case 4: { // Climb hierarchy looking for closest cpaloFindSwObjects match
		if ((ploMatch->pvtlo->grfcid & 1) == 0) {
			current = ploMatch->paloParent;
		}
		else {
			current = (ALO*)ploMatch;
		}

		while (current != nullptr) {
			uint64_t flags = *(uint64_t*)&current->bitfield;

			if ((flags & 0x1E0000000000) != 0) {
				int score = (flags >> 41) & 0xF;
				if (score <= *pcpaloBest) {
					if (score < *pcpaloBest) {
						*pcpaloBest = score;
						*pcploMatch = 0;
					}
					goto ADD_MATCH;
				}
			}

			if (current == candidate) return;

			proxy = current->ppxr;
			if (proxy == nullptr) {
				current = current->paloParent;
			}
			else if (current == (ALO*)ploMatch && fProxyMatch != 0) {
				current = current->paloParent;
			}
			else {
				candidate = (ALO*)ploContext;
				oidB = OID_Nil;

				// Find oidProxyRoot of the context (if any)
				while (candidate != nullptr) {
					if (candidate->ppxr != nullptr) {
						oidB = candidate->ppxr->oidProxyRoot;
						break;
					}
					candidate = candidate->paloParent;
				}

				oidA = proxy->oidProxyRoot;
				if (oidA != oidB) return;

				current = current->paloParent;
			}
		}

		if (*pcpaloBest != 0x7FFFFFFF) return;
		break;
	}

	case 5:
		// Always match
		break;

	default:
		// Invalid or unsupported mask
		return;
	}

ADD_MATCH:
	// Optionally check if object is in-world
	if (fIncludeRemoved == 0 && FIsLoInWorld(ploMatch) == 0) return;

	// Store match if within bounds
	int matchCount = *pcploMatch;
	if (matchCount < cploMax) {
		aplo[matchCount] = ploMatch;
	}
	*pcploMatch = matchCount + 1;
}

int CploFindSwObjects(SW* psw, GRFFSO grffso, OID oid, LO* ploContext, int cploMax, LO** aplo)
{
	if (oid == OID_Nil) {
		return 0;  // No valid OID to search for
	}

	uint32_t grffsoMask = grffso & 0xFF;
	bool shouldLimit = !(grffso & 0x200);
	bool allowProxy = grffso & 0x100;

	ALO* contextAlo = NULL;

	// Determine parent ALO traversal if mask indicates
	if (grffsoMask == 1 || grffsoMask == 2 || grffsoMask == 3) {
		contextAlo = (ALO*)ploContext;

		// Special case: if context is not suitable, use its parent
		if (ploContext && ((ploContext->pvtlo->grfcid ^ 1U) & 1))
		{
			contextAlo = ploContext->paloParent;
		}

		// Flag the ALO parent chain for match context
		for (ALO* alo = contextAlo; alo; alo = alo->paloParent)
		{
			uint64_t flags = *(uint64_t*)&alo->bitfield;
			// Manipulating the bitfield flags in `field_0x2d8`
			*(uint64_t*)&alo->bitfield = (flags & ~0x1E0000000000) | ((alo ? 1 : 0) << 0x29);
		}

		// If context is null and mask == 3, nothing to do
		if (!ploContext && grffsoMask == 3) {
			return 0;
		}

		// If mask == 3, fall through with mask updated to 5
		grffsoMask = 5;
	}

	// Lookup objects with matching OID
	DL* pdl = PdlFromSwOid(psw, oid);
	if (!pdl) return 0;

	int cploMatch = 0;
	int bestDistance = INT_MAX;
	LO* match = pdl->ploFirst;

	while (match) {
		if (match->oid == oid) {
			bool isProxy = match->ppxr && (match->ppxr->oidProxyRoot == oid);
			bool isDeleted = match->pvtlo->grfcid & 0x100;

			// Checking `ALO` specific bitflags for matching:
			// `ALO->fHidden` flag, for example, indicates if the object is hidden
			if (!isDeleted) {
				if (!isProxy) {
					// Regular object match
					MatchSwObject((ALO*)match, grffsoMask, allowProxy, 0, ploContext,
						cploMax, &cploMatch, aplo, &bestDistance);
				}
				else
				{
					// Proxy object match through ACTSEG list
					/*ACTSEG* seg = (ACTSEG*)match[0xb].dleChild.field1_0x4.ppxrPrev;
					while (seg) {
						LO* segObj = *(LO**)&seg->field0_0x0;
						MatchSwObject(segObj, grffsoMask, allowProxy, 1, ploContext,
							cploMax, &cploMatch, aplo, &bestDistance);
						seg = seg->dleAlo.field0_0x0.pactsegNext;
					}*/
				}
			}
		}

		match = match->dleOid.ploNext;
	}

	// Clear temporary flags from context ALO tree
	for (ALO* alo = contextAlo; alo; alo = alo->paloParent) {
		uint64_t flags = *(uint64_t*)&alo->bitfield;
		*(uint64_t*)&alo->bitfield = flags & ~0x1E0000000000;
	}

	// Enforce cploMax if limit flag is not set
	if (shouldLimit && cploMatch > cploMax) {
		cploMatch = cploMax;
	}

	return cploMatch;
}

LO* PloFindSwObject(SW* psw, GRFFSO grffso, OID oid, LO* ploContext)
{
	LO* plo = nullptr;
	CploFindSwObjects(psw, grffso | 0x200, oid, ploContext, 1, &plo);
	return plo;
}

LO* PloFindSwNearest(SW* psw, OID oid, LO* ploContext)
{
	LO* plo = nullptr;
	CploFindSwObjects(psw, 0x204, oid, ploContext, 1, &plo);
	return plo;
}

void UpdateSw(SW* psw, float dt)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
	{
		if (allSWAloObjs[i]->pvtalo->pfnUpdateAlo != nullptr)
			allSWAloObjs[i]->pvtalo->pfnUpdateAlo(allSWAloObjs[i], dt);
	}
}

void DeleteWorld(SW *psw)
{
	renderBuffer.clear();
	renderBuffer.shrink_to_fit();

	DeallocateLightBlkList();

	numRo = 0;
	g_dynamicTextureCount = 0;
	g_backGroundCount = 0;
	g_blotContextCount = 0;
	g_opaqueCount = 0;
	g_cutOutCount = 0;
	g_celBorderCount = 0;
	g_projVolumeCount = 0;
	g_opaqueAfterProjVolumeCount = 0;
	g_cutoutAfterProjVolumeCount = 0;
	g_celBorderAfterProjVolumeCount = 0;
	g_murkClearCount = 0;
	g_murkOpaqueCount = 0;
	g_murkFillCount = 0;
	g_translucentCount = 0;
	g_translucentCelBorderCount = 0;
	g_blipCount = 0;
	g_foreGroundCount = 0;
	g_worldMapCount = 0;
	g_maxCount = 0;

	for (int i = 0; i < allSWAloObjs.size(); i++)
		DeleteModel(allSWAloObjs[i]);

	DeleteSwCollision();

	for (int i = 0; i < allWorldObjs.size(); i++)
		allWorldObjs[i]->pvtlo->pfnDeleteLo(allWorldObjs[i]);

	allSWAloObjs.clear();
	allSWAloObjs.shrink_to_fit();
	allWorldObjs.clear();
	allWorldObjs.shrink_to_fit();
	DeallocateLightVector();
	DeallocateSoVector();

	UnloadShaders();

	g_psw = nullptr;
	g_pcm = nullptr;

	std::cout << "World Deleted\n";
}

void DeleteSw(SW* psw)
{
	delete psw;
}