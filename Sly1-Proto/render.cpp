#include "render.h"
#include "main.h"

void AllocateRpl()
{
	renderBuffer.resize(numRo);

	numRo = 0;
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
		g_dynamicTextureCount++;
		break;

	case RP_Background:
		if (prpl->grfshd != 2)
		{
			prpl->grfshd = 0;
			g_backGroundCount++;
		}
		else
			g_backGroundBlendCount++;
		break;

	case RP_BlotContext:
		g_blotContextCount++;
		break;

	case RP_Opaque:
		g_opaqueCount++;
		break;

	case RP_Cutout:
		switch (prpl->grfshd)
		{
		case 2:
		case 6:
			if (prpl->grfshd == 6)
				prpl->grfshd = 2;
			g_cutOutCount++;
			break;

		case 3:
			g_cutOutBlendAddCount++;
			break;
		}
		break;

	case RP_CelBorder:
		g_celBorderCount++;
		break;

	case RP_ProjVolume:
		g_projVolumeCount++;
		break;

	case RP_OpaqueAfterProjVolume:
		g_opaqueAfterProjVolumeCount++;
		break;

	case RP_CutoutAfterProjVolume:
		switch (prpl->grfshd)
		{
		case 2:
		case 6:
			if (prpl->grfshd == 6)
				prpl->grfshd = 2;
			g_cutOutAfterProjVolumeCount++;
			break;

		case 3:
			g_cutOutAfterProjVolumeAddCount++;
			break;
		}
		break;

	case RP_CelBorderAfterProjVolume:
		g_celBorderAfterProjVolumeCount++;
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
		switch (prpl->grfshd)
		{
		case 2:
		case 6:
			if (prpl->grfshd == 6)
				prpl->grfshd = 2;
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

	renderBuffer[numRo] = *prpl;
	numRo++;
}

void SortRenderRpl()
{
	std::sort(renderBuffer.begin(), renderBuffer.begin() + numRo, compareRP);

	int offset = g_dynamicTextureCount;

	// Background: [plain][blend], Z-sort blend subgroup
	if (g_backGroundCount + g_backGroundBlendCount > 1)
	{
		std::sort(renderBuffer.begin() + offset,
			renderBuffer.begin() + offset + g_backGroundCount + g_backGroundBlendCount,
			compareGrfShd);

		// move to start of BLEND subgroup
		offset += g_backGroundCount;

		if (g_backGroundBlendCount > 1) {
			std::sort(renderBuffer.begin() + offset,
				renderBuffer.begin() + offset + g_backGroundBlendCount,
				compareZ); // back-to-front
		}

		// IMPORTANT: advance past the BLEND subgroup too
		offset += g_backGroundBlendCount;   //add this line
	}
	else
	{
		offset += g_backGroundCount + g_backGroundBlendCount;
	}

	offset += g_blotContextCount;
	offset += g_opaqueCount;

	// Cutout: [alpha][add]
	if (g_cutOutCount + g_cutOutBlendAddCount > 1)
	{
		std::sort(renderBuffer.begin() + offset,
			renderBuffer.begin() + offset + g_cutOutCount + g_cutOutBlendAddCount,
			compareGrfShd);

		// Optional: Z-sort alpha subgroup if truly blended
		// if (g_cutOutCount > 1)
		//     std::sort(renderBuffer.begin() + offset,
		//               renderBuffer.begin() + offset + g_cutOutCount, compareZ);

		int addStart = offset + g_cutOutCount;
		if (g_cutOutBlendAddCount > 1) {
			std::sort(renderBuffer.begin() + addStart,
				renderBuffer.begin() + addStart + g_cutOutBlendAddCount,
				compareZ); // or your front-to-back variant
		}

		offset += g_cutOutCount + g_cutOutBlendAddCount;
	}
	else
	{
		offset += g_cutOutCount + g_cutOutBlendAddCount;
	}

	offset += g_celBorderCount;

	// ProjVolume: group only
	/*if (g_projVolumeCount + g_projVolumeBlendAddCount + g_projVolumeBlendCount > 1) {
		std::sort(renderBuffer.begin() + offset,
			renderBuffer.begin() + offset + g_projVolumeCount + g_projVolumeBlendAddCount + g_projVolumeBlendCount,
			compareGrfShd);
	}*/
	offset += g_projVolumeCount /*+ g_projVolumeBlendAddCount + g_projVolumeBlendCount*/; // (only once)

	offset += g_opaqueAfterProjVolumeCount;

	// CutoutAfterProjVolume: [alpha][add]
	if (g_cutOutAfterProjVolumeCount + g_cutOutAfterProjVolumeAddCount > 1)
	{
		std::sort(renderBuffer.begin() + offset,
			renderBuffer.begin() + offset + g_cutOutAfterProjVolumeCount + g_cutOutAfterProjVolumeAddCount,
			compareGrfShd);

		// Optional alpha Z-sort:
		// if (g_cutOutAfterProjVolumeCount > 1)
		//     std::sort(renderBuffer.begin() + offset,
		//               renderBuffer.begin() + offset + g_cutOutAfterProjVolumeCount, compareZ);

		int addStart = offset + g_cutOutAfterProjVolumeCount;
		if (g_cutOutAfterProjVolumeAddCount > 1) {
			std::sort(renderBuffer.begin() + addStart,
				renderBuffer.begin() + addStart + g_cutOutAfterProjVolumeAddCount,
				compareZ);
		}

		offset += g_cutOutAfterProjVolumeCount + g_cutOutAfterProjVolumeAddCount;
	}
	else
	{
		offset += g_cutOutAfterProjVolumeCount + g_cutOutAfterProjVolumeAddCount;
	}

	offset += g_celBorderAfterProjVolumeCount;
	offset += g_murkClearCount;
	offset += g_murkOpaqueCount;
	offset += g_murkFillCount;

	// Translucent: [alpha][add]
	if (g_translucentCount + g_translucentAddCount > 1)
	{
		std::sort(renderBuffer.begin() + offset,
			renderBuffer.begin() + offset + g_translucentCount + g_translucentAddCount,
			compareGrfShd);

		// Alpha must be back-to-front
		if (g_translucentCount > 1) {
			std::sort(renderBuffer.begin() + offset,
				renderBuffer.begin() + offset + g_translucentCount,
				compareZ);
		}

		// Optional: additive front-to-back for overdraw
		// int addStart = offset + g_translucentCount;
		// if (g_translucentAddCount > 1) {
		//     std::sort(renderBuffer.begin() + addStart,
		//               renderBuffer.begin() + addStart + g_translucentAddCount,
		//               Z_FrontToBack);
		// }
	}

	// If you have more buckets after translucent (TranslucentCelBorder, Blip, Foreground, WorldMap)
	// either advance `offset` accordingly or leave them as-is if no subgroup sort is needed.
}

inline bool compareRP(RPL& prpl0, RPL& prpl1)
{
	return prpl0.rp < prpl1.rp;
}

inline bool compareGrfShd(RPL& prpl0, RPL& prpl1)
{
	return prpl0.grfshd < prpl1.grfshd;
}

inline bool compareZ(RPL& prpl0, RPL& prpl1)
{
	return prpl0.z > prpl1.z;
}

void DrawSw(SW* psw, CM* pcm)
{
	glGlobShader.Use();

	//std::cout << numRo << "\n";
	SortRenderRpl();

	PrepareSwLights(psw, pcm);

	glUniformMatrix4fv(glslmatWorldToClip, 1, GL_FALSE, glm::value_ptr(pcm->matWorldToClip));
	glUniform3fv(glslCameraPos, 1, glm::value_ptr(pcm->pos));

	glUniform1f(glslLsmShadow, g_psw->lsmDefault.uShadow);
	glUniform1f(glslLsmDiffuse, g_psw->lsmDefault.uMidtone);

	glUniform1i(glslFogType, g_fogType);
	glUniform1f(glslFogNear, pcm->sNearFog);
	glUniform1f(glslFogFar, pcm->sFarFog);
	glUniform1f(glslFogMax, pcm->uFogMax);
	glUniform4fv(glslFogColor, 1, glm::value_ptr(pcm->rgbaFog));

	glUniform4fv(glslRgbaCel, 1, glm::value_ptr(g_rgbaCel));

	RescaleLineWidth();
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	int idx = 0;

	if (g_dynamicTextureCount > 0)
	{
		for (int i = 0; i < g_dynamicTextureCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}
	}

	if (g_backGroundCount > 0)
	{
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}

		glDepthFunc(GL_LESS);
		glDepthMask(true);
	}

	if (g_backGroundBlendCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glDepthFunc(GL_ALWAYS);
		glDepthMask(false);

		for (int i = 0; i < g_backGroundBlendCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		glDepthMask(true);
	}

	if (g_blotContextCount > 0)
	{
		for (int i = 0; i < g_blotContextCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}
	}

	if (g_opaqueCount > 0)
	{
		for (int i = 0; i < g_opaqueCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}
	}

	if (g_cutOutCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glUniform1i(glslfAlphaTest, 1);

		for (int i = 0; i < g_cutOutCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawCutout(&renderBuffer[idx]);
			idx++;
		}

		glUniform1i(glslfAlphaTest, 0);
		glDepthMask(true);
		glDisable(GL_BLEND);
	}

	if (g_cutOutBlendAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
		glUniform1i(glslfAlphaTest, 1);

		for (int i = 0; i < g_cutOutBlendAddCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawCutout(&renderBuffer[idx]);
			idx++;
		}

		glUniform1i(glslfAlphaTest, 0);
		glDepthMask(true);
		glDisable(GL_BLEND);
	}

	if (g_celBorderCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUniform1i(glslRko, 2);

		glDepthFunc(GL_LEQUAL);

		glBindVertexArray(gEmptyVAO);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE0);

		for (int i = 0; i < g_celBorderCount; i++)
		{
			if (renderBuffer[idx].ro.uAlphaCelBorder * g_rgbaCel.a != 0.0)
			{
				BindRenderObjectCel(&renderBuffer[idx]);
				DrawCelBorder(&renderBuffer[idx]);
			}
			idx++;
		}

		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
	}

	if (g_projVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glStencilMask(0xFF);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);

		for (int i = 0; i < g_projVolumeCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawProjVolume(&renderBuffer[idx]);
			idx++;
		}

		glDepthMask(true);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);
	}

	if (g_opaqueAfterProjVolumeCount > 0)
	{
		for (int i = 0; i < g_opaqueAfterProjVolumeCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}
	}

	if (g_cutOutAfterProjVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1i(glslfAlphaTest, 1);

		for (int i = 0; i < g_cutOutAfterProjVolumeCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawCutout(&renderBuffer[idx]);
			idx++;
		}

		glUniform1i(glslfAlphaTest, 0);
		glDepthMask(true);
		glDisable(GL_BLEND);

	}

	if (g_cutOutAfterProjVolumeAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glUniform1i(glslfAlphaTest, 1);

		for (int i = 0; i < g_cutOutAfterProjVolumeAddCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawCutout(&renderBuffer[idx]);
			idx++;
		}

		glUniform1i(glslfAlphaTest, 0);
		glDepthMask(true);
		glDisable(GL_BLEND);
	}

	if (g_celBorderAfterProjVolumeCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1i(glslRko, 2);

		glDepthFunc(GL_LEQUAL);

		glBindVertexArray(gEmptyVAO);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE0);

		for (int i = 0; i < g_celBorderAfterProjVolumeCount; i++)
		{
			if (renderBuffer[idx].ro.uAlphaCelBorder * g_rgbaCel.a != 0.0)
			{
				BindRenderObjectCel(&renderBuffer[idx]);
				DrawCelBorder(&renderBuffer[idx]);
			}
			idx++;
		}

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
	}

	if (g_murkClearCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glBlendColor(0, 0, 0, 0);

		for (int i = 0; i < g_murkClearCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawMurkClear(&renderBuffer[idx]);
			idx++;
		}

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
	}

	if (g_murkOpaqueCount > 0)
	{
		for (int i = 0; i < g_murkOpaqueCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}
	}

	if (g_murkFillCount > 0)
	{
		glEnable(GL_BLEND);
		glDepthMask(false);
		glDepthFunc(GL_LEQUAL);

		for (int i = 0; i < g_murkFillCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawMurkFill(&renderBuffer[idx]);
			idx++;
		}

		glDisable(GL_BLEND);
		glDepthMask(true);
		glDepthFunc(GL_LESS);
	}

	if (g_translucentCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int i = 0; i < g_translucentCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawTranslucent(&renderBuffer[idx]);
			idx++;
		}

		glDepthMask(true);
		glDisable(GL_BLEND);
	}

	if (g_translucentAddCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		for (int i = 0; i < g_translucentAddCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawTranslucent(&renderBuffer[idx]);
			idx++;
		}

		glDepthMask(true);
		glDisable(GL_BLEND);
	}

	if (g_translucentCelBorderCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1i(glslRko, 2);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glActiveTexture(GL_TEXTURE0);

		glDepthFunc(GL_LEQUAL);

		glBindVertexArray(gEmptyVAO);

		for (int i = 0; i < g_translucentCelBorderCount; i++)
		{
			if (renderBuffer[idx].ro.uAlphaCelBorder * g_rgbaCel.a != 0.0)
			{
				BindRenderObjectCel(&renderBuffer[idx]);
				DrawCelBorder(&renderBuffer[idx]);
			}
			idx++;
		}

		glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
	}

	if (g_blipCount > 0)
	{
		for (int i = 0; i < g_blipCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}
	}

	if (g_foreGroundCount > 0)
	{
		for (int i = 0; i < g_foreGroundCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}
	}

	if (g_worldMapCount > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glDepthFunc(GL_ALWAYS);

		for (int i = 0; i < g_worldMapCount; i++)
		{
			BindRenderObject(&renderBuffer[idx]);
			DrawGlob(&renderBuffer[idx]);
			idx++;
		}

		//glDepthFunc(GL_LESS);
		glDisable(GL_BLEND);
	}

	numRo = 0;
	g_dynamicTextureCount = 0;
	g_backGroundCount = 0;
	g_backGroundBlendCount = 0;
	g_blotContextCount = 0;
	g_opaqueCount = 0;
	g_cutOutCount = 0;
	g_cutOutBlendAddCount = 0;
	g_celBorderCount = 0;
	g_projVolumeCount = 0;
	g_opaqueAfterProjVolumeCount = 0;
	g_cutOutAfterProjVolumeCount = 0;
	g_cutOutAfterProjVolumeCount = 0;
	g_celBorderAfterProjVolumeCount = 0;
	g_murkClearCount = 0;
	g_murkOpaqueCount = 0;
	g_murkFillCount = 0;
	g_translucentCount = 0;
	g_translucentAddCount = 0;
	g_transluscentOnScreen = 0;
	g_translucentCelBorderCount = 0;
	g_blipCount = 0;
	g_foreGroundCount = 0;
	g_worldMapCount = 0;

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_CULL_FACE);
}

