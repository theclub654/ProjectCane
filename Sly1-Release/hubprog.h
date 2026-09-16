#pragma once
#include "alo.h"

class HUBPROG : public ALO
{
	public:
	ALO* apaloProgress[9]{};
};

HUBPROG*NewHubProg();
void CloneHubProg(HUBPROG* phubprog, HUBPROG* phubprogBase);
void PostHubProgLoad(HUBPROG* phubprog);
void DeleteHubProg(HUBPROG* phubprog);

inline SNIP s_asnipHubProg[9] =
{
    { 0x02, (OID)0x51A, offsetof(HUBPROG, apaloProgress[0]) },
    { 0x02, (OID)0x51B, offsetof(HUBPROG, apaloProgress[1]) },
    { 0x02, (OID)0x51C, offsetof(HUBPROG, apaloProgress[2]) },
    { 0x02, (OID)0x51D, offsetof(HUBPROG, apaloProgress[3]) },
    { 0x02, (OID)0x51E, offsetof(HUBPROG, apaloProgress[4]) },
    { 0x02, (OID)0x51F, offsetof(HUBPROG, apaloProgress[5]) },
    { 0x02, (OID)0x520, offsetof(HUBPROG, apaloProgress[6]) },
    { 0x02, (OID)0x521, offsetof(HUBPROG, apaloProgress[7]) },
    { 0x02, (OID)0x522, offsetof(HUBPROG, apaloProgress[8]) },
};
