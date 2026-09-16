#include "blend.h"
#include "acp.h"
#include "acr.h"
#include "acpbl.h"
#include "acrbl.h"
#include "acgblt.h"
#include "acgblp.h"

void VerifyAeaEquivalence(int cea0, const EA* aea0, int cea1, const EA* aea1)
{
    /*assert(cea0 == cea1);

    for (int iea = 0; iea < cea0; ++iea)
    {
        assert(aea0[iea].eatyp == aea1[iea].eatyp);
        assert(aea0[iea].tLocal == aea1[iea].tLocal);
    }*/
}

bool CopyBlendBytes(std::vector<std::byte>& destination, const void* source, int cbBl, int cbl)
{
    if (source == nullptr || cbBl <= 0 || cbl <= 0)
    {
        destination.clear();
        return false;
    }

    const size_t recordSize = static_cast<size_t>(cbBl);
    const size_t recordCount = static_cast<size_t>(cbl);

    if (recordCount > std::numeric_limits<size_t>::max() / recordSize)
    {
        destination.clear();
        return false;
    }

    const size_t byteCount = recordSize * recordCount;
    const std::byte* first = static_cast<const std::byte*>(source);

    destination.assign(first, first + byteCount);
    return true;
}

void EnsureAsegBlendDynamic(ALO* palo, int cbBl, int cbl, void* abl, int cmrsgc, const int* mpimrsgcib, const int* mpimrsgccmrsg, ASEGBL** ppasegbl)
{
    if (ppasegbl == nullptr)
        return;

    *ppasegbl = nullptr;

    if (palo == nullptr || abl == nullptr || cbl <= 0 || cbBl < static_cast<int>(sizeof(BL)))
        return;

    if (cmrsgc < 0)
        return;

    if (cmrsgc > 0 && (mpimrsgcib == nullptr || mpimrsgccmrsg == nullptr))
        return;

    auto GetBlend = [abl, cbBl, cbl](int ibl) -> BL*
    {
        if (ibl < 0 || ibl >= cbl)
            return nullptr;

        std::byte* record = static_cast<std::byte*>(abl) + static_cast<std::size_t>(ibl) * static_cast<std::size_t>(cbBl);
        return reinterpret_cast<BL*>(record);
    };

    auto IsKnownObject = [](const LO* plo) -> bool
    {
        if (plo == nullptr)
            return false;

        return std::find(allWorldObjs.begin(), allWorldObjs.end(), plo) != allWorldObjs.end();
    };

    auto IsValidAseg = [&IsKnownObject](ASEG* paseg) -> bool
    {
        if (!IsKnownObject(static_cast<LO*>(paseg)))
            return false;

        if (paseg->pvtlo == nullptr)
            return false;

        return paseg->pvtlo->cid == CID_ASEG || paseg->pvtlo->cid == CID_ASEGBL;
    };

    ASEG* pasegFirst = nullptr;

    for (int ibl = 0; ibl < cbl; ++ibl)
    {
        BL* pbl = GetBlend(ibl);

        if (pbl == nullptr || pbl->paseg == nullptr)
            continue;

        if (!IsValidAseg(pbl->paseg))
        {
            assert(false && "EnsureAsegBlendDynamic received an invalid ASEG pointer");
            continue;
        }

        pasegFirst = pbl->paseg;
        break;
    }

    if (pasegFirst == nullptr)
        return;

    ASEGBL* pasegbl = static_cast<ASEGBL*>(PloNew(CID_ASEGBL, palo->psw, palo, OID_Nil, -1));

    if (pasegbl == nullptr)
        return;

    pasegbl->pvtlo->pfnRemoveLo(pasegbl);

    pasegbl->cbBl = cbBl;
    pasegbl->cbl = cbl;

    const std::size_t blendByteCount = static_cast<std::size_t>(cbBl) * static_cast<std::size_t>(cbl);
    pasegbl->abl.resize(blendByteCount);
    std::memcpy(pasegbl->abl.data(), abl, blendByteCount);

    pasegbl->cmrsgc = cmrsgc;
    pasegbl->amrsgc.resize(static_cast<std::size_t>(cmrsgc));

    for (int imrsgc = 0; imrsgc < cmrsgc; ++imrsgc)
    {
        const int markerCount = mpimrsgccmrsg[imrsgc];

        if (markerCount < 0)
        {
            assert(false && "Negative MRSG count");
            return;
        }

        MRSGC& mrsgc = pasegbl->amrsgc[imrsgc];
        mrsgc.ibMrsg = mpimrsgcib[imrsgc];
        mrsgc.cmrsg = markerCount;
        mrsgc.amrsg.resize(static_cast<std::size_t>(markerCount));
    }

    MRSGC* pmrsgc = pasegbl->amrsgc.empty() ? nullptr : pasegbl->amrsgc.data();
    std::vector<CHN> blendChannels;

    for (int ibl = 0; ibl < cbl; ++ibl)
    {
        BL* pbl = GetBlend(ibl);

        if (pbl == nullptr || pbl->paseg == nullptr)
            continue;

        ASEG* paseg = pbl->paseg;

        if (!IsValidAseg(paseg))
        {
            assert(false && "Invalid ASEG pointer in blend array");
            continue;
        }

        // achn may contain the stripped locomotion/root channel at index cchn.
        // Retail only blends the normal [0, cchn) channels here; pchnStrip is
        // consumed separately by the step/run code to produce vTarget.  Folding
        // it back into ASEGBL makes the root ACP drive Sly toward animation-local
        // coordinates as though they were a world-space position goal.
        const int sourceChannelCount = std::clamp(
            paseg->cchn,
            0,
            static_cast<int>(paseg->achn.size()));

        for (int ichn = 0; ichn < sourceChannelCount; ++ichn)
        {
            const CHN& sourceChannel = paseg->achn[ichn];

            if (sourceChannel.pacp == nullptr && sourceChannel.pacr == nullptr && sourceChannel.pacgTwist == nullptr && sourceChannel.apacgPose.empty())
                continue;

            auto it = std::find_if(blendChannels.begin(), blendChannels.end(), [&sourceChannel](const CHN& channel)
                {
                    return channel.oid == sourceChannel.oid;
                });

            if (it == blendChannels.end())
            {
                blendChannels.emplace_back();
                blendChannels.back().oid = sourceChannel.oid;
                it = std::prev(blendChannels.end());
            }

            CHN& blendChannel = *it;

            if (sourceChannel.pacp != nullptr)
            {
                if (blendChannel.pacp == nullptr)
                {
                    blendChannel.pacp = PacpNew(ACVK_Blend);

                    ACPBL* pacpbl = static_cast<ACPBL*>(blendChannel.pacp.get());
                    pacpbl->pasegbl = pasegbl;
                    pacpbl->pmrsgc = pmrsgc;
                    pacpbl->mpiblpacp.resize(static_cast<std::size_t>(cbl), nullptr);
                    pacpbl->fContiguous = 1;
                }

                ACPBL* pacpbl = static_cast<ACPBL*>(blendChannel.pacp.get());

                if (sourceChannel.pacp->fContiguous == 0)
                    pacpbl->fContiguous = 0;

                pacpbl->mpiblpacp[ibl] = sourceChannel.pacp.get();
            }

            if (sourceChannel.pacr != nullptr)
            {
                if (blendChannel.pacr == nullptr)
                {
                    blendChannel.pacr = PacrNew(ACVK_Blend);

                    ACRBL* pacrbl = static_cast<ACRBL*>(blendChannel.pacr.get());
                    pacrbl->pasegbl = pasegbl;
                    pacrbl->pmrsgc = pmrsgc;
                    pacrbl->mpiblpacr.resize(static_cast<std::size_t>(cbl), nullptr);
                    pacrbl->fContiguous = 1;
                    pacrbl->fSlerp = 0;
                }

                ACRBL* pacrbl = static_cast<ACRBL*>(blendChannel.pacr.get());

                if (sourceChannel.pacr->fContiguous == 0)
                    pacrbl->fContiguous = 0;

                pacrbl->mpiblpacr[ibl] = sourceChannel.pacr.get();
            }

            if (sourceChannel.pacgTwist != nullptr)
            {
                if (blendChannel.pacgTwist == nullptr)
                {
                    blendChannel.pacgTwist = PacgNew(ACGK_BlendTwist);

                    ACGBLT* pacgblt = static_cast<ACGBLT*>(blendChannel.pacgTwist.get());
                    pacgblt->pasegbl = pasegbl;
                    pacgblt->pmrsgc = pmrsgc;
                    pacgblt->mpiblpacg.resize(static_cast<std::size_t>(cbl), nullptr);
                }

                ACGBLT* pacgblt = static_cast<ACGBLT*>(blendChannel.pacgTwist.get());
                pacgblt->mpiblpacg[ibl] = sourceChannel.pacgTwist.get();
            }

            if (!sourceChannel.apacgPose.empty())
            {
                if (blendChannel.apacgPose.size() < sourceChannel.apacgPose.size())
                    blendChannel.apacgPose.resize(sourceChannel.apacgPose.size());

                blendChannel.cpacgPose = static_cast<int>(blendChannel.apacgPose.size());

                for (std::size_t ipose = 0; ipose < sourceChannel.apacgPose.size(); ++ipose)
                {
                    if (sourceChannel.apacgPose[ipose] == nullptr)
                        continue;

                    if (blendChannel.apacgPose[ipose] == nullptr)
                    {
                        blendChannel.apacgPose[ipose] = PacgNew(ACGK_BlendPose);

                        ACGBLP* pacgblp = static_cast<ACGBLP*>(blendChannel.apacgPose[ipose].get());
                        pacgblp->pasegbl = pasegbl;
                        pacgblp->pmrsgc = pmrsgc;
                        pacgblp->mpiblpacg.resize(static_cast<std::size_t>(cbl), nullptr);
                        pacgblp->ipose = static_cast<int>(ipose);
                    }

                    ACGBLP* pacgblp = static_cast<ACGBLP*>(blendChannel.apacgPose[ipose].get());
                    pacgblp->mpiblpacg[ibl] = sourceChannel.apacgPose[ipose].get();
                }
            }
        }
    }

    pasegbl->achn = std::move(blendChannels);
    pasegbl->cchn = static_cast<int>(pasegbl->achn.size());

    pasegbl->segrpt = pasegFirst->segrpt;
    pasegbl->nPriority = pasegFirst->nPriority;
    pasegbl->svtMaster = pasegFirst->svtMaster;

    pasegbl->aeaApply = pasegFirst->aeaApply;
    pasegbl->ceaApply = static_cast<int>(pasegbl->aeaApply.size());

    pasegbl->aeaRetract = pasegFirst->aeaRetract;
    pasegbl->ceaRetract = static_cast<int>(pasegbl->aeaRetract.size());

    if (cmrsgc == 0)
    {
        pasegbl->aeaFrame = pasegFirst->aeaFrame;
        pasegbl->ceaFrame = static_cast<int>(pasegbl->aeaFrame.size());

        for (int ibl = 0; ibl < cbl; ++ibl)
        {
            BL* pbl = GetBlend(ibl);

            if (pbl == nullptr || pbl->paseg == nullptr || !IsValidAseg(pbl->paseg))
                continue;

            ASEG* paseg = pbl->paseg;
            VerifyAeaEquivalence(pasegbl->ceaFrame, pasegbl->aeaFrame.data(), paseg->ceaFrame, paseg->aeaFrame.data());
        }
    }

    *ppasegbl = pasegbl;
}