void BindRenderObject(RPL* prpl)
{
	glBindVertexArray(*prpl->ro.VAO);

	glUniformMatrix4fv(glslModel, 1, GL_FALSE, glm::value_ptr(prpl->ro.model));
	glUniform1f(glslUFog, prpl->ro.uFog);
	glUniform1f(glslUAlpha, prpl->ro.uAlpha);
	glUniform1f(glslRDarken, prpl->ro.darken);

	switch (prpl->ro.pshd->shdk)
	{
	case SHDK_ThreeWay:
		//FindSwLights(g_psw, g_pcm, prpl->posCenter, prpl->sRadius);

		glUniform1i(glslRko, 1);

		glUniform1f(glslusSelfIllum, prpl->ro.unSelfIllum);
		glUniform1i(glslFDynamic, prpl->ro.fDynamic);
		glUniform3fv(glslPosCenter, 1, glm::value_ptr(prpl->ro.posCenter));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->atex[0].abmp[0]->glShadowMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->atex[0].abmp[0]->glDiffuseMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->atex[0].abmp[0]->glSaturateMap);
		break;

	default:
		glUniform1i(glslRko, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->atex[0].abmp[0]->glDiffuseMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		break;
	}
}

void BindRenderObjectCel(RPL* prpl)
{
	glUniformMatrix4fv(glslModel, 1, GL_FALSE, glm::value_ptr(prpl->ro.model));

	glBindTexture(GL_TEXTURE_BUFFER, prpl->ro.edgeTex);
}

