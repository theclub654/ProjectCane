#include "aseg.h"
#include "asega.h"
#include "acp.h"
#include "acr.h"
#include "acs.h"
#include "acg.h"
#include "actseg.h"
#include "acrbl.h"
#include "acrb.h"
#include "acrc.h"
#include <array>
#include <cstring>

ASEG* NewAseg()
{
    return new ASEG{};
}

void InitAseg(ASEG* paseg)
{
    InitLo(paseg);
    InitDl(&paseg->dlAsega, offsetof(ASEGA, dleAseg));

    paseg->clqMasterSuck.g0 = 1.0;
    paseg->clqMasterSuck.g2 = 0.0;
    paseg->clqMasterSuck.g1 = 0.0;
    paseg->svtMasterSuck = 1.0;
    paseg->lmMasterSuck.gMin = 0.0;
    paseg->lmMasterSuck.gMax = 1.0;
}

int GetAsegSize()
{
    return sizeof(ASEG);
}

void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis)
{
    paseg->oidRoot = (OID)pbis->S16Read();
    paseg->tMax = pbis->S16Read() * 0.016666668f;
    paseg->coidSearchRoot = pbis->U16Read();

    paseg->aoidSearchRoot.resize(paseg->coidSearchRoot);

    for (int i = 0; i < paseg->coidSearchRoot; i++)
        paseg->aoidSearchRoot[i] = (OID)pbis->S16Read();

    paseg->cchn = pbis->S8Read();
    paseg->achn.resize(paseg->cchn);

    int ichnStrip = -1;

    for (int i = 0; i < paseg->cchn; i++)
    {
        CHN& chn = paseg->achn[i];

        chn.oid = (OID)pbis->S16Read();
        byte flags = pbis->S8Read();

        if ((flags & 1) != 0)
        {
            ACVK acvk = (ACVK)pbis->S8Read();
            std::shared_ptr<ACP> pacp = PacpNew(acvk);
            chn.pacp = pacp;

            if (pacp->pvtacp->pfnLoadAcpFromBrx != nullptr)
                pacp->pvtacp->pfnLoadAcpFromBrx(pacp.get(), pbis);
        }

        if ((flags & 2) != 0)
        {
            ACVK acvk = (ACVK)pbis->S8Read();
            std::shared_ptr<ACR> pacr = PacrNew(acvk);
            chn.pacr = pacr;

            if (pacr->pvtacr->pfnLoadAcrFromBrx != nullptr)
                pacr->pvtacr->pfnLoadAcrFromBrx(pacr.get(), pbis);
        }

        if ((flags & 4) != 0)
        {
            ACVK acvk = (ACVK)pbis->S8Read();
            std::shared_ptr<ACS> pacs = PacsNew(acvk);
            chn.pacs = pacs;

            if (pacs->pvtacs->pfnLoadAcsFromBrx != nullptr)
                pacs->pvtacs->pfnLoadAcsFromBrx(pacs.get(), pbis);
        }

        if ((flags & 8) != 0)
        {
            ACGK acgk = (ACGK)pbis->S8Read();
            std::shared_ptr<ACG> pacg = PacgNew(acgk);
            chn.pacgTwist = pacg;

            if (pacg->pvtacg->pfnLoadAcgFromBrx != nullptr)
                pacg->pvtacg->pfnLoadAcgFromBrx(pacg.get(), pbis);
        }

        if ((flags & 0x10) != 0)
        {
            byte cpacgPose = pbis->U8Read();
            chn.cpacgPose = cpacgPose;
            chn.apacgPose.resize(cpacgPose);

            for (int a = 0; a < cpacgPose; a++)
            {
                ACGK acgk = (ACGK)pbis->S8Read();

                if ((int)acgk != -1)
                {
                    std::shared_ptr <ACG> pacgPose = PacgNew(acgk);
                    chn.apacgPose[a] = pacgPose;

                    if (pacgPose->pvtacg->pfnLoadAcgFromBrx != nullptr)
                        pacgPose->pvtacg->pfnLoadAcgFromBrx(pacgPose.get(), pbis);
                }
            }
        }

        if ((flags & 0x20) != 0)
            ichnStrip = i;

    }

    if (ichnStrip >= 0)
        StripAsegChn(paseg, &paseg->achn[ichnStrip]);

    LoadAsegEventsFromBrx(paseg, pbis, 0, &paseg->ceaApply,   paseg->aeaApply);
    LoadAsegEventsFromBrx(paseg, pbis, 1, &paseg->ceaFrame,   paseg->aeaFrame);
    LoadAsegEventsFromBrx(paseg, pbis, 0, &paseg->ceaRetract, paseg->aeaRetract);
    LoadOptionsFromBrx(paseg, pbis);

}

