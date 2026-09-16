#pragma once
#include "emitter.h"

struct EXPLSTE 
{
    class EXPLS* pexpls;
    class EXPLSO explso;
    float tEmit;
    DLE dle;
};

class EXPL : public XFM
{
	public:
	class EXPLG* pexplgParent;
};

EXPL* NewExpl();
int  GetExplSize();
void CloneExpl(EXPL* pexpl, EXPL* pexplBase);
void PostExplLoad(EXPL* pexpl);
void CalculateExplTransform(EXPL* pexpl, glm::vec3* pposLocal, glm::mat3* pmatLocal);
void InferExpl(EXPL** ppexpl, ALO* palo);
void FireSwTimedExplodeStyles(SW* psw);
void FireExplsExplso(EXPLS* pexpls, EXPLSO* pexplso);
void ExplodeExpl(EXPL* pexpl);
void ExplodeExplParams(EXPL* pexpl, GRFEXPLSO grfexplso, ALO* paloReference, const glm::vec3* pvec, const glm::vec3* pposOrigin, float rScale, float sRadius);
void DeleteExpl(EXPL* pexpl);

static LM s_lmrSv = {0.5, 2.0};