#pragma once
#include "lo.h"

class VISMAP : public LO
{
	int cvbsp;
	struct VBSP *avbsp;
	int cgrfzon;
	int *agrfzonOneHop;
};

void* NewVismap();
// Initializing vismap object
void InitVismap(VISMAP *pvismap);
// Loading all the visibility map data from binary file
void LoadVismapFromBrx(VISMAP *pvismap, CBinaryInputStream *pbis);
void DeleteVismap(LO* plo);