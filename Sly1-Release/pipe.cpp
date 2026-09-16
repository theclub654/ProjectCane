#include "pipe.h"

void StartupPipe()
{
	InitDl(&g_dlPipe, offsetof(PIPE, dlePipe));
}

void ResetPipeList()
{
	ClearDl(&g_dlPipe);
}

PIPE* NewPipe()
{
	return new PIPE{};
}

void InitPipe(PIPE* ppipe)
{
	InitLo(ppipe);

	ppipe->vecFace = g_normalX;
	ppipe->sfxidGrab = static_cast<SFXID>(8);
	ppipe->sfxidRelease = static_cast<SFXID>(9);
	ppipe->unkInt0 = -1;
	ppipe->dzJumpTargetMax = FLT_MAX;
	ppipe->oidRope = OID_Nil;
}

void* GetPipeVecFace(PIPE* ppipe)
{
	return &ppipe->vecFace;
}

void SetPipeVecFace(PIPE* ppipe, glm::vec3 vecFace)
{
	ppipe->vecFace = vecFace;
}

void* GetPipeSfxidGrab(PIPE* ppipe)
{
	return &ppipe->sfxidGrab;
}

void SetPipeSfxidGrab(PIPE* ppipe, int sfxidGrab)
{
	ppipe->sfxidGrab = (SFXID)sfxidGrab;
}

void* GetPipeSfxidRelease(PIPE* ppipe)
{
	return &ppipe->sfxidRelease;
}

void SetPipeSfxidRelease(PIPE* ppipe, int sfxidRelease)
{
	ppipe->sfxidRelease = (SFXID)sfxidRelease;
}

void* GetPipeUnkInt0(PIPE* ppipe)
{
	return &ppipe->unkInt0;
}

void SetPipeUnkInt0(PIPE* ppipe, int unkInt0)
{
	ppipe->unkInt0 = unkInt0;
}

void* GetPipeDzJumpTargetMax(PIPE* ppipe)
{
	return &ppipe->dzJumpTargetMax;
}

void SetPipeDzJumpTargetMax(PIPE* ppipe, float unkFloat)
{
	ppipe->dzJumpTargetMax = unkFloat;
}

void* GetPipeUnkInt1(PIPE* ppipe)
{
	return &ppipe->unkInt1;
}

void SetPipeUnkInt1(PIPE* ppipe, int unkInt1)
{
	ppipe->unkInt1 = unkInt1;
}

void SetPipeOidRope(PIPE* ppipe, int oidRope)
{
	ppipe->oidRope = oidRope;
}

void* GetPipeShape(PIPE* ppipe)
{
	return &ppipe->pshape;
}

void SetPipeShape(PIPE* ppipe, SHAPE* pshape)
{
	ppipe->pshape = pshape;
}

int GetPipeSize()
{
	return sizeof(PIPE);
}

void OnPipeAdd(PIPE* ppipe)
{
	OnLoAdd(ppipe);
	AppendDlEntry(&g_dlPipe, ppipe);
}

void OnPipeRemove(PIPE* ppipe)
{
	OnLoRemove(ppipe);
	RemoveDlEntry(&g_dlPipe, ppipe);
}

void ClonePipe(PIPE* ppipe, PIPE* ppipeBase)
{
	DLE dlePipe = ppipe->dlePipe;

	CloneShape(ppipe, ppipeBase);

	ppipe->dlePipe = ppipeBase->dlePipe;
	ppipe->vecFace = ppipeBase->vecFace;
	ppipe->sfxidGrab = ppipeBase->sfxidGrab;
	ppipe->sfxidRelease = ppipeBase->sfxidRelease;
	ppipe->unkInt0 = ppipeBase->unkInt0;
	ppipe->dzJumpTargetMax = ppipeBase->dzJumpTargetMax;
	ppipe->unkInt1 = ppipeBase->unkInt1;
	ppipe->oidRope = ppipeBase->oidRope;
	ppipe->pshape = ppipeBase->pshape;
	ppipe->prope = ppipeBase->prope;

	ppipe->dlePipe = dlePipe;
}

void PostPipeLoad(PIPE* ppipe)
{
	PostLoLoad(ppipe);
	
	if (ppipe->oidRope != OID_Nil)
		ppipe->prope = (ROPE*)PloFindSwObject(ppipe->psw, 4, (OID)ppipe->oidRope, ppipe);
}

void DeletePipe(PIPE* ppipe)
{
	delete ppipe;
}

DL g_dlPipe;
