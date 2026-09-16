#include "stepzap.h"
#include <cstdio>
#include "jt.h"
#include "stephide.h"
#include "actval.h"
#include "water.h"
#include "explg.h"
#include "expls.h"
#include "render.h"

void PostJtLoadZap(JT* pjt)
{
    pjt->aspks.assign(std::begin(s_aspks), std::end(s_aspks));
    pjt->cspks = static_cast<int>(pjt->aspks.size());

    for (SPKS& spks : pjt->aspks)
        spks.palo = (ALO*)PloFindSwObject(pjt->psw, 1, spks.oid, pjt);
}

void PresetJtAccelZap(JT* pjt)
{
    switch (pjt->jtbs)
    {
        case 37:
        case 38:
        {
            glm::vec3 dv = -pjt->dvGravity;
            AddSoAcceleration(pjt, &dv);
            AccelSoTowardPosSpring(pjt, nullptr, nullptr, &g_vecZero, &s_clqZapDamping, g_clock.dt);
            break;
        }

        case 41:
        case 48:
        {
            glm::vec3 dv = -pjt->dvGravity;
            AddSoAcceleration(static_cast<SO*>(pjt), &dv);

            glm::vec3 v = g_vecZero;

            if (pjt->vTarget.z != 0.0f)
                v.z = pjt->xf.v.z;

            AccelSoTowardPosSpring(pjt, nullptr, nullptr, &v, &s_clqZapDamping, g_clock.dt);

            if (pjt->vTarget.z != 0.0f)
            {
                dv = glm::vec3(0.0f);
                dv.z = (-pjt->vTarget.z - v.z) / g_clock.dt;
                AddSoAcceleration(pjt, &dv);
            }

            break;
        }
    }

    AccelSoTowardMatSmooth(pjt, g_clock.dt, &pjt->matTarget, &pjt->smpSpin);
}

void UpdateJtZap(JT* pjt)
{
    if (pjt->jtbs == 43)
    {
        const float radVelocity = std::atan2(pjt->xf.v.y, pjt->xf.v.x);
        pjt->radTarget = RadSmooth(pjt->radTarget, radVelocity + 3.1415927f, g_clock.dt, &s_smpJtZapTurn, nullptr);

        if (g_clock.t - pjt->tJtbs <= 0.25f || pjt->fBaseXp == 0)
            return;

        FinishJtZap             (pjt);
        return;
    }

    if (pjt->jtbs != 44)
        return;

    ACTVAL* pactvalJump = pjt->pactvalJump;
    const float dtRemaining = pactvalJump->tMatch - g_clock.t;

    if (dtRemaining < 0.0f)
    {
        FinishJtZap             (pjt);
        return;
    }

    LO* ploTarget = pjt->jtJumpTarget.plo;

    if (ploTarget != nullptr && dtRemaining < 0.25f)
    {
        const bool fCanLand =
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploTarget), CID_HND) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploTarget), CID_PIPE) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploTarget), CID_RAIL) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploTarget), CID_SPIRE) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploTarget), CID_HSHAPE) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploTarget), CID_HPNT);

        if (fCanLand)
        {
            if (pjt->pfnZqe != nullptr)
            {
                FinishJtZap             (pjt);
                return;
            }

            SetJtJts(pjt, 2, 2);
            const int jtbsLanding = JtbsChooseJtLanding(pjt, ploTarget);
            SetJtJts(pjt, 2, jtbsLanding);
            return;
        }
    }

    CalculateJtJumpTrajectory(pjt, dtRemaining, &pjt->jtJumpTarget, nullptr, &pactvalJump->posGoal, nullptr);
}

void SetJtPfnZqe(JT* pjt, PFNZQE pfnZqe)
{
    pjt->pfnZqe = pfnZqe;
}

void FinishJtZap (JT* pjt)
{
    PFNZQE pfnZqe = reinterpret_cast<PFNZQE>(pjt->pfnZqe);

    pjt->pfnZqe = nullptr;
    pjt->grfjtsc &= ~0x10U;

    if (pfnZqe != nullptr)
    {
        pfnZqe(pjt);
        return;
    }

    if (static_cast<unsigned int>(pjt->jtbs - 45) < 5U)
        OnJtDead(pjt);
    else
        SetJtJts(pjt, 2, 2);
}

