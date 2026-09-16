#pragma once
#include "pair.h"
#include "frame.h"
#include "bif.h"

using PFNBIF = CRef(*)(int, CRef*, CFrame*);

struct BIF 
{
    PFNBIF pfnbif;
    int crefReq;
    int fVarArg;
    const char* achz;
};

CRef RefEvalSymbol(CPair* ppair, CFrame* pframe);
CRef RefEval(CPair* ppair, CFrame* pframe);
CRef RefEvalSet(CPair* ppair, CFrame* pframe);
CRef RefEvalDefine(CPair* ppair, CFrame* pframe);
CRef RefEvalAssert(CPair* ppair, CFrame* pframe);
CRef RefEvalIf(CPair* ppair, CFrame* pframe);
CRef RefEvalOr(CPair* ppair, CFrame* pframe);
CRef RefEvalAnd(CPair* ppair, CFrame* pframe);
CRef RefEvalCond(CPair* ppair, CFrame* pframe);
CRef RefEvalCase(CPair* ppair, CFrame* pframe);
CRef RefEvalLet(CPair* ppair, CFrame* pframe);
CRef RefEvalWhile(CPair* ppair, CFrame* pframe);
CRef RefEvalLambda(CPair* ppair, CFrame* pframe);
CRef RefEvalBegin(CPair* ppair, CFrame* pframe);
CRef RefEvalLambdaBody(CPair* ppair, CFrame* pframe);
CRef RefEvalImport(CPair* ppair, CFrame* pframe);
CRef RefEvalApply(CPair* ppair, CFrame* pframe);
CRef RefEvalModule(SW* psw, int isplice);

extern BIF g_mpbifkbif[125];
