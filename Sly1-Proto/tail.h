#pragma once
#include "alo.h"

class TAIL : public ALO
{
	public:
        struct TAIL* ptailFirst;
        int ctsd;
        struct TSD* atsd;
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
int  GetTailSize();
void CloneTail(TAIL* ptail, TAIL* ptailBase);
void DeleteTail(TAIL* ptail);