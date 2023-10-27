#pragma once
#include "sw.h"

// Merge
// Used to merge ALO object's
struct MRG
{
	DLE dle;
	// Number of ALO objects to be merged
	int cpalo;
	// The ALO object thats gonna be merged
	struct ALO** apalo;
};

void MergeSwGroup(SW *psw, MRG *pmrg);