void FillBlAmrsg(int cbBl, int cbl, void* abl, int ibMrsg, int coid, const OID* aoid)
{
    if (coid <= 0)
        return;

    std::byte* blendData = static_cast<std::byte*>(abl);

    for (int ibl = 0; ibl < cbl; ++ibl)
    {
        std::byte* record =
            blendData + static_cast<std::size_t>(ibl) * cbBl;

        BL* pbl = reinterpret_cast<BL*>(record);

        if (pbl->paseg == nullptr)
            continue;

        MRSG* amrsg = reinterpret_cast<MRSG*>(record + ibMrsg);

        // Replace the original TFindAsegLabel loop with this.
        for (int imrsg = 0; imrsg < coid; ++imrsg)
        {
            const OID oidLabel = aoid[imrsg];
            EA* peaLabel = PeaFindAsegLabel(pbl->paseg, oidLabel);

            if (peaLabel != nullptr)
            {
                amrsg[imrsg].t = peaLabel->tLocal;
            }
            else
            {
                amrsg[imrsg].t = 0.0f;
            }
        }

        // Keep this duration calculation after the diagnostic loop.
        for (int imrsg = 0; imrsg < coid; ++imrsg)
        {
            const int imrsgNext = (imrsg + 1) % coid;

            float dt =
                amrsg[imrsgNext].t -
                amrsg[imrsg].t;

            if (dt < 0.0f)
                dt += pbl->paseg->tMax;

            amrsg[imrsg].dt = dt;
        }
    }
}

