#include "boss.h"

void StartupBossCtr(BOSSCTR* bossctr)
{
	bossctr->pvtbossctr = &g_vtbossctr;
}

void PostBossctrLoad(BOSSCTR* pbossctr)
{
    // Initialize as a CTR-based UI element
    PostCtrLoad(pbossctr);

    // Set font scale for boss counter
    pbossctr->rFontScale = 1.2f;

    // Regenerate the draw string (e.g., "Boss: 1/3")
    RebuildCtrAchzDraw(pbossctr);

    // Measure width of the text with scaling
    pbossctr->pfont->PushScaling(pbossctr->rFontScale, pbossctr->rFontScale);
    float dx = pbossctr->pfont->DxFromPchz(pbossctr->achzDraw);
    float dy = static_cast<float>(pbossctr->pfont->m_dyUnscaled) * pbossctr->pfont->m_ryScale;
    pbossctr->pfont->PopScaling();

    // Resize the BLOT to fit the text
    ResizeBlot(pbossctr, dx, dy);

    // Set initial display value
    pbossctr->gDisplay = 0.0f;
}

void DrawBossCtr(BOSSCTR* pbossctr)
{
    if (pbossctr->achzDraw[0] == '\0' || pbossctr->pnActual == nullptr)
        return;

    // Smooth interpolation of gDisplay toward actual counter
    float gDisplay = GSmooth(
        pbossctr->gDisplay,
        static_cast<float>(*pbossctr->pnActual),
        g_clock.dt,
        &s_smpBossctrSlide,
        nullptr
    );
    pbossctr->gDisplay = gDisplay;

    // Compute total text width with dynamic scaling
    float totalWidth = 0.0f;
    for (size_t i = 0; i < strlen(pbossctr->achzDraw); ++i)
    {
        float t = std::clamp(std::abs(gDisplay - static_cast<float>(i)), 0.0f, 1.0f);
        float scale = pbossctr->rFontScale * ((1.0f - t) + t * 0.7f);

        pbossctr->pfont->PushScaling(scale, scale);
        totalWidth += pbossctr->pfont->DxFromCh(pbossctr->achzDraw[i]);
        pbossctr->pfont->PopScaling();
    }

    // Line height and starting Y position
    float charHeight = static_cast<float>(pbossctr->pfont->m_dyUnscaled) * pbossctr->pfont->m_ryScale;
    float y = 492.8f - charHeight;
    float xCursor = 320.0f - totalWidth * 0.5f;

    // Draw each character
    for (size_t i = 0; i < strlen(pbossctr->achzDraw); ++i)
    {
        char ch = pbossctr->achzDraw[i];

        float delta = std::abs(gDisplay - static_cast<float>(i));
        float t = std::clamp(delta, 0.0f, 1.0f);
        float scale = pbossctr->rFontScale * ((1.0f - t) + t * 0.7f);

        float fade = (1.0f - delta * 0.25f) + delta * 0.25f * 0.2f;

        // Apply scaling
        pbossctr->pfont->PushScaling(scale, scale);
        float charWidth = pbossctr->pfont->DxFromCh(ch);

        // Modulate alpha using fade
        glm::vec4 color = pbossctr->rgba;
        color.a *= fade;

        // Set up textbox
        CTextBox tbx;
        tbx.SetPos(xCursor + charWidth * 0.5f, y);
        tbx.SetSize(0.0f, 0.0f);
        tbx.SetTextColor(&color);
        tbx.SetHorizontalJust(JH_Center);
        tbx.SetVerticalJust(JV_Center);

        // Draw single character
        char text[2] = { ch, '\0' };
        pbossctr->pfont->DrawPchz(text, &tbx);

        xCursor += charWidth;
        pbossctr->pfont->PopScaling();
    }
}

BOSSCTR g_bossctr;
SMP s_smpBossctrSlide = { 3.0, 0.0, 0.5 };