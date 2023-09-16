#pragma once
#include "lo.h"
#include "difficulty.h"
#include "cm.h"

// Static World
class SW : public LO
{
public:
	// Number of SO in world
	int cpsoAll;
	int cpsoRoot;
	DL dlRoot;
	DL dlChild;
	DL dlMRD;
	DL dlBusy;
	DL dlBusySo;
	DL dlMRDRealClock;
	// Used to store the first parent of a local object.
	DL objectParents[512];
	LO* aploCidHead[162];
	DL dlAsega;
	DL dlAsegaRealClock;
	DL dlAsegaPending;
	DL dlSma;
	DL dlAmb;
	DL dlExc;
	DL dlExplste;
	DL dlLight;
	DL dlShadow;
	DL dlFader;
	DL dlRealClockFader;
	DL dlShape;
	DL dlPathzone;
	DL dlProxy;
	DL dlFly;
	DL dlDprize;
	DL dlRat;
	DL dlRathole;
	DL dlDartFree;
	DL dlSpire;
	DL dlRail;
	DL dlLanding;
	DL dlBusyLasen;
	DL dlBlipg;
	DL dlBlipgFree;
	DL dlCrfod;
	// Sound bank index for level
	int ibnk;
};

// Initializing SW object
void InitSw(SW* psw); // GOTTA COME BACK TO THIS
void InitSwDlHash(SW *psw);
// Loads all world data from binary file
void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis);// NOT FINISHED
// Loads level filenames from file
void LoadNameTableFromBrx(CBinaryInputStream* pbis);
// Loads the world table from file
void LoadWorldTableFromBrx(CBinaryInputStream* pbis);
// Resets SW object
void DeleteSw(SW* psw);

void UpdateSw(SW *psw, float dt);
void GetSwParams(SW* psw, SOP** ppsop);
// Global pointer to parent static world object
static SW* g_psw;