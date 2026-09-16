#include "asega.h"
#include "actseg.h"
#include "acp.h"
#include "sm.h"
#include "acr.h"
#include "dialog.h"
#include "xform.h"
#include "timer.h"
#include "jt.h"
#include "credit.h"
#include "explo.h"
#include "jsg.h"
#include "fader.h"
#include "hubblot.h"
#include "hubsel.h"
#include "fmv.h"

ASEGA* PasegaNew(SW* psw)
{
    auto pasegaOwner = std::make_shared<ASEGA>();
    ASEGA* pasega = pasegaOwner.get();

    pasega->pvtasega = &g_vtasega;
    InitDl(&pasega->dlActseg, offsetof(ACTSEG, dleAsega));

    pasega->mrg.apalo.clear();
    pasega->mrg.apalo.reserve(32);

    psw->asegaOwners.push_back(std::move(pasegaOwner));

    return pasega;
}

void* GetAsegaPaseg(ASEGA* pasega)
{
    return &pasega->paseg;
}

void SetAsegaPaseg(ASEGA* pasega, ASEG* pasega1)
{
    pasega->paseg = pasega1;
}

void* GetAsegaPaloRoot(ASEGA* pasega)
{
    return &pasega->paloRoot;
}

void SetAsegaPaloRoot(ASEGA* pasega, ALO* paloRoot)
{
    pasega->paloRoot = paloRoot;
}

void* GetAsegaTLocal(ASEGA* pasega)
{
    return &pasega->tLocal;
}

void SetAsegaTLocal(ASEGA* pasega, float tLocal)
{
    pasega->tLocal = tLocal;
}

void* GetAsegaHandsOff(ASEGA* pasega)
{
    return &pasega->fHandsOff;
}

void SetAsegaHandsOff(ASEGA* pasega, int fHandsOff)
{
    if (fHandsOff != pasega->fHandsOff) 
    {
        if (fHandsOff == 0) {
            DecrementSwHandsOff(g_psw);
            pasega->fHandsOff = fHandsOff;
        }
        else {
            IncrementSwHandsOff(g_psw);
            pasega->fHandsOff = fHandsOff;
        }
    }
}

void UpdateAsegaIeaCur(ASEGA* pasega)
{
    ASEG* paseg = pasega->paseg;
    const int ceaFrame = static_cast<int>(paseg->aeaFrame.size());

    if (ceaFrame == 0)
        return;

    if (pasega->svtLocal >= 0.0f)
    {
        int iea = 0;

        while (iea < ceaFrame && paseg->aeaFrame[iea].tLocal < pasega->tLocal)
            ++iea;

        pasega->ieaCur = iea;
    }
    else
    {
        int iea = ceaFrame - 1;

        while (iea >= 0 && paseg->aeaFrame[iea].tLocal > pasega->tLocal)
            --iea;

        pasega->ieaCur = iea + 1;
    }
}

ACTSEG* PactsegFindAsega(ASEGA* pasega, OID oid)
{
    for (ACTSEG* pactseg = pasega->dlActseg.pactsegFirst; pactseg != nullptr; pactseg = pactseg->dleAsega.pactsegNext) {
        if (FMatchesLoName(pactseg->palo, oid)) {
            return pactseg;
        }
    }

    return nullptr;
}

