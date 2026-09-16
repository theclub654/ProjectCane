#include "barrier.h"
#include "pnt.h"
#include "xform.h"
#include "bbmark.h"

BARRIER* NewBarrier()
{
	return new BARRIER{};
}

void InitBarrier(BARRIER* pbarrier)
{
	InitSo(pbarrier);
	SetSoConstraints(pbarrier, CT_Locked, nullptr, CT_Locked, nullptr);
    pbarrier->barwarp.oidWarp = OID_Nil;
}

void* GetBarrierFAllow(BARRIER* pbarrier)
{
    return &pbarrier->fAllow;
}

void SetBarrierFAllow(BARRIER* pbarrier, int fAllow)
{
    pbarrier->fAllow = fAllow;
}

void* GetBarrierOidWarp(BARRIER* pbarrier)
{
    return &pbarrier->barwarp.oidWarp;
}

void SetBarrierOidWarp(BARRIER* pbarrier, OID oidWarp)
{
    pbarrier->barwarp.oidWarp = oidWarp;
}

void CloneBarrier(BARRIER* pbarrier, BARRIER* pbarrierBase)
{
    CloneSo(pbarrier, pbarrierBase);

    pbarrier->fAllow = pbarrierBase->fAllow;
    pbarrier->coid = pbarrierBase->coid;

    for (int i = 0; i < 4; i++)
        pbarrier->aoid[i] = pbarrierBase->aoid[i];

    pbarrier->ccid = pbarrierBase->ccid;
     
    for (int i = 0; i < 4; i++)
        pbarrier->acid[i] = pbarrierBase->acid[i];

    pbarrier->barwarp = pbarrierBase->barwarp;
    pbarrier->psoWarp = pbarrierBase->psoWarp;
    pbarrier->psoLastWarped = pbarrierBase->psoLastWarped;

    if (pbarrier->paloParent == nullptr)
        RecalcSwOxfFilterForObject(pbarrier->psw, pbarrier);
}

void PostBarrierLoad(BARRIER* pbarrier)
{
    PostAloLoad(pbarrier);

    auto& barwarp = pbarrier->barwarp;

    if (barwarp.oidWarp == OID_Nil) {
        DLI dli{};

        dli.m_pdl = &pbarrier->dlChild;
        dli.m_ibDle = pbarrier->dlChild.ibDle;
        dli.m_pdliNext = s_pdliFirst;

        PNT* point = (PNT*)pbarrier->dlChild.ploFirst;
        dli.m_ppv = point != nullptr ? reinterpret_cast<void**>(reinterpret_cast<char*>(point) + dli.m_ibDle) : nullptr;

        if (point != nullptr) {
            s_pdliFirst = &dli;

            if (FIsBasicDerivedFrom(point, CID_PNT)) 
            {
                void* next = *dli.m_ppv;
                dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<char*>(next) + dli.m_ibDle);

                if (next == nullptr) 
{
                    ConvertAloPos(pbarrier, nullptr, &point->posLocal, &barwarp.pos);
                    barwarp.bwk = BWK_Pos;
                    point->pvtlo->pfnRemoveLo(point);
                }
            }
        }

        s_pdliFirst = dli.m_pdliNext;
    }
    else {
        barwarp.pwarp = (WARP*)PloFindSwNearest(pbarrier->psw, (OID)barwarp.oidWarp, pbarrier);
        barwarp.bwk = BWK_Warp;
    }
}

void UpdateBarrier(BARRIER* pbarrier, float dt)
{
    UpdateSo(pbarrier, dt);

    SO* warpedObject = pbarrier->psoWarp;

    if (warpedObject == nullptr) {
        return;
    }

    glm::vec3 zeroVelocity(0.0f);

    warpedObject->pvtalo->pfnSetAloVelocityVec(warpedObject, &zeroVelocity);
    warpedObject->pvtalo->pfnSetAloAngularVelocityVec(warpedObject, &zeroVelocity);

    switch (pbarrier->barwarp.bwk) 
    {
        case BWK_Pos:
        warpedObject->pvtalo->pfnTranslateAloToPos(warpedObject, &pbarrier->barwarp.pos);
        break;

        case BWK_Warp:
        {
            WARP* warp = pbarrier->barwarp.pwarp;
            glm::vec3 position;

            ConvertAloPos(warp->paloParent, nullptr, &warp->posLocal, &position);
            warpedObject->pvtalo->pfnTranslateAloToPos(warpedObject, &position);
            break;
        }

        default:
        break;
    }

    pbarrier->psoWarp = nullptr;
    pbarrier->psoLastWarped = warpedObject;
}

int FIgnoreBarrierIntersection(BARRIER* pbarrier, SO* psoOther)
{
    int fMatched = 0;

    if (FIgnoreSoIntersection(pbarrier, psoOther)) {
        return 1;
    }

    for (int i = 0; i < pbarrier->coid; i++) {
        if (FMatchesLoName(psoOther, (OID)pbarrier->aoid[i])) {
            fMatched = 1;
            break;
        }
    }

    if (!fMatched) {
        for (int i = 0; i < pbarrier->ccid; i++) {
            if (FIsBasicDerivedFrom(psoOther, pbarrier->acid[i])) {
                fMatched = 1;
                break;
            }
        }
    }

    if (!pbarrier->fAllow) {
        fMatched = !fMatched;
    }

    return fMatched;
}


void NotifyBarrierImpact(BARRIER* pbarrier, XP* pxp, int ixpd)
{
    HandleLoSpliceEvent((LO*)pbarrier, 2, 0, nullptr);

    if (pbarrier->barwarp.bwk == BWK_None || pbarrier->psoWarp != nullptr) {
        return;
    }

    SO* lastWarped = pbarrier->psoLastWarped;

    if (lastWarped != nullptr) 
    {
        glm::vec3 warpPosition;

        switch (pbarrier->barwarp.bwk) 
        {
            case BWK_Pos:
            warpPosition = pbarrier->barwarp.pos;
            break;

            case BWK_Warp:
            {
                WARP* warp = pbarrier->barwarp.pwarp;
                ConvertAloPos(warp->paloParent, nullptr, &warp->posLocal, &warpPosition);
                break;
            }

            default:
            return;
        }

        float distance = glm::distance(glm::vec3(lastWarped->xf.posWorld), glm::vec3(warpPosition));

        if (distance <= lastWarped->sRadiusAll * 2.0f) {
            return;
        }
    }

    pbarrier->psoWarp = (SO*)pxp->axpd[1 - ixpd].psoRoot;
}

void AddBarrierObject(BARRIER* pbarrier, OID oid)
{
    pbarrier->aoid[pbarrier->coid++] = oid;
}

void AddBarrierClass(BARRIER* pbarrier, CID cid)
{
    pbarrier->acid[pbarrier->ccid++] = cid;
}

int GetBarrierSize()
{
	return sizeof(BARRIER);
}

void DeleteBarrier(BARRIER* pbarrier)
{
	delete pbarrier;
}
