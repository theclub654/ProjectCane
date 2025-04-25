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
    CloneAlo(pwaypoint, pwaypointBase);

    pwaypoint->wps = pwaypointBase->wps;
    pwaypoint->tWps = pwaypointBase->tWps;
    pwaypoint->dtPause = pwaypointBase->dtPause;
    pwaypoint->paseg = pwaypointBase->paseg;
    pwaypoint->oidSync = pwaypointBase->oidSync;
    pwaypoint->crsmgSet = pwaypointBase->crsmgSet;

    // Clone RSMG array
    for (int i = 0; i < 4; i++) {
        pwaypoint->arsmgSet[i] = pwaypointBase->arsmgSet[i];
    }

    pwaypoint->pwpsg = pwaypointBase->pwpsg;
}

void DeleteWaypoint(WAYPOINT* pwaypoint)
{
	delete pwaypoint;
}
