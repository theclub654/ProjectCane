#include "render.h"

void RenderSw(SW *psw, CM *pcm)
{
	glGlobShader.Use();

	DLI dlBusyDli;

	// Loading SW object list
	dlBusyDli.m_pdl = &psw->dlBusy;
	// Loading base offset to next object
	dlBusyDli.m_ibDle = psw->dlBusy.ibDle;

	dlBusyDli.m_pdliNext = s_pdliFirst;

	// Loading first object in SW object list
	LO *localObject = psw->dlBusy.ploFirst;
	// Loading pointer to next object in SW list
	dlBusyDli.m_ppv = (void**)((uintptr_t)localObject + dlBusyDli.m_ibDle);
	
	s_pdliFirst = &dlBusyDli;

	// Looping through all objects in a level
	while (localObject != 0)
	{
		// Setting object up to be rendered
		localObject->pvtalo->pfnRenderAloAll((ALO*)localObject, pcm, 0);
		// Loading next object
		localObject = (LO*)*dlBusyDli.m_ppv;
		// Loading pointer to next object to render
		dlBusyDli.m_ppv = (void**)((uintptr_t)localObject + dlBusyDli.m_ibDle);
	}
}

void RenderSwGlobset(SW *psw, CM *pcm)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
		allSWAloObjs[i]->pvtalo->pfnRenderAloGlobset(allSWAloObjs[i], pcm, nullptr);
}

void SubmitRpl(RPL *prpl)
{
	renderBuffer.push_back(*prpl);

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
			g_translucentCount++;
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
	std::sort(renderBuffer.begin(), renderBuffer.end(), compareRP);

	if (g_cutOutCount != 0)
	{
		int startIndex = g_dynamicTextureCount + g_backGroundCount + g_blotContextCount + g_opaqueCount;
		int endIndex = startIndex + g_cutOutCount;

		std::sort(renderBuffer.begin() + startIndex, renderBuffer.begin() + endIndex, compareZ);
	}

	if (g_cutoutAfterProjVolumeCount != 0)
	{
		int startIndex = g_dynamicTextureCount + g_backGroundCount + g_blotContextCount + g_opaqueCount + g_cutOutCount + g_celBorderCount + g_projVolumeCount + g_opaqueAfterProjVolumeCount;
		int endIndex = startIndex + g_cutoutAfterProjVolumeCount;

		std::sort(renderBuffer.begin() + startIndex, renderBuffer.begin() + endIndex, compareZ);
	}

	if (g_translucentCount != 0)
	{
		int startIndex = g_dynamicTextureCount + g_backGroundCount + g_blotContextCount + g_opaqueCount + g_cutOutCount + g_celBorderCount + g_projVolumeCount + g_opaqueAfterProjVolumeCount +
		g_cutoutAfterProjVolumeCount + g_celBorderAfterProjVolumeCount + g_murkClearCount + g_murkOpaqueCount + g_murkFillCount;

		int endIndex = startIndex + g_translucentCount;

		std::sort(renderBuffer.begin() + startIndex, renderBuffer.begin() + endIndex, compareZ);
	}

	if (g_backGroundCount != 0)
	{
		int startIndex = g_dynamicTextureCount;
		int endIndex = startIndex + g_backGroundCount;

		std::sort(renderBuffer.begin() + startIndex, renderBuffer.begin() + endIndex, compareZ);
	}

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

	PrepareSwLightsForDraw(g_psw);

	SortRenderRpl();

	int matWorldToClipLocation = glGetUniformLocation(glGlobShader.ID, "matWorldToClip");
	glUniformMatrix4fv(matWorldToClipLocation, 1, GL_FALSE, glm::value_ptr(pcm->matWorldToClip));

	glUniform1f(glGetUniformLocation(glGlobShader.ID, "lsm.uShadow"),  g_psw->lsmDefault.uShadow);
	glUniform1f(glGetUniformLocation(glGlobShader.ID, "lsm.uMidtone"), g_psw->lsmDefault.uMidtone);

	for (int i = 0; i < renderBuffer.size(); i++)
		renderBuffer[i].PFNDRAW(&renderBuffer[i]);

	renderBuffer.clear();
}

void DrawSwCollisionAll()
{
	glCollisionShader.Use();

	int matWorldToClipLocation = glGetUniformLocation(glCollisionShader.ID, "matWorldToClip");
	glUniformMatrix4fv(matWorldToClipLocation, 1, GL_FALSE, glm::value_ptr(g_pcm->matWorldToClip));

	for (int i = 0; i < allSWSoObjs.size(); i++)
		DrawCollision(allSWSoObjs[i]);
}