void LoadAsegEventsFromBrx(ASEG* paseg, CBinaryInputStream* pbis, int fFrame, int* pcea, std::vector<EA>& aea)
{
    const int ceaInput = pbis->S8Read();

    aea.clear();
    aea.resize(ceaInput);

    int ceaOutput = 0;

    for (int ieaInput = 0; ieaInput < ceaInput; ++ieaInput)
    {
        EA ea{};
        ea.eatyp = static_cast<EATYP>(pbis->S8Read());

        if (fFrame != 0)
            ea.tLocal = pbis->S16Read() * 0.016666668f;

        switch (ea.eatyp)
        {
            case EATYP_SoundEffect:
            {
                const SFXID sfxid = static_cast<SFXID>(pbis->S16Read());
                const OID oid = static_cast<OID>(pbis->S16Read());
                const s32 sFull = pbis->S32Read();
                const s32 sStop = pbis->S32Read();
                const float gVolume = pbis->F32Read();
                const float gPitch = pbis->F32Read();
                const float gDoppler = pbis->F32Read();

                EASND* peasndMerge = nullptr;

                for (int jea = ceaOutput - 1; jea >= 0; --jea)
                {
                    EA& eaPrevious = aea[jea];

                    if (eaPrevious.tLocal != ea.tLocal)
                        break;

                    if (eaPrevious.eatyp == EATYP_SoundEffect)
                    {
                        peasndMerge = eaPrevious.sound.peasnd.get();
                        break;
                    }
                }

                if (peasndMerge != nullptr)
                {
                    if (peasndMerge->csfxid < 6)
                        peasndMerge->asfxid[peasndMerge->csfxid++] = sfxid;

                    continue;
                }

                ea.sound.peasnd = std::make_shared<EASND>();
                ea.sound.peasnd->csfxid = 1;
                ea.sound.peasnd->asfxid[0] = sfxid;
                ea.sound.peasnd->isfxidLast = 0;
                ea.sound.peasnd->oid = oid;
                ea.sound.peasnd->sFull = sFull;
                ea.sound.peasnd->sStop = sStop;
                ea.sound.peasnd->gVolume = gVolume;
                ea.sound.peasnd->gPitch = gPitch;
                ea.sound.peasnd->gDoppler = gDoppler;
                break;
            }

            case EATYP_Seek:
            case EATYP_SeekIfMessage:
            ea.seek.tToLocal = pbis->S16Read() * 0.016666668f;
            ea.seek.svtLocal = pbis->F32Read();
            break;

            case EATYP_SetSpeed:
            ea.setspeed.svtLocal = pbis->F32Read();
            break;

            case EATYP_StartTimer:
            ea.timer.dt = pbis->F32Read();
            break;

            case EATYP_SetDark:
            case EATYP_SetDarkSmooth:
            ea.dark.rDark = pbis->F32Read();
            break;

            case EATYP_ShowDialog:
            case EATYP_EnableCamera:
            case EATYP_DisableCamera:
            case EATYP_EnableObjectDrive:
            case EATYP_DisableObjectDrive:
            case EATYP_DisableObjectPosErrorFunc:
            case EATYP_DisableObjectRotErrorFunc:
            case EATYP_EnableObjectScale:
            case EATYP_DisableObjectScale:
            case EATYP_EnableObjectMesh:
            case EATYP_DisableObjectMesh:
            case EATYP_CutObjectPos:
            case EATYP_CutObjectRot:
            case EATYP_Waypoint:
            case EATYP_StartScript:
            ea.object.oid = static_cast<OID>(pbis->S16Read());
            break;

            case EATYP_Label:
            ea.label.oidLabel = static_cast<OID>(pbis->S16Read());
            break;

            case EATYP_ShowObject:
            case EATYP_HideObject:
            ea.showhide.oid = static_cast<OID>(pbis->S16Read());
            ea.showhide.dt = pbis->F32Read();
            break;

            case EATYP_SetObjectInteract:
            ea.setinteract.oid = static_cast<OID>(pbis->S16Read());
            ea.setinteract.ctForce = pbis->S8Read();
            ea.setinteract.ctTorque = pbis->S8Read();
            break;

            case EATYP_EnableObjectPuppet:
            case EATYP_DisableObjectPuppet:
            ea.puppet.oid = static_cast<OID>(pbis->S16Read());
            ea.puppet.fSnap = pbis->U8Read();
            break;

            case EATYP_EnableObjectPosErrorFunc:
            case EATYP_EnableObjectRotErrorFunc:
            ea.errorfunc.peaef = std::make_shared<EAEF>();
            ea.errorfunc.peaef->oid = static_cast<OID>(pbis->S16Read());
            ea.errorfunc.peaef->sFull = pbis->F32Read();
            ea.errorfunc.peaef->sStop = pbis->F32Read();
            break;

            case EATYP_Wipe:
            case EATYP_WipeOnButton:
            {
                ea.wipe.wipek = pbis->S16Read();

                ea.wipe.ptrans = std::make_shared<TRANS>();
                ea.wipe.ptrans->fSet = 1;
                ea.wipe.ptrans->oidWarp = static_cast<OID>(pbis->S16Read());
                ea.wipe.ptrans->oidWarpContext = OID_Nil;
                ea.wipe.ptrans->grftrans = static_cast<GRFTRANS>(0);

                LEVELINFOBRX levelBrx{};

                pbis->file.read(reinterpret_cast<char*>(&levelBrx), sizeof(levelBrx));

                const uint32_t searchKey = levelBrx.searchValue ^ levelBrx.searchCipher;

                ea.wipe.ptrans->plevel = PlevelinfoFromSearchKey(searchKey);


                break;
            }

            case EATYP_TriggerDefaultExit:
            ea.defexit.fInhibitAnims = pbis->U8Read();
            ea.defexit.wipek = pbis->S16Read();
            break;

            case EATYP_StartAnimation:
            ea.animation.peaa = std::make_shared<EAA>();
            ea.animation.peaa->oidAseg = static_cast<OID>(pbis->S16Read());
            ea.animation.peaa->oidRoot = static_cast<OID>(pbis->S16Read());
            ea.animation.peaa->tLocal = pbis->F32Read();
            ea.animation.peaa->svtLocal = pbis->F32Read();

            break;

            case EATYP_StartRumble:
            ea.rumble.dframe = pbis->S16Read();
            ea.rumble.rumk = pbis->S16Read();
            break;

            case EATYP_SendUserMessage:
            ea.sendusermessage.oid = static_cast<OID>(pbis->S16Read());
            ea.sendusermessage.oidLabel = static_cast<OID>(pbis->S16Read());
            break;

            case EATYP_PoseCombo:
            ea.posecombo.oidTarget = static_cast<OID>(pbis->S16Read());
            ea.posecombo.oidCombo = static_cast<OID>(pbis->S16Read());
            break;

            case EATYP_RetractSm:
            ea.retractsm.oidSm = static_cast<OID>(pbis->S16Read());
            ea.retractsm.oidRoot = static_cast<OID>(pbis->S16Read());
            break;

            case EATYP_Splice:
            ea.splice.frame = pbis->U16Read();
            break;

            case EATYP_PreloadVag:
            case EATYP_ShowMovie:
            {
                std::array<uint32_t, 8> descriptor{};
                bool fDescriptorPresent = false;
                for (uint32_t& word : descriptor)
                {
                    word = pbis->U32Read();
                    fDescriptorPresent |= word != 0;
                }

                if (fDescriptorPresent)
                {
                    ea.string.pchz = std::shared_ptr<char[]>(new char[33]());
                    std::memcpy(ea.string.pchz.get(), descriptor.data(), 32);
                }
                else
                {
                    ea.string.pchz.reset();
                }

                break;
            }

            case EATYP_SetSmGoal:
            ea.setsm.oidSm = pbis->S16Read();
            ea.setsm.oidRoot = pbis->S16Read();
            ea.setsm.oidState = pbis->S16Read();
            break;

            case EATYP_PlaceCredits:
            ea.place.x = pbis->S16Read();
            ea.place.y = pbis->S16Read();
            ea.place.cline = pbis->S16Read();
            break;

            case EATYP_Explode:
            ea.explode.peaex = std::make_shared<EAEX>();
            ea.explode.peaex->oidExpl = static_cast<OID>(pbis->S16Read());
            ea.explode.peaex->oidRef = static_cast<OID>(pbis->S16Read());
            ea.explode.peaex->oidTouch = static_cast<OID>(pbis->S16Read());
            ea.explode.peaex->gScale = pbis->F32Read();
            break;

            case EATYP_SetCredit:
            ea.credit.iline = pbis->S16Read();
            ea.credit.dtVisible = pbis->S16Read();
            ea.credit.pchz = pbis->ReadStringAlloc();
            break;
        }

        aea[ceaOutput++] = std::move(ea);
    }

    aea.resize(ceaOutput);
    *pcea = ceaOutput;
}

