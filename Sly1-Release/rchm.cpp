#include "rchm.h"
#include "target.h"
#include "jt.h"
#include "pipe.h"
#include "blend.h"

RCHM* NewRchm()
{
    return new RCHM{};
}

void InitRchm(RCHM* prchm)
{
    InitLo(prchm);
    prchm->oidHost  = OID_Nil;
    prchm->oidTouch = OID_Nil;
}

int GetRchmSize()
{
    return sizeof(RCHM);
}

void* GetRchmRchmk(RCHM* prchm)
{
    return &prchm->rchmk;
}

void SetRchmRchmk(RCHM* prchm, RCHMK rchmk)
{
    prchm->rchmk = rchmk;
}

void* GetRchmOidHost(RCHM* prchm)
{
    return &prchm->oidHost;
}

void SetRchmOidHost(RCHM* prchm, int oidHost)
{
    prchm->oidHost = (OID)oidHost;
}

void* GetRchmOidTouch(RCHM* prchm)
{
    return &prchm->oidTouch;
}

void SetRchmOidTouch(RCHM* prchm, int oidTouch)
{
    prchm->oidTouch = (OID)oidTouch;
}

void* GetRchmGrftak(RCHM* prchm)
{
    return &prchm->grftak;
}

void SetRchmGrftak(RCHM* prchm, GRFTAK grftak)
{
    prchm->grftak = grftak;
}

void LoadRchmFromBrx(RCHM *prchm, CBinaryInputStream *pbis)
{
    for (int i = 0; i < 24; i++)
        prchm->ablrch[i].oidAseg = (OID)pbis->S16Read();

    prchm->cposGrid = pbis->S16Read();

    ReadGeom(&prchm->geomLocal, pbis);
    
    prchm->mpiposrch.resize(prchm->geomLocal.cpos);

    for (int ipos = 0; ipos < prchm->geomLocal.cpos; ipos++)
    {
        RCH& rch = prchm->mpiposrch[ipos];

        rch.fEnabled = pbis->S8Read();

        float rcl;
        float io;
        float lhub;

        ConvertRchmIposToRclIoLhub(prchm, ipos, &rcl, &io, &lhub);
        BuildRchmCoefficients(prchm, rcl, io, lhub, rch.mpiblu);
    }

    prchm->ctwr = pbis->S16Read();
    prchm->atwr.resize(prchm->ctwr);

    for (int i = 0; i < prchm->ctwr; i++)
    {
        TWR& twr = prchm->atwr[i];

        for (int j = 0; j < 4; j++)
            twr.aipos[j] = pbis->S16Read();
    };

    prchm->bspcCat.cbspFull = pbis->S16Read();
    prchm->bspcCat.cbsp = prchm->bspcCat.cbspFull;
    prchm->bspcCat.absp.resize(prchm->bspcCat.cbsp);
    prchm->mpibsptwdCat.resize(prchm->bspcCat.cbsp);

    for (int i = 0; i < prchm->bspcCat.cbsp; i++)
    {
        BSP &bsp = prchm->bspcCat.absp[i];
        TWD &twd = prchm->mpibsptwdCat[i];

        s16 surfIndex = pbis->S16Read();
        s16 negIndex  = pbis->S16Read();
        s16 posIndex  = pbis->S16Read();

        bsp.psurf = &prchm->geomLocal.asurf[surfIndex];

        bsp.pbspNeg = nullptr;
        bsp.pbspPos = nullptr;
        twd.ptwrNeg = nullptr;
        twd.ptwrPos = nullptr;

        if (negIndex > 0)
            bsp.pbspNeg = &prchm->bspcCat.absp[negIndex - 1];
        else if (negIndex < 0)
            twd.ptwrNeg = &prchm->atwr[~negIndex];

        if (posIndex > 0)
            bsp.pbspPos = &prchm->bspcCat.absp[posIndex - 1];
        else if (posIndex < 0)
            twd.ptwrPos = &prchm->atwr[~posIndex];
    }

    prchm->gRadiusSquared = pbis->F32Read();

    LoadOptionsFromBrx(prchm, pbis);
}

