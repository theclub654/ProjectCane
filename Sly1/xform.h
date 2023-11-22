#pragma once
#include "alo.h"
#include "wipe.h"
#include "vec.h"

enum EXITS
{
	EXITS_Nil = -1,
	EXITS_Blocked = 0,
	EXITS_Unblocked = 1,
	EXITS_Disabled = 2,
	EXITS_Enabled = 3,
	EXITS_Totals = 4,
	EXITS_Exiting = 5,
	EXITS_Max = 6
};
enum EDK
{
	EDK_Nil = -1,
	EDK_DestinationKey = 0,
	EDK_CurrentKey = 1,
	EDK_Max = 2
};

class XFM : public LO
{
	public:
		glm::vec3 posLocal;
		glm::mat3 matLocal;
};

class WARP : public XFM
{
	public:
		WID widmenu;
		glm::vec3 v;
		char fDefault;
		float radCmInit;
		int cpaseg;
		void** apaseg;
		int coidHide;
		std::vector <OID> aoidHide;
		int coidShowWhenDifficult;
		OID aoidShowWhenDifficult[4];
		OID oidAlias;
		int crsmg;
		RSMG arsmg[4];
};

class EXIT : public ALO
{
	public:
		int fDefault;
		int fKeyed;
		int fFollowDefault;
		int fTotals;
		EXITS exits;
		float tExits;
		int ctsurf;
		void* atsurf;
		int ctbsp;
		void* atbsp;
		WID widWarp;
		OID oidWarp;
		EDK edkAlt;
		WID widAlt;
		OID oidAlt;
		int cpaseg;
		void** apaseg;
		WIPEK wipek;
		float tWipe;
		float dtUnblock;
		float dtTriggerWipe;
};

class CAMERA : public ALO
{
	public:
		OID oidTarget;
		struct PNT* ppntTarget;
		struct ALO* paloTarget;
		glm::vec3 posEye;
		glm::vec3 vecView;
		glm::vec3 vecUp;
		int fSetCplcy;
};

void* NewXfm();
void InitXfm(XFM* pxfm);
void LoadXfmFromBrx(XFM* pxfm, CBinaryInputStream* pbis);
void DeleteXfm(LO* plo);
void* NewWarp();
void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis);
void DeleteWarp(LO* plo);
void* NewExit();
void LoadExitFromBrx(EXIT *pexit, CBinaryInputStream* pbis);
void DeleteExit(LO* plo);
void* NewCamera();
void InitCamera(CAMERA* pcamera);
void DeleteCamera(LO* plo);