void* GetAsegSegrpt(ASEG* paseg)
{
    return &paseg->segrpt;
}

void SetAsegSegrpt(ASEG* paseg, SEGRPT segrpt)
{
    paseg->segrpt = segrpt;
}

void* GetAsegfDefault(ASEG* paseg)
{
    return &paseg->fDefault;
}

void SetAsegfDefault(ASEG* paseg, int fDefault)
{
    paseg->fDefault = fDefault;
}

void* GetAsegfHandsOff(ASEG* paseg)
{
    return &paseg->fHandsOff;
}

void SetAsegfHandsOff(ASEG* paseg, int fHandsOff)
{
    paseg->fHandsOff = fHandsOff;
}

void* GetAsegnPriority(ASEG* paseg)
{
    return &paseg->nPriority;
}

void SetAsegnPriority(ASEG* paseg, int nPriority)
{
    paseg->nPriority = nPriority;
}

void* GetAsegsvtMaster(ASEG* paseg)
{
    return &paseg->svtMaster;
}

void SetAsegsvtMaster(ASEG* paseg, float svtMaster)
{
    paseg->svtMaster = svtMaster;
}

void* GetAsegclqMasterSuck(ASEG* paseg)
{
    return &paseg->clqMasterSuck;
}

void SetAsegclqMasterSuck(ASEG* paseg, const CLQ& clqMasterSuck)
{
    paseg->clqMasterSuck = clqMasterSuck;
}