void CloneRchm(RCHM* prchm, RCHM* prchmBase)
{
    CloneLo(prchm, prchmBase);

    prchm->oidHost = prchmBase->oidHost;
    prchm->oidTouch = prchmBase->oidTouch;
    prchm->rchmk = prchmBase->rchmk;
    prchm->cposGrid = prchmBase->cposGrid;
    prchm->grftak = prchmBase->grftak;
    prchm->fEnabled = prchmBase->fEnabled;
    prchm->rclNatural = prchmBase->rclNatural;
    prchm->ioNatural = prchmBase->ioNatural;
    prchm->lhubNatural = prchmBase->lhubNatural;
    prchm->dtContactNatural = prchmBase->dtContactNatural;
    prchm->posContactNatural = prchmBase->posContactNatural;
    prchm->fCenterSet = prchmBase->fCenterSet;
    prchm->rclCenter = prchmBase->rclCenter;
    prchm->ioCenter = prchmBase->ioCenter;
    prchm->lhubCenter = prchmBase->lhubCenter;
    prchm->posCenter = prchmBase->posCenter;
    prchm->geomLocal = prchmBase->geomLocal;
    prchm->gRadiusSquared = prchmBase->gRadiusSquared;
    prchm->dtPause = prchmBase->dtPause;

    // Shallow copy of the pointer members
    prchm->paloHost = prchmBase->paloHost;
    prchm->paloTouch = prchmBase->paloTouch;
    prchm->pasegbl = prchmBase->pasegbl;

    // Shallow copy of the std::vector members
    prchm->mpiposrch = prchmBase->mpiposrch; // Shallow copy the vector of RCH objects
    prchm->atwr = prchmBase->atwr; // Shallow copy the vector of TWR objects
    prchm->mpibsptwdCat = prchmBase->mpibsptwdCat; // Shallow copy the vector of TWD objects
    for (int i = 0; i < 24; i++)
        prchm->ablrch[i] = prchmBase->ablrch[i];

    prchm->bspcCat = prchmBase->bspcCat;
}

void PostRchmLoad(RCHM* prchm)
{
    constexpr int cbl = 24;
    const int cbBl = static_cast<int>(sizeof(BLRCH));

    PostLoLoad(prchm);

    prchm->paloHost = static_cast<ALO*>(PloFindSwNearest(prchm->psw, static_cast<OID>(prchm->oidHost), prchm));

    if (prchm->paloHost == nullptr)
        return;

    prchm->paloTouch = static_cast<ALO*>(PloFindSwChild(prchm->psw, static_cast<OID>(prchm->oidTouch), prchm->paloHost));

    if (prchm->paloTouch == nullptr)
        return;

    for (int i = 0; i < cbl; ++i)
    {
        BLRCH& blrch = prchm->ablrch[i];

        blrch.paseg = static_cast<ASEG*>(PloFindSwChild(prchm->psw, static_cast<OID>(blrch.oidAseg), prchm->paloHost));

        if (blrch.paseg != nullptr)
            blrch.paseg->pvtlo->pfnRemoveLo(blrch.paseg);
    }

    FillBlAmrsg(cbBl, cbl, prchm->ablrch, offsetof(BLRCH, mprchsmrsg), 2, (OID*)s_mprchsoid);
    EnsureAsegBlendDynamic(prchm->paloHost, cbBl, cbl, prchm->ablrch, 1, &s_mpimrsgcibReach, &s_mpimrsgccmrsgReach, &prchm->pasegbl);

    if (prchm->pasegbl->nPriority == 0)
        prchm->pasegbl->nPriority = 1;

    ALO* paloRightHand = static_cast<ALO*>(PloFindSwChild(prchm->psw, static_cast<OID>(54), prchm->paloHost));

    if (paloRightHand != nullptr)
        SetAsegblSlerp(prchm->pasegbl, paloRightHand);

    std::array<float, cbl> ablrchu{};

    BuildRchmCoefficients(prchm, prchm->rclNatural, prchm->ioNatural, prchm->lhubNatural, ablrchu.data());

    for (int i = 0; i < cbl; ++i)
        prchm->ablrch[i].u = ablrchu[i];

    ReblendAsegbl(prchm->pasegbl, cbBl, cbl, prchm->ablrch);

    const float dtContact = prchm->pasegbl->amrsgc[0].amrsg[1].t;

    PredictAsegEffect(prchm->pasegbl, dtContact, 0.0f, prchm->paloTouch, prchm->paloHost, 1, &prchm->posContactNatural, nullptr, nullptr, nullptr);

    prchm->dtContactNatural = dtContact;

    if (prchm->fCenterSet == 0)
        prchm->posCenter = prchm->posContactNatural;
    else
    {
        BuildRchmCoefficients(prchm, prchm->rclCenter, prchm->ioCenter, prchm->lhubCenter, ablrchu.data());

        for (int i = 0; i < cbl; ++i)
            prchm->ablrch[i].u = ablrchu[i];

        ReblendAsegbl(prchm->pasegbl, cbBl, cbl, prchm->ablrch);
        PredictAsegEffect(prchm->pasegbl, dtContact, 0.0f, prchm->paloTouch, prchm->paloHost, 1, &prchm->posCenter, nullptr, nullptr, nullptr);
    }

    prchm->fEnabled = 1;
}
void ReblendRchm(RCHM* prchm, TWR* ptwr, const glm::vec3* ppos)
{
    float au[4]{};

    // Compute the barycentric weight for each tetrahedron vertex.
    for (int i = 0; i < 4; ++i)
    {
        const int i0 = ptwr->aipos[(i + 1) % 4];
        const int i1 = ptwr->aipos[(i + 2) % 4];
        const int i2 = ptwr->aipos[(i + 3) % 4];
        const int iVertex = ptwr->aipos[i];

        const glm::vec3& pos0 = prchm->geomLocal.apos[i0];
        const glm::vec3& pos1 = prchm->geomLocal.apos[i1];
        const glm::vec3& pos2 = prchm->geomLocal.apos[i2];
        const glm::vec3& posVertex = prchm->geomLocal.apos[iVertex];

        const glm::vec3 edge01 = pos1 - pos0;
        const glm::vec3 edge02 = pos2 - pos0;
        const glm::vec3 normal = glm::normalize(glm::cross(edge01, edge02));

        const float planeDot = glm::dot(pos0, normal);
        const float vertexDot = glm::dot(posVertex, normal);
        const float pointDot = glm::dot(*ppos, normal);

        au[i] = (pointDot - planeDot) / (vertexDot - planeDot);
    }

    // Clear the 24 output blend channels.
    for (int iblrch = 0; iblrch < 24; ++iblrch)
        prchm->ablrch[iblrch].u = 0.0f;

    // Blend the 24-channel values belonging to each tetrahedron vertex.
    for (int i = 0; i < 4; ++i)
    {
        const int ipos = ptwr->aipos[i];
        const float* ablu = prchm->mpiposrch[ipos].mpiblu;

        for (int iblrch = 0; iblrch < 24; ++iblrch)
            prchm->ablrch[iblrch].u += ablu[iblrch] * au[i];
    }

    ReblendAsegbl(
        prchm->pasegbl,
        static_cast<int>(sizeof(BLRCH)),
        static_cast<int>(std::size(prchm->ablrch)),
        prchm->ablrch);
}

