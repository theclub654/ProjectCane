#include "shd.h"
#include "upscale.h"

void UnloadShaders()
{
    for (SHD& shd : g_ashd)
    {
        for (TEX& tex : shd.atex)
        {
            for (uint64_t& handle : tex.hDiffuseMap)
            {
                if (handle != 0)
                    glMakeTextureHandleNonResidentARB(handle);
                handle = 0;
            }

            for (GLuint& texture : tex.glDiffuseMap)
            {
                if (texture != 0)
                    glDeleteTextures(1, &texture);
                texture = 0;
            }
        }
    }

    for (int i = 0; i < g_cbmp; i++)
    {
        if (g_abmp[i].hShadowMap != 0)
        {
            glMakeTextureHandleNonResidentARB(g_abmp[i].hShadowMap);
            g_abmp[i].hShadowMap = 0;
        }

        if (g_abmp[i].glShadowMap != 0)
        {
            glDeleteTextures(1, &g_abmp[i].glShadowMap);
            g_abmp[i].glShadowMap = 0;
        }

        if (g_abmp[i].hDiffuseMap != 0)
        {
            glMakeTextureHandleNonResidentARB(g_abmp[i].hDiffuseMap);
            g_abmp[i].hDiffuseMap = 0;
        }

        if (g_abmp[i].glDiffuseMap != 0)
        {
            glDeleteTextures(1, &g_abmp[i].glDiffuseMap);
            g_abmp[i].glDiffuseMap = 0;
        }

        if (g_abmp[i].hSaturateMap != 0)
        {
            glMakeTextureHandleNonResidentARB(g_abmp[i].hSaturateMap);
            g_abmp[i].hSaturateMap = 0;
        }

        if (g_abmp[i].glSaturateMap != 0)
        {
            glDeleteTextures(1, &g_abmp[i].glSaturateMap);
            g_abmp[i].glSaturateMap = 0;
        }
    }

    g_cshd = 0;
    g_ashd.clear();
    g_ashd.shrink_to_fit();
    g_cbmp = 0;
    g_abmp.clear();
    g_abmp.shrink_to_fit();
    g_cclut = 0;
    g_aclut.clear();
    g_aclut.shrink_to_fit();
    g_cfontBrx = 0;
    g_afontBrx.clear();
    g_afontBrx.shrink_to_fit();
    g_pfont = nullptr;
    g_pfontScreenCounters = nullptr;
    g_pfontJoy = nullptr;
    g_grfzonShaders = 0;

    for (int i = 0; i < g_apsaaSw.size(); i++)
        g_apsaaSw[i]->pvtsaa->pfnDeleteSaa(g_apsaaSw[i]);

    g_cpsaa = 0;
    g_apsaa.clear();
    g_apsaa.shrink_to_fit();
    g_apsaaSw.clear();
    g_apsaaSw.shrink_to_fit();
    g_testFontBrx.clear();
    g_testFontBrx.shrink_to_fit();
    textureDataStart = 0;
}

void LoadColorTablesFromBrx(CBinaryInputStream* pbis)
{
    // Loads the number of CLUT propertys from binary file
    g_cclut = pbis->U16Read();
    g_aclut.resize(g_cclut);

    // Loading CLUT propertys from binary file
    for (int i = 0; i < g_cclut; i++)
    {
        g_aclut[i].grfzon     = pbis->U32Read();
        g_aclut[i].numColors  = pbis->U16Read();
        g_aclut[i].colorSize  = pbis->U16Read();
        g_aclut[i].baseOffset = pbis->U32Read();
    }
}

void LoadBitmapsFromBrx(CBinaryInputStream* pbis)
{
    // Loads number of texture propertys
    g_cbmp = pbis->U16Read();
    g_abmp.resize(g_cbmp);

    // Loading texture propertys from binary file
    for (int i = 0; i < g_cbmp; i++)
    {
        g_abmp[i].bmpWidth   = pbis->U16Read();
        g_abmp[i].bmpHeight  = pbis->U16Read();
        g_abmp[i].grfzon     = pbis->U32Read();
        g_abmp[i].psm        = pbis->S8Read();
        g_abmp[i].cgsRow     = pbis->S8Read();
        g_abmp[i].cgsPixels  = pbis->U16Read();
        g_abmp[i].cbPixels   = pbis->U32Read();
        g_abmp[i].baseOffset = pbis->U32Read();
    }
}

