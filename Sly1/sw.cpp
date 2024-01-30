#include "sw.h"
#include "debug.h"

std::vector<LO*> allWorldObjs;
std::vector<ALO*> allSWAloObjs;
std::vector<void*> allSwLights;
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
	//StartupSplice();
	// Loading unknown debug flag from file
	pbis->U8Read();
	// Loading index sound bank from file
	psw->ibnk = pbis->S16Read();
	// Loads world table from file
	LoadWorldTableFromBrx(pbis);
	// Loads level filenames from file
	LoadNameTableFromBrx(pbis);
	// Initializing camera object for world
	g_pcm = (CM*)PloNew(CID_CM, psw, nullptr, OID__CAMERA, -1);
	// Loads all splice script events from binary file
	LoadSwSpliceFromBrx(psw, pbis);
	LoadOptionFromBrx(psw, pbis);
	// Loads all textures and shader data from file
	LoadShadersFromBrx(pbis);
	// Loads all the static world objects from the binary file
	LoadSwObjectsFromBrx(psw, 0x0, pbis);

	pbis->Align(0x10);
	std::cout << "Loading Textures...\n";
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

	void* temp = psw->apsl;
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
		glDeleteTextures(1, &g_ashd[i].glTexture);

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

void UpdateSw(SW *psw, float dt)
{

}

void GetSwParams(SW* psw, SOP** ppsop)
{

}