#include "steppower.h"
#include "jt.h"
#include "credit.h"
#include "decoy.h"
#include "stepguard.h"
#include "binoc.h"
#include "mine.h"
#include "credit.h"

void SetFsp(FSP fsp)
{
    if (g_pjt != nullptr)
    {
        if (g_fsp == FSP_Ball)
        {
            if (g_pjt->jts == 13)
                SetJtJts(g_pjt, 0, -1);
        }
        else if (g_fsp == FSP_Mine)
        {
            if (g_pjt->pmine != nullptr && FIsLoInWorld(g_pjt->pmine))
                FadeAloOut(g_pjt->pmine, 0.5f);
        }

        g_rtClockPowerUp = 1.0f;

        if (g_pjt->pdecoy != nullptr && FIsLoInWorld(g_pjt->pdecoy))
            DamageJtDecoy(g_pjt->pdecoy, g_pjt, 1);

        SetJtStun(g_pjt, 0);

        if (g_pjt == PpoCur())
            g_fsp = fsp;
    }

    if (fsp == g_pgsCur->nPowerupLast)
        return;

    if (static_cast<unsigned int>(fsp) >= 7)
        return;

    const bool fCurrentPlayer = g_pjt != nullptr && g_pjt == PpoCur();
    const float dtVisible = fCurrentPlayer ? 3.0f : 8.0f;

    SetBlotDtVisible(&g_note, dtVisible);
    SetBlotFontScale(&g_note, 0.6f);

    g_note.pvtnote->pfnSetBlotAchzDraw(&g_note, (char*)s_mpfspszPowerUp[fsp]);
    g_note.pvtblot->pfnShowBlot(&g_note);

    g_pgsCur->nPowerupLast = fsp;
}

void UpdateJtActivePowerUp(JT* pjt, JOY* pjoy)
{
    if (pjt == nullptr || pjoy == nullptr || g_pgsCur == nullptr)
        return;

    if (g_fsp > FSP_Nil && (s_agrfvaultFsp[g_fsp] & GetAvailableVaultFlags()) == 0)
    {
        SetFsp(FSP_Nil);
        g_pgsCur->nPowerupLast = FSP_Nil;
    }

    bool fCanRotatePowerUp = false;

    switch (g_fsp)
    {
        case FSP_Dive:
        fCanRotatePowerUp = g_pjt == nullptr || (g_pjt->jts != 10 && !(g_pjt->jts == 2 && g_pjt->jtbs == 8));
        break;

        case FSP_Ball:
        fCanRotatePowerUp = g_pjt == nullptr || g_pjt->jts != 13;
        break;

        case FSP_SlowDownClock:
        case FSP_SpeedUpClock:
        fCanRotatePowerUp = FFloatsNear(g_rtClockPowerUp, 1.0f, 0.0001f);
        break;

        case FSP_Mine:
        fCanRotatePowerUp = g_pjt != nullptr && g_pjt->jts != 16;
        break;

        case FSP_Decoy:
        fCanRotatePowerUp = g_pjt != nullptr && g_pjt->jts != 14;
        break;

        case FSP_Stun:
        fCanRotatePowerUp = g_pjt != nullptr && g_pjt->jts != 15 && pjt->fStun == 0;
        break;

        case FSP_Nil:
        fCanRotatePowerUp = true;
        break;

        default:
        break;
    }

    if (g_pjt != nullptr && g_pjt->jts == 4)
        fCanRotatePowerUp = false;

    if (g_binoc.blots != 0)
        fCanRotatePowerUp = false;

    if (fCanRotatePowerUp && (pjoy->IsPressed(BTN_L2) || pjoy->IsPressed(BTN_R2)))
    {
        const FSP fsp = static_cast<FSP>(IRotatePowerUp(pjoy, g_fsp, FSP_Max, s_agrfvaultFsp));
        g_pgsCur->nPowerupLast = FSP_Nil;
        SetFsp(fsp);
    }

    switch (g_fsp)
    {
        case FSP_Ball:
        if (static_cast<unsigned int>(pjt->jts) < 3 && pjoy->IsPressed(BTN_TRIANGLE))
        {
            pjoy->SetHandled(BTN_TRIANGLE);
            SetJtJts(pjt, 13, pjt->fBaseXp == 0 ? 54 : 55);
        }
        break;

        case FSP_SlowDownClock:
        {
            if ((g_grfusr & 3) != 0 || g_binoc.binocs == BINOCS_Instruct)
            {
                pjt->fSlowDownClock = 0;
            }
            else if (pjt->jts == 2)
            {
                if (pjoy->IsPressed(BTN_TRIANGLE))
                {
                    pjoy->SetHandled(BTN_TRIANGLE);
                    pjt->fSlowDownClock = 1;
                }
            }
            else if (!pjoy->IsHeld(BTN_TRIANGLE))
            {
                pjt->fSlowDownClock = 0;
            }
            else
            {
                pjt->fSlowDownClock = (GetAvailableVaultFlags() & 0x1000) != 0;
            }

            g_rtClockPowerUp = pjt->fSlowDownClock != 0 ? RT_JtClockSlowDown : 1.0f;
            break;
        }

        case FSP_Mine:
        if (static_cast<unsigned int>(pjt->jts) < 2 && pjoy->IsPressed(BTN_TRIANGLE) && pjt->pmine != nullptr)
        {
            pjoy->SetHandled(BTN_TRIANGLE);

            if (!FIsLoInWorld(pjt->pmine))
            {
                SetJtJts(pjt, 16, 57);
            }
            else
            {
                const glm::vec3 dposMine = pjt->pmine->xf.posWorld - pjt->xf.posWorld;
                pjt->radTarget = std::atan2(dposMine.y, dposMine.x);
                FixStepAngularVelocity(pjt);
                SetJtJts(pjt, 16, 58);
            }
        }
        break;

        case FSP_SpeedUpClock:
        if (g_binoc.binocs == BINOCS_Instruct)
            pjt->uSpeedUpClock = 0.0f;
        else if (!pjoy->IsHeld(BTN_TRIANGLE))
            pjt->uSpeedUpClock = GSmooth(pjt->uSpeedUpClock, 0.0f, g_clock.dtReal, &s_smpSpeedUpClock, nullptr);
        else
            pjt->uSpeedUpClock = 1.0f;

        g_rtClockPowerUp = 1.0f + pjt->uSpeedUpClock * (RT_JtClockSpeedUp - 1.0f);
        break;

        case FSP_Decoy:
        if (static_cast<unsigned int>(pjt->jts) < 2 && pjoy->IsPressed(BTN_TRIANGLE) && pjt->pdecoy != nullptr)
        {
            pjoy->SetHandled(BTN_TRIANGLE);
            SetJtJts(pjt, 14, -1);
        }
        break;

        case FSP_Stun:
        if (static_cast<unsigned int>(pjt->jts) < 2 && pjoy->IsPressed(BTN_TRIANGLE))
        {
            pjoy->SetHandled(BTN_TRIANGLE);
            SetJtJts(pjt, 15, -1);
        }

        if (pjt->fStun != 0 && g_clock.t - pjt->tStun > 3.0f)
            SetJtStun(pjt, 0);
        break;

        default:
        break;
    }
}