void LoadFontsFromBrx(CBinaryInputStream* pbis)
{
    // Loading number of fonts from file
    g_cfontBrx = pbis->U16Read();
    g_afontBrx.resize(g_cfontBrx);

    // Loading font property's from binary file
    for (int i = 0; i < g_cfontBrx; i++)
    {
        g_afontBrx[i].LoadFromBrx(pbis);
        g_afontBrx[i].m_grffont = 1 << (i & 0x1f);
    }

    g_pfont = PfontFromFont(0);
}

void LoadTexFromBrx(TEX* ptex, CBinaryInputStream* pbis)
{
    ptex->oid    = (OID)pbis->U16Read();
    ptex->grftex = pbis->S16Read();
    ptex->cibmp  = pbis->U8Read();
    ptex->ciclut = pbis->U8Read();

    ptex->bmpIndex.resize(ptex->cibmp);
    ptex->abmp.resize(ptex->cibmp);
    for (int i = 0; i < ptex->cibmp; i++)
    {
        int bmpIndex = pbis->U16Read();

        ptex->bmpIndex[i] = bmpIndex;
        if (bmpIndex < g_cbmp)
            ptex->abmp[i] = &g_abmp[bmpIndex];
    }

    ptex->clutIndex.resize(ptex->ciclut);
    ptex->aclut.resize(ptex->ciclut);
    for (int i = 0; i < ptex->ciclut; i++)
    {
        int clutIndex = pbis->U16Read();

        ptex->clutIndex[i] = clutIndex;
        if (clutIndex < g_cclut)
            ptex->aclut[i] = &g_aclut[clutIndex];
    }
}

void ConvertUserHsvToUserRgb(glm::vec3& pvecHSV, glm::vec3& pvecRGB)
{
    float h = pvecHSV.x;
    float s = pvecHSV.y * (1.0f / 255.0f);  // scale from 0�255
    float v = pvecHSV.z * (1.0f / 255.0f);  // scale from 0�255

    // Clamp hue to [0, 360], or set to -1 for undefined
    float hue = (h >= 0.0f) ? glm::clamp(h, 0.0f, 360.0f) : -1.0f;

    // Clamp saturation and value to [0, 1]
    s = glm::clamp(s, 0.0f, 1.0f);
    v = glm::clamp(v, 0.0f, 1.0f);

    glm::vec3 rgb(0.0f);

    if (s == 0.0f || hue < 0.0f)
        rgb = glm::vec3(v);
    else
    {
        hue = (hue == 360.0f) ? 0.0f : hue;
        float hueSegment = hue / 60.0f;
        int i = static_cast<int>(std::floor(hueSegment));
        float f = hueSegment - i;

        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));

        switch (i)
        {
            case 0:  rgb = glm::vec3(v, t, p); break;
            case 1:  rgb = glm::vec3(q, v, p); break;
            case 2:  rgb = glm::vec3(p, v, t); break;
            case 3:  rgb = glm::vec3(p, q, v); break;
            case 4:  rgb = glm::vec3(t, p, v); break;
            case 5:  rgb = glm::vec3(v, p, q); break;
            default: rgb = glm::vec3(0.0f);    break;
        }
    }

    pvecRGB = rgb * 255.0f;
}

SHD* PshdFindShader(OID oid)
{
    if (oid == -1)
        return nullptr;

    for (int i = 0; i < g_cshd; i++)
    {
        if (g_ashd[i].oid == oid)
            return &g_ashd[i];
    }

    return nullptr;
}