bool FTakeJtDamage(JT* pjt, ZPR* pzpr)
{
    const bool fInvulnerable = FInvulnerableJt(pjt, pzpr->zpk);

    if (fInvulnerable)
        return 0;

    // Damage while hiding in a basket knocks JT back into the jump state.
    if (pjt->jts == 6 && pjt->jthk == 2)
    {
        ZPR* apzpr[1] = { pzpr };

        HandleLoSpliceEvent(pjt->phbsk, 19, 1, (void**)apzpr);
        SetJtJts(pjt, 2, 5);
        return 0;
    }

    // Save the complete damage request for deferred callbacks.
    pjt->zpr = *pzpr;

    const bool fCanStartDamage =
        pjt->jts != 4 &&
        pjt->jts != 11 &&
        pjt->fFlash == 0 &&
        (g_grfcht & 1) == 0 &&
        !FSwHandsOff(pjt->psw);

    if (fCanStartDamage)
    {
        if (pjt->zpr.zpk == 3)
        {
            if ((GetAvailableVaultFlags() & 0x0100) == 0)
                pjt->czqe = 1;
        }
        else if (pjt->zpr.zpk == 5)
        {
            if ((GetAvailableVaultFlags() & 0x4000) == 0)
                pjt->czqe = 1;
        }
        else
            pjt->czqe = 1;

        if (pjt->czqe && FCharmAvailable())
        {
            pjt->fFlash = 1;
            pjt->tFlashPulse = g_clock.t;
        }
    }

    SetJtPfnZqe(pjt, nullptr);

    switch (pzpr->zpk)
    {
        case 0:
        {
            StartSound(static_cast<SFXID>(163), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

            if (pjt->zpr.pfnzap != nullptr)
            {
                PFNZAP pfnzap = pjt->zpr.pfnzap;
                pjt->zpr.pfnzap = nullptr;
                pfnzap(pjt->zpr.pv, pjt);
            }
            else
                SetJtJts(pjt, 4, 43);

            SetJtPfnZqe(pjt, (PFNZQE)HandleJtDeathOrCharm);
            break;
        }

        case 1:
        case 2:
        {
            if (pjt->jts == 13)
            {
                pjt->vDamagePrev = glm::vec3(0.0f);
                pjt->wDamagePrev = glm::vec3(0.0f);
            }
            else
            {
                pjt->vDamagePrev = pjt->xf.v;
                pjt->wDamagePrev = pjt->xf.w;
            }

            pjt->pvtso->pfnSetSoVelocityVec(pjt, &g_vecZero);
            pjt->pvtso->pfnSetSoAngularVelocityVec(pjt, &g_vecZero);

            const int jtbsDamage = pzpr->zpk == 1 ? 37 : 38;

            SetJtJts(pjt, 4, jtbsDamage);
            SetJtPfnZqe(pjt, (PFNZQE)RestoreJtDamageVelocity);
            break;
        }

        case 3:
        {
            if (pjt->czqe && !FCharmAvailable())
                SetJtJts(pjt, 4, 47);
            else
            {
                SetJtJts(pjt, 4, 39);
                SetJtPfnZqe(pjt, (PFNZQE)UseJtCharm);
            }

            break;
        }

        case 4:
        {
            pjt->pvtso->pfnSetSoVelocityVec(pjt, &s_vJtDamageZpk4);

            if (!FCharmAvailable())
                SetJtJts(pjt, 4, 49);
            else
            {
                SetJtJts(pjt, 4, 40);
                SetJtPfnZqe(pjt, (PFNZQE)UseJtCharm);
            }

            break;
        }

        case 5:
        {
            if (pjt->czqe && !FCharmAvailable())
                SetJtJts(pjt, 4, 48);
            else
            {
                SetJtJts(pjt, 4, 41);
                SetJtPfnZqe(pjt, (PFNZQE)UseJtCharm);
            }

            break;
        }

        case 6:
        {
            if (pjt->zpr.pfnzap != nullptr)
            {
                PFNZAP pfnzap = pjt->zpr.pfnzap;
                pjt->zpr.pfnzap = nullptr;
                pfnzap(pjt->zpr.pv, pjt);
            }
            else
                SetJtJts(pjt, 4, 43);

            if (pjt->xf.v.z > 400.0f)
                StartSound(static_cast<SFXID>(163), nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

            SetJtPfnZqe(pjt, (PFNZQE)StartJtZapSequence);
            break;
        }
    }

    if (pjt->czqe)
    {
        struct JT_DAMAGE_MSG
        {
            JT* pjt;
            LO* ploSource;
        };

        JT_DAMAGE_MSG damageMsg = {
            pjt,
            pzpr->ploSource
        };
        
        pjt->pvtlo->pfnSendLoMessage(pjt, MSGID_damaged, &damageMsg);
    }

    return pjt->czqe;
}

void HandleJtDeathOrCharm(JT* pjt)
{
    LO* ploJumpTarget = pjt->jtJumpTarget.plo;

    const bool fCanLandOnJumpTarget =
        pjt->jtbs == 44 &&
        ploJumpTarget != nullptr &&
        (
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploJumpTarget), CID_PIPE) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploJumpTarget), CID_RAIL) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploJumpTarget), CID_SPIRE) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploJumpTarget), CID_HSHAPE) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploJumpTarget), CID_HPNT) ||
            FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(ploJumpTarget), CID_HND)
            );

    if (!FCharmAvailable())
        SetJtJts(pjt, 4, 45);
    else if (fCanLandOnJumpTarget)
    {
        UsePoCharm(pjt);

        SetJtJts(pjt, 2, 2);

        const int jtbsLanding = JtbsChooseJtLanding(pjt, ploJumpTarget);
        SetJtJts(pjt, 2, jtbsLanding);
    }
    else
    {
        SetJtJts(pjt, 4, 36);
        SetJtPfnZqe(pjt, (PFNZQE)UseJtCharm);
    }

    if (pjt->jts == 4)
    {
        const float vz = pjt->xf.v.z;
        const float volumeUnclamped = s_clqJtZapSoundVolume.g0 + vz * (s_clqJtZapSoundVolume.g1 + vz * s_clqJtZapSoundVolume.g2);
        const float volume = std::clamp(volumeUnclamped, s_lmJtZapSoundVolume.gMin, s_lmJtZapSoundVolume.gMax);

        StartSound(static_cast<SFXID>(73), nullptr, nullptr, nullptr, 0.0f, 0.0f, volume, 0.0, 0.0f, nullptr, nullptr);
    }
}