void CalculateBlendAmrsg(int cbBl, int cbl, void* abl, int ibMrsg, int cmrsg, MRSG* amrsg, float* ptMax)
{
    auto GetBlend = [abl, cbBl](int ibl) -> BL*
    {
        return reinterpret_cast<BL*>(static_cast<std::byte*>(abl) + static_cast<size_t>(ibl) * cbBl);
    };

    auto GetMarkers = [abl, cbBl, ibMrsg](int ibl) -> MRSG*
    {
        std::byte* record = static_cast<std::byte*>(abl) + static_cast<size_t>(ibl) * cbBl;
        return reinterpret_cast<MRSG*>(record + ibMrsg);
    };

    float tMax = 0.0f;

    if (cmrsg == 0)
    {
        if (ptMax == nullptr)
            return;

        for (int ibl = 0; ibl < cbl; ++ibl)
        {
            BL* pbl = GetBlend(ibl);

            if (pbl->paseg != nullptr && pbl->u != 0.0f)
                tMax += pbl->paseg->tMax * pbl->u;
        }

        *ptMax = tMax;
        return;
    }

    for (int imrsg = 0; imrsg < cmrsg; ++imrsg)
    {
        float dtBlend = 0.0f;

        for (int ibl = 0; ibl < cbl; ++ibl)
        {
            BL* pbl = GetBlend(ibl);

            if (pbl->paseg == nullptr)
                continue;

            MRSG* sourceMarkers = GetMarkers(ibl);
            dtBlend += sourceMarkers[imrsg].dt * pbl->u;
        }

        amrsg[imrsg].dt = dtBlend;
        tMax += dtBlend;
    }

    bool hasReferencePhase = false;
    float referencePhase = 0.0f;
    float blendedPhase = 0.0f;

    for (int ibl = 0; ibl < cbl; ++ibl)
    {
        BL* pbl = GetBlend(ibl);

        if (pbl->paseg == nullptr || pbl->u == 0.0f || pbl->paseg->tMax <= 0.0f)
            continue;

        MRSG* sourceMarkers = GetMarkers(ibl);
        const float phase = sourceMarkers[0].t / pbl->paseg->tMax;

        if (!hasReferencePhase)
        {
            hasReferencePhase = true;
            referencePhase = phase;
            blendedPhase = phase;
            continue;
        }

        float phaseDelta = phase - referencePhase;

        if (phaseDelta <= -0.5f)
            phaseDelta += 1.0f;
        else if (phaseDelta > 0.5f)
            phaseDelta -= 1.0f;

        blendedPhase += phaseDelta * pbl->u;
    }

    blendedPhase = GModPositive(blendedPhase, 1.0f);
    amrsg[0].t = blendedPhase * tMax;

    for (int imrsg = 1; imrsg < cmrsg; ++imrsg)
    {
        amrsg[imrsg].t = amrsg[imrsg - 1].t + amrsg[imrsg - 1].dt;

        if (amrsg[imrsg].t >= tMax)
            amrsg[imrsg].t -= tMax;
    }

    if (ptMax != nullptr)
        *ptMax = tMax;
}

