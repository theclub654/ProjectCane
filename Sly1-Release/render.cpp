#include "render.h"
#include "main.h"

void SetRpCount(RP rp, byte grfshd)
{
	switch (rp)
	{
		case RP_DynamicTexture:
		g_dynamicTextureCount++;
		break;

		case RP_Background:
		if (grfshd != 2)
			g_backGroundCount++;
		else
			g_backGroundBlendCount++;
		break;

		case RP_BlotContext:
		g_blotContextCount++;
		break;

		case RP_Opaque:
		g_opaqueCount++;
		g_translucentCount++;
		break;

		case RP_Cutout:
		switch (grfshd)
		{
			case 2:
			case 6:
			g_cutOutCount++;
			g_translucentCount++;
			break;

			case 3:
			g_cutOutBlendAddCount++;
			g_translucentCount++;
			break;
		}
		break;

		case RP_CelBorder:
		g_celBorderCount++;
		g_translucentCelBorderCount++;
		break;

		case RP_ProjVolume:
		switch (grfshd)
		{
			case 0:
			g_projVolumeCount++;
			break;

			case 2:
			g_projVolumeAlphaAddCount++;
			break;

			case 1:
			case 3:
			g_projVolumeAddCount++;
			break;
		}
		break;

		case RP_OpaqueAfterProjVolume:
		g_opaqueAfterProjVolumeCount++;
		g_translucentCount++;
		break;

		case RP_CutoutAfterProjVolume:
		switch (grfshd)
		{
			case 2:
			case 6:
			g_cutOutAfterProjVolumeCount++;
			g_translucentCount++;
			break;

			case 3:
			g_cutOutAfterProjVolumeAddCount++;
			g_translucentCount++;
			break;
		}
		break;

		case RP_CelBorderAfterProjVolume:
		g_celBorderAfterProjVolumeCount++;
		g_translucentCelBorderCount++;
		break;

		case RP_MurkClear:
		g_murkClearCount++;
		break;

		case RP_MurkOpaque:
		g_murkOpaqueCount++;
		break;

		case RP_MurkFill:
		g_murkFillCount++;
		break;

		case RP_Translucent:
		switch (grfshd)
		{
			case 2:
			case 6:
			g_translucentCount++;
			break;

			case 3:
			g_translucentAddCount++;
			break;
		}
		break;

		case RP_TranslucentCelBorder:
		g_translucentCelBorderCount++;
		break;

		case RP_Blip:
		g_blipCount++;
		break;

		case RP_Foreground:
		g_foreGroundCount++;
		break;

		case RP_WorldMap:
		g_worldMapCount++;
		break;

		case RP_Max:
		g_maxCount++;
		break;
	}
}

void AllocateRpl()
{
	g_dynamicTexturePrpl.resize(g_dynamicTextureCount);
	g_backGroundPrpl.resize(g_backGroundCount);
	g_backGroundBlendPrpl.resize(g_backGroundBlendCount);
	g_blotContextPrpl.resize(g_blotContextCount);
	g_opaquePrpl.resize(g_opaqueCount);
	g_cutOutPrpl.resize(g_cutOutCount);
	g_cutOutBlendAddPrpl.resize(g_cutOutBlendAddCount);
	g_celBorderPrpl.resize(g_celBorderCount);
	g_projVolumePrpl.resize(g_projVolumeCount);
	g_projVolumeAlphaAddPrpl.resize(g_projVolumeAlphaAddCount);
	g_projVolumeAddPrpl.resize(g_projVolumeAddCount);
	g_opaqueAfterProjVolumePrpl.resize(g_opaqueAfterProjVolumeCount);
	g_cutOutAfterProjVolumePrpl.resize(g_cutOutAfterProjVolumeCount);
	g_cutOutAfterProjVolumeAddPrpl.resize(g_cutOutAfterProjVolumeAddCount);
	g_celBorderAfterProjVolumePrpl.resize(g_celBorderAfterProjVolumeCount);
	g_murkClearPrpl.resize(g_murkClearCount);
	g_murkOpaquePrpl.resize(g_murkOpaqueCount);
	g_murkFillPrpl.resize(g_murkFillCount);
	g_translucentPrpl.resize(g_translucentCount);
	g_translucentAddPrpl.resize(g_translucentAddCount);
	g_translucentCelBorderPrpl.resize(g_translucentCelBorderCount);
	g_blipPrpl.resize(g_blipCount);
	g_foreGroundPrpl.resize(g_foreGroundCount);
	g_worldMapPrpl.resize(g_worldMapCount);
	g_maxPrpl.resize(g_maxCount);

	g_dynamicTextureCount = 0;
	g_backGroundCount = 0;
	g_backGroundBlendCount = 0;
	g_blotContextCount = 0;
	g_opaqueCount = 0;
	g_cutOutCount = 0;
	g_cutOutBlendAddCount = 0;
	g_celBorderCount = 0;
	g_projVolumeCount = 0;
	g_projVolumeAlphaAddCount = 0;
	g_projVolumeAddCount = 0;
	g_opaqueAfterProjVolumeCount = 0;
	g_cutOutAfterProjVolumeCount = 0;
	g_cutOutAfterProjVolumeCount = 0;
	g_celBorderAfterProjVolumeCount = 0;
	g_murkClearCount = 0;
	g_murkOpaqueCount = 0;
	g_murkFillCount = 0;
	g_translucentCount = 0;
	g_translucentAddCount = 0;
	g_translucentCelBorderCount = 0;
	g_blipCount = 0;
	g_foreGroundCount = 0;
	g_worldMapCount = 0;
}

void RenderSw(SW* psw, CM* pcm)
{
	// Set up a DLI walker for the busy object list in the current SW (Scene/World)
	DLI dlBusyWalker;
	dlBusyWalker.m_pdl = &psw->dlBusy;                // Point to the actual DL list
	dlBusyWalker.m_ibDle = psw->dlBusy.ibDle;         // Offset to the 'next' pointer inside each object
	dlBusyWalker.m_pdliNext = s_pdliFirst;            // Link this walker into a global list of DLI walkers

	// Get the first object (LO) in the busy list
	LO* currentObject = psw->dlBusy.ploFirst;

	// Set up the pointer to the "next" object in the list,
	// using offset-based pointer arithmetic from current object
	dlBusyWalker.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle);

	// Save the current DLI walker globally
	s_pdliFirst = &dlBusyWalker;

	// Loop over every object in the busy list
	while (currentObject != nullptr)
	{
		// Call the rendering function on the current object
		// This renders the object and all of its attached ALO children
		currentObject->pvtalo->pfnRenderAloAll(reinterpret_cast<ALO*>(currentObject), pcm, nullptr);

		// Move to the next object in the list using the stored offset
		currentObject = reinterpret_cast<LO*>(*dlBusyWalker.m_ppv);

		// If there is a next object, update the walker’s pointer to its next link
		if (currentObject != nullptr)
			dlBusyWalker.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle);
	}
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
	for (int i = 0; i < allSWAloObjs.size(); i++)
		allSWAloObjs[i]->pvtalo->pfnRenderAloGlobset(allSWAloObjs[i], pcm, nullptr);
}

