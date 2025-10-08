#include "logo.h"

void StartupLogo(LOGO* plogo)
{
    plogo->pvtlogo = &g_vtlogo;
}

void PostLogoLoad(LOGO* plogo)
{
    // Initialize base BLOT layout
    PostBlotLoad(plogo);

    // Clone the font with logo-specific scale
    if (plogo->pfont)
    {
        plogo->pfont = plogo->pfont->PfontClone(RX_LogoText, RY_LogoText);
    }

    // Set up text edge font if available
    plogo->pte = &g_teLogo;
    g_teLogo.m_pfont = &g_afontBrx[2];

    // Try to find the shader for the attract logo
    plogo->pshd = PshdFindShader((OID)1171);

    if (plogo->pshd == nullptr)
    {
        // Fallback: set a default draw string if no shader was found
        if (plogo->pvtblot && plogo->pvtblot->pfnSetBlotAchzDraw)
        {
            plogo->pvtblot->pfnSetBlotAchzDraw(plogo, (char*)logoPchz);
        }
    }
    else
    {
        // If shader found, resize the logo BLOT to a specific size
        ResizeBlot(plogo, 366.75f, 165.75f);
    }
}

void DrawLogo(LOGO* plogo)
{
    if (!plogo->pshd) {
        DrawBlot(plogo);
        return;
    }

    BMP* pbmp = plogo->pshd->atex[0].abmp[0]; // first frame
    GLuint texture = pbmp->glDiffuseMap;

    float width = plogo->dx;
    float height = plogo->dy;
    float x = plogo->x;
    float y = plogo->y;

    // Set up UVs
    float u0 = 0.0f, v0 = 0.0f;
    float u1 = 1.0f, v1 = 1.0f;

    // Bind shader and state
    glBlotShader.Use();
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniform4f(uvRectLoc, u0, v0, u1, v1);
    glUniform4fv(blotColorLoc, 1, glm::value_ptr(glm::vec4(1.0f)));

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(g_gl.gao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y + height, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(width, -height, 1.0f));
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}

LOGO g_logo;
CTextEdge g_teLogo;
const char *logoPchz = "Sly Cooper\nand the Thievius Raccoonus";
float RX_LogoText = 0.89999998;
float RY_LogoText = 0.89999998;