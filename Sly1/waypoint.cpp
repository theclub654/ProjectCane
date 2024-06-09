#include "waypoint.h"

WAYPOINT* NewWaypoint()
{
	return new WAYPOINT{};
}

void InitWaypoint(WAYPOINT* pwaypoint)
{
	InitAlo(pwaypoint);
}

int GetWaypointSize()
{
	return sizeof(WAYPOINT);
}

void CloneWaypoint(WAYPOINT* pwaypoint, WAYPOINT* pwaypointBase)
{
	LO lo = *pwaypoint;
	*pwaypoint = *pwaypointBase;
	memcpy(pwaypoint, &lo, sizeof(LO));

	CloneLo(pwaypoint, pwaypointBase);

	ClearDl(&pwaypoint->dlChild);
}

void DeleteWaypoint(LO* plo)
{
	delete(WAYPOINT*)plo;
}
