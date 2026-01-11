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
int  GetTzpSize();
void CloneTzp(TZP *ptzp, TZP *ptzpBase);
void PostTzpLoad(TZP* ptzp);
void UpdateTzp(TZP* ptzp, float dt);
void DeleteTzp(TZP *ptzp);

VOLZP*NewVolzp();
void InitVolzp(VOLZP *pvolzp);
int  GetVolzpSize();
void CloneVolzp(VOLZP *pvolzp, VOLZP *pvolzpBase);
void UpdateVolzp(VOLZP* pvolzp, float dt);
void RenderTzpAll(TZP *ptzp, CM *pcm, RO *pro);
void DeleteVolzp(VOLZP *pvolzp);