void SubmitRpl(RPL* prpl)
{
	switch (prpl->rp)
	{
		case RP_DynamicTexture:
		g_dynamicTexturePrpl[g_dynamicTextureCount] = prpl;
		g_dynamicTextureCount++;
		break;

		case RP_Background:
		if (prpl->grfshd != 2)
		{
			prpl->grfshd = 0;
			g_backGroundPrpl[g_backGroundCount] = prpl;
			g_backGroundCount++;
		}
		else
		{
			g_backGroundBlendPrpl[g_backGroundBlendCount] = prpl;
			g_backGroundBlendCount++;
		}
		break;

		case RP_BlotContext:
		g_blotContextPrpl[g_blotContextCount] = prpl;;
		g_blotContextCount++;
		break;

		case RP_Opaque:
		g_opaquePrpl[g_opaqueCount] = prpl;;
		g_opaqueCount++;
		break;

		case RP_Cutout:
		switch (prpl->grfshd)
		{
			case 2:
			case 6:
			if (prpl->grfshd == 6)
				prpl->grfshd = 2;
			g_cutOutPrpl[g_cutOutCount] = prpl;
			g_cutOutCount++;
			break;

			case 3:
			g_cutOutBlendAddPrpl[g_cutOutBlendAddCount] = prpl;
			g_cutOutBlendAddCount++;
			break;
		}
		break;

		case RP_CelBorder:
		g_celBorderPrpl[g_celBorderCount] = prpl;
		g_celBorderCount++;
		break;

		case RP_ProjVolume:
		switch (prpl->grfshd)
		{
			case 0:
			g_projVolumePrpl[g_projVolumeCount] = prpl;
			g_projVolumeCount++;
			break;

			case 2:
			g_projVolumeAlphaAddPrpl[g_projVolumeAlphaAddCount] = prpl;
			g_projVolumeAlphaAddCount++;
			break;

			case 1:
			case 3:
			g_projVolumeAddPrpl[g_projVolumeAddCount] = prpl;
			g_projVolumeAddCount++;
			break;
		}
		break;

		case RP_OpaqueAfterProjVolume:
		g_opaqueAfterProjVolumePrpl[g_opaqueAfterProjVolumeCount] = prpl;
		g_opaqueAfterProjVolumeCount++;
		break;

		case RP_CutoutAfterProjVolume:
		switch (prpl->grfshd)
		{
			case 2:
			case 6:
			if (prpl->grfshd == 6)
				prpl->grfshd = 2;
			g_cutOutAfterProjVolumePrpl[g_cutOutAfterProjVolumeCount] = prpl;
			g_cutOutAfterProjVolumeCount++;
			break;

			case 3:
			g_cutOutAfterProjVolumePrpl[g_cutOutAfterProjVolumeAddCount] = prpl;
			g_cutOutAfterProjVolumeAddCount++;
			break;
		}
		break;

		case RP_CelBorderAfterProjVolume:
		g_celBorderAfterProjVolumePrpl[g_celBorderAfterProjVolumeCount] = prpl;
		g_celBorderAfterProjVolumeCount++;
		break;

		case RP_MurkClear:
		g_murkClearPrpl[g_murkClearCount] = prpl;
		g_murkClearCount++;
		break;

		case RP_MurkOpaque:
		g_murkOpaquePrpl[g_murkOpaqueCount] = prpl;
		g_murkOpaqueCount++;
		break;

		case RP_MurkFill:
		g_murkFillPrpl[g_murkFillCount] = prpl;
		g_murkFillCount++;
		break;

		case RP_Translucent:
		switch (prpl->grfshd)
		{
			case 2:
			case 6:
			if (prpl->grfshd == 6)
				prpl->grfshd = 2;
			g_translucentPrpl[g_translucentCount] = prpl;
			g_translucentCount++;
			break;

			case 3:
			g_translucentAddPrpl[g_translucentAddCount] = prpl;
			g_translucentAddCount++;
			break;
		}
		break;

		case RP_TranslucentCelBorder:
		g_translucentCelBorderPrpl[g_translucentCelBorderCount] = prpl;
		g_translucentCelBorderCount++;
		break;

		case RP_Blip:
		g_blipPrpl[g_blipCount] = prpl;
		g_blipCount++;
		break;

		case RP_Foreground:
		g_foreGroundPrpl[g_foreGroundCount] = prpl;
		g_foreGroundCount++;
		break;

		case RP_WorldMap:
		g_worldMapPrpl[g_worldMapCount] = prpl;
		g_worldMapCount++;
		break;

		case RP_Max:
		g_maxPrpl[g_maxCount] = prpl;
		g_maxCount++;
		break;
	}
}