void BuildRchmCoefficients(RCHM* prchm, float rcl, float io, float lhub, float* mpiblu)
{
    float invIo = 1.0f - io;

    float rclNeg;
    float rclMid;
    float rclPos;

    if (rcl < 0.0f)
    {
        rclNeg = -rcl;
        rclMid = rcl + 1.0f;
        rclPos = 0.0f;
    }
    else
    {
        rclNeg = 0.0f;
        rclMid = 1.0f - rcl;
        rclPos = rcl;
    }

    float lhub0;
    float lhub1;
    float lhub2;
    float lhub3;

    if (lhub < 1.0f)
    {
        lhub0 = 1.0f - lhub;
        lhub1 = lhub;
        lhub2 = 0.0f;
        lhub3 = 0.0f;
    }
    else if (lhub < 2.0f)
    {
        lhub0 = 0.0f;
        lhub1 = 2.0f - lhub;
        lhub2 = 1.0f - lhub1;
        lhub3 = 0.0f;
    }
    else
    {
        lhub0 = 0.0f;
        lhub1 = 0.0f;
        lhub2 = 3.0f - lhub;
        lhub3 = 1.0f - lhub2;
    }

    float b00 = lhub0 * invIo;
    float b01 = lhub1 * invIo;
    float b02 = lhub2 * invIo;
    float b03 = lhub3 * invIo;

    float b10 = lhub0 * io;
    float b11 = lhub1 * io;
    float b12 = lhub2 * io;
    float b13 = lhub3 * io;

    mpiblu[0] = b00 * rclNeg;
    mpiblu[1] = b01 * rclNeg;
    mpiblu[2] = b02 * rclNeg;
    mpiblu[3] = b03 * rclNeg;
    mpiblu[4] = b10 * rclNeg;
    mpiblu[5] = b11 * rclNeg;
    mpiblu[6] = b12 * rclNeg;
    mpiblu[7] = b13 * rclNeg;

    mpiblu[8] = b00 * rclMid;
    mpiblu[9] = b01 * rclMid;
    mpiblu[10] = b02 * rclMid;
    mpiblu[11] = b03 * rclMid;
    mpiblu[12] = b10 * rclMid;
    mpiblu[13] = b11 * rclMid;
    mpiblu[14] = b12 * rclMid;
    mpiblu[15] = b13 * rclMid;

    mpiblu[16] = b00 * rclPos;
    mpiblu[17] = b01 * rclPos;
    mpiblu[18] = b02 * rclPos;
    mpiblu[19] = b03 * rclPos;
    mpiblu[20] = b10 * rclPos;
    mpiblu[21] = b11 * rclPos;
    mpiblu[22] = b12 * rclPos;
    mpiblu[23] = b13 * rclPos;
}

