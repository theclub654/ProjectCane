#pragma once
#include "so.h"

class TZP : public SO
{
	public:
};

class VOLZP : public TZP
{

};

void InitTzp(TZP* ptzp);
void InitVolzp(VOLZP* pvolzp);
void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro);