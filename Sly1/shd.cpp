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
	g_afontBrx.resize(g_cfontBrx);

	// Loading font property's from binary file
	for (int i = 0; i < g_cfontBrx ; i++)
		g_afontBrx[i].LoadFromBrx(pbis);
}

void LoadTexFromBrx(CBinaryInputStream *pbis, TEX *ptex)
{
	ptex->oid = pbis->U16Read();
	ptex->grftex = pbis->S16Read();
	ptex->cibmp = pbis->U8Read();
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
		g_ashd[i].shdk = pbis->U8Read();
		g_ashd[i].grfshd = pbis->U8Read();
		g_ashd[i].oid = pbis->U16Read();

		g_ashd[i].rgba = (RGBA)pbis->U32Read();

		g_ashd[i].rgbaVolume = (RGBA)pbis->U32Read();

		g_ashd[i].grfzon = pbis->U32Read();
		g_ashd[i].oidAltSat = pbis->U16Read();
		g_ashd[i].rp = pbis->U8Read();
		g_ashd[i].ctex = pbis->U8Read();

		g_ashd[i].atex.resize(g_ashd[i].ctex);

		// Reading shader animation from file
		PsaaLoadFromBrx(pbis);

		// Reading texture tables from file
		for (int a = 0; a < g_ashd[i].ctex; a++)
			LoadTexFromBrx(pbis, &g_ashd[i].atex[a]);
	}

	LoadFontsFromBrx(pbis);
}