void StartJtZapSequence(JT* pjt)
{
    StartSound(static_cast<SFXID>(166), nullptr, pjt, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
    SetJtJts(pjt, 4, 42);
    SetJtPfnZqe(pjt, (PFNZQE)HandleJtZapCharmCheck);
}

void HandleJtZapCharmCheck(JT* pjt)
{
    if (FCharmAvailable())
        pjt->pvtjt->pfnUseJtCharm(pjt);
    else
        SetJtJts(pjt, 4, 45);
}

void RestoreJtDamageVelocity(JT* pjt)
{
    pjt->pvtjt->pfnSetSoVelocityVec(pjt, &pjt->vDamagePrev);
    pjt->pvtso->pfnSetSoAngularVelocityVec(pjt, &pjt->wDamagePrev);

    if (!FCharmAvailable())
        SetJtJts(pjt, 4, 45);
    else
        pjt->pvtjt->pfnUseJtCharm(pjt);
}

bool FInvulnerableJt(JT* pjt, int damageKind)
{
    if (damageKind != 3 && damageKind != 5)
    {
        if (pjt->jts == 4)
            return 1;

        if (pjt->jts == 11)
            return 1;

        if (pjt->fFlash != 0)
            return 1;

        if ((g_grfcht & 1U) != 0)
            return 1;

        return FSwHandsOff(pjt->psw) != 0;
    }

    switch (pjt->jts)
    {
        case 2:
        if (pjt->jtbs > 8 && pjt->jtbs < 16)
            return 1;
        break;

        case 3:
        case 7:
        case 8:
        return 1;

        case 4:
        switch (pjt->jtbs)
        {
            case 39:
            case 41:
            case 44:
            case 47:
            case 48:
            return 1;

            default:
            return 0;
        }

        case 6:
        return pjt->jthk != 3 && pjt->jthk != 6;

        case 11:
        return 1;
    }

    return FSwHandsOff(pjt->psw) != 0;
}

void OnJtDead(JT *pjt)
{
    SetAloEyesClosed(pjt, 1.0f);
    HandlePlayerDeath();
}

void DrawJtHaloElectric(RPL* prpl)
{
    if (prpl == nullptr || prpl->palo == nullptr)
        return;

    JT* pjt = static_cast<JT*>(prpl->palo);

    static SPKD spkdElectric = []
    {
        SPKD spkd{};
        spkd.spkk = SPKK_Radial;
        spkd.uRepeat = 5.0f;
        spkd.svu = 1.0f;

        spkd.aspkr[0].clq = { 0.0f, 0.0f, 0.0f };
        spkd.aspkr[0].v = 0.0f;
        spkd.aspkr[0].dz = 0.0f;
        spkd.aspkr[0].rgba = glm::vec4(0.0f);

        spkd.aspkr[1].clq = { 0.0f, 1.0f, 0.0f };
        spkd.aspkr[1].v = 0.5f;
        spkd.aspkr[1].dz = -100.0f;
        spkd.aspkr[1].rgba = glm::vec4(1.0f);

        spkd.aspkr[2].clq = { 80.0f, 1.0f, 0.0f };
        spkd.aspkr[2].v = 1.0f;
        spkd.aspkr[2].dz = 0.0f;
        spkd.aspkr[2].rgba = glm::vec4(1.0f);
        return spkd;
    }();

    spkdElectric.apshd[0] = nullptr;
    spkdElectric.apshd[1] = PshdFindShader((OID)348);

    const int cspks = std::min(pjt->cspks, static_cast<int>(pjt->aspks.size()));
    DrawHaloSpks(&pjt->xf.posWorld, cspks, pjt->aspks.data(), &spkdElectric, prpl);
}

void DrawJtHaloFire(RPL* prpl)
{
    if (prpl == nullptr || prpl->palo == nullptr)
        return;

    JT* pjt = static_cast<JT*>(prpl->palo);

    static SPKD spkdFire = []
    {
        SPKD spkd{};
        spkd.spkk = SPKK_Mirror;
        spkd.uRepeat = 5.0f;
        spkd.svu = 0.5f;

        spkd.aspkr[0].clq = { 0.0f, 0.0f, 0.0f };
        spkd.aspkr[0].v = 0.0f;
        spkd.aspkr[0].dz = 100.0f;
        spkd.aspkr[0].rgba = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);

        spkd.aspkr[1].clq = { 0.0f, 0.5f, 0.0f };
        spkd.aspkr[1].v = 0.0f;
        spkd.aspkr[1].dz = 50.0f;
        spkd.aspkr[1].rgba = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);

        spkd.aspkr[2].clq = { 80.0f, 1.0f, 0.0f };
        spkd.aspkr[2].v = 1.0f;
        spkd.aspkr[2].dz = -100.0f;
        spkd.aspkr[2].rgba = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);
        return spkd;
    }();

    SHD* pshdFireRim = PshdFindShader((OID)349);
    spkdFire.apshd[0] = pshdFireRim;
    spkdFire.apshd[1] = pshdFireRim;

    const int cspks = std::min(pjt->cspks, static_cast<int>(pjt->aspks.size()));
    DrawHaloSpks(&pjt->xf.posWorld, cspks, pjt->aspks.data(), &spkdFire, prpl);
}

