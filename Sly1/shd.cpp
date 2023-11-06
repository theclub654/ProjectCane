#include "shd.h"

void UnloadShaders()
{
	//g_pfont = (CFont*)&g_fontDebug;
	g_cshd = 0;
	g_ashd.clear();
	g_cbmp = 0;
	g_abmp.clear();
	g_cclut = 0;
	g_aclut.clear();
	g_cfontBrx = 0;
	g_afontBrx = 0x0;
	g_grfzonShaders = 0;
	g_cpsaa = 0;
	g_apsaa.clear();
}

void LoadColorTablesFromBrx(CBinaryInputStream *pbis)
{
	// Loads the number of CLUT propertys from binary file
	g_cclut = pbis->U16Read();

	g_aclut.resize(g_cclut);

	// Loading CLUT propertys from binary file
	for (int i = 0; i < g_cclut; i++)
	{
		g_aclut[i].grfzon = pbis->U32Read();
		g_aclut[i].numColors = pbis->U16Read();
		g_aclut[i].colorSize = pbis->U16Read();
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
		g_abmp[i].bmpWidth = pbis->U16Read();
		g_abmp[i].bmpheight = pbis->U16Read();
		g_abmp[i].grfzon = pbis->U32Read();
		g_abmp[i].psm = pbis->S8Read();
		g_abmp[i].cgsRow = pbis->S8Read();
		g_abmp[i].cgsPixels = pbis->U16Read();
		g_abmp[i].cbPixels = pbis->U32Read();
		g_abmp[i].baseOffset = pbis->U32Read();
	}
}

void LoadFontsFromBrx(CBinaryInputStream* pbis)
{
	// Loading number of fonts from file
	g_cfontBrx = pbis->U16Read();

	g_afontBrx = (CFontBrx*)malloc(sizeof(CFontBrx));
	
	g_afontBrx->m_grffont = g_cfontBrx;

	// Loading font property's from binary file
	for (int i = 0; i < g_cfontBrx ; i++)
		g_afontBrx->LoadFromBrx(pbis);

	if (g_cfontBrx != 0)
	{
		g_pfont = &g_afontBrx->CFont;

		if (g_cfontBrx < 2)
			g_pfontScreenCounters = &g_afontBrx->CFont;

		if (g_cfontBrx < 3) 
			g_pfontJoy = &g_afontBrx->CFont;
	}

}

void LoadTexFromBrx(CBinaryInputStream *pbis, SHD shader)
{
	TEX tex;

	tex.pshd = shader;
	tex.texf.oid = pbis->U16Read();
	tex.texf.grftex = pbis->S16Read();
	tex.texf.cibmp = pbis->U8Read();
	tex.texf.ciclut = pbis->U8Read();

	tex.texf.bmpIndex.resize(tex.texf.cibmp);

	for (int i = 0; i < tex.texf.cibmp; i++)
		tex.texf.bmpIndex[i] = pbis->U16Read();

	tex.texf.clutIndex.resize(tex.texf.ciclut);

	for (int i = 0; i < tex.texf.ciclut; i++)
		tex.texf.clutIndex[i] = pbis->U16Read();

	g_atex.push_back(tex);
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
		SHD shader;

		shader.SHD.shdk = pbis->U8Read();
		shader.SHD.grfshd = pbis->U8Read();
		shader.SHD.oid = pbis->U16Read();

		shader.SHD.rgba.bRed = pbis->U8Read();
		shader.SHD.rgba.bGreen = pbis->U8Read();
		shader.SHD.rgba.bBlue = pbis->U8Read();
		shader.SHD.rgba.bAlpha = pbis->U8Read();

		shader.SHD.rgbaVolume.bRed = pbis->U8Read();
		shader.SHD.rgbaVolume.bGreen = pbis->U8Read();
		shader.SHD.rgbaVolume.bBlue = pbis->U8Read();
		shader.SHD.rgbaVolume.bAlpha = pbis->U8Read();

		shader.SHD.grfzon = pbis->U32Read();
		shader.SHD.oidAltSat = pbis->U16Read();
		shader.SHD.rp = pbis->U8Read();
		shader.SHD.ctex = pbis->U8Read();

		// Reading shader animation from file
		PsaaLoadFromBrx(pbis);

		// Reading texture tables from file
		for (int i = 0; i < shader.SHD.ctex; i++)
			LoadTexFromBrx(pbis, shader);
	}

	LoadFontsFromBrx(pbis);
}