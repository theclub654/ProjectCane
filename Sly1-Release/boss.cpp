#include "boss.h"
#include "gui_layout.h"
#include "gl.h"
#include "render.h"

void StartupBoss(BOSS* pboss)
{
	pboss->pvtboss = &g_vtboss;
}

void PostBossLoad(BOSS* pboss)
{
    PostBlotLoad(pboss);

    pboss->pfont = PfontFromFont(4);
    pboss->rFontScale = 1.0f;
    pboss->uHealthDisplay = 0.0f;
    pboss->uHealthTarget = 1.0f;

    switch (g_pgsCur->gameWorldCur)
    {
        case GAMEWORLD_Underwater:
        pboss->cPhaseMax = 4;
        pboss->cHealthPerPhase = 1;
        break;

        case GAMEWORLD_Muggshot:
        pboss->cPhaseMax = 3;
        pboss->cHealthPerPhase = 1;
        pboss->uHealthDisplay = 1.0f;
        break;

        case GAMEWORLD_Voodoo:
        pboss->cPhaseMax = 4;
        pboss->cHealthPerPhase = 1;
        pboss->uHealthDisplay = 1.0f;
        break;

        case GAMEWORLD_Snow:
        pboss->cPhaseMax = 5;
        pboss->cHealthPerPhase = 10;
        break;

        case GAMEWORLD_Clockwerk:
        pboss->cPhaseMax = 4;
        pboss->cHealthPerPhase = 5;
        break;

        default:
        return;
    }

    const float scale = pboss->rFontScale;

    pboss->xHealthBar  = 18.0f * scale;
    pboss->yHealthBar  = 8.0f * scale;
    pboss->dxHealthBar = 26.0f * scale;
    pboss->dyHealthBar = 340.0f * scale;

    pboss->pfont->PushScaling(scale, scale);

    const float dyFont = static_cast<float>(pboss->pfont->m_dyUnscaled) * pboss->pfont->m_ryScale;
    const float dxPhaseGlyph = pboss->pfont->DxFromCh('z');
    constexpr float sSpacing = 5.0f;

    ResizeBlot(pboss, dyFont, dxPhaseGlyph + dyFont + sSpacing);

    pboss->pfont->PopScaling();

    pboss->tHealthChanged = 0.0f;
    pboss->cPhasesRemaining = pboss->cPhaseMax;
    pboss->cHealthCurrent = pboss->cHealthPerPhase;
    pboss->cHealthTotal = pboss->cPhaseMax * pboss->cHealthPerPhase;

    pboss->xPhaseCount = static_cast<int>(dyFont * 0.5f);
    pboss->yPhaseCount = static_cast<int>(dxPhaseGlyph + sSpacing + dyFont * 0.5f);
}

void DecrementBossHealth(BOSS* pboss)
{
    --pboss->cHealthCurrent;

    if (pboss->cHealthCurrent <= 0)
    {
        --pboss->cPhasesRemaining;

        if (pboss->cPhasesRemaining <= 0)
        {
            pboss->cPhasesRemaining = 0;
            pboss->cHealthCurrent = 0;
        }
        else
        {
            pboss->cHealthCurrent = pboss->cHealthPerPhase;
        }
    }

    const int cCompletedPhases = std::max(pboss->cPhasesRemaining - 1, 0);
    const int cHealthRemaining = cCompletedPhases * pboss->cHealthPerPhase + pboss->cHealthCurrent;

    pboss->uHealthTarget = static_cast<float>(cHealthRemaining) / static_cast<float>(pboss->cHealthTotal);
    pboss->tHealthChanged = g_clock.t;
}

