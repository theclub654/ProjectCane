#pragma once
#include "button.h"

class JLO : public SO
{
	public:
};

class JLOC : public ALO
{
	public:
};

class JLOVOL : public VOLBTN
{
	public:
};

void InitJlovol(JLOVOL* pjlovol);
void InitJlo(JLO* pjlo);
void InitJloc(JLOC* pjloc);
void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis);
void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis);