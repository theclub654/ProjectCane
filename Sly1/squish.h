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

void LoadSquishFromBrx(SQUISH *psquish, CBinaryInputStream *pbis);