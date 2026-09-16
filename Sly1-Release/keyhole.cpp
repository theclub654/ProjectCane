#include "keyhole.h"
#include "render.h"
#include "po.h"

KEYHOLE* NewKeyhole()
{
    return new KEYHOLE{};
}

void InitKeyhole(KEYHOLE *pkeyhole)
{
	InitLo(pkeyhole);
	g_pkeyhole = pkeyhole;
}

int GetKeyholeSize()
{
    return sizeof(KEYHOLE);
}

void LoadKeyholeFromBrx(KEYHOLE *pkeyhole, CBinaryInputStream *pbis)
{
	LoadOptionsFromBrx(pkeyhole, pbis);

    pkeyhole->cpos = pbis->U16Read();
    pkeyhole->apos.resize(pkeyhole->cpos);

    for (int i = 0; i < pkeyhole->cpos; i++)
    {
        pkeyhole->apos[i].x =  pbis->F32Read();
        pkeyhole->apos[i].y = -pbis->F32Read();
        pkeyhole->apos[i].w = 1.0;
    }

    pkeyhole->posMin.x = pbis->F32Read();
    pkeyhole->posMin.y = pbis->F32Read();
    pkeyhole->posMin.w = 1.0;

    pkeyhole->posMax.x = pbis->F32Read();
    pkeyhole->posMax.y = pbis->F32Read();
    pkeyhole->posMax.w = 1.0;
    pkeyhole->dx = pkeyhole->posMax.x - pkeyhole->posMin.x;

    for (int i = 0; i < 5; ++i)
    {
        KS& ks = pkeyhole->mpkpks[i];

        SHD* pshd = PshdFindShader(static_cast<OID>(1166 + i));
        ks.rgba = pshd != nullptr ? pshd->rgba : glm::vec4(0.5f);

        ks.ctri = static_cast<int>(pbis->U16Read());
        ks.atri.resize(ks.ctri);

        for (TRI& tri : ks.atri)
        {
            tri.aipos[0] = static_cast<int>(pbis->U16Read());
            tri.aipos[1] = static_cast<int>(pbis->U16Read());
            tri.aipos[2] = static_cast<int>(pbis->U16Read());
        }

        if (ks.ctri == 0)
            continue;

        if (ks.vao == 0)
            glGenVertexArrays(1, &ks.vao);

        if (ks.vbo == 0)
            glGenBuffers(1, &ks.vbo);

        if (ks.ebo == 0)
            glGenBuffers(1, &ks.ebo);

        glBindVertexArray(ks.vao);

        glBindBuffer(GL_ARRAY_BUFFER, ks.vbo);
        glBufferData(GL_ARRAY_BUFFER, pkeyhole->apos.size() * sizeof(glm::vec4), pkeyhole->apos.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ks.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, ks.atri.size() * sizeof(TRI), ks.atri.data(), GL_STATIC_DRAW);

        glDisableVertexAttribArray(1);
        glVertexAttrib2f(1, 0.0f, 0.0f);

        glBindVertexArray(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CloneKeyhole(KEYHOLE* pkeyhole, KEYHOLE* pkeyholeBase)
{
    CloneLo(pkeyhole, pkeyholeBase);

    pkeyhole->cpos = pkeyholeBase->cpos;

    pkeyhole->apos = pkeyholeBase->apos;

    pkeyhole->posMin = pkeyholeBase->posMin;
    pkeyhole->posMax = pkeyholeBase->posMax;

    pkeyhole->dx = pkeyholeBase->dx;

    std::copy(std::begin(pkeyholeBase->mpkpks), std::end(pkeyholeBase->mpkpks), std::begin(pkeyhole->mpkpks));
}

void DrawKeyholeMask(KEYHOLE* pkeyhole, float x, float y, float rScale, float uAlpha)
{
    if (pkeyhole == nullptr)
        return;

    const float alpha = glm::clamp(uAlpha, 0.0f, 1.0f);

    if (alpha <= 0.0f)
        return;

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(rScale, rScale, 1.0f));

    glBlotShader.Use();

    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, whiteTex);
    glUniform1i(u_fontTexLoc, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    for (int ikpks = 4; ikpks >= 1; --ikpks)
    {
        const KS& ks = pkeyhole->mpkpks[ikpks];

        if (ks.ctri <= 0 || ks.vao == 0)
            continue;

        const glm::vec4& baseColor = ikpks >= 3 ? g_rgbaKeyholeEyes : g_rgbaKeyholeMask;
        const glm::vec4 color(baseColor.r, baseColor.g, baseColor.b, baseColor.a * alpha);

        glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));

        glBindVertexArray(ks.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ks.ctri * 3), GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);
}

