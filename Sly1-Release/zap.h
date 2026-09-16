#pragma once
#include "so.h"

struct RZPR 
{
    struct RAT* prat;
    glm::vec3 pos;
    glm::vec3 normal;
};

class TZP : public SO
{
	public:
    ZPD zpd;
    RZPR arzpr[10];
    int crzpr;
};

class VOLZP : public TZP
{
	public:

};

TZP* NewTzp();
void InitTzp(TZP *ptzp);
void*GetTzpZpk(TZP* ptzp);
void SetTzpZpk(TZP* ptzp, ZPK zpk);
void*GetTzpDzThrow(TZP* ptzp);
void SetTzpDzThrow(TZP* ptzp, float dzThrow);
void*GetTzpFThrowCut(TZP* ptzp);
int  GetTzpSize();
void CloneTzp(TZP *ptzp, TZP *ptzpBase);
void PostTzpLoad(TZP* ptzp);
void UpdateTzp(TZP* ptzp, float dt);
void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro);
int  FInflictTzpZap(TZP* ptzp, XP* pxp, ZPR* pzpr);
void*PzpdEnsureTzp(TZP* ptzp, ENSK ensk);
void ResetTzpThrowCount(TZP* ptzp);
void DeleteTzp(TZP *ptzp);

VOLZP*NewVolzp();
void InitVolzp(VOLZP *pvolzp);
int  GetVolzpSize();
void CloneVolzp(VOLZP *pvolzp, VOLZP *pvolzpBase);
void UpdateVolzp(VOLZP* pvolzp, float dt);
void DeleteVolzp(VOLZP *pvolzp);

void InitZpd(ZPD* pzpd, SO* pso);
void PostZpdLoad(ZPD* pzpd);
void AddZpdZapLo(ZPD* pzpd, LO* plo);
void ApplyZpdThrow(ZPD* pzpd, PO* ppo);
void InflictZpdZap(ZPD* pzpd, XP* pxp, ZPR* pzpr);
void AddZpdZapObject(ZPD* pzpd, OID oid);
void RemoveZpdZapLo(ZPD* pzpd, LO* plo);
