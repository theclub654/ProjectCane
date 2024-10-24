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
		//localObject->pvtalo->pfnRenderAloAll((ALO*)localObject, pcm, 0);
		//DrawAlo((ALO*)localObject);
		// Loading next object
		localObject = (LO*)*dlBusyDli.m_ppv;
		// Loading pointer to next object to render
		dlBusyDli.m_ppv = (void**)((uintptr_t)localObject + dlBusyDli.m_ibDle);
	}
}

void DrawSw(SW* psw, CM* pcm)
{

}

void DrawSwAll(SW* psw, GLFWwindow* window)
{
	glGlobShader.Use();

	//PrepareSwLightsForDraw(g_psw);
	
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "shadowTexture"),   0);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "diffuseTexture"),  1);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "saturateTexture"), 2);
	
	/*glUniform3fv(glGetUniformLocation(glGlobShader.ID, "lightColor"), 1, glm::value_ptr(allSwLights[68]->rgbaColor));
	glUniform3fv(glGetUniformLocation(glGlobShader.ID, "lightDir"),   1, glm::value_ptr(allSwLights[68]->agFallOff));

	glUniform1f(glGetUniformLocation(glGlobShader.ID, "ruShadow"),    allSwLights[68]->ltfn.ruShadow);
	glUniform1f(glGetUniformLocation(glGlobShader.ID, "ruMidtone"),   allSwLights[68]->ltfn.ruMidtone);
	glUniform1f(glGetUniformLocation(glGlobShader.ID, "ruHighlight"), allSwLights[68]->ltfn.ruHighlight);

	glUniform1f(glGetUniformLocation(glGlobShader.ID, "duShadow"),    allSwLights[68]->ltfn.duShadow);
	glUniform1f(glGetUniformLocation(glGlobShader.ID, "duMidtone"),   allSwLights[68]->ltfn.duMidtone);
	glUniform1f(glGetUniformLocation(glGlobShader.ID, "duHighlight"), allSwLights[68]->ltfn.duHighlight);*/

	glUniform1f(glGetUniformLocation(glGlobShader.ID, "uShadow"),  g_psw->lsmDefault.uShadow);
	glUniform1f(glGetUniformLocation(glGlobShader.ID, "uMidtone"), g_psw->lsmDefault.uMidtone);
	
	for (int i = 0; i < allSWAloObjs.size(); i++)
		DrawGlob(allSWAloObjs[i], i);
}

void DrawSwCollisionAll()
{
	glCollisionShader.Use();

	for (int i = 0; i < allSWSoObjs.size(); i++)
		DrawSoCollision(allSWSoObjs[i]);
}