#pragma once
#include "shape.h"

class PIPE : public SHAPE
{
	public:
		DLE dlePipe;
		glm::vec3 vecFace;
		int fCameraManual;
		float szCameraOffset;
		float sxyCameraOffset;
		SFXID sfxidGrab;
		SFXID sfxidRelease;
};

PIPE*NewPipe();
void InitPipe(PIPE* ppipe);
int  GetPipeSize();
void OnPipeAdd(PIPE* ppipe);
void OnPipeRemove(PIPE* ppipe);
void ClonePipe(PIPE* ppipe, PIPE* ppipeBase);
void DeletePipe(PIPE* ppipe);