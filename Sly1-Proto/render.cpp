#include "render.h"

void AllocateRpl()
{
	renderBuffer.resize(numRo);

	numRo = 0;
}

void RenderSw(SW *psw, CM *pcm)
{
	// Set up a DLI walker for the busy object list in the current SW (Scene/World)
	DLI dlBusyWalker;
	dlBusyWalker.m_pdl = &psw->dlBusy;                // Point to the actual DL list
	dlBusyWalker.m_ibDle = psw->dlBusy.ibDle;         // Offset to the 'next' pointer inside each object
	dlBusyWalker.m_pdliNext = s_pdliFirst;            // Link this walker into a global list of DLI walkers

	// Get the first object (LO) in the busy list
	ALO* currentObject = psw->dlBusy.paloFirst;

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
		currentObject->pvtalo->pfnRenderAloAll(currentObject, pcm, nullptr);

		// Move to the next object in the list using the stored offset
		currentObject = reinterpret_cast<ALO*>(*dlBusyWalker.m_ppv);

		// If there is a next object, update the walker’s pointer to its next link
		if (currentObject != nullptr) 
			dlBusyWalker.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle);
	}
}

void RenderSwAloAll(SW* psw, CM* pcm)
{
	for (int i = 0; i < 78; i++)
	{
		CID cid = allSWAloObjs[i]->pvtalo->cid;
		
		allSWAloObjs[i]->pvtalo->pfnRenderAloAll(allSWAloObjs[i], pcm, nullptr);
	}
}

void RenderSwGlobset(SW *psw, CM *pcm)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
		allSWAloObjs[i]->pvtalo->pfnRenderAloGlobset(allSWAloObjs[i], pcm, nullptr);
}

void SubmitRpl(RPL *prpl)
{
	renderBuffer[numRo] = *prpl;
	numRo++;

	switch (prpl->rp)
	{
		case RP_DynamicTexture:
		g_dynamicTextureCount++;
		break;

		case RP_Background:
		g_backGroundCount++;
		break;

		case RP_BlotContext:
		g_blotContextCount++;
		break;

		case RP_Opaque:
		g_opaqueCount++;
		break;

		case RP_Cutout:
		g_cutOutCount++;
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
		g_cutoutAfterProjVolumeCount++;
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
		g_translucentCount++;
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

void SortRenderRpl()
{
	std::sort(renderBuffer.begin(), renderBuffer.begin() + numRo, compareRP);

	int offset = g_dynamicTextureCount;

	if (g_backGroundCount > 1)
		std::sort(renderBuffer.begin() + offset, renderBuffer.begin() + offset + g_backGroundCount, compareZ);

	offset += g_backGroundCount;

	offset += g_blotContextCount;
	offset += g_opaqueCount;

	if (g_cutOutCount > 1)
		std::sort(renderBuffer.begin() + offset, renderBuffer.begin() + offset + g_cutOutCount, compareZ);

	offset += g_cutOutCount;
	offset += g_celBorderCount;
	offset += g_projVolumeCount;
	offset += g_opaqueAfterProjVolumeCount;

	if (g_cutoutAfterProjVolumeCount > 1)
		std::sort(renderBuffer.begin() + offset, renderBuffer.begin() + offset + g_cutoutAfterProjVolumeCount, compareZ);

	offset += g_cutoutAfterProjVolumeCount;
	offset += g_celBorderAfterProjVolumeCount;
	offset += g_murkClearCount;
	offset += g_murkOpaqueCount;
	offset += g_murkFillCount;

	if (g_translucentCount > 1)
		std::sort(renderBuffer.begin() + offset, renderBuffer.begin() + offset + g_translucentCount, compareZ);

	g_dynamicTextureCount = 0;
	g_backGroundCount = 0;
	g_blotContextCount = 0;
	g_opaqueCount = 0;
	g_cutOutCount = 0;
	g_celBorderCount = 0;
	g_projVolumeCount = 0;
	g_opaqueAfterProjVolumeCount = 0;
	g_cutoutAfterProjVolumeCount = 0;
	g_celBorderAfterProjVolumeCount = 0;
	g_murkClearCount = 0;
	g_murkOpaqueCount = 0;
	g_murkFillCount = 0;
	g_translucentCount = 0;
	g_translucentCelBorderCount = 0;
	g_blipCount = 0;
	g_foreGroundCount = 0;
	g_worldMapCount = 0;
	g_maxCount = 0;
}

inline bool compareRP(RPL &prpl0, RPL &prpl1)
{
	return prpl0.rp < prpl1.rp;
}

inline bool compareZ(RPL &prpl0, RPL &prpl1)
{
	return prpl0.z > prpl1.z;
}

void DrawSw(SW *psw, CM *pcm)
{
	glGlobShader.Use();

	//std::cout << numRo << "\n";
	SortRenderRpl();

	PrepareSwLights(psw, pcm);

	glUniformMatrix4fv(glslmatWorldToClip, 1, GL_FALSE, glm::value_ptr(pcm->matWorldToClip));
	glUniform3fv(glslCameraPos, 1, glm::value_ptr(pcm->pos));

	glUniform1f(glslLsmShadow,  g_psw->lsmDefault.uShadow);
	glUniform1f(glslLsmDiffuse, g_psw->lsmDefault.uMidtone);

	glUniform1i(glslfCull, 1);

	glUniform1i(glslFogType, g_fogType);
	glUniform1f(glslFogNear, pcm->sNearFog);
	glUniform1f(glslFogFar,  pcm->sFarFog);
	glUniform1f(glslFogMax,  pcm->uFogMax);
	glUniform4fv(glslFogColor, 1, glm::value_ptr(pcm->rgbaFog));
	
	glUniform4fv(glslRgbaCel, 1, glm::value_ptr(g_rgbaCel));

	for (int i = 0; i < numRo; i++)
		renderBuffer[i].PFNDRAW(&renderBuffer[i]);

	numRo = 0;
}

void DrawSwCollisionAll(CM *pcm)
{
	glUniform1i(glslRko, 3);

	for (int i = 0; i < allSWSoObjs.size(); i++)
	{
		if ((SphereInFrustum(pcm->frustum, allSWSoObjs[i]->xf.posWorld, allSWSoObjs[i]->geomLocal.sRadius)) == 0)
			continue;

		DrawCollision(pcm, allSWSoObjs[i]);
	}
}