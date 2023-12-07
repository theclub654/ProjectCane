#include "sw.h"
#include "debug.h"

std::vector<LO*> allWorldObjs;
std::vector<ALO*> allSWAloObjs;
extern std::vector<void*> allSwLights;
extern std::vector <GEOM*> allcollisionModels;

void* NewSw()
{
	return new SW;
}

void InitSw(SW* psw)
{
	InitLo(psw);

	InitDl(&psw->dlChild, 0x38);
	InitDl(&psw->dlMRD, 0xA0);
	InitDl(&psw->dlMRDRealClock, 0x1C + 0x54);
	InitDl(&psw->dlBusy, 0x90);
	InitDl(&psw->dlBusySo, 0x1C + 0x434);
	InitDl(&psw->dlRoot, 0x428);
	InitDl(&psw->dlAsega, 0x1C + 0x34);
	InitDl(&psw->dlAsegaRealClock, 0x1C + 0x34);
	InitDl(&psw->dlAsegaPending, 0x1C + 0x34);
	InitDl(&psw->dlSma, 0x10);
	InitDl(&psw->dlAmb, 0x1C + 0x60);
	InitDl(&psw->dlExc, 0x1C + 4);
	InitDl(&psw->dlLight, 0x538);
	InitDl(&psw->dlShadow, 0x1C + 0xb8);
	InitDl(&psw->dlExplste, 0x1C + 100);
	InitDl(&psw->dlProxy, 0x1C + 0x2ec);
	InitDl(&psw->dlFly, 0x1C + 0x60c);
	InitDl(&psw->dlDprize, 0x1C + 900);
	InitDl(&psw->dlRat, 0x1C + 0x630);
	InitDl(&psw->dlRathole, 0x1C + 0x90);
	InitDl(&psw->dlDartFree, 0x1C + 0x59c);
	InitDl(&psw->dlSpire, 0x1C + 0x50);
	InitDl(&psw->dlRail, 0x1C + 0x50);
	InitDl(&psw->dlLanding, 0x1C + 0x50);
	InitDl(&psw->dlBusyLasen, 0x1C + 0xb20);
	InitDl(&psw->dlBlipg, 0x1C + 0x640);
	InitDl(&psw->dlBlipgFree, 0x1C + 0x640);
	InitDl(&psw->dlFader, 0x1C + 0xc);
	InitDl(&psw->dlRealClockFader, 0x1C + 0xc);
	InitDl(&psw->dlCrfod, 0x1C + 0xb90);
	InitDl(&psw->dlShape, 0x1C + 0x44);
	InitDl(&psw->dlPathzone, 0x1C + 100);
}

void InitSwDlHash(SW* psw)
{
	for (int i = 0; i < 0x200; i++)
		InitDl(&psw->adlHash[i], 0x38);
}

void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis)
{
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
	std::cout << "Loading Textures\n";
	ParseTextures(pbis);
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

void DeleteSw(SW* psw)
{
	if (psw != nullptr)
	{
		DeleteWorld(psw);
		UnloadShaders();
		g_pcm = nullptr;
	}
}

void DeleteWorld(SW* psw)
{
	for (int i = 0; i < allcollisionModels.size(); i++)
	{
		glDeleteVertexArrays(1, &allcollisionModels[i]->VAO);
		glDeleteBuffers(1, &allcollisionModels[i]->VBO);
	}

	for (int i = 0; i < allSWAloObjs.size(); i++)
		DeleteModel(allSWAloObjs[i]);

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
	std::cout << "World Deleted\n";
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