#include "letterbox.h"
#include "binoc.h"
#include "totals.h"

void StartupLetterbox(LETTERBOX *pletterbox)
{
	pletterbox->pvtletterbox = &g_vtletterbox;
}

void PostLetterBoxLoad(LETTERBOX* pletterbox)
{
    PostBlotLoad(pletterbox);
    pletterbox->dtDisappear = 0.5;
}

void SetLetterboxBlots(LETTERBOX* pletterbox, BLOTS blots)
{
    const BLOTS blotsCurrent = pletterbox->blots;

    if (blotsCurrent == blots)
    {
        SetBlotBlots(pletterbox, blots);
        return;
    }

    // Do not hide the letterbox while this wipe phase is active.
    if (g_wipe.wipes == 1 && (blots == BLOTS_Disappearing || blots == BLOTS_Hidden))
        blots = blotsCurrent;

    if (g_wipe.wipes == 3 && (blots == BLOTS_Appearing || blots == BLOTS_Visible))
    {
        blots = BLOTS_Hidden;

        if (!(g_transition.m_grftrans & 8))
        {
            blots = BLOTS_Visible;

            if (g_pgsCur->gameWorldCur != GAMEWORLD_Intro && !g_binoc.pdialogPlaying)
            {
                g_totals.fReshow = 0;
                g_totals.pvttotals->pfnSetTotalsBlots(&g_totals, BLOTS_Hidden);

                SetTotalsLevelText(&g_totals, g_pgsCur->gameWorldCur, g_pgsCur->worldLevelCur);

                g_totals.fDrawOverLetterbox = 1;
                SetBlotDtVisible(&g_totals, 0.0f);
                g_totals.pvttotals->pfnSetTotalsBlots(&g_totals, BLOTS_Visible);
            }
        }
    }

    if (blotsCurrent != blots && blots == BLOTS_Hidden)
    {
        if (g_totals.fDrawOverLetterbox)
            g_totals.pvttotals->pfnSetTotalsBlots(&g_totals, BLOTS_Hidden);

        if (g_autosave.fSaveComplete)
            SetBlotBlots(&g_autosave, BLOTS_Hidden);
    }

    SetBlotBlots(pletterbox, blots);
}

void DrawLetterBox(LETTERBOX* pletterbox)
{
    constexpr float originalHeight = 492.80002f;
    constexpr float originalBarHeight = 66.40001f;

    const float screenWidth = static_cast<float>(g_gl.width);
    const float screenHeight = static_cast<float>(g_gl.height);
    const float uiScale = screenHeight / originalHeight;
    const float uOn = glm::clamp(pletterbox->uOn, 0.0f, 1.0f);
    const float barHeight = originalBarHeight * uiScale * uOn;

    if (barHeight <= 0.0f)
        return;

    GLint previousDepthFunction = GL_LESS;
    GLint previousVertexArray = 0;
    GLint previousTexture = 0;
    GLboolean previousDepthMask = GL_TRUE;

    const GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blendEnabled = glIsEnabled(GL_BLEND);

    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunction);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVertexArray);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);

    glBlotShader.Use();
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

    const glm::vec4 color(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(g_gl.gao);

    {
        const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(screenWidth, barHeight, 1.0f));
        glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    }

    {
        const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, screenHeight - barHeight, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(screenWidth, barHeight, 1.0f));
        glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    }

    glBindVertexArray(static_cast<GLuint>(previousVertexArray));
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
    glDepthMask(previousDepthMask);
    glDepthFunc(previousDepthFunction);

    if (depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (blendEnabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}
LETTERBOX g_letterbox;
