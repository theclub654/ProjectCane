#include "render.h"

GLint GetUniformLocation(GLuint program​, const std::string& name)
{
	return glGetUniformLocation(program​, name.c_str());
}

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
		//localObject->pvtalo->pfnRenderAloAll((ALO*)localObject, pcm, 0);
		//DrawAlo((ALO*)localObject);
		// Loading next object
		localObject = (LO*)*dlBusyDli.m_ppv;
		// Loading pointer to next object to render
		dlBusyDli.m_ppv = (void**)((uintptr_t)localObject + dlBusyDli.m_ibDle);
	}
}

void RenderSwAll(SW* psw, CM* pcm)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
		allSWAloObjs[i]->pvtalo->pfnRenderAloAll(allSWAloObjs[i], pcm, nullptr);
}

void DrawSw(SW* psw, CM* pcm)
{

}

void DrawSwAll(SW* psw, GLFWwindow* window)
{
	glGlobShader.Use();

	PrepareSwLightsForDraw(g_psw);

	int numDirLights = 0;
	int numPointLights = 0;

	for (int i = 0; i < allSwLights.size(); i++)
	{
		if (allSwLights[i]->lightk == LIGHTK_Position)
		{
			/*glUniform3fv(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].pos"),     1, glm::value_ptr(allSwLights[i]->xf.posWorld));
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].color"),   1, glm::value_ptr(allSwLights[i]->rgbaColor));
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].falloff"), 1, glm::value_ptr(allSwLights[i]->agFallOff));

			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].ruShadow"),    allSwLights[i]->ltfn.ruShadow);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].ruMidtone"),   allSwLights[i]->ltfn.ruMidtone);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].ruHighlight"), allSwLights[i]->ltfn.ruHighlight);

			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].duShadow"),    allSwLights[i]->ltfn.duShadow);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].duMidtone"),   allSwLights[i]->ltfn.duMidtone);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "pointlights[" + std::to_string(numPointLights) + "].duHighlight"), allSwLights[i]->ltfn.duHighlight);

			numPointLights++;*/
		}
		
		else if (allSwLights[i]->lightk == LIGHTK_Direction)
		{
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].dir"),   1, glm::value_ptr(allSwLights[i]->agFallOff));
			glUniform3fv(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].color"), 1, glm::value_ptr(allSwLights[i]->rgbaColor));

			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].ruShadow"),    allSwLights[i]->ltfn.ruShadow);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].ruMidtone"),   allSwLights[i]->ltfn.ruMidtone);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].ruHighlight"), allSwLights[i]->ltfn.ruHighlight);

			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].duShadow"),    allSwLights[i]->ltfn.duShadow);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].duMidtone"),   allSwLights[i]->ltfn.duMidtone);
			glUniform1f(GetUniformLocation(glGlobShader.ID, "dirlights[" + std::to_string(numDirLights) + "].duHighlight"), allSwLights[i]->ltfn.duHighlight);
			
			numDirLights++;
		}
	}

	glUniform1i(glGetUniformLocation(glGlobShader.ID, "numDirLights"),   numDirLights);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "numPointLights"), numPointLights);
	
	glUniform1f(glGetUniformLocation(glGlobShader.ID, "lsm.uShadow"),  g_psw->lsmDefault.uShadow);
	glUniform1f(glGetUniformLocation(glGlobShader.ID, "lsm.uMidtone"), g_psw->lsmDefault.uMidtone);
	
	for (int i = 0; i < allSWAloObjs.size(); i++)
		DrawGlob(allSWAloObjs[i], i);
}

void DrawSwCollisionAll()
{
	glCollisionShader.Use();

	for (int i = 0; i < allSWSoObjs.size(); i++)
		DrawSoCollision(allSWSoObjs[i]);
}