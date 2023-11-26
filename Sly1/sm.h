#pragma once
#include "lo.h"

struct SMS
{
	OID oid;
	union
	{
		OID oidNext;
		int ismsNext;
	};
};
struct SMT
{
	union
	{
		int fAseg;
		struct ASEG* paseg;
	};
	int ismsFrom;
	int ismsTo;
	int grfsmt;
	float gProbability;
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

class SM : public LO
{
	public:
		int csms;
		std::vector<SMS> asms;
		int csmt;
		std::vector<SMT> asmt;
		int fDefault;
		DL dlSma;
};

void* NewSm();
void LoadSmFromBrx(SM *psm, CBinaryInputStream *pbis);
void DeleteSm(LO* plo);
void* NewSma();
void DeleteSma(LO* plo);