void HandleAsegaEvent(ASEGA* pasega, EA* pea, int* pfRetracted)
{
    ASEG* paseg = pasega->paseg;
    SW* psw = paseg->psw;

    const bool fReverse = pasega->svtLocal < 0.0f && std::any_of(paseg->aeaApply.begin(), paseg->aeaApply.end(), [pea](const EA& ea) { return &ea == pea; });

    ALO* paloRoot = pasega->paloRoot ? pasega->paloRoot : (ALO*)paseg;

    switch (pea->eatyp)
    {
        case EATYP_SoundEffect:
        {
            EASND* peasnd = pea->sound.peasnd.get();

            if (!peasnd || peasnd->csfxid <= 0)
                break;

            int isfxid = 0;

            if (peasnd->csfxid > 1)
            {
                const int offset = NRandInRange(0, peasnd->csfxid - 2);
                isfxid = (peasnd->isfxidLast + offset + 1) % peasnd->csfxid;
                peasnd->isfxidLast = static_cast<s16>(isfxid);
            }

            const SFXID sfxid = static_cast<SFXID>(peasnd->asfxid[isfxid]);

            if (sfxid == static_cast<SFXID>(-1))
                break;

            const bool fContinuous = FContinuousSound(sfxid);

            if (pasega->pambContinuous && fContinuous)
                return;

            ALO* paloSound = paloRoot;

            if (peasnd->oid != -1)
                paloSound = static_cast<ALO*>(PloFindSwObject(psw, 260, static_cast<OID>(peasnd->oid), paloRoot));

            AMB** ppamb = fContinuous ? &pasega->pambContinuous : nullptr;

            if (!paloSound)
                StartSound(sfxid, ppamb, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            else
            {
                // StartSound takes the outer stop radius first and the inner
                // full-volume radius second. EASND stores them as sFull/sStop,
                // and retail passes offsets +24, +20 in that order.
                StartSound(sfxid, ppamb, paloSound, nullptr,
                    static_cast<float>(peasnd->sStop), static_cast<float>(peasnd->sFull),
                    peasnd->gVolume, peasnd->gPitch, peasnd->gDoppler, nullptr, nullptr);

                if (fContinuous && pasega->pambContinuous) 
                    paloSound->pvtlo->pfnSendLoMessage(paloSound, MSGID_sfx_start, nullptr);
            }

            break;
        }

        case EATYP_SoundStop:
        {
            if (pasega->pambContinuous)
                StopSound(pasega->pambContinuous, 0);

            break;
        }

        case EATYP_MusicStart:
        ContinueMusic();
        break;

        case EATYP_MusicStop:
        PauseMusic();
        break;

        case EATYP_Seek:
        case EATYP_SeekIfMessage:
        {
            pasega->tLocal = pea->seek.tToLocal;
            SetAsegaSpeed(pasega, pea->seek.svtLocal);
            UpdateAsegaIeaCur(pasega);
            AdaptAsega(pasega);
            break;
        }

        case EATYP_SetSpeed:
        SetAsegaSpeed(pasega, pea->setspeed.svtLocal);
        break;

        case EATYP_ShowDialog:
        {
            DIALOG* pdialog = static_cast<DIALOG*>(PloFindSwObject(psw, 260, pea->object.oid, paloRoot));

            if (pdialog)
                TriggerDialog(pdialog);

            break;
        }

        case EATYP_Retract:
        {
            RetractAsega(pasega);
            *pfRetracted = 1;
            break;
        }

        case EATYP_ShowObject:
        case EATYP_HideObject:
        {
            const bool fShow = (pea->eatyp == EATYP_ShowObject) ^ fReverse;
            ALO* palo = static_cast<ALO*>(PloFindSwObject(psw, 260, pea->showhide.oid, paloRoot));

            if (!palo)
                break;

            if (pea->showhide.dt == 0.0f)
            {
                if (fShow)
                    palo->pvtlo->pfnAddLo(palo);
                else
                    palo->pvtlo->pfnRemoveLo(palo);
            }
            else if (fShow)
                FadeAloIn(palo, pea->showhide.dt);
            else
                FadeAloOut(palo, pea->showhide.dt);

            break;
        }

        case EATYP_Pause:
        {
            pasega->tLocal = std::clamp(pea->tLocal, 0.0f, paseg->tMax);
            pasega->svtLocal = 0.0f;
            break;
        }

        case EATYP_EnableCamera:
        case EATYP_DisableCamera:
        {
            CAMERA* pcamera = static_cast<CAMERA*>(PloFindSwObject(psw, 4, pea->object.oid, paseg));

            if (!pcamera)
                break;

            const bool fEnable = (pea->eatyp == EATYP_EnableCamera) ^ fReverse;

            if (fEnable)
                EnableCamera(pcamera);
            else
                DisableCamera(pcamera);

            break;
        }

        case EATYP_EnableHandsOff:
        case EATYP_DisableHandsOff:
        {
            const bool fEnable = (pea->eatyp == EATYP_EnableHandsOff) ^ fReverse;
            SetAsegaHandsOff(pasega, fEnable);
            break;
        }

        case EATYP_SetObjectInteract:
        {
            SO* pso = static_cast<SO*>(PloFindSwObject(psw, 260, pea->setinteract.oid, paloRoot));

            if (!pso)
                break;

            CT ctForce  = static_cast<CT>(pea->setinteract.ctForce);
            CT ctTorque = static_cast<CT>(pea->setinteract.ctTorque);

            if (pea->setinteract.ctForce < 0)
                ctForce = pso->constrForce.ct;

            if (pea->setinteract.ctTorque < 0)
                ctTorque = pso->constrTorque.ct;

            SetSoConstraints(pso, ctForce, nullptr, ctTorque, nullptr);
            break;
        }

        case EATYP_EnableObjectDrive:
        case EATYP_DisableObjectDrive:
        {
            ACTSEG* pactseg = PactsegFindAsega(pasega, pea->object.oid);

            if (!pactseg)
                break;

            const bool fEnable = (pea->eatyp == EATYP_EnableObjectDrive) ^ fReverse;

            if (fEnable)
            {
                pactseg->ackPos = ACK_Drive;
                pactseg->ackRot = ACK_Drive;
            }
            else
                CalculateActDefaultAck(pactseg);

            break;
        }

        case EATYP_EnableObjectPuppet:
        case EATYP_DisableObjectPuppet:
        {
            bool fEnable = (pea->eatyp == EATYP_EnableObjectPuppet) ^ fReverse;

            SetJtPuppet(g_pjt, (EA*)fEnable);

            if (fEnable && pea->puppet.fSnap)
                SnapAsega(pasega, 1);

            break;
        }

        case EATYP_EnableObjectPosErrorFunc:
        case EATYP_DisableObjectPosErrorFunc:
        case EATYP_EnableObjectRotErrorFunc:
        case EATYP_DisableObjectRotErrorFunc:
        {
            EAEF* peaef = pea->errorfunc.peaef.get();

            if (!peaef)
                break;

            ACTSEG* pactseg = PactsegFindAsega(pasega, peaef->oid);

            if (!pactseg)
                break;

            switch (pea->eatyp)
            {
                case EATYP_EnableObjectPosErrorFunc:
                pasega->peaPosError = pea;
                pasega->pactsegError = pactseg;
                return;

                case EATYP_DisableObjectPosErrorFunc:
                pasega->peaPosError = nullptr;
                break;

                case EATYP_EnableObjectRotErrorFunc:
                pasega->peaRotError = pea;
                pasega->pactsegError = pactseg;
                return;

                case EATYP_DisableObjectRotErrorFunc:
                pasega->peaRotError = nullptr;
                break;

                default:
                break;
            }

            if (!pasega->peaPosError && !pasega->peaRotError)
                pasega->pactsegError = nullptr;

            break;
        }

        case EATYP_Wipe:
        ActivateWipe(&g_wipe, pea->wipe.ptrans.get(), static_cast<WIPEK>(pea->wipe.wipek));
        break;

        case EATYP_WipeOnButton:
        SetWipeButtonTrans(&g_wipe, pea->wipe.ptrans.get(), static_cast<WIPEK>(pea->wipe.wipek));
        break;

        case EATYP_TriggerDefaultExit:
        {
            const bool fCelebratingKey =
                g_pjt &&
                g_pjt->jts == JTS_Celebrate &&
                g_pjt->jtbs == JTBS_Celebrate_Key &&
                reinterpret_cast<WS*>(g_plsCur) == g_pwsCur;

            if (!fCelebratingKey)
                TriggerDefaultExit(pea->defexit.fInhibitAnims, static_cast<WIPEK>(pea->defexit.wipek));

            break;
        }

        case EATYP_StartAnimation:
        {
            EAA* peaa = pea->animation.peaa.get();

            if (!peaa)
            {
                break;
            }

            ASEG* pasegStart = static_cast<ASEG*>(
                PloFindSwNearest(psw, peaa->oidAseg, paseg->paloParent));

            ALO* paloStart = nullptr;

            if (peaa->oidRoot != OID_Nil)
            {
                paloStart = static_cast<ALO*>(
                    PloFindSwNearest(psw, peaa->oidRoot, paseg->paloParent));
            }

            if (pasegStart)
                ApplyAseg(pasegStart, paloStart, peaa->tLocal, peaa->svtLocal, 0, nullptr);

            break;
        }

        case EATYP_EnableObjectScale:
        case EATYP_DisableObjectScale:
        case EATYP_EnableObjectMesh:
        case EATYP_DisableObjectMesh:
        {
            ACTSEG* pactseg = PactsegFindAsega(pasega, pea->object.oid);

            if (!pactseg)
                break;

            CHN& chn = paseg->achn[pactseg->ichn];
            bool fRecache = false;

            if (pea->eatyp == EATYP_EnableObjectScale && chn.pacs)
            {
                const ACK ackPrevious = pactseg->ackScale;
                pactseg->ackScale = ACK_Velocity;
                fRecache = ackPrevious == ACK_Nil || ackPrevious == ACK_Spring;
            }
            else if (pea->eatyp == EATYP_DisableObjectScale && chn.pacs)
            {
                const ACK ackPrevious = pactseg->ackScale;
                pactseg->ackScale = ACK_Nil;
                fRecache = ackPrevious != ACK_Nil;
            }
            else if (pea->eatyp == EATYP_EnableObjectMesh && chn.apacgPose.size() != 0)
            {
                const ACK ackPrevious = pactseg->ackPose;
                pactseg->ackPose = ACK_Velocity;
                fRecache = ackPrevious == ACK_Nil || ackPrevious == ACK_Spring;
            }
            else if (pea->eatyp == EATYP_DisableObjectMesh && chn.apacgPose.size() != 0)
            {
                const ACK ackPrevious = pactseg->ackPose;
                pactseg->ackPose = ACK_Nil;
                fRecache = ackPrevious != ACK_Nil;
            }

            if (fRecache)
                pactseg->palo->pvtalo->pfnRecacheAloActList(pactseg->palo);

            break;
        }

        case EATYP_StartTimer:
        SetTimer(&g_timer, pea->timer.dt);
        StartTimer(&g_timer);
        break;

        case EATYP_StopTimer:
        StopTimer(&g_timer);
        break;

        case EATYP_CutObjectPos:
        case EATYP_CutObjectRot:
        {
            ACTSEG* pactseg = PactsegFindAsega(pasega, pea->object.oid);

            if (!pactseg)
                break;

            CHN& chn = paseg->achn[pactseg->ichn];
            ALO* palo = pactseg->palo;

            if (pea->eatyp == EATYP_CutObjectPos)
            {
                glm::vec3 pos{};
                glm::vec3 velocity{};

                chn.pacp->pvtacp->pfnEvaluateAcp(chn.pacp.get(), palo, pea->tLocal, pasega->svtLocal, static_cast<GRFEVAL>(0), &pos, &velocity);

                palo->pvtalo->pfnTranslateAloToPos(palo, &pos);
                palo->pvtalo->pfnSetAloVelocityVec(palo, &velocity);
            }
            else
            {
                glm::mat3 rotation{ 1.0f };
                glm::vec3 angularVelocity{};

                chn.pacr->pvtacr->pfnEvaluateAcr(chn.pacr.get(), palo, pea->tLocal, pasega->svtLocal, static_cast<GRFEVAL>(0), &rotation, &angularVelocity);

                palo->pvtalo->pfnRotateAloToMat(palo, &rotation);
                palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &angularVelocity);
            }

            break;
        }

        case EATYP_StartRumble:
        //TriggerJoyRumbleRumk(&g_joy, static_cast<RUMK>(pea->rumble.rumk), static_cast<float>(pea->rumble.dframe) / 60.0f);
        break;

        case EATYP_SendUserMessage:
        {
            ALO* paloTarget = nullptr;

            if (pea->sendusermessage.oid == OID_Nil)
                paloTarget = pasega->paloRoot;
            else
                paloTarget = static_cast<ALO*>(PloFindSwNearest(psw, pea->sendusermessage.oid, pasega->paloRoot));

            if (paloTarget)
            {
                void* apvArgs[2] =
                {
                    pasega,
                    reinterpret_cast<void*>(static_cast<intptr_t>(pea->sendusermessage.oidLabel))
                };

                paloTarget->pvtlo->pfnSendLoMessage(paloTarget, MSGID_user_message, apvArgs);
            }
        
            break;
        }

        case EATYP_Label:
        {
            if (!pasega->paloRoot)
                return;

            ASEGLABELMSG msg{};
            msg.pasega = pasega;
            msg.tLocal = pea->tLocal;
            msg.label = pea->label.oidLabel;

            SendAsegaMessage(pasega, MSGID_label, &msg);
            break;
        }

        case EATYP_Splice:
        {
            void* apvArgs[2] =
            {
                &pasega,
                &pea->splice
            };

            HandleLoSpliceEvent(paseg, 10, 2, apvArgs);
            break;
        }

        case EATYP_Waypoint:
        {
            if (!pasega->paloRoot)
                return;

            LO* ploWaypoint = PloFindSwObject(psw, 260, pea->object.oid, pasega->paloRoot);

            if (!ploWaypoint)
                return;

            void* apvArgs[2] =
            {
                pasega,
                ploWaypoint
            };

            SendAsegaMessage(pasega, MSGID_waypoint, apvArgs);
            break;
        }

        case EATYP_PauseForVag:
        {
            if (FPauseForVag())
            {
                pasega->tLocal = std::clamp(pea->tLocal, 0.0f, paseg->tMax);
                --pasega->ieaCur;
                *pfRetracted = 1;
            }

            break;
        }

        case EATYP_StopVag:
        StopVag();
        break;

        case EATYP_PreloadVag:
        PreloadVag(pea->string.pchz.get());
        break;

        case EATYP_PoseCombo:
        {
            ALO* palo = static_cast<ALO*>(
                PloFindSwObject(psw, 260, pea->posecombo.oidTarget, pasega->paloRoot));

            if (palo)
                SetAloPoseCombo(palo, pea->posecombo.oidCombo);

            break;
        }

        case EATYP_SetSmGoal:
        {
            SM* psm = static_cast<SM*>(
                PloFindSwObject(psw, 260, static_cast<OID>(pea->setsm.oidSm), paloRoot));

            ALO* palo = static_cast<ALO*>(
                PloFindSwObject(psw, 260, static_cast<OID>(pea->setsm.oidRoot), paloRoot));

            if (!psm || !palo)
                break;

            SMA* psma = PsmaFindSm(psm, palo);

            if (!psma)
                psma = PsmaApplySm(psm, palo, static_cast<OID>(pea->setsm.oidState), 1);

            if (psma)
                SetSmaGoal(psma, static_cast<OID>(pea->setsm.oidState));

            break;
        }

        case EATYP_RetractSm:
        {
            SM* psm = static_cast<SM*>(PloFindSwNearest(psw, pea->retractsm.oidSm, paloRoot));

            if (!psm)
                break;

            ALO* palo = static_cast<ALO*>(PloFindSwNearest(psw, pea->retractsm.oidRoot, paloRoot));
            SMA* psma = PsmaFindSm(psm, palo);

            if (psma)
                RetractSma(psma);

            break;
        }

        case EATYP_Explode:
        {
            EAEX* peaex = pea->explode.peaex.get();

            if (!peaex) {
                break;
            }

            EXPLO* pexplo = static_cast<EXPLO*>(PloFindSwObject(psw, 260, peaex->oidExpl, paloRoot));

            if (!pexplo)
                break;

            GRFEXPLSO grfexplso = 0;
            ALO* paloReference = nullptr;
            glm::vec3 vecReference{};
            SO* psoTouch = nullptr;

            if (peaex->oidRef != OID_Nil)
            {
                LO* ploReference = PloFindSwObject(psw, 4, peaex->oidRef, paloRoot);

                if (ploReference)
                {
                    if (FIsBasicDerivedFrom(ploReference, CID_ALO))
                    {
                        grfexplso |= 1;
                        paloReference = static_cast<ALO*>(ploReference);
                    }
                    else if (FIsBasicDerivedFrom(ploReference, CID_PNT))
                    {
                        grfexplso |= 4;
                        GetPntPos(static_cast<PNT*>(ploReference), &vecReference);
                    }
                    else if (FIsBasicDerivedFrom(ploReference, CID_XFM))
                    {
                        grfexplso |= 4;
                        GetXfmPos(static_cast<XFM*>(ploReference), &vecReference);
                    }
                }
            }

            if (peaex->oidTouch != OID_Nil)
            {
                psoTouch = static_cast<SO*>(PloFindSwObject(psw, 4, peaex->oidTouch, paloRoot));

                if (psoTouch)
                    grfexplso |= 32;
            }

            EXPLSO explso{};

            explso.grfexplso = grfexplso;
            explso.paloReference = paloReference;
            explso.vec = vecReference;
            explso.posOrigin = vecReference;
            explso.psoTouch = psoTouch;

            if (peaex->gScale != 1.0f)
            {
                explso.grfexplso |= 24;
                explso.rScale = peaex->gScale;
                explso.sRadius = peaex->gScale;
            }

            pexplo->pvtexpl->pfnExplodeExplExplso(pexplo, &explso);
            break;
        }

        case EATYP_ShowLetterbox:
        AcquireSwLetterbox(psw);
        break;

        case EATYP_HideLetterbox:
        ReleaseSwLetterbox(psw);
        break;

        case EATYP_ShowMovie:
        QueueCutsceneDescriptor(pea->string.pchz.get());
        break;

        case EATYP_PlaceCredits:
        g_credit.pvtblot->pfnSetBlotClock(&g_credit, &pasega->tLocal);
        PlaceCredit(&g_credit, static_cast<float>(pea->place.x), static_cast<float>(pea->place.y), pea->place.cline);
        break;

        case EATYP_VacateCredits:
        g_credit.pvtblot->pfnSetBlotClock(&g_credit, nullptr);
        VacateCredit(&g_credit);
        break;

        case EATYP_SetCredit:
        SetCreditLine(&g_credit, pea->credit.iline, pea->credit.pchz.get(), static_cast<float>(pea->credit.dtVisible) / 256.0f);
        break;

        case EATYP_StartSplash:
        g_ui.uisPlaying = UIS_Splash;
        SetUiUis(&g_ui, UIS_Splash);
        break;

        case EATYP_StartAttract:
        g_ui.uisPlaying = UIS_Attract;
        SetUiUis(&g_ui, UIS_Attract);
        break;

        case EATYP_SetDark:
        SetSwDarken(psw, pea->dark.rDark);
        break;

        case EATYP_SetDarkSmooth:
        SetSwDarkenSmooth(psw, pea->dark.rDark);
        break;

        case EATYP_StartScript:
        {
            JSG* pjsg = static_cast<JSG*>(PloFindSwObject(psw, 260, pea->object.oid, paloRoot));

            if (pjsg)
                ApplyJsg(pjsg, g_pjt);

            break;
        }

        default:
      
        break;
    }
}