int ImrsgLookUp(float t, float tMax, int cmrsg, const MRSG* amrsg, float* pdtBest)
{
    for (int imrsg = 0; imrsg < cmrsg; ++imrsg)
    {
        const MRSG& mrsg = amrsg[imrsg];

        if (mrsg.dt == 0.0f)
            continue;

        float dt = t - mrsg.t;

        if (dt < 0.0f)
            dt += tMax;

        if (dt < mrsg.dt + 0.0001f)
        {
            if (pdtBest != nullptr)
                *pdtBest = dt;

            return imrsg;
        }
    }

    return cmrsg;
}

void ReblendAsegbl(ASEGBL* pasegbl, int cbBl, int cbl, const void* abl)
{
    const float tMaxOld = pasegbl->tMax;

    for (ASEGA* pasega = pasegbl->dlAsega.pasegaFirst; pasega != nullptr; pasega = pasega->dleAseg.pasegaNext)
    {
        if (tMaxOld != 0.0f)
            pasega->tLocal /= tMaxOld;
        else
            pasega->tLocal = 0.0f;
    }

    if (!CopyBlendBytes(pasegbl->abl, abl, cbBl, cbl))
        return;

    if (pasegbl->amrsgc.empty())
        CalculateBlendAmrsg(cbBl, cbl, pasegbl->abl.data(), 0, 0, nullptr, &pasegbl->tMax);
    else
    {
        for (MRSGC& mrsgc : pasegbl->amrsgc)
            CalculateBlendAmrsg(cbBl, cbl, pasegbl->abl.data(), mrsgc.ibMrsg, static_cast<int>(mrsgc.amrsg.size()), mrsgc.amrsg.data(), &pasegbl->tMax);
    }

    for (ASEGA* pasega = pasegbl->dlAsega.pasegaFirst; pasega != nullptr; pasega = pasega->dleAseg.pasegaNext)
    {
        pasega->tLocal *= pasegbl->tMax;
        AdaptAsega(pasega);
    }
}

void SetAsegblSlerp(ASEGBL* pasegbl, ALO* palo)
{
    CHN* pchn = PchnFindAseg(pasegbl, palo);

    if (pchn == nullptr || pchn->pacr == nullptr)
        return;

    static_cast<ACRBL*>(pchn->pacr.get())->fSlerp = true;
}
