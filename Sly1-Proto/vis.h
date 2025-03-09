#pragma once
#include "lo.h"

struct VBSP 
{
    glm::vec3 normal;
    float gDot;
	VBSP *pvbspNeg;
	VBSP *pvbspPos;
};

class VISMAP : public LO
{
	public:

	int cvbsp;
	std::vector <VBSP> avbsp;
	int cgrfzon;
	std::vector <GRFZON> agrfzonOneHop;
};

VISMAP* NewVismap();
// Initializing vismap object
void InitVismap(VISMAP *pvismap);
int  GetVismapSize();
// Loading visibility data from binary file
void LoadVismapFromBrx(VISMAP *pvismap, CBinaryInputStream *pbis);
// Traverse BSP tree to check collision
void ClipVismapSphereOneHop(VISMAP *pvismap, glm::vec3 *ppos, float sRadius, GRFZON *pgrfzon);
void ClipVbspSphereOneHop(VISMAP *pvismap, VBSP *pvbsp, float sRadius, glm::vec3 *ppos); // NOT DONE
// Checks to see which zone the position is at in the BSP tree
void ClipVismapPointNoHop(VISMAP *pvismap, glm::vec3 *ppos, GRFZON *pgrfzon);
void DeleteVismap(VISMAP *pvismap);