int IRotatePowerUp(JOY* pjoy, int iCur, int iMax, GRFVAULT* mpigrfvault)
{
    if (pjoy == nullptr || mpigrfvault == nullptr || iMax <= 0)
        return iCur;

    const bool fPreviousPressed = pjoy->IsPressed(BTN_L2);
    const bool fNextPressed = pjoy->IsPressed(BTN_R2);

    if (!fPreviousPressed && !fNextPressed)
        return iCur;

    const int di = fPreviousPressed ? -1 : 1;
    int iResult = iCur;

    for (int i = 0; i < iMax; ++i)
    {
        iResult += di;

        if (iResult < 0)
            iResult = iMax - 1;
        else if (iResult >= iMax)
            iResult = 0;

        if ((GetAvailableVaultFlags() & mpigrfvault[iResult]) != 0)
            return iResult;
    }

    return iCur;
}

void SetJtStun(JT* pjt, int fStun)
{
    if (pjt->fStun == fStun)
        return;

    STEPGUARD* apstepguard[256];
    const int cstepguard = CploFindSwObjectsByClass(pjt->psw, 261, CID_STEPGUARD, nullptr, 256, reinterpret_cast<LO**>(apstepguard));
    const float svtLocal = fStun != 0 ? 0.0f : 1.0f;

    for (int istepguard = 0; istepguard < cstepguard; ++istepguard)
    {
        STEPGUARD* pstepguard = apstepguard[istepguard];

        if (pstepguard->pasegaSgs != nullptr)
            SeekAsega(pstepguard->pasegaSgs, SEEK_Start, 0.0f, svtLocal);
    }

    pjt->fStun = fStun;
    pjt->tStun = g_clock.t;
}

FSP g_fsp = FSP_Nil;