void LoadShadersFromBrx(CBinaryInputStream* pbis)
{
    LoadColorTablesFromBrx(pbis);
    LoadBitmapsFromBrx(pbis);

    g_cshd = pbis->U16Read();
    g_ashd.resize(g_cshd);

    g_cpsaa = pbis->U16Read();
    g_apsaa.reserve(g_cpsaa);

    for (int i = 0; i < g_cshd; i++)
    {
        SHD& shd = g_ashd[i];

        shd.shdk = (SHDK)pbis->U8Read();

        shd.grfshd = pbis->U8Read();
        shd.oid = (OID)pbis->S16Read();

        shd.rgba.r = (pbis->U8Read() * 2.0f) / 0x1FE;
        shd.rgba.g = (pbis->U8Read() * 2.0f) / 0x1FE;
        shd.rgba.b = (pbis->U8Read() * 2.0f) / 0x1FE;
        shd.rgba.a = (pbis->U8Read() * 2.0f) / 0x1FE;

        shd.rgbaVolume.r = (pbis->U8Read() * 2.0f) / 0x1FE;
        shd.rgbaVolume.g = (pbis->U8Read() * 2.0f) / 0x1FE;
        shd.rgbaVolume.b = (pbis->U8Read() * 2.0f) / 0x1FE;
        shd.rgbaVolume.a = (pbis->U8Read() * 2.0f) / 0x1FE;

        shd.grfzon = pbis->U32Read();
        shd.oidAltSat = (OID)pbis->U16Read();
        shd.rp = (RP)pbis->U8Read();
        shd.ctex = pbis->U8Read();

        shd.atex.resize(shd.ctex);

        SAA* psaa = PsaaLoadFromBrx(pbis);
        shd.psaa = psaa;

        if (psaa != nullptr)
        {
            psaa->sai.pshd = &shd;
            g_apsaa.push_back(psaa);
        }

        for (int a = 0; a < shd.ctex; a++)
        {
            LoadTexFromBrx(&shd.atex[a], pbis);
            shd.atex[a].pshd = &shd;
        }

        if (!shd.atex.empty())
            shd.cframe = shd.atex[0].cibmp;
        else
            shd.cframe = 0;

        // ------------------------------------------------------------
        // Create GL textures for every iframe, not just frame 0.
        // Original PackTexGifs uses ptex->apbmp[iframe].
        // ------------------------------------------------------------
        for (int a = 0; a < shd.ctex; a++)
        {
            TEX& tex = shd.atex[a];

            // A BMP contains indexed source pixels, but the resolved RGBA
            // texture also depends on this TEX's CLUT.  Keep that resolved
            // texture on TEX so shaders which share a BMP cannot overwrite
            // one another's palette selection.
            if (shd.shdk != SHDK_ThreeWay)
            {
                tex.glDiffuseMap.resize(tex.abmp.size(), 0);
                tex.hDiffuseMap.resize(tex.abmp.size(), 0);
                tex.diffuseTexture.resize(tex.abmp.size());
            }

            for (int iframe = 0; iframe < (int)tex.abmp.size(); iframe++)
            {
                BMP* pbmp = tex.abmp[iframe];

                if (pbmp == nullptr)
                    continue;

                int width = pbmp->bmpWidth;
                int height = pbmp->bmpHeight;
                int maxDim = std::max(width, height);
                int mipLevels = 1 + (int)std::floor(std::log2((double)maxDim));

                auto SetupTexture = [&](GLuint& glTex, uint64_t& handle)
                {
                    if (glTex != 0)
                        return;

                    glGenTextures(1, &glTex);
                    glBindTexture(GL_TEXTURE_2D, glTex);

                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                    if (tex.grftex & 1)
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

                    if (tex.grftex & 2)
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    glTexStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA8, width, height);

                    handle = glGetTextureHandleARB(glTex);
                    glMakeTextureHandleResidentARB(handle);
                };

                if (shd.shdk == SHDK_ThreeWay)
                {
                    SetupTexture(pbmp->glShadowMap,   pbmp->hShadowMap);
                    SetupTexture(pbmp->glDiffuseMap,  pbmp->hDiffuseMap);
                    SetupTexture(pbmp->glSaturateMap, pbmp->hSaturateMap);
                }
                else
                    SetupTexture(tex.glDiffuseMap[iframe], tex.hDiffuseMap[iframe]);
            }
        }

        // ------------------------------------------------------------
        // Original Sly-style SHDP allocation.
        // ------------------------------------------------------------
        switch (shd.shdk)
        {
            case SHDK_ThreeWay:
            shd.cshdp = 3;
            break;

            case SHDK_ProjectedVolume:
            shd.cshdp = 4;
            break;

            default:
            shd.cshdp = 1;
            break;
        }

        shd.ashdp.clear();
        shd.ashdp.resize(shd.cshdp);

        auto AllocShdp = [&](int index, int cqwRegs)
        {
            SHDP& shdp = shd.ashdp[index];

            shdp.cqwRegs = cqwRegs;

            // vector<uint16_t>
            // 1 QW = 16 bytes = 8 uint16_t
            shdp.aaqwRegs.resize(shd.cframe * cqwRegs * 8);
        };

        switch (shd.shdk)
        {
            case SHDK_ThreeWay:
            {
                AllocShdp(0, 8);
                AllocShdp(1, 8);
                AllocShdp(2, 8);
                break;
            }

            case SHDK_ProjectedVolume:
            {
                int extra = 0;

                if ((shd.grfshd & 2) != 0)
                    extra = 3;

                AllocShdp(0, extra + 5);
                AllocShdp(1, 5);
                AllocShdp(2, 7);
                AllocShdp(3, 4);
                break;
            }

            case SHDK_Shadow:
            case SHDK_SpotLight:
            {
                AllocShdp(0, 6);
                break;
            }

            case SHDK_CreateTexture:
            {
                AllocShdp(0, 7);
                break;
            }

            case SHDK_MurkClear:
            {
                AllocShdp(0, 4);
                break;
            }

            default:
            {
                AllocShdp(0, 8);
                break;
            }
        }
    }

    LoadFontsFromBrx(pbis);

    for (int i = 0; i < g_cfontBrx; i++)
    {
        if (g_afontBrx[i].m_pbmp != nullptr && g_afontBrx[i].m_pbmp->glDiffuseMap == 0)
        {
            BMP* pbmp = g_afontBrx[i].m_pbmp;

            int width  = pbmp->bmpWidth;
            int height = pbmp->bmpHeight;

            glGenTextures(1, &pbmp->glDiffuseMap);
            glBindTexture(GL_TEXTURE_2D, pbmp->glDiffuseMap);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

            pbmp->hDiffuseMap = glGetTextureHandleARB(pbmp->glDiffuseMap);
            glMakeTextureHandleResidentARB(pbmp->hDiffuseMap);
        }
    }

    for (int i = 0; i < g_apsaa.size(); i++)
    {
        if (g_apsaa[i]->pvtsaa != nullptr)
            g_apsaa[i]->pvtsaa->pfnPostSaaLoad(g_apsaa[i]);
    }

    PostBlotsLoad();
}

