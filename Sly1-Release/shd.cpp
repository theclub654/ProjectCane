#include "shd.h"
#include "upscale.h"

void UnloadShaders()
{
    for (int i = 0; i < g_cbmp; i++)
    {
        glDeleteTextures(1, &g_abmp[i].glShadowMap);
        glDeleteTextures(1, &g_abmp[i].glDiffuseMap);
        glDeleteTextures(1, &g_abmp[i].glSaturateMap);
    }

    for (int i = 0; i < g_afontBrx.size(); i++)
    {
        glDeleteTextures(1, &g_afontBrx[i].m_pbmp->glShadowMap);
        glDeleteTextures(1, &g_afontBrx[i].m_pbmp->glDiffuseMap);
        glDeleteTextures(1, &g_afontBrx[i].m_pbmp->glSaturateMap);
    }

    glDeleteTextures(1, &whiteTex);

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
    g_cpsaa = 0;
    g_apsaa.clear();
    g_apsaa.shrink_to_fit();
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

    if (g_cfontBrx != 0)
    {
        g_pfont = &g_afontBrx[0];

        if (g_cfontBrx < 2) {
            g_pfontScreenCounters = &g_afontBrx[0];
        }

        else {
            g_pfontScreenCounters = &g_afontBrx[1];
        }

        if (g_cfontBrx < 3) {
            g_pfontJoy = &g_afontBrx[0];
        }

        else {
            g_pfontJoy = &g_afontBrx[2];
        }
    }

    whiteTex;
    glGenTextures(1, &whiteTex);
    glBindTexture(GL_TEXTURE_2D, whiteTex);
    uint32_t white = 0xFFFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
    float s = pvecHSV.y * (1.0f / 255.0f);  // scale from 0–255
    float v = pvecHSV.z * (1.0f / 255.0f);  // scale from 0–255

    // Clamp hue to [0, 360], or set to -1 for undefined
    float hue = (h >= 0.0f) ? glm::clamp(h, 0.0f, 360.0f) : -1.0f;

    // Clamp saturation and value to [0, 1]
    s = glm::clamp(s, 0.0f, 1.0f);
    v = glm::clamp(v, 0.0f, 1.0f);

    glm::vec3 rgb(0.0f);

    if (s == 0.0f || hue < 0.0f)
    {
        // Grayscale
        rgb = glm::vec3(v);
    }
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
    // Loads CLUT property's from binary file
    LoadColorTablesFromBrx(pbis);
    // Loads texture property's from binary file
    LoadBitmapsFromBrx(pbis);

    // Loading number of shaders from binary file
    g_cshd = pbis->U16Read();
    g_ashd.resize(g_cshd);

    // Loading number of shader animation's from file
    g_cpsaa = pbis->U16Read();
    g_apsaa.resize(g_cpsaa);

    for (int i = 0; i < g_cshd; i++)
    {
        // Loading shader property's from binary file
        g_ashd[i].shdk   = (SHDK)pbis->U8Read();
        g_ashd[i].grfshd = pbis->U8Read();
        g_ashd[i].oid    = (OID)pbis->S16Read();

        g_ashd[i].rgba.r = (pbis->U8Read() * 2.0f) / 0x1FE;
        g_ashd[i].rgba.g = (pbis->U8Read() * 2.0f) / 0x1FE;
        g_ashd[i].rgba.b = (pbis->U8Read() * 2.0f) / 0x1FE;
        g_ashd[i].rgba.a = (pbis->U8Read() * 2.0f) / 0x1FE;

        g_ashd[i].rgbaVolume.r = (pbis->U8Read() * 2.0f) / 0x1FE;
        g_ashd[i].rgbaVolume.g = (pbis->U8Read() * 2.0f) / 0x1FE;
        g_ashd[i].rgbaVolume.b = (pbis->U8Read() * 2.0f) / 0x1FE;
        g_ashd[i].rgbaVolume.a = (pbis->U8Read() * 2.0f) / 0x1FE;

        g_ashd[i].grfzon    = pbis->U32Read();
        g_ashd[i].oidAltSat = (OID)pbis->U16Read();
        g_ashd[i].rp        = (RP)pbis->U8Read();
        g_ashd[i].ctex      = pbis->U8Read();

        g_ashd[i].atex.resize(g_ashd[i].ctex);

        // Reading shader animation from file
        PsaaLoadFromBrx(pbis);

        // Reading texture tables from file
        for (int a = 0; a < g_ashd[i].ctex; a++)
            LoadTexFromBrx(&g_ashd[i].atex[a], pbis);
    }

    LoadFontsFromBrx(pbis);


    PostBlotsLoad();
}