void HandleAsegaEventsFF(ASEGA* pasega, ASEG* paseg, int* pfRetract)
{
    constexpr float kEventEpsilon = 0.0001f;

    while (pasega->ieaCur < static_cast<int>(paseg->aeaFrame.size()))
    {
        EA* pea = &paseg->aeaFrame[pasega->ieaCur];

        if (pasega->tLocal + kEventEpsilon <= pea->tLocal)
            return;

        ++pasega->ieaCur;

        switch (pea->eatyp)
        {
            case EATYP_Retract:
            pasega->tLocal = pea->tLocal;
            pasega->svtLocal = 0.0f;
            *pfRetract = 1;
            return;

            case EATYP_ShowObject:
            case EATYP_HideObject:
            case EATYP_EnableHandsOff:
            case EATYP_DisableHandsOff:
            case EATYP_SetObjectInteract:
            case EATYP_EnableObjectDrive:
            case EATYP_DisableObjectDrive:
            case EATYP_StartAnimation:
            case EATYP_Label:
            HandleAsegaEvent(pasega, pea, pfRetract);
            break;

            default:
            break;
        }
    }
}

void HandleAsegaEvents(ASEGA* pasega, ASEG* paseg, int* pfRetracted)
{
    const int ceaFrame = static_cast<int>(paseg->aeaFrame.size());

    if (ceaFrame == 0)
        return;

    while (true)
    {
        int iea = pasega->ieaCur;

        if (iea < ceaFrame)
        {
            EA* pea = &paseg->aeaFrame[iea];
            const bool fPassedForward = pea->tLocal < pasega->tLocal || (pea->tLocal == pasega->tLocal && pasega->svtLocal > 0.0f);

            if (fPassedForward)
            {
                pasega->ieaCur = iea + 1;
                HandleAsegaEvent(pasega, pea, pfRetracted);

                if (*pfRetracted != 0)
                    return;

                continue;
            }
        }

        iea = pasega->ieaCur;

        if (iea <= 0)
            return;

        EA* pea = &paseg->aeaFrame[iea - 1];
        const bool fPassedBackward = pea->tLocal > pasega->tLocal || (pea->tLocal == pasega->tLocal && pasega->svtLocal < 0.0f);

        if (!fPassedBackward)
            return;

        pasega->ieaCur = iea - 1;
        HandleAsegaEvent(pasega, pea, pfRetracted);

        if (*pfRetracted != 0)
            return;
    }
}

