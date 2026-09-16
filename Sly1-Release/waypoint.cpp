#include "waypoint.h"
#include "button.h"
#include "stepguard.h"

WAYPOINT* NewWaypoint()
{
	return new WAYPOINT{};
}

void InitWaypoint(WAYPOINT* pwaypoint)
{
	InitAlo(pwaypoint);

    pwaypoint->oidSync = OID_Nil;
    pwaypoint->wps = WPS_Nil;
}

void SetWaypointDtPause(WAYPOINT* pwaypoint, float dtPause)
{
    pwaypoint->dtPause = dtPause;
}

void* GetWaypointDtPause(WAYPOINT* pwaypoint)
{
    return &pwaypoint->dtPause;
}

void SetWaypointOidSync(WAYPOINT* pwaypoint, OID oidSync)
{
    pwaypoint->oidSync = oidSync;
}

void* GetWaypointOidSync(WAYPOINT* pwaypoint)
{
    return &pwaypoint->oidSync;
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

void PostWaypointLoad(WAYPOINT* pwaypoint)
{
    PostAloLoad(pwaypoint);
    SnipAloObjects(pwaypoint, 1, s_asnipPostWaypointLoad);
    SetWaypointWps(pwaypoint, WPS_Passive);
    pwaypoint->pvtlo->pfnRemoveLo(pwaypoint);
}

void SetWaypointRsmg(WAYPOINT* pwaypoint, int fOnTrigger, OID oidRoot, OID oidSm, OID oidGoal)
{
    FAddRsmg(pwaypoint->arsmgSet, 4, &pwaypoint->crsmgSet, fOnTrigger, oidRoot, oidSm, oidGoal);
}

void SetWaypointWps(WAYPOINT* pwaypoint, WPS wps)
{
    if (pwaypoint->wps == wps)
        return;

    const WPS wpsFrom = pwaypoint->wps;

    if (wps == WPS_Active)
        TriggerRsmg(pwaypoint->psw, pwaypoint->crsmgSet, pwaypoint->arsmgSet, pwaypoint, 1);
    else if (wps == WPS_Passive && wpsFrom == WPS_Active)
        TriggerRsmg(pwaypoint->psw, pwaypoint->crsmgSet, pwaypoint->arsmgSet, pwaypoint, 0);

    pwaypoint->wps = wps;
    pwaypoint->tWps = g_clock.t;

    MSGWPT msgwpt{};
    msgwpt.pwaypoint = pwaypoint;
    msgwpt.wpsFrom = wpsFrom;
    msgwpt.wpsTo = wps;

    pwaypoint->pvtlo->pfnSendLoMessage(pwaypoint, static_cast<MSGID>(22), &msgwpt);
}

void HandleWaypointMessage(WAYPOINT* pwaypoint, MSGID msgid, void* pv)
{
    HandleAloMessage((ALO*)pwaypoint, msgid, pv);

    if (msgid == MSGID_asega_limit &&
        *reinterpret_cast<ASEG**>(static_cast<unsigned char*>(pv) + 8) == pwaypoint->paseg)
    {
        SetWaypointWps(pwaypoint, WPS_Passive);
    }
}

std::shared_ptr<WPSG> PwpsgNew()
{
    auto pwpsg = std::make_shared<WPSG>();
    pwpsg->pvtwpsg = &g_vtwpsg;
    return pwpsg;
}

void AddWpsgWaypoint(WPSG* pwpsg, WAYPOINT* pwaypoint)
{
    if (pwpsg->cpwaypoint < 8)
    {
        pwpsg->apwaypoint[pwpsg->cpwaypoint] = pwaypoint;
        pwpsg->cpwaypoint++;
    }
}

void RemoveWpsgWaypoint(WPSG* pwpsg, WAYPOINT* pwaypoint)
{
    int ipwaypoint = -1;

    for (int i = 0; i < pwpsg->cpwaypoint; ++i)
    {
        if (pwpsg->apwaypoint[i] == pwaypoint)
        {
            ipwaypoint = i;
            break;
        }
    }

    if (ipwaypoint < 0)
        return;

    const int cpwaypointMove = pwpsg->cpwaypoint - ipwaypoint - 1;

    if (cpwaypointMove > 0)
        std::memmove(&pwpsg->apwaypoint[ipwaypoint], &pwpsg->apwaypoint[ipwaypoint + 1], cpwaypointMove * sizeof(WAYPOINT*));

    --pwpsg->cpwaypoint;
    pwpsg->apwaypoint[pwpsg->cpwaypoint] = nullptr;
}

void UpdateWpsgCallback(WPSG* pwpsg, MSGID msgid, void* pv)
{
    pwpsg->fCallback = 0;

    if (pwpsg->psgg != nullptr && pwpsg->psgg->oidSync != OID_Nil && pwpsg->cpwaypoint > 0)
    {
        const OID oidSync = (OID)pwpsg->psgg->oidSync;

        if (pwpsg->apwaypoint[0]->oidSync == oidSync)
        {
            for (int i = 0; i < pwpsg->cpwaypoint; ++i)
            {
                if (pwpsg->apwaypoint[i]->wps != WPS_Sync)
                    return;
            }

            for (int i = 0; i < pwpsg->cpwaypoint; ++i)
                SetWaypointWps(pwpsg->apwaypoint[i], WPS_Active);

            pwpsg->psgg->oidSync = OID_Nil;
            return;
        }
    }

    for (int i = 0; i < pwpsg->cpwaypoint; ++i)
    {
        WAYPOINT* pwaypoint = pwpsg->apwaypoint[i];

        if (pwaypoint->wps == WPS_Active && pwaypoint->paseg == nullptr && pwaypoint->dtPause < g_clock.t - pwaypoint->tWps)
            SetWaypointWps(pwaypoint, WPS_Passive);
    }
}

void EnsureWpsgCallback(WPSG* pwpsg)
{
    if (pwpsg == nullptr)
        return;

    if (pwpsg->fCallback == 0) 
    {
        pwpsg->fCallback = 1;
        PostSwCallback(g_psw, (PFNMQ)UpdateWpsgCallback, pwpsg, MSGID_Nil, nullptr);
    }
}

void DeleteWaypoint(WAYPOINT* pwaypoint)
{
	delete pwaypoint;
}

VTWPSG g_vtwpsg;

SNIP s_asnipPostWaypointLoad[1] =
{
    2, (OID)0x2EB, offsetof(WAYPOINT, paseg)
};