void SetSaiIframe(SAI* psai, int iframe)
{
    if (!psai || !psai->pshd || psai->pshd->cframe <= 0)
        return;

    int maxFrame = psai->pshd->cframe - 1;

    if (iframe < 0)
        iframe = 0;
    else if (iframe > maxFrame)
        iframe = maxFrame;

    if (psai->iframe == iframe)
        return;

    psai->iframe = iframe;
}

void LoadTexturesFromBrx(CBinaryInputStream* pbis)
{
    for (uint16_t i = 0; i < 0x100; i += 0x20)
    {
        for (uint16_t j = i; j < i + 8; j++)
        {
            csm1ClutIndices[j + 0x0]  = static_cast<uint8_t>(j) + 0x0;
            csm1ClutIndices[j + 0x8]  = static_cast<uint8_t>(j) + 0x10;
            csm1ClutIndices[j + 0x10] = static_cast<uint8_t>(j) + 0x8;
            csm1ClutIndices[j + 0x18] = static_cast<uint8_t>(j) + 0x18;
        }
    }

    textureDataStart = pbis->file.tellg();

    for (int i = 0; i < g_cshd; i++)
    {
        SHD& shd = g_ashd[i];

        if (shd.atex.empty())
            continue;

        TEX &tex = shd.atex[0];

        for (int iframe = 0; iframe < (int)tex.abmp.size(); iframe++)
        {
            BMP* pbmp = tex.abmp[iframe];

            if (!pbmp)
                continue;

            if (shd.shdk == SHDK_ThreeWay)
            {
                CLUT *ambientClut  = nullptr;
                CLUT *diffuseClut  = nullptr;
                CLUT *saturateClut = nullptr;

                int clutBase = iframe * 3;

                if (clutBase + 0 < (int)tex.aclut.size())
                    ambientClut = tex.aclut[clutBase + 0];

                if (clutBase + 1 < (int)tex.aclut.size())
                    diffuseClut = tex.aclut[clutBase + 1];

                if (clutBase + 2 < (int)tex.aclut.size())
                    saturateClut = tex.aclut[clutBase + 2];

                MakeTexture(pbmp->glShadowMap,   pbmp->hShadowMap,   &tex, pbmp, pbmp->shadowTexture,   ambientClut,  false, true, pbis);
                MakeTexture(pbmp->glDiffuseMap,  pbmp->hDiffuseMap,  &tex, pbmp, pbmp->diffuseTexture,  diffuseClut,  false, true, pbis);
                MakeTexture(pbmp->glSaturateMap, pbmp->hSaturateMap, &tex, pbmp, pbmp->saturateTexture, saturateClut, false, true, pbis);
            }
            else
            {
                CLUT *clut = nullptr;

                if (iframe < (int)tex.aclut.size())
                    clut = tex.aclut[iframe];

                if (iframe < static_cast<int>(tex.glDiffuseMap.size()))
                {
                    MakeTexture(tex.glDiffuseMap[iframe], tex.hDiffuseMap[iframe],
                        &tex, pbmp, tex.diffuseTexture[iframe], clut, false, true, pbis);

                }
                else
                {
                    MakeTexture(pbmp->glDiffuseMap, pbmp->hDiffuseMap,
                        &tex, pbmp, pbmp->diffuseTexture, clut, false, true, pbis);
                }
            }
        }
    }

    for (int i = 0; i < g_cfontBrx; i++)
        MakeTexture(g_afontBrx[i].m_pbmp->glDiffuseMap, g_afontBrx[i].m_pbmp->hDiffuseMap, nullptr, g_afontBrx[i].m_pbmp, g_afontBrx[i].m_pbmp->diffuseTexture, g_afontBrx[i].m_pclut, true, false, pbis);

}