void RemoveAsega(ASEGA* pasega)
{
    SW* psw = pasega->paseg->psw;

    RemoveDlEntry(&psw->dlAsegaPending, pasega);
    std::erase_if(psw->asegaOwners, [pasega](const std::shared_ptr<ASEGA>& owner) { return owner.get() == pasega; });
}

void RetractAsega(ASEGA* pasega)
{
    ASEG* paseg = pasega->paseg;
    SW* psw = paseg->psw;
    for (EA& ea : paseg->aeaRetract)
    {
        int fRetracted = 0;
        HandleAsegaEvent(pasega, &ea, &fRetracted);
    }

    SendAsegaMessage(pasega, MSGID_asega_retracted, pasega);

    FreeSwMqList(psw, pasega->pmqFirst);
    pasega->pmqFirst = nullptr;

    RemoveSwMergeGroup(psw, &pasega->mrg);
    SetAsegaHandsOff(pasega, 0);
    ClearSwCallbacks(psw, 2, nullptr, pasega, MSGID_Nil, nullptr);

    RemoveDlEntry(&paseg->dlAsega, pasega);

    if (paseg->fRealClock)
        RemoveDlEntry(&psw->dlAsegaRealClock, pasega);
    else
        RemoveDlEntry(&psw->dlAsega, pasega);

    AppendDlEntry(&psw->dlAsegaPending, pasega);

    ACTSEG* pactseg = pasega->dlActseg.pactsegFirst;

    while (pactseg != nullptr)
    {
        // RetractAct removes the entry and releases its shared owner. Preserve
        // the list link before that call so traversal never reads freed storage.
        ACTSEG* pactsegNext = pactseg->dleAsega.pactsegNext;
        pactseg->pvtact->pfnRetractAct(pactseg, 7);
        pactseg = pactsegNext;
    }

    if (pasega->pambContinuous != nullptr)
        DropPamb(&pasega->pambContinuous);

    void* apvArgs[1] = { &pasega->paloRoot };
    HandleLoSpliceEvent(static_cast<LO*>(paseg), 11, 1, apvArgs);

}

