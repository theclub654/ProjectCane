#pragma once

struct CHN;

#include "aseg.h"

struct ASEGLABELMSG
{
    ASEGA* pasega;
    float tLocal;
    int label;
};

class ASEGA : public BASIC
{
	public:
    struct ASEG* paseg;
    class ALO* paloRoot;
    float dtPause;
    float tLocal;
    float svtLocal;
    float svtMaster;
    int ieaCur;
    char fHandsOff;
    struct ACTSEG* pactsegError;
    struct EA* peaPosError;
    struct EA* peaRotError;
    DLE dleSw;
    DLE dleAseg;
    DL dlActseg;
    struct AMB* pambContinuous;
    MRG mrg;
    class ALO* apaloMrg[32];
    std::shared_ptr <MQ> pmqFirst;
    int nPriority;
};

ASEGA* PasegaNew(SW* psw);
void*GetAsegaPaseg(ASEGA* pasega);
void SetAsegaPaseg(ASEGA* pasega, ASEG* pasega1);
void*GetAsegaPaloRoot(ASEGA* pasega);
void SetAsegaPaloRoot(ASEGA* pasega, ALO* paloRoot);
void*GetAsegaTLocal(ASEGA* pasega);
void SetAsegaTLocal(ASEGA* pasega, float tLocal);
void*GetAsegaHandsOff(ASEGA* pasega);
void SetAsegaHandsOff(ASEGA* pasega, int fHandsOff);
void UpdateAsegaIeaCur(ASEGA* pasega);
ACTSEG* PactsegFindAsega(ASEGA* pasega, OID oid);
//GOTTA COME BACK TO THIS LATER
void HandleAsegaEvent(ASEGA* pasega, EA* pea, int* pfRetracted);
void HandleAsegaEventsFF(ASEGA* pasega, ASEG* paseg, int* pfRetract);
void HandleAsegaEvents(ASEGA* pasega, ASEG* paseg, int* pfRetracted);
void RemoveAsega(ASEGA* pasega);
void RetractAsega(ASEGA* pasega);
void RetractAsegaActseg(ASEGA* pasega, OID oid, GRFRA grfra);
float UFromEaErrorFunc(EA* pea, float s);
int  FWrapAsegaTime(ASEGA* pasega, float* ptLocal, float* psvtLocal);
void UpdateAsega(ASEGA* pasega, float dt);
void SeekAsega(ASEGA* pasega, SEEK seek, float dtLocal, float svtLocal);
void SnapAsega(ASEGA* pasega, int fForce);
void AdaptAsega(ASEGA* pasega);
void FindChnClosestPointLocal(CHN* pchn, ALO* palo, const glm::vec3* ppos, float tAsegMax, float sIgnore, float t, float* ptClosest, glm::vec3* pposClosest, glm::vec3* pvClosest);
void*GetAsegaSpeed(ASEGA* pasega);
void SetAsegaSpeed(ASEGA* pasega, float svt);
void*GetAsegaMasterSpeed(ASEGA* pasega);
void SetAsegaMasterSpeed(ASEGA* pasega, float svtMaster);
void*GetAsegaPriority(ASEGA* pasega);
void SetAsegaPriority(ASEGA* pasega, int nPriority);
void SendAsegaMessage(ASEGA* pasega, MSGID msgid, void* pv);
void SubscribeAsegaStruct(ASEGA* pasega, PFNMQ pfnmq, void* pvContext);
void SubscribeAsegaObject(ASEGA* pasega, LO* ploTarget);
int  GetAsegaSize();
void DeleteAsega(ASEGA* pasega);