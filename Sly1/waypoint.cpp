#include "waypoint.h"

void InitWaypoint(WAYPOINT* pwaypoint)
{
	std::cout << "WAYPOINT Size: " << sizeof(WAYPOINT) << "\n";
	InitAlo(pwaypoint);
}

void DeleteWaypoint(LO* plo)
{
	delete(WAYPOINT*)plo;
}