void DrawHaloSpks(glm::vec3* pposRoot, int cspks, SPKS* aspks, SPKD* pspkd, RPL* prpl)
{
    if (pposRoot == nullptr || aspks == nullptr || pspkd == nullptr || prpl == nullptr || g_pcm == nullptr || cspks <= 0)
        return;

    struct HaloNode
    {
        glm::vec3 screen;
        float angle;
    };

    constexpr int maxNodes = 256;
    constexpr float haloMaxAngle = glm::pi<float>() * 0.25f;
    constexpr float halfScreenWidth = 320.0f;
    constexpr float halfScreenHeight = 224.0f;
    constexpr glm::vec2 sampleOffsets[7] =
    {
        { 1.0f, 0.0f }, { 0.5f, 0.8660254f }, { -0.5f, 0.8660254f },
        { -1.0f, 0.0f }, { -0.5f, -0.8660254f }, { 0.5f, -0.8660254f },
        { 0.0f, 0.0f }
    };

    glm::vec3 rootScreen;
    ConvertCmWorldToScreen(g_pcm, pposRoot, &rootScreen);
    if (!std::isfinite(rootScreen.z) || std::abs(rootScreen.z) < 0.0001f)
        return;

    rootScreen.x *= halfScreenWidth;
    rootScreen.y *= halfScreenHeight;

    std::vector<HaloNode> nodes;
    nodes.reserve(std::min(maxNodes, cspks * 7));

    for (int i = 0; i < cspks && static_cast<int>(nodes.size()) < maxNodes; ++i)
    {
        if (aspks[i].palo == nullptr)
            continue;

        for (const glm::vec2& offset : sampleOffsets)
        {
            glm::vec3 world = aspks[i].palo->xf.posWorld;
            world += g_pcm->mat[1] * (offset.x * aspks[i].sRadius);
            world += g_pcm->mat[2] * (offset.y * aspks[i].sRadius);

            glm::vec3 screen;
            ConvertCmWorldToScreen(g_pcm, &world, &screen);
            if (!std::isfinite(screen.x) || !std::isfinite(screen.y) || !std::isfinite(screen.z))
                continue;
            screen.x *= halfScreenWidth;
            screen.y *= halfScreenHeight;

            const glm::vec2 delta(screen.x - rootScreen.x, screen.y - rootScreen.y);
            if (glm::dot(delta, delta) < 0.000001f)
                continue;

            nodes.push_back({ screen, std::atan2(delta.y, delta.x) });
            if (static_cast<int>(nodes.size()) == maxNodes)
                break;
        }
    }

    if (nodes.size() < 3)
        return;

    // Retail starts at the leftmost sample and performs a Graham-style scan.
    // Build the same screen-space convex silhouette before deriving angles.
    std::sort(nodes.begin(), nodes.end(), [](const HaloNode& a, const HaloNode& b)
    {
        return a.screen.x < b.screen.x ||
            (a.screen.x == b.screen.x && a.screen.y < b.screen.y);
    });

    const auto cross = [](const HaloNode& a, const HaloNode& b, const HaloNode& c)
    {
        const glm::vec2 ab(b.screen.x - a.screen.x, b.screen.y - a.screen.y);
        const glm::vec2 ac(c.screen.x - a.screen.x, c.screen.y - a.screen.y);
        return ab.x * ac.y - ab.y * ac.x;
    };

    std::vector<HaloNode> outline;
    outline.reserve(maxNodes);
    for (const HaloNode& node : nodes)
    {
        while (outline.size() >= 2 && cross(outline[outline.size() - 2], outline.back(), node) <= 0.0f)
            outline.pop_back();
        outline.push_back(node);
    }

    const size_t lowerCount = outline.size();
    for (size_t i = nodes.size() - 1; i-- > 0;)
    {
        const HaloNode& node = nodes[i];
        while (outline.size() > lowerCount && cross(outline[outline.size() - 2], outline.back(), node) <= 0.0f)
            outline.pop_back();

        outline.push_back(node);
    }

    if (!outline.empty())
        outline.pop_back();

    if (outline.size() < 3)
        return;

    glm::vec3 posMin = outline[0].screen;
    glm::vec3 posMax = outline[0].screen;
    for (const HaloNode& node : outline)
    {
        posMin = glm::min(posMin, node.screen);
        posMax = glm::max(posMax, node.screen);
    }
    rootScreen = (posMin + posMax) * 0.5f;

    for (HaloNode& node : outline)
        node.angle = std::atan2(node.screen.y - rootScreen.y, node.screen.x - rootScreen.x);

    std::sort(outline.begin(), outline.end(), [](const HaloNode& a, const HaloNode& b)
    {
        return a.angle < b.angle;
    });

    std::vector<HaloNode> smoothOutline;
    smoothOutline.reserve(maxNodes);
    for (size_t i = 0; i < outline.size() && smoothOutline.size() < maxNodes; ++i)
    {
        const HaloNode& current = outline[i];
        const HaloNode& next = outline[(i + 1) % outline.size()];
        smoothOutline.push_back(current);

        float delta = next.angle - current.angle;
        if (i + 1 == outline.size())
            delta += glm::two_pi<float>();

        const int inserts = std::min(static_cast<int>(std::abs(delta) / haloMaxAngle),
            maxNodes - static_cast<int>(smoothOutline.size()));
        for (int j = 1; j <= inserts; ++j)
        {
            const float u = static_cast<float>(j) / static_cast<float>(inserts + 1);
            const glm::vec3 screen = glm::mix(current.screen, next.screen, u);
            smoothOutline.push_back({ screen, current.angle + delta * u });
        }
    }
    outline.swap(smoothOutline);

    glGlobShader.Use();
    AppendStream(&ropStream, &prpl->ro, sizeof(ROGL), sizeof(ROGL));
    glUniform1i(glslRko, 0);
    glUniform1i(glslfAlphaTest, 0);
    glUniform1i(glslfAnimateUv, 0);
    glUniform1i(glslfSkin, 0);
    glUniform1i(glslfPose, 0);
    glUniform1f(glslUnSelfIllum, 1.0f);

    const GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
    GLboolean depthWriteWasEnabled = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteWasEnabled);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glBindVertexArray(g_gl.sqtrVao);
    glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, g_gl.sqtrVbo);

    for (int pass = 0; pass < 2; ++pass)
    {
        SHD* pshd = pspkd->apshd[pass];
        if (pshd == nullptr || pshd->atex.empty() || pshd->atex[0].abmp.empty())
            continue;

        TEX& tex = pshd->atex[0];
        BMP* pbmp = tex.abmp[0];
        const GLuint64 handle = !tex.hDiffuseMap.empty() && tex.hDiffuseMap[0] != 0
            ? tex.hDiffuseMap[0] : (pbmp != nullptr ? pbmp->hDiffuseMap : 0);
        if (handle == 0)
            continue;

        // Retail emits one strip between two adjacent SPKR rings per shader:
        // pass 0 is ring 0 -> 1, and pass 1 is ring 1 -> 2.
        std::vector<SQTRGPU> vertices;
        vertices.reserve((outline.size() + 1) * 2);
        const float uvScroll = GModPositive(g_clock.t * pspkd->svu, 1.0f);
        float previousAngle = -glm::pi<float>();

        for (size_t i = 0; i <= outline.size(); ++i)
        {
            const HaloNode& node = outline[i % outline.size()];
            float angle = node.angle;
            while (angle < previousAngle)
                angle += glm::two_pi<float>();
            previousAngle = angle;

            float u;
            if (pspkd->spkk == SPKK_Mirror)
                u = uvScroll + pspkd->uRepeat * std::abs(RadNormalize(angle - glm::half_pi<float>())) /
                glm::two_pi<float>();
            else
                u = uvScroll + pspkd->uRepeat * angle / glm::two_pi<float>();

            const glm::vec3 delta = node.screen - rootScreen;
            const float distance = glm::length(glm::vec2(delta.x, delta.y));
            if (distance < 0.0001f)
                continue;

            for (int iring = pass; iring <= pass + 1; ++iring)
            {
                const SPKR& ring = pspkd->aspkr[iring];
                const float radius = ring.clq.g0 + distance *
                    (ring.clq.g1 + distance * ring.clq.g2);
                glm::vec3 screen = rootScreen + delta * (radius / distance);
                screen.z += ring.dz;

                screen.x /= halfScreenWidth;
                screen.y /= halfScreenHeight;
                glm::vec3 world;
                ConvertCmScreenToWorld(g_pcm, &screen, &world);
                vertices.push_back({ world, glm::vec2(u, ring.v), ring.rgba * prpl->ro.uAlpha });
            }
        }

        if (vertices.size() < 6)
            continue;

        glUniformHandleui64ARB(glslDiffuseMap, handle);
        const GLsizeiptr size = static_cast<GLsizeiptr>(vertices.size() * sizeof(SQTRGPU));
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices.data());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(vertices.size()));
    }

    glBindVertexArray(0);
    glDepthMask(depthWriteWasEnabled);
    if (cullWasEnabled)
        glEnable(GL_CULL_FACE);
}