void DrawKeyhole(KEYHOLE* pkeyhole, float uClosed)
{
    if (pkeyhole == nullptr || pkeyhole->dx <= 0.0f)
        return;

    uClosed = glm::clamp(uClosed, 0.0f, 1.0f);

    const float screenWidth = g_gl.width;
    const float screenHeight = g_gl.height;
    const float halfWidth = screenWidth * 0.5f;
    const float halfHeight = screenHeight * 0.5f;

    const float scaleOpen = (screenWidth * 2.0f) / pkeyhole->dx;
    const float scaleClosed = 4.0f / pkeyhole->dx;
    const float scale = glm::mix(scaleOpen, scaleClosed, uClosed);

    glm::vec2 center(halfWidth, halfHeight);

    PO* ppo = PpoCur();

    if (ppo != nullptr)
    {
        glm::vec3 posScreen{};
        ConvertCmWorldToScreen(g_pcm, &ppo->xf.posWorld, &posScreen);

        const float xTarget = glm::clamp(posScreen.x * halfWidth + halfWidth, 10.0f, screenWidth - 10.0f);
        const float yTarget = glm::clamp(-posScreen.y * halfHeight + halfHeight, 16.0f, screenHeight - 16.0f);

        center.x = glm::mix(halfWidth, xTarget, uClosed);
        center.y = glm::mix(halfHeight, yTarget, uClosed);
    }

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(scale, scale, 1.0f));

    glBlotShader.Use();

    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, whiteTex);
    glUniform1i(u_fontTexLoc, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    const float revealAlpha = glm::clamp(uClosed * 2.5f, 0.0f, 1.0f);

    glm::vec4 backgroundColor(0.0f);

    for (int ikpks = 4; ikpks >= 0; --ikpks)
    {
        const KS& ks = pkeyhole->mpkpks[ikpks];

        glm::vec4 color = ks.rgba;

        if (ikpks == 0)
        {
            color.r *= 1.0f - uClosed;
            color.g *= 1.0f - uClosed;
            color.b *= 1.0f - uClosed;
            color.a = glm::mix(color.a, 1.0f, uClosed);
        }

        color.a *= revealAlpha;

        if (ikpks == 0)
            backgroundColor = color;

        if (ks.ctri <= 0 || ks.vao == 0)
            continue;

        glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));

        glBindVertexArray(ks.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ks.ctri * 3), GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);

    const glm::vec2 transformedMin = center + glm::vec2(pkeyhole->posMin.x, pkeyhole->posMin.y) * scale;
    const glm::vec2 transformedMax = center + glm::vec2(pkeyhole->posMax.x, pkeyhole->posMax.y) * scale;

    const float xMin = glm::clamp(glm::min(transformedMin.x, transformedMax.x), 0.0f, screenWidth);
    const float yMin = glm::clamp(glm::min(transformedMin.y, transformedMax.y), 0.0f, screenHeight);
    const float xMax = glm::clamp(glm::max(transformedMin.x, transformedMax.x), 0.0f, screenWidth);
    const float yMax = glm::clamp(glm::max(transformedMin.y, transformedMax.y), 0.0f, screenHeight);

    const bool intersectsScreen = xMax > 0.0f && xMin < screenWidth && yMax > 0.0f && yMin < screenHeight;

    if (!intersectsScreen)
        return;

    const auto ToByte = [](float value)
    {
        return static_cast<int>(glm::clamp(value, 0.0f, 1.0f) * 255.0f);
    };

    const int red = ToByte(backgroundColor.r);
    const int green = ToByte(backgroundColor.g);
    const int blue = ToByte(backgroundColor.b);
    const int alpha = ToByte(backgroundColor.a);

    FillScreenRect(red, green, blue, alpha, 0.0f, 0.0f, screenWidth, yMin);
    FillScreenRect(red, green, blue, alpha, 0.0f, yMax, screenWidth, screenHeight);
    FillScreenRect(red, green, blue, alpha, 0.0f, yMin, xMin, yMax);
    FillScreenRect(red, green, blue, alpha, xMax, yMin, screenWidth, yMax);

    const float finalCloseAlpha = glm::clamp((uClosed - 0.9f) * 10.0f, 0.0f, 1.0f);

    if (finalCloseAlpha > 0.0f)
        FillScreenRect(red, green, blue, ToByte(backgroundColor.a * finalCloseAlpha), xMin, yMin, xMax, yMax);
}

void DeleteKeyhole(KEYHOLE* pkeyhole)
{
    delete pkeyhole;
}

// Release keyhole.c static initializer:
//   mask = RGBA(0, 135, 255, 127)
//   eyes = RGBA(191, 191, 191, 127)
glm::vec4 g_rgbaKeyholeEyes(
    191.0f / 255.0f,
    191.0f / 255.0f,
    191.0f / 255.0f,
    127.0f / 128.0f);

glm::vec4 g_rgbaKeyholeMask(
    0.0f,
    135.0f / 255.0f,
    1.0f,
    127.0f / 128.0f);

KEYHOLE* g_pkeyhole = nullptr;