void DrawBoss(BOSS* pboss)
{
    const GuiScale guiScale = GetGuiScale();
    const float uniformScale = std::min(guiScale.x, guiScale.y);
    const float scaleX = uniformScale;
    const float scaleY = uniformScale;

    const float fontScaleX = pboss->rFontScale * scaleX;
    const float fontScaleY = pboss->rFontScale * scaleY;

    const float dxBoss = pboss->dx * scaleX;
    const float dyBossBlot = pboss->dy * scaleY;
    float xBoss = pboss->x;
    float yBoss = pboss->y;

    // Expand the complete boss HUD around its original blot peg. The bar and
    // glyph still share one origin and one uniform scale.
    if (pboss->pbloti != nullptr)
    {
        if (pboss->pbloti->x < 0.0f)
            xBoss -= dxBoss - pboss->dx;
        else if (pboss->pbloti->x == 0.0f)
            xBoss -= (dxBoss - pboss->dx) * 0.5f;

        if (pboss->pbloti->y < 0.0f)
            yBoss -= dyBossBlot - pboss->dy;
        else if (pboss->pbloti->y == 0.0f)
            yBoss -= (dyBossBlot - pboss->dy) * 0.5f;
    }

    if (FModernGui())
    {
        // The blot dimensions describe the boss icon/count, but the vertical
        // health bar extends farther. Center the complete modern HUD on the
        // right side without changing its existing horizontal placement.
        const float dyHealthBounds =
            (pboss->yHealthBar + pboss->dyHealthBar) * scaleY;
        const float dyBossHud = std::max(dyBossBlot, dyHealthBounds);
        yBoss = (static_cast<float>(g_gl.height) - dyBossHud) * 0.5f;
    }

    pboss->pfont->PushScaling(fontScaleX, fontScaleY);
    float uFlash = 0.0f;
    bool fShake = false;

    if (pboss->tHealthChanged != 0.0f)
    {
        const float dtHealthChanged = g_clock.t - pboss->tHealthChanged;

        if (dtHealthChanged < 0.5f)
            uFlash = 1.0f - dtHealthChanged * 2.0f;

        fShake = dtHealthChanged < 0.75f;
    }

    pboss->uHealthDisplay = GSmooth(pboss->uHealthDisplay, pboss->uHealthTarget, g_clock.dt, &BossHealthDisplay, 0);

    const float dxHealthBar = pboss->dxHealthBar * scaleX;
    const float dyHealthBar = pboss->dyHealthBar * scaleY;
    const float dyHealthFilled = pboss->uHealthDisplay * dyHealthBar;
    const float dyHealthEmpty = dyHealthBar - dyHealthFilled;

    const float xHealthLeft = xBoss + pboss->xHealthBar * scaleX;
    const float xHealthRight = xHealthLeft + dxHealthBar;
    const float yHealthTop = yBoss + pboss->yHealthBar * scaleY;
    const float yHealthFilled = yHealthTop + dyHealthEmpty;
    const float yHealthBottom = yHealthFilled + dyHealthFilled;

    FillScreenRect(32, 32, 32, 192, xHealthLeft, yHealthTop, xHealthRight, yHealthFilled);

    const int red = 96 + static_cast<int>(uFlash * 96.99899f);
    const int green = static_cast<int>(uFlash * 192.999f);
    const int blue = static_cast<int>(uFlash * 192.999f);
    const int alpha = 192 + static_cast<int>(uFlash * 63.998993f);

    FillScreenRect(red, green, blue, alpha, xHealthLeft, yHealthFilled, xHealthRight, yHealthBottom);

    float dxShake = 0.0f;
    float dyShake = 0.0f;

    if (fShake)
    {
        dxShake = std::sin(g_clock.tReal * 40.0f) * 8.0f * scaleX;
        dyShake = std::cos(g_clock.tReal * 15.0f) * 4.0f * scaleY;
    }

    char achzPhaseCount[] =
    {
        static_cast<char>(pboss->cPhasesRemaining + 114),
        '\0'
    };

    // Retail draws the boss HUD's fixed 'z' glyph at the blot origin, then
    // draws the phase-dependent glyph below it.
    pboss->pfont->SetupDraw();
    pboss->pfont->DxDrawChRotated90('z', xBoss, yBoss, pboss->rgba);
    pboss->pfont->CleanUpDraw();

    const float dxPhaseCount = pboss->pfont->DxFromCh(achzPhaseCount[0]);
    const float dyPhaseCount =
        static_cast<float>(pboss->pfont->m_dyUnscaled) * pboss->pfont->m_ryScale;
    const float xPhaseCount =
        xBoss + static_cast<float>(pboss->xPhaseCount) * scaleX - dxPhaseCount * 0.5f;
    const float yPhaseCount =
        yBoss + static_cast<float>(pboss->yPhaseCount) * scaleY - dyPhaseCount * 0.5f;

    CTextBox tbxPhaseCount{};
    tbxPhaseCount.SetPos(xPhaseCount + dxShake, yPhaseCount + dyShake);
    tbxPhaseCount.SetSize(0.0f, 0.0f);
    tbxPhaseCount.SetTextColor(&pboss->rgba);
    tbxPhaseCount.SetHorizontalJust(JH_Left);
    tbxPhaseCount.SetVerticalJust(JV_Top);

    pboss->pfont->DrawPchz(achzPhaseCount, &tbxPhaseCount);

    pboss->pfont->PopScaling();
}

BOSS g_boss;
SMP s_smpBossctrSlide = { 3.0, 0.0, 0.5 };
SMP BossHealthDisplay = { 0.5, 0.25, 0.2};
