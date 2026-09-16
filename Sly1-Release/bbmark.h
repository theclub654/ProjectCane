#pragma once
#include "so.h"
#include "sw.h"

typedef unsigned char OXF;
typedef int GRFPVA;

void RecalcSwOxfFilterForObject(SW* psw, SO* pso);
void UpdateSwPox(SW* psw, OXA* poxa0, OXA* poxa1, OXF oxfAdd, OXF oxfRemove);
OX*  PoxAddSw(SW* psw, OXA* poxa, OXA* poxaOther);
OX*  PoxRemoveSw(SW* psw, OXA* poxa, OXA* poxaOther);
void FreeSwOx(SW* psw, OX* pox);
int  IoxaFromPoxa(SW* psw, OXA* poxa);
void UpdateSwAaox(SW* psw);
void RecalcSwXpAll(SW* psw, int fCenterXp);
void AddSwAaobrObject(SW* psw, SO* pso);
void RemoveSwAaobrObject(SW* psw, SO* pso);
void InvalidateSwAaox(SW* psw);
void InvalidateSwXpForObject(SW* psw, SO* pso, GRFPVA grfpvaInvalid);
OX* PoxFromSoSo(SO* pso, SO* psoOther);
XP* PxpFirstFromSoSo(SO* pso, SO* psoOther);
