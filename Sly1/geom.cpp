#include "geom.h"

void InitGeom(GEOM* pgeom)
{
	pgeom->aedge = (EDGE*)0x0;
	pgeom->cpos = 0;
	pgeom->apos = (glm::vec3*)0x0;
	pgeom->csurf = 0;
	pgeom->asurf = (SURF*)0x0;
	pgeom->cedge = 0;
}
