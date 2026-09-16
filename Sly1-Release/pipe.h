#pragma once
#include "shape.h"

class PIPE : public SHAPE
{
	public:
	DLE dlePipe;
	glm::vec3 vecFace;
	SFXID sfxidGrab;
	SFXID sfxidRelease;
	int unkInt0;
	float dzJumpTargetMax;
	int unkInt1;
	int oidRope;
	class SHAPE* pshape;
	class ROPE* prope;
};

void StartupPipe();
void ResetPipeList();

PIPE*NewPipe();
void InitPipe(PIPE* ppipe);
void*GetPipeVecFace(PIPE* ppipe);
void SetPipeVecFace(PIPE* ppipe, glm::vec3 vecFace);
void*GetPipeSfxidGrab(PIPE* ppipe);
void SetPipeSfxidGrab(PIPE* ppipe, int sfxidGrab);
void*GetPipeSfxidRelease(PIPE* ppipe);
void SetPipeSfxidRelease(PIPE* ppipe, int sfxidRelease);
void*GetPipeUnkInt0(PIPE* ppipe);
void SetPipeUnkInt0(PIPE* ppipe, int unkInt0);
void*GetPipeDzJumpTargetMax(PIPE* ppipe);
void SetPipeDzJumpTargetMax(PIPE* ppipe, float unkFloat);
void*GetPipeUnkInt1(PIPE* ppipe);
void SetPipeUnkInt1(PIPE* ppipe, int unkInt1);
void SetPipeOidRope(PIPE* ppipe, int oidRope);
void*GetPipeShape(PIPE* ppipe);
void SetPipeShape(PIPE* ppipe, SHAPE* pshape);
int  GetPipeSize();
void OnPipeAdd(PIPE* ppipe);
void OnPipeRemove(PIPE* ppipe);
void ClonePipe(PIPE* ppipe, PIPE* ppipeBase);
void PostPipeLoad(PIPE* ppipe);
void DeletePipe(PIPE* ppipe);

extern DL g_dlPipe;
