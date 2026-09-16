#include "render.h"
#include "main.h"
#include "blip.h"
#include "emitter.h"
#include "gl.h"
#include "tv.h"
#include "thndflash.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <vector>

void SetGlobDraw(GLOB* pglob)
{
	switch (pglob->rp)
	{
		case RP_DynamicTexture:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_Background:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_BlotContext:
		pglob->PFNDRAW = DrawTranslucent;
		break;

		case RP_Opaque:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_Cutout:
		pglob->PFNDRAW = DrawTranslucent;
		break;

		case RP_ProjVolume:
		switch (pglob->grfshd)
		{
			case 0:
			pglob->PFNDRAW = DrawProjVolume;
			break;

			case 2:
			pglob->PFNDRAW = DrawProjVolumeAlphaAdd;
			break;

			case 1:
			case 3:
			pglob->PFNDRAW = DrawProjVolumeAdd;
			break;
		}
		break;

		case RP_OpaqueAfterProjVolume:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_CutoutAfterProjVolume:
		pglob->PFNDRAW = DrawTranslucent;
		break;

		case RP_MurkClear:
		pglob->PFNDRAW = DrawMurkClear;
		break;

		case RP_MurkOpaque:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_MurkFill:
		pglob->PFNDRAW = DrawMurkFill;
		break;

		case RP_Translucent:
		pglob->PFNDRAW = DrawTranslucent;
		break;

		/*case RP_Blip:
		pglob->PFNDRAW = Draw;
		break;*/

		case RP_Foreground:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_WorldMap:
		pglob->PFNDRAW = DrawSubGlob;
		break;

		case RP_Max:
		pglob->PFNDRAW = DrawSubGlob;
		break;
	}
}

void AllocateRpl()
{
	g_dynamicTexturePrpl.resize(2048);
	g_backGroundPrpl.resize(2048);
	g_backGroundBlendPrpl.resize(2048);
	g_blotContextPrpl.resize(2048);
	g_opaquePrpl.resize(2048);
	g_cutOutBlendAddPrpl.resize(2048);
	g_cutOutPrpl.resize(2048);
	g_celBorderPrpl.resize(2048);
	g_projVolumePrpl.resize(2048);
	g_projVolumeAlphaAddPrpl.resize(2048);
	g_projVolumeAddPrpl.resize(2048);
	g_opaqueAfterProjVolumePrpl.resize(2048);
	g_cutOutAfterProjVolumeAddPrpl.resize(2048);
	g_cutOutAfterProjVolumePrpl.resize(2048);
	g_celBorderAfterProjVolumePrpl.resize(2048);
	g_murkClearPrpl.resize(2048);
	g_murkOpaquePrpl.resize(2048);
	g_murkFillPrpl.resize(2048);
	g_translucentAddPrpl.resize(2048);
	g_translucentPrpl.resize(2048);
	g_translucentCelBorderPrpl.resize(2048);
	g_blipPrpl.resize(2048);
	g_foreGroundPrpl.resize(2048);
	g_worldMapPrpl.resize(2048);
	g_maxPrpl.resize(2048);
	
	g_dynamicTextureCount = 0;
	g_backGroundCount = 0;
	g_backGroundBlendCount = 0;
	g_blotContextCount = 0;
	g_opaqueCount = 0;
	g_cutOutBlendAddCount = 0;
	g_cutOutCount = 0;
	g_celBorderCount = 0;
	g_projVolumeCount = 0;
	g_projVolumeAlphaAddCount = 0;
	g_projVolumeAddCount = 0;
	g_opaqueAfterProjVolumeCount = 0;
	g_cutOutAfterProjVolumeAddCount = 0;
	g_cutOutAfterProjVolumeCount = 0;
	g_celBorderAfterProjVolumeCount = 0;
	g_murkClearCount = 0;
	g_murkOpaqueCount = 0;
	g_murkFillCount = 0;
	g_translucentAddCount = 0;
	g_translucentCount = 0;
	g_translucentCelBorderCount = 0;
	g_blipCount = 0;
	g_foreGroundCount = 0;
	g_worldMapCount = 0;
}

void RenderSw(SW* psw, CM* pcm)
{
	DLI dlBusyWalker;
	dlBusyWalker.m_pdl = &psw->dlBusy;
	dlBusyWalker.m_ibDle = psw->dlBusy.ibDle;
	dlBusyWalker.m_pdliNext = s_pdliFirst;

	LO* currentObject = psw->dlBusy.ploFirst;

	dlBusyWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle) : nullptr;

	s_pdliFirst = &dlBusyWalker;

	while (currentObject != nullptr)
	{
		currentObject->pvtalo->pfnRenderAloAll((ALO*)currentObject, pcm, nullptr);

		currentObject = reinterpret_cast<LO*>(*dlBusyWalker.m_ppv);
		dlBusyWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle) : nullptr;
	}

	s_pdliFirst = dlBusyWalker.m_pdliNext;

	DLI dlRealClockWalker;
	dlRealClockWalker.m_pdl = &psw->dlMRDRealClock;
	dlRealClockWalker.m_ibDle = psw->dlMRDRealClock.ibDle;
	dlRealClockWalker.m_pdliNext = s_pdliFirst;

	currentObject = psw->dlMRDRealClock.ploFirst;

	dlRealClockWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlRealClockWalker.m_ibDle) : nullptr;

	s_pdliFirst = &dlRealClockWalker;

	while (currentObject != nullptr)
	{
		currentObject->pvtalo->pfnRenderAloAll((ALO*)currentObject, pcm, nullptr);

		currentObject = reinterpret_cast<LO*>(*dlRealClockWalker.m_ppv);
		dlRealClockWalker.m_ppv = (currentObject != nullptr) ? reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlRealClockWalker.m_ibDle) : nullptr;
	}

	s_pdliFirst = dlRealClockWalker.m_pdliNext;

	DLI dlThndFlashWalker;
	dlThndFlashWalker.m_pdl = &psw->dlThndFlash;
	dlThndFlashWalker.m_ibDle = psw->dlThndFlash.ibDle;
	dlThndFlashWalker.m_pdliNext = s_pdliFirst;

	THNDFLASH* pthndflash = static_cast<THNDFLASH*>(psw->dlThndFlash.pvFirst);
	dlThndFlashWalker.m_ppv = pthndflash != nullptr
		? reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pthndflash) + dlThndFlashWalker.m_ibDle)
		: nullptr;
	s_pdliFirst = &dlThndFlashWalker;

	while (pthndflash != nullptr)
	{
		RenderThndFlash(pthndflash);

		pthndflash = static_cast<THNDFLASH*>(*dlThndFlashWalker.m_ppv);
		dlThndFlashWalker.m_ppv = pthndflash != nullptr
			? reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pthndflash) + dlThndFlashWalker.m_ibDle)
			: nullptr;
	}

	s_pdliFirst = dlThndFlashWalker.m_pdliNext;

}