void RenderJtSelfZap(JT* pjt, CM* pcm, RO* pro)
{
    (void)pro;

    if (pjt->jtbs > JTBS_Zap_Blunt && pjt->jtbs < JTBS_Zap_Water)
    {
        RPL rpl{};

        const glm::vec3 dposCamera = pjt->xf.posWorld - pcm->pos;

        rpl.rp = RP_Translucent;
        rpl.z = glm::dot(dposCamera, pcm->mat[0]);
        if (pjt->jtbs == JTBS_Zap_Electric)
        {
            rpl.PFNDRAWRPL = DrawJtHaloElectric;
            rpl.palo = pjt;
            rpl.ro.model = glm::mat4(1.0f);
            rpl.ro.uAlpha = pro != nullptr ? pro->uAlpha : 1.0f;
            rpl.ro.darken = 1.0f;
            rpl.ro.warpType = WARP_NONE;
            // SubmitRpl assigns DrawGlob to ordinary translucent entries.
            // This is a procedural halo, so retain its own draw callback.
            if (g_translucentAddCount < static_cast<int>(g_translucentAddPrpl.size()))
                g_translucentAddPrpl[g_translucentAddCount++] = rpl;
        }
        else
        {
            rpl.PFNDRAWRPL = DrawJtHaloFire;
            rpl.palo = pjt;
            rpl.ro.model = glm::mat4(1.0f);
            rpl.ro.uAlpha = pro != nullptr ? pro->uAlpha : 1.0f;
            rpl.ro.darken = 1.0f;
            rpl.ro.warpType = WARP_NONE;

            if (g_translucentAddCount < static_cast<int>(g_translucentAddPrpl.size()))
                g_translucentAddPrpl[g_translucentAddCount++] = rpl;
        }
    }

    glm::vec4 colorOverride = g_rgbaCel;

    if (pjt->fFlash != 0)
    {
        const float uZap = pjt->jts == JTS_Zap ? 0.0f : (g_clock.t - pjt->tCharm) * 0.5f;
        const float uFlash = std::cos((g_clock.t - pjt->tFlashPulse) * 10.0f);

        const float uZapCelUnclamped = s_clqUZapToUCel.g0 + uZap * (s_clqUZapToUCel.g1 + uZap * s_clqUZapToUCel.g2);
        const float uFlashCelUnclamped = s_clqUFlashToUCel.g0 + uFlash * (s_clqUFlashToUCel.g1 + uFlash * s_clqUFlashToUCel.g2);

        const float uZapCel = std::clamp(uZapCelUnclamped, 0.0f, 1.0f);
        const float uFlashCel = std::clamp(uFlashCelUnclamped, 0.0f, 1.0f);
        const float uCel = uZapCel * uFlashCel;

        const glm::vec4 colorFlash(160.0f / 255.0f, 0.0f, 0.0f, 128.0f / 255.0f);

        colorOverride = glm::mix(g_rgbaCel, colorFlash, uCel);

        // The original only blends RGB and preserves g_rgbaCel's alpha.
        colorOverride.a = g_rgbaCel.a;
    }

    SetAloOverrideCel(pjt, &colorOverride);
}

