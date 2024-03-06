#include "sw.h"
#include "debug.h"

std::vector<LO*> allWorldObjs;
std::vector<ALO*> allSWAloObjs;
std::vector<LIGHT*> allSwLights;
extern std::vector <SO*> allSWSoObjs;

void* NewSw()
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
	psw->lsmDefault.uShadow = 50.0;
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
	std::cout << "Loading World...\n";
	// Setting difficulty for world
	OnDifficultyWorldPreLoad(&g_difficulty);
	// Loading unknown debug flag from file
	pbis->U8Read();
	// Loading index sound bank from file
	psw->ibnk = pbis->S16Read();
	// Loads world table from file
	LoadWorldTableFromBrx(pbis);
	// Loads level filenames from file
	LoadNameTableFromBrx(pbis);
	// Making new camera object for world
	g_pcm = (CM*)PloNew(CID_CM, psw, nullptr, OID__CAMERA, -1);
	// Loads all splice script events from binary file
	LoadSwSpliceFromBrx(psw, pbis);
	//LoadOptionFromBrx(psw, pbis);
	LoadOptionsFromBrx(psw, pbis);
	// Loads all textures and shader data from file
	LoadShadersFromBrx(pbis);
	// Loads all the static world objects from the binary file
	LoadSwObjectsFromBrx(psw, nullptr, pbis);
	// Aligns binary stream to texture data
	pbis->Align(0x10);
	std::cout << "Loading Textures...\n";
	// Loads textures from binary file
	LoadTexturesFromBrx(pbis);
	psw->lsmDefault.uShadow = psw->lsmDefault.uShadow * 0.003921569;
	psw->lsmDefault.uMidtone = psw->lsmDefault.uMidtone * 0.003921569;
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
	PSL* psl = psw->apsl + ipsl;
	// Returns proxy source LO from that psl
	return psl->aploClone[psl->cploCloneFree -= 1];
}

void GetSwParams(SW* psw, SOP** ppsop)
{

}

void SetSwIllum(SW* psw, float uMidtone)
{
	psw->lsmDefault.uMidtone = uMidtone;
}

void SetSwIllumShadow(SW* psw, float uShadow)
{
	psw->lsmDefault.uShadow = uShadow;
}

void SetSwDarken(SW* psw, float rDarken)
{
	psw->rDarken = rDarken;
	psw->rDarkenSmooth = rDarken;
}

void SetSwDarkenSmooth(SW* psw, float rDarkenSmooth)
{
	psw->rDarkenSmooth = rDarkenSmooth;
}

void MatchSwObject(LO* ploMatch, GRFFSO grffsoMask, int fIncludeRemoved, int fProxyMatch, LO* ploContext, int cploMax, int& pcploMatch, LO** aplo, int& pcpaloBest)
{
	ALO* palo = nullptr;

	switch (grffsoMask)
	{
		case 1:
			if (ploMatch->ppxr == nullptr)
				palo = ploMatch->paloParent;

			else
			{
				if (fProxyMatch == 0)
					return;

				palo = ploMatch->paloParent;
			}

			if (palo == nullptr)
				return;


	}
}

int CploFindSwObjects(SW* psw, GRFFSO grffso, OID oid, LO* ploContext, int cploMax, LO** aplo)
{
	int cpaloBest = 0;
	int cploMatch = 0;
	int count = 0;
	ALO* palo = nullptr;

	if (oid == OID_Nil)
		cploMatch = 0;

	else
	{
		if ((grffso & 0xff) == 1)
		{
			palo = (ALO*)ploContext;

			if (ploContext != nullptr && ((ploContext->pvtlo->grfcid ^ 1U) & 1) != 0)
				palo = ploContext->paloParent;

			count = 0;

			if (palo != nullptr)
			{
				while (true)
				{
					count++;
					palo = palo->paloParent;
					if (palo == nullptr) break;
				}
			}

			cpaloBest = 0x7fffffff;
			if (ploContext == nullptr && (grffso & 0xff) == 3, (grffso & 0xff) == 5)
			{
				cploMatch = 0;
				return cploMatch;
			}
		}

		else if ((grffso & 0xff) == 0 || 4 < (grffso & 0xff))
			palo = nullptr;

		else
		{
			palo = nullptr;
			///////////////////////////////////////////
		}

		cploMatch = 0;

		LO* ploMatch = PdlFromSwOid(psw, oid)->ploFirst;

		if (ploMatch != nullptr)
		{
			while (true)
			{
				if (ploMatch->oid == oid)
				{
					if ((ploMatch->pvtlo->grfcid & 0x100U) == 0)
					{
						if (ploMatch->ppxr == nullptr || ploMatch->ppxr->oidProxyRoot != ploMatch->oid)
							MatchSwObject(ploMatch, grffso & 0xFF, grffso & 0x100, 0, ploContext, cploMax, cploMatch, aplo, cpaloBest);
					}

					else
					{
						///GOTTA COME BACK HERE
					}

				}

				else
					ploMatch = ploMatch->dleOid.ploNext;

				if (ploMatch == nullptr) break;
			}
		}
	}

	return cploMatch;
}

LO* PloFindSwObject(SW* psw, GRFFSO grffso, OID oid, LO* ploContext)
{
	LO* plo = nullptr;
	CploFindSwObjects(psw, grffso | 0x200, oid, ploContext, 1, &plo);
	return plo;
}

void UpdateSw(SW* psw, float dt)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
	{
		if (allSWAloObjs[i]->pvtalo->pfnUpdateAlo != nullptr)
			allSWAloObjs[i]->pvtalo->pfnUpdateAlo(allSWAloObjs[i], 0);
	}
}

void DeleteSw(SW* psw)
{
	if (psw != nullptr)
	{
		DeleteWorld(psw);
		UnloadShaders();
		g_pcm = nullptr;
		std::cout << "World Deleted\n";
	}
}

void DeleteWorld(SW* psw)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
		DeleteModel(allSWAloObjs[i]);

	DeleteSwCollision();

	for (int i = 0; i < allWorldObjs.size(); i++)
		allWorldObjs[i]->pvtlo->pfnDeleteLo(allWorldObjs[i]);

	for (int i = 0; i < g_ashd.size(); i++)
	{
		glDeleteTextures(1, &g_ashd[i].glAmbientTexture);
		glDeleteTextures(1, &g_ashd[i].glDiffuseTexture);
		glDeleteTextures(1, &g_ashd[i].glGradiantTexture);
	}

	allSWAloObjs.clear();
	allSWAloObjs.shrink_to_fit();
	allWorldObjs.clear();
	allWorldObjs.shrink_to_fit();
	allSwLights.clear();
	allSwLights.shrink_to_fit();

	g_psw = nullptr;
}

void DeleteSwObj(LO* plo)
{
	delete (SW*)plo;
}