#include "screen.h"
#include "mgv.h"
#include "jt.h"

void StartupScreen()
{
    StartupLetterbox(&g_letterbox);
    StartupBinoc(&g_binoc);
    StartupLgnr(&g_lgnr);
    StartupLifeCtr(&g_lifectr);
    StartupClueCtr(&g_cluectr);
    StartupKeyCtr(&g_keyctr);
    StartupGoldCtr(&g_goldctr);
    StartupCoinCtr(&g_coinctr);
    StartupCrusherCtr(&g_crusherctr);
    StartupLapCtr(&g_lapctr);
    StartupBoostCtr(&g_boostctr);
    StartupPlaceCtr(&g_placectr);
    StartupSecurityCodeCtr(&g_securitycodectr);
    StartupMgvHealthCtr(&g_mgvhealthctr);
    StartupBoss(&g_boss);
    StartupPuffChargerCtr(&g_puffchargectr);
    StartupTimer(&g_timer);
    StartupNote(&g_note);
    StartupCredit(&g_credit);
    StartupTitle(&g_title);
    StartupTotals(&g_totals);
    StartupCall(&g_call);
    StartupWmc(&g_wmc);
    StartupPrompt(&g_prompt);
    StartupTvLeft(&g_tvLeft);
    StartupTvRight(&g_tvRight);
    StartupVan(&g_van);
    StartupScores(&g_scores);
    StartupLogo(&g_logo);
    StartupAttract(&g_attract);
    StartupJtIcon(&g_rubyicon);
    StartupJtIcon(&g_jticon);
    StartupSaveBlot(&g_autosave);
    StartupHubBlot(&g_hubblot);
    StartupFmvMenu(&g_fmvmenu);
    StartupPercentCtr(&g_percentctr);
    StartupDebugMenu(&g_debugmenu);

    for (int i = 0; i < BLOTK_Max; i++)
    {
        BLOT* pblot = PblotFromBlotk(i);
        if (pblot)
            pblot->pvtblot->pfnInitBlot(pblot, (BLOTK)i);
    }

    InitWipe(&g_wipe);
}

void DrawLineScreen(float x1, float y1, float z1, float x2, float y2, float z2, const glm::vec4& color, bool depthTest)
{
    const BLOTLINEVERTEX vertices[2] =
    {
        { glm::vec3(x1, y1, z1), glm::vec2(0.0f, 0.0f), color },
        { glm::vec3(x2, y2, z2), glm::vec2(1.0f, 0.0f), color }
    };

    const GLboolean wasDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);

    if (depthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBlotShader.Use();

    const glm::mat4 model(1.0f);
    const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);
    const glm::vec4 blotColor(1.0f);

    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4fv(uvRectLoc, 1, glm::value_ptr(uvRect));
    glUniform4fv(blotColorLoc, 1, glm::value_ptr(blotColor));
    glUniform1i(u_useVertexColorLoc, 1);

    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);

    glBindVertexArray(lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, lineVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUniform1i(u_useVertexColorLoc, 0);

    if (!wasDepthTestEnabled)
        glDisable(GL_DEPTH_TEST);

    if (!wasBlendEnabled)
        glDisable(GL_BLEND);
}

void DrawLineWorld()
{
}
