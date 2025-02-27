#pragma once
#include "alo.h"

enum WPS 
{
    WPS_Nil = -1,
    WPS_Passive = 0,
    WPS_Sync = 1,
    WPS_Active = 2,
    WPS_Max = 3
};

class WAYPOINT : public ALO
{
	public:
        WPS wps;
        float tWps;
        float dtPause;
        struct ASEG* paseg;
        OID oidSync;
        int crsmgSet;
        RSMG arsmgSet[4];
        struct WPSG* pwpsg;
};

WAYPOINT*NewWaypoint();
void InitWaypoint(WAYPOINT* pwaypoint);
int  GetWaypointSize();
void CloneWaypoint(WAYPOINT* pwaypoint, WAYPOINT* pwaypointBase);
void DeleteWaypoint(WAYPOINT* pwaypoint);