void RenderSwAloAll(SW* psw, CM* pcm)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
	{
		CID cid = allSWAloObjs[i]->pvtalo->cid;

		allSWAloObjs[i]->pvtalo->pfnRenderAloAll(allSWAloObjs[i], pcm, nullptr);
	}
}

void RenderSwGlobset(SW* psw, CM* pcm)
{
	for (int i = 0; i < 40; i++)
		allSWAloObjs[i]->pvtalo->pfnRenderAloGlobset(allSWAloObjs[i], pcm, nullptr);
}

void SubmitRpl(RPL* prpl)
{ 
	switch (prpl->rp)
	{
		case RP_DynamicTexture:
		g_dynamicTexturePrpl[g_dynamicTextureCount] = *prpl;
		g_dynamicTexturePrpl[g_dynamicTextureCount].PFNDRAWRPL = DrawDysh;
		g_dynamicTextureCount++;
		break;

		case RP_Background:
		if (prpl->fTransluscentSort == 1)
		{
			g_backGroundBlendPrpl[g_backGroundBlendCount] = *prpl;
			g_backGroundBlendPrpl[g_backGroundBlendCount].PFNDRAWRPL = DrawGlob;
			g_backGroundBlendCount++;
		}
		else
		{
			g_backGroundPrpl[g_backGroundCount] = *prpl;
			g_backGroundPrpl[g_backGroundCount].PFNDRAWRPL = DrawGlob;
			g_backGroundCount++;
		}
		break;

		case RP_BlotContext:
		g_blotContextPrpl[g_blotContextCount] = *prpl;
		// Blot-context entries are an ordered stream.  Ordinary ALO globs do
		// not supply a callback here and use DrawGlob, while TV context entries
		// supply PreTvContext/PostTvContext and must retain those callbacks.
		if (g_blotContextPrpl[g_blotContextCount].PFNDRAWRPL == nullptr)
			g_blotContextPrpl[g_blotContextCount].PFNDRAWRPL = DrawGlob;
		g_blotContextCount++;
		break;

		case RP_Opaque:
		g_opaquePrpl[g_opaqueCount] = *prpl;
		g_opaquePrpl[g_opaqueCount].PFNDRAWRPL = DrawGlob;
		g_opaqueCount++;
		break;

		case RP_Cutout:
		if (prpl->fTransluscentSort == 1)
		{
			g_cutOutPrpl[g_cutOutCount] = *prpl;
			g_cutOutPrpl[g_cutOutCount].PFNDRAWRPL = DrawGlob;
			g_cutOutCount++;
		}
		else
		{
			g_cutOutBlendAddPrpl[g_cutOutBlendAddCount] = *prpl;
			g_cutOutBlendAddPrpl[g_cutOutBlendAddCount].PFNDRAWRPL = DrawGlob;
			g_cutOutBlendAddCount++;
		}
		break;

		case RP_ProjVolume:
		switch (prpl->pglob->grfshd)
		{
			case 0:
			g_projVolumePrpl[g_projVolumeCount] = *prpl;
			g_projVolumePrpl[g_projVolumeCount].PFNDRAWRPL = DrawGlob;
			g_projVolumeCount++;
			break;

			case 2:
			g_projVolumeAlphaAddPrpl[g_projVolumeAlphaAddCount] = *prpl;
			g_projVolumeAlphaAddPrpl[g_projVolumeAlphaAddCount].PFNDRAWRPL = DrawGlob;
			g_projVolumeAlphaAddCount++;
			break;

			case 1:
			case 3:
			g_projVolumeAddPrpl[g_projVolumeAddCount] = *prpl;
			g_projVolumeAddPrpl[g_projVolumeAddCount].PFNDRAWRPL = DrawGlob;
			g_projVolumeAddCount++;
			break;
		}
		break;

		case RP_OpaqueAfterProjVolume:
		g_opaqueAfterProjVolumePrpl[g_opaqueAfterProjVolumeCount] = *prpl;
		g_opaqueAfterProjVolumePrpl[g_opaqueAfterProjVolumeCount].PFNDRAWRPL = DrawGlob;
		g_opaqueAfterProjVolumeCount++;
		break;

		case RP_CutoutAfterProjVolume:
		if (prpl->fTransluscentSort == 1)
		{
			g_cutOutAfterProjVolumePrpl[g_cutOutAfterProjVolumeCount] = *prpl;
			g_cutOutAfterProjVolumePrpl[g_cutOutAfterProjVolumeCount].PFNDRAWRPL = DrawGlob;
			g_cutOutAfterProjVolumeCount++;
		}
		else
		{
			g_cutOutAfterProjVolumeAddPrpl[g_cutOutAfterProjVolumeAddCount] = *prpl;
			g_cutOutAfterProjVolumeAddPrpl[g_cutOutAfterProjVolumeAddCount].PFNDRAWRPL = DrawGlob;
			g_cutOutAfterProjVolumeAddCount++;
		}
		break;

		case RP_MurkClear:
		g_murkClearPrpl[g_murkClearCount] = *prpl;
		g_murkClearPrpl[g_murkClearCount].PFNDRAWRPL = DrawGlob;
		g_murkClearCount++;
		break;

		case RP_MurkOpaque:
		g_murkOpaquePrpl[g_murkOpaqueCount] = *prpl;
		g_murkOpaquePrpl[g_murkOpaqueCount].PFNDRAWRPL = DrawGlob;
		g_murkOpaqueCount++;
		break;

		case RP_MurkFill:
		g_murkFillPrpl[g_murkFillCount] = *prpl;
		g_murkFillPrpl[g_murkFillCount].PFNDRAWRPL = DrawGlob;
		g_murkFillCount++;
		break;

		case RP_Translucent:
		if (prpl->fTransluscentSort == 1)
		{
			g_translucentPrpl[g_translucentCount] = *prpl;
			g_translucentPrpl[g_translucentCount].PFNDRAWRPL = DrawGlob;
			g_translucentCount++;
		}
		else
		{
			g_translucentAddPrpl[g_translucentAddCount] = *prpl;
			g_translucentAddPrpl[g_translucentAddCount].PFNDRAWRPL = DrawGlob;
			g_translucentAddCount++;
		}
		break;

		case RP_Blip:
		g_blipPrpl[g_blipCount] = *prpl;
		g_blipPrpl[g_blipCount].PFNDRAWRPL = DrawBlip;
		g_blipCount++;
		break;

		case RP_Foreground:
		g_foreGroundPrpl[g_foreGroundCount] = *prpl;
		if (g_foreGroundPrpl[g_foreGroundCount].PFNDRAWRPL == nullptr)
			g_foreGroundPrpl[g_foreGroundCount].PFNDRAWRPL = DrawGlob;
		g_foreGroundCount++;
		break;

		case RP_WorldMap:
		g_worldMapPrpl[g_worldMapCount] = *prpl;
		g_worldMapPrpl[g_worldMapCount].PFNDRAWRPL = DrawGlob;
		g_worldMapCount++;
		break;

		case RP_Max:
		g_maxPrpl[g_maxCount] = *prpl;
		g_maxPrpl[g_maxCount].PFNDRAWRPL = DrawGlob;
		g_maxCount++;
		break;
	}

	g_cFrameGlobs++;
	//numFrameObjs += prpl->pglob->asubglob.size();
}

void SubmitRplCel(RPLCEL* prplcel)
{
	switch (prplcel->rp)
	{
		case RP_CelBorder:
		g_celBorderPrpl[g_celBorderCount] = *prplcel;
		g_celBorderCount++;
		break;

		case RP_CelBorderAfterProjVolume:
		g_celBorderAfterProjVolumePrpl[g_celBorderAfterProjVolumeCount] = *prplcel;
		g_celBorderAfterProjVolumeCount++;
		break;

		case RP_TranslucentCelBorder:
		g_translucentCelBorderPrpl[g_translucentCelBorderCount] = *prplcel;
		g_translucentCelBorderCount++;
		break;
	}

	g_cFrameCelGlobs++;
	//numFrameObjs++;
}

void SortRenderRpl()
{
	if (g_backGroundBlendCount > 1)
		std::stable_sort(g_backGroundBlendPrpl.begin(), g_backGroundBlendPrpl.begin() + g_backGroundBlendCount, compareZ);

	if (g_cutOutCount > 1)
		std::stable_sort(g_cutOutPrpl.begin(), g_cutOutPrpl.begin() + g_cutOutCount, compareZ);

	if (g_cutOutAfterProjVolumeCount > 1)
		std::stable_sort(g_cutOutAfterProjVolumePrpl.begin(), g_cutOutAfterProjVolumePrpl.begin() + g_cutOutAfterProjVolumeCount, compareZ);

	if (g_translucentCount > 1)
		std::stable_sort(g_translucentPrpl.begin(), g_translucentPrpl.begin() + g_translucentCount, compareZ);
}

inline bool compareZ(const RPL& prpl0, const RPL& prpl1)
{
	return prpl0.z > prpl1.z;
}