void DrawGlob(RPL* prpl)
{
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawCutout(RPL* prpl)
{
	glDepthMask(true);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

	glDepthMask(false);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawCelBorder(RPL* prpl)
{
	glUniform1f(glsluAlphaCelBorder, prpl->ro.uAlphaCelBorder);
	glDrawArrays(GL_LINES, 0, prpl->ro.edgeCount * 2);
}

void DrawProjVolume(RPL* prpl)
{
	switch (prpl->ro.pshd->grfshd)
	{
	case 2:
	case 3:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;

	default:
		glBlendFunc(GL_ZERO, GL_ONE);
		break;
	}

	glDepthFunc(GL_LESS);
	glStencilFunc(GL_ALWAYS, 128, 128);
	glStencilOp(GL_NONE, GL_REPLACE, GL_NONE);
	glColorMask(0, 0, 0, 0);
	glFrontFace(GL_CW);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

	glColorMask(1, 1, 1, 1);
	glStencilOp(GL_KEEP, GL_NONE, GL_KEEP);
	glFrontFace(GL_CCW);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

	glDepthFunc(GL_ALWAYS);
	glStencilFunc(GL_EQUAL, 128, 128);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	switch (prpl->ro.pshd->grfshd)
	{
	case 0:
	case 2:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;

	case 1:
	case 3:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	}
	glFrontFace(GL_CW);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawMurkClear(RPL* prpl)
{
	glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
	glFrontFace(GL_CW);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

	glFrontFace(GL_CCW);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawMurkFill(RPL* prpl)
{
	glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA, GL_ONE, GL_ZERO);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawTranslucent(RPL* prpl)
{
	glUniform1i(glslfAlphaTest, 1);
	glDepthMask(true);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
	glUniform1i(glslfAlphaTest, 0);

	glDepthMask(false);
	glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
}

void DrawSwCollisionAll(CM* pcm)
{
	glLineWidth(2.0);
	glEnable(GL_CULL_FACE);
	glUniform1i(glslRko, 3);

	for (int i = 0; i < allSWSoObjs.size(); i++)
	{
		if ((SphereInFrustum(pcm->frustum, allSWSoObjs[i]->xf.posWorld, allSWSoObjs[i]->geomLocal.sRadius)) == 0)
			continue;

		DrawCollision(allSWSoObjs[i]);
	}

	glDisable(GL_CULL_FACE);
}

std::vector <RPL> renderBuffer;
int numRo;

int g_dynamicTextureCount = 0;
int g_backGroundCount = 0;
int g_backGroundBlendCount = 0;
int g_blotContextCount = 0;
int g_opaqueCount = 0;
int g_cutOutCount = 0;
int g_cutOutBlendAddCount = 0;
int g_celBorderCount = 0;
int g_projVolumeCount = 0;
int g_projVolumeBlendAddCount = 0;
int g_projVolumeBlendCount = 0;
int g_opaqueAfterProjVolumeCount = 0;
int g_cutOutAfterProjVolumeCount = 0;
int g_cutOutAfterProjVolumeAddCount = 0;
int g_celBorderAfterProjVolumeCount = 0;
int g_murkClearCount = 0;
int g_murkOpaqueCount = 0;
int g_murkFillCount = 0;
int g_translucentCount = 0;
int g_translucentAddCount = 0;
int g_transluscentOnScreen = 0;
int g_translucentCelBorderCount = 0;
int g_blipCount = 0;
int g_foreGroundCount = 0;
int g_worldMapCount = 0;
int g_maxCount = 0;

glm::vec4 g_rgbaCel = glm::vec4(16.0f / 255.0f, 16.0f / 255.0f, 16.0f / 255.0f, 1.0);