void RetractAsegaActseg(ASEGA* pasega, OID oid, GRFRA grfra)
{
    ACTSEG* pactseg = PactsegFindAsega(pasega, oid);

    if (pactseg != nullptr)
        pactseg->pvtactseg->pfnRetractActseg(pactseg, grfra);
}

float UFromEaErrorFunc(EA* pea, float s)
{
    EAEF* peaef = pea->errorfunc.peaef.get();

    if (s < peaef->sFull)
        return 1.0f;

    if (s >= peaef->sStop)
        return 0.15f;

    const float u = (s - peaef->sFull) / (peaef->sStop - peaef->sFull);
    return 1.0f - u * 0.85f;
}

int FWrapAsegaTime(ASEGA* pasega, float* ptLocal, float* psvtLocal)
{
    ASEG* paseg = pasega->paseg;

    float t = *ptLocal;
    float tMax = paseg->tMax;

    if (t >= tMax)
    {
        switch (paseg->segrpt)
        {
            case SEGRPT_Loop:
            *ptLocal = fmodf(t, tMax);
            return 1;

            case SEGRPT_Once:
            if (t > tMax)
            {
                *ptLocal = tMax;
                *psvtLocal = 0.0f;
            }
            return 1;

            case SEGRPT_PingPong:
            {
                float period = tMax * 2.0f;

                if (t >= period)
                {
                    t = fmodf(t, period);
                    *ptLocal = t;
                    *psvtLocal = fabsf(*psvtLocal);
                }

                if (t < tMax)
                    return 1;

                *ptLocal = period - t;
                *psvtLocal = -(*psvtLocal);
                return 1;
            }

            default:
            return 1;
        }
    }

    if (t < 0.0f)
    {
        switch (paseg->segrpt)
        {
            case SEGRPT_Loop:
            *ptLocal = GModPositive(t, tMax);
            return 1;

            case SEGRPT_Once:
            *ptLocal = 0.0f;
            *psvtLocal = 0.0f;
            return 1;

            case SEGRPT_PingPong:
            {
                if (-t <= tMax)
                {
                    *ptLocal = -t;
                }
                else
                {
                    *psvtLocal = fabsf(*psvtLocal);

                    float period = tMax * 2.0f;
                    t = GModPositive(t, period);

                    *ptLocal = t;

                    if (t < tMax)
                        return 1;

                    *ptLocal = period - t;
                }

                *psvtLocal = -(*psvtLocal);
                return 1;
            }

            default:
            return 1;
        }
    }

    return 0;
}

