#pragma once
#include "lo.h"
#include "dec.h"

struct SMS
{
	OID oid;
	OID oidNext;
	int ismsNext;
};

struct SMT
{
	int fAseg;
	//ASEG *paseg;
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

static int LoadSmFromBrxCount;
void LoadSmFromBrx(SM *psm, CBinaryInputStream *pbis);