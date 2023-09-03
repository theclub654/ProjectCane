#include "sw.h"

SW::SW(CID cid, ALO* paloParent, OID oid)
{
	this->cid = cid;
	this->paloParent;
	this->oid = oid;

	for (int i = 0; i < 0x200; i++)
		InitDl(&objectParents[i], 0xC);
}

void InitSw(SW *psw)
{
	InitLo((LO*)psw);

	InitDl(&psw->dlChild, 0x1C);
	InitDl(&psw->dlMRD, 0x54);
	InitDl(&psw->dlMRDRealClock, 0x54);
	InitDl(&psw->dlBusy, 0x4C);
	InitDl(&psw->dlBusySo, 0x434);
	InitDl(&psw->dlRoot, 0x2e0);
	InitDl(&psw->dlAsega, 0x34);
	InitDl(&psw->dlAsegaRealClock, 0x34);
	InitDl(&psw->dlAsegaPending, 0x34);
	InitDl(&psw->dlSma, 0x10);
	InitDl(&psw->dlAmb, 0x60);
	InitDl(&psw->dlExc, 4);
	InitDl(&psw->dlLight, 0x420);
	InitDl(&psw->dlShadow, 0xb8);
	InitDl(&psw->dlExplste, 100);
	InitDl(&psw->dlProxy, 0x2ec);
	InitDl(&psw->dlFly, 0x60c);
	InitDl(&psw->dlDprize, 900);
	InitDl(&psw->dlRat, 0x630);
	InitDl(&psw->dlRathole, 0x90);
	InitDl(&psw->dlDartFree, 0x59c);
	InitDl(&psw->dlSpire, 0x50);
	InitDl(&psw->dlRail, 0x50);
	InitDl(&psw->dlLanding, 0x50);
	InitDl(&psw->dlBusyLasen, 0xb20);
	InitDl(&psw->dlBlipg, 0x640);
	InitDl(&psw->dlBlipgFree, 0x640);
	InitDl(&psw->dlFader, 0xc);
	InitDl(&psw->dlRealClockFader, 0xc);
	InitDl(&psw->dlCrfod, 0xb90);
	InitDl(&psw->dlShape, 0x44);
	InitDl(&psw->dlPathzone, 100);
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
}

void LoadNameTableFromBrx(CBinaryInputStream* pbis)
{
	uint32_t numNameTables = pbis->U32Read();
}

void LoadWorldTableFromBrx(CBinaryInputStream* pbis)
{
	// Storing number of world tables from binary file
	int worldTableCount = pbis->U32Read();

	std::string temp;

	// Loading world table from binary file
	for (int i = 0; i < worldTableCount; i++)
		pbis->ReadStringSw(&temp);
}

void DeleteSw(SW* psw)
{

}