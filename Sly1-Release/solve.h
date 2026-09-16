#pragma once
#include "so.h"

constexpr GRFSG GRFSG_None = 0;
constexpr GRFSG GRFSG_Pivot = 1 << 0;
constexpr GRFSG GRFSG_Impact = 1 << 1;
constexpr GRFSG GRFSG_Penetration = 1 << 2;

void SolveSwGroup(SW* psw, int cpsoRoot, SO** apsoRoot, int cpxp, XP** apxp, float dt, GRFSG grfsg);
int  CpxpBuildXpArray(int cpso, SO** apso, int cpxpMax, XP** apxp);
void BuildContactGroup(SO* psoSeed, int* pcpso, SO** apso, int* pcpsoLocked, SO** apsoLocked);
void SplitSoFreeze(SO* pso, int fFreeze);
void SortSwGroup(int cpso, SO** apso);
void SplitSwGroup(SW* psw, int cpso, SO** apso, float dt, GRFSG grfsg);
void SolveSw(SW* psw, float dt, GRFSG flags);