std::vector <byte> MakeBmp(BMP *pbmp, CBinaryInputStream *pbis)
{
    std::vector <byte> bmpBuffer;

    size_t off  = textureDataStart + pbmp->baseOffset;
    // cbPixels is the encoded byte count. A 4-bit indexed bitmap stores two
    // pixels per byte, so width * height would read twice the allocation and
    // can poison the stream before its CLUT is read (font 4 is affected).
    size_t size = static_cast<size_t>(pbmp->cbPixels);

    bmpBuffer.resize(size);

    pbis->file.clear();
    pbis->file.seekg(off, std::ios::beg);
    pbis->file.read(reinterpret_cast<char*>(bmpBuffer.data()), static_cast<std::streamsize>(size));

    return bmpBuffer;
}

std::vector <byte> MakePallete(CLUT *pclut, CBinaryInputStream *pbis)
{
    std::vector <byte> palleteBuffer;

    size_t off  = textureDataStart + pclut->baseOffset;
    size_t size = static_cast<size_t>(pclut->numColors) * pclut->colorSize * 4;

    palleteBuffer.resize(size);

    // All texture offsets are absolute relative to textureDataStart. Recover
    // from an earlier short read before seeking to the palette.
    pbis->file.clear();
    pbis->file.seekg(off, std::ios::beg);
    pbis->file.read(reinterpret_cast<char*>(palleteBuffer.data()), static_cast<std::streamsize>(size));

    return palleteBuffer;
}

