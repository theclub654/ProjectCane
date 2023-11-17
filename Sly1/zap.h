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

void InitTzp(TZP* ptzp);
void InitVolzp(VOLZP* pvolzp);
void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro);