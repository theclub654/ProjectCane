#include "shd.h"

void UnloadShaders()
{
    for (int i = 0; i < g_ashd.size(); i++)
    {
        glDeleteTextures(1, &g_ashd[i].glShadowMap);
        glDeleteTextures(1, &g_ashd[i].glDiffuseMap);
        glDeleteTextures(1, &g_ashd[i].glSaturateMap);
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
	g_pfont.clear();
	g_pfont.shrink_to_fit();
	g_pfontScreenCounters.clear();
	g_pfontScreenCounters.shrink_to_fit();
	g_pfontJoy.clear();
	g_pfontJoy.shrink_to_fit();
	g_aglyff.clear();
	g_aglyff.shrink_to_fit();
	g_grfzonShaders = 0;
	g_cpsaa = 0;
	g_apsaa.clear();
	g_apsaa.shrink_to_fit();
    textureDataStart = 0;
}

void LoadColorTablesFromBrx(CBinaryInputStream *pbis)
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

void LoadBitmapsFromBrx(CBinaryInputStream *pbis)
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
	for (int i = 0; i < g_cfontBrx ; i++)
		g_afontBrx[i].LoadFromBrx(pbis);
}

void LoadTexFromBrx(TEX *ptex, CBinaryInputStream* pbis)
{
	ptex->oid    = (OID)pbis->U16Read();
	ptex->grftex = pbis->S16Read();
	ptex->cibmp  = pbis->U8Read();
	ptex->ciclut = pbis->U8Read();
    
	ptex->bmpIndex.resize(ptex->cibmp);
	for (int i = 0; i < ptex->cibmp; i++)
		ptex->bmpIndex[i] = pbis->U16Read();

	ptex->clutIndex.resize(ptex->ciclut);
	for (int i = 0; i < ptex->ciclut; i++)
		ptex->clutIndex[i] = pbis->U16Read();
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
            case 0: rgb = glm::vec3(v, t, p); break;
            case 1: rgb = glm::vec3(q, v, p); break;
            case 2: rgb = glm::vec3(p, v, t); break;
            case 3: rgb = glm::vec3(p, q, v); break;
            case 4: rgb = glm::vec3(t, p, v); break;
            case 5: rgb = glm::vec3(v, p, q); break;
            default: rgb = glm::vec3(0.0f); break;
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

void LoadShadersFromBrx(CBinaryInputStream *pbis)
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
		g_ashd[i].rp        = pbis->U8Read();
		g_ashd[i].ctex      = pbis->U8Read();

		g_ashd[i].atex.resize(g_ashd[i].ctex);

		// Reading shader animation from file
		PsaaLoadFromBrx(pbis);

		// Reading texture tables from file
		for (int a = 0; a < g_ashd[i].ctex; a++)
			LoadTexFromBrx(&g_ashd[i].atex[a], pbis);
	}

	LoadFontsFromBrx(pbis);
}

