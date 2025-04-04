#pragma once
#include "so.h"

class SQUISH : public SO
{
	public:
        float elasLast;
        CLQ clqDtImpact;
        LM lmDtImpact;
        CLQ clqSquishImpact;
        LM lmSquishImpact;
        CLQ clqStretchImpact;
        LM lmStretchImpact;
        CLQ clqSquishMoving;
        LM lmSquishMoving;
        CLQ clqStretchMoving;
        LM lmStretchMoving;
        float sdvSquish;
        glm::vec3 normalSquish;
        float tSquish;
        float dtSquish;
};

SQUISH*NewSquish();
int  GetSquishSize();
void LoadSquishFromBrx(SQUISH *psquish, CBinaryInputStream *pbis);
void CloneSquish(SQUISH* psquish, SQUISH* psquishBase);
void RenderSquishSelf(SQUISH* psquish, CM* pcm, RO* pro);
void DeleteSquish(SQUISH* psquish);