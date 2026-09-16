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

struct VTWPSG
{

};

struct MSGWPT 
{
    class WAYPOINT* pwaypoint;
    WPS wpsFrom;
    WPS wpsTo;
};

struct WPSG 
{
    struct VTWPSG* pvtwpsg;
    int cpwaypoint;
    struct WAYPOINT* apwaypoint[8];
    int fCallback;
    struct SGG* psgg;
};

class WAYPOINT : public ALO
{
	public:
    WPS wps;
    float tWps;
    float dtPause;
    struct ASEG* paseg;
    short oidSync;
    int crsmgSet;
    RSMG arsmgSet[4];
    std::shared_ptr <WPSG> pwpsg;
};

WAYPOINT*NewWaypoint();
void InitWaypoint(WAYPOINT* pwaypoint);
void SetWaypointDtPause(WAYPOINT* pwaypoint, float dtPause);
void*GetWaypointDtPause(WAYPOINT* pwaypoint);
void SetWaypointOidSync(WAYPOINT* pwaypoint, OID oidSync);
void*GetWaypointOidSync(WAYPOINT* pwaypoint);
int  GetWaypointSize();
void CloneWaypoint(WAYPOINT* pwaypoint, WAYPOINT* pwaypointBase);
void PostWaypointLoad(WAYPOINT* pwaypoint);
void SetWaypointRsmg(WAYPOINT* pwaypoint, int fOnTrigger, OID oidRoot, OID oidSm, OID oidGoal);
void SetWaypointWps(WAYPOINT* pwaypoint, WPS wps);
void HandleWaypointMessage(WAYPOINT* pwaypoint, MSGID msgid, void* pv);
std::shared_ptr <WPSG> PwpsgNew();
void AddWpsgWaypoint(WPSG* pwpsg, WAYPOINT* pwaypoint);
void RemoveWpsgWaypoint(WPSG* pwpsg, WAYPOINT* pwaypoint);
void UpdateWpsgCallback(WPSG* pwpsg, MSGID msgid, void* pv);
void EnsureWpsgCallback(WPSG* pwpsg);
void DeleteWaypoint(WAYPOINT* pwaypoint);

extern VTWPSG g_vtwpsg;
extern SNIP s_asnipPostWaypointLoad[1];