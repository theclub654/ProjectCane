#include "geom.h"

void InitGeom(GEOM* pgeom)
{
	pgeom->aedge.clear();
	pgeom->cpos = 0;
	pgeom->apos.clear();
	pgeom->csurf = 0;
	pgeom->asurf.clear();
	pgeom->cedge = 0;
}