void ConvertRchmIposToRclIoLhub(RCHM* prchm, int ipos, float* prcl, float* pio, float* plhub)
{
    int cposGrid = prchm->cposGrid;

    int clhub = cposGrid * 3 + 1;
    int cio = cposGrid + 1;

    int ioStride = clhub;
    int rclStride = cio * clhub;

    int ircl = ipos / rclStride;

    int remaining = ipos - ircl * rclStride;

    int iio = remaining / ioStride;
    int ilhub = remaining - iio * ioStride;

    *prcl = (float)(ircl - cposGrid) / (float)cposGrid;
    *pio = (float)iio / (float)cposGrid;
    *plhub = (float)ilhub / (float)cposGrid;
}

void SetRchmNaturalCoefficients(RCHM* prchm, float rcl, float io, float lhub)
{
    prchm->lhubNatural = lhub;
    prchm->rclNatural = rcl;
    prchm->ioNatural = io;
}

void SetRchmCenterCoefficients(RCHM* prchm, float rcl, float io, float lhub)
{
    prchm->lhubNatural = lhub;
    prchm->fCenterSet = 1;
    prchm->rclNatural = rcl;
    prchm->ioNatural = io;
}

void PredictRchmTargetPos(RCHM* prchm, TARGET* ptarget, float dt, glm::vec3* ppos)
{
    glm::vec3 posParent{};
    glm::mat3 matParent{ 1.0f };

    PredictAloTransform(ptarget->paloParent, nullptr, dt, &posParent, &matParent, nullptr, nullptr);

    *ppos = posParent + matParent * ptarget->posLocal;
}

void PredictRchmTargetLocalPos(RCHM* prchm, TARGET* ptarget, float dt, glm::vec3* pposLocal)
{
    glm::vec3 posTarget{};
    glm::vec3 posHost{};
    glm::mat3 matHost{ 1.0f };

    PredictRchmTargetPos(prchm, ptarget, dt, &posTarget);
    PredictAloTransform(prchm->paloHost, nullptr, dt, &posHost, &matHost, nullptr, nullptr);

    *pposLocal = glm::transpose(matHost) * (posTarget - posHost);
}

TWR* PtwrMapRchmSafe(RCHM* prchm, BSP* pbsp, const glm::vec3* ppos)
{
    constexpr float kPlaneEpsilon = 0.0001f;

    BSP* const absp = prchm->bspcCat.absp.data();

    if (pbsp == nullptr)
        pbsp = absp;

    while (pbsp != nullptr)
    {
        BSP* const pbspCurrent = pbsp;
        const float distance = glm::dot(*ppos, pbspCurrent->psurf->normal) - pbspCurrent->psurf->gDot;
        const std::size_t ibsp = static_cast<std::size_t>(pbspCurrent - absp);

        if (distance < -kPlaneEpsilon)
        {
            if (pbspCurrent->pbspNeg == nullptr)
                return prchm->mpibsptwdCat[ibsp].ptwrNeg;

            pbsp = pbspCurrent->pbspNeg;
            continue;
        }

        if (distance > kPlaneEpsilon)
        {
            if (pbspCurrent->pbspPos == nullptr)
                return prchm->mpibsptwdCat[ibsp].ptwrPos;

            pbsp = pbspCurrent->pbspPos;
            continue;
        }

        // The point is approximately on the partition plane.
        TWR* ptwr = nullptr;

        if (pbspCurrent->pbspPos == nullptr)
            ptwr = prchm->mpibsptwdCat[ibsp].ptwrPos;
        else
            ptwr = PtwrMapRchmSafe(prchm, pbspCurrent->pbspPos, ppos);

        if (ptwr != nullptr)
            return ptwr;

        if (pbspCurrent->pbspNeg == nullptr)
            return prchm->mpibsptwdCat[ibsp].ptwrNeg;

        pbsp = pbspCurrent->pbspNeg;
    }

    return nullptr;
}

