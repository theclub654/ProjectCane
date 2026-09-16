#pragma once
#include "expl.h"

class EXPLG : public EXPL
{
	public:
	int cpexpl;
	EXPL* apexpl[8];
};

EXPLG* NewExplg();
int  GetExplgSize();
void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis);
void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase);
void BindExplg(EXPLG* pexplg);
void ExplodeExplgExplso(EXPLG* pexplg, EXPLSO* pexplso);
void DeleteExplg(EXPLG* pexplg);