void* GetAseglmMasterSuck(ASEG* paseg)
{
    return &paseg->lmMasterSuck;
}

void SetAseglmMasterSuck(ASEG* paseg, const LM& lmMasterSuck)
{
    paseg->lmMasterSuck = lmMasterSuck;
}

void* GetAsegtMax(ASEG* paseg)
{
    return &paseg->tMax;
}

void SetAsegtMax(ASEG* paseg, float tMax)
{
    paseg->tMax = tMax;
}

void* GetAsegfRealClock(ASEG* paseg)
{
    return &paseg->fRealClock;
}

void SetAsegfRealClock(ASEG* paseg, int fRealClock)
{
    paseg->fRealClock = fRealClock;
}

void CloneAseg(ASEG* paseg, ASEG* pasegBase)
{
    CloneLo(paseg, pasegBase);

    paseg->tMax = pasegBase->tMax;
    paseg->cchn = pasegBase->cchn;
    paseg->achn = pasegBase->achn;
    paseg->oidRoot = pasegBase->oidRoot;
    paseg->segrpt = pasegBase->segrpt;
    paseg->fDefault = pasegBase->fDefault;
    paseg->fHandsOff = pasegBase->fHandsOff;
    paseg->fRealClock = pasegBase->fRealClock;
    paseg->ceaApply = pasegBase->ceaApply;
    paseg->aeaApply = pasegBase->aeaApply;
    paseg->ceaFrame = pasegBase->ceaFrame;
    paseg->aeaFrame = pasegBase->aeaFrame;
    paseg->ceaRetract = pasegBase->ceaRetract;
    paseg->aeaRetract = pasegBase->aeaRetract;
    paseg->dlAsega = pasegBase->dlAsega;
    paseg->nPriority = pasegBase->nPriority;
    paseg->svtMaster = pasegBase->svtMaster;
    paseg->clqMasterSuck = pasegBase->clqMasterSuck;
    paseg->lmMasterSuck = pasegBase->lmMasterSuck;
    paseg->svtMasterSuck = pasegBase->svtMasterSuck;
    paseg->pchnStrip = pasegBase->pchnStrip;
    paseg->coidSearchRoot = pasegBase->coidSearchRoot;
    paseg->aoidSearchRoot = pasegBase->aoidSearchRoot;
}

