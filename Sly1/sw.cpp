#include "sw.h"
#include "debug.h"

std::vector<LO*> allWorldObjs;
std::vector<ALO*> allSWAloObjs;
std::vector<void*> allSwLights;
extern std::vector <GEOM*> allcollisionModels;

void* NewSw()
{
	return new SW{};
}

void InitSw(SW* psw)
{
	InitLo(psw);

	InitDl(&psw->dlChild, offsetof(LO, dleChild));
	InitDl(&psw->dlMRD, offsetof(ALO, dleMRD));
	InitDl(&psw->dlMRDRealClock, 0x1C + 0x54); // GOTTA COME BACK TO THIS
	InitDl(&psw->dlBusy, offsetof(ALO, dleBusy));
	InitDl(&psw->dlBusySo, 0x790);
	InitDl(&psw->dlRoot, 0x458);
	InitDl(&psw->dlAsega, 0x1C + 0x34); // GOTTA COME BACK TO THIS
	InitDl(&psw->dlAsegaRealClock, 0x1C + 0x34); // GOTTA COME BACK TO THIS
	InitDl(&psw->dlAsegaPending, 0x1C + 0x34); // GOTTA COME BACK TO THIS
	InitDl(&psw->dlSma, 0x10);
	InitDl(&psw->dlAmb, 0x1C + 0x60); // GOTTA COME BACK TO THIS
	InitDl(&psw->dlExc, 0x1C + 4); // GOTTA COME BACK TO THIS
	InitDl(&psw->dlLight, 0x568);
	InitDl(&psw->dlShadow, 0x1C + 0xb8); // GOTTA COME BACK TO THIS
	InitDl(&psw->dlExplste, 0x1C + 100);// GOTTA COME BACK TO THIS
	InitDl(&psw->dlProxy, 0x470);
	InitDl(&psw->dlFly, 0xB50);
	InitDl(&psw->dlDprize, 0x500);
	InitDl(&psw->dlRat, 0xB90);
	InitDl(&psw->dlRathole, 0xC8);
	InitDl(&psw->dlDartFree, 0xA70);
	InitDl(&psw->dlSpire, 0x88);
	InitDl(&psw->dlRail, 0x90);
	InitDl(&psw->dlLanding, 0x90); 
	InitDl(&psw->dlBusyLasen, 0x1200);
	InitDl(&psw->dlBlipg, 0x1C + 0x640);// GOTTA COME BACK TO THIS
	InitDl(&psw->dlBlipgFree, 0x1C + 0x640);// GOTTA COME BACK TO THIS
	InitDl(&psw->dlFader, 0x1C + 0xc);// GOTTA COME BACK TO THIS
	InitDl(&psw->dlRealClockFader, 0x1C + 0xc);// GOTTA COME BACK TO THIS
	InitDl(&psw->dlCrfod, 0x11A8);
	InitDl(&psw->dlShape, 0x80);
	InitDl(&psw->dlPathzone, 0xB8);
}

int GetSwSize()
{
	return sizeof(SW);
}

void InitSwDlHash(SW* psw)
{
	for (int i = 0; i < 0x200; i++)
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

	PSL proxySourceList;

	proxySourceList.cploCloneFree = cploClone;
	proxySourceList.aploClone.resize(cploClone);

	for (int i = 0; i < cploClone; i++)
	{
		LO* clonedLocalObject = PloCloneLo(ploProxySource, psw, nullptr);
		proxySourceList.aploClone[i] = clonedLocalObject;
	}

	psw->apsl.push_back(proxySourceList);
	psw->cpsl++;
}

LO* PloGetSwProxySource(SW* psw, int ipsl)
{
	PSL psl = psw->apsl[ipsl];
	int numClones = psl.cploCloneFree--;
	numClones--;
	psl.cploCloneFree = numClones;
	return psl.aploClone[numClones];
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

	for (int i = 0; i < allcollisionModels.size(); i++)
	{
		glDeleteVertexArrays(1, &allcollisionModels[i]->VAO);
		glDeleteBuffers(1, &allcollisionModels[i]->VBO);
		glDeleteBuffers(1, &allcollisionModels[i]->EBO);
	}

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
	allcollisionModels.clear();
	allcollisionModels.shrink_to_fit();

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