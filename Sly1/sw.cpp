#include "sw.h"

void InitSw(SW* psw)
{
	InitLo((LO*)psw);

	InitDl(&psw->dlChild, 0x1C + 0xC);
	InitDl(&psw->dlMRD, 0x54 + 0xC);
	InitDl(&psw->dlMRDRealClock, 0x54 + 0xC);
	InitDl(&psw->dlBusy, 0x4C + 0xC);
	InitDl(&psw->dlBusySo, 0x434 + 0xC);
	InitDl(&psw->dlRoot, 0x2e0 + 0xC);
	InitDl(&psw->dlAsega, 0x34 + 0xC);
	InitDl(&psw->dlAsegaRealClock, 0x34 + 0xC);
	InitDl(&psw->dlAsegaPending, 0x34 + 0xC);
	InitDl(&psw->dlSma, 0x10 + 0xC);
	InitDl(&psw->dlAmb, 0x60 + 0xC);
	InitDl(&psw->dlExc, 4 + 0xC);
	InitDl(&psw->dlLight, 0x420 + 0xC);
	InitDl(&psw->dlShadow, 0xb8 + 0xC);
	InitDl(&psw->dlExplste, 100 + 0xC);
	InitDl(&psw->dlProxy, 0x2ec + 0xC);
	InitDl(&psw->dlFly, 0x60c + 0xC);
	InitDl(&psw->dlDprize, 900 + 0xC);
	InitDl(&psw->dlRat, 0x630 + 0xC);
	InitDl(&psw->dlRathole, 0x90 + 0xC);
	InitDl(&psw->dlDartFree, 0x59c + 0xC);
	InitDl(&psw->dlSpire, 0x50 + 0xC);
	InitDl(&psw->dlRail, 0x50 + 0xC);
	InitDl(&psw->dlLanding, 0x50 + 0xC);
	InitDl(&psw->dlBusyLasen, 0xb20 + 0xC);
	InitDl(&psw->dlBlipg, 0x640 + 0xC);
	InitDl(&psw->dlBlipgFree, 0x640 + 0xC);
	InitDl(&psw->dlFader, 0xc + 0xC);
	InitDl(&psw->dlRealClockFader, 0xc + 0xC);
	InitDl(&psw->dlCrfod, 0xb90 + 0xC);
	InitDl(&psw->dlShape, 0x44 + 0xC);
	InitDl(&psw->dlPathzone, 100 + 0xC);
}

void InitSwDlHash(SW* psw)
{
	for (int i = 0; i < 0x200; i++)
		InitDl(&psw->objectParents[i], 0xC + 0xC);
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

	DLI dli;
	dli.m_pdl = &psw->dlChild;

	// Loads all splice script events from binary file
	LoadSwSpliceFromBrx(psw, pbis);
	LoadOptionFromBrx(psw, pbis);
	// Loads all textures and shader data from file
	LoadShadersFromBrx(pbis);
	// Loads all the static world objects from the binary file
	LoadSwObjectsFromBrx(psw, 0x0, pbis);
	pbis->Align(0x10);
}

void LoadNameTableFromBrx(CBinaryInputStream* pbis)
{
	uint32_t numNameTables = pbis->U32Read();
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
	g_pcm = 0x0;
}

void UpdateSw(SW *psw, float dt)
{

}

void GetSwParams(SW* psw, SOP** ppsop)
{

}