void PostAsegLoad(ASEG* paseg)
{
    PostLoLoad(paseg);

    const float uSuck = g_plsCur->uSuck;
    const float svtMasterSuck = paseg->clqMasterSuck.g0 + uSuck * (paseg->clqMasterSuck.g1 + uSuck * paseg->clqMasterSuck.g2);

    paseg->svtMasterSuck = std::clamp(svtMasterSuck, paseg->lmMasterSuck.gMin, paseg->lmMasterSuck.gMax);

    if (paseg->fDefault)
    {
        EnsureSwCallback(paseg->psw, reinterpret_cast<PFNMQ>(PostAsegLoadCallback), paseg, MSGID_callback, nullptr);
    }
}

void PostAsegLoadCallback(ASEG* paseg, MSGID msgid, void* pvData)
{
    ALO* paloAsegRoot = nullptr;

    if (paseg->oidRoot != OID_Nil)
        paloAsegRoot = (ALO*)PloFindSwNearest(paseg->psw, (OID)paseg->oidRoot, paseg);

    ASEGA* pasega = nullptr;

    ApplyAseg(paseg, paloAsegRoot, 0.0f, 1.0f, 0, &pasega);

}

void ApplyAsegOvr(ASEG* paseg, ALO* paloAsegRoot, int covr, OVR* aovr, float tLocal, float svtLocal, GRFAPL grfapl, ASEGA** ppasega)
{
    SW* psw = paseg->psw;
    ASEGA* pasega = PasegaNew(psw);


    if (ppasega != nullptr)
        *ppasega = pasega;

    pasega->tLocal = tLocal;
    pasega->paseg = paseg;
    pasega->paloRoot = paloAsegRoot;
    pasega->svtMaster = paseg->svtMaster;

    SetAsegaSpeed(pasega, svtLocal);
    AppendDlEntry(&paseg->dlAsega, pasega);

    if (paseg->fRealClock)
        AppendDlEntry(&psw->dlAsegaRealClock, pasega);
    else
        AppendDlEntry(&psw->dlAsega, pasega);

    LO* ploContext = paloAsegRoot != nullptr ? static_cast<LO*>(paloAsegRoot) : paseg;

    std::vector <ALO*> apaloSearchRoot;
    apaloSearchRoot.reserve(paseg->coidSearchRoot);

    for (int i = 0; i < paseg->coidSearchRoot; ++i)
    {
        ALO* paloSearchRoot = static_cast<ALO*>(PloFindSwObject(psw, 4, (OID)paseg->aoidSearchRoot[i], ploContext));
        apaloSearchRoot.push_back(paloSearchRoot);
    }

    const int cchn = paseg->cchn + (paseg->pchnStrip != nullptr ? 1 : 0);
    for (int ichn = 0; ichn < cchn; ++ichn)
    {
        CHN* pchn = &paseg->achn[ichn];

        ALO* palo = nullptr;
        for (int iovr = 0; iovr < covr; ++iovr)
        {
            if (aovr[iovr].oid == pchn->oid)
            {
                palo = aovr[iovr].palo;
                break;
            }
        }

        if (palo == nullptr)
        {
            palo = static_cast<ALO*>(PloFindSwObject(psw, 4, pchn->oid, ploContext));
        }

        if (palo == nullptr)
        {
            for (ALO* paloSearchRoot : apaloSearchRoot)
            {
                if (paloSearchRoot == nullptr)
                    continue;

                palo = static_cast<ALO*>(PloFindSwObject(psw, 1, pchn->oid, static_cast<LO*>(paloSearchRoot)));

                if (palo != nullptr)
                {
                    break;
                }
            }
        }

        if (palo == nullptr)
        {
            palo = static_cast<ALO*>(PloFindSwObject(psw, 260, pchn->oid, ploContext));
        }

        if (palo == nullptr)
            continue;

        ACTSEG* pactseg = PactsegNewAlo(palo);
        pactseg->ichn = ichn;
        pactseg->pasega = pasega;
        pactseg->nPriority = paseg->nPriority;



        if (ichn < paseg->cchn)
        {
            if ((grfapl & 1U) != 0)
            {
                pactseg->ackPos = ACK_Smooth;
                pactseg->ackRot = ACK_Smooth;
            }
            else
            {
                CalculateActDefaultAck(pactseg);
            }

            if (pchn->pacp == nullptr)
                pactseg->ackPos = ACK_Nil;

            if (pchn->pacr == nullptr && pchn->pacgTwist == nullptr)
                pactseg->ackRot = ACK_Nil;
        }
        else if (pchn->pacs == nullptr && pchn->apacgPose.empty())
        {
            std::erase_if(psw->actOwners, [pactseg](const std::shared_ptr<ACT>& owner)
            {
                return owner.get() == pactseg;
            });
            continue;
        }

        if (pchn->pacs != nullptr)
            pactseg->ackScale = ACK_Velocity;

        if (pchn->apacgPose.size() != 0)
        {
            pactseg->ackPose = ACK_Velocity;
        }

        AppendDlEntry(&pasega->dlActseg, pactseg);
        InsertAloAct(palo, static_cast<ACT*>(pactseg));
        ALO* paloRoot = palo->paloRoot;

        if (paloRoot == nullptr)
        {
            paloRoot = palo;

            while (paloRoot->paloParent != nullptr)
                paloRoot = paloRoot->paloParent;
        }

        if (!paloRoot->fApplyAseg && pasega->mrg.apalo.size() < 32)
        {
            paloRoot->fApplyAseg = 1;
            pasega->mrg.apalo.push_back(paloRoot);
        }
    }

    for (ALO* palo : pasega->mrg.apalo)
        palo->fApplyAseg = 0;

    AddSwMergeGroup(psw, &pasega->mrg);

    if (paseg->fHandsOff)
        SetAsegaHandsOff(pasega, 1);

    int fRetract = 0;

    for (int iea = 0; iea < paseg->ceaApply; ++iea)
        HandleAsegaEvent(pasega, &paseg->aeaApply[iea], &fRetract);

    const bool fSnap = (static_cast<uint32_t>(grfapl) & 4U) != 0;

    SnapAsega(pasega, fSnap);

    if ((grfapl & 2U) != 0)
        HandleAsegaEventsFF(pasega, paseg, &fRetract);
    else
        UpdateAsegaIeaCur(pasega);

    ASEGA* pasegaArg = pasega;
    void* pvArg = &pasegaArg;

    HandleLoSpliceEvent(paseg, 9, 1, &pvArg);

}

