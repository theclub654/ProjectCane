#pragma once
#include "so.h"

class JLO : public SO
{
	public:
};

class JLOC : public ALO
{
	public:
};

void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis);
void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis);