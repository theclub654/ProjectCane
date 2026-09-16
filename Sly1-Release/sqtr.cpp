#include "sqtr.h"
#include "alo.h"
#include "render.h"

void ClearSqtrm(SQTRM* psqtrm)
{
    psqtrm->isqtrMic = 0;
    psqtrm->isqtrMac = 0;
}

void UpdateSqtrm(SQTRM* psqtrm, glm::vec3* ppos, glm::mat3* pmat, float swExtra, float dt)
{
    if (psqtrm == nullptr || psqtrm->pshd == nullptr || psqtrm->palo == nullptr || dt == 0.0f)
        return;

    ALO* palo = psqtrm->palo;

    glm::vec3& posCurrent = ppos != nullptr ? *ppos : palo->xf.posWorld;
    glm::mat3& matCurrent = pmat != nullptr ? *pmat : palo->xf.matWorld;

    glm::vec3 apos[2] = {};
    glm::vec3 av[2] = {};

    int cpos = std::clamp(psqtrm->cposLocal, 0, 2);

    /*
     * Transform the trail control points into world space and calculate
     * the world-space velocity of each point.
     */
    for (int i = 0; i < cpos; ++i)
    {
        apos[i] = posCurrent + matCurrent * psqtrm->aposLocal[i];
        CalculateAloMovement(palo, palo->paloRoot, apos[i], &av[i], nullptr, nullptr, nullptr);
    }

    float speed = 0.0f;

    if (cpos == 1)
    {
        speed = glm::length(av[0]);
        apos[1] = apos[0];

        /*
         * A zero-radius trail with one control point is treated as having
         * no visible width and therefore no effective movement.
         */
        if (psqtrm->sRadius == 0.0f)
            speed = 0.0f;
    }
    else if (cpos >= 2)
    {
        speed = std::max(glm::length(av[0]), glm::length(av[1]));
    }

    /*
     * swExtra scales the trail intensity according to the distance of its
     * furthest local control point from the owning object's origin.
     */
    if (swExtra != 0.0f)
    {
        float maxLocalDistance = 0.0f;

        for (int i = 0; i < cpos; ++i)
            maxLocalDistance = std::max(maxLocalDistance, glm::length(psqtrm->aposLocal[i]));

        speed += swExtra * maxLocalDistance;
    }

    /*
     * Evaluate the original quadratic CLQ curve:
     *
     *     alpha = g0 + speed * (g1 + speed * g2)
     */
    float alpha = psqtrm->clqSvToAlpha.g0 + speed * (psqtrm->clqSvToAlpha.g1 + speed * psqtrm->clqSvToAlpha.g2);
    alpha = glm::clamp(alpha, 0.0f, 1.0f);

    auto AdvanceRingIndex = [](int index)
    {
        return (index + 1) & 63;
    };

    auto PushTrailSample = [&](glm::vec3 samplePos[2], float sampleAlpha)
    {
        SQTR& sample = psqtrm->asqtr[psqtrm->isqtrMac];

        sample.t = g_clock.t;
        sample.uAlpha = sampleAlpha;
        sample.apos[0] = samplePos[0];
        sample.apos[1] = samplePos[1];

        psqtrm->isqtrMac = AdvanceRingIndex(psqtrm->isqtrMac);

        /*
        * The producer caught the consumer, so discard the oldest entry.
        */
        if (psqtrm->isqtrMac == psqtrm->isqtrMic)
        psqtrm->isqtrMic = AdvanceRingIndex(psqtrm->isqtrMic);
    };

    /*
     * Avoid repeatedly appending completely transparent samples. One
     * transparent sample is retained to terminate/fade the existing strip.
     */
    bool shouldAddSample = alpha != 0.0f;

    if (!shouldAddSample && psqtrm->isqtrMic != psqtrm->isqtrMac)
    {
        int previousIndex = (psqtrm->isqtrMac + 63) & 63;
        shouldAddSample = psqtrm->asqtr[previousIndex].uAlpha != 0.0f;
    }

    if (shouldAddSample)
    {
        /*
         * Give a one-point trail camera-facing width.
         */
        if (psqtrm->sRadius > 0.0f)
        {
            glm::vec3 cameraDepthAxis = g_pcm->mat[2];

            apos[0] += cameraDepthAxis * psqtrm->sRadius;
            apos[1] -= cameraDepthAxis * psqtrm->sRadius;
        }

        /*
         * Determine the rotation between the previous and current matrices.
         *
         * For orthonormal rotation matrices:
         *
         *     matDelta * matPrev = matCurrent
         */
        glm::mat3 matDelta = matCurrent * glm::transpose(psqtrm->matPrev);
        glm::quat quatDelta = glm::normalize(glm::quat_cast(matDelta));

        float cosHalfAngle = glm::clamp(quatDelta.w, -1.0f, 1.0f);
        float rad = 2.0f * std::acos(cosHalfAngle);

        if (rad > glm::pi<float>())
            rad -= glm::two_pi<float>();

        glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
        float sinHalfAngle = std::sqrt(std::max(0.0f, 1.0f - cosHalfAngle * cosHalfAngle));

        if (sinHalfAngle > 0.0001f)
            rotationAxis = glm::vec3(quatDelta.x, quatDelta.y, quatDelta.z) / sinHalfAngle;

        /*
         * Large rotations are subdivided so that the generated trail does
         * not cut directly across the object's rotational arc.
         */
        if (std::abs(rad) > 0.2f)
        {
            int subdivisionCount = static_cast<int>(std::abs(rad) * 5.0f) + 1;
            float inverseSubdivisionCount = 1.0f / static_cast<float>(subdivisionCount);

            glm::quat rotationStep = glm::angleAxis(rad * inverseSubdivisionCount, glm::normalize(rotationAxis));
            glm::mat3 matStep = glm::mat3_cast(rotationStep);

            glm::mat3 mat = psqtrm->matPrev;
            glm::vec3 pos = psqtrm->posPrev;
            glm::vec3 dpos = (posCurrent - psqtrm->posPrev) * inverseSubdivisionCount;

            for (int i = 0; i < subdivisionCount; ++i)
            {
                mat = matStep * mat;
                pos += dpos;

                glm::vec3 interpolatedPoints[2] = {};

                for (int j = 0; j < cpos; ++j)
                    interpolatedPoints[j] = pos + mat * psqtrm->aposLocal[j];

                if (cpos == 1)
                    interpolatedPoints[1] = interpolatedPoints[0];

                if (psqtrm->sRadius > 0.0f)
                {
                    glm::vec3 cameraDepthAxis = g_pcm->mat[2];

                    interpolatedPoints[0] += cameraDepthAxis * psqtrm->sRadius;
                    interpolatedPoints[1] -= cameraDepthAxis * psqtrm->sRadius;
                }

                PushTrailSample(interpolatedPoints, alpha);
            }
        }
        else
        {
            PushTrailSample(apos, alpha);
        }
    }

    psqtrm->posPrev = posCurrent;
    psqtrm->matPrev = matCurrent;
}

void RenderSqtrm(SQTRM* psqtrm, CM* pcm)
{
    if (psqtrm == nullptr || pcm == nullptr || psqtrm->palo == nullptr)
        return;

    if (psqtrm->isqtrMic == psqtrm->isqtrMac)
        return;

    RPL rpl{};
    rpl.rp = RP_Translucent;
    rpl.PFNDRAWRPL = DrawSqtrm;

    const glm::vec3 dpos = psqtrm->palo->xf.posWorld - pcm->pos;

    /*
     * Camera-space depth used to sort the translucent trail.
     * mat[0] corresponds to the camera depth/sort axis in this engine.
     */
    rpl.z = glm::dot(dpos, pcm->mat[0]);
	rpl.ro.model = glm::mat4(1.0f);
    rpl.ro.uAlpha = 1.0f;
	rpl.ro.darken = 1.0f;
	rpl.ro.warpType = WARP_NONE;
	rpl.ro.sRadius = psqtrm->sRadius;
	rpl.ro.posCenter = glm::vec4(psqtrm->palo->xf.posWorld, 1.0f);
    rpl.psqtrm = psqtrm;

    g_translucentAddPrpl[g_translucentAddCount] = rpl;
    //g_translucentAddPrpl[g_translucentAddCount].PFNDRAWRPL = DrawGlob;
    g_translucentAddCount++;
    //SubmitRpl(&rpl);
}

