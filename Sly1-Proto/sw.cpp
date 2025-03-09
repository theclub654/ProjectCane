#include "sw.h"
#include "debug.h"

class LIGHT;

std::vector<LO*> allWorldObjs;
std::vector<ALO*> allSWAloObjs;
std::vector<LIGHT*> allSwLights;
extern std::vector <SO*> allSWSoObjs;

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

void MatchSwObject(LO* ploMatch, GRFFSO grffsoMask, int fIncludeRemoved, int fProxyMatch, LO* ploContext, int cploMax, int& pcploMatch, LO** aplo, int& pcpaloBest)
{
	
}

int CploFindSwObjects(SW* psw, GRFFSO grffso, OID oid, LO* ploContext, int cploMax, LO** aplo)
{
	int cploMatch;
	int grffsoMask = grffso & 0xFF;

	if (oid == OID_Nil)
		cploMatch = 0;

	else
	{
		
	}

	return 0;
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
			allSWAloObjs[i]->pvtalo->pfnUpdateAlo(allSWAloObjs[i], dt);
	}
}

void DeleteWorld(SW *psw)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
		DeleteModel(allSWAloObjs[i]);

	DeleteSwCollision();

	for (int i = 0; i < allWorldObjs.size(); i++)
		allWorldObjs[i]->pvtlo->pfnDeleteLo(allWorldObjs[i]);

	for (int i = 0; i < g_ashd.size(); i++)
	{
		glDeleteTextures(1, &g_ashd[i].glShadowMap);
		glDeleteTextures(1, &g_ashd[i].glDiffuseMap);
		glDeleteTextures(1, &g_ashd[i].glSaturateMap);
	}

	allSWAloObjs.clear();
	allSWAloObjs.shrink_to_fit();
	allWorldObjs.clear();
	allWorldObjs.shrink_to_fit();
	allSwLights.clear();
	allSwLights.shrink_to_fit();
	allSWSoObjs.clear();
	allSWSoObjs.shrink_to_fit();

	UnloadShaders();

	g_psw = nullptr;
	g_pcm = nullptr;
	std::cout << "World Deleted\n";
}

void DeleteSw(SW* psw)
{
	delete psw;
}