void ApplyAseg(ASEG* paseg, ALO* paloAsegRoot, float tLocal, float svtLocal, int grfapl, ASEGA** ppasega)
{
    ApplyAsegOvr(paseg, paloAsegRoot, 0, nullptr, tLocal, svtLocal, grfapl, ppasega);
}

ASEGA* PasegaApplyAseg(ASEG* paseg, ALO* paloAsegRoot, float tLocal, float svtLocal, GRFAPL grfapl)
{
    ASEGA* pasega = nullptr;

    ApplyAsegOvr(paseg, paloAsegRoot, 0, nullptr, tLocal, svtLocal, grfapl, &pasega);

    return pasega;
}

ASEGA* PasegaFindAseg(ASEG* paseg, ALO* paloRoot)
{
    ASEGA* pasega;

    for (pasega = (ASEGA*)paseg->dlAsega.ploFirst; pasega != nullptr; pasega = (ASEGA*)pasega->dleAseg.ploNext)
    {
        if (pasega->paloRoot == paloRoot)
            return pasega;
    }

    return nullptr;
}

void EnsureAseg(ASEG* paseg, ALO* paloRoot, SEEK seek, float t, float svt, GRFAPL grfapl, ASEGA** ppasega)
{
    ASEGA* pasegaLocal = nullptr;
    ASEGA** ppasegaOut = ppasega != nullptr ? ppasega : &pasegaLocal;

    ASEGA* pasega = PasegaFindAseg(paseg, paloRoot);

    if (pasega == nullptr)
        ApplyAseg(paseg, paloRoot, 0.0f, 1.0f, grfapl, ppasegaOut);
    else
        *ppasegaOut = pasega;

    SeekAsega(*ppasegaOut, seek, t, svt);
}

ASEGA* PasegaEnsureAseg(ASEG* paseg, ALO* paloRoot, SEEK seek, float t, float svt, GRFAPL grfapl)
{
    ASEGA* pasega = nullptr;
    EnsureAseg(paseg, paloRoot, seek, t, svt, grfapl, &pasega);
    return pasega;
}

