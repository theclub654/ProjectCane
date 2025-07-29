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

SM*  NewSm();
int  GetSmSize();
void LoadSmFromBrx(SM *psm, CBinaryInputStream *pbis);
OID  OidFromSmIsms(SM* psm, int isms);
void CloneSm(SM* psm, SM* psmBase);
void DeleteSm(SM* psm);

SMA* NewSma();
void InitSwSmaDl(SW* psw);
void CloneSma(SMA* psma, SMA* psmaBase);
int  GetSmaSize();
void DeleteSma(LO* plo);