JTHS JthsCurrentJt(JT* pjt)
{
    if (pjt->jts != JTS_Zap)
        return pjt->fFlash ? JTHS_Damaged : JTHS_Normal;

    if (pjt->jtbs < 45)
        return JTHS_Damaged;

    return JTHS_Dead;
}

void UseJtCharm(JT* pjt)
{
    if (pjt->czqe != 0)
    {
        UsePoCharm(pjt);
        pjt->czqe = 0;
    }

    PFNZAP pfnzap = pjt->zpr.pfnzap;

    if (pfnzap == nullptr)
    {
        glm::vec3 velocity = pjt->xf.v;
        velocity.z = std::max(velocity.z, VZ_JtCharmJump);

        pjt->pvtso->pfnSetSoVelocityVec(pjt, &velocity);
        SetJtJts(pjt, JTS_Jump, JTBS_Jump_Thrown);
    }
    else
    {
        pjt->zpr.pfnzap = nullptr;
        pfnzap(pjt->zpr.pv, pjt);
    }

    pjt->fCanBoost = 0;
    pjt->svxyAirSteer = 0.0f;
}

void UpdateJtWater(JT* pjt)
{
    WATER* pwater = pjt->pwater;

    if (pwater == nullptr || pwater->fZap == 0)
        return;

    glm::vec3 posSearch = pjt->xf.posWorld;
    posSearch.z += DZ_JtWaterSearchFloat;

    if (pjt->jtbs == JTBS_Zap_Water || pjt->jtbs == JTBS_Zap_DeadInWater)
        posSearch.z -= 10.0f;

    WRBG* pwrbg = pwater->globset.pwrbgFirst.get();

    if (pwrbg != nullptr)
    {
        glm::vec3 posWarp;
        WarpWrTransform(pwrbg->pwr, 50.0f, &posSearch, nullptr, &posWarp, nullptr, nullptr);
        posSearch = posSearch * 2.0f - posWarp;
    }

    BSP* pbspWater = PbspPointInBspQuick(&posSearch, pwater->bspc.absp.data());

    if (pbspWater == nullptr)
        return;

    if (pjt->jts != JTS_Jump || pjt->xf.v.z < 0.0f)
    {
        ZPR zpr;
        InitZpr(&zpr, ZPK_Water, reinterpret_cast<LO*>(pwater));

        if (pwater->pvtso->pfnFInflictSoZap(pwater, nullptr, &zpr))
            pjt->pvtpo->pfnFTakePoDamage(pjt, &zpr);
    }

    glm::vec3 posWaterSurface;

    if (!FFindJtWaterSurface(pjt, pwater, &posWaterSurface))
        return;

    for (int i = 0; i < 8; ++i)
    {
        JTWATERCONTACT* pcontact = &pjt->jtWaterContact[i];
        EXPL* pexplContact = pcontact->pexpl;

        if (pexplContact == nullptr)
            continue;

        if (g_clock.t - pcontact->tContact < 0.05f)
            continue;

        EXPL* apexpl[8];
        int cexpl;

        if (FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(pexplContact), CID_EXPLG))
        {
            EXPLG* pexplg = reinterpret_cast<EXPLG*>(pexplContact);

            cexpl = std::min(pexplg->cpexpl, 8);
            std::copy_n(pexplg->apexpl, cexpl, apexpl);
        }
        else
        {
            apexpl[0] = pexplContact;
            cexpl = 1;
        }

        for (int j = 0; j < cexpl; ++j)
        {
            EXPL* pexpl = apexpl[j];

            if (pexpl == nullptr)
                continue;

            glm::vec3 posExpl;
            GetXfmPos(reinterpret_cast<XFM*>(pexpl), &posExpl);

            glm::vec3 posExplNext = pexpl->posLocal;
            PredictAloTransformAdjust(pexplContact->paloParent, nullptr, g_clock.dt, &posExplNext, nullptr, nullptr, nullptr);

            bool fCrossedWaterSurface;

            if (posWaterSurface.z <= posExpl.z)
                fCrossedWaterSurface = posExplNext.z <= posWaterSurface.z;
            else
                fCrossedWaterSurface = posWaterSurface.z <= posExplNext.z;

            if (!fCrossedWaterSurface)
                continue;

            EXPLSO explso = {};
            explso.posOrigin = glm::vec3(posExpl.x, posExpl.y, posWaterSurface.z);
            explso.grfexplso = 4;

            if (FIsBasicDerivedFrom(pexpl, CID_EXPLS))
            {
                EXPLS* pexpls = reinterpret_cast<EXPLS*>(pexpl);
                EMITB* pemitb = pexpls->pemitb.get();

                if (pemitb != nullptr && pemitb->emitp.emitpk == EMITPK_Rip && pemitb->emitp.emitrip.ript == RIPT_Droplet)
                {
                    explso.psoTouch = reinterpret_cast<SO*>(pwater);
                    explso.grfexplso = 36;
                }
            }

            pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
            pcontact->tContact = g_clock.t;
        }
    }
}

