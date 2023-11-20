#include "render.h"

void RenderSw(SW *psw, CM *pcm)
{
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

void DrawSw(SW* psw, CM* pcm)
{

}

void DrawSwAll(SW* psw, CM* pcm)
{
	for (int i = 0; i < allSWAloObjs.size(); i++)
		DrawLo(allSWAloObjs[i]);
}