void ApplyAsegCur(ASEG* paseg, ALO* paloRoot, float t, float svt, GRFAPL grfapl, ASEGA** ppasega)
{
    if (*ppasega != nullptr) {
        RetractAsega(*ppasega);
        *ppasega = nullptr;
    }
    if (paseg != nullptr) {
        ApplyAseg(paseg, paloRoot, t, svt, grfapl, ppasega);
    }
}

int FWipingAseg(ASEG* paseg)
{
    for (int i = 0; i < paseg->ceaFrame; ++i) {
        if (paseg->aeaFrame[i].eatyp == EATYP_Wipe) {
            return 1;
        }
    }
    return 0;
}

void FindAsegClosestPoint(ASEG* paseg, ALO* palo, const glm::vec3* ppos, float t, float* ptClosest, glm::vec3* pposClosest, glm::vec3* pvClosest)
{
    CHN* pchn = PchnFindAseg(paseg, palo);
    ACP* pacp = pchn->pacp.get();

    if (pacp == nullptr) {
        if (ptClosest != nullptr) {
            *ptClosest = 0.0f;
        }

        if (pposClosest != nullptr) {
            *pposClosest = palo->xf.pos;
        }

        if (pvClosest != nullptr) {
            *pvClosest = glm::vec3(0.0f);
        }

        return;
    }

    glm::vec3 posCur(0.0f);
    pacp->pvtacp->pfnEvaluateAcp(pacp, palo, t, 1.0f, static_cast<GRFEVAL>(0), &posCur, nullptr);

    float tBest = t;
    float distBest = glm::dot(posCur - *ppos, posCur - *ppos);

    const float tMax = paseg->tMax;
    // Original: min(tMax * 0.1, 0.5).  Using max here skips most of a
    // short curve and can seed the local closest-point search in the wrong
    // section of the patrol animation.
    const float step = glm::min(tMax * 0.1f, 0.5f);

    for (float tCur = 0.0f;; tCur = glm::min(tCur + step, tMax))
    {
        pacp = pchn->pacp.get();
        pacp->pvtacp->pfnEvaluateAcp(pacp, palo, tCur, 1.0f, static_cast<GRFEVAL>(0), &posCur, nullptr);

        float distCur = glm::dot(posCur - *ppos, posCur - *ppos);
        if (distCur < distBest) {
            tBest = tCur;
            distBest = distCur;
        }

        if (tCur >= tMax) {
            break;
        }
    }

    FindChnClosestPointLocal(pchn, palo, ppos, tMax, FLT_MAX, tBest, ptClosest, pposClosest, pvClosest);
}

EA* PeaFindAsegLabel(ASEG* paseg, OID oidLabel)
{
    for (int i = 0; i < paseg->ceaApply; ++i)
    {
        EA* pea = &paseg->aeaApply[i];

        if (pea->eatyp == EATYP_Label && pea->label.oidLabel == oidLabel)
            return pea;
    }

    for (int i = 0; i < paseg->ceaFrame; ++i)
    {
        EA* pea = &paseg->aeaFrame[i];

        if (pea->eatyp == EATYP_Label && pea->label.oidLabel == oidLabel)
            return pea;
    }

    for (int i = 0; i < paseg->ceaRetract; ++i)
    {
        EA* pea = &paseg->aeaRetract[i];

        if (pea->eatyp == EATYP_Label && pea->label.oidLabel == oidLabel)
            return pea;
    }

    return nullptr;
}

float TFindAsegLabel(ASEG* paseg, OID oidLabel)
{
    EA* pea = PeaFindAsegLabel(paseg, oidLabel);

    if (pea != nullptr)
        return pea->tLocal;

    return 0.0f;
}

CHN* PchnFindAseg(ASEG* paseg, ALO* palo)
{
    for (int ichn = 0; ichn < paseg->cchn; ichn++)
    {
        CHN* pchn = &paseg->achn[ichn];

        if (FMatchesLoName(palo, pchn->oid))
            return pchn;
    }

    return nullptr;
}

