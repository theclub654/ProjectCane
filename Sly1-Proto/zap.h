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
void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro);
void DrawZap(RPL *prpl);
void DeleteTzp(TZP *ptzp);

VOLZP*NewVolzp();
void InitVolzp(VOLZP *pvolzp);
int  GetVolzpSize();
void CloneVolzp(VOLZP *pvolzp, VOLZP *pvolzpBase);
void DeleteVolzp(VOLZP *pvolzp);

inline bool g_fShowZapGeometry;