void LoadTexturesFromBrx(CBinaryInputStream* pbis)
{
    for (uint16_t i = 0; i < 0x100; i += 0x20)
    {
        for (uint16_t j = i; j < i + 8; j++) {
            csm1ClutIndices[j + 0x0]  = static_cast <uint8_t>(j) + 0x0;
            csm1ClutIndices[j + 0x8]  = static_cast <uint8_t>(j) + 0x10;
            csm1ClutIndices[j + 0x10] = static_cast <uint8_t>(j) + 0x8;
            csm1ClutIndices[j + 0x18] = static_cast <uint8_t>(j) + 0x18;
        }
    }

    textureDataStart = pbis->file.tellg();

    for (int i = 0; i < g_cshd; i++)
    {
        switch (g_ashd[i].shdk)
        {
            case SHDK_ThreeWay:
            MakeTexture(g_ashd[i].atex[0].abmp[0]->glShadowMap,   &g_ashd[i].atex[0], g_ashd[i].atex[0].abmp[0], g_ashd[i].atex[0].abmp[0]->shadowTexture   ,g_ashd[i].atex[0].aclut[0], false, pbis);
            MakeTexture(g_ashd[i].atex[0].abmp[0]->glDiffuseMap,  &g_ashd[i].atex[0], g_ashd[i].atex[0].abmp[0], g_ashd[i].atex[0].abmp[0]->diffuseTexture  ,g_ashd[i].atex[0].aclut[1], false, pbis);
            MakeTexture(g_ashd[i].atex[0].abmp[0]->glSaturateMap, &g_ashd[i].atex[0], g_ashd[i].atex[0].abmp[0], g_ashd[i].atex[0].abmp[0]->saturateTexture ,g_ashd[i].atex[0].aclut[2], false, pbis);
            break;

            default:
            MakeTexture(g_ashd[i].atex[0].abmp[0]->glDiffuseMap, &g_ashd[i].atex[0], g_ashd[i].atex[0].abmp[0], g_ashd[i].atex[0].abmp[0]->diffuseTexture, g_ashd[i].atex[0].aclut[0], false, pbis);
            break;
        }
    }

    for (int i = 0; i < g_cfontBrx; i++)
        MakeTexture(g_afontBrx[i].m_pbmp->glDiffuseMap, &g_ashd[i].atex[0], g_afontBrx[i].m_pbmp, g_afontBrx[i].m_pbmp->diffuseTexture, g_afontBrx[i].m_pclut, true, pbis);
}

std::vector <byte> MakeBmp(BMP* pbmp, CBinaryInputStream* pbis)
{
    std::vector <byte> bmpBuffer;

    size_t bufferOff = textureDataStart + pbmp->baseOffset;
    int width  = pbmp->bmpWidth;
    int height = pbmp->bmpHeight;

    bmpBuffer.resize(width * height);
    pbis->file.seekg(bufferOff, SEEK_SET);

    for (int i = 0; i < width * height; i++)
        bmpBuffer[i] = pbis->U8Read();

    return bmpBuffer;
}

std::vector <byte> MakePallete(CLUT* pclut, CBinaryInputStream* pbis)
{
    std::vector<byte> palleteBuffer;
    size_t off = textureDataStart + pclut->baseOffset;
    int numColors = pclut->numColors;
    int colorSize = pclut->colorSize;

    palleteBuffer.resize(numColors * colorSize * 4);
    pbis->file.seekg(off, SEEK_SET);

    for (int i = 0; i < numColors * colorSize * 4; ++i)
        palleteBuffer[i] = pbis->U8Read();

    return palleteBuffer;
}