void DrawSqtrm(RPL* prpl)
{
    if (prpl == nullptr || prpl->psqtrm == nullptr)
        return;

    SQTRM* psqtrm = prpl->psqtrm;
    if (psqtrm->pshd == nullptr || psqtrm->isqtrMic == psqtrm->isqtrMac ||
        psqtrm->pshd->atex.empty() || psqtrm->pshd->atex[0].abmp.empty())
    {
        return;
    }

    const TEX& tex = psqtrm->pshd->atex[0];
    BMP* pbmp = tex.abmp[0];
    const GLuint64 diffuseHandle = !tex.hDiffuseMap.empty() && tex.hDiffuseMap[0] != 0
        ? tex.hDiffuseMap[0]
        : (pbmp != nullptr ? pbmp->hDiffuseMap : 0);

    if (pbmp == nullptr || diffuseHandle == 0)
        return;

    constexpr int sqtrCapacity = 64;
    const glm::vec2 uvTip(0.0f, 0.0f);
    const glm::vec2 uvBase(0.0f, 1.0f);
    const glm::vec4 shaderColor = glm::clamp(
        psqtrm->pshd->rgba, glm::vec4(0.0f), glm::vec4(1.0f));

    SQTRGPU vertices[sqtrCapacity * 2];
    int vertexCount = 0;
    int isqtr = psqtrm->isqtrMic;
    int firstLive = -1;

    while (isqtr != psqtrm->isqtrMac)
    {
        const SQTR& sqtr = psqtrm->asqtr[isqtr];
        const float age = g_clock.t - sqtr.t;
        const float fade = glm::clamp(
            psqtrm->clqDtToAlpha.g0 +
            age * (psqtrm->clqDtToAlpha.g1 + age * psqtrm->clqDtToAlpha.g2),
            0.0f, 1.0f);
        const float alpha = glm::clamp(prpl->ro.uAlpha * sqtr.uAlpha * fade, 0.0f, 1.0f);

        if (alpha > 0.0f && firstLive < 0)
            firstLive = isqtr;

        vertices[vertexCount++] = {
            sqtr.apos[0], uvTip,
            glm::vec4(shaderColor.r, shaderColor.g, shaderColor.b,
                alpha * shaderColor.a)
        };
        vertices[vertexCount++] = {
            sqtr.apos[1], uvBase,
            glm::vec4(shaderColor.r, shaderColor.g, shaderColor.b,
                glm::clamp(alpha * psqtrm->rAlpha * shaderColor.a, 0.0f, 1.0f))
        };

        isqtr = (isqtr + 1) & (sqtrCapacity - 1);
    }

    psqtrm->isqtrMic = firstLive < 0 ? psqtrm->isqtrMac : firstLive;
    if (vertexCount < 4)
        return;

    glGlobShader.Use();
    AppendStream(&ropStream, &prpl->ro, sizeof(ROGL), sizeof(ROGL));

    if (psqtrm->pshd->shdk == SHDK_ThreeWay)
    {
        glUniform1i(glslRko, 1);
        glUniformHandleui64ARB(glslAmbientMap, pbmp->hShadowMap);
        glUniformHandleui64ARB(glslDiffuseMap, diffuseHandle);
        glUniformHandleui64ARB(glslSaturateMap, pbmp->hSaturateMap);
    }
    else
    {
        glUniform1i(glslRko, 0);
        glUniformHandleui64ARB(glslDiffuseMap, diffuseHandle);
    }
    glUniform1i(glslfAlphaTest, 0);
    glUniform1i(glslfAnimateUv, 0);
    glUniform1i(glslfSkin, 0);
    glUniform1i(glslfPose, 0);
    glUniform1f(glslUnSelfIllum, 1.0f);
    glUniform3fv(glslSubGlobPosCenter, 1, glm::value_ptr(psqtrm->palo->xf.posWorld));
    glUniform1f(glslSubGlobRadius, psqtrm->sRadius);

    glBindVertexArray(g_gl.sqtrVao);
    glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, g_gl.sqtrVbo);

    // Retail DrawSqtrm sets GLBS::m_rgld.fCull to zero.  The ribbon must be
    // visible from both sides because opposite cane swings reverse which
    // side of the strip faces the camera.
    const GLboolean cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);
    const GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean depthWriteWasEnabled = GL_TRUE;
    GLint depthFunc = GL_LESS;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteWasEnabled);
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);

    const GLsizeiptr uploadSize = static_cast<GLsizeiptr>(vertexCount * sizeof(SQTRGPU));

    // Give the driver a fresh backing store so this upload cannot overwrite
    // ribbon vertices which are still being consumed by an earlier draw.
    glBufferData(GL_ARRAY_BUFFER, uploadSize, nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, uploadSize, vertices);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

    glDepthMask(depthWriteWasEnabled);
    glDepthFunc(depthFunc);
    if (!depthTestWasEnabled)
        glDisable(GL_DEPTH_TEST);

    if (cullFaceWasEnabled)
        glEnable(GL_CULL_FACE);

    glBindVertexArray(0);
}