void UpdateAsega(ASEGA* pasega, float dt)
{
    ASEG* paseg = pasega->paseg;

    // Retail bypasses the frozen-object early-out while Ruby's synchronized
    // music system is active.  Her animation sequences intentionally include
    // objects which may be frozen during world restart; treating those like a
    // normal ASEG leaves the entire sequence parked at tLocal == 0.
    if (paseg->psw->fRubyMusicActive == 0)
    {
        for (ACTSEG* pactseg = pasega->dlActseg.pactsegFirst; pactseg != nullptr;
             pactseg = pactseg->dleAsega.pactsegNext)
        {
            // The original reads packed bit 38 from the 64-bit flag block that
            // begins at mtlk. In this port that bit is represented explicitly by
            // ALO::fFrozen; ALO::mtlk itself is only the first 8-bit field.
            if (pactseg->palo->fFrozen != 0)
            {
                StopSound(pasega->pambContinuous, 0);
                return;
            }
        }
    }

    if (std::abs(pasega->svtLocal) < 0.0001f)
    {
        return;
    }

    float uError = 1.0f;

    if (pasega->pactsegError != nullptr)
    {
        ACTSEG* pactsegError = pasega->pactsegError;

        if (pasega->peaPosError != nullptr)
        {
            glm::vec3 posGoal(0.0f);

            pactsegError->pvtactseg->pfnGetActsegPositionGoal(pactsegError, 0.0f, &posGoal, nullptr);

            glm::vec3 dpos = pactsegError->palo->xf.pos - posGoal;

            if (pactsegError->ackPos == ACK_Drive)
                dpos.z = 0.0f;

            uError = glm::min(uError, UFromEaErrorFunc(pasega->peaPosError, glm::length(dpos)));
        }

        if (pasega->peaRotError != nullptr)
        {
            glm::mat3 matGoal(1.0f);
            glm::mat3 dmat(1.0f);

            pactsegError->pvtactseg->pfnGetActsegRotationGoal(pactsegError, 0.0f, &matGoal, nullptr);
            CalculateDmat(&pactsegError->palo->xf.mat, &matGoal, &dmat);

            const float cosRotation = glm::clamp(CosRotateMatrixMagnitude(&dmat), -1.0f, 1.0f);
            const float radError = std::acos(cosRotation);

            uError = glm::min(uError, UFromEaErrorFunc(pasega->peaRotError, radError));
        }

        uError = glm::clamp(uError, 0.0f, 1.0f);

        if (uError < 0.0001f)
            return;
    }

    if (pasega->dtPause > 0.0f)
    {
        pasega->dtPause -= dt;

        if (pasega->dtPause <= 0.0f)
        {
            dt = -pasega->dtPause;
            pasega->dtPause = 0.0f;
        }
        else
            dt = 0.0f;
    }

    pasega->tLocal += dt * pasega->svtLocal * uError;

    int fRetracted = 0;
    bool fWrapped = false;
    bool fAtLimit = false;

    HandleAsegaEvents(pasega, paseg, &fRetracted);

    if (fRetracted != 0)
        return;

    if (FWrapAsegaTime(pasega, &pasega->tLocal, &pasega->svtLocal) != 0)
    {
        if (paseg->segrpt == SEGRPT_Once)
            fAtLimit = true;
        else if (paseg->segrpt == SEGRPT_Loop)
        {
            fWrapped = true;
            pasega->ieaCur = pasega->svtLocal <= 0.0f ? paseg->ceaFrame : 0;
        }
    }

    HandleAsegaEvents(pasega, paseg, &fRetracted);

    if (fRetracted != 0)
        return;

    for (int i = 0; i < pasega->mrg.apalo.size(); ++i)
        ResolveAlo(pasega->mrg.apalo[i]);

    if (fWrapped)
    {
        for (ACTSEG* pactseg = pasega->dlActseg.pactsegFirst; pactseg != nullptr;)
        {
            ACTSEG* pactsegNext = pactseg->dleAsega.pactsegNext;
            CHN* pchn = pactseg->ichn < paseg->cchn ? &paseg->achn[pactseg->ichn] : paseg->pchnStrip;

            if (pchn != nullptr)
            {
                if (pchn->pacp != nullptr && pchn->pacp->fContiguous == 0 && pactseg == pactseg->palo->pactPos)
                {
                    glm::vec3 posGoal(0.0f);
                    glm::vec3 velocity(0.0f);

                    pactseg->pvtactseg->pfnGetActsegPositionGoal(pactseg, 0.0f, &posGoal, &velocity);
                    pactseg->palo->pvtalo->pfnTranslateAloToPos(pactseg->palo, &posGoal);
                    pactseg->palo->pvtalo->pfnSetAloVelocityVec(pactseg->palo, &velocity);
                }

                if (pchn->pacr != nullptr && pchn->pacr->fContiguous == 0 && pactseg == pactseg->palo->pactRot)
                {
                    glm::mat3 matGoal(1.0f);
                    glm::vec3 angularVelocity(0.0f);

                    pactseg->pvtactseg->pfnGetActsegRotationGoal(pactseg, 0.0f, &matGoal, &angularVelocity);

                    pactseg->palo->pvtalo->pfnRotateAloToMat(pactseg->palo, &matGoal);
                    pactseg->palo->pvtalo->pfnSetAloAngularVelocityVec(pactseg->palo, &angularVelocity);
                }
            }

            pactseg = pactsegNext;
        }
    }

    if (fAtLimit)
        SendAsegaMessage(pasega, MSGID_asega_limit, pasega);
    else if (fWrapped)
        SendAsegaMessage(pasega, MSGID_asega_wrapped, pasega);
}

