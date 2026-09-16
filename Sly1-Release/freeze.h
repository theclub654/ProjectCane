#pragma once
#include "sw.h"

void RemergeSwObject(SW* psw, ALO* palo);
void MergeSwGroup(SW *psw, MRG *pmrg);
void AddSwMergeGroup(SW* psw, MRG* pmrg);
void RemoveFromArray(int* count, void** items, void* item);
void RemoveSwMergeGroup(SW* psw, MRG* pmrg);
void RemergeSwObjects(SW* psw);
void RefreezeSwObjects(SW* psw);
void MergeSwFreezeGroups(SW* psw, ALO* freezeGroup1, ALO* freezeGroup2);
void SplinterSwFreezeGroup(SW* psw, ALO* paloFreezeRoot);
void FreezeAloHierarchy(ALO* alo, int freeze);
void UpdateSwBusyList(SW* psw);