bool FFindJtWaterSurface(JT* pjt, WATER* pwater, glm::vec3* ppos)
{
    glm::vec3 dposWarp(0.0f);

    WRBG* pwrbg = pwater->globset.pwrbgFirst.get();

    if (pwrbg != nullptr)
    {
        glm::vec3 posWarp;
        WarpWrTransform(pwrbg->pwr, DZ_JtWaterSearchFloat, &pjt->xf.posWorld, nullptr, &posWarp, nullptr, nullptr);
        dposWarp = posWarp - pjt->xf.posWorld;
    }

    glm::vec3 posBase = pjt->xf.posWorld - dposWarp;
    glm::vec3 posMin = posBase;
    glm::vec3 posMax = posBase;

    posMin.z += DZ_JtWaterSearchMin;
    posMax.z += DZ_JtWaterSearchMax;

    LSG lsg = {};

    if (!ClsgClipEdgeToBsp(pwater->bspc.absp.data(), &posMax, &posMin, nullptr, 1, &lsg))
        return 0;

    *ppos = lsg.apos[0] + dposWarp;
    return 1;
}

void AddJtWaterAcceleration(JT* pjt, WATER* pwater, float dt)
{
    int jtbs = pjt->jtbs;

    // States 6 and 43 only receive normal water acceleration while falling.
    if (jtbs == 6 || jtbs == 43)
    {
        if (pjt->xf.v.z >= 0.0f)
            return;

        AddSoWaterAcceleration(pjt, pwater, dt);
        return;
    }

    // All states other than 39 and 47 use the normal SO water behavior.
    if (jtbs != 39 && jtbs != 47)
    {
        AddSoWaterAcceleration(pjt, pwater, dt);
        return;
    }

    glm::vec3 posWaterSurface;

    if (!FFindJtWaterSurface(pjt, pwater, &posWaterSurface))
        return;

    glm::vec3 vTargetWorld = pjt->matTarget * pjt->vTarget;

    glm::vec3 vWater;
    glm::vec3 wWater;
    CalculateWaterCurrent(pwater, &posWaterSurface, &vWater, &wWater);

    glm::vec3 vDesired = pjt->xf.v + vTargetWorld;
    glm::vec3 dv = (vDesired - vWater) * R_JtWaterDamping;

    float vz;
    GSmoothA(pjt->xf.posWorld.z + DZ_JtWaterSearchFloat, pjt->xf.v.z - vWater.z, posWaterSurface.z, dt, &s_smpaJtFloat, &vz);

    float azFloat = (vz - pjt->xf.v.z) / dt - pjt->dvGravity.z;

    if (dv.z < azFloat)
        dv.z = azFloat;

    AddSoAcceleration(pjt, &dv);
}

void InitZpr(ZPR* pzpr, ZPK zpk, LO* ploSource)
{
    memset(pzpr, 0, sizeof(*pzpr));

    pzpr->pv = ploSource;
    pzpr->zpk = zpk;
    pzpr->ploSource = ploSource;
}