void SeekAsega(ASEGA* pasega, SEEK seek, float dtLocal, float svtLocal)
{
    float tLocal;

    switch (seek)
    {
        case SEEK_Start:
        tLocal = dtLocal;
        break;

        case SEEK_Current:
        tLocal = pasega->tLocal + dtLocal;
        break;

        case SEEK_End:
        case SEEK_EndDoEvents:
        tLocal = pasega->paseg->tMax + dtLocal;
        break;

        default:
        return;
    }

    SetAsegaSpeed(pasega, svtLocal);

    if (tLocal == pasega->tLocal)
        return;

    pasega->tLocal = tLocal;

    int fRetract = 0;

    if (seek == SEEK_EndDoEvents)
        HandleAsegaEventsFF(pasega, pasega->paseg, &fRetract);
    else
        UpdateAsegaIeaCur(pasega);

    SnapAsega(pasega, 0);
    AdaptAsega(pasega);

    if (fRetract != 0)
        RetractAsega(pasega);
}

void SnapAsega(ASEGA* pasega, int fForce)
{
    for (ACTSEG* pactseg = pasega->dlActseg.pactsegFirst; pactseg != nullptr; pactseg = pactseg->dleAsega.pactsegNext)
        SnapAct(static_cast<ACT*>(pactseg), fForce);
}

void AdaptAsega(ASEGA* pasega)
{
    for (ACTSEG* pactseg = pasega->dlActseg.pactsegFirst; pactseg != nullptr; pactseg = pactseg->dleAsega.pactsegNext)
        AdaptAct(pactseg);
}