void LoadTexturesFromBrx(CBinaryInputStream* pbis)
{
    uint8_t csm1ClutIndices[256];

    for (uint16_t i = 0; i < 0x100; i += 0x20) 
    {
        for (uint16_t j = i; j < i + 8; j++) 
        {
            csm1ClutIndices[j + 0x0]  = static_cast <uint8_t>(j) + 0x0;
            csm1ClutIndices[j + 0x8]  = static_cast <uint8_t>(j) + 0x10;
            csm1ClutIndices[j + 0x10] = static_cast <uint8_t>(j) + 0x8;
            csm1ClutIndices[j + 0x18] = static_cast <uint8_t>(j) + 0x18;
        }
    }

    textureDataStart = pbis->file.tellg();

    for (int i = 0; i < g_ashd.size(); i++)
    {
        switch (g_ashd[i].shdk)
        {
            case SHDK_ThreeWay:
            MakeTexture(g_ashd[i].glShadowMap,  g_ashd[i].atex[0].clutIndex[0], g_ashd[i].atex[0].bmpIndex[0], csm1ClutIndices, pbis);
            MakeTexture(g_ashd[i].glDiffuseMap, g_ashd[i].atex[0].clutIndex[1], g_ashd[i].atex[0].bmpIndex[0], csm1ClutIndices, pbis);

            SHD *pshd;
            pshd = PshdFindShader(g_ashd[i].oidAltSat);

            if (pshd == nullptr)
                MakeTexture(g_ashd[i].glSaturateMap, g_ashd[i].atex[0].clutIndex[2], g_ashd[i].atex[0].bmpIndex[0], csm1ClutIndices, pbis);
            else
                MakeTexture(pshd->glSaturateMap, pshd->atex[0].clutIndex[2], pshd->atex[0].bmpIndex[0], csm1ClutIndices, pbis);

            break;

            case SHDK_Prelit:
            case SHDK_Background:
            case SHDK_MurkFill:
            case SHDK_Max:
            MakeTexture(g_ashd[i].glDiffuseMap, g_ashd[i].atex[0].clutIndex[0], g_ashd[i].atex[0].bmpIndex[0], csm1ClutIndices, pbis);
            break;

            case SHDK_Shadow:
            case SHDK_SpotLight:
            MakeTexture(g_ashd[i].glDiffuseMap, g_ashd[i].atex[0].clutIndex[0], g_ashd[i].atex[0].bmpIndex[0], csm1ClutIndices, pbis);
            break;

            case SHDK_ProjectedVolume:
            MakeTexture(g_ashd[i].glDiffuseMap, g_ashd[i].atex[0].clutIndex[0], g_ashd[i].atex[0].bmpIndex[0], csm1ClutIndices, pbis);
            break;

            case SHDK_CreateTexture:
            MakeTexture(g_ashd[i].glDiffuseMap, g_ashd[i].atex[0].clutIndex[0], g_ashd[i].atex[0].bmpIndex[0], csm1ClutIndices, pbis);
            break;
        }
    }
}

std::vector <byte> MakeBmp(uint32_t bmpIndex, CBinaryInputStream* pbis)
{
    std::vector <byte> buffer;

    size_t bufferOff = textureDataStart + g_abmp[bmpIndex].baseOffset;
    int width  = g_abmp[bmpIndex].bmpWidth;
    int height = g_abmp[bmpIndex].bmpHeight;

    buffer.resize(width * height);
    pbis->file.seekg(bufferOff, SEEK_SET);

    for (int i = 0; i < width * height; i++)
        buffer[i] = pbis->U8Read();

    return buffer;
}

std::vector <byte> MakePallete(uint32_t clutIndex, CBinaryInputStream* pbis)
{
    std::vector <byte> buffer;

    size_t paletteBuffer = textureDataStart + g_aclut[clutIndex].baseOffset;
    int numColors = g_aclut[clutIndex].numColors;
    int colorSize = g_aclut[clutIndex].colorSize;

    buffer.resize(numColors * colorSize * 4);

    pbis->file.seekg(paletteBuffer, SEEK_SET);

    for (int i = 0; i < numColors * colorSize * 4; i++)
        buffer[i] = pbis->U8Read();

    return buffer;
}

void MakeTexture(GLuint &textureReference, int16_t clutIndex, int16_t bmpIndex, uint8_t *csm1ClutIndices, CBinaryInputStream* pbis)
{
    if (clutIndex >= g_aclut.size() || bmpIndex >= g_abmp.size())
        return;

    std::vector <byte> image;
    std::vector <byte> pallete;
    std::vector <byte> texture;

    image   = MakeBmp(bmpIndex, pbis);
    pallete = MakePallete(clutIndex, pbis);

    short width  = g_abmp[bmpIndex].bmpWidth;
    short height = g_abmp[bmpIndex].bmpHeight;

    texture.resize(width * height * 4);

    byte alpha;
    if (g_aclut[clutIndex].numColors > 16)
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

    glGenTextures(1, &textureReference);
    glBindTexture(GL_TEXTURE_2D, textureReference);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.data());
}


int g_cclut;
std::vector <CLUT> g_aclut;
int g_grfzonShaders;
int g_cbmp;
int g_cshd;
std::vector <BMP> g_abmp;
std::vector <SHD> g_ashd;
int g_cpsaa;
std::vector <SAA> g_apsaa;
std::vector <TEX> g_atex;
// Start of texture data
size_t textureDataStart;