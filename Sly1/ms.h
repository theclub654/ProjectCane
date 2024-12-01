#pragma once
#include "so.h"

// Merged Static
class MS : public SO
{
	public:
};

MS*  NewMs();
int  GetMsSize();
void RenderMsGlobset(MS* pms, CM* pcm, RO* pro);
void DeleteMs(LO* plo);