void FindRchmClosestPoint(RCHM* prchm, const glm::vec3* ppos, glm::vec3* pposClosest, TWR** pptwr, float* ps)
{
    TWR* ptwr = PtwrMapRchmSafe(prchm, nullptr, ppos);

    // The requested position is already inside the reach map.
    if (ptwr != nullptr)
    {
        if (pposClosest != nullptr)
            *pposClosest = *ppos;

        if (pptwr != nullptr)
            *pptwr = ptwr;

        if (ps != nullptr)
            *ps = 0.0f;

        return;
    }

    glm::vec3 posBest{};
    TWR* ptwrBest = nullptr;

    float uValid = 0.0f;
    float uInvalid = 1.0f;

    // Search from the known interior center toward the invalid point.
    for (int i = 0; i < 10; ++i)
    {
        const float u = (uValid + uInvalid) * 0.5f;
        const glm::vec3 pos = glm::mix(prchm->posCenter, *ppos, u);
        TWR* const ptwrCandidate = PtwrMapRchmSafe(prchm, nullptr, &pos);

        if (ptwrCandidate != nullptr)
        {
            uValid = u;
            posBest = pos;
            ptwrBest = ptwrCandidate;
        }
        else
        {
            uInvalid = u;
        }
    }

    // If the binary search never found a valid point, use the natural
    // contact position as the fallback.
    if (ptwrBest == nullptr)
    {
        posBest = prchm->posContactNatural;
        ptwrBest = PtwrMapRchmSafe(prchm, nullptr, &posBest);
    }

    if (pposClosest != nullptr)
        *pposClosest = posBest;

    if (pptwr != nullptr)
        *pptwr = ptwrBest;

    if (ps != nullptr)
        *ps = glm::distance(*ppos, posBest);
}

void TrackJtTarget(JT* pjt, RCHM* prchm, TARGET* ptarget)
{
    if (ptarget == nullptr)
        return;

    const float dt = glm::max(pjt->tContact - g_clock.t, g_clock.dt);

    glm::vec3 posLocal{};
    glm::vec3 posClosest{};
    TWR* ptwr = nullptr;
    float distance = 0.0f;

    PredictRchmTargetLocalPos(prchm, ptarget, dt, &posLocal);
    FindRchmClosestPoint(prchm, &posLocal, &posClosest, &ptwr, &distance);

    if (ptwr != nullptr)
        ReblendRchm(prchm, ptwr, &posClosest);
}

void TrackJtPipe(JT* pjt, RCHM* prchm, PIPE* ppipe, float* psPipe)
{
    glm::vec3 posJtPipe{};
    glm::vec3 posClosestPipe{};
    glm::vec3 posLocal{};
    glm::vec3 posClosest{};
    TWR* ptwr = nullptr;
    float distance = 0.0f;

    // Convert the joint's world position into the pipe parent's local space.
    ConvertAloPos(nullptr, ppipe->paloParent, &pjt->xf.posWorld, &posJtPipe);

    CRV* const pcrv = ppipe->pcrv.get();

    if (pcrv != nullptr && pcrv->pvtcrv->pfnFindCrvClosestPointFromS != nullptr)
        pcrv->pvtcrv->pfnFindCrvClosestPointFromS(pcrv, &posJtPipe, *psPipe, nullptr, &posClosestPipe, nullptr, nullptr, psPipe);

    // Convert the point on the pipe from pipe-parent space into joint space.
    ConvertAloPos(ppipe->paloParent, static_cast<ALO*>(pjt), &posClosestPipe, &posLocal);

    FindRchmClosestPoint(prchm, &posLocal, &posClosest, &ptwr, &distance);

    if (ptwr != nullptr)
        ReblendRchm(prchm, ptwr, &posClosest);
}

void DeleteRchm(RCHM* prchm)
{
    delete prchm;
}

OID s_mprchsoid[2] =
{
    (OID)0x130, (OID)0x132
};

int s_mpimrsgcibReach = offsetof(BLRCH, mprchsmrsg);
int s_mpimrsgccmrsgReach = 2;