void SortRenderRpl()
{
	if (g_backGroundBlendCount > 1)
		std::sort(g_backGroundBlendPrpl.begin(), g_backGroundBlendPrpl.begin() + g_backGroundBlendCount, compareZ);

	if (g_cutOutCount > 1)
		std::sort(g_cutOutPrpl.begin(), g_cutOutPrpl.begin() + g_cutOutCount, compareZ);

	if (g_cutOutAfterProjVolumeCount > 1)
		std::sort(g_cutOutAfterProjVolumePrpl.begin(), g_cutOutAfterProjVolumePrpl.begin() + g_cutOutAfterProjVolumeCount, compareZ);

	if (g_translucentCount > 1)
		std::sort(g_translucentPrpl.begin(), g_translucentPrpl.begin() + g_translucentCount, compareZ);
}

inline bool compareZ(RPL* prpl0, RPL* prpl1)
{
	return prpl0->z > prpl1->z;
}

void DrawSw(SW *psw, CM *pcm)
{
	glGlobShader.Use();

	//std::cout << numRo << "\n";
	SortRenderRpl();

	PrepareSwLights(psw, pcm);

	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);

	glBufferData(GL_UNIFORM_BUFFER, sizeof(CMGL), nullptr, GL_STREAM_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &pcm->matWorldToClip);

	glBindBuffer(GL_UNIFORM_BUFFER, ropUBO);

	glLineWidth(6.0);
	glEnable(GL_CULL_FACE);

	if (g_dynamicTextureCount > 0)
	{
		for (int i = 0; i < g_dynamicTextureCount; i++)
		{
			BindRenderObject(g_dynamicTexturePrpl[i]);
			DrawGlob(g_dynamicTexturePrpl[i]);
		}

		g_dynamicTextureCount = 0;
	}

	if (g_backGroundCount > 0)
	{
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundCount; i++)
		{
			BindRenderObject(g_backGroundPrpl[i]);
			DrawGlob(g_backGroundPrpl[i]);
		}

		glDepthFunc(GL_LESS);
		glDepthMask(true);

		g_backGroundCount = 0;
	}

	if (g_backGroundBlendCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundBlendCount; i++)
		{
			BindRenderObject(g_backGroundBlendPrpl[i]);
			DrawGlob(g_backGroundBlendPrpl[i]);
		}

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glDepthMask(true);

		g_backGroundBlendCount = 0;
	}

	if (g_blotContextCount > 0)
	{
		for (int i = 0; i < g_blotContextCount; i++)
		{
			BindRenderObject(g_blotContextPrpl[i]);
			DrawGlob(g_blotContextPrpl[i]);
		}

		g_blotContextCount = 0;
	}

	if (g_opaqueCount > 0)
	{
		for (int i = 0; i < g_opaqueCount; i++)
		{
			BindRenderObject(g_opaquePrpl[i]);
			DrawGlob(g_opaquePrpl[i]);
		}

		g_opaqueCount = 0;
	}

	if (g_cutOutCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int i = 0; i < g_cutOutCount; i++)
		{
			BindRenderObject(g_cutOutPrpl[i]);
			DrawTranslucent(g_cutOutPrpl[i]);
		}

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutCount = 0;
	}

	if (g_cutOutBlendAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		for (int i = 0; i < g_cutOutBlendAddCount; i++)
		{
			BindRenderObject(g_cutOutBlendAddPrpl[i]);
			DrawTranslucent(g_cutOutBlendAddPrpl[i]);
		}

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutBlendAddCount = 0;
	}

	if (g_celBorderCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthFunc(GL_LEQUAL);

		glBindVertexArray(gEmptyVAO);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE0);

		for (int i = 0; i < g_celBorderCount; i++)
		{
			BindRenderObjectCel(g_celBorderPrpl[i]);
			DrawCelBorder(g_celBorderPrpl[i]);
		}

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		g_celBorderCount = 0;
	}

	if (g_projVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeCount; i++)
		{
			BindRenderObject(g_projVolumePrpl[i]);
			DrawProjVolume(g_projVolumePrpl[i]);
		}

		glDepthMask(true);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeCount = 0;
	}

	if (g_projVolumeAlphaAddCount > 0)
	{
		glEnable(GL_BLEND);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeAlphaAddCount; i++)
		{
			BindRenderObject(g_projVolumeAlphaAddPrpl[i]);
			DrawProjVolumeAlphaAdd(g_projVolumeAlphaAddPrpl[i]);
		}

		glDepthMask(true);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeAlphaAddCount = 0;
	}

	if (g_projVolumeAddCount > 0)
	{
		glEnable(GL_BLEND);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glStencilMask(128);

		for (int i = 0; i < g_projVolumeAddCount; i++)
		{
			BindRenderObject(g_projVolumeAddPrpl[i]);
			DrawProjVolumeAdd(g_projVolumeAddPrpl[i]);
		}

		glDepthMask(true);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);

		g_projVolumeAddCount = 0;
	}

	if (g_opaqueAfterProjVolumeCount > 0)
	{
		for (int i = 0; i < g_opaqueAfterProjVolumeCount; i++)
		{
			BindRenderObject(g_opaqueAfterProjVolumePrpl[i]);
			DrawGlob(g_opaqueAfterProjVolumePrpl[i]);
		}

		g_opaqueAfterProjVolumeCount = 0;
	}

	if (g_cutOutAfterProjVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int i = 0; i < g_cutOutAfterProjVolumeCount; i++)
		{
			BindRenderObject(g_cutOutAfterProjVolumePrpl[i]);
			DrawTranslucent(g_cutOutAfterProjVolumePrpl[i]);
		}

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutAfterProjVolumeCount = 0;
	}

	if (g_cutOutAfterProjVolumeAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		for (int i = 0; i < g_cutOutAfterProjVolumeAddCount; i++)
		{
			BindRenderObject(g_cutOutAfterProjVolumeAddPrpl[i]);
			DrawTranslucent(g_cutOutAfterProjVolumeAddPrpl[i]);
		}

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_cutOutAfterProjVolumeAddCount = 0;
	}

	if (g_celBorderAfterProjVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthFunc(GL_LEQUAL);

		glBindVertexArray(gEmptyVAO);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE0);

		for (int i = 0; i < g_celBorderAfterProjVolumeCount; i++)
		{
			BindRenderObjectCel(g_celBorderAfterProjVolumePrpl[i]);
			DrawCelBorder(g_celBorderAfterProjVolumePrpl[i]);
		}

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		g_celBorderAfterProjVolumeCount = 0;
	}

	if (g_murkClearCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_murkClearCount; i++)
		{
			BindRenderObject(g_murkClearPrpl[i]);
			DrawMurkClear(g_murkClearPrpl[i]);
		}

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		g_murkClearCount = 0;
	}

	if (g_murkOpaqueCount > 0)
	{
		for (int i = 0; i < g_murkOpaqueCount; i++)
		{
			BindRenderObject(g_murkOpaquePrpl[i]);
			DrawGlob(g_murkOpaquePrpl[i]);
		}

		g_murkOpaqueCount = 0;
	}

	if (g_murkFillCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(false);
		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_murkFillCount; i++)
		{
			BindRenderObject(g_murkFillPrpl[i]);
			DrawMurkFill(g_murkFillPrpl[i]);
		}

		glDisable(GL_BLEND);
		glDepthMask(true);
		glDepthFunc(GL_LESS);

		g_murkFillCount = 0;
	}

	if (g_translucentCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		for (int i = 0; i < g_translucentCount; i++)
		{
			BindRenderObject(g_translucentPrpl[i]);
			DrawTranslucent(g_translucentPrpl[i]);
		}

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_translucentCount = 0;
	}

	if (g_translucentAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);

		for (int i = 0; i < g_translucentAddCount; i++)
		{
			BindRenderObject(g_translucentAddPrpl[i]);
			DrawTranslucent(g_translucentAddPrpl[i]);
		}

		glDepthMask(true);
		glDisable(GL_BLEND);

		g_translucentAddCount = 0;
	}

	if (g_translucentCelBorderCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE0);

		glDepthFunc(GL_LEQUAL);

		glBindVertexArray(gEmptyVAO);

		for (int i = 0; i < g_translucentCelBorderCount; i++)
		{
			BindRenderObjectCel(g_translucentCelBorderPrpl[i]);
			DrawCelBorder(g_translucentCelBorderPrpl[i]);
		}

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		g_translucentCelBorderCount = 0;
	}

	if (g_blipCount > 0)
	{
		for (int i = 0; i < g_blipCount; i++)
		{
			BindRenderObject(g_blipPrpl[i]);
			DrawGlob(g_blipPrpl[i]);
		}

		g_blipCount = 0;
	}

	if (g_foreGroundCount > 0)
	{
		for (int i = 0; i < g_foreGroundCount; i++)
		{
			BindRenderObject(g_foreGroundPrpl[i]);
			DrawGlob(g_foreGroundPrpl[i]);
		}

		g_foreGroundCount = 0;
	}

	if (g_worldMapCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1i(glslfAlphaTest, 1);
		//glDepthFunc(GL_ALWAYS);

		for (int i = 0; i < g_worldMapCount; i++)
		{
			BindRenderObject(g_worldMapPrpl[i]);
			DrawGlob(g_worldMapPrpl[i]);
		}

		glUniform1i(glslfAlphaTest, 0);
		//glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);

		g_worldMapCount = 0;
	}

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_CULL_FACE);
	glClearStencil(0x00);
}

