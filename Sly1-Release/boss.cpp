#include "boss.h"
#include "render.h"

void StartupBoss(BOSS* pboss)
{
	pboss->pvtboss = &g_vtboss;
}

void PostBossLoad(BOSS* pboss)
{
    PostBlotLoad(pboss);

    pboss->pfont = PfontFromFont(4);
    // The retail UI coordinates are expanded by the PS2 display path.  Blots
    // rendered directly in window pixels need that scale applied explicitly.
    pboss->rFontScale = 1.5f;
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

    pboss->xPhaseCount = dyFont * 0.5f;
    pboss->yPhaseCount = dxPhaseGlyph + sSpacing + dyFont * 0.5f;
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

    const float dyHealthFilled = pboss->uHealthDisplay * pboss->dyHealthBar;
    const float dyHealthEmpty = pboss->dyHealthBar - dyHealthFilled;

    const float xHealthLeft = pboss->x + pboss->xHealthBar;
    const float xHealthRight = xHealthLeft + pboss->dxHealthBar;
    const float yHealthTop = pboss->y + pboss->yHealthBar;
    const float yHealthFilled = yHealthTop + dyHealthEmpty;
    const float yHealthBottom = yHealthFilled + dyHealthFilled;
	const float sHealthBorder = 2.0f * pboss->rFontScale;

	// Draw the frame as four strips. A single backing rectangle would overlap
	// the interior at the same depth, causing the later fills to fail GL_LESS.
	FillScreenRect(0, 0, 0, 255,
		xHealthLeft - sHealthBorder, yHealthTop - sHealthBorder,
		xHealthRight + sHealthBorder, yHealthTop);
	FillScreenRect(0, 0, 0, 255,
		xHealthLeft - sHealthBorder, yHealthBottom,
		xHealthRight + sHealthBorder, yHealthBottom + sHealthBorder);
	FillScreenRect(0, 0, 0, 255,
		xHealthLeft - sHealthBorder, yHealthTop,
		xHealthLeft, yHealthBottom);
	FillScreenRect(0, 0, 0, 255,
		xHealthRight, yHealthTop,
		xHealthRight + sHealthBorder, yHealthBottom);

    FillScreenRect(32, 32, 32, 192, xHealthLeft, yHealthTop, xHealthRight, yHealthFilled);

    const int red = 96 + static_cast<int>(uFlash * 96.99899f);
    const int green = static_cast<int>(uFlash * 192.999f);
    const int blue = static_cast<int>(uFlash * 192.999f);
    const int alpha = 192 + static_cast<int>(uFlash * 63.998993f);

    FillScreenRect(red, green, blue, alpha, xHealthLeft, yHealthFilled, xHealthRight, yHealthBottom);

    pboss->pfont->PushScaling(pboss->rFontScale, pboss->rFontScale);

    float dxShake = 0.0f;
    float dyShake = 0.0f;

    if (fShake)
    {
        dxShake = std::sin(g_clock.tReal * 40.0f) * 8.0f;
        dyShake = std::cos(g_clock.tReal * 15.0f) * 4.0f;
    }

    char achzPhaseCount[] =
    {
        static_cast<char>(pboss->cPhasesRemaining + 114),
        '\0'
    };

    CTextBox tbxPhaseCount{};
    tbxPhaseCount.SetPos(pboss->x + pboss->xPhaseCount + dxShake, pboss->y + pboss->yPhaseCount + dyShake);
    tbxPhaseCount.SetSize(0.0f, 0.0f);
    tbxPhaseCount.SetTextColor(&pboss->rgba);
    tbxPhaseCount.SetHorizontalJust(JH_Left);
    tbxPhaseCount.SetVerticalJust(JV_Top);

    if (pboss->pte != nullptr)
        pboss->pfont->EdgeRect(pboss->pte, &tbxPhaseCount);

    pboss->pfont->DrawPchz(achzPhaseCount, &tbxPhaseCount);

    pboss->pfont->PopScaling();
}

BOSS g_boss;
SMP s_smpBossctrSlide = { 3.0, 0.0, 0.5 };
SMP BossHealthDisplay = { 0.5, 0.25, 0.2};
