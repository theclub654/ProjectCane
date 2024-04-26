#include "shd.h"

void UnloadShaders()
{
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
	ptex->oid    = pbis->U16Read();
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
		g_ashd[i].shdk   = pbis->U8Read();
		g_ashd[i].grfshd = pbis->U8Read();
		g_ashd[i].oid    = pbis->S16Read();

        g_ashd[i].rgba.bRed   = pbis->U8Read();
        g_ashd[i].rgba.bGreen = pbis->U8Read();
        g_ashd[i].rgba.bBlue  = pbis->U8Read();
        g_ashd[i].rgba.bAlpha = pbis->U8Read();

        if (g_ashd[i].rgba.bAlpha == 0x80)
            g_ashd[i].rgba.bAlpha = 0xFF;

        g_ashd[i].rgbaVolume.bRed   = pbis->U8Read();
        g_ashd[i].rgbaVolume.bGreen = pbis->U8Read();
        g_ashd[i].rgbaVolume.bBlue  = pbis->U8Read();
        g_ashd[i].rgbaVolume.bAlpha = pbis->U8Read();

        if (g_ashd[i].rgbaVolume.bAlpha == 0x80)
            g_ashd[i].rgbaVolume.bAlpha = 0xFF;

		g_ashd[i].grfzon    = pbis->U32Read();
		g_ashd[i].oidAltSat = pbis->U16Read();
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
    for (uint16_t i = 0; i < 0x100; i += 0x20) {
        for (uint16_t j = i; j < i + 8; j++) {
            csm1ClutIndices[j] = static_cast<uint8_t>(j);
            csm1ClutIndices[j + 8] = static_cast<uint8_t>(j) + 0x10;
            csm1ClutIndices[j + 0x10] = static_cast<uint8_t>(j) + 0x8;
            csm1ClutIndices[j + 0x18] = static_cast<uint8_t>(j) + 0x18;
        }
    }

    textureDataStart = pbis->file.tellg();

    for (int i = 0; i < g_ashd.size(); i++)
    {
        for (int a = 0; a < g_ashd[i].atex.size(); a++)
        {
            bool is1Img1Pal       = ((g_ashd[i].atex[a].bmpIndex.size() == 1) && (g_ashd[i].atex[a].clutIndex.size() == 1));
            bool is1ImgManyPal    = ((g_ashd[i].atex[a].bmpIndex.size() == 1) && (g_ashd[i].atex[a].clutIndex.size() >  1));
            bool isManyImgManyPal = ((g_ashd[i].atex[a].bmpIndex.size() >  1) && (g_ashd[i].atex[a].clutIndex.size() >  1));

            if (is1Img1Pal)
                MakeTexture(g_ashd[i].glDiffuseTexture, g_ashd[i].atex[a].clutIndex[0], g_ashd[i].atex[a].bmpIndex[0], pbis);
            
            else if (is1ImgManyPal)
            {
                if (g_ashd[i].atex[a].clutIndex.size() == 3)
                {
                    MakeTexture(g_ashd[i].glAmbientTexture,   g_ashd[i].atex[a].clutIndex[0], g_ashd[i].atex[a].bmpIndex[0], pbis);
                    MakeTexture(g_ashd[i].glDiffuseTexture,   g_ashd[i].atex[a].clutIndex[1], g_ashd[i].atex[a].bmpIndex[0], pbis);
                    MakeTexture(g_ashd[i].glGreyScaleTexture, g_ashd[i].atex[a].clutIndex[2], g_ashd[i].atex[a].bmpIndex[0], pbis);
                }

                else
                    MakeTexture(g_ashd[i].glDiffuseTexture, g_ashd[i].atex[a].clutIndex[1], g_ashd[i].atex[a].bmpIndex[0], pbis);
            }
            
            else if (isManyImgManyPal)
                MakeTexture(g_ashd[i].glDiffuseTexture, g_ashd[i].atex[a].clutIndex[1], g_ashd[i].atex[a].bmpIndex[1], pbis);
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

void MakeTexture(GLuint &textureReference, int16_t clutIndex, int16_t bmpIndex, CBinaryInputStream* pbis)
{
    if (clutIndex >= g_aclut.size() || bmpIndex >= g_abmp.size())
        return;

    std::vector <byte> image;
    std::vector <byte> pallete;
    std::vector <byte> texture;

    image = MakeBmp(bmpIndex, pbis);
    pallete = MakePallete(clutIndex, pbis);

    short width = g_abmp[bmpIndex].bmpWidth;
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
            texture[4 * i + 3] = pallete[index + 3];

            if (texture[4 * i + 3] == 0x80)
                texture[4 * i + 3] = 0xFF;
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
            texture[8 * i + 3] = pallete[4 * index1 + 3];

            if (texture[8 * i + 3] == 0x80)
                texture[8 * i + 3] = 0xFF;

            texture[8 * i + 4] = pallete[4 * index2 + 0];
            texture[8 * i + 5] = pallete[4 * index2 + 1];
            texture[8 * i + 6] = pallete[4 * index2 + 2];
            texture[8 * i + 7] = pallete[4 * index2 + 3];

            if (texture[8 * i + 7] == 0x80)
                texture[8 * i + 7] = 0xFF;
        }
    }

    glGenTextures(1, &textureReference);
    glBindTexture(GL_TEXTURE_2D, textureReference);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    textureReferences.push_back(&textureReference);
}