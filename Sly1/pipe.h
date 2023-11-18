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

void InitPipe(PIPE* ppipe);
void OnPipeAdd(PIPE* ppipe);
void OnPipeRemove(PIPE* ppipe);
void DeletePipe(LO* plo);