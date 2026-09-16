#pragma once
#include "alo.h"

struct TSD 
{
    glm::vec3 pos;
    glm::vec3 v;
    glm::vec3 posNext;
    float s;
    class TAIL* ptail;
};

class TAIL : public ALO
{
	public:
    class TAIL* ptailFirst;
    int ctsd;
    std::vector <TSD> atsd;
    int fReset;
    float rSpring;
    float rDampingWorld;
    float rDampingLocal;
    float radMax;
    int cIterate;
    glm::vec3 dvGravity;
    int fUnlockRot;
    glm::vec3 posTip;
};

TAIL*NewTail();
void InitTail(TAIL* ptail);
void SetTailRSpring(TAIL* ptail, float rSpring);
void*GetTailRSpring(TAIL* ptail);
void SetTailRDampingWorld(TAIL* ptail, float rDampingWorld);
void*GetTailRDampingWorld(TAIL* ptail);
void SetTailRDampingLocal(TAIL* ptail, float rDampingLocal);
void*GetTailRDampingLocal(TAIL* ptail);
void SetTailRadMax(TAIL* ptail, float radMax);
void*GetTailRadMax(TAIL* ptail);
void SetTailCIterate(TAIL* ptail, int cIterate);
void*GetTailCIterate(TAIL* ptail);
void SetTailDvGravity(TAIL* ptail, glm::vec3 dvGravity);
void*GetTailDvGravity(TAIL* ptail);
void SetTailFUnlockRot(TAIL* ptail, int fUnlockRot);
void*GetTailFUnlockRot(TAIL* ptail);
void SetTailPosTip(TAIL* ptail, glm::vec3 posTip);
void*GetTailPosTip(TAIL* ptail);
int  GetTailSize();
void CloneTail(TAIL* ptail, TAIL* ptailBase);
void PostTailLoad(TAIL* ptail);
void UpdateTailConstraints(TAIL* ptail);
void MatchTailOtherObject(TAIL* ptail, ALO* paloOther);
void DeleteTail(TAIL* ptail);

extern glm::vec3 s_posTipDefault;