void DrawSw(SW* psw, CM* pcm)
{
	/*std::cout << numFrameObjs << "\n";*/
	//numFrameObjs = 0;

	glGlobShader.Use();

	//std::cout << g_cFrameGlobs << "\n";
	BeginFrameStream(&ropStream);

	bool anyCel = g_cFrameCelGlobs > 0;

	if (anyCel == true)
		BeginFrameStream(&rcbStream);
	
	SortRenderRpl();

	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &pcm->matWorldToClip);

	PrepareSwLights(psw, pcm);
	PrepareSwShadows(psw, pcm);

	// OpenGL line widths are measured in render-target pixels. Scale the
	// authored cel-border width with the internal resolution so resolving the
	// scene back to the window does not make outlines appear thinner.
	const float celBorderRasterScale = static_cast<float>(g_gl.renderHeight) /
		static_cast<float>((std::max)(g_gl.presentHeight, 1));
	glLineWidth(4.125f * celBorderRasterScale);
	glEnable(GL_CULL_FACE);


	if (g_dynamicTextureCount > 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, g_gl.dyshFbo);
		glViewport(0, 0, g_gl.dyshWidth, g_gl.dyshHeight);

		/*glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		glDyshadow.Use();

		for (int i = 0; i < g_dynamicTextureCount; i++)
			g_dynamicTexturePrpl[i].PFNDRAWRPL(&g_dynamicTexturePrpl[i]);

		glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);
		glViewport(0, 0, g_gl.renderWidth, g_gl.renderHeight);

		//glDisable(GL_BLEND);

		glDepthFunc(GL_LESS);
		glDepthMask(true);

		glGlobShader.Use();

		g_dynamicTextureCount = 0;
	}

	if (g_backGroundCount > 0)
	{
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundCount; i++)
			g_backGroundPrpl[i].PFNDRAWRPL(&g_backGroundPrpl[i]);

		glDepthFunc(GL_LESS);
		glDepthMask(true);

		g_backGroundCount = 0;
	}

	if (g_backGroundBlendCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundBlendCount; i++)
			g_backGroundBlendPrpl[i].PFNDRAWRPL(&g_backGroundBlendPrpl[i]);

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glDepthMask(true);

		g_backGroundBlendCount = 0;
	}

	if (g_blotContextCount > 0)
	{
		// TV lighting is scoped by PreTvContext/PostTvContext entries inside
		// this ordered list. Never carry an unfinished context across frames.
		g_ptvDrawContext = nullptr;

		for (int i = 0; i < g_blotContextCount; i++)
			g_blotContextPrpl[i].PFNDRAWRPL(&g_blotContextPrpl[i]);

		g_ptvDrawContext = nullptr;
		g_blotContextCount = 0;

	}

	if (g_opaqueCount > 0)
	{
		for (int i = 0; i < g_opaqueCount; i++)
			g_opaquePrpl[i].PFNDRAWRPL(&g_opaquePrpl[i]);

		g_opaqueCount = 0;
	}

	if (g_cutOutBlendAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutBlendAddCount; i++)
			g_cutOutBlendAddPrpl[i].PFNDRAWRPL(&g_cutOutBlendAddPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutBlendAddCount = 0;
	}

	if (g_cutOutCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutCount; i++)
			g_cutOutPrpl[i].PFNDRAWRPL(&g_cutOutPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutCount = 0;
	}

	if (g_celBorderCount > 0)
	{
		glCelBorderShader.Use();

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glEnable(GL_STENCIL_TEST);
		glStencilMask(0x40);
		glStencilFunc(GL_ALWAYS, 0x40, 0x40);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);

		for (int i = 0; i < g_celBorderCount; i++)
			DrawCelBorder(&g_celBorderPrpl[i]);

		glStencilMask(0xFF);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		glGlobShader.Use();

		g_celBorderCount = 0;
	}

	if (g_projVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LESS);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeCount; i++)
			g_projVolumePrpl[i].PFNDRAWRPL(&g_projVolumePrpl[i]);

		glStencilMask(0xFF);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeCount = 0;
	}

	if (g_projVolumeAlphaAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LESS);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeAlphaAddCount; i++)
			g_projVolumeAlphaAddPrpl[i].PFNDRAWRPL(&g_projVolumeAlphaAddPrpl[i]);

		glStencilMask(0xFF);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeAlphaAddCount = 0;
	}

	if (g_projVolumeAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LESS);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeAddCount; i++)
		{
			g_grfshd = g_projVolumeAddPrpl[i].pglob->grfshd;
			g_projVolumeAddPrpl[i].PFNDRAWRPL(&g_projVolumeAddPrpl[i]);
		}

		glStencilMask(0xFF);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeAddCount = 0;
	}

	if (g_opaqueAfterProjVolumeCount > 0)
	{
		for (int i = 0; i < g_opaqueAfterProjVolumeCount; i++)
			g_opaqueAfterProjVolumePrpl[i].PFNDRAWRPL(&g_opaqueAfterProjVolumePrpl[i]);

		g_opaqueAfterProjVolumeCount = 0;
	}

	if (g_cutOutAfterProjVolumeAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutAfterProjVolumeAddCount; i++)
			g_cutOutAfterProjVolumeAddPrpl[i].PFNDRAWRPL(&g_cutOutAfterProjVolumeAddPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutAfterProjVolumeAddCount = 0;
	}

	if (g_cutOutAfterProjVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_cutOutAfterProjVolumeCount; i++)
			g_cutOutAfterProjVolumePrpl[i].PFNDRAWRPL(&g_cutOutAfterProjVolumePrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutAfterProjVolumeCount = 0;
	}

	if (g_celBorderAfterProjVolumeCount > 0)
	{
		glCelBorderShader.Use();

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_celBorderAfterProjVolumeCount; i++)
			DrawCelBorder(&g_celBorderAfterProjVolumePrpl[i]);

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		glGlobShader.Use();

		g_celBorderAfterProjVolumeCount = 0;
	}

	if (g_murkClearCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_murkClearCount; i++)
			g_murkClearPrpl[i].PFNDRAWRPL(&g_murkClearPrpl[i]);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		g_murkClearCount = 0;
	}

	if (g_murkOpaqueCount > 0)
	{
		glUniform1i(glslfAlphaTest, 1);
		glUniform1f(glslAlphaCutOff, 0.1);

		for (int i = 0; i < g_murkOpaqueCount; i++)
			g_murkOpaquePrpl[i].PFNDRAWRPL(&g_murkOpaquePrpl[i]);

		glUniform1i(glslfAlphaTest, 0);

		g_murkOpaqueCount = 0;
	}

	if (g_murkFillCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(false);
		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_murkFillCount; i++)
			g_murkFillPrpl[i].PFNDRAWRPL(&g_murkFillPrpl[i]);

		glDisable(GL_BLEND);
		glDepthMask(true);
		glDepthFunc(GL_LESS);

		g_murkFillCount = 0;
	}

	if (g_translucentAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_translucentAddCount; i++)
			g_translucentAddPrpl[i].PFNDRAWRPL(&g_translucentAddPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_translucentAddCount = 0;
	}

	if (g_translucentCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1f(glslAlphaCutOff, 0.9);

		for (int i = 0; i < g_translucentCount; i++)
			g_translucentPrpl[i].PFNDRAWRPL(&g_translucentPrpl[i]);

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_translucentCount = 0;
	}

	if (g_translucentCelBorderCount > 0)
	{
		glCelBorderShader.Use();

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_translucentCelBorderCount; i++)
			DrawCelBorder(&g_translucentCelBorderPrpl[i]);

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		glGlobShader.Use();

		g_translucentCelBorderCount = 0;
	}

	if (g_blipCount > 0)
	{
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(false);
		glDisable(GL_CULL_FACE);

		glBlipShader.Use();
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, cmUBO);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(g_gl.blipVao);
		glBindBuffer(GL_ARRAY_BUFFER, g_gl.blipInstanceVbo);
		BeginFrameStream(&blipStream);

		glUniform1f(glslBlipCurrentTime, g_clock.t);
		glUniform1f(glslBlipDtFrame, g_clock.dt);
		glUniformMatrix3fv(glslBlipCameraMat, 1, GL_FALSE, glm::value_ptr(g_pcm->mat));

		for (int i = 0; i < g_blipCount; i++)
			g_blipPrpl[i].PFNDRAWRPL(&g_blipPrpl[i]);

		EndFrameStream(&blipStream);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDepthMask(true);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glGlobShader.Use();

		g_blipCount = 0;
	}

	if (g_foreGroundCount > 0)
	{
		// Foreground objects are camera/UI composites (including confrontation
		// TV speakers), not members of the level depth hierarchy.
		glDepthFunc(GL_ALWAYS);
		glDepthMask(GL_FALSE);

		for (int i = 0; i < g_foreGroundCount; i++)
			g_foreGroundPrpl[i].PFNDRAWRPL(&g_foreGroundPrpl[i]);

		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		g_foreGroundCount = 0;
	}

	if (g_worldMapCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glDepthMask(false);
		glDepthFunc(GL_ALWAYS);

		for (int i = 0; i < g_worldMapCount; i++)
		{
			g_worldMapPrpl[i].PFNDRAWRPL(&g_worldMapPrpl[i]);
		}

		glDepthMask(true);
		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		g_worldMapCount = 0;
	}

	if (g_maxCount > 0)
	{
		for (int i = 0; i < g_maxCount; i++)
			g_maxPrpl[i].PFNDRAWRPL(&g_maxPrpl[i]);

		g_maxCount = 0;
	}

	EndFrameStream(&ropStream);

	if (anyCel == true)
		EndFrameStream(&rcbStream);

	g_cFrameGlobs = 0;
	g_cFrameCelGlobs = 0;

	glDisable(GL_CULL_FACE);
	glClearStencil(0x00);
}

void FillScreenRect(int r, int g, int b, int alpha, float xLeft, float yTop, float xRight, float yBottom, bool fAdditive)
{
	const float width = xRight - xLeft;
	const float height = yBottom - yTop;

	if (width <= 0.0f || height <= 0.0f || alpha <= 0)
		return;

	const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(xLeft, yTop, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
	const glm::vec4 color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(alpha) / 255.0f);
	const GLboolean fCullEnabled = glIsEnabled(GL_CULL_FACE);
	const GLboolean fDepthEnabled = glIsEnabled(GL_DEPTH_TEST);
	const GLboolean fBlendEnabled = glIsEnabled(GL_BLEND);
	GLint blendEquationRgb = GL_FUNC_ADD;
	GLint blendEquationAlpha = GL_FUNC_ADD;
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEquationRgb);
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEquationAlpha);

	glBlotShader.Use();

	glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
	glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
	glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));

	// Screen-space overlays may be submitted from a 3D render pass.  Do not let
	// that pass's culling, depth, or subtractive blend state discard the quad.
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, fAdditive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, whiteTex);
	glUniform1i(u_fontTexLoc, 0);

	glBindVertexArray(g_gl.gao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);

	glBlendEquationSeparate(blendEquationRgb, blendEquationAlpha);
	if (!fBlendEnabled)
		glDisable(GL_BLEND);
	if (fDepthEnabled)
		glEnable(GL_DEPTH_TEST);
	if (fCullEnabled)
		glEnable(GL_CULL_FACE);
}

void DrawDysh(RPL *prpl)
{
	DYSH *pdysh = prpl->pdysh;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pdysh->shadowTex, 0);

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 matWorldToClip = g_uvToClip * pdysh->pshadowGen->matWorldToUv;

	glUniformMatrix4fv(glslDyshMatWorldClip, 1, GL_FALSE, glm::value_ptr(matWorldToClip));
	glUniformMatrix4fv(glslDyshModel, 1, GL_FALSE, glm::value_ptr(prpl->ro.model));

	GLOBSET* pglobset = &pdysh->globset;
	const bool fHasBones = pglobset->cbnd > 0 && pglobset->boneMatrixSSBO != 0;

	if (fHasBones)
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, pglobset->boneMatrixSSBO);

	for (int i = 0; i < pglobset->aglob.size(); i++)
	{
		GLOB& glob = pglobset->aglob[i];
		glBindVertexArray(glob.VAO);

		for (int a = 0; a < glob.asubglob.size(); a++)
		{
			SUBGLOB& sg = glob.asubglob[a];
			glUniform1i(glslDyshfSkin, fHasBones && sg.cibnd > 0 ? 1 : 0);
			glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)sg.indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(sg.firstIndex * sizeof(uint32_t)), (GLint)sg.baseVertex);
		}
	}

	glUniform1i(glslDyshfSkin, 0);
}

void DrawGlob(RPL* prpl)
{
	GLOB* pglob = prpl->pglob;
	const bool fQueuedCacheWrite = prpl->ro.trlk == TRLK_Relight;
	prpl->ro.blotTvLight = 0;

	// Retail evaluates three-way TV globs through the Quick path against the
	// final posed/skinned normals. The glob directly selects its persistent TV
	// light slot, completely bypassing the world's light selection.
	// pglob->pblot persists after a dialogue and is also present on full-screen
	// confrontation speakers. Only PreTvContext establishes an active isolated
	// TV-light domain; PostTvContext clears it again.
	if (g_ptvDrawContext != nullptr && pglob != nullptr &&
		pglob->fThreeWay == 1 &&
		pglob->pblot == static_cast<BLOT*>(g_ptvDrawContext))
	{
		if (g_ptvDrawContext == &g_tvLeft)
			prpl->ro.blotTvLight = 1;
		else if (g_ptvDrawContext == &g_tvRight)
			prpl->ro.blotTvLight = 2;

		if (prpl->ro.blotTvLight != 0)
		{
			prpl->ro.trlk = TRLK_Quick;
			prpl->ro.fDynamicLight = 0;
		}
	}

	AppendStream(&ropStream, &prpl->ro, sizeof(ROGL), sizeof(ROGL));

	GLOBSET* pglobset = prpl->palo != nullptr ? &prpl->palo->globset : nullptr;

	const bool fHasBones = pglobset != nullptr && pglobset->cbnd > 0 && pglobset->boneMatrixSSBO != 0;
	const bool fHasPoses = pglobset != nullptr && pglob->poseCount > 0 && pglob->poseDposSSBO != 0 && pglob->poseDnormalSSBO != 0 && !pglobset->agPoses.empty();

	glUniform1i(glslfPose, fHasPoses ? 1 : 0);

	if (fHasBones)
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, pglobset->boneMatrixSSBO);

	if (fHasPoses)
	{
		const int poseCount = pglob->poseCount;

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, pglob->poseDposSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, pglob->poseDnormalSSBO);
		glUniform1i(glslPoseCount, pglob->poseCount);
		glUniform1fv(glslPoseWeights, poseCount, pglobset->agPoses.data());
	}

	if (pglob->fThreeWay == 1)
	{
		if (prpl->ro.trlk == TRLK_Relight ||
			(prpl->ro.trlk == TRLK_Baked && prpl->ro.fDynamic == 0))
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, pglob->ssboCachedMaterial);
	}

	glBindVertexArray(pglob->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pglob->EBO);

	if (prpl->ro.warpType != WARP_NONE)
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, pglob->pwarpGlob->ssboState);

	for (int i = 0; i < static_cast<int>(pglob->asubglob.size()); ++i)
	{
		SUBGLOB& sub = pglob->asubglob[i];
		SAI* sai = sub.uvSai;
		int iframe = 0;

		if (sai != nullptr && sai->pshd != nullptr && sai->pshd->cframe > 0)
		{
			iframe = sai->iframe;

			if (iframe < 0)
				iframe = 0;
			else if (iframe >= sai->pshd->cframe)
				iframe = sai->pshd->cframe - 1;
		}

		const int fSkin = fHasBones && sub.cibnd > 0 ? 1 : 0;
		glUniform1i(glslfSkin, fSkin);

		const int animate = sai != nullptr && (sai->grfsai & 0x2) != 0 ? 1 : 0;

		glUniform1i(glslfAnimateUv, animate);

		if (animate)
			glUniform2f(glsluvOffsets, sai->tcx.du, sai->tcx.dv);

		SHD* pshd = sub.pshd;

		if (pshd != nullptr && !pshd->atex.empty() && !pshd->atex[0].abmp.empty())
		{
			if (iframe >= static_cast<int>(pshd->atex[0].abmp.size()))
				iframe = static_cast<int>(pshd->atex[0].abmp.size()) - 1;

			BMP* pbmp = pshd->atex[0].abmp[iframe];

			if (pbmp != nullptr)
			{
				if (pshd->shdk != SHDK_ThreeWay)
				{
					glUniform1i(glslRko, 0);
					const TEX& tex = pshd->atex[0];
					const uint64_t diffuseHandle =
						iframe < static_cast<int>(tex.hDiffuseMap.size()) && tex.hDiffuseMap[iframe] != 0
						? tex.hDiffuseMap[iframe]
						: pbmp->hDiffuseMap;
					glUniformHandleui64ARB(glslDiffuseMap, diffuseHandle);
				}
				else
				{
					glUniform1i(glslRko, 1);
					glUniformHandleui64ARB(glslAmbientMap, pbmp->hShadowMap);
					glUniformHandleui64ARB(glslDiffuseMap, pbmp->hDiffuseMap);
					glUniformHandleui64ARB(glslSaturateMap, pbmp->hSaturateMap);
				}
			}
		}

		if (activeShadows.numShadows > 0)
		{
			const glm::vec4 subGlobPosCenter = prpl->ro.model * glm::vec4(sub.posCenter, 1.0f);
			const float subGlobRadius = pglob->rSubglobRadius * sub.sRadius;

			glUniform3f(glslSubGlobPosCenter, subGlobPosCenter.x, subGlobPosCenter.y, subGlobPosCenter.z);
			glUniform1f(glslSubGlobRadius, subGlobRadius);
		}

		glUniform1f(glslUnSelfIllum, sub.unSelfIllum);
		pglob->PFNDRAW(sub.baseVertex, sub.firstIndex, sub.indexCount);
	}

	glUniform1i(glslfSkin, 0);
	glUniform1i(glslfPose, 0);

	if (prpl->ro.trlk == TRLK_Relight)
	{
		// Make vertex-shader SSBO writes visible before any later packet reads
		// this glob's cached three-way material.
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		if (pglob->trlk == TRLK_BakePending)
			pglob->trlk = TRLK_Baked;

	}
	else if (fQueuedCacheWrite && pglob->trlk == TRLK_BakePending)
	{
		// A TV draw can redirect a queued relight packet through TRLK_Quick.
		// No cache was written, so make the normal world draw try again.
		pglob->trlk = TRLK_Relight;
	}

}

void DrawCelBorder(RPLCEL* prplcel)
{
	AppendStream(&rcbStream, &prplcel->rocel, sizeof(ROCEL), sizeof(ROCEL));

	GLOB* pglob = prplcel->pglob;
	GLOBSET* pglobset = prplcel->palo != nullptr ? &prplcel->palo->globset : nullptr;

	const bool fSkin = pglobset != nullptr && pglobset->cbnd > 0 && pglobset->boneMatrixSSBO != 0 && pglob->edgeBoneIndicesSSBO != 0 && pglob->edgeBoneWeightsSSBO != 0;
	const bool fPose = pglobset != nullptr && pglob->poseCount > 0 && pglob->edgePoseDposSSBO != 0 && !pglobset->agPoses.empty();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, pglob->edgeSSBO);

	if (fSkin)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, pglobset->boneMatrixSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, pglob->edgeBoneIndicesSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, pglob->edgeBoneWeightsSSBO);
	}

	if (fPose)
	{
		const int cposeUpload = pglobset->agPoses.size();

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, pglob->edgePoseDposSSBO);
		glUniform1i(glslCelPoseCount, pglob->poseCount);
		glUniform1fv(glslCelPoseWeights, cposeUpload, pglobset->agPoses.data());
	}

	glUniform1i(glslCelSkin, fSkin ? 1 : 0);
	glUniform1i(glslCelPose, fPose ? 1 : 0);
	glDrawArrays(GL_LINES, 0, pglob->edgeCount * 2);
	glUniform1i(glslCelSkin, 0);
	glUniform1i(glslCelPose, 0);
}

void DrawSubGlob(int baseVertex, int firstIndex, int indexCount)
{
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawProjVolume(int baseVertex, int firstIndex, int indexCount)
{
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_ZERO, GL_REPLACE, GL_ZERO);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_ZERO, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glDepthFunc(GL_ALWAYS);
	// Bit 6 marks cel-border pixels.  Projection volumes use bit 7, and may
	// shade only pixels where the border-protection bit remains clear.
	glStencilFunc(GL_EQUAL, 128, 0xC0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawProjVolumeAlphaAdd(int baseVertex, int firstIndex, int indexCount)
{
	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_NONE, GL_REPLACE, GL_NONE);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_NONE, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 0xC0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawProjVolumeAdd(int baseVertex, int firstIndex, int indexCount)
{
	switch (g_grfshd)
	{
		case 1:
		glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
		break;

		case 3:
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		break;
	}

	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_ZERO, GL_REPLACE, GL_ZERO);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_ZERO, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 0xC0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawMurkClear(int baseVertex, int firstIndex, int indexCount)
{
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
	glFrontFace(GL_CW);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glFrontFace(GL_CCW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawMurkFill(int baseVertex, int firstIndex, int indexCount)
{
	glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA, GL_ONE, GL_NONE);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);
}

void DrawTranslucent(int baseVertex, int firstIndex, int indexCount)
{
	GLboolean depthWriteWasEnabled = GL_TRUE;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWriteWasEnabled);

	// Retail's translucent packet performs an alpha-tested depth-writing
	// pass followed by a blended pass which does not write depth.  The packet
	// must not leak that second pass's Z-write mask into the next glob.  Sly's
	// TV model mixes translucent and ordinary globs, so leaking GL_FALSE here
	// caused every glob after his first translucent piece to stop populating
	// depth and visually overlap into his face.
	glUniform1i(glslfAlphaTest, 1);
	glDepthMask(GL_TRUE);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glUniform1i(glslfAlphaTest, 0);
	glDepthMask(GL_FALSE);
	glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)(firstIndex * sizeof(uint32_t)), (GLint)baseVertex);

	glDepthMask(depthWriteWasEnabled);
}

void DrawBlip(RPL* prplblip)
{
	if (prplblip == nullptr || prplblip->pblipg == nullptr || g_pcm == nullptr)
		return;

	BLIPG* pblipg = prplblip->pblipg;
	if (pblipg->pshd == nullptr || pblipg->pshd->atex.empty() ||
		pblipg->pshd->atex[0].abmp.empty() || pblipg->cblipe <= 0)
		return;

	// DrawBlipg selected between the PS2's normal and clamped-add GIF
	// packets from shader flag bit 0.  Applying additive blending to every
	// BLIP makes effects such as jump/landing dust glow and disappear into
	// the scene, even though additive rain still happens to look plausible.
	const bool additiveBlip = (pblipg->pshd->grfshd & 1u) != 0;
	if (additiveBlip)
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
	else
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	struct alignas(16) BLIPGPU
	{
		glm::vec4 posCreated;
		glm::vec4 velocity;
		glm::vec4 color;
		glm::vec4 roll;
	};

	struct BLIPBATCH
	{
		GLuint texture = 0;
		std::vector<BLIPGPU> instances;
		GLuint baseInstance = 0;
	};

	// A BLIP texture table is capped at 32 entries by PropagateBlipgShader.
	// Retain these buffers between calls so particle rendering does not create
	// and destroy several vectors every frame.
	static BLIPBATCH batches[32];
	static std::vector<BLIPGPU> uploadInstances;
	static std::vector<BLIPGPU> orderedInstances;
	static std::vector<GLuint> orderedTextures;
	static std::vector<BLIPBATCH> orderedBatches;
	for (BLIPBATCH& batch : batches)
	{
		batch.texture = 0;
		batch.instances.clear();
		batch.baseInstance = 0;
	}
	uploadInstances.clear();
	orderedInstances.clear();
	orderedTextures.clear();
	orderedBatches.clear();

	const TEX& tex = pblipg->pshd->atex[0];
	const int textureTableCount = (std::min)(pblipg->cqwTexture, 32);
	if (textureTableCount <= 0)
		return;

	GLuint tableTextures[32]{};
	int tableBatches[32]{};
	int batchCount = 0;
	for (int tableIndex = 0; tableIndex < textureTableCount; ++tableIndex)
	{
		tableBatches[tableIndex] = -1;
		const int iframe = pblipg->aiframeTexture[tableIndex];
		if (iframe < 0 || iframe >= static_cast<int>(tex.abmp.size()) || tex.abmp[iframe] == nullptr)
			continue;

		BMP* const pframeBmp = tex.abmp[iframe];
		// The original BLIP texture packet is built with CTK_Ambient. For a
		// three-way shader that is the ambient/shadow texture, not diffuse.
		const bool threeWay = pblipg->pshd->shdk == SHDK_ThreeWay;
		// Non-three-way textures are palette-resolved per TEX/iframe during
		// LoadShadersFromBrx. The underlying BMP can be shared by TEX records
		// with different CLUTs, so its GL texture fields are intentionally zero.
		const GLuint textureId = threeWay
			? pframeBmp->glShadowMap
			: (iframe < static_cast<int>(tex.glDiffuseMap.size())
				? tex.glDiffuseMap[iframe] : 0);
		if (textureId == 0)
			continue;

		tableTextures[tableIndex] = textureId;
		int ibatch = 0;
		while (ibatch < batchCount && batches[ibatch].texture != textureId)
			++ibatch;
		if (ibatch == batchCount)
			batches[batchCount++].texture = textureId;
		tableBatches[tableIndex] = ibatch;
	}

	auto evaluateClq = [](const CLQ& clq, float t)
	{
		return clq.g0 + t * (clq.g1 + t * clq.g2);
	};

	for (BLIP* pblip = pblipg->dlBlip.pblipFirst; pblip != nullptr; pblip = pblip->dle.pblipNext)
	{
		const int iblipf = pblip->iblipfLatest & 1;
		const BLIPF& blipf = pblip->ablipf[iblipf];
		const int particleCount = pblip->cblipe;

		for (int i = 0; i < particleCount; ++i)
		{
			const BLIPP& pos = blipf.ablipp[i];
			const BLIPV& velocity = blipf.ablipv[i];
			const BLIPX& extra = blipf.ablipx[i];
			// VU1 evaluates every BLIP curve from the raw creation-time delta.
			// Emitters may place a creation time slightly ahead of the current
			// tick, so clamping here changes the first texture/color frame.
			const float age = g_clock.t - pos.tCreated;

			const int frameOffset = static_cast<int>(evaluateClq(pblipg->clqTexture, age));
			int tableIndex = static_cast<int>(extra.itex0) + frameOffset;
			if ((textureTableCount & (textureTableCount - 1)) == 0)
				tableIndex &= textureTableCount - 1;
			else
				tableIndex = ((tableIndex % textureTableCount) + textureTableCount) % textureTableCount;

			const int ibatch = tableBatches[tableIndex];
			if (ibatch < 0 || tableTextures[tableIndex] == 0)
				continue;

			glm::vec4 color(1.0f);
			if (pblipg->crgba > 0)
			{
				const int colorIndex = (std::min)(static_cast<int>(extra.irgba), pblipg->crgba - 1);
				color = pblipg->argba[colorIndex];
				if (pblipg->fColorRanges != 0 && colorIndex + 1 < pblipg->crgba)
				{
					const float u = glm::clamp(evaluateClq(pblipg->clqColor, age), 0.0f, 1.0f);
					color = glm::mix(color, pblipg->argba[colorIndex + 1], u);
				}
			}

			BLIPGPU instance{};
			instance.posCreated = glm::vec4(pos.x, pos.y, pos.z, pos.tCreated);
			instance.velocity = glm::vec4(velocity.x, velocity.y, velocity.z, 0.0f);
			instance.color = color;
			instance.roll = glm::vec4(extra.radRoll, extra.swRoll, 0.0f, 0.0f);
			if (additiveBlip)
				batches[ibatch].instances.push_back(instance);
			else
			{
				orderedInstances.push_back(instance);
				orderedTextures.push_back(tableTextures[tableIndex]);
			}
		}
	}

	if (additiveBlip)
	{
		for (int ibatch = 0; ibatch < batchCount; ++ibatch)
		{
			BLIPBATCH& batch = batches[ibatch];
			batch.baseInstance = static_cast<GLuint>(uploadInstances.size());
			uploadInstances.insert(uploadInstances.end(), batch.instances.begin(), batch.instances.end());
		}
	}
	else
	{
		// Alpha blending is order-dependent. Preserve the BLIP list order and
		// merge only adjacent particles which already reference the same frame.
		for (size_t i = 0; i < orderedInstances.size(); ++i)
		{
			if (orderedBatches.empty() || orderedBatches.back().texture != orderedTextures[i])
			{
				orderedBatches.emplace_back();
				orderedBatches.back().texture = orderedTextures[i];
				orderedBatches.back().baseInstance = static_cast<GLuint>(uploadInstances.size());
			}

			orderedBatches.back().instances.push_back(orderedInstances[i]);
			uploadInstances.push_back(orderedInstances[i]);
		}
	}

	if (uploadInstances.empty())
		return;

	BLIPGROUPGPU groupGpu{};
	groupGpu.model = prplblip->ro.model;
	groupGpu.params = glm::vec4(prplblip->ro.uAlpha, pblipg->rSFlying, static_cast<float>(pblipg->blipok), 0.0f);
	groupGpu.clqScale = glm::vec4(pblipg->clqScale.g0, pblipg->clqScale.g1, pblipg->clqScale.g2, 0.0f);
	groupGpu.clqAlpha = glm::vec4(pblipg->clqAlpha.g0, pblipg->clqAlpha.g1, pblipg->clqAlpha.g2, 0.0f);
	AppendStream(&blipStream, &groupGpu, sizeof(groupGpu), sizeof(groupGpu));

	const GLsizeiptr uploadSize = static_cast<GLsizeiptr>(uploadInstances.size() * sizeof(BLIPGPU));

	// Orphan the previous instance store before uploading this frame. This
	// avoids synchronizing against draws which may still consume the old store
	// and does not rely on unsynchronized mapped-buffer behavior.
	glBufferData(GL_ARRAY_BUFFER, uploadSize, nullptr, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, uploadSize, uploadInstances.data());
	g_gl.blipInstanceCapacity = uploadSize;

	auto drawBatches = [&]()
	{
		if (!additiveBlip)
		{
			for (const BLIPBATCH& batch : orderedBatches)
			{
				glBindTexture(GL_TEXTURE_2D, batch.texture);
				glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr,
					static_cast<GLsizei>(batch.instances.size()), batch.baseInstance);
			}
			return;
		}

		for (int ibatch = 0; ibatch < batchCount; ++ibatch)
		{
			const BLIPBATCH& batch = batches[ibatch];
			if (batch.instances.empty())
				continue;
			glBindTexture(GL_TEXTURE_2D, batch.texture);
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(batch.instances.size()), batch.baseInstance);
		}
	};

	if (!additiveBlip)
	{
		glUniform1i(glslBlipAlphaPass, 1);
		glDepthMask(GL_TRUE);
		drawBatches();

		glUniform1i(glslBlipAlphaPass, 2);
		glDepthMask(GL_FALSE);
		drawBatches();
	}
	else
	{
		glUniform1i(glslBlipAlphaPass, 0);
		glDepthMask(GL_FALSE);
		drawBatches();
	}

}

void DrawSwCollisionAll(CM* pcm)
{
	glGeomShader.Use();

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cmUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &pcm->matWorldToClip);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, geomUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, geomUBO);

	glLineWidth(2.0);

	glm::mat4 model{};

	for (int i = 0; i < allSWSoObjs.size(); i++)
	{
		SO* pso = allSWSoObjs[i];

		if (!SphereInFrustum(pcm->frustum, pso->xf.posWorld, pso->geomLocal.sRadius))
			continue;

		LoadMatrixFromPosRot(&pso->xf.posWorld, &pso->xf.matWorld, &model);

		glm::mat4 modelToClip = pcm->matWorldToClip * model;
		glUniformMatrix4fv(glslGeomModelToClip, 1, GL_FALSE, glm::value_ptr(modelToClip));

		if (pso->geomLocal.VAO != 0) 
		{
			glBindVertexArray(pso->geomLocal.VAO);
			glDrawElements(GL_LINES, (GLsizei)pso->geomLocal.indices.size(), GL_UNSIGNED_SHORT, 0);
		}

		if (pso->geomCameraLocal.VAO != 0) 
		{
			glBindVertexArray(pso->geomCameraLocal.VAO);
			glDrawElements(GL_LINES, (GLsizei)pso->geomCameraLocal.indices.size(), GL_UNSIGNED_SHORT, 0);
		}
	}

	glBindVertexArray(0);
}

int numRo = 0;
int numRoCel = 0;

int g_cFrameGlobs = 0;
int g_cFrameCelGlobs = 0;

int numFrameObjs = 0;

int g_cframe = 0;
int g_cframeStaticLightsInvalid = 0;

int g_boundVAO = 0;

int g_shdIDBound = 0;
int g_lastAnimateUv = -1;

int g_grfshd = 0;

int g_dynamicTextureCount = 0;
std::vector <RPL> g_dynamicTexturePrpl;

int g_backGroundCount = 0;
std::vector <RPL> g_backGroundPrpl;

int g_backGroundBlendCount = 0;
std::vector <RPL> g_backGroundBlendPrpl;

int g_blotContextCount = 0;
std::vector <RPL> g_blotContextPrpl;

int g_opaqueCount = 0;
std::vector <RPL> g_opaquePrpl;

int g_cutOutCount = 0;
std::vector <RPL> g_cutOutPrpl;

int g_cutOutBlendAddCount = 0;
std::vector <RPL> g_cutOutBlendAddPrpl;

int g_celBorderCount = 0;
std::vector <RPLCEL> g_celBorderPrpl;


int g_projVolumeCount = 0;
std::vector <RPL> g_projVolumePrpl;

int g_projVolumeAlphaAddCount = 0;
std::vector <RPL> g_projVolumeAlphaAddPrpl;

int g_projVolumeAddCount = 0;
std::vector <RPL> g_projVolumeAddPrpl;

int g_opaqueAfterProjVolumeCount = 0;
std::vector <RPL> g_opaqueAfterProjVolumePrpl;

int g_cutOutAfterProjVolumeCount = 0;
std::vector <RPL> g_cutOutAfterProjVolumePrpl;

int g_cutOutAfterProjVolumeAddCount = 0;
std::vector <RPL> g_cutOutAfterProjVolumeAddPrpl;

int g_celBorderAfterProjVolumeCount = 0;
std::vector <RPLCEL> g_celBorderAfterProjVolumePrpl;

int g_murkClearCount = 0;
std::vector <RPL> g_murkClearPrpl;

int g_murkOpaqueCount = 0;
std::vector <RPL> g_murkOpaquePrpl;

int g_murkFillCount = 0;
std::vector <RPL> g_murkFillPrpl;

int g_translucentCount = 0;
std::vector <RPL> g_translucentPrpl;

int g_translucentAddCount = 0;
std::vector <RPL> g_translucentAddPrpl;

int g_translucentCelBorderCount = 0;
std::vector <RPLCEL> g_translucentCelBorderPrpl;

int g_blipCount = 0;
std::vector <RPL> g_blipPrpl;

int g_foreGroundCount = 0;
std::vector <RPL> g_foreGroundPrpl;

int g_worldMapCount = 0;
std::vector <RPL> g_worldMapPrpl;

int g_maxCount = 0;
std::vector <RPL> g_maxPrpl;

bool g_fVsync = false;

int g_fDisableSkin = 0;
int g_fDisablePoses = 0;
