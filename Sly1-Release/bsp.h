#pragma once
#include "geom.h"

struct BSP 
{
    SURF *psurf;
	BSP  *pbspPos;
    BSP  *pbspNeg;
};

struct BSPC
{
	int cbsp;
	int cbspFull;
	std::vector <BSP> absp;
};

void ReadBspc(GEOM *pgeom, BSPC *pbspc, CBinaryInputStream *pbis);