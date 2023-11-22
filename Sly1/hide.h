#pragma once
#include "so.h"
#include "shape.h"
#include "pnt.h"

enum JTHK
{
	JTHK_Nil = -1,
	JTHK_Flatten = 0,
	JTHK_Duck = 1,
	JTHK_Basket = 2,
	JTHK_Nonchalant = 3,
	JTHK_Rail = 4,
	JTHK_Spire = 5,
	JTHK_Reach = 6,
	JTHK_Vault = 7,
	JTHK_Max = 8
};
enum HBSKS 
{
	HBSKS_Nil = -1,
	HBSKS_Available = 0,
	HBSKS_InUse = 1,
	HBSKS_Max = 2
};

class HBSK : public SO
{
	public:
		HBSKS hbsks;
		float tHbsks;
		DLE dleHbsk;
		float sFlattenRadius;
		int cMaxDartsStuck;
};

class HSHAPE : public SHAPE
{
	public:
		DLE dleHshape;
		JTHK jthk;
		OID oidTnHide;
		struct TN* ptnHide;
		int fTunnel;
		int grfhp;
		int fDetect;
};

class HPNT : public PNT
{
	public:
		DLE dleHpnt;
		JTHK jthk;
		float sFlattenRadius;
		OID oidTnHide;
		struct TN* ptnHide;
		int fTunnel;
		int fDetect;
		float dzIgnore;
};

void* NewHbsk();
void InitHbsk(HBSK* phbsk);
void OnHbskAdd(HBSK* phbsk);
void DeleteHbsk(LO* plo);
void* NewHshape();
void InitHshape(HSHAPE* phshape);
void OnHshapeAdd(HSHAPE* phshape);
void DeleteHshape(LO* plo);
void* NewHpnt();
void InitHpnt(HPNT* phpnt);
void DeleteHpnt(LO* plo);
void OnHpntAdd(HPNT* phpnt);
void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis);
