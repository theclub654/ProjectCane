#include "waypoint.h"

void* NewWaypoint()
{
	return new WAYPOINT;
}

void InitWaypoint(WAYPOINT* pwaypoint)
{
	InitAlo(pwaypoint);
}

void DeleteWaypoint(LO* plo)
{
	delete(WAYPOINT*)plo;
}
