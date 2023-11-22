#pragma once
#include "lo.h"

struct SMS
{
	OID oid;
	OID oidNext;
	int ismsNext;
};
struct SMT
{
	int fAseg;
	struct ASEG *paseg;
	int ismsFrom;
	int ismsTo;
	int grfsmt;
	float gProbability;
};

class SM : public LO
{
	public:
		int csms;
		SMS *asms;
		int csmt;
		SMT *asmt;
		int fDefault;
		DL dlSma;
};

struct SMA : public BASIC
{
	DLE dleSm;
	DLE dleSw;
	struct SM* psm;
	struct ALO* paloRoot;
	int grfapl;
	struct ASEGA* pasegaCur;
	int ismsCur;
	int ismsNext;
	int ismsGoal;
	struct SMT* psmtCur;
	float svtLocal;
	struct MQ* pmqFirst;
};
static int LoadSmFromBrxCount;

void* NewSm();
void LoadSmFromBrx(SM *psm, CBinaryInputStream *pbis);
void DeleteSm(LO* plo);
void* NewSma();
void DeleteSma(LO* plo);