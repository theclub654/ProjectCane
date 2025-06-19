#include "letterbox.h"

void StartupLetterbox(LETTERBOX *pletterbox)
{
	pletterbox->pvtletterbox = &g_vtletterbox;
}

void DrawLetterBox(LETTERBOX* pletterbox)
{
    const float screenWidth = g_gl.width;
    const float screenHeight = g_gl.height;
    const float barMaxHeight = 66.4f;

    float uOn = glm::clamp(pletterbox->uOn, 0.0f, 1.0f);
    float barHeight = uOn * barMaxHeight;

    glBlotShader.Use();
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniform4fv(blotColorLoc, 1, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))); // solid black

    glDisable(GL_BLEND);
    glDepthFunc(GL_ALWAYS);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(g_gl.gao);

    // Top bar (from top of screen downward)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(screenWidth, barHeight, 1.0f));
        glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }

    // Bottom bar (from bottom of screen upward)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, screenHeight - barHeight, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(screenWidth, barHeight, 1.0f));
        glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }

    glBindVertexArray(0);
}

LETTERBOX g_letterbox;