void FindChnClosestPointLocal(CHN* pchn, ALO* palo, const glm::vec3* ppos, float tAsegMax, float sIgnore, float t, float* ptClosest, glm::vec3* pposClosest, glm::vec3* pvClosest)
{
    ACP* pacp = pchn->pacp.get();

    glm::vec3 pos{};
    glm::vec3 velocity{};

    pacp->pvtacp->pfnEvaluateAcp(pacp, palo, t, 1.0f, static_cast<GRFEVAL>(0), &pos, &velocity);

    glm::vec3 dpos = *ppos - pos;
    float g = glm::dot(dpos, velocity);

    if (glm::dot(dpos, dpos) <= sIgnore * sIgnore && std::abs(g) >= 0.0001f)
    {
        float tMin;
        float tMax;
        float gMin;
        float gMax;

        glm::vec3 posMin;
        glm::vec3 posMax;
        glm::vec3 velocityMin;
        glm::vec3 velocityMax;

        if (g > 0.0f)
        {
            tMin = t;
            gMin = g;
            posMin = pos;
            velocityMin = velocity;

            tMax = t;
            gMax = g;
            posMax = pos;
            velocityMax = velocity;

            while (gMax > 0.0f && tMax < tAsegMax - 0.0001f)
            {
                tMax = std::min(tMax + 0.2f, tAsegMax);
                pacp->pvtacp->pfnEvaluateAcp(pacp, palo, tMax, 1.0f, static_cast<GRFEVAL>(0), &posMax, &velocityMax);
                gMax = glm::dot(*ppos - posMax, velocityMax);
            }
        }
        else
        {
            tMax = t;
            gMax = g;
            posMax = pos;
            velocityMax = velocity;

            tMin = t;
            gMin = g;
            posMin = pos;
            velocityMin = velocity;

            while (gMin < 0.0f && tMin > 0.0001f)
            {
                tMin = std::max(tMin - 0.2f, 0.0f);
                pacp->pvtacp->pfnEvaluateAcp(pacp, palo, tMin, 1.0f, static_cast<GRFEVAL>(0), &posMin, &velocityMin);
                gMin = glm::dot(*ppos - posMin, velocityMin);
            }
        }

        while (tMax - tMin > 0.05f)
        {
            const float tMid = (tMin + tMax) * 0.5f;

            glm::vec3 posMid;
            glm::vec3 velocityMid;

            pacp->pvtacp->pfnEvaluateAcp(pacp, palo, tMid, 1.0f, static_cast<GRFEVAL>(0), &posMid, &velocityMid);

            const float gMid = glm::dot(*ppos - posMid, velocityMid);

            if (gMid > 0.0f)
            {
                tMin = tMid;
                gMin = gMid;
                posMin = posMid;
                velocityMin = velocityMid;
            }
            else
            {
                tMax = tMid;
                gMax = gMid;
                posMax = posMid;
                velocityMax = velocityMid;
            }
        }

        const float dg = gMax - gMin;

        if (std::abs(dg) > 0.0001f)
        {
            const float u = std::clamp(gMax / dg, 0.0f, 1.0f);
            t = tMin * u + tMax * (1.0f - u);
            pacp->pvtacp->pfnEvaluateAcp(pacp, palo, t, 1.0f, static_cast<GRFEVAL>(0), &pos, &velocity);
        }
        else if (std::abs(gMin) <= std::abs(gMax))
        {
            t = tMin;
            pos = posMin;
            velocity = velocityMin;
        }
        else
        {
            t = tMax;
            pos = posMax;
            velocity = velocityMax;
        }
    }

    if (ptClosest != nullptr)
        *ptClosest = t;

    if (pposClosest != nullptr)
        *pposClosest = pos;

    if (pvClosest != nullptr)
        *pvClosest = velocity;
}

void*GetAsegaSpeed(ASEGA* pasega)
{
    return &pasega->svtLocal;
}

void SetAsegaSpeed(ASEGA* pasega, float svt)
{
    pasega->svtLocal = svt * pasega->svtMaster * pasega->paseg->svtMasterSuck;

}

void* GetAsegaMasterSpeed(ASEGA* pasega)
{
    return &pasega->svtMaster;
}

void SetAsegaMasterSpeed(ASEGA* pasega, float svtMaster)
{
    float svtBase = pasega->svtLocal;
    const float svtOldScale = pasega->svtMaster * pasega->paseg->svtMasterSuck;

    if (svtOldScale != 0.0f)
        svtBase /= svtOldScale;

    pasega->svtMaster = svtMaster;
    SetAsegaSpeed(pasega, svtBase);
}

void* GetAsegaPriority(ASEGA* pasega)
{
    return &pasega->nPriority;
}

void SetAsegaPriority(ASEGA* pasega, int nPriority)
{
    for (ACTSEG* pactseg = pasega->dlActseg.pactsegFirst; pactseg != nullptr; pactseg = pactseg->dleAsega.pactsegNext)
    {
        pactseg->nPriority = nPriority;
        ResortAloActList(pactseg->palo);
    }
}

void SendAsegaMessage(ASEGA* pasega, MSGID msgid, void* pv)
{
    if (pasega->paloRoot != nullptr)
        pasega->paloRoot->pvtlo->pfnSendLoMessage(pasega->paloRoot, msgid, pv);

    for (MQ* pmq = pasega->pmqFirst.get(); pmq != nullptr;)
    {
        MQ* pmqNext = pmq->pmqNext.get();
        pmq->pfnmq((LO*)pmq->pvContext, msgid, pv);
        pmq = pmqNext;
    }
}

void SubscribeAsegaStruct(ASEGA* pasega, PFNMQ pfnmq, void* pvContext)
{
	SubscribeSwPpmqStruct(pasega->paseg->psw, &pasega->pmqFirst, pfnmq, pvContext);
}

void SubscribeAsegaObject(ASEGA* pasega, LO* ploTarget)
{
	SubscribeSwPpmqStruct(pasega->paseg->psw, &pasega->pmqFirst, (PFNMQ)ploTarget->pvtlo->pfnHandleLoMessage, ploTarget);
}

int GetAsegaSize()
{
	return sizeof(ASEGA);
}

void DeleteAsega(ASEGA* pasega)
{
	delete pasega;
}