void MakeTexture(GLuint& textureReference, uint64_t& textureHandle, TEX* ptex, BMP* pbmp, std::vector<byte>& texture, CLUT* pclut, bool fFlip, bool fMipMap, CBinaryInputStream* pbis)
{
    if (pbmp == nullptr || pclut == nullptr || textureReference == 0)
        return;

    std::vector <byte> image   = MakeBmp(pbmp, pbis);
    std::vector <byte> pallete = MakePallete(pclut, pbis);

    int width  = pbmp->bmpWidth;
    int height = pbmp->bmpHeight;

    texture.resize(width * height * 4);

    if (pclut->numColors > 16)
    {
        for (int i = 0; i < width * height; i++)
        {
            int index = csm1ClutIndices[image[i]] * 4;

            texture[4 * i + 0] = pallete[index + 0];
            texture[4 * i + 1] = pallete[index + 1];
            texture[4 * i + 2] = pallete[index + 2];
            texture[4 * i + 3] = pallete[index + 3] * 255u / 128u;
        }
    }
    else
    {
        for (int i = 0; i < width * height / 2; i++)
        {
            int index1 = image[i] & 0x0F;
            int index2 = image[i] >> 4;

            texture[8 * i + 0] = pallete[4 * index1 + 0];
            texture[8 * i + 1] = pallete[4 * index1 + 1];
            texture[8 * i + 2] = pallete[4 * index1 + 2];
            texture[8 * i + 3] = pallete[4 * index1 + 3] * 255u / 128u;

            texture[8 * i + 4] = pallete[4 * index2 + 0];
            texture[8 * i + 5] = pallete[4 * index2 + 1];
            texture[8 * i + 6] = pallete[4 * index2 + 2];
            texture[8 * i + 7] = pallete[4 * index2 + 3] * 255u / 128u;
        }
    }

    if (fFlip == true)
    {
        int rowSize = width * 4;
        std::vector <byte> tempRow(rowSize);

        for (int y = 0; y < height / 2; ++y)
        {
            byte* rowTop = &texture[y * rowSize];
            byte* rowBottom = &texture[(height - 1 - y) * rowSize];

            std::memcpy(tempRow.data(), rowTop, rowSize);
            std::memcpy(rowTop, rowBottom, rowSize);
            std::memcpy(rowBottom, tempRow.data(), rowSize);
        }
    }

    glBindTexture(GL_TEXTURE_2D, textureReference);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texture.data());

    if (fMipMap == true)
        glGenerateMipmap(GL_TEXTURE_2D);

    if (g_fDebugMode < 1)
    {
        texture.clear();
        texture.shrink_to_fit();
    }
}

void UpdateShaders(float dt)
{
    for (int i = 0; i < g_apsaa.size(); ++i)
    {
        SAA* saa = g_apsaa[i];
        if (!saa)
            continue;

        if (!FUpdatableSaa(saa))
            continue;

        auto* vtsaa = saa->pvtscroller;
        if (!vtsaa || !vtsaa->pfnUpdateScroller)
            continue;

        vtsaa->pfnUpdateScroller((SCROLLER*)saa, dt);
    }
}

int g_cclut;
std::vector <CLUT> g_aclut;
int g_grfzonShaders;
int g_cbmp;
std::vector <BMP> g_abmp;
int g_cshd;
std::vector <SHD> g_ashd;
std::vector <TEX> g_atex;
int g_cpsaa;
std::vector <SAA*> g_apsaa;
std::vector <SAA*> g_apsaaSw;
SAI* g_psaiUpdate = nullptr;
SAI* g_psaiUpdateTail = nullptr;
size_t textureDataStart;
uint8_t csm1ClutIndices[256];