void BindRenderObject(RPL* prpl)
{
	glBindVertexArray(prpl->VAO);

	glBufferData(GL_UNIFORM_BUFFER, roSize, nullptr, GL_STREAM_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, roSize, &prpl->ro);

	prpl->PFNBIND(prpl);
}

void BindOneWay(RPL* prpl)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, prpl->pshd->atex[0].abmp[0]->glDiffuseMap);
}

void BindThreeWay(RPL *prpl)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, prpl->pshd->atex[0].abmp[0]->glShadowMap);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, prpl->pshd->atex[0].abmp[0]->glDiffuseMap);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, prpl->pshd->atex[0].abmp[0]->glSaturateMap);

	/*if (prpl->ro.rko == SHDK_ThreeWay)
		FindSwLights(g_psw, g_pcm, prpl->ro.posCenter, prpl->ro.sRadius);*/
}

void BindRenderObjectCel(RPL *prpl)
{
	glBufferData(GL_UNIFORM_BUFFER, roCelSize, nullptr, GL_STREAM_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, roCelSize, &prpl->ro);

	glBindTexture(GL_TEXTURE_BUFFER, prpl->edgeTex);
}

void DrawGlob(RPL *prpl)
{
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawCelBorder(RPL *prpl)
{
	glDrawArrays(GL_LINES, 0, prpl->edgeCount * 2);
}

void DrawProjVolume(RPL *prpl)
{
	glBlendFunc(GL_ZERO, GL_ONE);
	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_NONE, GL_REPLACE, GL_NONE);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_NONE, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 128);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawProjVolumeAlphaAdd(RPL* prpl)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_NONE, GL_REPLACE, GL_NONE);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_NONE, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 128);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawProjVolumeAdd(RPL* prpl)
{
	// Expect grfshd in {1, 3}. If not, early-out or clamp.
	switch (prpl->grfshd)
	{
		case 1: // edge pass uses (ZERO, ONE)
		glBlendFunc(GL_ZERO, GL_ONE);
		break;

		case 3: // edge pass uses (SRC_ALPHA, ONE)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;

		default:
		// Not an additive type; route elsewhere (alpha path) or early-out.
		return;
	}
	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_NONE, GL_REPLACE, GL_NONE);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_NONE, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 128);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawMurkClear(RPL *prpl)
{
	glBlendFunc(GL_ZERO, GL_ONE);
	glFrontFace(GL_CW);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);

	glFrontFace(GL_CCW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawMurkFill(RPL *prpl)
{
	glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawTranslucent(RPL *prpl)
{
	glUniform1i(glslfAlphaTest, 1);
	glDepthMask(true);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);
	glUniform1i(glslfAlphaTest, 0);

	glDepthMask(false);
	glDrawElements(GL_TRIANGLES, prpl->cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawSwCollisionAll(CM *pcm)
{
	glGlobShader.Use();

	glLineWidth(2.0);
	glEnable(GL_CULL_FACE);

	RPL rpl{};
	rpl.ro.rko = 3;

	for (int i = 0; i < allSWSoObjs.size(); i++)
	{
		if ((SphereInFrustum(pcm->frustum, allSWSoObjs[i]->xf.posWorld, allSWSoObjs[i]->geomLocal.sRadius)) == 0)
			continue;

		SO* pso = allSWSoObjs[i];

		if (pso->geomLocal.VAO != 0 || pso->geomCameraLocal.VAO != 0)
		{
			glm::mat4 model = pso->xf.matWorld;

			model[3][0] = pso->xf.posWorld[0];
			model[3][1] = pso->xf.posWorld[1];
			model[3][2] = pso->xf.posWorld[2];
			model[3][3] = 1.0;

			rpl.ro.model = model;

			glBindVertexArray(pso->geomLocal.VAO);

			glBufferData(GL_UNIFORM_BUFFER, roCollisionSize, nullptr, GL_STREAM_DRAW);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, roCollisionSize, &rpl.ro);

			glDrawElements(GL_LINES, pso->geomLocal.indices.size() * sizeof(uint16_t), GL_UNSIGNED_SHORT, 0);

			if (pso->geomCameraLocal.VAO != 0)
			{
				glBindVertexArray(pso->geomCameraLocal.VAO);
				glDrawElements(GL_LINES, pso->geomCameraLocal.indices.size() * sizeof(uint16_t), GL_UNSIGNED_SHORT, 0);
			}
		}
	}

	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
}

int numRo = 0;

int g_dynamicTextureCount = 0;
std::vector <RPL*> g_dynamicTexturePrpl;

int g_backGroundCount = 0;
std::vector <RPL*> g_backGroundPrpl;

int g_backGroundBlendCount = 0;
std::vector <RPL*> g_backGroundBlendPrpl;

int g_blotContextCount = 0;
std::vector <RPL*> g_blotContextPrpl;

int g_opaqueCount = 0;
std::vector <RPL*> g_opaquePrpl;

int g_cutOutCount = 0;
std::vector <RPL*> g_cutOutPrpl;

int g_cutOutBlendAddCount = 0;
std::vector <RPL*> g_cutOutBlendAddPrpl;

int g_celBorderCount = 0;
std::vector <RPL*> g_celBorderPrpl;

int g_projVolumeCount = 0;
std::vector <RPL*> g_projVolumePrpl;

int g_projVolumeAlphaAddCount = 0;
std::vector <RPL*> g_projVolumeAlphaAddPrpl;

int g_projVolumeAddCount = 0;
std::vector <RPL*> g_projVolumeAddPrpl;

int g_opaqueAfterProjVolumeCount = 0;
std::vector <RPL*> g_opaqueAfterProjVolumePrpl;

int g_cutOutAfterProjVolumeCount = 0;
std::vector <RPL*> g_cutOutAfterProjVolumePrpl;

int g_cutOutAfterProjVolumeAddCount = 0;
std::vector <RPL*> g_cutOutAfterProjVolumeAddPrpl;

int g_celBorderAfterProjVolumeCount = 0;
std::vector <RPL*> g_celBorderAfterProjVolumePrpl;

int g_murkClearCount = 0;
std::vector <RPL*> g_murkClearPrpl;

int g_murkOpaqueCount = 0;
std::vector <RPL*> g_murkOpaquePrpl;

int g_murkFillCount = 0;
std::vector <RPL*> g_murkFillPrpl;

int g_translucentCount = 0;
std::vector <RPL*> g_translucentPrpl;

int g_translucentAddCount = 0;
std::vector <RPL*> g_translucentAddPrpl;

int g_translucentCelBorderCount = 0;
std::vector <RPL*> g_translucentCelBorderPrpl;

int g_blipCount = 0;
std::vector <RPL*> g_blipPrpl;

int g_foreGroundCount = 0;
std::vector <RPL*> g_foreGroundPrpl;

int g_worldMapCount = 0;
std::vector <RPL*> g_worldMapPrpl;

int g_maxCount = 0;
std::vector <RPL*> g_maxPrpl;