void PredictAsegEffect(ASEG* paseg, float t, float svt, ALO* paloLeaf, ALO* paloBasis, int cpaloPredict, glm::vec3* apos, glm::mat3* amat, glm::vec3* av, glm::vec3* aw)
{
    const float svtScaled = svt * paseg->svtMaster * paseg->svtMasterSuck;

    std::vector<ALO*> apalo;

    for (ALO* palo = paloLeaf; palo != paloBasis && palo != nullptr; palo = palo->paloParent)
        apalo.push_back(palo);

    for (int i = 0; i < cpaloPredict; ++i)
    {
        if (apos != nullptr)
            apos[i] = glm::vec3(0.0f);

        if (amat != nullptr)
            amat[i] = glm::mat3(1.0f);

        if (av != nullptr)
            av[i] = glm::vec3(0.0f);

        if (aw != nullptr)
            aw[i] = glm::vec3(0.0f);
    }

    glm::vec3 posSum(0.0f);
    glm::mat3 matSum(1.0f);
    glm::vec3 vSum(0.0f);
    glm::vec3 wSum(0.0f);

    for (int ipalo = static_cast<int>(apalo.size()) - 1; ipalo >= 0; --ipalo)
    {
        ALO* palo = apalo[ipalo];
        CHN* pchn = PchnFindAseg(paseg, palo);

        ACP* pacp = pchn != nullptr ? pchn->pacp.get() : nullptr;
        ACR* pacr = pchn != nullptr ? pchn->pacr.get() : nullptr;

        glm::vec3 posLocal(0.0f);
        glm::vec3 vLocal(0.0f);
        glm::mat3 matLocal(1.0f);
        glm::vec3 wLocal(0.0f);

        if (apos != nullptr || av != nullptr)
        {
            if (pacp != nullptr)
                pacp->pvtacp->pfnEvaluateAcp(pacp, palo, t, svtScaled, static_cast<GRFEVAL>(0), &posLocal, &vLocal);
            else
            {
                posLocal = palo->xf.pos;
                vLocal = palo->xf.v;
            }
        }

        if (pacr != nullptr)
            pacr->pvtacr->pfnEvaluateAcr(pacr, palo, t, svtScaled, static_cast<GRFEVAL>(0), &matLocal, &wLocal);
        else
        {
            matLocal = palo->xf.mat;
            wLocal = palo->xf.w;
        }

        if (apos != nullptr || av != nullptr)
        {
            const glm::vec3 posParent = matSum * posLocal;
            posSum += posParent;

            if (av != nullptr)
                vSum += matSum * vLocal + glm::cross(wSum, posParent);
        }

        if (aw != nullptr || av != nullptr)
            wSum += matSum * wLocal;

        matSum = matSum * matLocal;

        if (ipalo >= cpaloPredict)
            continue;

        if (apos != nullptr)
            apos[ipalo] = posSum;

        if (amat != nullptr)
            amat[ipalo] = matSum;

        if (av != nullptr)
            av[ipalo] = vSum;

        if (aw != nullptr)
            aw[ipalo] = wSum;
    }
}

void GetAsegWaypoints(ASEG* paseg, ALO* paloContext, int* pcpwaypoint, std::vector<WAYPOINT*>& waypoints)
{
    for (int i = 0; i < paseg->ceaFrame; i++)
    {
        EA* pea = &paseg->aeaFrame[i];

        if (pea->eatyp == EATYP_Waypoint)
        {
            WAYPOINT* pwaypoint = (WAYPOINT*)PloFindSwObject(
                paseg->psw, 260, pea->object.oid, (LO*)paloContext);

            if (pwaypoint != nullptr)
                waypoints.push_back(pwaypoint);
        }
    }

    if (pcpwaypoint != nullptr)
        *pcpwaypoint = static_cast<int>(waypoints.size());
}

void StripAsegChn(ASEG* paseg, CHN* pchn)
{
    int ichn = static_cast<int>(pchn - paseg->achn.data());

    CHN chnStrip = std::move(*pchn);

    paseg->achn.erase(paseg->achn.begin() + ichn);

    paseg->achn.push_back(std::move(chnStrip));

    paseg->cchn = static_cast<int>(paseg->achn.size()) - 1;
    paseg->pchnStrip = &paseg->achn[paseg->cchn];
}

void StripAsegAlo(ASEG* paseg, ALO* palo)
{
    if (paseg->pchnStrip != nullptr)
        return;

    CHN* pchn = PchnFindAseg(paseg, palo);

    if (pchn != nullptr)
        StripAsegChn(paseg, pchn);
}

void DeleteAseg(ASEG *paseg)
{
    delete paseg;
}
