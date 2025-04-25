#include "pipe.h"

PIPE* NewPipe()
{
	return new PIPE{};
}

void InitPipe(PIPE* ppipe)
{
	InitShape(ppipe);
}

int GetPipeSize()
{
	return sizeof(PIPE);
}

void OnPipeAdd(PIPE* ppipe)
{
	OnLoAdd(ppipe);
}

void OnPipeRemove(PIPE* ppipe)
{
	OnLoRemove(ppipe);
	//RemoveDlEntry(&g_dlPipe, ppipe);
}

void ClonePipe(PIPE* ppipe, PIPE* ppipeBase)
{
	CloneShape(ppipe, ppipeBase);

	ppipe->dlePipe = ppipeBase->dlePipe;
	ppipe->vecFace = ppipeBase->vecFace;
	ppipe->fCameraManual = ppipeBase->fCameraManual;
	ppipe->szCameraOffset = ppipeBase->szCameraOffset;
	ppipe->sxyCameraOffset = ppipeBase->sxyCameraOffset;
	ppipe->sfxidGrab = ppipeBase->sfxidGrab;
	ppipe->sfxidRelease = ppipeBase->sfxidRelease;
}

void DeletePipe(PIPE* ppipe)
{
	delete ppipe;
}