void MakeTexture(GLuint& textureReference, TEX *ptex ,BMP* pbmp, std::vector <byte>& texture, CLUT* pclut, bool fFlip, CBinaryInputStream* pbis)
{
    if (pbmp == nullptr || pclut == nullptr || textureReference != 0)
        return;

    std::vector <byte> image;
    std::vector <byte> pallete;

    image   = MakeBmp(pbmp, pbis);
    pallete = MakePallete(pclut, pbis);

    short width  = pbmp->bmpWidth;
    short height = pbmp->bmpHeight;

    texture.resize(width * height * 4);

    if (pclut->numColors > 16)
    {
        for (int i = 0; i < width * height; i++)
        {
            int index = csm1ClutIndices[image[i]] * 4;

            texture[4 * i + 0] = pallete[index + 0];
            texture[4 * i + 1] = pallete[index + 1];
            texture[4 * i + 2] = pallete[index + 2];
            texture[4 * i + 3] = pallete[index + 3] * 255 / 128;
        }
    }

    else
    {
        for (int i = 0; i < width * height / 2; i++)
        {
            byte index1 = image[i] >> 4;
            byte index2 = image[i] & 0x0F;

            texture[8 * i + 0] = pallete[4 * index1 + 0];
            texture[8 * i + 1] = pallete[4 * index1 + 1];
            texture[8 * i + 2] = pallete[4 * index1 + 2];
            texture[8 * i + 3] = pallete[4 * index1 + 3] * 255 / 128;

            texture[8 * i + 4] = pallete[4 * index2 + 0];
            texture[8 * i + 5] = pallete[4 * index2 + 1];
            texture[8 * i + 6] = pallete[4 * index2 + 2];
            texture[8 * i + 7] = pallete[4 * index2 + 3] * 255 / 128;
        }
    }

    if (fFlip == true)
    {
        int rowSize = width * 4;
        std::vector<byte> tempRow(rowSize);

        for (int y = 0; y < height / 2; ++y) {
            byte* rowTop = &texture[y * rowSize];
            byte* rowBottom = &texture[(height - 1 - y) * rowSize];

            std::memcpy(tempRow.data(), rowTop, rowSize);
            std::memcpy(rowTop, rowBottom, rowSize);
            std::memcpy(rowBottom, tempRow.data(), rowSize);
        }
    }

    // Choose your final scale (net 2x is a great default for Sly 1)
        //const int finalScale = 2;              // 2x final
        //const SampleEdge edgeMode = SampleEdge::Wrap; // Wrap for tiling; Clamp for sprites/UI

        //// 1) 4x nearest
        //std::vector<byte> big;
        //nearestScaleRGBA4x(texture, width, height, big);
        //int bw = width * 4, bh = height * 4;

        //// 2) small Gaussian blur in premult+linear (removes stair-steps/dither)
        //gaussian3_premult_linear(big, bw, bh, edgeMode);

        //// 3) downsample to final (gamma-aware bilinear)
        //std::vector <byte> finalTex;
        //int newW = width * finalScale;
        //int newH = height * finalScale;
        //resizeRGBA_bilinear_gamma(big, bw, bh, finalTex, newW, newH, edgeMode);

        //// swap & update dimensions
        //texture.swap(finalTex);
        //width = (short)newW;
        //height = (short)newH;
    
    glGenTextures(1, &textureReference);
    glBindTexture(GL_TEXTURE_2D, textureReference);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (ptex->grftex & 1) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    }
    if (ptex->grftex & 2) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.data());

    glGenerateMipmap(GL_TEXTURE_2D);
}

int g_cclut;
std::vector <CLUT> g_aclut;
int g_grfzonShaders;
int g_cbmp;
std::vector <BMP> g_abmp;
int g_cshd;
std::vector <SHD> g_ashd;
int g_cpsaa;
std::vector <SAA> g_apsaa;
std::vector <TEX> g_atex;
size_t textureDataStart;
uint8_t csm1ClutIndices[256];